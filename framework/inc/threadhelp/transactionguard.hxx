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

#include <threadhelp/transactionmanager.hxx>

namespace framework{

class TransactionGuard
{
    public:
        inline TransactionGuard( TransactionManager& rManager, EExceptionMode eMode )
            : m_pManager( &rManager )
        {
            m_pManager->registerTransaction( eMode );
        }

        inline ~TransactionGuard()
        {
            m_pManager->unregisterTransaction();
        }
        TransactionGuard(const TransactionGuard&) = delete;
        TransactionGuard& operator=(const TransactionGuard&) = delete;

    private:
        TransactionManager*   m_pManager;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
