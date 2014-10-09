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
#ifndef INCLUDED_PACKAGE_INC_ZIPOUTPUTENTRY_HXX
#define INCLUDED_PACKAGE_INC_ZIPOUTPUTENTRY_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/crypto/XCipherContext.hpp>
#include <com/sun/star/xml/crypto/XDigestContext.hpp>

#include <package/Deflater.hxx>
#include <ByteChucker.hxx>
#include <CRC32.hxx>

struct ZipEntry;
class ZipPackageStream;

class ZipOutputEntry
{
    ::com::sun::star::uno::Sequence< sal_Int8 > m_aDeflateBuffer;
    ZipUtils::Deflater  m_aDeflater;

    ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XCipherContext > m_xCipherContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XDigestContext > m_xDigestContext;

    CRC32               m_aCRC;
    ByteChucker         &m_rChucker;
    ZipEntry            *m_pCurrentEntry;
    sal_Int16           m_nDigested;
    bool                m_bEncryptCurrentEntry;
    ZipPackageStream*   m_pCurrentStream;

public:
    ZipOutputEntry(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
        ByteChucker& rChucker, ZipEntry& rEntry, ZipPackageStream* pStream, bool bEncrypt = false);

    ~ZipOutputEntry();

    // rawWrite to support a direct write to the output stream
    void SAL_CALL rawWrite( ::com::sun::star::uno::Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL rawCloseEntry(  )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    // XZipOutputEntry interfaces
    void SAL_CALL closeEntry(  )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL write( const ::com::sun::star::uno::Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    static sal_uInt32 getCurrentDosTime ( );

private:
    void doDeflate();
    sal_Int32 writeLOC( const ZipEntry &rEntry )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    void writeEXT( const ZipEntry &rEntry )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
