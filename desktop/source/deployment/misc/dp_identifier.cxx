/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_identifier.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-01-19 09:15:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
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

#include "boost/optional.hpp"
#include "com/sun/star/beans/Optional.hpp"
#include "com/sun/star/deployment/XPackage.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "osl/diagnose.h"
#include "rtl/string.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.hxx"

#include "dp_identifier.hxx"

namespace {
    namespace css = ::com::sun::star;
}

namespace dp_misc {

::rtl::OUString generateIdentifier(
    ::boost::optional< ::rtl::OUString > const & optional,
    ::rtl::OUString const & fileName)
{
    return optional ? *optional : generateLegacyIdentifier(fileName);
}

::rtl::OUString getIdentifier(
    css::uno::Reference< css::deployment::XPackage > const & package)
{
    OSL_ASSERT(package.is());
    css::beans::Optional< ::rtl::OUString > id(package->getIdentifier());
    return id.IsPresent
        ? id.Value : generateLegacyIdentifier(package->getName());
}

::rtl::OUString generateLegacyIdentifier(::rtl::OUString const & fileName) {
    rtl::OUStringBuffer b;
    b.appendAscii(RTL_CONSTASCII_STRINGPARAM("org.openoffice.legacy."));
    b.append(fileName);
    return b.makeStringAndClear();
}

}
