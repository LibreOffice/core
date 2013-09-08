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

#ifndef __FRAMEWORK_THREADHELP_TRANSACTIONMANAGER_HXX_
#define __FRAMEWORK_THREADHELP_TRANSACTIONMANAGER_HXX_

#include <threadhelp/inoncopyable.h>
#include <threadhelp/itransactionmanager.h>
#include <threadhelp/gate.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

#include <osl/mutex.hxx>
#include <fwidllapi.h>

namespace framework{

/*-************************************************************************************************************//**
    @short          implement a transaction manager to support non breakable interface methods
    @descr          Use it to support non breakable interface methods without using any thread
                    synchronization like e.g. mutex, rw-lock!
                    That protect your code against wrong calls at wrong time ... e.g. calls after disposing an object!
                    Use combination of EExceptionMode and ERejectReason to detect rejected requests
                    and react for it. You can enable automaticly throwing of exceptions too.

    @implements     ITransactionManager
    @base           INonCopyable
                    ITransactionManager

    @devstatus      draft
*//*-*************************************************************************************************************/
class FWI_DLLPUBLIC TransactionManager : public  ITransactionManager
                         , private INonCopyable
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

                                   TransactionManager           (                                              );
        virtual                    ~TransactionManager          (                                              );
        virtual void               setWorkingMode               ( EWorkingMode eMode                           );
        virtual EWorkingMode       getWorkingMode               (                                              ) const;
        virtual sal_Bool           isCallRejected               ( ERejectReason& eReason                       ) const;
        virtual void               registerTransaction          ( EExceptionMode eMode, ERejectReason& eReason ) throw( css::uno::RuntimeException, css::lang::DisposedException );
        virtual void               unregisterTransaction        (                                              ) throw( css::uno::RuntimeException, css::lang::DisposedException );

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------
    private:

        void impl_throwExceptions( EExceptionMode eMode, ERejectReason eReason ) const throw( css::uno::RuntimeException, css::lang::DisposedException );

    //-------------------------------------------------------------------------------------------------------------
    //  private member
    //-------------------------------------------------------------------------------------------------------------
    private:

        mutable ::osl::Mutex    m_aAccessLock           ;   /// regulate access on internal member of this instance
        Gate                    m_aBarrier              ;   /// used to block transactions requests during change or work mode
        EWorkingMode            m_eWorkingMode          ;   /// current working mode of object which use this manager (used to reject calls at wrong time)
        sal_Int32               m_nTransactionCount     ;   /// every transaction request is registered by this counter

};      //  class TransactionManager

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_THREADHELP_TRANSACTIONMANAGER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
