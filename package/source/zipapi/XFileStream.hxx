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
#ifndef _XFILE_STREAM_HXX
#define _XFILE_STREAM_HXX

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <cppuhelper/implbase2.hxx>
#include <rtl/ref.hxx>
#include <Inflater.hxx>
#include <ZipEntry.hxx>

namespace com { namespace sun { namespace star {
    namespace io { class XOutputStream; }
} } }
class EncryptionData;
typedef void* rtlCipher;
class XFileStream : public cppu::WeakImplHelper2
<
    com::sun::star::io::XInputStream,
    com::sun::star::io::XSeekable
>
{
protected:
    com::sun::star::uno::Reference < com::sun::star::io::XInputStream > mxZipStream;
    com::sun::star::uno::Reference < com::sun::star::io::XSeekable > mxZipSeek;
    com::sun::star::uno::Reference < com::sun::star::io::XInputStream > mxTempIn;
    com::sun::star::uno::Reference < com::sun::star::io::XSeekable > mxTempSeek;
    com::sun::star::uno::Reference < com::sun::star::io::XOutputStream > mxTempOut;
    com::sun::star::uno::Sequence < sal_Int8 > maBuffer, maCompBuffer;
    ZipEntry maEntry;
    rtl::Reference < EncryptionData > mxData;
    rtlCipher maCipher;
    Inflater maInflater;
    sal_Bool mbRawStream, mbFinished;
    sal_Int64 mnZipCurrent, mnZipEnd, mnZipSize;
    void fill( sal_Int64 nUntil );

public:
    XFileStream( ZipEntry & rEntry,
                 com::sun::star::uno::Reference < com::sun::star::io::XInputStream > xNewZipStream,
                 com::sun::star::uno::Reference < com::sun::star::io::XInputStream > xNewTempStream,
                 const rtl::Reference < EncryptionData > &rData,
                 sal_Bool bRawStream,
                 sal_Bool bIsEncrypted );
    virtual ~XFileStream();

    // XInputStream
    virtual sal_Int32 SAL_CALL readBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL readSomeBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL available(  )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL closeInput(  )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    // XSeekable
    virtual void SAL_CALL seek( sal_Int64 location )
        throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getPosition(  )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getLength(  )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
};
#endif
