/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/htmltesttools.hxx>

htmlDocPtr HtmlTestTools::parseHtml(utl::TempFile& aTempFile)
{
    SvFileStream aFileStream(aTempFile.GetURL(), STREAM_READ);
    sal_Size nSize = aFileStream.remainingSize();

    boost::scoped_array<sal_uInt8> pBuffer(new sal_uInt8[nSize + 1]);

    aFileStream.Read(pBuffer.get(), nSize);

    pBuffer[nSize] = 0;

    return htmlParseDoc(reinterpret_cast<xmlChar*>(pBuffer.get()), NULL);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
