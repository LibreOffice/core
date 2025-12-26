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

#include <vcl/dllapi.h>

#include <memory>
#include <vector>

class SvStream;
namespace com::sun::star::io
{
class XInputStream;
}
namespace com::sun::star::uno
{
template <class E> class Sequence;
}
namespace com::sun::star::uno
{
template <class interface_type> class Reference;
}

/** Container for the binary data, whose responsibility is to manage the
 *  make it as simple as possible to manage the binary data. The binary
 *  data can be anything, but typically it is a in-memory data from
 *  files (i.e. files of graphic formats).
 */
class VCL_DLLPUBLIC BinaryDataContainer final
{
    struct Impl;

    std::shared_ptr<Impl> mpImpl;

    SAL_DLLPRIVATE void ensureSwappedIn() const;

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
    SAL_DLLPRIVATE css::uno::Sequence<sal_Int8> getCopyAsByteSequence() const;

    // Returns the data as a readonly stream open for reading
    SAL_DLLPRIVATE std::unique_ptr<SvStream> getAsStream() const;

    // Returns the data as a readonly stream open for reading
    SAL_DLLPRIVATE css::uno::Reference<css::io::XInputStream> getAsXInputStream() const;

    /// writes the contents to the given stream
    std::size_t writeToStream(SvStream& rStream) const;

    /// return the in-memory size in bytes as of now.
    SAL_DLLPRIVATE std::size_t getSizeBytes() const;

    /// swap out to disk for now
    SAL_DLLPRIVATE void swapOut() const;

    SAL_DLLPRIVATE size_t calculateHash() const;
    std::vector<unsigned char> calculateSHA1() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
