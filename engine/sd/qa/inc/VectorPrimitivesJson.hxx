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

#include <optional>
#include <string_view>
#include <vector>

#include <boost/property_tree/ptree.hpp>
#include <rtl/string.hxx>
#include <rtl/stringconcat.hxx>
#include <tools/JsonPath.hxx>

/// The entries of the given kind in the objects array of a vector primitives response. An entry
/// is named by its kind, while a primitive inside one is named by its type.
inline std::vector<tools::JsonPath> entriesOfKind(const tools::JsonPath& rJson,
                                                  std::string_view sKind)
{
    std::vector<tools::JsonPath> aEntries;
    const OString aWanted(sKind);
    const size_t nCount = rJson.getSize("/objects").value_or(0);
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex)
    {
        const auto oEntry
            = rJson.at(rtl::Concat2View("/objects/" + OString::number(sal_Int32(nIndex))));
        if (oEntry && oEntry->getString("kind").value_or(OString()) == aWanted)
            aEntries.push_back(*oEntry);
    }
    return aEntries;
}

/// The first entry of the given kind, or nothing when the response carries none.
inline std::optional<tools::JsonPath> findEntryOfKind(const tools::JsonPath& rJson,
                                                      std::string_view sKind)
{
    const std::vector<tools::JsonPath> aEntries = entriesOfKind(rJson, sKind);
    if (aEntries.empty())
        return std::nullopt;
    return aEntries.front();
}

/// The first text portion anywhere under the node whose text contains rText, or nothing.
inline std::optional<tools::JsonPath> findTextPortionUnder(const tools::JsonPath& rNode,
                                                           const OString& rText)
{
    const OString sType = rNode.getString("type").value_or(OString());
    if ((sType == "textSimplePortion" || sType == "textDecoratedPortion")
        && rNode.getString("text").value_or(OString()).indexOf(rText) >= 0)
        return rNode;
    for (const auto& rChild : rNode.tree())
    {
        auto oFound = findTextPortionUnder(rNode.sub(rChild.second), rText);
        if (oFound)
            return oFound;
    }
    return std::nullopt;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
