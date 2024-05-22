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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBAFILTERPROPSFROMFORMAT_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBAFILTERPROPSFROMFORMAT_HXX

#include <sal/config.h>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>
#include <ooo/vba/word/WdSaveFormat.hpp>

namespace
{
inline bool setFilterPropsFromFormat(sal_Int32 nFormat,
                                     css::uno::Sequence<css::beans::PropertyValue>& rProps)
{
    auto[begin, end] = asNonConstRange(rProps);
    auto pProp = std::find_if(begin, end, [](const css::beans::PropertyValue& rProp) {
        return rProp.Name == "FilterName";
    });
    if (pProp != end)
    {
        switch (nFormat)
        {
            case ooo::vba::word::WdSaveFormat::wdFormatDocument:
                pProp->Value <<= u"MS Word 97"_ustr;
                break;
            // Just save all the text formats as "Text"
            case ooo::vba::word::WdSaveFormat::wdFormatDOSText:
            case ooo::vba::word::WdSaveFormat::wdFormatDOSTextLineBreaks:
            case ooo::vba::word::WdSaveFormat::wdFormatEncodedText:
            case ooo::vba::word::WdSaveFormat::wdFormatText:
            case ooo::vba::word::WdSaveFormat::wdFormatTextLineBreaks:
                pProp->Value <<= u"Text"_ustr;
                break;
            case ooo::vba::word::WdSaveFormat::wdFormatFilteredHTML:
            case ooo::vba::word::WdSaveFormat::wdFormatHTML:
                pProp->Value <<= u"HTML"_ustr;
                break;
            case ooo::vba::word::WdSaveFormat::wdFormatRTF:
                pProp->Value <<= u"Rich Text Format"_ustr;
                break;
            case ooo::vba::word::WdSaveFormat::wdFormatTemplate:
                pProp->Value <<= u"MS Word 97 Vorlage"_ustr;
                break;

            // Default to "MS Word 97"
            default:
                pProp->Value <<= u"MS Word 97"_ustr;
                break;
        }
        return true;
    }
    return false;
}
}

#endif
