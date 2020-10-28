/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/types.h>
#include <sal/log.hxx>
#include <vcl/dllapi.h>

#include <map>
#include <vector>
#include <memory>

#include <vcl/filter/pdfdocument.hxx>

namespace vcl
{
struct VCL_DLLPUBLIC ExternalPDFStream
{
    std::vector<sal_uInt8> maData;
    std::unique_ptr<filter::PDFDocument> mpPDFDocument;
    std::map<sal_Int32, sal_Int32> maCopiedResources;

    std::map<sal_Int32, sal_Int32>& getCopiedResources() { return maCopiedResources; }

    filter::PDFDocument& getPDFDocument()
    {
        if (!mpPDFDocument)
        {
            SvMemoryStream aPDFStream;
            aPDFStream.WriteBytes(maData.data(), maData.size());
            aPDFStream.Seek(0);
            mpPDFDocument = std::make_unique<filter::PDFDocument>();
            if (!mpPDFDocument->Read(aPDFStream))
            {
                SAL_WARN("vcl.pdfwriter",
                         "PDFWriterImpl::writeReferenceXObject: reading the PDF document failed");
            }
        }
        return *mpPDFDocument;
    }
};

class VCL_DLLPUBLIC ExternalPDFStreams
{
private:
    std::map<std::vector<sal_uInt8>, sal_Int32> maStreamIndexMap;
    std::vector<ExternalPDFStream> maStreamList;

public:
    ExternalPDFStreams() {}

    sal_Int32 store(const sal_uInt8* pData, sal_uInt32 nLength);

    ExternalPDFStream& get(sal_uInt32 nIndex);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
