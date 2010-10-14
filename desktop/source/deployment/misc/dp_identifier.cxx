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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
