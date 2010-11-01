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
#include <list>

#include <osl/conditn.h>
#include <osl/mutex.hxx>

#include <rtl/ustring.hxx>

#include <osl/thread.hxx>

#include <com/sun/star/uno/Type.hxx>

struct remote_Connection;

namespace bridges_urp
{
    struct RemoteReleaseCall
    {
        ::rtl::OUString sOid;
        ::com::sun::star::uno::Type typeInterface;
    };

    struct urp_BridgeImpl;
    class OWriterThread :
        public ::osl::Thread
    {
    public:
        OWriterThread( remote_Connection *pConnection,
                       urp_BridgeImpl *m_pBridgeImpl,
                       uno_Environment *pEnvRemote);
        ~OWriterThread(  );

        virtual void SAL_CALL run();

        void touch( sal_Bool bImmediately );
        void sendEmptyMessage();

        void abortThread();

        void SAL_CALL insertReleaseRemoteCall (
            rtl_uString *pOid,typelib_TypeDescriptionReference *pTypeRef);
        void SAL_CALL executeReleaseRemoteCalls();

    private:
        void write();
        oslCondition m_oslCondition;
        sal_Bool m_bAbort;
        sal_Bool m_bInBlockingWait;
        sal_Bool m_bEnterBlockingWait;
        remote_Connection *m_pConnection;
        urp_BridgeImpl *m_pBridgeImpl;
        uno_Environment *m_pEnvRemote; // this is held weak only

        ::osl::Mutex m_releaseCallMutex;
        ::std::list< struct RemoteReleaseCall > m_lstReleaseCalls;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
