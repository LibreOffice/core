/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fairrwlock.cxx,v $
 * $Revision: 1.7 $
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
#include <threadhelp/fairrwlock.hxx>
#include <threadhelp/resetableguard.hxx>
#include <macros/debug.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

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
    @short      standard ctor
    @descr      Initialize instance with right start values for correct working.
                no reader could exist               =>  m_nReadCount   = 0
                don't block first comming writer    =>  m_aWriteCondition.set()

    @seealso    -

    @param      -
    @return     -

    @onerror    -
*//*-*****************************************************************************************************/
FairRWLock::FairRWLock()
    : m_nReadCount( 0 )
{
    m_aWriteCondition.set();
}

/*-****************************************************************************************************//**
    @short      set lock for reading
    @descr      A guard should call this method to acquire read access on your member.
                Writing isn't allowed then - but nobody could check it for you!

    @seealso    method releaseReadAccess()

    @param      -
    @return     -

    @onerror    -
*//*-*****************************************************************************************************/
void SAL_CALL FairRWLock::acquireReadAccess()
{
    // Put call in "SERIALIZE"-queue!
    // After successful acquiring this mutex we are alone ...
    ResetableGuard aSerializeGuard( m_aSerializer );

    // ... but we should synchronize us with other reader!
    // May be - they will unregister himself by using releaseReadAccess()!
    ResetableGuard aAccessGuard( m_aAccessLock );

    // Now we must register us as reader by increasing counter.
    // If this the first writer we must close door for possible writer.
    // Other reader don't look for this barrier - they work parallel to us!
    if( m_nReadCount == 0 )
    {
        m_aWriteCondition.reset();
    }
    ++m_nReadCount;
}

/*-****************************************************************************************************//**
    @short      reset lock for reading
    @descr      A guard should call this method to release read access on your member.

    @seealso    method acquireReadAccess()

    @param      -
    @return     -

    @onerror    -
*//*-*****************************************************************************************************/
void SAL_CALL FairRWLock::releaseReadAccess()
{
    // The access lock is enough at this point
    // because it's not allowed to wait for all reader or writer here!
    // That will cause a deadlock!
    ResetableGuard aAccessGuard( m_aAccessLock );

    // Unregister as reader first!
    // Open writer barrier then if it was the last reader.
    --m_nReadCount;
    if( m_nReadCount == 0 )
    {
        m_aWriteCondition.set();
    }
}

/*-****************************************************************************************************//**
    @short      set lock for writing
    @descr      A guard should call this method to acquire write access on your member.
                Reading is allowed too - of course.
                After successfully calling of this method you are the only writer.

    @seealso    method setWorkingMode()
    @seealso    method releaseWriteAccess()

    @param      "eRejectReason"     , is the reason for rejected calls.
    @param      "eExceptionMode"    , use to enable/disable throwing exceptions automaticly for rejected calls
    @return     -

    @onerror    -
*//*-*****************************************************************************************************/
void SAL_CALL FairRWLock::acquireWriteAccess()
{
    // You have to stand in our serialize-queue till all reader
    // are registered (not for releasing them!) or writer finished their work!
    // Don't use a guard to do so - because you must hold the mutex till
    // you call releaseWriteAccess()!
    // After succesfull acquire you have to wait for current working reader.
    // Used condition will open by last gone reader object.
    m_aSerializer.acquire();
    m_aWriteCondition.wait();

    #ifdef ENABLE_MUTEXDEBUG
    // A writer is an exclusiv accessor!
    LOG_ASSERT2( m_nReadCount!=0, "FairRWLock::acquireWriteAccess()", "No threadsafe code detected ... : Read count != 0!" )
    #endif
}

/*-****************************************************************************************************//**
    @short      reset lock for writing
    @descr      A guard should call this method to release write access on your member.

    @seealso    method acquireWriteAccess()

    @param      -
    @return     -

    @onerror    -
*//*-*****************************************************************************************************/
void SAL_CALL FairRWLock::releaseWriteAccess()
{
    // The only one you have to do here is to release
    // hold seriliaze-mutex. All other user of these instance are blocked
    // by these mutex!
    // You don't need any other mutex here - you are the only one in the moment!

    #ifdef ENABLE_MUTEXDEBUG
    // A writer is an exclusiv accessor!
    LOG_ASSERT2( m_nReadCount!=0, "FairRWLock::releaseWriteAccess()", "No threadsafe code detected ... : Read count != 0!" )
    #endif

    m_aSerializer.release();
}

/*-****************************************************************************************************//**
    @short      downgrade a write access to a read access
    @descr      A guard should call this method to change a write to a read access.
                New readers can work too - new writer are blocked!

    @attention  Don't call this method if you are not a writer!
                Results are not defined then ...
                An upgrade can't be implemented realy ... because acquiring new access
                will be the same - there no differences!

    @seealso    -

    @param      -
    @return     -

    @onerror    -
*//*-*****************************************************************************************************/
void SAL_CALL FairRWLock::downgradeWriteAccess()
{
    // You must be a writer to call this method!
    // We can't check it - but otherwise it's your problem ...
    // Thats why you don't need any mutex here.

    #ifdef ENABLE_MUTEXDEBUG
    // A writer is an exclusiv accessor!
    LOG_ASSERT2( m_nReadCount!=0, "FairRWLock::downgradeWriteAccess()", "No threadsafe code detected ... : Read count != 0!" )
    #endif

    // Register himself as "new" reader.
    // This value must be 0 before - because we support single writer access only!
    ++m_nReadCount;
    // Close barrier for other writer!
    // Why?
    // You hold the serializer mutex - next one can be a reader OR a writer.
    // They must blocked then - because you will be a reader after this call
    // and writer use this condition to wait for current reader!
    m_aWriteCondition.reset();
    // Open door for next waiting thread in serialize queue!
    m_aSerializer.release();
}

}   //  namespace framework
