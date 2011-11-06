/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <vcl/svapp.hxx>
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
        DBG_ERROR("SCNumFormatAbbrev:: unbekanntes Zahlformat");
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
