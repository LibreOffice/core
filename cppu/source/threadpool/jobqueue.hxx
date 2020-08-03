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

#pragma once

#include <sal/config.h>

#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>

#include <sal/types.h>

namespace cppu_threadpool
{
    extern "C" typedef void (RequestFun)(void *);

    struct Job
    {
        void *pThreadSpecificData;
        RequestFun * doRequest;
    };

    class DisposedCallerAdmin;
    typedef std::shared_ptr<DisposedCallerAdmin> DisposedCallerAdminHolder;

    class JobQueue
    {
    public:
        JobQueue();

        void add( void *pThreadSpecificData, RequestFun * doRequest );

        void *enter( void const * nDisposeId , bool bReturnWhenNoJob = false );
        void dispose( void const * nDisposeId );

        void suspend();
        void resume();

        bool isEmpty() const;
        bool isCallstackEmpty() const;
        bool isBusy() const;

    private:
        mutable std::mutex m_mutex;
        std::deque < struct Job > m_lstJob;
        std::deque<void const *>  m_lstCallstack;
        sal_Int32 m_nToDo;
        bool m_bSuspended;
        std::condition_variable m_cndWait;
        DisposedCallerAdminHolder m_DisposedCallerAdmin;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
