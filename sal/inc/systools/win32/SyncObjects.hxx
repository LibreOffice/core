/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
