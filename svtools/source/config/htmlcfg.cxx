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

#include <osl/thread.h>
#include <svtools/htmlcfg.hxx>
#include <svtools/parhtml.hxx>
#include <unotools/syslocale.hxx>
#include <tools/debug.hxx>
#include <officecfg/Office/Common.hxx>

namespace SvxHtmlOptions
{
sal_uInt16 GetFontSize(sal_uInt16 nPos)
{
    switch (nPos)
    {
        case 0:
            return officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_1::get();
        case 1:
            return officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_2::get();
        case 2:
            return officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_3::get();
        case 3:
            return officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_4::get();
        case 4:
            return officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_5::get();
        case 5:
            return officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_6::get();
        case 6:
            return officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_7::get();
    }
    assert(false);
    return 0;
}

sal_uInt16 GetExportMode()
{
    sal_Int32 nExpMode = officecfg::Office::Common::Filter::HTML::Export::Browser::get();
    switch (nExpMode)
    {
        case 1:
            nExpMode = HTML_CFG_MSIE;
            break;
        case 3:
            nExpMode = HTML_CFG_WRITER;
            break;
        case 4:
            nExpMode = HTML_CFG_NS40;
            break;
        default:
            nExpMode = HTML_CFG_NS40;
            break;
    }
    return nExpMode;
}

bool IsPrintLayoutExtension()
{
    bool bRet = officecfg::Office::Common::Filter::HTML::Export::PrintLayout::get();
    switch (GetExportMode())
    {
        case HTML_CFG_MSIE:
        case HTML_CFG_NS40:
        case HTML_CFG_WRITER:
            break;
        default:
            bRet = false;
    }
    return bRet;
}

bool IsDefaultTextEncoding()
{
    std::optional<sal_Int32> x = officecfg::Office::Common::Filter::HTML::Export::Encoding::get();
    // if we have a value, then the text encoding is not default
    return !bool(x);
}
rtl_TextEncoding GetTextEncoding()
{
    std::optional<sal_Int32> x = officecfg::Office::Common::Filter::HTML::Export::Encoding::get();
    rtl_TextEncoding eRet;
    if (!x)
        eRet = SvtSysLocale::GetBestMimeEncoding();
    else
        eRet = static_cast<rtl_TextEncoding>(*x);
    return eRet;
}

void SetTextEncoding( rtl_TextEncoding eEnc )
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Filter::HTML::Export::Encoding::set(eEnc, batch);
    batch->commit();
}

} // namespace SvxHtmlOptions

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
