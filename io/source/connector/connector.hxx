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

#ifndef INCLUDED_IO_SOURCE_CONNECTOR_CONNECTOR_HXX
#define INCLUDED_IO_SOURCE_CONNECTOR_CONNECTOR_HXX

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/connection/XConnection.hpp>
#include <com/sun/star/connection/XConnectionBroadcaster.hpp>

#include <unordered_set>
#include <osl/socket.hxx>
#include <osl/pipe.hxx>

namespace stoc_connector
{
    template<class T>
    struct ReferenceHash
    {
        size_t operator () (const css::uno::Reference<T> & ref) const
        {
            return reinterpret_cast<size_t>(ref.get());
        }
    };

    template<class T>
    struct ReferenceEqual
    {
        bool operator () (const css::uno::Reference<T> & op1,
                          const css::uno::Reference<T> & op2) const
        {
            return op1.get() == op2.get();
        }
    };

    typedef std::unordered_set< css::uno::Reference< css::io::XStreamListener>,
                                ReferenceHash< css::io::XStreamListener>,
                                ReferenceEqual< css::io::XStreamListener> >
            XStreamListener_hash_set;

    class PipeConnection :
        public ::cppu::WeakImplHelper< css::connection::XConnection >

    {
    public:
        explicit PipeConnection( const OUString &sConnectionDescription );
        virtual ~PipeConnection();

        virtual sal_Int32 SAL_CALL read( css::uno::Sequence< sal_Int8 >& aReadBytes,
                                         sal_Int32 nBytesToRead )
            throw(css::io::IOException,
                  css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL write( const css::uno::Sequence< sal_Int8 >& aData )
            throw(css::io::IOException,
                  css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL flush(  ) throw(
            css::io::IOException,
            css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL close(  )
            throw(css::io::IOException,
                  css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getDescription(  )
            throw(css::uno::RuntimeException, std::exception) override;
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
        virtual ~SocketConnection();

        virtual sal_Int32 SAL_CALL read( css::uno::Sequence< sal_Int8 >& aReadBytes,
                                         sal_Int32 nBytesToRead )
            throw(css::io::IOException,
                  css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL write( const css::uno::Sequence< sal_Int8 >& aData )
            throw(css::io::IOException,
                  css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL flush(  ) throw(
            css::io::IOException,
            css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL close(  )
            throw(css::io::IOException,
                  css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getDescription(  )
            throw(css::uno::RuntimeException, std::exception) override;


        // XConnectionBroadcaster
        virtual void SAL_CALL addStreamListener(const css::uno::Reference< css::io::XStreamListener>& aListener)
            throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeStreamListener(const css::uno::Reference< css::io::XStreamListener>& aListener)
            throw(css::uno::RuntimeException, std::exception) override;

    public:
        void completeConnectionString();

        ::osl::ConnectorSocket m_socket;
        ::osl::SocketAddr m_addr;
        oslInterlockedCount m_nStatus;
        OUString m_sDescription;

        ::osl::Mutex _mutex;
        bool     _started;
        bool     _closed;
        bool     _error;

          XStreamListener_hash_set _listeners;
    };
}

#endif // INCLUDED_IO_SOURCE_CONNECTOR_CONNECTOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
