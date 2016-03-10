#include <Windows.h>
#include <fstream>
#include <atomic>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>


using std::ofstream;
using std::queue;
using std::atomic_int;
using std::condition_variable;
using std::mutex;
using std::unique_lock;

template <typename T>
class Queue
{
 public:
	
	queue<T> m_queue;
	condition_variable m_cond;
	mutex m_mutex;
	atomic_int counter;
	int writeIndex;

	Queue() : writeIndex(0) { }

	void push(T elem)
	{
		int numOrder = counter++;

		unique_lock<mutex> lock(m_mutex);
		while(writeIndex != numOrder)
		{
			m_cond.notify_one();
			m_cond.wait(lock);
		}

		m_queue.push(elem);
		writeIndex++;
		m_cond.notify_one();
	}

	T pop()
	{
		unique_lock<mutex> lock(m_mutex);
		while (m_queue.empty())
		{
			m_cond.wait(lock);
		}
		auto item = m_queue.front();
		m_queue.pop();
		return item;
	}

	void lockQueueFor3Seconds()
	{
		m_mutex.lock();
		Sleep(3000);
		m_mutex.unlock();
	}
};


ofstream myFile("result.txt");
Queue<char> myQueue;
char data = 'a';

DWORD WINAPI WritingThread(LPVOID lpParam);
DWORD WINAPI LockingThread(LPVOID lpParam);

int main()
{
	// This thread will block myQueue for 3 seconds
	CreateThread(NULL, 0, LockingThread, NULL, 0, NULL);

	// During the locked period, I ask myQueue to push letters from a to z
	for (int i = 0; i < 26; i++)
		CreateThread(NULL, 0, WritingThread, (LPVOID)0, 0, NULL);

	// If the mutex could wake up in order, myQueue would pop up the letters in order, but it doesn't.
	for (int i = 0; i < 26; i++)
		myFile << myQueue.pop() << ",";

	return EXIT_SUCCESS;
}

DWORD WINAPI LockingThread(LPVOID lpParam)
{
	myQueue.lockQueueFor3Seconds();
	return 0;
}

DWORD WINAPI WritingThread(LPVOID lpParam)
{
	myQueue.push(data++);
	return 0;
}
