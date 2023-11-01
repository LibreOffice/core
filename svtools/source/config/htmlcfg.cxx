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

#include <svtools/htmlcfg.hxx>
#include <unotools/syslocale.hxx>
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
        default:
            nExpMode = HTML_CFG_NS40;
            break;
    }
    return nExpMode;
}

} // namespace SvxHtmlOptions

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
