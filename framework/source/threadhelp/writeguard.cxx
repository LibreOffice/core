/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: writeguard.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 14:14:50 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

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
