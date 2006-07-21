/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: zforauto.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 11:46:22 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

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
    short nType;
    BOOL bNewInserted;
    xub_StrLen nCheckPos;
    return rFormatter.GetIndexPuttingAndConverting( sFormatstring, eLnge,
            eSysLnge, nType, bNewInserted, nCheckPos);
}
