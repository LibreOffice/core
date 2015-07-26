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

#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XTempFile.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/crypto/XCipherContext.hpp>
#include <com/sun/star/xml/crypto/XDigestContext.hpp>

#include <package/Deflater.hxx>
#include <CRC32.hxx>

struct ZipEntry;
class ZipPackageBuffer;
class ZipPackageStream;

class ZipOutputEntry
{
    ::com::sun::star::uno::Sequence< sal_Int8 > m_aDeflateBuffer;
    ZipUtils::Deflater m_aDeflater;
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    OUString m_aTempURL;
    css::uno::Reference< css::io::XOutputStream > m_xOutStream;

    ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XCipherContext > m_xCipherContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XDigestContext > m_xDigestContext;
    ::css::uno::Any m_aParallelDeflateException;

    CRC32               m_aCRC;
    ZipEntry            *m_pCurrentEntry;
    sal_Int16           m_nDigested;
    bool                m_bEncryptCurrentEntry;
    ZipPackageStream*   m_pCurrentStream;

public:
    ZipOutputEntry(
        const css::uno::Reference< css::io::XOutputStream >& rxOutStream,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
        ZipEntry& rEntry, ZipPackageStream* pStream, bool bEncrypt = false);

    ~ZipOutputEntry();

    /* This block of methods is for threaded zipping, where we compress to a temp stream, whose
       data is retrieved via getData */
    ZipOutputEntry(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
        ZipEntry& rEntry, ZipPackageStream* pStream, bool bEncrypt = false);
    void createBufferFile();
    void setParallelDeflateException(const ::css::uno::Any &rAny) { m_aParallelDeflateException = rAny; }
    css::uno::Reference< css::io::XInputStream > getData() const;
    ::css::uno::Any getParallelDeflateException() const { return m_aParallelDeflateException; }
    void closeBufferFile();
    void deleteBufferFile();

    ZipEntry* getZipEntry() { return m_pCurrentEntry; }
    ZipPackageStream* getZipPackageStream() { return m_pCurrentStream; }
    bool isEncrypt() { return m_bEncryptCurrentEntry; }

    void closeEntry();
    void write(const css::uno::Sequence< sal_Int8 >& rBuffer);

private:
    void doDeflate();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
