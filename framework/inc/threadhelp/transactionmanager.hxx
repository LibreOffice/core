/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef __FRAMEWORK_THREADHELP_TRANSACTIONMANAGER_HXX_
#define __FRAMEWORK_THREADHELP_TRANSACTIONMANAGER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <threadhelp/inoncopyable.h>
#include <threadhelp/itransactionmanager.h>
#include <threadhelp/gate.hxx>
#include <macros/debug.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <osl/mutex.hxx>
#include <fwidllapi.h>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

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
