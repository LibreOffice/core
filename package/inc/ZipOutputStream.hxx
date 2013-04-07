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
#ifndef _ZIP_OUTPUT_STREAM_HXX
#define _ZIP_OUTPUT_STREAM_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/xml/crypto/XCipherContext.hpp>
#include <com/sun/star/xml/crypto/XDigestContext.hpp>

#include <package/Deflater.hxx>
#include <ByteChucker.hxx>
#include <CRC32.hxx>

#include <vector>

struct ZipEntry;
class ZipPackageStream;
namespace rtl
{
    template < class T > class Reference;
}

class ZipOutputStream
{
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext> m_xContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > xStream;

    ::std::vector < ZipEntry * >            aZipList;

    ::com::sun::star::uno::Sequence< sal_Int8 > m_aDeflateBuffer;

    OUString     sComment;
    ZipUtils::Deflater  aDeflater;

    ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XCipherContext > m_xCipherContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XDigestContext > m_xDigestContext;

    CRC32               aCRC;
    ByteChucker         aChucker;
    ZipEntry            *pCurrentEntry;
    sal_Int16           nMethod, nLevel, mnDigested;
    sal_Bool            bFinished, bEncryptCurrentEntry;
    ZipPackageStream*   m_pCurrentStream;

public:
    ZipOutputStream(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > &xOStream );
    ~ZipOutputStream();

    // rawWrite to support a direct write to the output stream
    void SAL_CALL rawWrite( ::com::sun::star::uno::Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL rawCloseEntry(  )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    // XZipOutputStream interfaces
    void SAL_CALL setMethod( sal_Int32 nNewMethod )
        throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setLevel( sal_Int32 nNewLevel )
        throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL putNextEntry( ZipEntry& rEntry,
            ZipPackageStream* pStream,
            sal_Bool bEncrypt = sal_False )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL closeEntry(  )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL write( const ::com::sun::star::uno::Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL finish(  )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    static sal_uInt32 getCurrentDosTime ( );
protected:
    void doDeflate();
    void writeEND(sal_uInt32 nOffset, sal_uInt32 nLength)
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    void writeCEN( const ZipEntry &rEntry )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    void writeEXT( const ZipEntry &rEntry )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    sal_Int32 writeLOC( const ZipEntry &rEntry )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
