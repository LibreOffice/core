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
        *//*-*****************************************************************************************************/
        inline TransactionGuard( TransactionManager& rManager, EExceptionMode eMode )
            : m_pManager( &rManager )
        {
            m_pManager->registerTransaction( eMode );
        }

        /*-************************************************************************************************************
            @short      dtor
            @descr      We must release the transaction manager and can forget his pointer.
        *//*-*************************************************************************************************************/
        inline ~TransactionGuard()
        {
            m_pManager->unregisterTransaction();
        }

    private:

        TransactionManager*   m_pManager;   /// pointer to safed transaction manager

};      //  class TransactionGuard

}       //  namespace framework

#endif // INCLUDED_FRAMEWORK_INC_THREADHELP_TRANSACTIONGUARD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
