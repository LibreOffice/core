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

#include <vcl/dllapi.h>
#include <vector>
#include <memory>
#include <boost/functional/hash.hpp>

class VCL_DLLPUBLIC BinaryDataContainer final
{
private:
    // the binary data
    std::shared_ptr<std::vector<sal_uInt8>> mpData;

public:
    explicit BinaryDataContainer() {}

    explicit BinaryDataContainer(size_t nSize)
        : mpData(std::make_shared<std::vector<sal_uInt8>>(nSize))
    {
    }

    explicit BinaryDataContainer(const sal_uInt8* pData, size_t nSize)
        : mpData(std::make_shared<std::vector<sal_uInt8>>(nSize))
    {
        std::copy(pData, pData + nSize, mpData->data());
    }

    explicit BinaryDataContainer(const BinaryDataContainer& rBinaryDataContainer) = default;
    explicit BinaryDataContainer(BinaryDataContainer&& rBinaryDataContainer) = default;
    BinaryDataContainer& operator=(const BinaryDataContainer& rBinaryDataContainer) = default;
    BinaryDataContainer& operator=(BinaryDataContainer&& rBinaryDataContainer) = default;

    size_t getSize() const { return mpData ? mpData->size() : 0; }

    bool isEmpty() const { return mpData ? mpData->empty() : true; }

    const sal_uInt8* getData() const { return mpData ? mpData->data() : nullptr; }

    size_t calculateHash() const
    {
        size_t nSeed = 0;
        boost::hash_combine(nSeed, getSize());
        for (sal_uInt8 const& rByte : *mpData)
            boost::hash_combine(nSeed, rByte);
        return nSeed;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
