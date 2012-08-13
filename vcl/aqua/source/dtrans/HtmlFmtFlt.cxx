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

#include "HtmlFmtFlt.hxx"

#include <rtl/string.h>

#include <string>
#include <sstream>
#include <vector>
#include <iomanip>

#include <boost/assert.hpp>

using namespace com::sun::star::uno;

//------------------------------------------------------------------------------
// converts the openoffice text/html clipboard format to the HTML Format
// well known under MS Windows
// the MS HTML Format has a header before the real html data
//
// Version:1.0      Version number of the clipboard. Staring is 0.9
// StartHTML:       Byte count from the beginning of the clipboard to the start
//                  of the context, or -1 if no context
// EndHTML:         Byte count from the beginning of the clipboard to the end
//                  of the context, or -1 if no context
// StartFragment:   Byte count from the beginning of the clipboard to the
//                  start of the fragment
// EndFragment:     Byte count from the beginning of the clipboard to the
//                  end of the fragment
// StartSelection:  Byte count from the beginning of the clipboard to the
//                  start of the selection
// EndSelection:    Byte count from the beginning of the clipboard to the
//                  end of the selection
//
// StartSelection and EndSelection are optional
// The fragment should be preceded and followed by the HTML comments
// <!--StartFragment--> and <!--EndFragment--> (no space between !-- and the
// text
//------------------------------------------------------------------------------

namespace // private
{
std::string GetHtmlFormatHeader(size_t startHtml, size_t endHtml, size_t startFragment, size_t endFragment)
{
    std::ostringstream htmlHeader;
    htmlHeader << "Version:1.0" << '\r' << '\n';
    htmlHeader << "StartHTML:" << std::setw(10) << std::setfill('0') << std::dec << startHtml << '\r' << '\n';
    htmlHeader << "EndHTML:" << std::setw(10) << std::setfill('0') << std::dec << endHtml << '\r' << '\n';
    htmlHeader << "StartFragment:" << std::setw(10) << std::setfill('0') << std::dec << startFragment << '\r' << '\n';
    htmlHeader << "EndFragment:" << std::setw(10) << std::setfill('0') << std::dec << endFragment << '\r' << '\n';
    return htmlHeader.str();
}

} // namespace private


// the office allways writes the start and end html tag in upper cases and
// without spaces both tags don't allow parameters
const std::string TAG_HTML = std::string("<HTML>");
const std::string TAG_END_HTML = std::string("</HTML>");

// The body tag may have parameters so we need to search for the
// closing '>' manually e.g. <BODY param> #92840#
const std::string TAG_BODY = std::string("<BODY");
const std::string TAG_END_BODY = std::string("</BODY");

Sequence<sal_Int8> SAL_CALL TextHtmlToHTMLFormat(Sequence<sal_Int8>& aTextHtml)
{
    OSL_ASSERT(aTextHtml.getLength() > 0);

    if (!(aTextHtml.getLength() > 0))
        return Sequence<sal_Int8>();

    // fill the buffer with dummy values to calc the exact length
    std::string dummyHtmlHeader = GetHtmlFormatHeader(0, 0, 0, 0);
    size_t lHtmlFormatHeader = dummyHtmlHeader.length();

    std::string textHtml(
        reinterpret_cast<const sal_Char*>(aTextHtml.getConstArray()),
        reinterpret_cast<const sal_Char*>(aTextHtml.getConstArray()) + aTextHtml.getLength());

    std::string::size_type nStartHtml = textHtml.find(TAG_HTML) + lHtmlFormatHeader - 1; // we start one before '<HTML>' Word 2000 does also so
    std::string::size_type nEndHtml = textHtml.find(TAG_END_HTML) + lHtmlFormatHeader + TAG_END_HTML.length() + 1; // our SOffice 5.2 wants 2 behind </HTML>?

    // The body tag may have parameters so we need to search for the
    // closing '>' manually e.g. <BODY param> #92840#
    std::string::size_type nStartFragment = textHtml.find(">", textHtml.find(TAG_BODY)) + lHtmlFormatHeader + 1;
    std::string::size_type nEndFragment = textHtml.find(TAG_END_BODY) + lHtmlFormatHeader;

    std::string htmlFormat = GetHtmlFormatHeader(nStartHtml, nEndHtml, nStartFragment, nEndFragment);
    htmlFormat += textHtml;

    Sequence<sal_Int8> byteSequence(htmlFormat.length() + 1); // space the trailing '\0'
    memset(byteSequence.getArray(), 0, byteSequence.getLength());

    memcpy(
        static_cast<void*>(byteSequence.getArray()),
        static_cast<const void*>(htmlFormat.c_str()),
        htmlFormat.length());

    return byteSequence;
}

const char* HtmlStartTag = "<html";

Sequence<sal_Int8> HTMLFormatToTextHtml(const Sequence<sal_Int8>& aHTMLFormat)
{
  BOOST_ASSERT(isHTMLFormat(aHTMLFormat) && "No HTML Format provided");

  Sequence<sal_Int8>& nonconstHTMLFormatRef = const_cast< Sequence<sal_Int8>& >(aHTMLFormat);
  sal_Char* dataStart = reinterpret_cast<sal_Char*>(nonconstHTMLFormatRef.getArray());
  sal_Char* dataEnd = dataStart + nonconstHTMLFormatRef.getLength() - 1;
  const sal_Char* htmlStartTag = strcasestr(dataStart, HtmlStartTag);

  BOOST_ASSERT(htmlStartTag && "Seems to be no HTML at all");

  // It doesn't seem to be HTML? Well then simply return what has been
  // provided in non-debug builds
  if (htmlStartTag == NULL)
    {
    return aHTMLFormat;
    }

  sal_Int32 len = dataEnd - htmlStartTag;
  Sequence<sal_Int8> plainHtmlData(len);

  memcpy(static_cast<void*>(plainHtmlData.getArray()), htmlStartTag, len);

  return plainHtmlData;
}

/* A simple format detection. We are just comparing the first few bytes
   of the provided byte sequence to see whether or not it is the MS
   Office Html format. If it shows that this is not reliable enough we
   can improve this
*/
const char HtmlFormatStart[] = "Version:";
int HtmlFormatStartLen = (sizeof(HtmlFormatStart) - 1);

bool isHTMLFormat(const Sequence<sal_Int8>& aHtmlSequence)
{
  if (aHtmlSequence.getLength() < HtmlFormatStartLen)
    return false;

  return rtl_str_compareIgnoreAsciiCase_WithLength(HtmlFormatStart,
                                                   HtmlFormatStartLen,
                                                   reinterpret_cast<const sal_Char*>(aHtmlSequence.getConstArray()),
                                                   HtmlFormatStartLen) == 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
