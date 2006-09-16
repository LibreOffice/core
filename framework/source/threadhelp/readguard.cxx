/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: readguard.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 14:13:53 $
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

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
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
