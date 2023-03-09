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
#include <o3tl/hash_combine.hxx>

BinaryDataContainer::BinaryDataContainer(SvStream& stream, size_t size)
{
    auto pBuffer = std::make_shared<std::vector<sal_uInt8>>(size);
    if (stream.ReadBytes(pBuffer->data(), pBuffer->size()) == size)
        mpData = std::move(pBuffer);
}

size_t BinaryDataContainer::calculateHash() const
{
    size_t nSeed = 0;
    if (mpData)
    {
        o3tl::hash_combine(nSeed, getSize());
        for (sal_uInt8 const& rByte : *mpData)
            o3tl::hash_combine(nSeed, rByte);
    }
    return nSeed;
}

SvMemoryStream BinaryDataContainer::getMemoryStream()
{
    return SvMemoryStream(mpData ? mpData->data() : nullptr, getSize(), StreamMode::READ);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
