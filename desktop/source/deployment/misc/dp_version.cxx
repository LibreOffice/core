/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_version.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2006-11-06 14:54:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2006 by Sun Microsystems, Inc.
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "sal/config.h"

#include "com/sun/star/deployment/XPackage.hpp"
#include "rtl/ustring.hxx"

#include "dp_version.hxx"

namespace {

namespace css = ::com::sun::star;

::sal_Int64 getElement(::rtl::OUString const & version, ::sal_Int32 * index) {
    // If getToken returns an empty string (because *index is outside the range
    // of version) toInt64 will conveniently return 0:
    return version.getToken(0, '.', *index).toInt64(); // TODO: value too large
}

}

namespace dp_misc {

::dp_misc::Order compareVersions(
    ::rtl::OUString const & version1, ::rtl::OUString const & version2)
{
    for (::sal_Int32 i1 = 0, i2 = 0; i1 >= 0 || i2 >= 0;) {
        ::sal_Int64 e1(getElement(version1, &i1));
        ::sal_Int64 e2(getElement(version2, &i2));
        if (e1 < e2) {
            return ::dp_misc::LESS;
        } else if (e1 > e2) {
            return ::dp_misc::GREATER;
        }
    }
    return ::dp_misc::EQUAL;
}

::dp_misc::Order comparePackageVersions(
    css::uno::Reference< css::deployment::XPackage > const & package1,
    css::uno::Reference< css::deployment::XPackage > const & package2)
{
    return compareVersions(package1->getVersion(), package2->getVersion());
}

}
