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

#ifndef _PKGURI_HXX
#define _PKGURI_HXX

#include <rtl/ustring.hxx>

namespace package_ucp {

//=========================================================================

#define PACKAGE_URL_SCHEME          "vnd.sun.star.pkg"
#define PACKAGE_ZIP_URL_SCHEME      "vnd.sun.star.zip"
#define PACKAGE_URL_SCHEME_LENGTH   16

//=========================================================================

class PackageUri
{
    mutable ::rtl::OUString m_aUri;
    mutable ::rtl::OUString m_aParentUri;
    mutable ::rtl::OUString m_aPackage;
    mutable ::rtl::OUString m_aPath;
    mutable ::rtl::OUString m_aName;
    mutable ::rtl::OUString m_aParam;
    mutable ::rtl::OUString m_aScheme;
    mutable bool            m_bValid;

private:
    void init() const;

public:
    PackageUri() : m_bValid( false ) {}
    PackageUri( const ::rtl::OUString & rPackageUri )
    : m_aUri( rPackageUri ), m_bValid( false ) {}

    sal_Bool isValid() const
    { init(); return m_bValid; }

    const ::rtl::OUString & getUri() const
    { init(); return m_aUri; }

    void setUri( const ::rtl::OUString & rPackageUri )
    { m_aPath = ::rtl::OUString(); m_aUri = rPackageUri; m_bValid = false; }

    const ::rtl::OUString & getParentUri() const
    { init(); return m_aParentUri; }

    const ::rtl::OUString & getPackage() const
    { init(); return m_aPackage; }

    const ::rtl::OUString & getPath() const
    { init(); return m_aPath; }

    const ::rtl::OUString & getName() const
    { init(); return m_aName; }

    const ::rtl::OUString & getParam() const
    { init(); return m_aParam; }

    const ::rtl::OUString & getScheme() const
    { init(); return m_aScheme; }

    inline sal_Bool isRootFolder() const;
};

inline sal_Bool PackageUri::isRootFolder() const
{
    init();
    return ( ( m_aPath.getLength() == 1 ) &&
             ( m_aPath.getStr()[ 0 ] == sal_Unicode( '/' ) ) );
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
