/*************************************************************************
 *
 *  $RCSfile: zforauto.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:19 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#include <svtools/zforlist.hxx>
#include <svtools/zformat.hxx>
#include <vcl/system.hxx>
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

ScNumFormatAbbrev::ScNumFormatAbbrev(SvStream& rStream)
{
    Load(rStream);
}

ScNumFormatAbbrev::ScNumFormatAbbrev(ULONG nFormat,
                                     SvNumberFormatter& rFormatter)
{
    PutFormatIndex(nFormat, rFormatter);
}

void ScNumFormatAbbrev::Load( SvStream& rStream )
{
    USHORT nSysLang, nLang;
    rStream.ReadByteString( sFormatstring, rStream.GetStreamCharSet() );
    rStream >> nSysLang >> nLang;
    eLnge = (LanguageType) nLang;
    eSysLnge = (LanguageType) nSysLang;
    if ( eSysLnge == LANGUAGE_SYSTEM )          // von alten Versionen
        eSysLnge = System::GetLanguage();
}

void ScNumFormatAbbrev::Save( SvStream& rStream ) const
{
    rStream.WriteByteString( sFormatstring, rStream.GetStreamCharSet() );
    rStream << (USHORT) eSysLnge << (USHORT) eLnge;
}

void ScNumFormatAbbrev::PutFormatIndex(ULONG nFormat,
                                       SvNumberFormatter& rFormatter)
{
    const SvNumberformat* pFormat = rFormatter.GetEntry(nFormat);
    if (pFormat)
    {
        eSysLnge = System::GetLanguage();
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

    if ( eLnge == LANGUAGE_SYSTEM && eSysLnge != System::GetLanguage() )
    {
        ULONG nOrig = rFormatter.GetEntryKey( sFormatstring, eSysLnge );
        if ( nOrig != NUMBERFORMAT_ENTRY_NOT_FOUND )
            return rFormatter.GetFormatForLanguageIfBuiltIn( nOrig, System::GetLanguage() );
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





