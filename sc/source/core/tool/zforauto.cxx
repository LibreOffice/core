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

#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <vcl/svapp.hxx>

#include "zforauto.hxx"
#include "global.hxx"

ScNumFormatAbbrev::ScNumFormatAbbrev() :
    sFormatstring   ( "Standard" ),
    eLnge           (LANGUAGE_SYSTEM),
    eSysLnge        (LANGUAGE_GERMAN)       // sonst passt "Standard" nicht
{
}

ScNumFormatAbbrev::ScNumFormatAbbrev(const ScNumFormatAbbrev& aFormat) :
    sFormatstring   (aFormat.sFormatstring),
    eLnge           (aFormat.eLnge),
    eSysLnge        (aFormat.eSysLnge)
{
}

ScNumFormatAbbrev::ScNumFormatAbbrev(sal_uLong nFormat,
                                     SvNumberFormatter& rFormatter)
{
    PutFormatIndex(nFormat, rFormatter);
}

void ScNumFormatAbbrev::Load( SvStream& rStream, CharSet eByteStrSet )
{
    sal_uInt16 nSysLang, nLang;
    sFormatstring = rStream.ReadUniOrByteString( eByteStrSet );
    rStream >> nSysLang >> nLang;
    eLnge = (LanguageType) nLang;
    eSysLnge = (LanguageType) nSysLang;
    if ( eSysLnge == LANGUAGE_SYSTEM )          // old versions did write it
        eSysLnge = Application::GetSettings().GetLanguageTag().getLanguageType();
}

void ScNumFormatAbbrev::Save( SvStream& rStream, CharSet eByteStrSet ) const
{
    rStream.WriteUniOrByteString( sFormatstring, eByteStrSet );
    rStream << (sal_uInt16) eSysLnge << (sal_uInt16) eLnge;
}

void ScNumFormatAbbrev::PutFormatIndex(sal_uLong nFormat,
                                       SvNumberFormatter& rFormatter)
{
    const SvNumberformat* pFormat = rFormatter.GetEntry(nFormat);
    if (pFormat)
    {
        eSysLnge = Application::GetSettings().GetLanguageTag().getLanguageType();
        eLnge = pFormat->GetLanguage();
        sFormatstring = ((SvNumberformat*)pFormat)->GetFormatstring();
    }
    else
    {
        OSL_FAIL("SCNumFormatAbbrev:: unknown number format");
        eLnge = LANGUAGE_SYSTEM;
        eSysLnge = LANGUAGE_GERMAN;     // sonst passt "Standard" nicht
        sFormatstring = "Standard";
    }
}

sal_uLong ScNumFormatAbbrev::GetFormatIndex( SvNumberFormatter& rFormatter)
{
    short nType;
    bool bNewInserted;
    sal_Int32 nCheckPos;
    return rFormatter.GetIndexPuttingAndConverting( sFormatstring, eLnge,
                                                    eSysLnge, nType, bNewInserted, nCheckPos);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
