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


#include <sal/config.h>

#include <optional>
#include <com/sun/star/beans/Optional.hpp>
#include <com/sun/star/deployment/XPackage.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <osl/diagnose.h>
#include <rtl/ustring.hxx>

#include <dp_identifier.hxx>

namespace dp_misc {

OUString generateIdentifier(
    ::std::optional< OUString > const & optional,
    std::u16string_view fileName)
{
    return optional ? *optional : generateLegacyIdentifier(fileName);
}

OUString getIdentifier(
    css::uno::Reference< css::deployment::XPackage > const & package)
{
    OSL_ASSERT(package.is());
    css::beans::Optional< OUString > id(package->getIdentifier());
    return id.IsPresent
        ? id.Value : generateLegacyIdentifier(package->getName());
}

OUString generateLegacyIdentifier(std::u16string_view fileName) {
    return OUString::Concat("org.openoffice.legacy.") + fileName;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
