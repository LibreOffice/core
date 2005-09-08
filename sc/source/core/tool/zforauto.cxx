/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: zforauto.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 18:52:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#include <svtools/zforlist.hxx>
#include <svtools/zformat.hxx>
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#include <tools/debug.hxx>

#include "zforauto.hxx"
#include "global.hxx"

static const sal_Char __FAR_DATA pStandardName[] = "Standard";

//------------------------------------------------------------------------

ScNumFormatAbbrev::ScNumFormatAbbrev() :
    eLnge           (LANGUAGE_SYSTEM),
    eSysLnge        (LANGUAGE_GERMAN),      // sonst passt "Standard" nicht
    sFormatstring   ( RTL_CONSTASCII_USTRINGPARAM( pStandardName ) )
{
}

ScNumFormatAbbrev::ScNumFormatAbbrev(const ScNumFormatAbbrev& aFormat) :
    eLnge           (aFormat.eLnge),
    eSysLnge        (aFormat.eSysLnge),
    sFormatstring   (aFormat.sFormatstring)
{
}

ScNumFormatAbbrev::ScNumFormatAbbrev(ULONG nFormat,
                                     SvNumberFormatter& rFormatter)
{
    PutFormatIndex(nFormat, rFormatter);
}

void ScNumFormatAbbrev::Load( SvStream& rStream, CharSet eByteStrSet )
{
    USHORT nSysLang, nLang;
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
    rStream << (USHORT) eSysLnge << (USHORT) eLnge;
}

void ScNumFormatAbbrev::PutFormatIndex(ULONG nFormat,
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
        DBG_ERROR("SCNumFormatAbbrev:: unbekanntes Zahlformat");
        eLnge = LANGUAGE_SYSTEM;
        eSysLnge = LANGUAGE_GERMAN;     // sonst passt "Standard" nicht
        sFormatstring.AssignAscii( RTL_CONSTASCII_STRINGPARAM( pStandardName ) );
    }
}

ULONG ScNumFormatAbbrev::GetFormatIndex( SvNumberFormatter& rFormatter)
{
    //  #62389# leerer Formatstring (vom Writer) -> Standardformat
    if ( !sFormatstring.Len() )
        return rFormatter.GetStandardIndex( eLnge );

    if ( eLnge == LANGUAGE_SYSTEM && eSysLnge != Application::GetSettings().GetLanguage() )
    {
        ULONG nOrig = rFormatter.GetEntryKey( sFormatstring, eSysLnge );
        if ( nOrig != NUMBERFORMAT_ENTRY_NOT_FOUND )
            return rFormatter.GetFormatForLanguageIfBuiltIn( nOrig, Application::GetSettings().GetLanguage() );
        else
            return rFormatter.GetStandardIndex( eLnge );    // geht nicht -> Standard
    }

    xub_StrLen nCheckPos;
    short nType;
    ULONG nKey = rFormatter.GetEntryKey(sFormatstring, eLnge);
    if (nKey == NUMBERFORMAT_ENTRY_NOT_FOUND)
    {
        BOOL res = rFormatter.PutEntry(sFormatstring,
                                       nCheckPos, nType, nKey, eLnge);
        if (nCheckPos > 0)
            DBG_ERROR("SCNumFormatAbbrev:: unkorrekter Formatstring");
    }
    return nKey;
}





