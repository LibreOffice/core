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

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/connection/XConnection.hpp>
#include <com/sun/star/connection/XConnectionBroadcaster.hpp>

#include <unordered_set>
#include <osl/socket.hxx>
#include <osl/pipe.hxx>

namespace stoc_connector
{
    typedef std::unordered_set< css::uno::Reference< css::io::XStreamListener> >
            XStreamListener_hash_set;

    class PipeConnection :
        public ::cppu::WeakImplHelper< css::connection::XConnection >

    {
    public:
        explicit PipeConnection( const OUString &sConnectionDescription );
        virtual ~PipeConnection() override;

        virtual sal_Int32 SAL_CALL read( css::uno::Sequence< sal_Int8 >& aReadBytes,
                                         sal_Int32 nBytesToRead ) override;
        virtual void SAL_CALL write( const css::uno::Sequence< sal_Int8 >& aData ) override;
        virtual void SAL_CALL flush(  ) override;
        virtual void SAL_CALL close(  ) override;
        virtual OUString SAL_CALL getDescription(  ) override;
    public:
        ::osl::StreamPipe m_pipe;
        oslInterlockedCount m_nStatus;
        OUString m_sDescription;
    };

    class SocketConnection :
        public ::cppu::WeakImplHelper< css::connection::XConnection, css::connection::XConnectionBroadcaster >

    {
    public:
        explicit SocketConnection( const OUString & sConnectionDescription  );
        virtual ~SocketConnection() override;

        virtual sal_Int32 SAL_CALL read( css::uno::Sequence< sal_Int8 >& aReadBytes,
                                         sal_Int32 nBytesToRead ) override;
        virtual void SAL_CALL write( const css::uno::Sequence< sal_Int8 >& aData ) override;
        virtual void SAL_CALL flush(  ) override;
        virtual void SAL_CALL close(  ) override;
        virtual OUString SAL_CALL getDescription(  ) override;


        // XConnectionBroadcaster
        virtual void SAL_CALL addStreamListener(const css::uno::Reference< css::io::XStreamListener>& aListener) override;
        virtual void SAL_CALL removeStreamListener(const css::uno::Reference< css::io::XStreamListener>& aListener) override;

    public:
        void completeConnectionString();

        ::osl::ConnectorSocket m_socket;
        oslInterlockedCount m_nStatus;
        OUString m_sDescription;

        ::osl::Mutex _mutex;
        bool     _started;
        bool     _closed;
        bool     _error;

          XStreamListener_hash_set _listeners;
    };
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
