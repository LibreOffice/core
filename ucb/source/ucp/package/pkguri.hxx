/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pkguri.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:58:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _PKGURI_HXX
#define _PKGURI_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

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

    static ::rtl::OUString decodeSegment( const ::rtl::OUString& rSource );
    static ::rtl::OUString encodeSegment( const ::rtl::OUString& rSource );
    static ::rtl::OUString encodeSegmentsForSure( const rtl::OUString& rSource );

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
