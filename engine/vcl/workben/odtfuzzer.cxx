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

#include <tools/stream.hxx>
#include <vcl/FilterConfigItem.hxx>
#include "commonfuzzer.hxx"

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <comphelper/storagehelper.hxx>
#include <o3tl/string_view.hxx>
#include <unotools/streamwrap.hxx>

#include <cstring>
#include <vector>

using namespace css;

extern "C" bool TestImportODT(SvStream& rStream);

extern "C" size_t LLVMFuzzerMutate(uint8_t* data, size_t size, size_t maxSize);

namespace
{
// A package nests deeper than this only when it was built to, and such a member is not worth
// choosing.
constexpr int MAX_DEPTH = 8;

void CollectStreams(const uno::Reference<embed::XStorage>& xStorage, const OUString& rPrefix,
                    int nDepth, std::vector<OUString>& rPaths)
{
    for (const OUString& rName : xStorage->getElementNames())
    {
        if (xStorage->isStreamElement(rName))
            rPaths.push_back(rPrefix + rName);
        else if (nDepth < MAX_DEPTH)
            CollectStreams(xStorage->openStorageElement(rName, embed::ElementModes::READ),
                           rPrefix + rName + "/", nDepth + 1, rPaths);
    }
}

std::vector<uint8_t> ReadMember(uno::Reference<embed::XStorage> xStorage, std::u16string_view aPath)
{
    sal_Int32 nIndex = 0;
    OUString aName(o3tl::getToken(aPath, 0, '/', nIndex));
    while (nIndex >= 0)
    {
        xStorage = xStorage->openStorageElement(aName, embed::ElementModes::READ);
        aName = OUString(o3tl::getToken(aPath, 0, '/', nIndex));
    }
    uno::Reference<io::XInputStream> xIn
        = xStorage->openStreamElement(aName, embed::ElementModes::READ)->getInputStream();
    std::vector<uint8_t> aData;
    uno::Sequence<sal_Int8> aBuffer;
    while (sal_Int32 nRead = xIn->readBytes(aBuffer, 65536))
        aData.insert(aData.end(), aBuffer.getConstArray(), aBuffer.getConstArray() + nRead);
    return aData;
}

void WriteMember(const uno::Reference<embed::XStorage>& xStorage, std::u16string_view aPath,
                 const std::vector<uint8_t>& rData)
{
    sal_Int32 nIndex = 0;
    OUString aName(o3tl::getToken(aPath, 0, '/', nIndex));
    if (nIndex >= 0)
    {
        uno::Reference<embed::XStorage> xSub
            = xStorage->openStorageElement(aName, embed::ElementModes::READWRITE);
        WriteMember(xSub, aPath.substr(nIndex), rData);
        uno::Reference<embed::XTransactedObject>(xSub, uno::UNO_QUERY_THROW)->commit();
        return;
    }
    uno::Reference<io::XStream> xStream = xStorage->openStreamElement(
        aName, embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE);
    uno::Reference<io::XOutputStream> xOut = xStream->getOutputStream();
    xOut->writeBytes(
        uno::Sequence<sal_Int8>(reinterpret_cast<const sal_Int8*>(rData.data()), rData.size()));
    xOut->closeOutput();
}

// Mutate one member of the package and zip it up again, so that most mutations reach the XML
// instead of breaking the container. Input that is not a readable package is mutated as bytes.
size_t MutatePackageMember(uint8_t* data, size_t size, size_t maxSize, unsigned int seed)
{
    std::vector<uint8_t> aInput(data, data + size);
    SvMemoryStream aInStream(aInput.data(), aInput.size(), StreamMode::READ);
    uno::Reference<embed::XStorage> xSource
        = comphelper::OStorageHelper::GetStorageOfFormatFromInputStream(
            PACKAGE_STORAGE_FORMAT_STRING, new utl::OSeekableInputStreamWrapper(aInStream));

    std::vector<OUString> aPaths;
    CollectStreams(xSource, OUString(), 0, aPaths);
    if (aPaths.empty())
        return 0;
    const OUString& rPath = aPaths[seed % aPaths.size()];

    std::vector<uint8_t> aMember = ReadMember(xSource, rPath);
    const size_t nMemberSize = aMember.size();
    aMember.resize(maxSize);
    aMember.resize(LLVMFuzzerMutate(aMember.data(), nMemberSize, maxSize));

    SvMemoryStream aOutStream;
    uno::Reference<embed::XStorage> xTarget
        = comphelper::OStorageHelper::GetStorageOfFormatFromStream(
            PACKAGE_STORAGE_FORMAT_STRING, new utl::OStreamWrapper(aOutStream));
    xSource->copyToStorage(xTarget);
    WriteMember(xTarget, rPath, aMember);
    uno::Reference<embed::XTransactedObject>(xTarget, uno::UNO_QUERY_THROW)->commit();

    const size_t nOutSize = aOutStream.TellEnd();
    if (nOutSize == 0 || nOutSize > maxSize)
        return 0;
    std::memcpy(data, aOutStream.GetData(), nOutSize);
    return nOutSize;
}
}

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv)
{
    TypicalFuzzerInitialize(argc, argv);
    return 0;
}

extern "C" size_t LLVMFuzzerCustomMutator(uint8_t* data, size_t size, size_t maxSize,
                                          unsigned int seed)
{
    // One mutation in sixteen goes to the raw bytes, so the container code gets some too.
    if (seed % 16 != 0)
    {
        try
        {
            if (size_t nNewSize = MutatePackageMember(data, size, maxSize, seed / 16))
                return nNewSize;
        }
        catch (const uno::Exception&)
        {
        }
    }
    return LLVMFuzzerMutate(data, size, maxSize);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    SvMemoryStream aStream(const_cast<uint8_t*>(data), size, StreamMode::READ);
    // input that does not load as a document is kept out of the corpus
    return TestImportODT(aStream) ? 0 : -1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
