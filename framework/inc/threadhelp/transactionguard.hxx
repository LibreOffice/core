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

#ifndef INCLUDED_FRAMEWORK_INC_THREADHELP_TRANSACTIONGUARD_HXX
#define INCLUDED_FRAMEWORK_INC_THREADHELP_TRANSACTIONGUARD_HXX

#include <boost/noncopyable.hpp>
#include <threadhelp/transactionmanager.hxx>

namespace framework{

/*-************************************************************************************************************
    @short          implement a guard to support non breakable transactions
    @descr          If you wish to support non breakable method calls without locking any mutex, rw-lock or
                    something like that - you should use this guard implementation.
                    Initialize it at first in your method and don't release it till end of your function!
                    Your "transaction" is registered in ctor and automatically released in dtor.
                    Use set/get of working mode to enable/disable further transactions.
                    It's possible too, to enable automatically throwing of some exceptions for illegal
                    transaction requests ... e.g. interface call for already disposed objects.

    @attention      To prevent us against wrong using, the default ctor, copy ctor and the =operator are marked private!
    @devstatus      draft
*//*-*************************************************************************************************************/
class TransactionGuard : private boost::noncopyable
{

    //  public methods

    public:

        /*-****************************************************************************************************
            @short      ctors
            @descr      Use these ctor methods to initialize the guard right.
                        Given reference must be valid - otherwise crashes could occur!

            @attention  It's not necessary to lock any mutex here! Because a ctor should not be called
                        from different threads at the same time ... this class use no refcount mechanism!
            @param      "rManager"  reference to transaction manager for using to register a request
            @param      "eMode"     enable/disable throwing of exceptions for rejected calls
            @param      "eReason"   returns reason for rejected calls
        *//*-*****************************************************************************************************/
        inline TransactionGuard( TransactionManager& rManager, EExceptionMode eMode, ERejectReason* eReason = NULL )
            : m_pManager( &rManager )
        {
            // If exception mode is set to E_HARDEXCEPTIONS we don't need a buffer to return reason!
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

        /*-************************************************************************************************************
            @short      dtor
            @descr      We must release the transaction manager and can forget his pointer.
        *//*-*************************************************************************************************************/
        inline ~TransactionGuard()
        {
            stop();
        }

        /*-************************************************************************************************************
            @short      stop current transaction
            @descr      We must release the transaction manager and can forget his pointer.

            @attention  We don't support any start() method here - because it is not easy to
                        detect if a transaction already started or not!
                        (combination of EExceptionMode and ERejectReason)
        *//*-*************************************************************************************************************/
        inline void stop()
        {
            if( m_pManager != NULL )
            {
                m_pManager->unregisterTransaction();
                m_pManager = NULL;
            }
        }


    private:

        /*-****************************************************************************************************
            @short      disable using of these functions!
            @descr      It's not allowed to use this methods. Different problem can occur otherwise.
                        Thats why we disable it by make it private.

            @seealso    other ctor
        *//*-*****************************************************************************************************/
        TransactionGuard();

    //  private member

    private:

        TransactionManager*   m_pManager;   /// pointer to safed transaction manager

};      //  class TransactionGuard

}       //  namespace framework

#endif // INCLUDED_FRAMEWORK_INC_THREADHELP_TRANSACTIONGUARD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
