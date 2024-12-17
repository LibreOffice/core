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

#include <rtl/strbuf.hxx>
#include <map>
#include <vcl/pdfwriter.hxx>

namespace vcl::pdf
{
enum class ResourceKind
{
    XObject,
    ExtGState,
    Shading,
    Pattern
};

struct ResourceDict
{
    // note: handle fonts globally for performance
    std::map<OString, sal_Int32> m_aXObjects;
    std::map<OString, sal_Int32> m_aExtGStates;
    std::map<OString, sal_Int32> m_aShadings;
    std::map<OString, sal_Int32> m_aPatterns;

    void append(OStringBuffer& rBuffer, sal_Int32 nFontDictObject, PDFWriter::PDFVersion eVersion);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
