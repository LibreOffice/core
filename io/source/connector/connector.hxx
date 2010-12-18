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
#include <rtl/unload.h>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>

#include <com/sun/star/connection/XConnection.hpp>
#include <com/sun/star/connection/XConnectionBroadcaster.hpp>

#include <hash_set>
#       include <osl/socket.hxx>
#       include <osl/pipe.hxx>

namespace stoc_connector
{
    extern rtl_StandardModuleCount g_moduleCount;

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

    typedef ::std::hash_set< ::com::sun::star::uno::Reference< ::com::sun::star::io::XStreamListener>,
                             ReferenceHash< ::com::sun::star::io::XStreamListener>,
                             ReferenceEqual< ::com::sun::star::io::XStreamListener> >
            XStreamListener_hash_set;

    class PipeConnection :
        public ::cppu::WeakImplHelper1< ::com::sun::star::connection::XConnection >

    {
    public:
        PipeConnection( const ::rtl::OUString &sConnectionDescription );
        virtual ~PipeConnection();

        virtual sal_Int32 SAL_CALL read( ::com::sun::star::uno::Sequence< sal_Int8 >& aReadBytes,
                                         sal_Int32 nBytesToRead )
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL write( const ::com::sun::star::uno::Sequence< sal_Int8 >& aData )
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL flush(  ) throw(
            ::com::sun::star::io::IOException,
            ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL close(  )
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getDescription(  )
            throw(::com::sun::star::uno::RuntimeException);
    public:
        ::osl::StreamPipe m_pipe;
        oslInterlockedCount m_nStatus;
        ::rtl::OUString m_sDescription;
    };

    class SocketConnection :
        public ::cppu::WeakImplHelper2< ::com::sun::star::connection::XConnection, ::com::sun::star::connection::XConnectionBroadcaster >

    {
    public:
        SocketConnection( const ::rtl::OUString & sConnectionDescription  );
        virtual ~SocketConnection();

        virtual sal_Int32 SAL_CALL read( ::com::sun::star::uno::Sequence< sal_Int8 >& aReadBytes,
                                         sal_Int32 nBytesToRead )
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL write( const ::com::sun::star::uno::Sequence< sal_Int8 >& aData )
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL flush(  ) throw(
            ::com::sun::star::io::IOException,
            ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL close(  )
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getDescription(  )
            throw(::com::sun::star::uno::RuntimeException);


        // XConnectionBroadcaster
        virtual void SAL_CALL addStreamListener(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStreamListener>& aListener)
            throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeStreamListener(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStreamListener>& aListener)
            throw(::com::sun::star::uno::RuntimeException);

    public:
        void completeConnectionString();

        ::osl::ConnectorSocket m_socket;
        ::osl::SocketAddr m_addr;
        oslInterlockedCount m_nStatus;
        ::rtl::OUString m_sDescription;

        ::osl::Mutex _mutex;
        sal_Bool     _started;
        sal_Bool     _closed;
        sal_Bool     _error;

          XStreamListener_hash_set _listeners;
    };
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
