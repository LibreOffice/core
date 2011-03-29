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

#ifndef __FRAMEWORK_THREADHELP_TRANSACTIONGUARD_HXX_
#define __FRAMEWORK_THREADHELP_TRANSACTIONGUARD_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <threadhelp/inoncopyable.h>
#include <threadhelp/itransactionmanager.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

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
    @short          implement a guard to support non breakable transactions
    @descr          If you whish to support non breakable method calls without lockingf any mutex, rw-lock or
                    something like that - you should use this guard implementation.
                    Initialize it at first in your method and don't release it till end of your function!
                    Your "transaction" is registered in ctor and automaticly released in dtor.
                    Use set/get of working mode to enable/disable further transactions.
                    It's possible too, to enable automaticly throwing of some exceptions for illegal
                    transaction requests ... e.g. interface call for already disposed objects.

    @attention      To prevent us against wrong using, the default ctor, copy ctor and the =operator are maked private!

    @implements     -
    @base           INonCopyable

    @devstatus      draft
*//*-*************************************************************************************************************/
class TransactionGuard : private INonCopyable
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

        /*-****************************************************************************************************//**
            @short      ctors
            @descr      Use these ctor methods to initialize the guard right.
                        Given reference must be valid - otherwise crashes could occure!

            @attention  It's not neccessary to lock any mutex here! Because a ctor should not be called
                        from different threads at the same time ... this class use no refcount mechanism!

            @seealso    -

            @param      "rManager"  reference to transaction manager for using to register a request
            @param      "eMode"     enable/disable throwing of exceptions for rejected calls
            @param      "eReason"   returns reason for rejected calls if "eMode=E_NOEXCEPTIONS"!
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/
        inline TransactionGuard( ITransactionManager& rManager, EExceptionMode eMode, ERejectReason* eReason = NULL )
            : m_pManager( &rManager )
        {
            // If exception mode is set to E_HARDEXCETIONS we don't need a buffer to return reason!
            // We handle it private. If a call is rejected, our manager throw some exceptions ... and the reason
            // could be ignorable ...
            if( eReason == NULL )
            {
                ERejectReason eMyReason;
                m_pManager->registerTransaction( eMode, eMyReason );
            }
            else
            {
                m_pManager->registerTransaction( eMode, *eReason );
            }
        }

        /*-************************************************************************************************************//**
            @short      dtor
            @descr      We must release the transaction manager and can forget his pointer.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*************************************************************************************************************/
        inline ~TransactionGuard()
        {
            stop();
        }

        /*-************************************************************************************************************//**
            @short      stop current transaction
            @descr      We must release the transaction manager and can forget his pointer.

            @attention  We don't support any start() method here - because it is not easy to
                        detect if a transaction already started or not!
                        (combination of EExceptionMode and ERejectReason)

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*************************************************************************************************************/
        inline void stop()
        {
            if( m_pManager != NULL )
            {
                m_pManager->unregisterTransaction();
                m_pManager = NULL;
            }
        }

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------
    private:

        /*-****************************************************************************************************//**
            @short      disable using of these functions!
            @descr      It's not allowed to use this methods. Different problem can occure otherwise.
                        Thats why we disable it by make it private.

            @seealso    other ctor

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/
        TransactionGuard();

    //-------------------------------------------------------------------------------------------------------------
    //  private member
    //-------------------------------------------------------------------------------------------------------------
    private:

        ITransactionManager*   m_pManager   ;   /// pointer to safed transaction manager

};      //  class TransactionGuard

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_THREADHELP_TRANSACTIONGUARD_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
