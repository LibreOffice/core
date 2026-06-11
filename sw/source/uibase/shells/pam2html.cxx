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

#include <sal/config.h>

#include <config_features.h>
#include <shellio.hxx>
#include <swdtflvr.hxx>
#include <wrtsh.hxx>
#include <vcl/htmltransferable.hxx>
#include <pam2html.hxx>

namespace SwPam2Html
{
OString ExportPaMToHTML(SwPaM* pCursor)
{
    SolarMutexGuard gMutex;
    OString aResult;
    WriterRef xWrt;
    GetHTMLWriter(u"NoLineLimit,SkipHeaderFooter,NoPrettyPrint", OUString(), xWrt);
    if (pCursor != nullptr)
    {
        SvMemoryStream aMemoryStream;
        SwWriter aWriter(aMemoryStream, *pCursor);
        ErrCodeMsg nError = aWriter.Write(xWrt);
        if (nError.IsError())
        {
            SAL_WARN("sw.ui", "ExportPaMToHTML: failed to export selection to HTML " << nError);
            return {};
        }
        aResult
            = OString(static_cast<const char*>(aMemoryStream.GetData()), aMemoryStream.GetSize());
        aResult = aResult.replaceAll("<p"_ostr, "<span"_ostr);
        aResult = aResult.replaceAll("</p>"_ostr, "</span>"_ostr);

        // HTML has for that <br> and <p> also does new line
        aResult = aResult.replaceAll("<ul>"_ostr, ""_ostr);
        aResult = aResult.replaceAll("</ul>"_ostr, ""_ostr);
        aResult = aResult.replaceAll("<ol>"_ostr, ""_ostr);
        aResult = aResult.replaceAll("</ol>"_ostr, ""_ostr);
        aResult = aResult.replaceAll("\n"_ostr, ""_ostr).trim();
        return aResult;
    }
    return {};
}

void PasteHTMLToPaM(SwWrtShell& rWrtSh, const SwPaM* pCursor, const OString& rData)
{
    SolarMutexGuard gMutex;
    rtl::Reference<vcl::unohelper::HtmlTransferable> pHtmlTransferable
        = new vcl::unohelper::HtmlTransferable(rData);
    if (pHtmlTransferable.is())
    {
        TransferableDataHelper aDataHelper(pHtmlTransferable);
        if (aDataHelper.GetXTransferable().is()
            && SwTransferable::IsPasteSpecial(rWrtSh, aDataHelper))
        {
            rWrtSh.SetSelection(*pCursor);
            SwTransferable::Paste(rWrtSh, aDataHelper);
            rWrtSh.KillSelection(nullptr, false);
        }
    }
}
}