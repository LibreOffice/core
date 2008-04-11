/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: writeguard.cxx,v $
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
#include <threadhelp/writeguard.hxx>
#include <macros/debug.hxx>

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
WriteGuard::WriteGuard( IRWLock* pLock )
    :   m_pLock ( pLock     )
    ,   m_eMode ( E_NOLOCK  )
{
    lock();
}

//*****************************************************************************************************************
WriteGuard::WriteGuard( IRWLock& rLock )
    :   m_pLock ( &rLock    )
    ,   m_eMode ( E_NOLOCK  )
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
WriteGuard::~WriteGuard()
{
    unlock();
}

/*-****************************************************************************************************//**
    @short      set write lock
    @descr      Call this method to set the write lock. The call will block till all current threads are synchronized!

    @seealso    method unlock()

    @param      -
    @return     -

    @onerror    -
*//*-*****************************************************************************************************/
void WriteGuard::lock()
{
    switch( m_eMode )
    {
        case E_NOLOCK       :   {
                                    // Acquire write access and set return state.
                                    // Mode is set later if it was successful!
                                    m_pLock->acquireWriteAccess();
                                    m_eMode = E_WRITELOCK;
                                }
                                break;
        case E_READLOCK     :   {
                                    // User has downgrade to read access before!
                                    // We must release it before we can set a new write access!
                                    m_pLock->releaseReadAccess ();
                                    m_pLock->acquireWriteAccess();
                                    m_eMode = E_WRITELOCK;
                                }
                                break;
    }
}

/*-****************************************************************************************************//**
    @short      unset write lock
    @descr      Call this method to unlock the rw-lock temp.!
                Normaly we do it at dtor automaticly for you ...

    @seealso    method lock()

    @param      -
    @return     -

    @onerror    -
*//*-*****************************************************************************************************/
void WriteGuard::unlock()
{
    switch( m_eMode )
    {
        case E_READLOCK     :   {
                                    // Use has downgraded to a read lock!
                                    m_pLock->releaseReadAccess();
                                    m_eMode = E_NOLOCK;
                                }
                                break;
        case E_WRITELOCK    :   {
                                    m_pLock->releaseWriteAccess();
                                    m_eMode = E_NOLOCK;
                                }
                                break;
    }
}

/*-****************************************************************************************************//**
    @short      downgrade write access to read access without new blocking!
    @descr      If this write lock is set you can change it to a "read lock".
                An "upgrade" is the same like new calling "lock()"!

    @seealso    -

    @param      -
    @return     -

    @onerror    -
*//*-*****************************************************************************************************/
void WriteGuard::downgrade()
{
    if( m_eMode == E_WRITELOCK )
    {
        m_pLock->downgradeWriteAccess();
        m_eMode = E_READLOCK;
    }
}

/*-****************************************************************************************************//**
    @short      return internal lock state
    @descr      For user they dont know what they are doing there ...

    @seealso    -

    @param      -
    @return     Current set lock mode.

    @onerror    No error should occure.
*//*-*****************************************************************************************************/
ELockMode WriteGuard::getMode() const
{
    return m_eMode;
}

}   //  namespace framework
