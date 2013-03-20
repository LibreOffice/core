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
#if 1

#include <list>
#include <sal/types.h>

#include <osl/conditn.h>
#include <osl/mutex.hxx>

#include <boost/shared_ptr.hpp>

namespace cppu_threadpool
{
    extern "C" typedef void (SAL_CALL RequestFun)(void *);

    struct Job
    {
        void *pThreadSpecificData;
        RequestFun * doRequest;
    };

    typedef ::std::list < struct Job > JobList;

    typedef ::std::list < sal_Int64 > CallStackList;

    class DisposedCallerAdmin;
    typedef boost::shared_ptr<DisposedCallerAdmin> DisposedCallerAdminHolder;

    class JobQueue
    {
    public:
        JobQueue();
        ~JobQueue();

        void add( void *pThreadSpecificData, RequestFun * doRequest );

        void *enter( sal_Int64 nDisposeId , sal_Bool bReturnWhenNoJob = sal_False );
        void dispose( sal_Int64 nDisposeId );

        void suspend();
        void resume();

        sal_Bool isEmpty() const;
        sal_Bool isCallstackEmpty() const;
        sal_Bool isBusy() const;

    private:
        mutable ::osl::Mutex m_mutex;
        JobList      m_lstJob;
        CallStackList m_lstCallstack;
        sal_Int32 m_nToDo;
        sal_Bool m_bSuspended;
        oslCondition m_cndWait;
        DisposedCallerAdminHolder m_DisposedCallerAdmin;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
