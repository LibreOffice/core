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

#include <rtl/string.hxx>
#include <sal/types.h>
#include <vcl/dllapi.h>

class SvMemoryStream;

namespace vcl
{
class PDFObjectContainer;
namespace filter
{
class PDFObjectElement;
}

/// Copies objects from one PDF file into another one.
class VCL_DLLPUBLIC PDFObjectCopier
{
    PDFObjectContainer& m_rContainer;

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
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
