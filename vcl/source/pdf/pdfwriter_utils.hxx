/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <rtl/strbuf.hxx>

#include <vcl/outdev.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/pdfwriter.hxx>

#include <com/sun/star/util/DateTime.hpp>

#include <string_view>
#include <vector>

namespace vcl::pdf
{
struct PDFStructureElement;

constexpr std::string_view constNamespacePDF2("http://iso.org/pdf2/ssn");

constexpr sal_Int32 nLog10Divisor = 3;
constexpr double fDivisor = 1000.0;

constexpr double pixelToPoint(double px) { return px / fDivisor; }

constexpr sal_Int32 pointToPixel(double pt) { return sal_Int32(pt * fDivisor); }

void appendObjectID(sal_Int32 nObjectID, OStringBuffer& aLine);

void appendObjectReference(sal_Int32 nObjectID, OStringBuffer& aLine);

/*
 * Convert a string before using it.
 *
 * This string conversion function is needed because the destination name
 * in a PDF file seen through an Internet browser should be
 * specially crafted, in order to be used directly by the browser.
 * In this way the fragment part of a hyperlink to a PDF file (e.g. something
 * as 'test1/test2/a-file.pdf\#thefragment) will be (hopefully) interpreted by the
 * PDF reader (currently only Adobe Reader plug-in seems to be working that way) called
 * from inside the Internet browser as: 'open the file test1/test2/a-file.pdf
 * and go to named destination thefragment using default zoom'.
 * The conversion is needed because in case of a fragment in the form: Slide%201
 * (meaning Slide 1) as it is converted obeying the Inet rules, it will become Slide25201
 * using this conversion, in both the generated named destinations, fragment and GoToR
 * destination.
 *
 * The names for destinations are name objects and so they don't need to be encrypted
 * even though they expose the content of PDF file (e.g. guessing the PDF content from the
 * destination name).
 *
 * Further limitation: it is advisable to use standard ASCII characters for
 * OOo bookmarks.
*/
void appendDestinationName(std::u16string_view rString, OStringBuffer& rBuffer);

void appendFixedInt(sal_Int32 nValue, OStringBuffer& rBuffer);

// appends a double. PDF does not accept exponential format, only fixed point
void appendDouble(double fValue, OStringBuffer& rBuffer, sal_Int32 nPrecision = 10);

void appendColor(const Color& rColor, OStringBuffer& rBuffer, bool bConvertToGrey);

void appendPdfTimeDate(OStringBuffer& rBuffer, sal_Int16 year, sal_uInt16 month, sal_uInt16 day,
                       sal_uInt16 hours, sal_uInt16 minutes, sal_uInt16 seconds, sal_Int32 tzDelta);

const char* getPDFVersionStr(PDFWriter::PDFVersion ePDFVersion);

void computeDocumentIdentifier(std::vector<sal_uInt8>& o_rIdentifier,
                               const PDFWriter::PDFDocInfo& i_rDocInfo, const OString& i_rCString1,
                               const css::util::DateTime& rCreationMetaDate, OString& o_rCString2);

template <class GEOMETRY>
GEOMETRY convert(const MapMode& _rSource, const MapMode& _rDest, OutputDevice* _pPixelConversion,
                 const GEOMETRY& _rObject)
{
    GEOMETRY aPoint;
    if (MapUnit::MapPixel == _rSource.GetMapUnit())
    {
        aPoint = _pPixelConversion->PixelToLogic(_rObject, _rDest);
    }
    else
    {
        aPoint = OutputDevice::LogicToLogic(_rObject, _rSource, _rDest);
    }
    return aPoint;
}

} // end namespace vcl::pdf

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
