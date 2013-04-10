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

#include <osl/pipe.hxx>
#include <osl/socket.hxx>

#include <com/sun/star/connection/XConnection.hpp>

namespace io_acceptor {

    class PipeAcceptor
    {
    public:
        PipeAcceptor( const OUString &sPipeName , const OUString &sConnectionDescription );

        void init();
        ::com::sun::star::uno::Reference < ::com::sun::star::connection::XConnection >  accept(  );

        void stopAccepting();

        ::osl::Mutex m_mutex;
        ::osl::Pipe m_pipe;
        OUString m_sPipeName;
        OUString m_sConnectionDescription;
        sal_Bool m_bClosed;
    };

    class SocketAcceptor
    {
    public:
        SocketAcceptor( const OUString & sSocketName ,
                        sal_uInt16 nPort,
                        sal_Bool bTcpNoDelay,
                        const OUString &sConnectionDescription );

        void init();
        ::com::sun::star::uno::Reference < ::com::sun::star::connection::XConnection > accept();

        void stopAccepting();

        ::osl::SocketAddr m_addr;
        ::osl::AcceptorSocket m_socket;
        OUString m_sSocketName;
        OUString m_sConnectionDescription;
        sal_uInt16 m_nPort;
        sal_Bool m_bTcpNoDelay;
        sal_Bool m_bClosed;
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
