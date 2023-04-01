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

#include <sal/config.h>

#include <com/sun/star/uno/Sequence.hxx>
#include <tools/stream.hxx>
#include <vcl/dllapi.h>

#include <vector>
#include <memory>

/** Container for the binary data, whose responsibility is to manage the
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
    BinaryDataContainer() = default;
    BinaryDataContainer(SvStream& stream, size_t size);

    BinaryDataContainer(const BinaryDataContainer& rBinaryDataContainer) = default;

    BinaryDataContainer(BinaryDataContainer&& rBinaryDataContainer) noexcept = default;

    BinaryDataContainer& operator=(const BinaryDataContainer& rBinaryDataContainer) = default;

    BinaryDataContainer& operator=(BinaryDataContainer&& rBinaryDataContainer) noexcept = default;

    size_t getSize() const;
    bool isEmpty() const;
    const sal_uInt8* getData() const;
    css::uno::Sequence<sal_Int8> getAsSequence() const;

    // Returns the data as a stream open for reading
    SvMemoryStream getMemoryStream();
    std::size_t writeToStream(SvStream &rStream) const;

    size_t calculateHash() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
