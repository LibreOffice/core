/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vector>

#include <sot/storage.hxx>

#include <tools/stream.hxx>

#include "commonfuzzer.hxx"

namespace
{

void traverse(const tools::SvRef<SotStorage>& rStorage, std::vector<unsigned char>& rBuf)
{
    SvStorageInfoList infos;

    rStorage->FillInfoList(&infos);

    for (const auto& info: infos)
    {
        if (info.IsStream())
        {
            // try to open and read all content
            tools::SvRef<SotStorageStream> xStream(rStorage->OpenSotStream(info.GetName(), StreamMode::STD_READ));
            const size_t nSize = xStream->GetSize();
            const size_t nRead = xStream->ReadBytes(rBuf.data(), nSize);
            (void) nRead;
        }
        else if (info.IsStorage())
        {
            tools::SvRef<SotStorage> xStorage(rStorage->OpenSotStorage(info.GetName(), StreamMode::STD_READ));

            // continue with children
            traverse(xStorage, rBuf);
        }
        else
        {
        }
    }
}

void TestImportOLE2(SvStream &rStream, size_t nSize)
{
    try
    {
        tools::SvRef<SotStorage> xRootStorage(new SotStorage(&rStream, false));
        std::vector<unsigned char> aTmpBuf(nSize);
        traverse(xRootStorage, aTmpBuf);
    }
    catch (...)
    {
    }
}

}

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    TypicalFuzzerInitialize(argc, argv);
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    SvMemoryStream aStream(const_cast<uint8_t*>(data), size, StreamMode::READ);
    TestImportOLE2(aStream, size);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
