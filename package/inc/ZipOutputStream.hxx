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
#ifndef INCLUDED_PACKAGE_INC_ZIPOUTPUTSTREAM_HXX
#define INCLUDED_PACKAGE_INC_ZIPOUTPUTSTREAM_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/io/XOutputStream.hpp>

#include <ByteChucker.hxx>
#include <comphelper/threadpool.hxx>

#include <vector>

struct ZipEntry;
class ZipOutputEntry;
class ZipPackageStream;

class ZipOutputStream
{
    css::uno::Reference< css::io::XOutputStream > m_xStream;
    ::std::vector < ZipEntry * > m_aZipList;

    ByteChucker         m_aChucker;
    ZipEntry            *m_pCurrentEntry;
    comphelper::ThreadPool &m_rSharedThreadPool;
    std::vector< ZipOutputEntry* > m_aEntries;

public:
    ZipOutputStream(
        const css::uno::Reference< css::io::XOutputStream > &xOStream );
    ~ZipOutputStream();

    void addDeflatingThread( ZipOutputEntry *pEntry, comphelper::ThreadTask *pThreadTask );

    void writeLOC( ZipEntry *pEntry, bool bEncrypt = false )
        throw(css::io::IOException, css::uno::RuntimeException);
    void rawWrite( const css::uno::Sequence< sal_Int8 >& rBuffer )
        throw(css::io::IOException, css::uno::RuntimeException);
    void rawCloseEntry( bool bEncrypt = false )
        throw(css::io::IOException, css::uno::RuntimeException);

    void finish()
        throw(css::io::IOException, css::uno::RuntimeException);
    css::uno::Reference< css::io::XOutputStream > getStream();

    static sal_uInt32 getCurrentDosTime();
    static void setEntry( ZipEntry *pEntry );

private:
    void writeEND(sal_uInt32 nOffset, sal_uInt32 nLength)
        throw(css::io::IOException, css::uno::RuntimeException);
    void writeCEN( const ZipEntry &rEntry )
        throw(css::io::IOException, css::uno::RuntimeException);
    void writeEXT( const ZipEntry &rEntry )
        throw(css::io::IOException, css::uno::RuntimeException);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
