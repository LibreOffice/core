/*************************************************************************
 *
 *  $RCSfile: SyncObjects.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tra $ $Date: 2000-09-29 13:10:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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