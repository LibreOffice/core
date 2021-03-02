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
#include <vcl/BinaryDataContainer.hxx>

namespace vcl
{
// A external PDF stream, which stores the PDF stream data as byte array.
// This struct is also responsible to parsing the stream as a PDFDocument,
// and store its instance for the life-cycle of the struct, so that it
// reused to avoid unnecessary parsing.
struct VCL_DLLPUBLIC ExternalPDFStream
{
    BinaryDataContainer maDataContainer;
    std::shared_ptr<filter::PDFDocument> mpPDFDocument;
    std::map<sal_Int32, sal_Int32> maCopiedResources;

    std::map<sal_Int32, sal_Int32>& getCopiedResources() { return maCopiedResources; }

    std::shared_ptr<filter::PDFDocument>& getPDFDocument()
    {
        if (!mpPDFDocument)
        {
            SvMemoryStream aPDFStream;
            aPDFStream.WriteBytes(maDataContainer.getData(), maDataContainer.getSize());
            aPDFStream.Seek(0);
            auto pPDFDocument = std::make_shared<filter::PDFDocument>();
            if (!pPDFDocument->Read(aPDFStream))
            {
                SAL_WARN("vcl.pdfwriter",
                         "PDFWriterImpl::writeReferenceXObject: reading the PDF document failed");
            }
            else
            {
                mpPDFDocument = pPDFDocument;
            }
        }
        return mpPDFDocument;
    }
};

// Class to manage external PDF streams, for the de-duplication purpose.
class ExternalPDFStreams
{
private:
    std::map<std::vector<sal_uInt8>, sal_Int32> maStreamIndexMap;
    std::vector<ExternalPDFStream> maStreamList;

public:
    ExternalPDFStreams() {}

    sal_Int32 store(BinaryDataContainer const& rDataContainer);

    ExternalPDFStream& get(sal_uInt32 nIndex);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
