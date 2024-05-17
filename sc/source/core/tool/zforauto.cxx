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

#include <svl/numformat.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <osl/diagnose.h>

#include <zforauto.hxx>
#include <tools/stream.hxx>

ScNumFormatAbbrev::ScNumFormatAbbrev() :
    sFormatstring   ( u"Standard"_ustr ),
    eLanguage       (LANGUAGE_SYSTEM),
    eSysLanguage    (LANGUAGE_GERMAN)       // otherwise "Standard" does not fit
{
}

ScNumFormatAbbrev::ScNumFormatAbbrev(sal_uInt32 nFormat,
                                     const SvNumberFormatter& rFormatter)
{
    PutFormatIndex(nFormat, rFormatter);
}

void ScNumFormatAbbrev::Load( SvStream& rStream, rtl_TextEncoding eByteStrSet )
{
    sal_uInt16 nSysLang, nLang;
    sFormatstring = rStream.ReadUniOrByteString( eByteStrSet );
    rStream.ReadUInt16( nSysLang ).ReadUInt16( nLang );
    eLanguage = LanguageType(nLang);
    eSysLanguage = LanguageType(nSysLang);
    if ( eSysLanguage == LANGUAGE_SYSTEM )          // old versions did write it
        eSysLanguage = Application::GetSettings().GetLanguageTag().getLanguageType();
}

void ScNumFormatAbbrev::Save( SvStream& rStream, rtl_TextEncoding eByteStrSet ) const
{
    rStream.WriteUniOrByteString( sFormatstring, eByteStrSet );
    rStream.WriteUInt16( static_cast<sal_uInt16>(eSysLanguage) ).WriteUInt16( static_cast<sal_uInt16>(eLanguage) );
}

void ScNumFormatAbbrev::PutFormatIndex(sal_uInt32 nFormat,
                                       const SvNumberFormatter& rFormatter)
{
    const SvNumberformat* pFormat = rFormatter.GetEntry(nFormat);
    if (pFormat)
    {
        eSysLanguage = Application::GetSettings().GetLanguageTag().getLanguageType();
        eLanguage = pFormat->GetLanguage();
        sFormatstring = pFormat->GetFormatstring();
    }
    else
    {
        OSL_FAIL("SCNumFormatAbbrev:: unknown number format");
        eLanguage = LANGUAGE_SYSTEM;
        eSysLanguage = LANGUAGE_GERMAN;     // otherwise "Standard" does not fit
        sFormatstring = "Standard";
    }
}

sal_uInt32 ScNumFormatAbbrev::GetFormatIndex( SvNumberFormatter& rFormatter)
{
    SvNumFormatType nType;
    bool bNewInserted;
    sal_Int32 nCheckPos;
    return rFormatter.GetIndexPuttingAndConverting( sFormatstring, eLanguage,
                                                    eSysLanguage, nType, bNewInserted, nCheckPos);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
