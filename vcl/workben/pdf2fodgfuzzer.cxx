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

extern "C" bool TestFODGExportPDF(SvStream& rStream);

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv)
{
    TypicalFuzzerInitialize(argc, argv);
    return 0;
}

extern "C" void* SdCreateDialogFactory() { return nullptr; }

extern "C" void* com_sun_star_comp_Draw_VisioImportFilter_get_implementation() { return nullptr; }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    SvMemoryStream aStream(const_cast<uint8_t*>(data), size, StreamMode::READ);
    bool bPDFLoaded = TestFODGExportPDF(aStream);
    // if the pdf didn't load then reject so that input will not be added to the corpus
    // we're not interested in input that doesn't go on to exercise the fodg export
    return bPDFLoaded ? 0 : -1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
