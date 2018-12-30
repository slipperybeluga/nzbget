/*
 *  This file is part of nzbget. See <http://nzbget.net>.
 *
 *  Copyright (C) 2004 Sven Henkel <sidddy@users.sourceforge.net>
 *  Copyright (C) 2007-2016 Andrey Prygunkov <hugbug@users.sourceforge.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef THREAD_H
#define THREAD_H

typedef std::mutex Mutex;
typedef std::lock_guard<std::mutex> Guard;
typedef std::unique_lock<std::mutex> UniqueLock;

template<typename T>
class GuardedPtr
{
public:
	GuardedPtr(T* ptr, Mutex* mutex) : m_ptr(ptr), m_mutex(mutex) { if (m_mutex) m_mutex->lock(); }
	GuardedPtr(GuardedPtr&& other) : m_ptr(other.m_ptr), m_mutex(other.m_mutex) { other.m_mutex = nullptr; }
	GuardedPtr(const GuardedPtr& other) = delete;
	~GuardedPtr() { Unlock(); }
	T* operator->() { return m_ptr; }
	operator T*() { return m_ptr; }

	// for-range loops on GuardedPtr
	auto begin() { return m_ptr->begin(); }
	auto end() { return m_ptr->end(); }

private:
	T* m_ptr;
	Mutex* m_mutex;

	void Unlock() { if (m_mutex) { m_mutex->unlock(); m_mutex = nullptr; } }
};

class Thread
{
public:
	Thread();
	Thread(const Thread&) = delete;
	virtual ~Thread();

	virtual void Start();
	virtual void Stop();
	virtual void Resume();
	bool Kill();

	bool IsStopped() { return m_stopped; };
	bool IsRunning() const { return m_running; }
	bool GetAutoDestroy() { return m_autoDestroy; }
	void SetAutoDestroy(bool autoDestroy) { m_autoDestroy = autoDestroy; }
	static int GetThreadCount();

protected:
	virtual void Run() {}; // Virtual function - override in derivatives

private:
	static Mutex m_threadMutex;
	static std::atomic_int m_threadCount;
#ifdef WIN32
	HANDLE m_threadObj = 0;
#else
	pthread_t m_threadObj = 0;
#endif
	std::atomic_bool m_running{false};
	std::atomic_bool m_stopped{false};
	bool m_autoDestroy = false;
#ifdef WIN32
	static void __cdecl thread_handler(void* object);
#else
	static void *thread_handler(void* object);
#endif
};

#endif
