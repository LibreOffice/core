/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/htmltesttools.hxx>

#include <memory>

htmlDocPtr HtmlTestTools::parseHtml(utl::TempFile& aTempFile)
{
    SvFileStream aFileStream(aTempFile.GetURL(), StreamMode::READ);
    htmlDocPtr doc = parseHtmlStream(&aFileStream);
    xmlFree(doc->name);
    doc->name = reinterpret_cast<char *>(
        xmlStrdup(
            reinterpret_cast<xmlChar const *>(
                OUStringToOString(
                    aTempFile.GetURL(), RTL_TEXTENCODING_UTF8).getStr())));
    return doc;
}

htmlDocPtr HtmlTestTools::parseHtmlStream(SvStream* pStream)
{
    sal_Size nSize = pStream->remainingSize();
    std::unique_ptr<sal_uInt8[]> pBuffer(new sal_uInt8[nSize + 1]);
    pStream->Read(pBuffer.get(), nSize);
    pBuffer[nSize] = 0;
    return htmlParseDoc(reinterpret_cast<xmlChar*>(pBuffer.get()), nullptr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
