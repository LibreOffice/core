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

#include <vector>

struct ZipEntry;
class ZipPackageStream;

class ZipOutputStream
{
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > m_xStream;
    ::std::vector < ZipEntry * > m_aZipList;

    ByteChucker         m_aChucker;
    bool                m_bFinished;
    ZipEntry            *m_pCurrentEntry;

public:
    ZipOutputStream(
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > &xOStream );
    ~ZipOutputStream();

    // rawWrite to support a direct write to the output stream
    void rawWrite( ::com::sun::star::uno::Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    void rawCloseEntry()
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    void putNextEntry( ZipEntry& rEntry, bool bEncrypt = false )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    void finish()
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    ByteChucker& getChucker();

    static sal_uInt32 getCurrentDosTime();

private:
    void writeEND(sal_uInt32 nOffset, sal_uInt32 nLength)
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    void writeCEN( const ZipEntry &rEntry )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    sal_Int32 writeLOC( const ZipEntry &rEntry )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    void writeEXT( const ZipEntry &rEntry )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
