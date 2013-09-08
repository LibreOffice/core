/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <threadhelp/transactionmanager.hxx>
#include <threadhelp/resetableguard.hxx>
#include <macros/debug.hxx>

#include <macros/generic.hxx>
#include <fwidllapi.h>

#include <com/sun/star/lang/DisposedException.hpp>

namespace framework{

/*-************************************************************************************************************//**
    @short      standard ctor
    @descr      Initialize instance with right start values for correct working.

    @seealso    -

    @param      -
    @return     -

    @onerror    -
*//*-*************************************************************************************************************/
TransactionManager::TransactionManager()
    : m_eWorkingMode      ( E_INIT )
    , m_nTransactionCount ( 0      )
{
    m_aBarrier.open();
}

/*-************************************************************************************************************//**
    @short      standard dtor
    @descr      -

    @seealso    -

    @param      -
    @return     -

    @onerror    -
*//*-*************************************************************************************************************/
TransactionManager::~TransactionManager()
{
}

/*-****************************************************************************************************//**
    @interface  ITransactionManager
    @short      set new working mode
    @descr      These implementation knows for states of working: E_INIT, E_WORK, E_CLOSING, E_CLOSE
                You can step during this ones only from the left to the right side and start at left side again!
                (This is neccessary e.g. for refcounted objects!)
                This call will block till all current existing transactions was finished.
                Follow results occure:
                    E_INIT        :  All requests on this implementation are refused.
                                        It's your decision to react in a right way.

                    E_WORK        :  The object can work now. The full functionality is available.

                    E_BEFORECLOSE :  The object start the closing mechanism ... but sometimes
                                        e.g. the dispose() method need to call some private methods.
                                        These some special methods should use E_SOFTEXCEPTIONS or ignore
                                        E_INCLOSE as returned reason for E_NOEXCEPTIONS to detect this special case!

                    E_CLOSE       :  Object is already dead! All further requests will be refused.
                                        It's your decision to react in a right way.

    @seealso    -

    @param      "eMode", is the new mode - but we don't accept setting mode in wrong order!
    @return     -

    @onerror    We do nothing.
*//*-*****************************************************************************************************/
void  TransactionManager::setWorkingMode( EWorkingMode eMode )
{
    // Safe member access.
    ::osl::ClearableMutexGuard  aAccessGuard( m_aAccessLock );
    sal_Bool                    bWaitFor    = sal_False      ;
    // Change working mode first!
    if  (
            ( m_eWorkingMode == E_INIT        && eMode == E_WORK        ) ||
            ( m_eWorkingMode == E_WORK        && eMode == E_BEFORECLOSE ) ||
            ( m_eWorkingMode == E_BEFORECLOSE && eMode == E_CLOSE       ) ||
            ( m_eWorkingMode == E_CLOSE       && eMode == E_INIT        )
        )
    {
        m_eWorkingMode = eMode;
        if( m_eWorkingMode == E_BEFORECLOSE || m_eWorkingMode == E_CLOSE )
        {
            bWaitFor = sal_True;
        }
    }

    // Wait for current existing transactions then!
    // (Only necessary for changing to E_BEFORECLOSE or E_CLOSE! ...
    // otherwise; if you wait at setting E_WORK another thrad could finish a acquire-call during our unlock() and wait() call
    // ... and we will wait forever here!!!)
    // Don't forget to release access mutex before.
    aAccessGuard.clear();
    if( bWaitFor == sal_True )
    {
        m_aBarrier.wait();
    }
}

/*-****************************************************************************************************//**
    @interface  ITransactionManager
    @short      get current working mode
    @descr      If you stand in your close() or init() method ... but don't know
                if you called more then ones(!) ... you can use this function to get
                right information.
                e.g:    You have a method init() which is used to change working mode from
                        E_INIT to E_WORK and should be used to initialize some member too ...
                        What should you do:

                            void init( sal_Int32 nValue )
                            {
                                // Reject this call if our transaction manager say: "Object already initialized!"
                                // Otherwise initialize your member.
                                if( m_aTransactionManager.getWorkingMode() == E_INIT )
                                {
                                    // Object is uninitialized ...
                                    // Make member access threadsafe!
                                    ResetableGuard aGuard( m_aMutex );

                                    // Check working mode again .. because another instance could be faster.
                                    // (It's possible to set this guard at first of this method too!)
                                    if( m_aTransactionManager.getWorkingMode() == E_INIT )
                                    {
                                        m_aMember = nValue;

                                        // Object is initialized now ... set working mode to E_WORK!
                                        m_aTransactionManager.setWorkingMode( E_WORK );
                                    }
                                }
                            }

    @seealso    method setWorkingMode()

    @param      -
    @return     Current set mode.

    @onerror    No error should occure.
*//*-*****************************************************************************************************/
EWorkingMode TransactionManager::getWorkingMode() const
{
    // Synchronize access to internal member!
    ::osl::MutexGuard aAccessLock( m_aAccessLock );
    return m_eWorkingMode;
}

/*-****************************************************************************************************//**
    @interface  ITransactionManager
    @short      start new transaction
    @descr      A guard should use this method to start a new transaction. He should looks for rejected
                calls to by using parameter eMode and eReason.
                If call was not rejected your transaction will be non breakable during releasing your transaction
                guard! BUT ... your code isn't threadsafe then! It's a transaction manager only ....

    @seealso    method unregisterTransaction()

    @param      "eMode"     ,used to enable/disable throwing exceptions automatically for rejected calls
    @param      "eReason"   ,reason for rejected calls if eMode=E_NOEXCEPTIONS
    @return     -

    @onerror    -
*//*-*****************************************************************************************************/
void  TransactionManager::registerTransaction( EExceptionMode eMode, ERejectReason& eReason ) throw( css::uno::RuntimeException, css::lang::DisposedException )
{
    // Look for rejected calls first.
    // If call was refused we throw some exceptions or do nothing!
    // It depends from given parameter eMode.
    if( isCallRejected( eReason ) == sal_True )
    {
        impl_throwExceptions( eMode, eReason );
    }

    // BUT if no exception was thrown ... (may be eMode = E_SOFTEXCEPTIONS!)
    // we must register this transaction too!
    // Don't use "else" or a new scope here!!!

    // Safe access to internal member.
    ::osl::MutexGuard aAccessGuard( m_aAccessLock );

    // Register this new transaction.
    // If it is the first one .. close gate to disable changing of working mode.
    ++m_nTransactionCount;
    if( m_nTransactionCount == 1 )
    {
        m_aBarrier.close();
    }
}

/*-****************************************************************************************************//**
    @interface  ITransactionManager
    @short      finish transaction
    @descr      A guard should call this method to release current transaction.

    @seealso    method registerTransaction()

    @param      -
    @return     -

    @onerror    -
*//*-*****************************************************************************************************/
void  TransactionManager::unregisterTransaction() throw( css::uno::RuntimeException, css::lang::DisposedException )
{
    // This call could not rejected!
    // Safe access to internal member.
    ::osl::MutexGuard aAccessGuard( m_aAccessLock );

    // Deregister this transaction.
    // If it was the last one ... open gate to enable changing of working mode!
    // (see setWorkingMode())

    --m_nTransactionCount;
    if( m_nTransactionCount == 0 )
    {
        m_aBarrier.open();
    }
}

/*-****************************************************************************************************//**
    @interface  ITransactionManager
    @short      look for rejected calls
    @descr      Sometimes user need a possibility to get information about rejected calls
                without starting a transaction!

    @seealso    -

    @param      "eReason" returns reason of a rejected call
    @return     true if call was rejected, false otherwise

    @onerror    We return false.
*//*-*****************************************************************************************************/
sal_Bool  TransactionManager::isCallRejected( ERejectReason& eReason ) const
{
    // This call must safe access to internal member only.
    // Set "possible reason" for return and check reject-state then!
    // User should look for return value first - reason then ...
    ::osl::MutexGuard aAccessGuard( m_aAccessLock );
    switch( m_eWorkingMode )
    {
        case E_INIT        : eReason = E_UNINITIALIZED ;
                                break;
        case E_WORK        : eReason = E_NOREASON      ;
                                break;
        case E_BEFORECLOSE : eReason = E_INCLOSE       ;
                                break;
        case E_CLOSE       : eReason = E_CLOSED        ;
                                break;
    }
    return( eReason!=E_NOREASON );
}

/*-****************************************************************************************************//**
    @short      throw any exceptions for rejected calls
    @descr      If a user wishes to use our automatic exception mode we use this impl-method.
                We check all combinations of eReason and eExceptionMode and throw correct exception with some
                descriptions for the recipient.

    @seealso    method registerTransaction()
    @seealso    enum ERejectReason
    @seealso    enum EExceptionMode

    @param      "eReason" , reason for rejected call
    @param      "eMode"   , exception mode - set by user
    @return     -

    @onerror    -
*//*-*****************************************************************************************************/
void TransactionManager::impl_throwExceptions( EExceptionMode eMode, ERejectReason eReason ) const throw( css::uno::RuntimeException, css::lang::DisposedException )
{
    if( eMode != E_NOEXCEPTIONS )
    {
        switch( eReason )
        {
            case E_UNINITIALIZED   :    if( eMode == E_HARDEXCEPTIONS )
                                        {
                                            // Help programmer to find out, why this exception is thrown!
                                            SAL_WARN( "fwk", "TransactionManager...: Owner instance not correctly initialized yet. Call was rejected! Normally it's an algorithm error ... wrong use of class!" );
                                            //ATTENTION: temp. disabled - till all bad code positions are detected and changed! */
                                            // throw css::uno::RuntimeException( DECLARE_ASCII("TransactionManager...\nOwner instance not right initialized yet. Call was rejected! Normaly it's an algorithm error ... wrong usin of class!\n" ), css::uno::Reference< css::uno::XInterface >() );
                                        }
                                        break;
            case E_INCLOSE         :    if( eMode == E_HARDEXCEPTIONS )
                                        {
                                            // Help programmer to find out, why this exception is thrown!
                                            SAL_WARN( "fwk", "TransactionManager...: Owner instance stand in close method. Call was rejected!" );
                                            throw css::lang::DisposedException( DECLARE_ASCII("TransactionManager...\nOwner instance stand in close method. Call was rejected!\n" ), css::uno::Reference< css::uno::XInterface >() );
                                        }
                                        break;
            case E_CLOSED           :   {
                                            // Help programmer to find out, why this exception is thrown!
                                            SAL_WARN( "fwk", "TransactionManager...: Owner instance already closed. Call was rejected!" );
                                            throw css::lang::DisposedException( DECLARE_ASCII("TransactionManager...\nOwner instance already closed. Call was rejected!\n" ), css::uno::Reference< css::uno::XInterface >() );
                                        }
            case E_NOREASON         :   {
                                            // Help programmer to find out
                                            SAL_WARN( "fwk", "TransactionManager...: Impossible case E_NOREASON!" );
                                        }
                                        break;
            default:                    break; // nothing to do
        }
    }
}

}   //  namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
