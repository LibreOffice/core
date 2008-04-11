/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: readguard.cxx,v $
 * $Revision: 1.6 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <threadhelp/readguard.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  non exported declarations
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  definitions
//_________________________________________________________________________________________________________________

/*-****************************************************************************************************//**
    @short      ctor
    @descr      These ctors initialize the guard with a reference to used lock member of object to protect.
                Null isn't allowed as value!

    @seealso    -

    @param      "pLock" ,reference to used lock member of object to protect
    @param      "rLock" ,reference to used lock member of object to protect
    @return     -

    @onerror    -
*//*-*****************************************************************************************************/
ReadGuard::ReadGuard( IRWLock* pLock )
    :   m_pLock     ( pLock     )
    ,   m_bLocked   ( sal_False )
{
    lock();
}

//*****************************************************************************************************************
ReadGuard::ReadGuard( IRWLock& rLock )
    :   m_pLock     ( &rLock    )
    ,   m_bLocked   ( sal_False )
{
    lock();
}

/*-****************************************************************************************************//**
    @short      dtor
    @descr      We unlock the used lock member automaticly if user forget it.

    @seealso    -

    @param      -
    @return     -

    @onerror    -
*//*-*****************************************************************************************************/
ReadGuard::~ReadGuard()
{
    unlock();
}

/*-****************************************************************************************************//**
    @short      set read lock
    @descr      Call this method to set the read lock. The call will block till all current threads are synchronized!

    @seealso    method unlock()

    @param      -
    @return     -

    @onerror    -
*//*-*****************************************************************************************************/
void ReadGuard::lock()
{
    if( m_bLocked == sal_False )
    {
        m_pLock->acquireReadAccess();
        m_bLocked = sal_True;
    }
}

/*-****************************************************************************************************//**
    @short      unset read lock
    @descr      Call this method to unlock the rw-lock temp.!
                Normaly we do it at dtor automaticly for you ...

    @seealso    method lock()

    @param      -
    @return     -

    @onerror    -
*//*-*****************************************************************************************************/
void ReadGuard::unlock()
{
    if( m_bLocked == sal_True )
    {
        m_pLock->releaseReadAccess();
        m_bLocked = sal_False;
    }
}

/*-****************************************************************************************************//**
    @short      return internal lock state
    @descr      For user they dont know what they are doing there ...

    @seealso    -

    @param      -
    @return     true, if lock is set, false otherwise.

    @onerror    -
*//*-*****************************************************************************************************/
sal_Bool ReadGuard::isLocked() const
{
    return m_bLocked;
}

}   //  namespace framework
