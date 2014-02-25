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

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>

#include <com/sun/star/connection/XConnection.hpp>
#include <com/sun/star/connection/XConnectionBroadcaster.hpp>

#include <boost/unordered_set.hpp>
#       include <osl/socket.hxx>
#       include <osl/pipe.hxx>

namespace stoc_connector
{
    template<class T>
    struct ReferenceHash
    {
        size_t operator () (const ::com::sun::star::uno::Reference<T> & ref) const
        {
            return (size_t)ref.get();
        }
    };

    template<class T>
    struct ReferenceEqual
    {
        sal_Bool operator () (const ::com::sun::star::uno::Reference<T> & op1,
                              const ::com::sun::star::uno::Reference<T> & op2) const
        {
            return op1.get() == op2.get();
        }
    };

    typedef ::boost::unordered_set< ::com::sun::star::uno::Reference< ::com::sun::star::io::XStreamListener>,
                             ReferenceHash< ::com::sun::star::io::XStreamListener>,
                             ReferenceEqual< ::com::sun::star::io::XStreamListener> >
            XStreamListener_hash_set;

    class PipeConnection :
        public ::cppu::WeakImplHelper1< ::com::sun::star::connection::XConnection >

    {
    public:
        PipeConnection( const OUString &sConnectionDescription );
        virtual ~PipeConnection();

        virtual sal_Int32 SAL_CALL read( ::com::sun::star::uno::Sequence< sal_Int8 >& aReadBytes,
                                         sal_Int32 nBytesToRead )
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL write( const ::com::sun::star::uno::Sequence< sal_Int8 >& aData )
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL flush(  ) throw(
            ::com::sun::star::io::IOException,
            ::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL close(  )
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException, std::exception);
        virtual OUString SAL_CALL getDescription(  )
            throw(::com::sun::star::uno::RuntimeException, std::exception);
    public:
        ::osl::StreamPipe m_pipe;
        oslInterlockedCount m_nStatus;
        OUString m_sDescription;
    };

    class SocketConnection :
        public ::cppu::WeakImplHelper2< ::com::sun::star::connection::XConnection, ::com::sun::star::connection::XConnectionBroadcaster >

    {
    public:
        SocketConnection( const OUString & sConnectionDescription  );
        virtual ~SocketConnection();

        virtual sal_Int32 SAL_CALL read( ::com::sun::star::uno::Sequence< sal_Int8 >& aReadBytes,
                                         sal_Int32 nBytesToRead )
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL write( const ::com::sun::star::uno::Sequence< sal_Int8 >& aData )
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL flush(  ) throw(
            ::com::sun::star::io::IOException,
            ::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL close(  )
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException, std::exception);
        virtual OUString SAL_CALL getDescription(  )
            throw(::com::sun::star::uno::RuntimeException, std::exception);


        // XConnectionBroadcaster
        virtual void SAL_CALL addStreamListener(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStreamListener>& aListener)
            throw(::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL removeStreamListener(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStreamListener>& aListener)
            throw(::com::sun::star::uno::RuntimeException, std::exception);

    public:
        void completeConnectionString();

        ::osl::ConnectorSocket m_socket;
        ::osl::SocketAddr m_addr;
        oslInterlockedCount m_nStatus;
        OUString m_sDescription;

        ::osl::Mutex _mutex;
        sal_Bool     _started;
        sal_Bool     _closed;
        sal_Bool     _error;

          XStreamListener_hash_set _listeners;
    };
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
