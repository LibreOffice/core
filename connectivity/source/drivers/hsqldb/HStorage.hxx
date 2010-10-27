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
#ifndef CONNECTIVITY_HSQLDB_STORAGE_HXX
#define CONNECTIVITY_HSQLDB_STORAGE_HXX

#include <cppuhelper/compbase6.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/document/XDocumentSubStorageSupplier.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include "connectivity/CommonTools.hxx"
#include <comphelper/broadcasthelper.hxx>


#define DECLARE_SERVICE_INFO_STATIC()   \
    DECLARE_SERVICE_INFO(); \
    static ::rtl::OUString SAL_CALL getImplementationName_Static(  ) throw (::com::sun::star::uno::RuntimeException);   \
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_Static(  ) throw(::com::sun::star::uno::RuntimeException);  \
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >    \
        SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&)    \


namespace connectivity
{
    namespace hsqldb
    {
        typedef ::cppu::WeakComponentImplHelper6<   ::com::sun::star::io::XStream
                                        ,   ::com::sun::star::io::XInputStream
                                        ,   ::com::sun::star::io::XOutputStream
                                        ,   ::com::sun::star::io::XSeekable
                                        ,   ::com::sun::star::lang::XInitialization
                                        ,   ::com::sun::star::lang::XServiceInfo>   OStorage_Base;
        class OStorage : public ::comphelper::OBaseMutex
                        ,public OStorage_Base
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >            m_xServiceFactory;
            ::com::sun::star::uno::Reference< ::com::sun::star::document::XDocumentSubStorageSupplier>  m_xDS;


            ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage>    m_xStorge;
            ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >       m_xStream;
            ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >  m_xIn;
            ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > m_xOut;
            ::com::sun::star::uno::Reference< ::com::sun::star::io::XSeekable >     m_xSeek;

            OStorage();                         // never implemented
            OStorage(const OStorage&);          // never implemented
            int operator= (const OStorage&);    // never implemented

            OStorage(
                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory);
        protected:
            /** this function is called upon disposing the component
            */
            virtual void SAL_CALL disposing();
        public:
            // ::com::sun::star::lang::XServiceInfo
            DECLARE_SERVICE_INFO_STATIC();

            // XInitialization
            virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

            // XStream
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getInputStream(  ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > SAL_CALL getOutputStream(  ) throw (::com::sun::star::uno::RuntimeException);

            // XInputStream
            virtual ::sal_Int32 SAL_CALL readBytes( ::com::sun::star::uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nBytesToRead ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
            virtual ::sal_Int32 SAL_CALL readSomeBytes( ::com::sun::star::uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nMaxBytesToRead ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL skipBytes( ::sal_Int32 nBytesToSkip ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
            virtual ::sal_Int32 SAL_CALL available(  ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL closeInput(  ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

            // XOutputStream
            virtual void SAL_CALL writeBytes( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& aData ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL flush(  ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL closeOutput(  ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

            // XSeekable
            virtual void SAL_CALL seek( ::sal_Int64 location ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
            virtual ::sal_Int64 SAL_CALL getPosition(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
            virtual ::sal_Int64 SAL_CALL getLength(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
        };
    }
// .......................................................................
} // connectivity
// .......................................................................
#endif // CONNECTIVITY_HSQLDB_STORAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
