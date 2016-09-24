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

#ifndef INCLUDED_FRAMEWORK_INC_THREADHELP_TRANSACTIONBASE_HXX
#define INCLUDED_FRAMEWORK_INC_THREADHELP_TRANSACTIONBASE_HXX

#include <threadhelp/transactionmanager.hxx>

namespace framework{

/*-************************************************************************************************************
    @short          make it possible to instanciate a transacion manager as first member!
    @descr          If you use a transaction manager as a member of your class and wish to use it earlier then other ones
                    you should have a look on this implementation. You must use it as the first base class
                    of your implementation - because base classes are initialized by his order and before your
                    member! Thats why ist a good place to declare this member.
    @devstatus      ready to use
*//*-*************************************************************************************************************/
struct TransactionBase
{

    //  public member

    public:

        mutable TransactionManager    m_aTransactionManager;   /// "your" public manager-member!
                                                                   /// Make it mutable for using in const functions!

};      //  struct TransactionBase

}       //  namespace framework

#endif // INCLUDED_FRAMEWORK_INC_THREADHELP_TRANSACTIONBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
