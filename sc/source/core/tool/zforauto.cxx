/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <vcl/svapp.hxx>
#include <tools/debug.hxx>

#include "zforauto.hxx"
#include "global.hxx"

static const sal_Char pStandardName[] = "Standard";

//------------------------------------------------------------------------

ScNumFormatAbbrev::ScNumFormatAbbrev() :
    sFormatstring   ( RTL_CONSTASCII_USTRINGPARAM( pStandardName ) ),
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
    rStream.ReadByteString( sFormatstring, eByteStrSet );
    rStream >> nSysLang >> nLang;
    eLnge = (LanguageType) nLang;
    eSysLnge = (LanguageType) nSysLang;
    if ( eSysLnge == LANGUAGE_SYSTEM )          // old versions did write it
        eSysLnge = Application::GetSettings().GetLanguage();
}

void ScNumFormatAbbrev::Save( SvStream& rStream, CharSet eByteStrSet ) const
{
    rStream.WriteByteString( sFormatstring, eByteStrSet );
    rStream << (sal_uInt16) eSysLnge << (sal_uInt16) eLnge;
}

void ScNumFormatAbbrev::PutFormatIndex(sal_uLong nFormat,
                                       SvNumberFormatter& rFormatter)
{
    const SvNumberformat* pFormat = rFormatter.GetEntry(nFormat);
    if (pFormat)
    {
        eSysLnge = Application::GetSettings().GetLanguage();
        eLnge = pFormat->GetLanguage();
        sFormatstring = ((SvNumberformat*)pFormat)->GetFormatstring();
    }
    else
    {
        OSL_FAIL("SCNumFormatAbbrev:: unbekanntes Zahlformat");
        eLnge = LANGUAGE_SYSTEM;
        eSysLnge = LANGUAGE_GERMAN;     // sonst passt "Standard" nicht
        sFormatstring.AssignAscii( RTL_CONSTASCII_STRINGPARAM( pStandardName ) );
    }
}

sal_uLong ScNumFormatAbbrev::GetFormatIndex( SvNumberFormatter& rFormatter)
{
    short nType;
    sal_Bool bNewInserted;
    xub_StrLen nCheckPos;
    return rFormatter.GetIndexPuttingAndConverting( sFormatstring, eLnge,
            eSysLnge, nType, bNewInserted, nCheckPos);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
