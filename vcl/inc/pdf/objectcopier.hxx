/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <map>
#include <vector>

#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>
#include <sal/types.h>

class SvMemoryStream;

namespace vcl
{
class PDFObjectContainer;
namespace filter
{
class PDFObjectElement;
class PDFElement;
}

/// Copies objects from one PDF file into another one.
class PDFObjectCopier
{
    PDFObjectContainer& m_rContainer;

    void copyRecursively(OStringBuffer& rLine, filter::PDFElement& rInputElement,
                         SvMemoryStream& rDocBuffer,
                         std::map<sal_Int32, sal_Int32>& rCopiedResources);

public:
    PDFObjectCopier(PDFObjectContainer& rContainer);

    /// Copies resources of a given kind from an external page to the output,
    /// returning what has to be included in the new resource dictionary.
    OString copyExternalResources(filter::PDFObjectElement& rPage, const OString& rKind,
                                  std::map<sal_Int32, sal_Int32>& rCopiedResources);

    /// Copies a single resource from an external document, returns the new
    /// object ID in our document.
    sal_Int32 copyExternalResource(SvMemoryStream& rDocBuffer, filter::PDFObjectElement& rObject,
                                   std::map<sal_Int32, sal_Int32>& rCopiedResources);

    /// Copies resources of pPage into rLine.
    void copyPageResources(filter::PDFObjectElement* pPage, OStringBuffer& rLine);

    void copyPageResources(filter::PDFObjectElement* pPage, OStringBuffer& rLine,
                           std::map<sal_Int32, sal_Int32>& rCopiedResources);

    /// Copies page one or more page streams from rContentStreams into rStream.
    static sal_Int32 copyPageStreams(std::vector<filter::PDFObjectElement*>& rContentStreams,
                                     SvMemoryStream& rStream, bool& rCompressed,
                                     bool bIsTaggedNonReferenceXObject = false);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
