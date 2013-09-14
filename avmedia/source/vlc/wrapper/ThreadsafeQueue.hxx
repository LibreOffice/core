/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _THREADSAFE_QUEUE_HXX
#define _THREADSAFE_QUEUE_HXX
#include <queue>
#include <iostream>
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>
#include <boost/noncopyable.hpp>

namespace avmedia
{
namespace vlc
{
namespace wrapper
{
template<class T>
class ThreadsafeQueue : boost::noncopyable
{
public:
    ThreadsafeQueue();

    void push( const T& data );
    void pop( T& data );

private:
    std::queue< T > mQueue;
    mutable ::osl::Mutex mMutex;
    ::osl::Condition mCondition;
};

template<class T>
ThreadsafeQueue<T>::ThreadsafeQueue()
{
}

template<class T>
void ThreadsafeQueue<T>::push( const T& data )
{
    ::osl::MutexGuard guard( mMutex );
    mQueue.push( data );
    mMutex.release();
    mCondition.set();
}

template<class T>
void ThreadsafeQueue<T>::pop( T& data )
{
    mCondition.wait();
    ::osl::MutexGuard guard( mMutex );
    while ( mQueue.empty() )
    {
        mMutex.release();
        mCondition.wait();
        mMutex.acquire();
    }
    data = mQueue.front();
    mQueue.pop();
}
}
}
}

#endif // _THREADSAFE_QUEUE_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */