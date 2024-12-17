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

#include <rtl/string.hxx>
#include <tools/stream.hxx>
#include <memory>
#include <vector>

namespace vcl::pdf
{
class XmpMetadata
{
private:
    bool mbWritten = false;
    std::unique_ptr<SvMemoryStream> mpMemoryStream;

public:
    OString msTitle;
    OString msAuthor;
    OString msSubject;
    OString msProducer;
    OString msPDFVersion;
    // For PDF/A-1 to PDF/A-3 this is required and we shall use pdfaid:conformance value B (we don't support A)
    // For PDF/A-4 the pdfaid:conformance entry is optional and the only valid values are E or F
    // We shall set F if we use embedded files
    OString msConformance = "B"_ostr;
    OString msKeywords;
    std::vector<OString> maContributor;
    OString msCoverage;
    OString msIdentifier;
    std::vector<OString> maPublisher;
    std::vector<OString> maRelation;
    OString msRights;
    OString msSource;
    OString msType;
    OString m_sCreatorTool;
    OString m_sCreateDate;

    sal_Int32 mnPDF_A = 0;
    bool mbPDF_UA = false;

public:
    XmpMetadata();
    sal_uInt64 getSize();
    const void* getData();

private:
    void write();
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
