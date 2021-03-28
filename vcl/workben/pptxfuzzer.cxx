/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/stream.hxx>
#include <vcl/FilterConfigItem.hxx>
#include "commonfuzzer.hxx"

extern "C" bool TestImportPPTX(SvStream& rStream);

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv)
{
    TypicalFuzzerInitialize(argc, argv);
    return 0;
}

extern "C" void* SdCreateDialogFactory() { return nullptr; }

extern "C" void* com_sun_star_comp_Draw_VisioImportFilter_get_implementation() { return nullptr; }

extern "C" void* sdext_PDFIHybridAdaptor_get_implementation() { return nullptr; }

extern "C" void* sdext_PDFIRawAdaptor_Writer_get_implementation() { return nullptr; }

extern "C" void* sdext_PDFIRawAdaptor_Draw_get_implementation() { return nullptr; }

extern "C" void* sdext_PDFIRawAdaptor_Impress_get_implementation() { return nullptr; }

extern "C" void* sdext_PDFDetector_get_implementation() { return nullptr; }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    SvMemoryStream aStream(const_cast<uint8_t*>(data), size, StreamMode::READ);
    (void)TestImportPPTX(aStream);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
