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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_IDENTIFIER_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_IDENTIFIER_HXX

#include <sal/config.h>

#include <boost/optional.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include "dp_misc_api.hxx"

namespace com { namespace sun { namespace star { namespace deployment {
    class XPackage;
} } } }

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
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC OUString generateIdentifier(
    ::boost::optional< OUString > const & optional,
    OUString const & fileName);

/**
   Gets the identifier of a package.

   @param package
   a non-null package

   @return
   the explicit identifier of the given package if present, otherwise the
   implicit legacy identifier of the given package

   @throws css::uno::RuntimeException
*/
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC OUString getIdentifier(
    css::uno::Reference< css::deployment::XPackage >
        const & package);

/**
   Generates a legacy identifier based on a file name.

   @param fileName
   a file name

   @return
   a legacy identifier based on the given file name
*/
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC OUString generateLegacyIdentifier(
    OUString const & fileName);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
