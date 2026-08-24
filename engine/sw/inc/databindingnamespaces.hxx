/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/xml/xpath/XXPathAPI.hpp>
#include <comphelper/string.hxx>
#include <sal/log.hxx>

#include <string_view>

namespace sw
{
/// Registers the prefixes an OOXML data binding declares, so that the xpath from the same data
/// binding can be evaluated. The mapping string holds declarations in the form xmlns:prefix='url',
/// separated by spaces. A declaration in any other form is skipped.
inline void RegisterDataBindingNamespaces(
    std::u16string_view aPrefixMappings,
    const css::uno::Reference<css::xml::xpath::XXPathAPI>& xXPathAPI)
{
    for (const auto& rDeclaration : comphelper::string::split(aPrefixMappings, ' '))
    {
        auto aParts = comphelper::string::split(rDeclaration, '=');
        if (aParts.size() < 2)
        {
            SAL_WARN("sw", "invalid data binding namespace: " << rDeclaration);
            continue;
        }

        auto aName = comphelper::string::split(aParts[0], ':');
        if (aName.size() < 2)
        {
            SAL_WARN("sw", "invalid data binding namespace: " << aParts[0]);
            continue;
        }

        OUString sURL = comphelper::string::strip(aParts[1], ' ');
        xXPathAPI->registerNS(aName[1], comphelper::string::strip(sURL, '\''));
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
