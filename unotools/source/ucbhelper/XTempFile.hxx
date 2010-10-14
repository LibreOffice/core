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
#ifndef _XTEMPFILE_HXX_
#define _XTEMPFILE_HXX_

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XTempFile.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase5.hxx>
#include <cppuhelper/propertysetmixin.hxx>
#include <osl/mutex.hxx>

class SvStream;
namespace utl { class TempFile; }

typedef  ::cppu::WeakImplHelper5<   ::com::sun::star::io::XTempFile
                                    ,   ::com::sun::star::io::XInputStream
                                                  , ::com::sun::star::io::XOutputStream
                                                  , ::com::sun::star::io::XTruncate
                                                  , ::com::sun::star::lang::XServiceInfo
                                                  >
                                    OTempFileBase;

class OTempFileService :
    public OTempFileBase,
    public ::cppu::PropertySetMixin< ::com::sun::star::io::XTempFile >
{
protected:
    ::utl::TempFile*    mpTempFile;
    ::osl::Mutex        maMutex;
    SvStream*           mpStream;
    sal_Bool            mbRemoveFile;
    sal_Bool            mbInClosed;
    sal_Bool            mbOutClosed;

    sal_Int64           mnCachedPos;
    sal_Bool            mbHasCachedPos;

    void checkError () const;
    void checkConnected ();

public:
    OTempFileService (::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & context);

    //Methods
    //  XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  )
        throw ();
    virtual void SAL_CALL release(  )
        throw ();
    //  XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL getImplementationId(  )
        throw (::com::sun::star::uno::RuntimeException);

    //  XTempFile
    virtual ::sal_Bool SAL_CALL getRemoveFile()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setRemoveFile( ::sal_Bool _removefile )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getUri()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getResourceName()
        throw (::com::sun::star::uno::RuntimeException);

    // XInputStream
    virtual ::sal_Int32 SAL_CALL readBytes( ::com::sun::star::uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nBytesToRead )
        throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL readSomeBytes( ::com::sun::star::uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nMaxBytesToRead )
        throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL skipBytes( ::sal_Int32 nBytesToSkip )
        throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL available(  )
        throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL closeInput(  )
        throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    // XOutputStream
    virtual void SAL_CALL writeBytes( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& aData )
        throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL flush(  )
        throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL closeOutput(  )
        throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    // XSeekable
    virtual void SAL_CALL seek( sal_Int64 location )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getPosition(  )
        throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getLength(  )
        throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    // XStream
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getInputStream(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > SAL_CALL getOutputStream(  )
        throw (::com::sun::star::uno::RuntimeException);
    // XTruncate
    virtual void SAL_CALL truncate()
        throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException);

    //::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface > SAL_CALL XTempFile_createInstance( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & context);
    static ::rtl::OUString getImplementationName_Static ();
    static ::com::sun::star::uno::Sequence < ::rtl::OUString > getSupportedServiceNames_Static();

    static ::com::sun::star::uno::Reference < com::sun::star::lang::XSingleComponentFactory > createServiceFactory_Static( com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > const & rServiceFactory );

private:
    OTempFileService( OTempFileService & );
    virtual ~OTempFileService ();

};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
