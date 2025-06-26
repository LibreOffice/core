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
#include <unotools/tempfile.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/seqstream.hxx>
#include <comphelper/hash.hxx>
#include <sal/log.hxx>

#include <vector>

struct BinaryDataContainer::Impl
{
    // temp file to store the data out of RAM if necessary
    std::unique_ptr<utl::TempFileFast> mpFile;
    // the binary data
    std::shared_ptr<std::vector<sal_uInt8>> mpData;

    Impl(SvStream& stream, size_t size) { readData(stream, size); }

    /// Populate mpData from the stream
    void readData(SvStream& stream, size_t size)
    {
        auto pData = std::make_shared<std::vector<sal_uInt8>>(size);
        if (stream.ReadBytes(pData->data(), pData->size()) == size)
            mpData = std::move(pData);
    }

    /// ensure the data is in-RAM
    void ensureSwappedIn()
    {
        if (mpData || !mpFile)
            return;

        auto pStream = mpFile->GetStream(StreamMode::READ);
        pStream->Seek(0);
        readData(*pStream, pStream->remainingSize());

        // Horrifying data loss ...
        SAL_WARN_IF(pStream->GetError(), "vcl",
                    "Inconsistent system - failed to swap image back in");
    }

    void swapOut()
    {
        if (mpFile)
        {
            // we already have it swapped out.
            mpData.reset();
            return;
        }

        if (!mpData || mpData->empty())
            return;

        mpFile.reset(new utl::TempFileFast());
        auto pStream = mpFile->GetStream(StreamMode::READWRITE);

        pStream->WriteBytes(mpData->data(), mpData->size());

        mpData.reset();
    }
};

BinaryDataContainer::BinaryDataContainer(SvStream& stream, size_t size)
    : mpImpl(new Impl(stream, size))
{
}

size_t BinaryDataContainer::calculateHash() const
{
    size_t nSeed = 0;
    if (mpImpl && mpImpl->mpData && !mpImpl->mpData->empty())
    {
        o3tl::hash_combine(nSeed, getSize());
        for (sal_uInt8 const& rByte : *mpImpl->mpData)
            o3tl::hash_combine(nSeed, rByte);
    }
    return nSeed;
}

std::vector<unsigned char> BinaryDataContainer::calculateSHA1() const
{
    comphelper::Hash aHashEngine(comphelper::HashType::SHA1);
    aHashEngine.update(getData(), getSize());
    return aHashEngine.finalize();
}

css::uno::Sequence<sal_Int8> BinaryDataContainer::getCopyAsByteSequence() const
{
    if (isEmpty())
        return css::uno::Sequence<sal_Int8>();
    assert(mpImpl);

    css::uno::Sequence<sal_Int8> aData(getSize());

    std::copy(mpImpl->mpData->cbegin(), mpImpl->mpData->cend(), aData.getArray());

    return aData;
}

namespace
{
/*
 * Hold a reference on the internal state in case we swap out
 * and free the vector while someone holds an SvStream pointer.
 */
class ReferencedMemoryStream : public SvMemoryStream
{
    std::shared_ptr<std::vector<sal_uInt8>> mpData;

public:
    ReferencedMemoryStream(const std::shared_ptr<std::vector<sal_uInt8>>& pData)
        : SvMemoryStream(pData->data(), pData->size(), StreamMode::READ)
        , mpData(pData)
    {
    }
};

class ReferencedXInputStream : public comphelper::MemoryInputStream
{
    std::shared_ptr<std::vector<sal_uInt8>> mpData;

public:
    ReferencedXInputStream(const std::shared_ptr<std::vector<sal_uInt8>>& pData)
        : comphelper::MemoryInputStream(reinterpret_cast<const sal_Int8*>(pData->data()),
                                        pData->size())
        , mpData(pData)
    {
    }
};
}

std::shared_ptr<SvStream> BinaryDataContainer::getAsStream()
{
    ensureSwappedIn(); // TODO: transfer in streamed chunks
    return std::make_shared<ReferencedMemoryStream>(mpImpl->mpData);
}

css::uno::Reference<css::io::XInputStream> BinaryDataContainer::getAsXInputStream()
{
    ensureSwappedIn(); // TODO: transfer in streamed chunks
    return new ReferencedXInputStream(mpImpl->mpData);
}

std::size_t BinaryDataContainer::writeToStream(SvStream& rStream) const
{
    ensureSwappedIn(); // TODO: transfer in streamed chunks
    return rStream.WriteBytes(getData(), getSize());
}

size_t BinaryDataContainer::getSize() const
{
    ensureSwappedIn();
    return mpImpl && mpImpl->mpData ? mpImpl->mpData->size() : 0;
}

size_t BinaryDataContainer::getSizeBytes() const
{
    return mpImpl && mpImpl->mpData ? mpImpl->mpData->size() : 0;
}

bool BinaryDataContainer::isEmpty() const
{
    ensureSwappedIn();
    return !mpImpl || !mpImpl->mpData || mpImpl->mpData->empty();
}

const sal_uInt8* BinaryDataContainer::getData() const
{
    ensureSwappedIn();
    return mpImpl && mpImpl->mpData ? mpImpl->mpData->data() : nullptr;
}

void BinaryDataContainer::ensureSwappedIn() const
{
    if (mpImpl)
        mpImpl->ensureSwappedIn();
}

void BinaryDataContainer::swapOut() const
{
    // Only bother reducing memory footprint in kit mode - for mobile/online etc.
    if (!mpImpl || !comphelper::LibreOfficeKit::isActive())
        return;

    mpImpl->swapOut();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
