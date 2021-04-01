/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <algorithm>
#include <unordered_set>
#include <vector>

namespace o3tl
{
// removes duplicated elements in a vector
template <typename T> void remove_duplicates(std::vector<T>& rVector)
{
    std::unordered_set<T> aSet;
    auto aEnd = std::copy_if(rVector.begin(), rVector.end(), rVector.begin(),
                             [&aSet](T const& rElement) { return aSet.insert(rElement).second; });
    rVector.erase(aEnd, rVector.end());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
