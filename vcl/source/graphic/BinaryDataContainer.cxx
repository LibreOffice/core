/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/BinaryDataContainer.hxx>
#include <boost/functional/hash.hpp>

BinaryDataContainer::BinaryDataContainer() = default;

BinaryDataContainer::BinaryDataContainer(const sal_uInt8* pData, size_t nSize)
    : mpData(std::make_shared<std::vector<sal_uInt8>>(nSize))
{
    std::copy(pData, pData + nSize, mpData->data());
}

BinaryDataContainer::BinaryDataContainer(std::unique_ptr<std::vector<sal_uInt8>> aData)
    : mpData(std::shared_ptr<std::vector<sal_uInt8>>(aData.release(), aData.get_deleter()))
{
}

size_t BinaryDataContainer::calculateHash() const
{
    size_t nSeed = 0;
    boost::hash_combine(nSeed, getSize());
    for (sal_uInt8 const& rByte : *mpData)
        boost::hash_combine(nSeed, rByte);
    return nSeed;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
