/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SyncObjects.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:48:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
