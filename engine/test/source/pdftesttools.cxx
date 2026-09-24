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

#include <sal/config.h>

#include <test/pdftesttools.hxx>

#include <cppunit/TestAssert.h>

#include <rtl/strbuf.hxx>
#include <tools/stream.hxx>
#include <tools/zcodec.hxx>
#include <vcl/filter/pdfdocument.hxx>

OString PdfTestTools::getObjectStreamsData(vcl::filter::PDFDocument& rDocument)
{
    OStringBuffer aData;
    for (auto* pObject : rDocument.GetObjects())
    {
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"_ostr));
        if (!pType || pType->GetValue() != "ObjStm")
            continue;

        vcl::filter::PDFStreamElement* pObjStm = pObject->GetStream();
        CPPUNIT_ASSERT_MESSAGE("an /ObjStm object with no stream", pObjStm);
        SvMemoryStream aDecompressed;
        ZCodec aCodec;
        aCodec.BeginCompression();
        pObjStm->GetMemory().Seek(0);
        aCodec.Decompress(pObjStm->GetMemory(), aDecompressed);
        CPPUNIT_ASSERT_MESSAGE("an /ObjStm that does not inflate", aCodec.EndCompression() >= 0);
        aData.append(static_cast<const char*>(aDecompressed.GetData()), aDecompressed.GetSize());
    }
    return aData.makeStringAndClear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
