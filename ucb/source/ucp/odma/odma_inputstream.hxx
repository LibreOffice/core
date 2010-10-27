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

#ifndef ODMA_INPUTSTREAM_HXX
#define ODMA_INPUTSTREAM_HXX

#include <osl/mutex.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <cppuhelper/implbase5.hxx>

#include "rtl/ref.hxx"

namespace ucbhelper
{
    class Content;
}
namespace odma
{
    typedef ::cppu::WeakImplHelper5<    ::com::sun::star::io::XInputStream,
                                        ::com::sun::star::io::XStream,
                                        ::com::sun::star::io::XTruncate,
                                        ::com::sun::star::io::XSeekable,
                                        ::com::sun::star::io::XOutputStream> OOdmaStreamBase;

    class ContentProvider;
    class ContentProperties;
    class OOdmaStream : public OOdmaStreamBase
    {
        ::osl::Mutex    m_aMutex;
        ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream>  m_xOutput;
        ::com::sun::star::uno::Reference< ::com::sun::star::io::XTruncate>      m_xTruncate;
        ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream>   m_xInput;
        ::com::sun::star::uno::Reference< ::com::sun::star::io::XSeekable>      m_xInputSeek;

        ::rtl::Reference<ContentProperties> m_aProp;
        ::ucbhelper::Content*               m_pContent;
        ContentProvider*                    m_pProvider;
        sal_Bool                            m_bInputStreamCalled;
        sal_Bool                            m_bOutputStreamCalled;
        sal_Bool                            m_bModified;

        void ensureInputStream()  throw( ::com::sun::star::io::IOException );
        void ensureOutputStream()  throw( ::com::sun::star::io::IOException );
        void SAL_CALL closeStream() throw(  ::com::sun::star::io::NotConnectedException,
                                            ::com::sun::star::io::IOException,
                                            ::com::sun::star::uno::RuntimeException );
    public:
        OOdmaStream(::ucbhelper::Content* _pContent,
                    ContentProvider* _pProvider,
                    const ::rtl::Reference<ContentProperties>& _rProp);
        virtual ~OOdmaStream();
        // com::sun::star::io::XInputStream
        virtual sal_Int32 SAL_CALL readBytes( ::com::sun::star::uno::Sequence<sal_Int8>& aData, sal_Int32 nBytesToRead )
            throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException,
                  ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

        virtual sal_Int32 SAL_CALL readSomeBytes( ::com::sun::star::uno::Sequence<sal_Int8>& aData, sal_Int32 nMaxBytesToRead )
            throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException,
                  ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip )
            throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException,
                  ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

        virtual sal_Int32 SAL_CALL available(  )
            throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL closeInput(  )
            throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

        // com::sun::star::io::XStream
        virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream > SAL_CALL getInputStream(  ) throw( com::sun::star::uno::RuntimeException );
        virtual com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > SAL_CALL getOutputStream(  ) throw( com::sun::star::uno::RuntimeException );

        // com::sun::star::io::XOutputStream
        void SAL_CALL writeBytes( const com::sun::star::uno::Sequence< sal_Int8 >& aData )
                    throw( com::sun::star::io::NotConnectedException,
                           com::sun::star::io::BufferSizeExceededException,
                           com::sun::star::io::IOException,
                           com::sun::star::uno::RuntimeException);

        void SAL_CALL flush()
                    throw( com::sun::star::io::NotConnectedException,
                           com::sun::star::io::BufferSizeExceededException,
                           com::sun::star::io::IOException,
                           com::sun::star::uno::RuntimeException);
        void SAL_CALL closeOutput()
                    throw( com::sun::star::io::NotConnectedException,
                           com::sun::star::io::IOException,
                           com::sun::star::uno::RuntimeException );
        // XTruncate
        virtual void SAL_CALL truncate( void )
            throw( com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );
        // XSeekable
        void SAL_CALL seek(sal_Int64 location )
            throw( com::sun::star::lang::IllegalArgumentException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );

        sal_Int64 SAL_CALL getPosition()
            throw( com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );

        sal_Int64 SAL_CALL getLength()
            throw( com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );
    };
}
#endif // ODMA_INPUTSTREAM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
