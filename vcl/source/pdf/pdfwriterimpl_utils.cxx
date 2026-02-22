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

#include <sal/types.h>
#include <rtl/character.hxx>
#include <rtl/math.hxx>
#include <osl/time.h>
#include <comphelper/hash.hxx>

#include <pdf/COSWriter.hxx>

#include "pdfwriter_utils.hxx"

#include <memory>
#include <stdlib.h>

namespace vcl::pdf
{

void appendObjectID(sal_Int32 nObjectID, OStringBuffer & aLine)
{
    aLine.append(nObjectID);
    aLine.append(" 0 obj\n");
}

void appendObjectReference(sal_Int32 nObjectID, OStringBuffer & aLine)
{
    aLine.append(nObjectID);
    aLine.append(" 0 R ");
}

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
void appendDestinationName( std::u16string_view rString, OStringBuffer& rBuffer )
{
    for( auto aChar: rString)
    {
        if( rtl::isAsciiAlphanumeric(aChar) || aChar == '-' )
        {
            rBuffer.append(static_cast<char>(aChar));
        }
        else
        {
            const sal_Int8 nValueHigh = sal_Int8(aChar >> 8);

            if (nValueHigh > 0)
                COSWriter::appendHex(nValueHigh, rBuffer);

            COSWriter::appendHex(static_cast<sal_Int8>(aChar & 255 ), rBuffer);
        }
    }
}

void appendFixedInt( sal_Int32 nValue, OStringBuffer& rBuffer )
{
    if( nValue < 0 )
    {
        rBuffer.append( '-' );
        nValue = -nValue;
    }

    sal_Int32 nFactor = 1;
    sal_Int32 nDiv = nLog10Divisor;

    while( nDiv-- )
        nFactor *= 10;

    const sal_Int32 nInt = nValue / nFactor;
    rBuffer.append( nInt );
    if (nFactor > 1 && nValue % nFactor)
    {
        rBuffer.append( '.' );
        do
        {
            nFactor /= 10;
            rBuffer.append((nValue / nFactor) % 10);
        }
        while (nFactor > 1 && nValue % nFactor); // omit trailing zeros
    }
}

// appends a double. PDF does not accept exponential format, only fixed point
void appendDouble(double fValue, OStringBuffer& rBuffer, sal_Int32 nPrecision)
{
    rtl::math::doubleToStringBuffer(rBuffer, fValue, rtl_math_StringFormat_F, nPrecision, '.', true);
}

void appendColor( const Color& rColor, OStringBuffer& rBuffer, bool bConvertToGrey )
{
    if( rColor == COL_TRANSPARENT )
        return;

    if( bConvertToGrey )
    {
        const sal_uInt8 cByte = rColor.GetLuminance();
        appendDouble( cByte / 255.0, rBuffer );
    }
    else
    {
        appendDouble( rColor.GetRed() / 255.0, rBuffer );
        rBuffer.append( ' ' );
        appendDouble( rColor.GetGreen() / 255.0, rBuffer );
        rBuffer.append( ' ' );
        appendDouble( rColor.GetBlue() / 255.0, rBuffer );
    }
}

void appendPdfTimeDate(OStringBuffer & rBuffer,
    sal_Int16 year, sal_uInt16 month, sal_uInt16 day, sal_uInt16 hours, sal_uInt16 minutes, sal_uInt16 seconds, sal_Int32 tzDelta)
{
    rBuffer.append("D:");
    rBuffer.append(char('0' + ((year / 1000) % 10)));
    rBuffer.append(char('0' + ((year / 100) % 10)));
    rBuffer.append(char('0' + ((year / 10) % 10)));
    rBuffer.append(char('0' + (year % 10)));
    rBuffer.append(char('0' + ((month / 10) % 10)));
    rBuffer.append(char('0' + (month % 10)));
    rBuffer.append(char('0' + ((day / 10) % 10)));
    rBuffer.append(char('0' + (day % 10)));
    rBuffer.append(char('0' + ((hours / 10) % 10)));
    rBuffer.append(char('0' + (hours % 10)));
    rBuffer.append(char('0' + ((minutes / 10) % 10)));
    rBuffer.append(char('0' + (minutes % 10)));
    rBuffer.append(char('0' + ((seconds / 10) % 10)));
    rBuffer.append(char('0' + (seconds % 10)));

    if (tzDelta == 0)
    {
        rBuffer.append("Z");
        return;
    }

    if (tzDelta > 0 )
        rBuffer.append("+");
    else
    {
        rBuffer.append("-");
        tzDelta = -tzDelta;
    }

    rBuffer.append(char('0' + ((tzDelta / 36000) % 10)));
    rBuffer.append(char('0' + ((tzDelta / 3600) % 10)));
    rBuffer.append("'");
    rBuffer.append(char('0' + ((tzDelta / 600) % 6)));
    rBuffer.append(char('0' + ((tzDelta / 60) % 10)));
}

const char* getPDFVersionStr(PDFWriter::PDFVersion ePDFVersion)
{
    switch (ePDFVersion)
    {
        case PDFWriter::PDFVersion::PDF_A_1:
        case PDFWriter::PDFVersion::PDF_1_4:
            return "1.4";
        case PDFWriter::PDFVersion::PDF_1_5:
            return "1.5";
        case PDFWriter::PDFVersion::PDF_1_6:
            return "1.6";
        default:
        case PDFWriter::PDFVersion::PDF_A_2:
        case PDFWriter::PDFVersion::PDF_A_3:
        case PDFWriter::PDFVersion::PDF_1_7:
            return "1.7";
        // PDF 2.0
        case PDFWriter::PDFVersion::PDF_A_4:
        case PDFWriter::PDFVersion::PDF_2_0:
            return "2.0";
    }
}

void computeDocumentIdentifier(std::vector<sal_uInt8>& o_rIdentifier,
                               const PDFWriter::PDFDocInfo& i_rDocInfo,
                               const OString& i_rCString1,
                               const css::util::DateTime& rCreationMetaDate, OString& o_rCString2)
{
    o_rIdentifier.clear();

    //build the document id
    OString aInfoValuesOut;
    OStringBuffer aID(1024);
    if (!i_rDocInfo.Title.isEmpty())
        COSWriter::appendUnicodeTextString(i_rDocInfo.Title, aID);
    if (!i_rDocInfo.Author.isEmpty())
        COSWriter::appendUnicodeTextString(i_rDocInfo.Author, aID);
    if (!i_rDocInfo.Subject.isEmpty())
        COSWriter::appendUnicodeTextString(i_rDocInfo.Subject, aID);
    if (!i_rDocInfo.Keywords.isEmpty())
        COSWriter::appendUnicodeTextString(i_rDocInfo.Keywords, aID);
    if (!i_rDocInfo.Creator.isEmpty())
        COSWriter::appendUnicodeTextString(i_rDocInfo.Creator, aID);
    if (!i_rDocInfo.Producer.isEmpty())
        COSWriter::appendUnicodeTextString(i_rDocInfo.Producer, aID);

    TimeValue aTVal, aGMT;
    oslDateTime aDT;
    aDT.NanoSeconds = rCreationMetaDate.NanoSeconds;
    aDT.Seconds = rCreationMetaDate.Seconds;
    aDT.Minutes = rCreationMetaDate.Minutes;
    aDT.Hours = rCreationMetaDate.Hours;
    aDT.Day = rCreationMetaDate.Day;
    aDT.Month = rCreationMetaDate.Month;
    aDT.Year = rCreationMetaDate.Year;

    osl_getSystemTime(&aGMT);
    osl_getLocalTimeFromSystemTime(&aGMT, &aTVal);
    osl_getDateTimeFromTimeValue( &aTVal, &aDT );
    OStringBuffer aCreationMetaDateString(64);

    // i59651: we fill the Metadata date string as well, if PDF/A is requested
    // according to ISO 19005-1:2005 6.7.3 the date is corrected for
    // local time zone offset UTC only, whereas Acrobat 8 seems
    // to use the localtime notation only
    // according to a recommendation in XMP Specification (Jan 2004, page 75)
    // the Acrobat way seems the right approach
    aCreationMetaDateString.append(
        OStringChar(char('0' + ((aDT.Year / 1000) % 10)))
        + OStringChar(char('0' + ((aDT.Year / 100) % 10)))
        + OStringChar(char('0' + ((aDT.Year / 10) % 10)))
        + OStringChar(char('0' + ((aDT.Year) % 10)))
        + OStringChar('-')
        + OStringChar(char('0' + ((aDT.Month / 10) % 10)))
        + OStringChar(char('0' + ((aDT.Month) % 10)))
        + OStringChar('-')
        + OStringChar(char('0' + ((aDT.Day / 10) % 10)))
        + OStringChar(char('0' + ((aDT.Day) % 10)))
        + OStringChar('T')
        + OStringChar(char('0' + ((aDT.Hours / 10) % 10)))
        + OStringChar(char('0' + ((aDT.Hours) % 10)))
        + OStringChar(':')
        + OStringChar(char('0' + ((aDT.Minutes / 10) % 10)))
        + OStringChar(char('0' + ((aDT.Minutes) % 10)))
        + OStringChar(':')
        + OStringChar(char('0' + ((aDT.Seconds / 10) % 10)))
        + OStringChar(char('0' + ((aDT.Seconds) % 10))));

    sal_uInt32 nDelta = 0;
    if (aGMT.Seconds > aTVal.Seconds)
    {
        nDelta = aGMT.Seconds - aTVal.Seconds;
        aCreationMetaDateString.append("-");
    }
    else if (aGMT.Seconds < aTVal.Seconds)
    {
        nDelta = aTVal.Seconds - aGMT.Seconds;
        aCreationMetaDateString.append("+");
    }
    else
    {
        aCreationMetaDateString.append("Z");
    }
    if (nDelta)
    {
        aCreationMetaDateString.append(char('0' + ((nDelta / 36000) % 10)));
        aCreationMetaDateString.append(char('0' + ((nDelta / 3600) % 10)));
        aCreationMetaDateString.append(":");
        aCreationMetaDateString.append(char('0' + ((nDelta / 600) % 6)));
        aCreationMetaDateString.append(char('0' + ((nDelta / 60) % 10)));
    }
    aID.append(i_rCString1.getStr(), i_rCString1.getLength());

    aInfoValuesOut = aID.makeStringAndClear();
    o_rCString2 = aCreationMetaDateString.makeStringAndClear();

    ::comphelper::Hash aDigest(::comphelper::HashType::MD5);
    aDigest.update(&aGMT, sizeof(aGMT));
    aDigest.update(aInfoValuesOut.getStr(), aInfoValuesOut.getLength());
    //the binary form of the doc id is needed for encryption stuff
    o_rIdentifier = aDigest.finalize();
}

} // end namespace vcl::pdf

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
