/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_version.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-10-04 16:54:37 $
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

#include "dp_version.hxx"

namespace {

namespace css = ::com::sun::star;

::sal_Int64 getElement(::rtl::OUString const & version, ::sal_Int32 * index) {
    ::rtl::OUString s(version.getToken(0, '.', *index));
    return *index < 0 ? 0 : s.toInt64(); // TODO: token too large
}

}

namespace dp_misc {

::dp_misc::Order comparePackageVersions(
    css::uno::Reference< css::deployment::XPackage > const & package1,
    css::uno::Reference< css::deployment::XPackage > const & package2)
{
    ::rtl::OUString s1(package1->getVersion());
    ::rtl::OUString s2(package2->getVersion());
    for (::sal_Int32 i1 = 0, i2 = 0; i1 >= 0 || i2 >= 0;) {
        ::sal_Int64 e1(getElement(s1, &i1));
        ::sal_Int64 e2(getElement(s2, &i2));
        if (e1 < e2) {
            return ::dp_misc::LESS;
        } else if (e1 > e2) {
            return ::dp_misc::GREATER;
        }
    }
    return ::dp_misc::EQUAL;
}

}
