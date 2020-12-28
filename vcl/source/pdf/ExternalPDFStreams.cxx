/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <pdf/ExternalPDFStreams.hxx>
#include <comphelper/hash.hxx>

namespace vcl
{
sal_Int32 ExternalPDFStreams::store(BinaryDataContainer const& rDataContainer)
{
    sal_Int32 nIndex = -1;

    std::vector<sal_uInt8> aHash = comphelper::Hash::calculateHash(
        rDataContainer.getData(), rDataContainer.getSize(), comphelper::HashType::SHA1);

    auto it = maStreamIndexMap.find(aHash);
    if (it == maStreamIndexMap.end())
    {
        auto& rExternalStream = maStreamList.emplace_back();
        rExternalStream.maDataContainer = rDataContainer;
        nIndex = maStreamList.size() - 1;
        maStreamIndexMap.emplace(aHash, nIndex);
    }
    else
    {
        nIndex = it->second;
    }

    return nIndex;
}

ExternalPDFStream& ExternalPDFStreams::get(sal_uInt32 nIndex) { return maStreamList.at(nIndex); }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
