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
#ifndef _ENTRY_INPUT_STREAM_HXX
#define _ENTRY_INPUT_STREAM_HXX

#include <cppuhelper/implbase2.hxx> // helper for implementations
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <Inflater.hxx>
#include <com/sun/star/packages/zip/ZipEntry.hpp>
#include <rtl/ref.hxx>
#include <EncryptionData.hxx>
class EntryInputStream : public cppu::WeakImplHelper2< com::sun::star::io::XInputStream,
                                                       com::sun::star::io::XSeekable >
{
protected:
    com::sun::star::uno::Reference< com::sun::star::io::XInputStream > xStream;
    com::sun::star::uno::Reference< com::sun::star::io::XSeekable > xSeek;
    sal_Int64 nEnd, nCurrent, nUncompressedSize;
    sal_Bool bRawStream, bHaveInMemory, bEncrypted;
    com::sun::star::uno::Sequence < sal_Int8 > aBuffer;
    const rtl::Reference < EncryptionData > xEncryptionData;
    const com::sun::star::packages::zip::ZipEntry aEntry;
    Inflater aInflater;
    void readIntoMemory()
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
public:
             EntryInputStream( com::sun::star::uno::Reference < com::sun::star::io::XInputStream > xInput,
                                const com::sun::star::packages::zip::ZipEntry &rNewEntry,
                               const rtl::Reference < EncryptionData > &xEncryptData,
                               sal_Bool bGetRawStream = sal_False);
    virtual ~EntryInputStream();

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
    /*
private:
    void fill( void );
    */
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
