/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "HtmlFmtFlt.hxx"

#include <rtl/string.h>

#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <cassert>

using namespace com::sun::star::uno;





//













//






namespace 
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

} 




const std::string TAG_HTML = std::string("<html>");
const std::string TAG_END_HTML = std::string("</html>");



const std::string TAG_BODY = std::string("<body");
const std::string TAG_END_BODY = std::string("</body");

Sequence<sal_Int8> SAL_CALL TextHtmlToHTMLFormat(Sequence<sal_Int8>& aTextHtml)
{
    OSL_ASSERT(aTextHtml.getLength() > 0);

    if (!(aTextHtml.getLength() > 0))
        return Sequence<sal_Int8>();

    
    std::string dummyHtmlHeader = GetHtmlFormatHeader(0, 0, 0, 0);
    size_t lHtmlFormatHeader = dummyHtmlHeader.length();

    std::string textHtml(
        reinterpret_cast<const sal_Char*>(aTextHtml.getConstArray()),
        reinterpret_cast<const sal_Char*>(aTextHtml.getConstArray()) + aTextHtml.getLength());

    std::string::size_type nStartHtml = textHtml.find(TAG_HTML) + lHtmlFormatHeader - 1; 
    std::string::size_type nEndHtml = textHtml.find(TAG_END_HTML) + lHtmlFormatHeader + TAG_END_HTML.length() + 1; 

    
    
    std::string::size_type nStartFragment = textHtml.find(">", textHtml.find(TAG_BODY)) + lHtmlFormatHeader + 1;
    std::string::size_type nEndFragment = textHtml.find(TAG_END_BODY) + lHtmlFormatHeader;

    std::string htmlFormat = GetHtmlFormatHeader(nStartHtml, nEndHtml, nStartFragment, nEndFragment);
    htmlFormat += textHtml;

    Sequence<sal_Int8> byteSequence(htmlFormat.length() + 1); 
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
  assert(isHTMLFormat(aHTMLFormat) && "No HTML Format provided");

  Sequence<sal_Int8>& nonconstHTMLFormatRef = const_cast< Sequence<sal_Int8>& >(aHTMLFormat);
  sal_Char* dataStart = reinterpret_cast<sal_Char*>(nonconstHTMLFormatRef.getArray());
  sal_Char* dataEnd = dataStart + nonconstHTMLFormatRef.getLength() - 1;
  const sal_Char* htmlStartTag = strcasestr(dataStart, HtmlStartTag);

  assert(htmlStartTag && "Seems to be no HTML at all");

  
  
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
