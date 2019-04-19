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
    bool bRes = false;
    for (sal_Int32 index = 0; index < rProps.getLength(); ++index)
    {
        if (rProps[index].Name == "FilterName")
        {
            switch (nFormat)
            {
                case ooo::vba::word::WdSaveFormat::wdFormatDocument:
                    rProps[index].Value <<= OUString("MS Word 97");
                    break;
                // Just save all the text formats as "Text"
                case ooo::vba::word::WdSaveFormat::wdFormatDOSText:
                case ooo::vba::word::WdSaveFormat::wdFormatDOSTextLineBreaks:
                case ooo::vba::word::WdSaveFormat::wdFormatEncodedText:
                case ooo::vba::word::WdSaveFormat::wdFormatText:
                case ooo::vba::word::WdSaveFormat::wdFormatTextLineBreaks:
                    rProps[index].Value <<= OUString("Text");
                    break;
                case ooo::vba::word::WdSaveFormat::wdFormatFilteredHTML:
                case ooo::vba::word::WdSaveFormat::wdFormatHTML:
                    rProps[index].Value <<= OUString("HTML");
                    break;
                case ooo::vba::word::WdSaveFormat::wdFormatRTF:
                    rProps[index].Value <<= OUString("Rich Text Format");
                    break;
                case ooo::vba::word::WdSaveFormat::wdFormatTemplate:
                    rProps[index].Value <<= OUString("MS Word 97 Vorlage");
                    break;

                // Default to "MS Word 97"
                default:
                    rProps[index].Value <<= OUString("MS Word 97");
                    break;
            }
            bRes = true;
            break;
        }
    }
    return bRes;
}
}

#endif
