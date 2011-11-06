/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/




#ifndef _SYNCOBJECTS_HXX_
#define _SYNCOBJECTS_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#include <windows.h>

//------------------------------------------------------------------------
// a simple helper template for automatic locking/unlocking
//------------------------------------------------------------------------

template< class LOCK >
class CLockGuard
{
public:
    CLockGuard( LOCK* aLock ) :
        m_pLock( aLock )
    {
        m_pLock->Lock( );
    }

    ~CLockGuard( )
    {
        m_pLock->Unlock( );
    }

private:
    LOCK* m_pLock;
};

//------------------------------------------------------------------------
// a interface base class for different locking sub classes
//------------------------------------------------------------------------

class CSyncObject
{
public:
    virtual ~CSyncObject( ) = 0;

    virtual int Lock( )     = 0;
    virtual int Unlock( )   = 0;
};

//------------------------------------------------------------------------
// if no synchronization is necessary this class will be used
// declaring the functions as inline safes runtime overhead
//------------------------------------------------------------------------

class CNullLock
{
public:
    inline virtual ~CNullLock ( ) {};
    inline virtual int Lock( )    {};
    inline virtual int Unlock()   {};
};

//------------------------------------------------------------------------
// a minimal wrapper for a win32 critical section
//------------------------------------------------------------------------

class CCriticalSection : public CSyncObject
{
public:
    CCriticalSection( );
    virtual ~CCriticalSection( );

    // both functions return always 0
    // because the win32 critsec functions
    // don't return any return code
    virtual int Lock( );
    virtual int Unlock( );

private:
    CRITICAL_SECTION m_critSec;
};


typedef CLockGuard< CSyncObject > SyncObjLockGuard_t;

#endif
