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

/** Container for the binary data, which responsiility is to manage the
 *  make it as simple as possible to manage the binary data. The binary
 *  data can be anything, but typically it is a in-memory data from
 *  files (i.e. files of graphic formats).
 */
class VCL_DLLPUBLIC BinaryDataContainer final
{
private:
    // the binary data
    std::shared_ptr<std::vector<sal_uInt8>> mpData;

public:
    explicit BinaryDataContainer();
    explicit BinaryDataContainer(const sal_uInt8* pData, size_t nSize);

    BinaryDataContainer(const BinaryDataContainer& rBinaryDataContainer)
        : mpData(rBinaryDataContainer.mpData)
    {
    }

    BinaryDataContainer(BinaryDataContainer&& rBinaryDataContainer)
        : mpData(std::move(rBinaryDataContainer.mpData))
    {
    }

    BinaryDataContainer& operator=(const BinaryDataContainer& rBinaryDataContainer)
    {
        mpData = rBinaryDataContainer.mpData;
        return *this;
    }

    BinaryDataContainer& operator=(BinaryDataContainer&& rBinaryDataContainer)
    {
        mpData = std::move(rBinaryDataContainer.mpData);
        return *this;
    }

    size_t getSize() const { return mpData ? mpData->size() : 0; }
    bool isEmpty() const { return !mpData || mpData->empty(); }
    const sal_uInt8* getData() const { return mpData ? mpData->data() : nullptr; }

    size_t calculateHash() const;

    auto cbegin() { return mpData->cbegin(); }

    auto cend() { return mpData->cend(); }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
