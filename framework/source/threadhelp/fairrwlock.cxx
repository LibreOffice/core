/*************************************************************************
 *
 *  $RCSfile: fairrwlock.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2001-03-29 13:17:16 $
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

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_THREADHELP_FAIRRWLOCK_HXX_
#include <threadhelp/fairrwlock.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
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

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
FairRWLock::FairRWLock()
    :   m_nReadCount    ( 0         )
    ,   m_eWorkingMode  ( E_INIT    )
{
    m_aWriteCondition.set();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SAL_CALL FairRWLock::setWorkingMode( EWorkingMode eMode )
{
    // We need the access lock only here - because
    // new calls are influenced by this call.
    // Current working reader or writer has already checked this value!
    // It's to late to refuse her requests ...
    ResetableGuard aAccessGuard( m_aAccessLock );

    if  (
            ( m_eWorkingMode    ==  E_INIT  )   &&
            ( eMode             ==  E_WORK  )
        )
    {
        m_eWorkingMode = E_WORK;
    }
    else
    if  (
            ( m_eWorkingMode    ==  E_WORK  )   &&
            ( eMode             ==  E_CLOSE )
        )
    {
        m_eWorkingMode = E_CLOSE;
    }
    else
    if  (
            ( m_eWorkingMode    ==  E_CLOSE )   &&
            ( eMode             ==  E_INIT  )
        )
    {
        m_eWorkingMode = E_INIT;
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
EWorkingMode SAL_CALL FairRWLock::getWorkingMode()
{
    // We don't must stand in serializer-queue!
    // But synchronize access to internal member.
    ResetableGuard aAccessLock( m_aAccessLock );

    return m_eWorkingMode;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SAL_CALL FairRWLock::acquireReadAccess( ERefusalReason& eReason )
{
    // impl-call is threadsafe himself!
    if( impl_isCallRefused( eReason ) == sal_False )
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
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
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

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SAL_CALL FairRWLock::acquireWriteAccess( ERefusalReason& eReason )
{
    // impl-call is threadsafe himself!
    if( impl_isCallRefused( eReason ) == sal_False )
    {
        // You have to stand in our serialize-queue till all reader
        // are registered (not for releasing them!) or writer finished their work!
        // Don't use a guard to do so - because you must hold the mutex till
        // you call releaseWriteAccess()!
        // After succesfull acquire you have to wait for current working reader.
        // Used condition will open by last gone reader object.
        m_aSerializer.acquire();
        m_aWriteCondition.wait();
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SAL_CALL FairRWLock::releaseWriteAccess()
{
    // The only one you have to do here is to release
    // hold seriliaze-mutex. All other user of these instance are blocked
    // by these mutex!
    // You don't need any other mutex here - you are the only one in the moment!
    m_aSerializer.release();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SAL_CALL FairRWLock::downgradeWriteAccess()
{
    // You must be a writer to call this method!
    // We can't check it - but otherwise it's your problem ...
    // Thats why you don't need any mutex here.

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

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
sal_Bool FairRWLock::impl_isCallRefused( ERefusalReason& eReason )
{
    // This call must safe access to internal member only.
    // Set "possible reason" for return and check reject-state then!
    // User should look ro return value first - reason then ...
    ResetableGuard aAccessGuard( m_aAccessLock );
    switch( m_eWorkingMode )
    {
        case E_INIT     :   eReason = E_UNINITIALIZED   ;
                            break;
        case E_WORK     :   eReason = E_NOREASON        ;
                            break;
        case E_CLOSE    :   eReason = E_CLOSED          ;
                            break;
    }
    return( eReason!=E_NOREASON );
}

}   //  namespace framework
