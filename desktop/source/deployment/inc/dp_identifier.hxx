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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_IDENTIFIER_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_IDENTIFIER_HXX

#include "sal/config.h"

#include "boost/optional.hpp"
#include "com/sun/star/uno/Reference.hxx"

#include "dp_misc_api.hxx"

namespace com { namespace sun { namespace star { namespace deployment {
    class XPackage;
} } } }
namespace rtl { class OUString; }

namespace dp_misc {

/**
   Generates an identifier from an optional identifier.

   @param optional
   an optional identifier

   @param fileName
   a file name

   @return
   the given optional identifier if present, otherwise a legacy identifier based
   on the given file name
*/
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC ::rtl::OUString generateIdentifier(
    ::boost::optional< ::rtl::OUString > const & optional,
    ::rtl::OUString const & fileName);

/**
   Gets the identifier of a package.

   @param package
   a non-null package

   @return
   the explicit identifier of the given package if present, otherwise the
   implicit legacy identifier of the given package

   @throws com::sun::star::uno::RuntimeException
*/
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC ::rtl::OUString getIdentifier(
    ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage >
        const & package);

/**
   Generates a legacy identifier based on a file name.

   @param fileName
   a file name

   @return
   a legacy identifier based on the given file name
*/
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC ::rtl::OUString generateLegacyIdentifier(
    ::rtl::OUString const & fileName);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
