/*************************************************************************
 *
 *  $RCSfile: zforlist.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:03 $
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

#pragma hdrstop

#include <math.h>

#ifndef _DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _INTN_HXX //autogen
#include <tools/intn.hxx>
#endif
#ifndef _SOUND_HXX //autogen
#include <vcl/sound.hxx>
#endif
#ifndef _SYSTEM_HXX //autogen
#include <vcl/system.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_SETTINGS_HXX //autogen
#include <vcl/settings.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif

#define _SVSTDARR_USHORTS
#include "svstdarr.hxx"

#define _ZFORLIST_CXX
#include "zforlist.hxx"
#undef _ZFORLIST_CXX

#include "zforscan.hxx"
#include "zforfind.hxx"
#include "zformat.hxx"
#include "numhead.hxx"


        // Konstanten fuer Typoffsets pro Land/Sprache (CL)
#define ZF_STANDARD              0
#define ZF_STANDARD_PERCENT     10
#define ZF_STANDARD_CURRENCY    20
#define ZF_STANDARD_DATE        30
#define ZF_STANDARD_TIME        40
#define ZF_STANDARD_DATETIME    50
#define ZF_STANDARD_SCIENTIFIC  60
#define ZF_STANDARD_FRACTION    70
#define ZF_STANDARD_NEWEXTENDED 75
#define ZF_STANDARD_NEWEXTENDEDMAX  SV_MAX_ANZ_STANDARD_FORMATE-2   // 98
#define ZF_STANDARD_LOGICAL     SV_MAX_ANZ_STANDARD_FORMATE-1 //  99
#define ZF_STANDARD_TEXT        SV_MAX_ANZ_STANDARD_FORMATE   // 100

//  Sprache, die am International gesetzt wird, wenn eine unbekannte Sprache
//  (von einem anderen System) geladen wird. Darf nicht SYSTEM sein, weil
//  "DM" aus deutschen Einstellungen sonst als Datum erkannt wird (#53155#).

#define UNKNOWN_SUBSTITUTE      LANGUAGE_ENGLISH_US

static BOOL bIndexTableInitialized = FALSE;
static ULONG __FAR_DATA theIndexTable[NF_INDEX_TABLE_ENTRIES];

BOOL SvNumberFormatter::bCurrencyTableInitialized = FALSE;
NfCurrencyTable SvNumberFormatter::theCurrencyTable;
USHORT SvNumberFormatter::nSystemCurrencyPosition = 0;
SV_IMPL_PTRARR( NfCurrencyTable, NfCurrencyEntry* );
const USHORT nCurrencyTableEuroPosition = 1;
SV_IMPL_PTRARR( NfWSStringsDtor, XubString* );

// ob das BankSymbol immer am Ende ist (1 $;-1 $) oder sprachabhaengig
#define NF_BANKSYMBOL_FIX_POSITION 1

/***********************Funktionen SvNumberFormatter**************************/

SvNumberFormatter::SvNumberFormatter(LanguageType eLnge)
{
//! if (eLnge == LANGUAGE_SYSTEM)               // Sprache Systemn uebersteuern
//!     eLnge = System::GetLanguage();
    if (eLnge == LANGUAGE_DONTKNOW)
        eLnge = LANGUAGE_ENGLISH_US;
    SysLnge = eLnge;
    ActLnge = eLnge;
    eEvalDateFormat = NF_EVALDATEFORMAT_INTL;
    nDefaultCurrencyFormat = NUMBERFORMAT_ENTRY_NOT_FOUND;
    if ( !International::IsFormatAvailable( eLnge ) )
        eLnge = UNKNOWN_SUBSTITUTE;
    pIntl = new International( eLnge );
    String aLanguage, aCountry, aVariant;
    ConvertLanguageToIsoNames( International::GetRealLanguage( eLnge ), aLanguage, aCountry );
    pCharClass = new CharClass( ::com::sun::star::lang::Locale( aLanguage, aCountry, aVariant ) );
    pStringScanner = new ImpSvNumberInputScan( this );
    pFormatScanner = new ImpSvNumberformatScan( this );
    pFormatTable = NULL;
    ImpGenerateFormats(0);                      // 0 .. 999 fuer SystemEinst.
    MaxCLOffset = 0;
    pMergeTable = new SvULONGTable;
    bNoZero = FALSE;
    pColorLink = NULL;
}

SvNumberFormatter::~SvNumberFormatter()
{
    SvNumberformat* pEntry = aFTable.First();
    while (pEntry)
    {
        delete pEntry;
        pEntry = aFTable.Next();
    }
    delete pCharClass;
    delete pIntl;
    delete pStringScanner;
    delete pFormatScanner;
    ClearMergeTable();
    delete pMergeTable;
}

Color* SvNumberFormatter::GetUserDefColor(USHORT nIndex)
{
    if( pColorLink && pColorLink->IsSet() )
        return (Color*) ( pColorLink->Call( (void*) &nIndex ));
    else
        return NULL;
}

void SvNumberFormatter::ChangeNullDate(USHORT nDay,
                                       USHORT nMonth,
                                       USHORT nYear)
{
    pFormatScanner->ChangeNullDate(nDay, nMonth, nYear);
    pStringScanner->ChangeNullDate(nDay, nMonth, nYear);
}

Date* SvNumberFormatter::GetNullDate()
{
    return pFormatScanner->GetNullDate();
}

void SvNumberFormatter::ChangeStandardPrec(short nPrec)
{
    pFormatScanner->ChangeStandardPrec(nPrec);
}

short SvNumberFormatter::GetStandardPrec()
{
    return pFormatScanner->GetStandardPrec();
}

void SvNumberFormatter::ImpChangeSysCL(LanguageType eLnge)
{
//! if (eLnge == LANGUAGE_SYSTEM)               // Sprache Systemn uebersteuern
//!     eLnge = System::GetLanguage();
    if (eLnge == LANGUAGE_DONTKNOW)
        eLnge = LANGUAGE_ENGLISH_US;
    if (eLnge != SysLnge)
    {
        SysLnge = eLnge;
        ChangeIntl(eLnge);
        SvNumberformat* pEntry = aFTable.First();
        while (pEntry)                          // alte Formate loeschen
        {
            pEntry = (SvNumberformat*) aFTable.Remove(aFTable.GetCurKey());
            delete pEntry;
            pEntry = (SvNumberformat*) aFTable.First();
        }
        ImpGenerateFormats(0);                      // wieder neue Standardformate
    }
}

void SvNumberFormatter::ChangeIntl(LanguageType eLnge)
{
//! if (eLnge == LANGUAGE_SYSTEM)               // Sprache Systemn uebersteuern
//!     eLnge = System::GetLanguage();
    if (ActLnge != eLnge)
    {
        delete pIntl;
        ActLnge = eLnge;
        if ( !International::IsFormatAvailable( eLnge ) )
            eLnge = UNKNOWN_SUBSTITUTE;
        pIntl = new International( eLnge );

        String aLanguage, aCountry, aVariant;
        ConvertLanguageToIsoNames( International::GetRealLanguage( eLnge ), aLanguage, aCountry );
        pCharClass->setLocale( ::com::sun::star::lang::Locale( aLanguage, aCountry, aVariant ) );

        pFormatScanner->ChangeIntl();
        pStringScanner->ChangeIntl();
    }
}

BOOL SvNumberFormatter::IsTextFormat(ULONG F_Index) const
{
    SvNumberformat* pFormat = (SvNumberformat*) aFTable.Get(F_Index);
    if (!pFormat)
        return FALSE;
    else
        return pFormat->IsTextFormat();
}

BOOL SvNumberFormatter::HasTextFormat(ULONG F_Index) const
{
    SvNumberformat* pFormat = (SvNumberformat*) aFTable.Get(F_Index);
    if (!pFormat)
        return FALSE;
    else
        return pFormat->HasTextFormat();
}

BOOL SvNumberFormatter::PutEntry(XubString& rString,
                                 xub_StrLen& nCheckPos,
                                 short& nType,
                                 ULONG& nKey,           // Formatnummer
                                 LanguageType eLnge)
{
    nKey = 0;
    if (rString.Len() == 0)                             // keinen Leerstring
    {
        nCheckPos = 1;                                  // -> Fehler
        return FALSE;
    }
    if (eLnge == LANGUAGE_DONTKNOW)
        eLnge = SysLnge;

    ChangeIntl(eLnge);                                  // ggfs. austauschen
    LanguageType eLge = eLnge;                          // Umgehung const fuer ConvertMode
    BOOL bCheck = FALSE;
    SvNumberformat* p_Entry = new SvNumberformat(rString,
                                                 pFormatScanner,
                                                 pStringScanner,
                                                 nCheckPos,
                                                 eLge);
    if (nCheckPos == 0)                         // Format ok
    {                                           // Typvergleich:
        short eCheckType = p_Entry->GetType();
        if ( eCheckType != NUMBERFORMAT_UNDEFINED)
        {
            p_Entry->SetType(eCheckType | NUMBERFORMAT_DEFINED);
            nType = eCheckType;
        }
        else
        {
            p_Entry->SetType(NUMBERFORMAT_DEFINED);
            nType = NUMBERFORMAT_DEFINED;
        }
        ULONG CLOffset = ImpGenerateCL(eLge);               // ggfs. neu Standard-
                                                        // formate anlegen
        nKey = ImpIsEntry(p_Entry->GetFormatstring(),CLOffset, eLge);
        if (nKey != NUMBERFORMAT_ENTRY_NOT_FOUND)               // schon vorhanden
            delete p_Entry;
        else
        {
            bCheck = TRUE;
            SvNumberformat* pStdFormat =
                     (SvNumberformat*) aFTable.Get(CLOffset + ZF_STANDARD);
            ULONG nPos = CLOffset + pStdFormat->GetLastInsertKey();
//          ULONG nPos = ImpGetLastCLEntryKey(CLOffset, eLge);
            if (nPos - CLOffset >= SV_COUNTRY_LANGUAGE_OFFSET)
            {
#ifndef DOS
                Sound::Beep();
#endif
                DBG_ERROR("SvNumberFormatter:: Zu viele Formate pro CL");
                delete p_Entry;
            }
            else if (!aFTable.Insert(nPos+1,p_Entry))
                delete p_Entry;
            else
            {
                nKey = nPos+1;
                pStdFormat->SetLastInsertKey((USHORT) (nKey-CLOffset));
            }
        }
    }
    else
        delete p_Entry;
    return bCheck;
}

BOOL SvNumberFormatter::PutandConvertEntry(XubString& rString,
                                           xub_StrLen& nCheckPos,
                                           short& nType,
                                           ULONG& nKey,
                                           LanguageType eLnge,
                                           LanguageType eNewLnge)
{
    BOOL bRes;
//! if (eNewLnge == LANGUAGE_SYSTEM)                // Sprache Systemn uebersteuern
//!     eNewLnge = System::GetLanguage();
    if (eNewLnge == LANGUAGE_DONTKNOW)
        eNewLnge = LANGUAGE_ENGLISH_US;

    pFormatScanner->SetConvertMode(eLnge, eNewLnge);
    bRes = PutEntry(rString, nCheckPos, nType, nKey, eLnge);
    pFormatScanner->SetConvertMode(FALSE);
    return bRes;
}

void SvNumberFormatter::DeleteEntry(ULONG nKey)
{
    SvNumberformat* pEntry = aFTable.Remove(nKey);
    delete pEntry;
}

void SvNumberFormatter::PrepareSave()
{
     SvNumberformat* pFormat = aFTable.First();
     while (pFormat)
     {
        pFormat->SetUsed(FALSE);
        pFormat = aFTable.Next();
     }
}

void SvNumberFormatter::SetFormatUsed(ULONG nFIndex)
{
    SvNumberformat* pFormat = (SvNumberformat*) aFTable.Get(nFIndex);
    if (pFormat)
        pFormat->SetUsed(TRUE);
}

BOOL SvNumberFormatter::Load( SvStream& rStream )
{
    LanguageType eSysLang = System::GetLanguage();
    SvNumberFormatter* pConverter = NULL;

    ImpSvNumMultipleReadHeader aHdr( rStream );
    USHORT nVersion;
    rStream >> nVersion;
    SvNumberformat* pEntry;
    ULONG nPos;
    LanguageType eSaveSysLang, eLoadSysLang;
    USHORT nSysOnStore, eLge, eDummy;           // Dummy fuer kompatibles Format
    rStream >> nSysOnStore >> eLge;             // Systemeinstellung aus
                                                // Dokument
    eSaveSysLang = (nVersion < SV_NUMBERFORMATTER_VERSION_SYSTORE ?
        LANGUAGE_SYSTEM : (LanguageType) nSysOnStore);
    LanguageType eLnge = (LanguageType) eLge;
    ImpChangeSysCL(eLnge);

    rStream >> nPos;
    while (nPos != NUMBERFORMAT_ENTRY_NOT_FOUND)
    {
        rStream >> eDummy >> eLge;
        eLnge = (LanguageType) eLge;
        ImpGenerateCL(eLnge);           // ggfs. neue Standardformate anlegen

        ULONG nOffset = nPos % SV_COUNTRY_LANGUAGE_OFFSET;      // relativIndex
        BOOL bUserDefined = (nOffset > SV_MAX_ANZ_STANDARD_FORMATE);
        //! HACK! ER 29.07.97 15:15
        // SaveLang wurde bei SYSTEM nicht gespeichert sondern war auch SYSTEM,
        // erst ab 364i Unterscheidung moeglich
        BOOL bConversionHack;
        if ( eLnge == LANGUAGE_SYSTEM )
        {
            if ( nVersion < SV_NUMBERFORMATTER_VERSION_SYSTORE )
            {
                bConversionHack = bUserDefined;
                eLoadSysLang = eSaveSysLang;
            }
            else
            {
                bConversionHack = FALSE;
                eLoadSysLang = eSysLang;
            }
        }
        else
        {
            bConversionHack = FALSE;
            eLoadSysLang = eSaveSysLang;
        }

        pEntry = new SvNumberformat(*pFormatScanner, eLnge);
        if ( bConversionHack )
        {   // SYSTEM
            // nVersion < SV_NUMBERFORMATTER_VERSION_SYSTORE
            // nVersion < SV_NUMBERFORMATTER_VERSION_KEYWORDS
            if ( !pConverter )
                pConverter = new SvNumberFormatter( eSysLang );
            NfHackConversion eHackConversion = pEntry->Load(
                rStream, aHdr, pConverter, *pStringScanner );
            switch ( eHackConversion )
            {
                case NF_CONVERT_GERMAN_ENGLISH :
                    pEntry->ConvertLanguage( *pConverter,
                        LANGUAGE_ENGLISH_US, eSysLang, TRUE );
                break;
                case NF_CONVERT_ENGLISH_GERMAN :
                    switch ( eSysLang )
                    {
                        case LANGUAGE_GERMAN:
                        case LANGUAGE_GERMAN_SWISS:
                        case LANGUAGE_GERMAN_AUSTRIAN:
                        case LANGUAGE_GERMAN_LUXEMBOURG:
                        case LANGUAGE_GERMAN_LIECHTENSTEIN:
                            // alles beim alten
                        break;
                        default:
                            pEntry->ConvertLanguage( *pConverter,
                                LANGUAGE_GERMAN, eSysLang, TRUE );
                    }
                break;
            }

        }
        else
        {
            pEntry->Load( rStream, aHdr, NULL, *pStringScanner );
            if ( !bUserDefined )
                bUserDefined = (pEntry->GetNewStandardDefined() > SV_NUMBERFORMATTER_VERSION);
            if ( bUserDefined )
            {
                if ( eSaveSysLang != eLoadSysLang )
                {   // SYSTEM verschieden
                    if ( !pConverter )
                        pConverter = new SvNumberFormatter( eSysLang );
                    if ( nVersion < SV_NUMBERFORMATTER_VERSION_KEYWORDS )
                    {
                        switch ( eSaveSysLang )
                        {
                            case LANGUAGE_GERMAN:
                            case LANGUAGE_GERMAN_SWISS:
                            case LANGUAGE_GERMAN_AUSTRIAN:
                            case LANGUAGE_GERMAN_LUXEMBOURG:
                            case LANGUAGE_GERMAN_LIECHTENSTEIN:
                                // alles beim alten
                                pEntry->ConvertLanguage( *pConverter,
                                    eSaveSysLang, eLoadSysLang, TRUE );
                            break;
                            default:
                                // alte english nach neuem anderen
                                pEntry->ConvertLanguage( *pConverter,
                                    LANGUAGE_ENGLISH_US, eLoadSysLang, TRUE );
                        }
                    }
                    else
                        pEntry->ConvertLanguage( *pConverter,
                            eSaveSysLang, eLoadSysLang, TRUE );
                }
                else
                {   // nicht SYSTEM oder gleiches SYSTEM
                    if ( nVersion < SV_NUMBERFORMATTER_VERSION_KEYWORDS )
                    {
                        LanguageType eLoadLang;
                        BOOL bSystem;
                        if ( eLnge == LANGUAGE_SYSTEM )
                        {
                            eLoadLang = eSysLang;
                            bSystem = TRUE;
                        }
                        else
                        {
                            eLoadLang = eLnge;
                            bSystem = FALSE;
                        }
                        switch ( eLoadLang )
                        {
                            case LANGUAGE_GERMAN:
                            case LANGUAGE_GERMAN_SWISS:
                            case LANGUAGE_GERMAN_AUSTRIAN:
                            case LANGUAGE_GERMAN_LUXEMBOURG:
                            case LANGUAGE_GERMAN_LIECHTENSTEIN:
                                // alles beim alten
                            break;
                            default:
                                // alte english nach neuem anderen
                                if ( !pConverter )
                                    pConverter = new SvNumberFormatter( eSysLang );
                                pEntry->ConvertLanguage( *pConverter,
                                    LANGUAGE_ENGLISH_US, eLoadLang, bSystem );
                        }
                    }
                }
            }
        }
        if ( nOffset == 0 )     // StandardFormat
        {
            SvNumberformat* pEnt = aFTable.Get(nPos);
            if (pEnt)
                pEnt->SetLastInsertKey(pEntry->GetLastInsertKey());
        }
        if (!aFTable.Insert(nPos, pEntry))
            delete pEntry;
        rStream >> nPos;
    }

    // ab SV_NUMBERFORMATTER_VERSION_YEAR2000
    if ( nVersion >= SV_NUMBERFORMATTER_VERSION_YEAR2000 )
    {
        aHdr.StartEntry();
        if ( aHdr.BytesLeft() >= sizeof(UINT16) )
        {
            UINT16 nY2k;
            rStream >> nY2k;
            if ( nVersion < SV_NUMBERFORMATTER_VERSION_TWODIGITYEAR && nY2k < 100 )
                nY2k += 1901;       // war vor src513e: 29, jetzt: 1930
            SetYear2000( nY2k );
        }
        aHdr.EndEntry();
    }

    if ( pConverter )
        delete pConverter;
    if (rStream.GetError())
        return FALSE;
    else
        return TRUE;
}

BOOL SvNumberFormatter::Save( SvStream& rStream ) const
{
    ImpSvNumMultipleWriteHeader aHdr( rStream );
    // ab 364i wird gespeichert was SYSTEM wirklich war, vorher hart LANGUAGE_SYSTEM
    rStream << (USHORT) SV_NUMBERFORMATTER_VERSION;
    rStream << (USHORT) System::GetLanguage() << (USHORT) SysLnge;
    SvNumberFormatTable* pTable = (SvNumberFormatTable*) &aFTable;
    SvNumberformat* pEntry = (SvNumberformat*) pTable->First();
    while (pEntry)
    {
        // Gespeichert werden alle markierten, benutzerdefinierten Formate und
        // jeweils das Standardformat zu allen angewaehlten CL-Kombinationen
        // sowie NewStandardDefined
        if ( pEntry->GetUsed() || (pEntry->GetType() & NUMBERFORMAT_DEFINED) ||
                pEntry->GetNewStandardDefined() ||
                (pTable->GetCurKey() % SV_COUNTRY_LANGUAGE_OFFSET == 0) )
        {
            rStream << pTable->GetCurKey()
                    << (USHORT) LANGUAGE_SYSTEM
                    << (USHORT) pEntry->GetLanguage();
            pEntry->Save(rStream, aHdr);
        }
        pEntry = (SvNumberformat*) pTable->Next();
    }
    rStream << NUMBERFORMAT_ENTRY_NOT_FOUND;                // EndeKennung

    // ab SV_NUMBERFORMATTER_VERSION_YEAR2000
    aHdr.StartEntry();
    rStream << (UINT16) GetYear2000();
    aHdr.EndEntry();

    if (rStream.GetError())
        return FALSE;
    else
        return TRUE;
}

// static
void SvNumberFormatter::SkipNumberFormatterInStream( SvStream& rStream )
{
    ImpSvNumMultipleReadHeader::Skip( rStream );
}

void SvNumberFormatter::GetUsedLanguages( SvUShorts& rList )
{
    rList.Remove( 0, rList.Count() );

    ULONG nOffset = 0;
    while (nOffset <= MaxCLOffset)
    {
        SvNumberformat* pFormat = (SvNumberformat*) aFTable.Get(nOffset);
        if (pFormat)
            rList.Insert( pFormat->GetLanguage(), rList.Count() );
        nOffset += SV_COUNTRY_LANGUAGE_OFFSET;
    }
}

XubString SvNumberFormatter::GetKeyword( LanguageType eLnge, USHORT nIndex )
{
    ChangeIntl(eLnge);
    const String* pTable = pFormatScanner->GetKeyword();
    if ( pTable && nIndex < NF_KEYWORD_ENTRIES_COUNT )
        return pTable[nIndex];

    DBG_ERROR("GetKeyword: invalid index");
    return XubString();
}

ULONG SvNumberFormatter::ImpGetCLOffset(LanguageType eLnge) const
{
    SvNumberformat* pFormat;
    ULONG nOffset = 0;
    while (nOffset <= MaxCLOffset)
    {
        pFormat = (SvNumberformat*) aFTable.Get(nOffset);
        if (pFormat && pFormat->GetLanguage() == eLnge)
            return nOffset;
        nOffset += SV_COUNTRY_LANGUAGE_OFFSET;
    }
    return nOffset;
}

ULONG SvNumberFormatter::ImpIsEntry(const XubString& rString,
                                       ULONG nCLOffset,
                                       LanguageType eLnge)
{
#ifndef NF_COMMENT_IN_FORMATSTRING
#error NF_COMMENT_IN_FORMATSTRING not defined (zformat.hxx)
#endif
#if NF_COMMENT_IN_FORMATSTRING
    XubString aStr( rString );
    SvNumberformat::EraseComment( aStr );
#endif
    ULONG res = NUMBERFORMAT_ENTRY_NOT_FOUND;
    SvNumberformat* pEntry;
    pEntry = (SvNumberformat*) aFTable.Seek(nCLOffset);
    while ( res == NUMBERFORMAT_ENTRY_NOT_FOUND &&
            pEntry && pEntry->GetLanguage() == eLnge )
    {
#if NF_COMMENT_IN_FORMATSTRING
        if ( pEntry->GetComment().Len() )
        {
            XubString aFormat( pEntry->GetFormatstring() );
            SvNumberformat::EraseComment( aFormat );
            if ( aStr == aFormat )
                res = aFTable.GetCurKey();
            else
                pEntry = (SvNumberformat*) aFTable.Next();
        }
        else
        {
            if ( aStr == pEntry->GetFormatstring() )
                res = aFTable.GetCurKey();
            else
                pEntry = (SvNumberformat*) aFTable.Next();
        }
#else
        if ( rString == pEntry->GetFormatstring() )
            res = aFTable.GetCurKey();
        else
            pEntry = (SvNumberformat*) aFTable.Next();
#endif
    }
    return res;
}

/*
ULONG SvNumberFormatter::ImpGetLastCLEntryKey(ULONG nCLOffset,
                                           LanguageType eLnge)
{
        // Diese Funktion ist ueberfluesssig geworden 3.3.95
    ULONG res = 0;
    SvNumberformat* pEntry;
    pEntry = (SvNumberformat*) aFTable.Seek(nCLOffset);
    while (pEntry && pEntry->GetLanguage() == eLnge)
    {
        res = aFTable.GetCurKey();
        pEntry = (SvNumberformat*) aFTable.Next();
    }
    if (res < nCLOffset + SV_MAX_ANZ_STANDARD_FORMATE) // nie bei den Standard-
        res = nCLOffset + SV_MAX_ANZ_STANDARD_FORMATE; // formaten
    return res;
}
*/

SvNumberFormatTable& SvNumberFormatter::GetFirstEntryTable(
                                                      short& eType,
                                                      ULONG& FIndex,
                                                      LanguageType& rLnge)
{
    short eTypetmp = eType;
    if (eType == NUMBERFORMAT_ALL)                  // Leere Zelle oder don't care
        rLnge = SysLnge;
    else
    {
        SvNumberformat* pFormat = (SvNumberformat*) aFTable.Get(FIndex);
        if (!pFormat)
        {
//          DBG_ERROR("SvNumberFormatter:: Unbekanntes altes Zahlformat (1)");
            rLnge = SysLnge;
            eType = NUMBERFORMAT_ALL;
            eTypetmp = eType;
        }
        else
        {
            rLnge = pFormat->GetLanguage();
            eType = pFormat->GetType()&~NUMBERFORMAT_DEFINED;
            if (eType == 0)
            {
                eType = NUMBERFORMAT_DEFINED;
                eTypetmp = eType;
            }
            else if (eType == NUMBERFORMAT_DATETIME)
            {
                eTypetmp = eType;
                eType = NUMBERFORMAT_DATE;
            }
            else
                eTypetmp = eType;
        }
    }
    ChangeIntl(rLnge);
    return GetEntryTable(eTypetmp, FIndex, rLnge);
}

ULONG SvNumberFormatter::ImpGenerateCL(LanguageType eLnge)
{
    ChangeIntl(eLnge);
    ULONG CLOffset = ImpGetCLOffset(ActLnge);
    if (CLOffset > MaxCLOffset)                     // CL noch nicht da
    {
        MaxCLOffset += SV_COUNTRY_LANGUAGE_OFFSET;
        ImpGenerateFormats(MaxCLOffset);
        CLOffset = MaxCLOffset;
    }
    return CLOffset;
}

SvNumberFormatTable& SvNumberFormatter::ChangeCL(short eType,
                                                 ULONG& FIndex,
                                                 LanguageType eLnge)
{
    ImpGenerateCL(eLnge);
    return GetEntryTable(eType, FIndex, ActLnge);
}

SvNumberFormatTable& SvNumberFormatter::GetEntryTable(
                                                    short eType,
                                                    ULONG& FIndex,
                                                    LanguageType eLnge)
{
    delete pFormatTable;
    pFormatTable = new SvNumberFormatTable; // neuer Table
    ChangeIntl(eLnge);
    ULONG CLOffset = ImpGetCLOffset(ActLnge);
    SvNumberformat* pEntry;
    pEntry = (SvNumberformat*) aFTable.Seek(CLOffset);

    if (eType == NUMBERFORMAT_ALL)
    {
        while (pEntry && pEntry->GetLanguage() == ActLnge)
        {
            pFormatTable->Insert(aFTable.GetCurKey(),pEntry);   // einhaengen
            pEntry = (SvNumberformat*) aFTable.Next();
        }
    }
    else
    {
        while (pEntry && pEntry->GetLanguage() == ActLnge)
        {
            if ((pEntry->GetType()) & eType)                // von diesem Typ
                pFormatTable->Insert(aFTable.GetCurKey(),pEntry);// einhaengen
            pEntry = (SvNumberformat*) aFTable.Next();
        }
    }
    pEntry = aFTable.Get(FIndex);
    if (!pEntry || !(pEntry->GetType() & eType)             // irgendeine Veraenderung
                || pEntry->GetLanguage() != ActLnge )           // => StandardForm.
        if (pFormatTable->Count() > 0)                      // aber nicht leer
            FIndex = GetStandardFormat(eType, ActLnge);
    return *pFormatTable;
}

BOOL SvNumberFormatter::IsNumberFormat(const XubString& sString,
                                       ULONG& F_Index,
                                       double& fOutNumber)
{
    short FType;
    const SvNumberformat* pFormat = (SvNumberformat*) aFTable.Get(F_Index);
    if (!pFormat)
    {
//      DBG_ERROR("SvNumberFormatter:: Unbekanntes altes Zahlformat (2)");
        ChangeIntl(SysLnge);
        FType = NUMBERFORMAT_NUMBER;
    }
    else
    {
        FType = pFormat->GetType() &~NUMBERFORMAT_DEFINED;
        if (FType == 0)
            FType = NUMBERFORMAT_DEFINED;
        ChangeIntl(pFormat->GetLanguage());
    }
    BOOL res;
    short RType = FType;
                                                        // Ergebnistyp
                                                        // ohne def-Kennung
    if (RType == NUMBERFORMAT_TEXT)                         // Zahlzelle ->Stringz.
        res = FALSE;
    else
        res = pStringScanner->IsNumberFormat(sString, RType, fOutNumber, pFormat);

    if (res && !IsCompatible(FType, RType))     // unpassender Typ
    {
        switch ( RType )
        {
            case NUMBERFORMAT_TIME :
            {
                if ( pStringScanner->GetDecPos() )
                {   // 100stel Sekunden
                    if ( pStringScanner->GetAnzNums() > 3 || fOutNumber < 0.0 )
                        F_Index = GetFormatIndex( NF_TIME_HH_MMSS00, ActLnge );
                    else
                        F_Index = GetFormatIndex( NF_TIME_MMSS00, ActLnge );
                }
                else if ( fOutNumber >= 1.0 || fOutNumber < 0.0 )
                    F_Index = GetFormatIndex( NF_TIME_HH_MMSS, ActLnge );
                else
                    F_Index = GetStandardFormat( RType, ActLnge );
            }
            break;
            default:
                F_Index = GetStandardFormat( RType, ActLnge );
        }
    }
    return res;
}

BOOL SvNumberFormatter::IsCompatible(short eOldType,
                                     short eNewType)
{
    if (eOldType == eNewType)
        return TRUE;
    else if (eOldType == NUMBERFORMAT_DEFINED)
        return TRUE;
    else
    {
        switch (eNewType)
        {
            case NUMBERFORMAT_NUMBER:
            {
                switch (eOldType)
                {
                    case NUMBERFORMAT_PERCENT:
                    case NUMBERFORMAT_CURRENCY:
                    case NUMBERFORMAT_SCIENTIFIC:
                    case NUMBERFORMAT_FRACTION:
//                  case NUMBERFORMAT_LOGICAL:
                    case NUMBERFORMAT_DEFINED:
                        return TRUE;
                    default:
                        return FALSE;
                }
            }
            break;
            case NUMBERFORMAT_DATE:
            {
                switch (eOldType)
                {
                    case NUMBERFORMAT_DATETIME:
                        return TRUE;
                    default:
                        return FALSE;
                }
            }
            break;
            case NUMBERFORMAT_TIME:
            {
                switch (eOldType)
                {
                    case NUMBERFORMAT_DATETIME:
                        return TRUE;
                    default:
                        return FALSE;
                }
            }
            break;
            case NUMBERFORMAT_DATETIME:
            {
                switch (eOldType)
                {
                    case NUMBERFORMAT_TIME:
                    case NUMBERFORMAT_DATE:
                        return TRUE;
                    default:
                        return FALSE;
                }
            }
            break;
            default:
            return FALSE;
        }
        return FALSE;
    }
}

ULONG SvNumberFormatter::GetStandardFormat( short eType, LanguageType eLnge )
{
    ULONG CLOffset = ImpGenerateCL(eLnge);
    switch(eType)
    {
        case NUMBERFORMAT_DATE      : return CLOffset + ZF_STANDARD_DATE;
        case NUMBERFORMAT_TIME      : return CLOffset + ZF_STANDARD_TIME+1;
        case NUMBERFORMAT_DATETIME  : return CLOffset + ZF_STANDARD_DATETIME;
        case NUMBERFORMAT_CURRENCY  :
        {
            if ( eLnge == LANGUAGE_SYSTEM )
                return ImpGetDefaultSystemCurrencyFormat();
            else
                return CLOffset + ZF_STANDARD_CURRENCY+3;
        }
        case NUMBERFORMAT_PERCENT   : return CLOffset + ZF_STANDARD_PERCENT+1;
        case NUMBERFORMAT_SCIENTIFIC: return CLOffset + ZF_STANDARD_SCIENTIFIC;
        case NUMBERFORMAT_FRACTION  : return CLOffset + ZF_STANDARD_FRACTION;
        case NUMBERFORMAT_LOGICAL   : return CLOffset + ZF_STANDARD_LOGICAL;
        case NUMBERFORMAT_TEXT      : return CLOffset + ZF_STANDARD_TEXT;
        case NUMBERFORMAT_ALL       :
        case NUMBERFORMAT_DEFINED   :
        case NUMBERFORMAT_NUMBER    :
        case NUMBERFORMAT_UNDEFINED :
        default               : return CLOffset + ZF_STANDARD;
    }
}

BOOL SvNumberFormatter::IsSpecialStandardFormat( ULONG nFIndex,
        LanguageType eLnge )
{
    return
        nFIndex == GetFormatIndex( NF_TIME_MMSS00, eLnge ) ||
        nFIndex == GetFormatIndex( NF_TIME_HH_MMSS00, eLnge ) ||
        nFIndex == GetFormatIndex( NF_TIME_HH_MMSS, eLnge )
        ;
}

ULONG SvNumberFormatter::GetStandardFormat( ULONG nFIndex, short eType,
        LanguageType eLnge )
{
    if ( IsSpecialStandardFormat( nFIndex, eLnge ) )
        return nFIndex;
    else
        return GetStandardFormat( eType, eLnge );
}

ULONG SvNumberFormatter::GetStandardFormat( double fNumber, ULONG nFIndex,
        short eType, LanguageType eLnge )
{
    if ( IsSpecialStandardFormat( nFIndex, eLnge ) )
        return nFIndex;

    switch( eType )
    {
        case NUMBERFORMAT_TIME :
        {
            BOOL bSign;
            if ( fNumber < 0.0 )
            {
                bSign = TRUE;
                fNumber = -fNumber;
            }
            else
                bSign = FALSE;
            double fSeconds = fNumber * 86400;
            if ( floor( fSeconds + 0.5 ) * 100 != floor( fSeconds * 100 + 0.5 ) )
            {   // mit 100stel Sekunden
                if ( bSign || fSeconds >= 3600 )
                    return GetFormatIndex( NF_TIME_HH_MMSS00, eLnge );
                else
                    return GetFormatIndex( NF_TIME_MMSS00, eLnge );
            }
            else
            {
                if ( bSign || fNumber >= 1.0 )
                    return GetFormatIndex( NF_TIME_HH_MMSS, eLnge );
                else
                    return GetStandardFormat( eType, eLnge );
            }
        }
        break;
        default:
            return GetStandardFormat( eType, eLnge );
    }
}

void SvNumberFormatter::GetInputLineString(const double& fOutNumber,
                                           ULONG nFIndex,
                                           XubString& sOutString)
{
    SvNumberformat* pFormat;
    short nOldPrec;
    Color* pColor;
    pFormat = (SvNumberformat*) aFTable.Get(nFIndex);
    if (!pFormat)
        pFormat = aFTable.Get(ZF_STANDARD);
    LanguageType eLang = pFormat->GetLanguage();
    ChangeIntl( eLang );
    short eType = pFormat->GetType() & ~NUMBERFORMAT_DEFINED;
    if (eType == 0)
        eType = NUMBERFORMAT_DEFINED;
    nOldPrec = -1;
    if (eType == NUMBERFORMAT_NUMBER || eType == NUMBERFORMAT_PERCENT
                                     || eType == NUMBERFORMAT_CURRENCY
                                     || eType == NUMBERFORMAT_SCIENTIFIC
                                     || eType == NUMBERFORMAT_FRACTION)
    {
        if (eType != NUMBERFORMAT_PERCENT)  // spaeter Sonderbehandlung %
            eType = NUMBERFORMAT_NUMBER;
        nOldPrec = pFormatScanner->GetStandardPrec();
        ChangeStandardPrec(300);                        // Merkwert
    }
    ULONG nKey = nFIndex;
    switch ( eType )
    {   // #61619# immer vierstelliges Jahr editieren
        case NUMBERFORMAT_DATE :
            nKey = GetFormatIndex( NF_DATE_SYS_DDMMYYYY, eLang );
        break;
        case NUMBERFORMAT_DATETIME :
            nKey = GetFormatIndex( NF_DATETIME_SYS_DDMMYYYY_HHMMSS, eLang );
        break;
        default:
            nKey = GetStandardFormat( fOutNumber, nFIndex, eType, eLang );
    }
    if ( nKey != nFIndex )
        pFormat = (SvNumberformat*) aFTable.Get( nKey );
    if (pFormat)
    {
        if ( eType == NUMBERFORMAT_TIME && pFormat->GetFormatPrecision() )
        {
            nOldPrec = pFormatScanner->GetStandardPrec();
            ChangeStandardPrec(300);                        // Merkwert
        }
        pFormat->GetOutputString(fOutNumber, sOutString, &pColor);
    }
    if (nOldPrec != -1)
        ChangeStandardPrec(nOldPrec);
}

void SvNumberFormatter::GetOutputString(const double& fOutNumber,
                                        ULONG nFIndex,
                                        XubString& sOutString,
                                        Color** ppColor)
{
    if (bNoZero && fOutNumber == 0.0)
    {
        sOutString.Erase();
        return;
    }
    SvNumberformat* pFormat = (SvNumberformat*) aFTable.Get(nFIndex);
    if (!pFormat)
        pFormat = aFTable.Get(ZF_STANDARD);
    ChangeIntl(pFormat->GetLanguage());
    pFormat->GetOutputString(fOutNumber, sOutString, ppColor);
}

void SvNumberFormatter::GetOutputString(XubString& sString,
                                        ULONG nFIndex,
                                        XubString& sOutString,
                                        Color** ppColor)
{
    SvNumberformat* pFormat = (SvNumberformat*) aFTable.Get(nFIndex);
    if (!pFormat)
        pFormat = aFTable.Get(ZF_STANDARD_TEXT);
    if (!pFormat->IsTextFormat() && !pFormat->HasTextFormat())
    {
        *ppColor = NULL;
        sOutString = sString;
    }
    else
    {
        ChangeIntl(pFormat->GetLanguage());
        pFormat->GetOutputString(sString, sOutString, ppColor);
    }
}

BOOL SvNumberFormatter::GetPreviewString(const XubString& sFormatString,
                                         double fPreviewNumber,
                                         XubString& sOutString,
                                         Color** ppColor,
                                         LanguageType eLnge)
{
    if (sFormatString.Len() == 0)                       // keinen Leerstring
        return FALSE;

    xub_StrLen nCheckPos;
    ULONG nKey;
    if (eLnge == LANGUAGE_DONTKNOW)
        eLnge = SysLnge;
    ChangeIntl(eLnge);                          // ggfs. austauschen
    eLnge = ActLnge;
    XubString sTmpString = sFormatString;
    SvNumberformat* p_Entry = new SvNumberformat(sTmpString,
                                                 pFormatScanner,
                                                 pStringScanner,
                                                 nCheckPos,
                                                 eLnge);
    if (nCheckPos == 0)                                 // String ok
    {
        ULONG CLOffset = ImpGenerateCL(eLnge);              // ggfs. neu Standard-
                                                        // formate anlegen
        nKey = ImpIsEntry(p_Entry->GetFormatstring(),CLOffset, eLnge);
        if (nKey != NUMBERFORMAT_ENTRY_NOT_FOUND)               // schon vorhanden
            GetOutputString(fPreviewNumber,nKey,sOutString,ppColor);
        else
            p_Entry->GetOutputString(fPreviewNumber,sOutString, ppColor);
        delete p_Entry;
        return TRUE;
    }
    else
    {
        delete p_Entry;
        return FALSE;
    }
}

BOOL SvNumberFormatter::GetPreviewStringGuess( const XubString& sFormatString,
                                         double fPreviewNumber,
                                         XubString& sOutString,
                                         Color** ppColor,
                                         LanguageType eLnge )
{
    if (sFormatString.Len() == 0)                       // keinen Leerstring
        return FALSE;

    if (eLnge == LANGUAGE_DONTKNOW)
        eLnge = SysLnge;

    ChangeIntl( eLnge );
    eLnge = ActLnge;
    BOOL bEnglish = (eLnge == LANGUAGE_ENGLISH_US);

    String aFormatStringUpper( pCharClass->upper( sFormatString ) );
    ULONG nCLOffset = ImpGenerateCL( eLnge );
    ULONG nKey = ImpIsEntry( aFormatStringUpper, nCLOffset, eLnge );
    if ( nKey != NUMBERFORMAT_ENTRY_NOT_FOUND )
    {   // Zielformat vorhanden
        GetOutputString( fPreviewNumber, nKey, sOutString, ppColor );
        return TRUE;
    }

    SvNumberformat *pEntry = NULL;
    xub_StrLen nCheckPos;
    XubString sTmpString;

    if ( bEnglish )
    {
        sTmpString = sFormatString;
        pEntry = new SvNumberformat( sTmpString, pFormatScanner,
            pStringScanner, nCheckPos, eLnge );
    }
    else
    {
        nCLOffset = ImpGenerateCL( LANGUAGE_ENGLISH_US );
        nKey = ImpIsEntry( aFormatStringUpper, nCLOffset, LANGUAGE_ENGLISH_US );
        BOOL bEnglishFormat = (nKey != NUMBERFORMAT_ENTRY_NOT_FOUND);

        // try english --> other bzw. english nach other konvertieren
        LanguageType eFormatLang = LANGUAGE_ENGLISH_US;
        pFormatScanner->SetConvertMode( LANGUAGE_ENGLISH_US, eLnge );
        sTmpString = sFormatString;
        pEntry = new SvNumberformat( sTmpString, pFormatScanner,
            pStringScanner, nCheckPos, eFormatLang );
        pFormatScanner->SetConvertMode( FALSE );
        ChangeIntl( eLnge );

        if ( !bEnglishFormat )
        {
            if ( nCheckPos > 0 || pIntl->CompareEqual( sFormatString,
                    pEntry->GetFormatstring(), INTN_COMPARE_IGNORECASE ) )
            {   // other Format
                delete pEntry;
                sTmpString = sFormatString;
                pEntry = new SvNumberformat( sTmpString, pFormatScanner,
                    pStringScanner, nCheckPos, eLnge );
            }
            else
            {   // verify english
                xub_StrLen nCheckPos2;
                // try other --> english
                eFormatLang = eLnge;
                pFormatScanner->SetConvertMode( eLnge, LANGUAGE_ENGLISH_US );
                sTmpString = sFormatString;
                SvNumberformat* pEntry2 = new SvNumberformat( sTmpString, pFormatScanner,
                    pStringScanner, nCheckPos2, eFormatLang );
                pFormatScanner->SetConvertMode( FALSE );
                ChangeIntl( eLnge );
                if ( nCheckPos2 == 0 && !pIntl->CompareEqual( sFormatString,
                        pEntry2->GetFormatstring(), INTN_COMPARE_IGNORECASE ) )
                {   // other Format
                    delete pEntry;
                    sTmpString = sFormatString;
                    pEntry = new SvNumberformat( sTmpString, pFormatScanner,
                        pStringScanner, nCheckPos, eLnge );
                }
                delete pEntry2;
            }
        }
    }

    if (nCheckPos == 0)                                 // String ok
    {
        ImpGenerateCL( eLnge );     // ggfs. neu Standardformate anlegen
        pEntry->GetOutputString( fPreviewNumber, sOutString, ppColor );
        delete pEntry;
        return TRUE;
    }
    delete pEntry;
    return FALSE;
}

ULONG SvNumberFormatter::TestNewString(const XubString& sFormatString,
                                      LanguageType eLnge)
{
    if (sFormatString.Len() == 0)                       // keinen Leerstring
        return NUMBERFORMAT_ENTRY_NOT_FOUND;

    xub_StrLen nCheckPos;
    if (eLnge == LANGUAGE_DONTKNOW)
        eLnge = SysLnge;
    ChangeIntl(eLnge);                                  // ggfs. austauschen
    eLnge = ActLnge;
    ULONG nRes;
    BOOL bCheck = FALSE;
    XubString sTmpString = sFormatString;
    SvNumberformat* pEntry = new SvNumberformat(sTmpString,
                                                pFormatScanner,
                                                pStringScanner,
                                                nCheckPos,
                                                eLnge);
    if (nCheckPos == 0)                                 // String ok
    {
        ULONG CLOffset = ImpGenerateCL(eLnge);              // ggfs. neu Standard-
                                                        // formate anlegen
        nRes = ImpIsEntry(pEntry->GetFormatstring(),CLOffset, eLnge);
                                                        // schon vorhanden ?
    }
    else
        nRes = NUMBERFORMAT_ENTRY_NOT_FOUND;
    delete pEntry;
    return nRes;
}

SvNumberformat* SvNumberFormatter::ImpInsertFormat(XubString& sStr,
                                     BOOL bStandard,
                                     ULONG nPos)
{
    xub_StrLen nCheckPos = 0;
    SvNumberformat* pFormat = new SvNumberformat(sStr,
                                                 pFormatScanner,
                                                 pStringScanner,
                                                 nCheckPos,
                                                 ActLnge);
    if (!pFormat || nCheckPos > 0)
    {
        DBG_ERROR
            ("SvNumberFormatter:: Unbekanntes oder falsches Zahlformat");
        delete pFormat;
        return NULL;;
    }
    if (bStandard)
        pFormat->SetStandard();
    if (!aFTable.Insert(nPos, pFormat))
    {
        delete pFormat;
        return NULL;
    }
    return pFormat;
}

SvNumberformat* SvNumberFormatter::ImpInsertNewStandardFormat( XubString& sStr,
        ULONG nPos, USHORT nVersion )
{
    SvNumberformat* pNewFormat = ImpInsertFormat( sStr, FALSE, nPos );
    if (pNewFormat)
        pNewFormat->SetNewStandardDefined( nVersion );
        // damit es gespeichert, richtig angezeigt und von alten Versionen konvertiert wird
#ifndef PRODUCT
    else
    {
        ByteString aErr( "New standard format not inserted: " );
        aErr += ByteString( sStr, RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aErr.GetBuffer() );
    }
#endif
    return pNewFormat;
}

void SvNumberFormatter::GetFormatSpecialInfo(ULONG nFormat,
                                             BOOL& bThousand,
                                             BOOL& IsRed,
                                             USHORT& nPrecision,
                                             USHORT& nAnzLeading)

{
    SvNumberformat* pFormat = aFTable.Get(nFormat);
    if (pFormat)
        pFormat->GetFormatSpecialInfo(bThousand, IsRed,
                                      nPrecision, nAnzLeading);
    else
    {
        bThousand = FALSE;
        IsRed = FALSE;
        nPrecision = pFormatScanner->GetStandardPrec();
        nAnzLeading = 0;
    }
}

USHORT SvNumberFormatter::GetFormatPrecision( ULONG nFormat ) const
{
    SvNumberformat* pFormat = aFTable.Get( nFormat );
    if ( pFormat )
        return pFormat->GetFormatPrecision();
    else
        return pFormatScanner->GetStandardPrec();
}


ULONG SvNumberFormatter::GetFormatSpecialInfo( const XubString& rFormatString,
            BOOL& bThousand, BOOL& IsRed, USHORT& nPrecision,
            USHORT& nAnzLeading, LanguageType eLnge )

{
    xub_StrLen nCheckPos = 0;
    if (eLnge == LANGUAGE_DONTKNOW)
        eLnge = SysLnge;
    ChangeIntl(eLnge);                                  // ggfs. austauschen
    eLnge = ActLnge;
    XubString aTmpStr( rFormatString );
    SvNumberformat* pFormat = new SvNumberformat( aTmpStr,
        pFormatScanner, pStringScanner, nCheckPos, eLnge );
    if ( nCheckPos == 0 )
        pFormat->GetFormatSpecialInfo( bThousand, IsRed, nPrecision, nAnzLeading );
    else
    {
        bThousand = FALSE;
        IsRed = FALSE;
        nPrecision = pFormatScanner->GetStandardPrec();
        nAnzLeading = 0;
    }
    delete pFormat;
    return nCheckPos;
}


inline ULONG SetIndexTable( NfIndexTableOffset nTabOff, ULONG nIndOff )
{
    if ( !bIndexTableInitialized )
    {
        DBG_ASSERT( theIndexTable[nTabOff] == NUMBERFORMAT_ENTRY_NOT_FOUND,
            "SetIndexTable: theIndexTable[nTabOff] bereits belegt" );
        theIndexTable[nTabOff] = nIndOff;
    }
    return nIndOff;
}


void SvNumberFormatter::ImpGenerateFormats(ULONG CLOffset)
{
    if ( !bIndexTableInitialized )
    {
        for ( USHORT j=0; j<NF_INDEX_TABLE_ENTRIES; j++ )
        {
            theIndexTable[j] = NUMBERFORMAT_ENTRY_NOT_FOUND;
        }
    }
    BOOL bOldConvertMode = pFormatScanner->GetConvertMode();
    if (bOldConvertMode)                        // fuer diese Funktion
        pFormatScanner->SetConvertMode(FALSE);      // ausschalten
    xub_StrLen nCheckPos = 0;                   // Eintrag fuer Standard, Text
                                                // und logisch direkt erzeugen
    SvNumberformat* pNewFormat = NULL;
    XubString aSystem( RTL_CONSTASCII_USTRINGPARAM( "System" ) );           // Kommentar
    XubString aDefSystem( RTL_CONSTASCII_USTRINGPARAM( "def/System" ) );        // Kommentar
    // Zaehler fuer zusaetzliche Standardformate, die nicht in die ersten 10
    // einer Kategorie passen, insgesamt nochmal ca. 20
    // Bei jedem ImpInsertNewStandardformat hochzaehlen, neue Formate
    // hinten anhaengen, nicht einfuegen!
    USHORT nNewExtended = ZF_STANDARD_NEWEXTENDED;

    SvNumberformat* pStandard = new SvNumberformat(
                                        pFormatScanner->GetStandardName(),
                                        pFormatScanner, pStringScanner,
                                        nCheckPos, ActLnge);
    pStandard->SetType(NUMBERFORMAT_NUMBER);
    pStandard->SetStandard();
    if ( !aFTable.Insert(
            CLOffset + SetIndexTable( NF_NUMBER_STANDARD, ZF_STANDARD ),
            pStandard))
        delete pStandard;
    else
        pStandard->SetLastInsertKey(SV_MAX_ANZ_STANDARD_FORMATE);
    SvNumberformat* pLogical = new SvNumberformat(
                                        pFormatScanner->GetBooleanString(),
                                        pFormatScanner, pStringScanner,
                                        nCheckPos, ActLnge);
    pLogical->SetType(NUMBERFORMAT_LOGICAL);
    pLogical->SetStandard();
    if ( !aFTable.Insert(
            CLOffset + SetIndexTable( NF_BOOLEAN, ZF_STANDARD_LOGICAL ),
            pLogical))
        delete pLogical;

    XubString aWSString = '@';
    SvNumberformat* pText = new SvNumberformat(aWSString,
                                               pFormatScanner, pStringScanner,
                                               nCheckPos, ActLnge);
    pText->SetType(NUMBERFORMAT_TEXT);
    pText->SetStandard();
    if ( !aFTable.Insert(
            CLOffset + SetIndexTable( NF_TEXT, ZF_STANDARD_TEXT ),
            pText))
        delete pText;

    xub_Unicode cDecSep = pIntl->GetNumDecimalSep();
    xub_Unicode cThousandSep = pIntl->GetNumThousandSep();
                                                        // Zahl:
    XubString s0 = '0';                                 // 0
    ImpInsertFormat(s0,FALSE,
        CLOffset + SetIndexTable( NF_NUMBER_INT, ZF_STANDARD+1 ));
    XubString s1( RTL_CONSTASCII_USTRINGPARAM( "000" ) );       // 0,00
    s1.Insert(cDecSep,1);
    ImpInsertFormat(s1,FALSE,
        CLOffset + SetIndexTable( NF_NUMBER_DEC2, ZF_STANDARD+2 ));
    XubString s2( RTL_CONSTASCII_USTRINGPARAM( "###0" ) );      // #.##0
    s2.Insert(cThousandSep,1);
    ImpInsertFormat(s2,FALSE,
            CLOffset + SetIndexTable( NF_NUMBER_1000INT, ZF_STANDARD+3 ));
    XubString s3( RTL_CONSTASCII_USTRINGPARAM( "###000" ) );        // #.##0,00
    s3.Insert(cThousandSep,1);
    s3.Insert(cDecSep,5);
    ImpInsertFormat(s3,FALSE,
        CLOffset + SetIndexTable( NF_NUMBER_1000DEC2, ZF_STANDARD+4 ));
    // ab Version 6
    XubString sNumSystem;                               // #.##0,00 System
    if ( pIntl->IsNumLeadingZero() )
        sNumSystem = '0';
    if ( pIntl->IsNumThousandSep() )
    {
        if ( pIntl->IsNumLeadingZero() )
            sNumSystem.Insert( XubString::CreateFromAscii(
                RTL_CONSTASCII_STRINGPARAM( "###" ) ), 0 );
        else
            sNumSystem.Insert( XubString::CreateFromAscii(
                RTL_CONSTASCII_STRINGPARAM( "####" ) ), 0 );
        sNumSystem.Insert( cThousandSep, 1 );
    }
    USHORT nNumDigits = pIntl->GetNumDigits();
    if ( nNumDigits )
    {
        sNumSystem += cDecSep;
        for ( USHORT j=0; j<nNumDigits; j++ )
        {
            sNumSystem += '0';
        }
    }
    pNewFormat = ImpInsertNewStandardFormat( sNumSystem,
        CLOffset + SetIndexTable( NF_NUMBER_SYSTEM, ZF_STANDARD+5 ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );
    if ( pNewFormat )
        pNewFormat->SetComment( aSystem );

                                                        // Prozent:
    XubString s4( RTL_CONSTASCII_USTRINGPARAM( "0%" ) );            // 0%
    ImpInsertFormat(s4,FALSE,
        CLOffset + SetIndexTable( NF_PERCENT_INT, ZF_STANDARD_PERCENT ));
    XubString s5( RTL_CONSTASCII_USTRINGPARAM( "000%" ) );      // 0,00%
    s5.Insert(cDecSep,1);
    ImpInsertFormat(s5,TRUE,
        CLOffset + SetIndexTable( NF_PERCENT_DEC2, ZF_STANDARD_PERCENT+1 ));

    // Waehrung: (keine Standardoption also kein TRUE bei ImpInsertFormat)

    XubString aRed( '[' );
    aRed += pFormatScanner->GetRedString();
    aRed += ']';

    XubString sCurrFlat1( RTL_CONSTASCII_USTRINGPARAM( "###0" ) );      // #.##0 DM
    sCurrFlat1.Insert(cThousandSep,1);
    XubString sCurrFlat2( sCurrFlat1 );                  // #.##0,00 DM
    XubString sCurrFlat3( sCurrFlat1 );                  // #.##0,-- DM
    // Anzahl Nachkommastellen in Waehrung
    USHORT nCurrDigits = pIntl->GetCurrDigits();
    if ( nCurrDigits )
    {
        sCurrFlat2 += cDecSep;
        sCurrFlat3 += cDecSep;
        sCurrFlat2.Expand( sCurrFlat2.Len() + nCurrDigits, '0' );
        sCurrFlat3.Expand( sCurrFlat3.Len() + nCurrDigits, '-' );
    }

    XubString s6 = sCurrFlat1;                          // #.##0 DM
    ImpGetPosCurrFormat(s6);
    XubString s7 = sCurrFlat2;                          // #.##0,00 DM
    ImpGetPosCurrFormat(s7);
    XubString sDash = sCurrFlat3;                       // #.##0,-- DM
    ImpGetPosCurrFormat(sDash);
    XubString sNegStr1 = sCurrFlat1;                        // -#.##0 DM
    ImpGetNegCurrFormat(sNegStr1);
    XubString sNegStr2 = sCurrFlat2;                        // -#.##0,00 DM
    ImpGetNegCurrFormat(sNegStr2);
    XubString sNegStr3 = sCurrFlat3;                        // -#.##0,-- DM
    ImpGetNegCurrFormat(sNegStr3);
    s6 += ';';
    XubString s8 = s6;
    s6 += sNegStr1;
    s8 += aRed;
    s8 += sNegStr1;
    s7 += ';';
    XubString s9 = s7;
    s7 += sNegStr2;
    s9 += aRed;
    s9 += sNegStr2;
    sDash += ';';
    XubString sDashed = sDash;
    sDash += sNegStr3;
    sDashed += aRed;
    sDashed += sNegStr3;
    ImpInsertFormat(s6,FALSE,
        CLOffset + SetIndexTable( NF_CURRENCY_1000INT, ZF_STANDARD_CURRENCY ));
    ImpInsertFormat(s7,FALSE,
        CLOffset + SetIndexTable( NF_CURRENCY_1000DEC2, ZF_STANDARD_CURRENCY+1 ));
                                                        // Muss immer 1 hinter
                                                        // Standard sein
                                                        // fuer Funktion DM()!!
    ImpInsertFormat(s8,FALSE,
        CLOffset + SetIndexTable( NF_CURRENCY_1000INT_RED, ZF_STANDARD_CURRENCY+2 ));
    ImpInsertFormat(s9,FALSE,
        CLOffset + SetIndexTable( NF_CURRENCY_1000DEC2_RED, ZF_STANDARD_CURRENCY+3 ));

    // ab Version 3 dazu
    XubString s28 = sCurrFlat2;                             // #.##0,00 DEM
#if NF_BANKSYMBOL_FIX_POSITION
    s28.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " CCC" ) );
#else
    XubString s28n( sCurrFlat2 ), aCCC( RTL_CONSTASCII_USTRINGPARAM( "CCC" ) );
    USHORT nPosiForm = NfCurrencyEntry::GetEffectivePositiveFormat(
        pIntl->GetCurrPositiveFormat(), pIntl->GetCurrPositiveFormat(), TRUE );
    USHORT nNegaForm = NfCurrencyEntry::GetEffectiveNegativeFormat(
        pIntl->GetCurrNegativeFormat(), pIntl->GetCurrNegativeFormat(), TRUE );
    NfCurrencyEntry::CompletePositiveFormatString( s28, aCCC, nPosiForm );
    NfCurrencyEntry::CompleteNegativeFormatString( s28n, aCCC, nNegaForm );
    s28 += ';';
    s28 += s28n;
#endif
    pNewFormat = ImpInsertFormat(s28,FALSE,
        CLOffset + SetIndexTable( NF_CURRENCY_1000DEC2_CCC, ZF_STANDARD_CURRENCY+4 ));
    if ( pNewFormat )
        pNewFormat->SetUsed(TRUE);          // damit es gespeichert wird

    // ab Version 6 dazu: #.##0,-- DM
    ImpInsertNewStandardFormat( sDashed,
        CLOffset + SetIndexTable( NF_CURRENCY_1000DEC2_DASHED, ZF_STANDARD_CURRENCY+5 ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );

#if 0   // erDEBUG
    {   // ab SV_NUMBERFORMATTER_VERSION_NEW_CURR EURo und letztes Format testen
        NfWSStringsDtor aArr;
        USHORT j, nDefault;
        const NfCurrencyTable& rTable = GetTheCurrencyTable();
        const NfCurrencyEntry* pCurr = rTable.GetObject( nCurrencyTableEuroPosition );
        GetCurrencyFormatStrings( aArr, *pCurr, FALSE );
        GetCurrencyFormatStrings( aArr, *pCurr, TRUE );
        for ( j=0; j<aArr.Count(); j++ )
        {
            ULONG nCheck, nKey;
            short nType;
            PutEntry( *aArr.GetObject(j), nCheck, nType, nKey, ActLnge );
        }
        aArr.Remove( 0, aArr.Count() );
        pCurr = rTable.GetObject( rTable.Count()-1 );
//      GetCurrencyFormatStrings( aArr, *pCurr, FALSE );
        GetCurrencyFormatStrings( aArr, *pCurr, TRUE );
        for ( j=0; j<aArr.Count(); j++ )
        {
            ULONG nCheck, nKey;
            short nType;
            PutEntry( *aArr.GetObject(j), nCheck, nType, nKey, ActLnge );
        }
    }
#endif

                                            // Datum:
    XubString s10;
    ImpGetDateFormat(s10);                  // TT.MM.JJ   kurzes Systemdatum
    pNewFormat = ImpInsertFormat(s10,TRUE,
        CLOffset + SetIndexTable( NF_DATE_SYSTEM_SHORT, ZF_STANDARD_DATE ));
    if ( pNewFormat )
        pNewFormat->SetComment( aSystem );

    const XubString* pKeyword = pFormatScanner->GetKeyword();
    xub_Unicode cDateSep = pIntl->GetDateSep();
    xub_Unicode cTimeSep = pIntl->GetTimeSep();

    XubString s11 = pKeyword[NF_KEY_NN];        // NN TT.MMM JJ
    s11 += ' ';
    s11 += pKeyword[NF_KEY_TT];
    s11 += cDateSep;
    s11 += pKeyword[NF_KEY_MMM];
    s11 += ' ';
    s11 += pKeyword[NF_KEY_JJ];
    ImpInsertFormat(s11,FALSE,
        CLOffset + SetIndexTable( NF_DATE_DEF_NNDDMMMYY, ZF_STANDARD_DATE+1 ));

    XubString s12;                           // MM.JJ
    XubString s13;                           // TT.MMM
    XubString sFullDate;                        // TT.MM.JJJJ
    XubString sDDMMYY;                       // TT.MM.JJ
    switch(pIntl->GetDateFormat())
    {
        case DMY:
        {
            s12 = pKeyword[NF_KEY_MM];          // MM.JJ
            s12 += cDateSep;
            s12 += pKeyword[NF_KEY_JJ];
            s13 = pKeyword[NF_KEY_TT];          // TT.MMM
            s13 += cDateSep;
            s13 += pKeyword[NF_KEY_MMM];
            sFullDate = pKeyword[NF_KEY_TT];    // TT
            sFullDate += cDateSep;
            sFullDate += pKeyword[NF_KEY_MM];   // MM
            sFullDate += cDateSep;
            sDDMMYY = sFullDate;
            sFullDate += pKeyword[NF_KEY_JJJJ]; // JJJJ
            sDDMMYY += pKeyword[NF_KEY_JJ];     // JJ
        }
        break;
        case YMD:
        {
            s12 = pKeyword[NF_KEY_JJ];          // JJ.MM
            s12 += cDateSep;
            s12 += pKeyword[NF_KEY_MM];
            s13 = pKeyword[NF_KEY_MMM];         // MMM TT
            s13 += ' ';
            s13 += pKeyword[NF_KEY_TT];
            sFullDate = pKeyword[NF_KEY_JJJJ];  // JJJJ
            sDDMMYY = pKeyword[NF_KEY_JJ];      // JJ
            XubString aTmp( cDateSep );
            aTmp += pKeyword[NF_KEY_MM];        // MM
            aTmp += cDateSep;
            aTmp += pKeyword[NF_KEY_TT];        // TT
            sFullDate += aTmp;
            sDDMMYY += aTmp;

        }
        break;
        case MDY:
        default:
        {
            s12 = pKeyword[NF_KEY_MM];          // MM.JJ
            s12 += cDateSep;
            s12 += pKeyword[NF_KEY_JJ];
            s13 = pKeyword[NF_KEY_MMM];         // MMM TT
            s13 += ' ';
            s13 += pKeyword[NF_KEY_TT];
            sFullDate = pKeyword[NF_KEY_MM];    // MM
            sFullDate += cDateSep;
            sFullDate += pKeyword[NF_KEY_TT];   // TT
            sFullDate += cDateSep;
            sDDMMYY = sFullDate;
            sFullDate += pKeyword[NF_KEY_JJJJ]; // JJJJ
            sDDMMYY += pKeyword[NF_KEY_JJ];     // JJ
        }
        break;
    }
    pNewFormat = ImpInsertFormat(s12,FALSE,
        CLOffset + SetIndexTable( NF_DATE_SYS_MMYY, ZF_STANDARD_DATE+2 ));
    if ( pNewFormat )
        pNewFormat->SetComment( aDefSystem );

    ImpInsertFormat(s13,FALSE,
        CLOffset + SetIndexTable( NF_DATE_SYS_DDMMM, ZF_STANDARD_DATE+3 ));

    XubString s14 = pKeyword[NF_KEY_MMMM];      // MMMM
    ImpInsertFormat(s14,FALSE,
        CLOffset + SetIndexTable( NF_DATE_MMMM, ZF_STANDARD_DATE+4 ));

    XubString s15 = pKeyword[NF_KEY_QQ];            // QQ JJ
    s15 += ' ';
    s15 += pKeyword[NF_KEY_JJ];
    ImpInsertFormat(s15,FALSE,
        CLOffset + SetIndexTable( NF_DATE_QQJJ, ZF_STANDARD_DATE+5 ));

    // ab Version 2 dazu, war TT.MM.[JJ]JJ jetzt (v6) TT.MM.JJJJ
    pNewFormat = ImpInsertFormat(sFullDate,FALSE,
        CLOffset + SetIndexTable( NF_DATE_SYS_DDMMYYYY, ZF_STANDARD_DATE+6 ));
    if ( pNewFormat )
    {
        pNewFormat->SetUsed(TRUE);          // damit es gespeichert wird
        pNewFormat->SetComment( aDefSystem );
    }

    // ab Version 6 dazu
    pNewFormat = ImpInsertNewStandardFormat( sDDMMYY,
        CLOffset + SetIndexTable( NF_DATE_SYS_DDMMYY, ZF_STANDARD_DATE+7 ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );
    if ( pNewFormat )
        pNewFormat->SetComment( aDefSystem );
    // lange Wochentage: statt "NNN, " steht "NNNN" Code wg. Kompatibilitaet
    // langes Systemdatum wie eingestellt
    XubString sLongDate;
    ImpGetLongDateFormat( sLongDate );              // NNN, T. MMMM JJJJ   System
    pNewFormat = ImpInsertNewStandardFormat( sLongDate,
        CLOffset + SetIndexTable( NF_DATE_SYSTEM_LONG, ZF_STANDARD_DATE+8 ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );
    if ( pNewFormat )
        pNewFormat->SetComment( aSystem );
    // harte aber systemabhaengige lange Datumformate
    sLongDate.Erase();
    ImpGetLongDateFormat( sLongDate, -1, 1, 1, 1 ); // T. MMM JJ   def/System
    pNewFormat = ImpInsertNewStandardFormat( sLongDate,
        CLOffset + SetIndexTable( NF_DATE_SYS_DMMMYY, ZF_STANDARD_DATE+9 ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );
    if ( pNewFormat )
        pNewFormat->SetComment( aDefSystem );
    //! leider hat das Templin'sche Erbe nur 10 Standardformate pro Kategorie
    //! vorgesehen, weitere wuerden ZF_STANDARD_TIME ueberschreiben :-((
    sLongDate.Erase();
    ImpGetLongDateFormat( sLongDate, -1, 1, 1, 2 ); // T. MMM JJJJ   def/System
    pNewFormat = ImpInsertNewStandardFormat( sLongDate,
        CLOffset + SetIndexTable( NF_DATE_SYS_DMMMYYYY, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );
    if ( pNewFormat )
        pNewFormat->SetComment( aDefSystem );
    sLongDate.Erase();
    ImpGetLongDateFormat( sLongDate, -1, 1, 2, 2 ); // T. MMMM JJJJ   def/System
    pNewFormat = ImpInsertNewStandardFormat( sLongDate,
        CLOffset + SetIndexTable( NF_DATE_SYS_DMMMMYYYY, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );
    if ( pNewFormat )
        pNewFormat->SetComment( aDefSystem );
    sLongDate.Erase();
    ImpGetLongDateFormat( sLongDate, 1, 1, 1, 1 );  // NN, T. MMM JJ   def/System
    pNewFormat = ImpInsertNewStandardFormat( sLongDate,
        CLOffset + SetIndexTable( NF_DATE_SYS_NNDMMMYY, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );
    if ( pNewFormat )
        pNewFormat->SetComment( aDefSystem );
    sLongDate.Erase();
    ImpGetLongDateFormat( sLongDate, 1, 1, 2, 2 );  // NN, T. MMMM JJJJ   def/System
    pNewFormat = ImpInsertNewStandardFormat( sLongDate,
        CLOffset + SetIndexTable( NF_DATE_SYS_NNDMMMMYYYY, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );
    if ( pNewFormat )
        pNewFormat->SetComment( aDefSystem );
    sLongDate.Erase();
    ImpGetLongDateFormat( sLongDate, 2, 1, 2, 2 );  // NNN, T. MMMM JJJJ   def/System
    pNewFormat = ImpInsertNewStandardFormat( sLongDate,
        CLOffset + SetIndexTable( NF_DATE_SYS_NNNNDMMMMYYYY, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );
    if ( pNewFormat )
        pNewFormat->SetComment( aDefSystem );

    XubString aDIN( RTL_CONSTASCII_USTRINGPARAM( "DIN 5008 (EN 28601)" ) );
    XubString sDinTMMMJJJJ;                     // DIN: T. MMM. JJJJ
    sDinTMMMJJJJ += pKeyword[NF_KEY_T];
    sDinTMMMJJJJ += '.';
    sDinTMMMJJJJ += ' ';
    sDinTMMMJJJJ += pKeyword[NF_KEY_MMM];
    sDinTMMMJJJJ += '.';
    sDinTMMMJJJJ += ' ';
    sDinTMMMJJJJ += pKeyword[NF_KEY_JJJJ];
    pNewFormat = ImpInsertNewStandardFormat( sDinTMMMJJJJ,
        CLOffset + SetIndexTable( NF_DATE_DIN_DMMMYYYY, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );
    if ( pNewFormat )
        pNewFormat->SetComment( aDIN );

    XubString sDinTMMMMJJJJ;                        // DIN: T. MMMM JJJJ
    sDinTMMMMJJJJ += pKeyword[NF_KEY_T];
    sDinTMMMMJJJJ += '.';
    sDinTMMMMJJJJ += ' ';
    sDinTMMMMJJJJ += pKeyword[NF_KEY_MMMM];
    sDinTMMMMJJJJ += ' ';
    sDinTMMMMJJJJ += pKeyword[NF_KEY_JJJJ];
    pNewFormat = ImpInsertNewStandardFormat( sDinTMMMMJJJJ,
        CLOffset + SetIndexTable( NF_DATE_DIN_DMMMMYYYY, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );
    if ( pNewFormat )
        pNewFormat->SetComment( aDIN );

    XubString sDinMMTT;                         // DIN: MM-TT
    sDinMMTT += pKeyword[NF_KEY_MM];
    sDinMMTT += '-';
    sDinMMTT += pKeyword[NF_KEY_TT];
    pNewFormat = ImpInsertNewStandardFormat( sDinMMTT,
        CLOffset + SetIndexTable( NF_DATE_DIN_MMDD, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );
    if ( pNewFormat )
        pNewFormat->SetComment( aDIN );

    XubString sDinJJMMTT = sDinMMTT;                // DIN: JJ-MM-TT
    sDinJJMMTT.Insert( '-', 0 );
    sDinJJMMTT.Insert( pKeyword[NF_KEY_JJ], 0 );
    pNewFormat = ImpInsertNewStandardFormat( sDinJJMMTT,
        CLOffset + SetIndexTable( NF_DATE_DIN_YYMMDD, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );
    if ( pNewFormat )
        pNewFormat->SetComment( aDIN );

    XubString sDinJJJJMMTT = sDinMMTT;          // DIN: JJJJ-MM-TT
    sDinJJJJMMTT.Insert( '-', 0 );
    sDinJJJJMMTT.Insert( pKeyword[NF_KEY_JJJJ], 0 );
    pNewFormat = ImpInsertNewStandardFormat( sDinJJJJMMTT,
        CLOffset + SetIndexTable( NF_DATE_DIN_YYYYMMDD, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );
    if ( pNewFormat )
        pNewFormat->SetComment( aDIN );

                                            // Uhrzeit:
    XubString s16 = pKeyword[NF_KEY_HH];        // HH:MM
    s16 += cTimeSep;
    s16 += pKeyword[NF_KEY_MMI];
    ImpInsertFormat(s16,FALSE,
        CLOffset + SetIndexTable( NF_TIME_HHMM, ZF_STANDARD_TIME ));
    XubString sFullTime = s16;              // HH:MM:SS
    sFullTime += cTimeSep;
    sFullTime += pKeyword[NF_KEY_SS];
    ImpInsertFormat(sFullTime,TRUE,
        CLOffset + SetIndexTable( NF_TIME_HHMMSS, ZF_STANDARD_TIME+1 ));
    XubString s18 = s16;                        // HH:MM AM/PM
    s18 += ' ';
    s18 += pKeyword[NF_KEY_AMPM];
    ImpInsertFormat(s18,FALSE,
        CLOffset + SetIndexTable( NF_TIME_HHMMAMPM, ZF_STANDARD_TIME+2 ));
    XubString s19 = sFullTime;              // HH:MM:SS AM/PM
    s19 += ' ';
    s19 += pKeyword[NF_KEY_AMPM];
    ImpInsertFormat(s19,FALSE,
        CLOffset + SetIndexTable( NF_TIME_HHMMSSAMPM, ZF_STANDARD_TIME+3 ));
    XubString s20 = sFullTime;              // [HH]:MM:SS
    s20.Insert('[',0);
    s20.Insert(']',3);
    ImpInsertFormat(s20,FALSE,
        CLOffset + SetIndexTable( NF_TIME_HH_MMSS, ZF_STANDARD_TIME+4 ));
    XubString s21 = pKeyword[NF_KEY_MMI];   // MM:SS,00
    s21 += cTimeSep;
    s21 += pKeyword[NF_KEY_SS];
    s21 += cDecSep;
    s21 += '0';
    s21 += '0';
    ImpInsertFormat(s21,FALSE,
        CLOffset + SetIndexTable( NF_TIME_MMSS00, ZF_STANDARD_TIME+5 ));
    XubString s29 = s20;                        // [HH]:MM:SS,00
    s29 += cDecSep;
    s29 += '0';
    s29 += '0';
    ImpInsertNewStandardFormat( s29,
        CLOffset + SetIndexTable( NF_TIME_HH_MMSS00, ZF_STANDARD_TIME+6 ),
        SV_NUMBERFORMATTER_VERSION_NF_TIME_HH_MMSS00 );

                                            // Datum u. Uhrzeit:
    XubString s22 = s10;                    // TT.MM.JJ HH:MM
    s22 += ' ';
    s22 += s16;
    ImpInsertFormat(s22,TRUE,
        CLOffset + SetIndexTable( NF_DATETIME_SYSTEM_SHORT_HHMM, ZF_STANDARD_DATETIME ));
    XubString sFullDateTime = sFullDate;        // TT.MM.JJJJ HH:MM:SS
    sFullDateTime += ' ';
    sFullDateTime += sFullTime;
    ImpInsertNewStandardFormat( sFullDateTime,
        CLOffset + SetIndexTable( NF_DATETIME_SYS_DDMMYYYY_HHMMSS, ZF_STANDARD_DATETIME+1 ),
        SV_NUMBERFORMATTER_VERSION_NF_DATETIME_SYS_DDMMYYYY_HHMMSS );

                                                        // Wissenschaft:
    XubString s23( RTL_CONSTASCII_USTRINGPARAM( "000E+000" ) );     // 0,00E+000
    s23.Insert(cDecSep,1);
    ImpInsertFormat(s23,TRUE,
        CLOffset + SetIndexTable( NF_SCIENTIFIC_000E000, ZF_STANDARD_SCIENTIFIC ));
    XubString s24( RTL_CONSTASCII_USTRINGPARAM( "000E+00" ) );      // 0,00E+00
    s24.Insert(cDecSep,1);
    ImpInsertFormat(s24,FALSE,
        CLOffset + SetIndexTable( NF_SCIENTIFIC_000E00, ZF_STANDARD_SCIENTIFIC+1 ));

                                                // Bruch: (keine Standardopt.)
    XubString s25( RTL_CONSTASCII_USTRINGPARAM( "# ?/?" ) );            // # ?/?
    ImpInsertFormat(s25,FALSE,
        CLOffset + SetIndexTable( NF_FRACTION_1, ZF_STANDARD_FRACTION ));
    // "??/" wird vom Compiler als Trigraph fuer '\' interpretiert!
    XubString s26( RTL_CONSTASCII_USTRINGPARAM( "# ?\?/?\?" ) );        // # ??/??
    ImpInsertFormat(s26,FALSE,
        CLOffset + SetIndexTable( NF_FRACTION_2, ZF_STANDARD_FRACTION+1 ));

    XubString sWeekOfYear( pKeyword[NF_KEY_WW] );
    pNewFormat = ImpInsertNewStandardFormat( sWeekOfYear,
        CLOffset + SetIndexTable( NF_DATE_WW, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NF_DATE_WW );

    bIndexTableInitialized = TRUE;
    if (bOldConvertMode)                        // jetzt wieder
        pFormatScanner->SetConvertMode(TRUE);   // einschalten
    DBG_ASSERT( nNewExtended <= ZF_STANDARD_NEWEXTENDEDMAX,
        "Ueberlauf der nNewExtended Standardformate" );
}

void SvNumberFormatter::ImpGetDateFormat(XubString& sDateStr)
{
    const XubString* pKeyword = pFormatScanner->GetKeyword();
    xub_Unicode cDateSep = pIntl->GetDateSep();
    switch (pIntl->GetDateFormat())
    {
        case MDY:
        {
            if (pIntl->IsDateMonthLeadingZero())
                sDateStr += pKeyword[NF_KEY_MM];
            else
                sDateStr += pKeyword[NF_KEY_M];
            sDateStr += cDateSep;
            if (pIntl->IsDateDayLeadingZero())
                sDateStr += pKeyword[NF_KEY_TT];
            else
                sDateStr += pKeyword[NF_KEY_T];
            sDateStr += cDateSep;
            if (pIntl->IsDateCentury())
                sDateStr += pKeyword[NF_KEY_JJJJ];
            else
                sDateStr += pKeyword[NF_KEY_JJ];
        }
        break;
        case DMY:
        {
            if (pIntl->IsDateDayLeadingZero())
                sDateStr += pKeyword[NF_KEY_TT];
            else
                sDateStr += pKeyword[NF_KEY_T];
            sDateStr += cDateSep;
            if (pIntl->IsDateMonthLeadingZero())
                sDateStr += pKeyword[NF_KEY_MM];
            else
                sDateStr += pKeyword[NF_KEY_M];
            sDateStr += cDateSep;
            if (pIntl->IsDateCentury())
                sDateStr += pKeyword[NF_KEY_JJJJ];
            else
                sDateStr += pKeyword[NF_KEY_JJ];
        }
        break;
        case YMD:
        {
            if (pIntl->IsDateCentury())
                sDateStr += pKeyword[NF_KEY_JJJJ];
            else
                sDateStr += pKeyword[NF_KEY_JJ];
            sDateStr += cDateSep;
            if (pIntl->IsDateMonthLeadingZero())
                sDateStr += pKeyword[NF_KEY_MM];
            else
                sDateStr += pKeyword[NF_KEY_M];
            sDateStr += cDateSep;
            if (pIntl->IsDateDayLeadingZero())
                sDateStr += pKeyword[NF_KEY_TT];
            else
                sDateStr += pKeyword[NF_KEY_T];
        }
        break;
        default:
        break;
    }
}


void lcl_ImplAddStringMaybeQuoted( String& rDes, const String& rSrc,
            CharClass* pChrCls )
{
    BOOL bQuoted = FALSE;
    const xub_StrLen nLen = rSrc.Len();
    xub_StrLen nPos = 0;
    while ( nPos < nLen )
    {
        if ( !bQuoted && pChrCls->isLetter( rSrc, nPos ) )
        {
            rDes += '"';
            bQuoted = TRUE;
        }
        else if ( bQuoted && !pChrCls->isLetter( rSrc, nPos ) )
        {
            rDes += '"';
            bQuoted = FALSE;
        }
        rDes += rSrc.GetChar( nPos++ );
    }
    if ( bQuoted )
        rDes += '"';
}


void SvNumberFormatter::ImpGetLongDateFormat( XubString& sDateStr,
                                short nShortLongDayOfWeek,
                                short nShortLongDay,
                                short nShortLongMonth,
                                short nShortLongYear )
{
    const XubString* pKeyword = pFormatScanner->GetKeyword();
    switch ( nShortLongDayOfWeek )
    {
        case -1 :
        break;
        case 0 :
            switch ( pIntl->GetLongDateDayOfWeekFormat() )
            {
                case DAYOFWEEK_NONE :
                break;
                case DAYOFWEEK_SHORT :
                    sDateStr += pKeyword[NF_KEY_NN];
                    lcl_ImplAddStringMaybeQuoted( sDateStr,
                        pIntl->GetLongDateDayOfWeekSep(), pCharClass );
                break;
                case DAYOFWEEK_LONG :
                    sDateStr += pKeyword[NF_KEY_NNNN];
                    // NNNN ist leider bereits mit Separator, so wird es in
                    // aelteren Versionen, die noch kein NNN kennen, korrekt
                    // dargestellt, nur der Formatstring sieht krank aus..
                break;
                default:
                    DBG_ERRORFILE( "unknown LongDateDayOfWeekFormat" );
            }
        break;
        case 1 :
            sDateStr += pKeyword[NF_KEY_NN];
            lcl_ImplAddStringMaybeQuoted( sDateStr,
                pIntl->GetLongDateDayOfWeekSep(), pCharClass );
        break;
        case 2 :
            sDateStr += pKeyword[NF_KEY_NNNN];
        break;
        default:
            DBG_ERRORFILE( "unknown nShortLongDayOfWeek" );
    }
    switch (pIntl->GetLongDateFormat())
    {
        case MDY:
        {
            switch ( nShortLongMonth )
            {
                case -1 :
                break;
                case 0 :
                    switch ( pIntl->GetLongDateMonthFormat() )
                    {
                        case MONTH_NORMAL :
                            sDateStr += pKeyword[NF_KEY_M];
                        break;
                        case MONTH_ZERO :
                            sDateStr += pKeyword[NF_KEY_MM];
                        break;
                        case MONTH_SHORT :
                            sDateStr += pKeyword[NF_KEY_MMM];
                        break;
                        case MONTH_LONG :
                            sDateStr += pKeyword[NF_KEY_MMMM];
                        break;
                        default:
                            DBG_ERRORFILE( "unknown LongDateMonthFormat" );
                    }
                break;
                case 1 :
                    sDateStr += pKeyword[NF_KEY_MMM];
                break;
                case 2 :
                    sDateStr += pKeyword[NF_KEY_MMMM];
                break;
                default:
                    DBG_ERRORFILE( "unknown nShortLongMonth" );
            }
            if ( nShortLongMonth != -1 )
                lcl_ImplAddStringMaybeQuoted( sDateStr,
                    pIntl->GetLongDateMonthSep(), pCharClass );

            switch ( nShortLongDay )
            {
                case -1 :
                break;
                case 0 :
                    if ( pIntl->IsLongDateDayLeadingZero() )
                        sDateStr += pKeyword[NF_KEY_TT];
                    else
                        sDateStr += pKeyword[NF_KEY_T];
                break;
                case 1 :
                    sDateStr += pKeyword[NF_KEY_T];
                break;
                case 2 :
                    sDateStr += pKeyword[NF_KEY_TT];
                break;
                default:
                    DBG_ERRORFILE( "unknown nShortLongDay" );
            }
            if ( nShortLongDay != -1 )
                lcl_ImplAddStringMaybeQuoted( sDateStr,
                    pIntl->GetLongDateDaySep(), pCharClass );

            switch ( nShortLongYear )
            {
                case -1 :
                break;
                case 0 :
                    if ( pIntl->IsLongDateCentury() )
                        sDateStr += pKeyword[NF_KEY_JJJJ];
                    else
                        sDateStr += pKeyword[NF_KEY_JJ];
                break;
                case 1 :
                    sDateStr += pKeyword[NF_KEY_JJ];
                break;
                case 2 :
                    sDateStr += pKeyword[NF_KEY_JJJJ];
                break;
                default:
                    DBG_ERRORFILE( "unknown nShortLongYear" );
            }
            //if ( nShortLongYear != -1 )
            //  lcl_ImplAddStringMaybeQuoted( sDateStr,
            //      pIntl->GetLongDateYearSep(), pCharClass );

        }
        break;
        case DMY:
        {
            switch ( nShortLongDay )
            {
                case -1 :
                break;
                case 0 :
                    if ( pIntl->IsLongDateDayLeadingZero() )
                        sDateStr += pKeyword[NF_KEY_TT];
                    else
                        sDateStr += pKeyword[NF_KEY_T];
                break;
                case 1 :
                    sDateStr += pKeyword[NF_KEY_T];
                break;
                case 2 :
                    sDateStr += pKeyword[NF_KEY_TT];
                break;
                default:
                    DBG_ERRORFILE( "unknown nShortLongDay" );
            }
            if ( nShortLongDay != -1 )
                lcl_ImplAddStringMaybeQuoted( sDateStr,
                    pIntl->GetLongDateDaySep(), pCharClass );

            switch ( nShortLongMonth )
            {
                case -1 :
                break;
                case 0 :
                    switch ( pIntl->GetLongDateMonthFormat() )
                    {
                        case MONTH_NORMAL :
                            sDateStr += pKeyword[NF_KEY_M];
                        break;
                        case MONTH_ZERO :
                            sDateStr += pKeyword[NF_KEY_MM];
                        break;
                        case MONTH_SHORT :
                            sDateStr += pKeyword[NF_KEY_MMM];
                        break;
                        case MONTH_LONG :
                            sDateStr += pKeyword[NF_KEY_MMMM];
                        break;
                        default:
                            DBG_ERRORFILE( "unknown LongDateMonthFormat" );
                    }
                break;
                case 1 :
                    sDateStr += pKeyword[NF_KEY_MMM];
                break;
                case 2 :
                    sDateStr += pKeyword[NF_KEY_MMMM];
                break;
                default:
                    DBG_ERRORFILE( "unknown nShortLongMonth" );
            }
            if ( nShortLongMonth != -1 )
                lcl_ImplAddStringMaybeQuoted( sDateStr,
                    pIntl->GetLongDateMonthSep(), pCharClass );

            switch ( nShortLongYear )
            {
                case -1 :
                break;
                case 0 :
                    if ( pIntl->IsLongDateCentury() )
                        sDateStr += pKeyword[NF_KEY_JJJJ];
                    else
                        sDateStr += pKeyword[NF_KEY_JJ];
                break;
                case 1 :
                    sDateStr += pKeyword[NF_KEY_JJ];
                break;
                case 2 :
                    sDateStr += pKeyword[NF_KEY_JJJJ];
                break;
                default:
                    DBG_ERRORFILE( "unknown nShortLongYear" );
            }
            //if ( nShortLongYear != -1 )
            //  lcl_ImplAddStringMaybeQuoted( sDateStr,
            //      pIntl->GetLongDateYearSep(), pCharClass );

        }
        break;
        case YMD:
        {
            switch ( nShortLongYear )
            {
                case -1 :
                break;
                case 0 :
                    if ( pIntl->IsLongDateCentury() )
                        sDateStr += pKeyword[NF_KEY_JJJJ];
                    else
                        sDateStr += pKeyword[NF_KEY_JJ];
                break;
                case 1 :
                    sDateStr += pKeyword[NF_KEY_JJ];
                break;
                case 2 :
                    sDateStr += pKeyword[NF_KEY_JJJJ];
                break;
                default:
                    DBG_ERRORFILE( "unknown nShortLongYear" );
            }
            if ( nShortLongYear != -1 )
                lcl_ImplAddStringMaybeQuoted( sDateStr,
                    pIntl->GetLongDateYearSep(), pCharClass );

            switch ( nShortLongMonth )
            {
                case -1 :
                break;
                case 0 :
                    switch ( pIntl->GetLongDateMonthFormat() )
                    {
                        case MONTH_NORMAL :
                            sDateStr += pKeyword[NF_KEY_M];
                        break;
                        case MONTH_ZERO :
                            sDateStr += pKeyword[NF_KEY_MM];
                        break;
                        case MONTH_SHORT :
                            sDateStr += pKeyword[NF_KEY_MMM];
                        break;
                        case MONTH_LONG :
                            sDateStr += pKeyword[NF_KEY_MMMM];
                        break;
                        default:
                            DBG_ERRORFILE( "unknown LongDateMonthFormat" );
                    }
                break;
                case 1 :
                    sDateStr += pKeyword[NF_KEY_MMM];
                break;
                case 2 :
                    sDateStr += pKeyword[NF_KEY_MMMM];
                break;
                default:
                    DBG_ERRORFILE( "unknown nShortLongMonth" );
            }
            if ( nShortLongMonth != -1 )
                lcl_ImplAddStringMaybeQuoted( sDateStr,
                    pIntl->GetLongDateMonthSep(), pCharClass );

            switch ( nShortLongDay )
            {
                case -1 :
                break;
                case 0 :
                    if ( pIntl->IsLongDateDayLeadingZero() )
                        sDateStr += pKeyword[NF_KEY_TT];
                    else
                        sDateStr += pKeyword[NF_KEY_T];
                break;
                case 1 :
                    sDateStr += pKeyword[NF_KEY_T];
                break;
                case 2 :
                    sDateStr += pKeyword[NF_KEY_TT];
                break;
                default:
                    DBG_ERRORFILE( "unknown nShortLongDay" );
            }
            //if ( nShortLongDay != -1 )
            //  lcl_ImplAddStringMaybeQuoted( sDateStr,
            //      pIntl->GetLongDateDaySep(), pCharClass );

        }
        break;
        default:
            DBG_ERRORFILE( "unknown LongDateFormat" );
    }
}

void SvNumberFormatter::ImpGetPosCurrFormat(XubString& sPosStr)
{
    NfCurrencyEntry::CompletePositiveFormatString( sPosStr,
        pIntl->GetCurrSymbol(), pIntl->GetCurrPositiveFormat() );
}

void SvNumberFormatter::ImpGetNegCurrFormat(XubString& sNegStr)
{
    NfCurrencyEntry::CompleteNegativeFormatString( sNegStr,
        pIntl->GetCurrSymbol(), pIntl->GetCurrNegativeFormat() );
}

void SvNumberFormatter::GenerateFormat(XubString& sString,
                                       ULONG nIndex,
                                       LanguageType eLnge,
                                       BOOL bThousand,
                                       BOOL IsRed,
                                       USHORT nPrecision,
                                       USHORT nAnzLeading)
{
    if (eLnge == LANGUAGE_DONTKNOW)
        eLnge = SysLnge;
    short eType = GetType(nIndex);
    USHORT i;
    ImpGenerateCL(eLnge);               // ggfs. neu Standard-
                                    // formate anlegen
    sString.Erase();

    if (nAnzLeading == 0)
    {
        if (!bThousand)
            sString += '#';
        else
        {
            sString += '#';
            sString += pIntl->GetNumThousandSep();
            sString += '#';
            sString += '#';
            sString += '#';
        }
    }
    else
    {
        for (i = 0; i < nAnzLeading; i++)
        {
            if (bThousand && i%3 == 0 && i > 0)
                sString.Insert(pIntl->GetNumThousandSep(),0);
            sString.Insert('0',0);
        }
        if (bThousand && nAnzLeading < 4)
        {
            for (i = nAnzLeading; i < 4; i++)
            {
                if (bThousand && i%3 == 0)
                    sString.Insert(pIntl->GetNumThousandSep(),0);
                sString.Insert('#',0);
            }
        }
    }
    if (nPrecision > 0)
    {
                sString += pIntl->GetNumDecimalSep();
                for (i = 0; i < nPrecision; i++)
                    sString += '0';
    }
    if (eType == NUMBERFORMAT_PERCENT)
        sString += '%';
    else if (eType == NUMBERFORMAT_CURRENCY)
    {
        XubString sNegStr = sString;
        XubString aCurr;
        const NfCurrencyEntry* pEntry;
        BOOL bBank;
        if ( GetNewCurrencySymbolString( nIndex, aCurr, &pEntry, &bBank ) )
        {
            if ( pEntry )
            {
                USHORT nPosiForm = NfCurrencyEntry::GetEffectivePositiveFormat(
                    pIntl->GetCurrPositiveFormat(),
                    pEntry->GetPositiveFormat(), bBank );
                USHORT nNegaForm = NfCurrencyEntry::GetEffectiveNegativeFormat(
                    pIntl->GetCurrNegativeFormat(),
                    pEntry->GetNegativeFormat(), bBank );
                pEntry->CompletePositiveFormatString( sString, bBank,
                    nPosiForm );
                pEntry->CompleteNegativeFormatString( sNegStr, bBank,
                    nNegaForm );
            }
            else
            {   // wir gehen von einem Banksymbol aus
                USHORT nPosiForm = NfCurrencyEntry::GetEffectivePositiveFormat(
                    pIntl->GetCurrPositiveFormat(),
                    pIntl->GetCurrPositiveFormat(), TRUE );
                USHORT nNegaForm = NfCurrencyEntry::GetEffectiveNegativeFormat(
                    pIntl->GetCurrNegativeFormat(),
                    pIntl->GetCurrNegativeFormat(), TRUE );
                NfCurrencyEntry::CompletePositiveFormatString( sString, aCurr,
                    nPosiForm );
                NfCurrencyEntry::CompleteNegativeFormatString( sNegStr, aCurr,
                    nNegaForm );
            }
        }
        else
        {
            ImpGetPosCurrFormat(sString);
            ImpGetNegCurrFormat(sNegStr);
        }
        if (IsRed)
        {
            sString += ';';
            sString += '[';
            sString += pFormatScanner->GetRedString();
            sString += ']';
        }
        else
            sString += ';';
        sString += sNegStr;
    }
    if (IsRed && eType != NUMBERFORMAT_CURRENCY)
    {
        XubString sTmpStr = sString;
        sTmpStr += ';';
        sTmpStr += '[';
        sTmpStr += pFormatScanner->GetRedString();
        sTmpStr += ']';
        sTmpStr += '-';
        sTmpStr +=sString;
        sString = sTmpStr;
    }
}

BOOL SvNumberFormatter::IsUserDefined(const XubString& sStr,
                                      LanguageType eLnge)
{
    if (eLnge == LANGUAGE_DONTKNOW)
        eLnge = SysLnge;
    ULONG CLOffset = ImpGenerateCL(eLnge);              // ggfs. neu Standard-
                                                    // formate anlegen
    eLnge = ActLnge;
    ULONG nKey = ImpIsEntry(sStr, CLOffset, eLnge);
    if (nKey == NUMBERFORMAT_ENTRY_NOT_FOUND)
        return TRUE;
    SvNumberformat* pEntry = aFTable.Get(nKey);
    if ( pEntry && ((pEntry->GetType() & NUMBERFORMAT_DEFINED) != 0) )
        return TRUE;
    return FALSE;
}

ULONG SvNumberFormatter::GetEntryKey(const XubString& sStr,
                                     LanguageType eLnge)
{
    if (eLnge == LANGUAGE_DONTKNOW)
        eLnge = SysLnge;
    ULONG CLOffset = ImpGenerateCL(eLnge);              // ggfs. neu Standard-
                                                    // formate anlegen
    return ImpIsEntry(sStr, CLOffset, eLnge);
}

ULONG SvNumberFormatter::GetStandardIndex(LanguageType eLnge)
{
    if (eLnge == LANGUAGE_DONTKNOW)
        eLnge = SysLnge;
    return GetStandardFormat(NUMBERFORMAT_NUMBER, eLnge);
}

short SvNumberFormatter::GetType(ULONG nFIndex)
{
    short eType;
    SvNumberformat* pFormat = (SvNumberformat*) aFTable.Get(nFIndex);
    if (!pFormat)
        eType = NUMBERFORMAT_UNDEFINED;
    else
    {
        eType = pFormat->GetType() &~NUMBERFORMAT_DEFINED;
        if (eType == 0)
            eType = NUMBERFORMAT_DEFINED;
    }
    return eType;
}

void SvNumberFormatter::ClearMergeTable()
{
    ULONG* pIndex = (ULONG*) pMergeTable->First();
    while (pIndex)
    {
        delete pIndex;
        pIndex = pMergeTable->Next();
    }
    pMergeTable->Clear();
}

SvULONGTable* SvNumberFormatter::MergeFormatter(SvNumberFormatter& rTable)
{
    ClearMergeTable();
    ULONG nCLOffset, nOldKey, nOffset, nNewKey;
    ULONG* pNewIndex;
    SvNumberformat* pNewEntry;
    SvNumberformat* pFormat = rTable.aFTable.First();
    while (pFormat)
    {
        nOldKey = rTable.aFTable.GetCurKey();
        nOffset = nOldKey % SV_COUNTRY_LANGUAGE_OFFSET;     // relativIndex
        if (nOffset == 0)                                   // 1. Format von CL
            nCLOffset = ImpGenerateCL(pFormat->GetLanguage());

        if (nOffset <= SV_MAX_ANZ_STANDARD_FORMATE)     // Std.form.
        {
            nNewKey = nCLOffset + nOffset;
            if (!aFTable.Get(nNewKey))                  // noch nicht da
            {
//              pNewEntry = new SvNumberformat(*pFormat);   // Copy reicht nicht !!!
                pNewEntry = new SvNumberformat( *pFormat, *pFormatScanner );
                if (!aFTable.Insert(nNewKey, pNewEntry))
                    delete pNewEntry;
            }
            if (nNewKey != nOldKey)                     // neuer Index
            {
                pNewIndex = new ULONG(nNewKey);
                if (!pMergeTable->Insert(nOldKey,pNewIndex))
                    delete pNewIndex;
            }
        }
        else                                            // benutzerdef.
        {
//          pNewEntry = new SvNumberformat(*pFormat);   // Copy reicht nicht !!!
            pNewEntry = new SvNumberformat( *pFormat, *pFormatScanner );
            nNewKey = ImpIsEntry(pNewEntry->GetFormatstring(),
                              nCLOffset,
                              pFormat->GetLanguage());
            if (nNewKey != NUMBERFORMAT_ENTRY_NOT_FOUND) // schon vorhanden
                delete pNewEntry;
            else
            {
                SvNumberformat* pStdFormat =
                        (SvNumberformat*) aFTable.Get(nCLOffset + ZF_STANDARD);
                ULONG nPos = nCLOffset + pStdFormat->GetLastInsertKey();
                nNewKey = nPos+1;
                if (nPos - nCLOffset >= SV_COUNTRY_LANGUAGE_OFFSET)
                {
#ifndef DOS
                    Sound::Beep();
#endif
                    DBG_ERROR(
                        "SvNumberFormatter:: Zu viele Formate pro CL");
                    delete pNewEntry;
                }
                else if (!aFTable.Insert(nNewKey, pNewEntry))
                        delete pNewEntry;
                else
                    pStdFormat->SetLastInsertKey((USHORT) (nNewKey - nCLOffset));
            }
            if (nNewKey != nOldKey)                     // neuer Index
            {
                pNewIndex = new ULONG(nNewKey);
                if (!pMergeTable->Insert(nOldKey,pNewIndex))
                    delete pNewIndex;
            }
        }
        pFormat = rTable.aFTable.Next();
    }
    return pMergeTable;
}


ULONG SvNumberFormatter::GetFormatForLanguageIfBuiltIn( ULONG nFormat,
        LanguageType eLnge )
{
    if ( eLnge == LANGUAGE_DONTKNOW )
        eLnge = SysLnge;
    if ( nFormat < SV_COUNTRY_LANGUAGE_OFFSET && eLnge == SysLnge )
        return nFormat;     // es bleibt wie es ist
    ULONG nOffset = nFormat % SV_COUNTRY_LANGUAGE_OFFSET;       // relativIndex
    if ( nOffset > SV_MAX_ANZ_STANDARD_FORMATE )
        return nFormat;                 // kein eingebautes Format
    ULONG nCLOffset = ImpGenerateCL(eLnge);     // ggbf. generieren
    return nCLOffset + nOffset;
}


ULONG SvNumberFormatter::GetFormatIndex( NfIndexTableOffset nTabOff,
        LanguageType eLnge )
{
    if ( nTabOff >= NF_INDEX_TABLE_ENTRIES
            || theIndexTable[nTabOff] == NUMBERFORMAT_ENTRY_NOT_FOUND )
        return NUMBERFORMAT_ENTRY_NOT_FOUND;
    if ( eLnge == LANGUAGE_DONTKNOW )
        eLnge = SysLnge;
    ULONG nCLOffset = ImpGenerateCL(eLnge);     // ggbf. generieren
    return nCLOffset + theIndexTable[nTabOff];
}


NfIndexTableOffset SvNumberFormatter::GetIndexTableOffset( ULONG nFormat ) const
{
    ULONG nOffset = nFormat % SV_COUNTRY_LANGUAGE_OFFSET;       // relativIndex
    if ( nOffset > SV_MAX_ANZ_STANDARD_FORMATE )
        return NF_INDEX_TABLE_ENTRIES;      // kein eingebautes Format
    for ( USHORT j = 0; j < NF_INDEX_TABLE_ENTRIES; j++ )
    {
        if ( theIndexTable[j] == nOffset )
            return (NfIndexTableOffset) j;
    }
    return NF_INDEX_TABLE_ENTRIES;      // bad luck
}


void SvNumberFormatter::SetYear2000( USHORT nVal )
{
    pStringScanner->SetYear2000( nVal );
}


USHORT SvNumberFormatter::GetYear2000() const
{
    return pStringScanner->GetYear2000();
}


USHORT SvNumberFormatter::ExpandTwoDigitYear( USHORT nYear ) const
{
    if ( nYear < 100 )
        return SvNumberFormatter::ExpandTwoDigitYear( nYear,
            pStringScanner->GetYear2000() );
    return nYear;
}


// static
USHORT SvNumberFormatter::GetYear2000Default()
{
    return Application::GetSettings().GetMiscSettings().GetTwoDigitYearStart();
}


const NfCurrencyTable& SvNumberFormatter::GetTheCurrencyTable() const
{
    while ( !bCurrencyTableInitialized )
        SvNumberFormatter::ImpInitCurrencyTable();
    return theCurrencyTable;
}


const NfCurrencyEntry* SvNumberFormatter::MatchSystemCurrency() const
{
    const NfCurrencyTable& rTable = GetTheCurrencyTable();
    return nSystemCurrencyPosition ? rTable[nSystemCurrencyPosition] : NULL;
}


const NfCurrencyEntry& SvNumberFormatter::GetCurrencyEntry( LanguageType eLang ) const
{
    if ( eLang == LANGUAGE_DONTKNOW )
        eLang = SysLnge;
    if ( eLang == LANGUAGE_SYSTEM )
    {
        const NfCurrencyEntry* pCurr = MatchSystemCurrency();
        return pCurr ? *pCurr : *(GetTheCurrencyTable()[0]);
    }
    else
    {
        const NfCurrencyTable& rTable = GetTheCurrencyTable();
        USHORT nCount = rTable.Count();
        const NfCurrencyEntryPtr* ppData = rTable.GetData();
        for ( USHORT j = 0; j < nCount; j++, ppData++ )
        {
            if ( (*ppData)->GetLanguage() == eLang )
                return **ppData;
        }
        return *(rTable[0]);
    }
}


ULONG SvNumberFormatter::ImpGetDefaultSystemCurrencyFormat()
{
    if ( nDefaultCurrencyFormat == NUMBERFORMAT_ENTRY_NOT_FOUND )
    {
        xub_StrLen nCheck;
        short nType;
        NfWSStringsDtor aCurrList;
        USHORT nDefault = GetCurrencyFormatStrings( aCurrList,
            GetCurrencyEntry( LANGUAGE_SYSTEM ), FALSE );
        DBG_ASSERT( nDefault < aCurrList.Count(), "wo ist das NewCurrency Standard Format?!?" );
        // wenn bereits geladen oder userdefined wird nDefaultCurrencyFormat
        // hierbei auf den richtigen Wert gesetzt
        PutEntry( *aCurrList.GetObject( nDefault ), nCheck, nType,
            nDefaultCurrencyFormat, LANGUAGE_SYSTEM );
        DBG_ASSERT( nCheck == 0, "NewCurrency CheckError" );
        DBG_ASSERT( nDefaultCurrencyFormat != NUMBERFORMAT_ENTRY_NOT_FOUND,
            "nDefaultCurrencyFormat == NUMBERFORMAT_ENTRY_NOT_FOUND" );
    }
    return nDefaultCurrencyFormat;
}


BOOL SvNumberFormatter::GetNewCurrencySymbolString( ULONG nFormat,
            XubString& rStr, const NfCurrencyEntry** ppEntry /* = NULL */,
            BOOL* pBank /* = NULL */ ) const
{
    rStr.Erase();
    if ( ppEntry )
        *ppEntry = NULL;
    if ( pBank )
        *pBank = FALSE;
    SvNumberformat* pFormat = (SvNumberformat*) aFTable.Get( nFormat );
    if ( pFormat )
    {
        XubString aSymbol, aExtension;
        if ( pFormat->GetNewCurrencySymbol( aSymbol, aExtension ) )
        {
            xub_StrLen nExtLen = aExtension.Len();
            if ( ppEntry )
            {
                LanguageType eExtLang;
                if ( nExtLen )
                {
                    ByteString aTmp( aExtension, 1, nExtLen-1, gsl_getSystemTextEncoding() );
                    char* pEnd;
                    long nExtLang = strtol( aTmp.GetBuffer(), &pEnd, 16 );
                    eExtLang = (LanguageType) (
                        (*pEnd || nExtLang == LONG_MIN || nExtLang == LONG_MAX) ?
                        LANGUAGE_DONTKNOW : nExtLang );
                }
                else
                    eExtLang = LANGUAGE_DONTKNOW;
                const NfCurrencyEntry* pFoundEntry = NULL;
                BOOL bFoundBank = FALSE;
                const NfCurrencyTable& rTable = GetTheCurrencyTable();
                USHORT nCount = rTable.Count();
                const NfCurrencyEntryPtr* ppData = rTable.GetData();
                for ( USHORT j = 0; j < nCount; j++, ppData++ )
                {
                    LanguageType eLang = (*ppData)->GetLanguage();
                    if ( !nExtLen || eLang == eExtLang ||
                            (eLang == LANGUAGE_SYSTEM && eExtLang == LANGUAGE_DONTKNOW) )
                    {
                        BOOL bFound;
                        if ( (*ppData)->GetSymbol() == aSymbol )
                        {
                            bFound = TRUE;
                            bFoundBank = FALSE;
                        }
                        else if ( (*ppData)->GetBankSymbol() == aSymbol )
                        {
                            bFound = TRUE;
                            bFoundBank = TRUE;
                        }
                        else
                            bFound = FALSE;
                        if ( bFound )
                        {
                            if ( pFoundEntry && pFoundEntry != *ppData )
                            {
                                pFoundEntry = NULL;
                                break;  // for, nicht eindeutig
                            }
                            if ( j == 0 )
                            {   // erster Eintrag System
                                pFoundEntry = MatchSystemCurrency();
                                if ( pFoundEntry )
                                    break;  // for, selbst wenn es mehrere
                                    // geben sollte, wird dieser gemeint sein
                                else
                                    pFoundEntry = *ppData;
                            }
                            else
                                pFoundEntry = *ppData;
                        }
                    }
                }
                if ( pFoundEntry )
                {
                    *ppEntry = pFoundEntry;
                    if ( pBank )
                        *pBank = bFoundBank;
                    pFoundEntry->BuildSymbolString( rStr, bFoundBank );
                }
            }
            if ( !rStr.Len() )
            {   // analog zu BuildSymbolString
                rStr  = '[';
                rStr += '$';
                if ( aSymbol.Search( '-' ) != STRING_NOTFOUND ||
                        aSymbol.Search( ']' ) != STRING_NOTFOUND )
                {
                    rStr += '"';
                    rStr += aSymbol;
                    rStr += '"';
                }
                else
                    rStr += aSymbol;
                if ( nExtLen )
                    rStr += aExtension;
                rStr += ']';
            }
            return TRUE;
        }
    }
    return FALSE;
}


#ifndef PRODUCT
void lcl_CheckCurrencySymbolPosition( const NfCurrencyEntry& rCurr )
{
    short nPos = -1;        // -1:=unknown, 0:=vorne, 1:=hinten
    short nNeg = -1;
    switch ( rCurr.GetPositiveFormat() )
    {
        case 0:                                         // $1
            nPos = 0;
        break;
        case 1:                                         // 1$
            nPos = 1;
        break;
        case 2:                                         // $ 1
            nPos = 0;
        break;
        case 3:                                         // 1 $
            nPos = 1;
        break;
        default:
            DBG_ERRORFILE("lcl_CheckCurrencySymbolPosition: unknown PositiveFormat");
        break;
    }
    switch ( rCurr.GetNegativeFormat() )
    {
        case 0:                                         // ($1)
            nNeg = 0;
        break;
        case 1:                                         // -$1
            nNeg = 0;
        break;
        case 2:                                         // $-1
            nNeg = 0;
        break;
        case 3:                                         // $1-
            nNeg = 0;
        break;
        case 4:                                         // (1$)
            nNeg = 1;
        break;
        case 5:                                         // -1$
            nNeg = 1;
        break;
        case 6:                                         // 1-$
            nNeg = 1;
        break;
        case 7:                                         // 1$-
            nNeg = 1;
        break;
        case 8:                                         // -1 $
            nNeg = 1;
        break;
        case 9:                                         // -$ 1
            nNeg = 0;
        break;
        case 10:                                        // 1 $-
            nNeg = 1;
        break;
        case 11:                                        // $ -1
            nNeg = 0;
        break;
        case 12 :                                       // $ 1-
            nNeg = 0;
        break;
        case 13 :                                       // 1- $
            nNeg = 1;
        break;
        case 14 :                                       // ($ 1)
            nNeg = 0;
        break;
        case 15 :                                       // (1 $)
            nNeg = 1;
        break;
        default:
            DBG_ERRORFILE("lcl_CheckCurrencySymbolPosition: unknown NegativeFormat");
        break;
    }
    if ( nPos >= 0 && nNeg >= 0 && nPos != nNeg )
    {
        ByteString aStr( "ER->TH: positions of currency symbols differ\nLanguage: " );
        aStr += ByteString::CreateFromInt32( rCurr.GetLanguage() );
        aStr += " <";
        aStr += ByteString( rCurr.GetSymbol(), RTL_TEXTENCODING_UTF8 );
        aStr += "> positive: ";
        aStr += ByteString::CreateFromInt32( rCurr.GetPositiveFormat() );
        aStr += ( nPos ? " (postfix)" : " (prefix)" );
        aStr += ", negative: ";
        aStr += ByteString::CreateFromInt32( rCurr.GetNegativeFormat() );
        aStr += ( nNeg ? " (postfix)" : " (prefix)" );
#if 0
        DBG_ERRORFILE( aStr.GetBuffer() );
#endif
    }
}
#endif


// static
void SvNumberFormatter::ImpInitCurrencyTable()
{
    // racing condition moeglich:
    // while ( !bCurrencyTableInitialized )
    //      ImpInitCurrencyTable();
    static BOOL bInitializing = FALSE;
    if ( bCurrencyTableInitialized || bInitializing )
        return ;
    bInitializing = TRUE;

    LanguageType eSysLang = System::GetLanguage();
    NfCurrencyEntryPtr pEntry;
    USHORT n = International::GetAvailableFormatCount();
    for ( USHORT j = 0; j < n; j++ )
    {
        LanguageType eLang = International::GetAvailableFormat( j );
#ifdef DEBUG
        LanguageType eReal = International::GetRealLanguage( eLang );
        LanguageType eNeut = International::GetNeutralLanguage( eLang );
        if ( eReal != eLang )
            BOOL bBreak = TRUE;
        if ( eNeut != eLang )
            BOOL bBreak = TRUE;
#endif
        International* pIntl = new International( eLang );
        pEntry = new NfCurrencyEntry( *pIntl );
#ifndef PRODUCT
        lcl_CheckCurrencySymbolPosition( *pEntry );
#endif
        theCurrencyTable.Insert( pEntry, j );
        delete pIntl;
        if ( !nSystemCurrencyPosition && pEntry->GetLanguage() == eSysLang )
            nSystemCurrencyPosition = j;
    }
    DBG_ASSERT( theCurrencyTable.Count(),
        "SvNumberFormatter::ImpInitCurrencyTable: kein NfCurrencyEntry ?!?" );
    // erster Eintrag ist System
    // landesunabhaengigen EURo an zweiter Stelle einfuegen
    pEntry = new NfCurrencyEntry;
    pEntry->SetEuro();
    theCurrencyTable.Insert( pEntry, nCurrencyTableEuroPosition );
    DBG_ASSERT( theCurrencyTable[0]->GetLanguage() == LANGUAGE_SYSTEM,
        "SvNumberFormatter::ImpInitCurrencyTable: erster Eintrag nicht System" );
    DBG_ASSERT( nSystemCurrencyPosition, "Regional Settings Language nicht in TH's Tabellen" );
    if ( nSystemCurrencyPosition )
    {
        if ( nSystemCurrencyPosition >= nCurrencyTableEuroPosition )
            nSystemCurrencyPosition++;
        if ( theCurrencyTable[nSystemCurrencyPosition]->GetSymbol() !=
                theCurrencyTable[0]->GetSymbol() )
        {
            if ( theCurrencyTable[0]->IsEuro() )
                nSystemCurrencyPosition = nCurrencyTableEuroPosition;
            else
                nSystemCurrencyPosition = 0;
        }
        if ( nSystemCurrencyPosition )
            theCurrencyTable[nSystemCurrencyPosition]->ApplyVariableInformation(
                *theCurrencyTable[0] );
    }
    bCurrencyTableInitialized = TRUE;
    bInitializing = FALSE;
}


USHORT SvNumberFormatter::GetCurrencyFormatStrings( NfWSStringsDtor& rStrArr,
            const NfCurrencyEntry& rCurr, BOOL bBank ) const
{
    USHORT nDefault = 0;
    if ( bBank )
    {   // nur Bankensymbole
        XubString aPositiveBank, aNegativeBank;
        rCurr.BuildPositiveFormatString( aPositiveBank, TRUE, *pIntl, 1 );
        rCurr.BuildNegativeFormatString( aNegativeBank, TRUE, *pIntl, 1 );

        WSStringPtr pFormat1 = new String( aPositiveBank );
        *pFormat1 += ';';
        WSStringPtr pFormat2 = new String( *pFormat1 );

        XubString aRed( '[' );
        aRed += pFormatScanner->GetRedString();
        aRed += ']';

        *pFormat2 += aRed;

        *pFormat1 += aNegativeBank;
        *pFormat2 += aNegativeBank;

        rStrArr.Insert( pFormat1, rStrArr.Count() );
        rStrArr.Insert( pFormat2, rStrArr.Count() );
        nDefault = rStrArr.Count() - 1;
    }
    else
    {   // gemischte Formate wie in SvNumberFormatter::ImpGenerateFormats
        // aber keine doppelten, wenn keine Nachkommastellen in Waehrung
        XubString aPositive, aNegative, aPositiveNoDec, aNegativeNoDec,
            aPositiveDashed, aNegativeDashed;
        WSStringPtr pFormat1, pFormat2, pFormat3, pFormat4, pFormat5;

        XubString aRed( '[' );
        aRed += pFormatScanner->GetRedString();
        aRed += ']';

        rCurr.BuildPositiveFormatString( aPositive, FALSE, *pIntl, 1 );
        rCurr.BuildNegativeFormatString( aNegative, FALSE, *pIntl, 1 );
        if ( rCurr.GetDigits() )
        {
            rCurr.BuildPositiveFormatString( aPositiveNoDec, FALSE, *pIntl, 0 );
            rCurr.BuildNegativeFormatString( aNegativeNoDec, FALSE, *pIntl, 0 );
            rCurr.BuildPositiveFormatString( aPositiveDashed, FALSE, *pIntl, 2 );
            rCurr.BuildNegativeFormatString( aNegativeDashed, FALSE, *pIntl, 2 );

            pFormat1 = new String( aPositiveNoDec );
            *pFormat1 += ';';
            pFormat3 = new String( *pFormat1 );
            pFormat5 = new String( aPositiveDashed );
            *pFormat5 += ';';

            *pFormat1 += aNegativeNoDec;

            *pFormat3 += aRed;
            *pFormat5 += aRed;

            *pFormat3 += aNegativeNoDec;
            *pFormat5 += aNegativeDashed;
        }
        else
        {
            pFormat1 = NULL;
            pFormat3 = NULL;
            pFormat5 = NULL;
        }

        pFormat2 = new String( aPositive );
        *pFormat2 += ';';
        pFormat4 = new String( *pFormat2 );

        *pFormat2 += aNegative;

        *pFormat4 += aRed;
        *pFormat4 += aNegative;

        if ( pFormat1 )
            rStrArr.Insert( pFormat1, rStrArr.Count() );
        rStrArr.Insert( pFormat2, rStrArr.Count() );
        if ( pFormat3 )
            rStrArr.Insert( pFormat3, rStrArr.Count() );
        rStrArr.Insert( pFormat4, rStrArr.Count() );
        nDefault = rStrArr.Count() - 1;
        if ( pFormat5 )
            rStrArr.Insert( pFormat5, rStrArr.Count() );
    }
    return nDefault;
}


//--- NfCurrencyEntry ----------------------------------------------------

NfCurrencyEntry::NfCurrencyEntry()
    :   eLanguage( LANGUAGE_DONTKNOW ),
        nPositiveFormat(3),
        nNegativeFormat(8),
        nDigits(2),
        cZeroChar('0')
{
}


NfCurrencyEntry::NfCurrencyEntry( const International& rIntl )
{
    aSymbol         = rIntl.GetCurrSymbol();
    aBankSymbol     = rIntl.GetCurrBankSymbol();
    eLanguage       = rIntl.GetLanguage();
    nPositiveFormat = rIntl.GetCurrPositiveFormat();
    nNegativeFormat = rIntl.GetCurrNegativeFormat();
    nDigits         = rIntl.GetCurrDigits();
    cZeroChar       = rIntl.GetCurrZeroChar();
}


void NfCurrencyEntry::SetEuro()
{
    aSymbol = NfCurrencyEntry::GetEuroSymbol();
    aBankSymbol.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "EUR" ) );
    eLanguage       = LANGUAGE_DONTKNOW;
    nPositiveFormat = 3;
    nNegativeFormat = 8;
    nDigits         = 2;
    cZeroChar       = '0';
}


BOOL NfCurrencyEntry::IsEuro() const
{
    if ( aBankSymbol.EqualsAscii( "EUR" ) )
        return TRUE;
    XubString aEuro( NfCurrencyEntry::GetEuroSymbol() );
    return aSymbol == aEuro;
}


void NfCurrencyEntry::ApplyVariableInformation( const NfCurrencyEntry& r )
{
    nPositiveFormat = r.nPositiveFormat;
    nNegativeFormat = r.nNegativeFormat;
    nDigits         = r.nDigits;
    cZeroChar       = r.cZeroChar;
}


void NfCurrencyEntry::BuildSymbolString( XubString& rStr, BOOL bBank,
            BOOL bWithoutExtension ) const
{
    rStr  = '[';
    rStr += '$';
    if ( bBank )
        rStr += aBankSymbol;
    else
    {
        if ( aSymbol.Search( '-' ) != STRING_NOTFOUND || aSymbol.Search( ']' ) != STRING_NOTFOUND )
        {
            rStr += '"';
            rStr += aSymbol;
            rStr += '"';
        }
        else
            rStr += aSymbol;
        if ( !bWithoutExtension && eLanguage != LANGUAGE_DONTKNOW && eLanguage != LANGUAGE_SYSTEM )
        {
            rStr += '-';
            rStr += UniString::CreateFromInt32( sal_Int32( eLanguage ), 16 );
        }
    }
    rStr += ']';
}


void NfCurrencyEntry::Impl_BuildFormatStringNumChars( XubString& rStr,
            const International& rIntl, USHORT nDecimalFormat ) const
{
    rStr.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "###0" ) );
    rStr.Insert( rIntl.GetNumThousandSep(), 1 );
    if ( nDecimalFormat && nDigits )
    {
        rStr += rIntl.GetNumDecimalSep();
        rStr.Expand( rStr.Len() + nDigits, (nDecimalFormat == 2 ? '-' : cZeroChar) );
    }
}


void NfCurrencyEntry::BuildPositiveFormatString( XubString& rStr, BOOL bBank,
            const International& rIntl, USHORT nDecimalFormat ) const
{
    Impl_BuildFormatStringNumChars( rStr, rIntl, nDecimalFormat );
    USHORT nPosiForm = NfCurrencyEntry::GetEffectivePositiveFormat(
        rIntl.GetCurrPositiveFormat(), nPositiveFormat, bBank );
    CompletePositiveFormatString( rStr, bBank, nPosiForm );
}


void NfCurrencyEntry::BuildNegativeFormatString( XubString& rStr, BOOL bBank,
            const International& rIntl, USHORT nDecimalFormat ) const
{
    Impl_BuildFormatStringNumChars( rStr, rIntl, nDecimalFormat );
    USHORT nNegaForm = NfCurrencyEntry::GetEffectiveNegativeFormat(
        rIntl.GetCurrNegativeFormat(), nNegativeFormat, bBank );
    CompleteNegativeFormatString( rStr, bBank, nNegaForm );
}


void NfCurrencyEntry::CompletePositiveFormatString( XubString& rStr, BOOL bBank,
            USHORT nPosiForm ) const
{
    XubString aSymStr;
    BuildSymbolString( aSymStr, bBank );
    NfCurrencyEntry::CompletePositiveFormatString( rStr, aSymStr, nPosiForm );
}


void NfCurrencyEntry::CompleteNegativeFormatString( XubString& rStr, BOOL bBank,
            USHORT nNegaForm ) const
{
    XubString aSymStr;
    BuildSymbolString( aSymStr, bBank );
    NfCurrencyEntry::CompleteNegativeFormatString( rStr, aSymStr, nNegaForm );
}


// static
void NfCurrencyEntry::CompletePositiveFormatString( XubString& rStr,
        const XubString& rSymStr, USHORT nPositiveFormat )
{
    switch( nPositiveFormat )
    {
        case 0:                                         // $1
            rStr.Insert( rSymStr , 0 );
        break;
        case 1:                                         // 1$
            rStr += rSymStr;
        break;
        case 2:                                         // $ 1
        {
            rStr.Insert( ' ', 0 );
            rStr.Insert( rSymStr, 0 );
        }
        break;
        case 3:                                         // 1 $
        {
            rStr += ' ';
            rStr += rSymStr;
        }
        break;
        default:
            DBG_ERROR("NfCurrencyEntry::CompletePositiveFormatString: unknown option");
        break;
    }
}


// static
void NfCurrencyEntry::CompleteNegativeFormatString( XubString& rStr,
        const XubString& rSymStr, USHORT nNegativeFormat )
{
    switch( nNegativeFormat )
    {
        case 0:                                         // ($1)
        {
            rStr.Insert( rSymStr, 0);
            rStr.Insert('(',0);
            rStr += ')';
        }
        break;
        case 1:                                         // -$1
        {
            rStr.Insert( rSymStr, 0);
            rStr.Insert('-',0);
        }
        break;
        case 2:                                         // $-1
        {
            rStr.Insert('-',0);
            rStr.Insert( rSymStr, 0);
        }
        break;
        case 3:                                         // $1-
        {
            rStr.Insert( rSymStr, 0);
            rStr += '-';
        }
        break;
        case 4:                                         // (1$)
        {
            rStr.Insert('(',0);
            rStr += rSymStr;
            rStr += ')';
        }
        break;
        case 5:                                         // -1$
        {
            rStr += rSymStr;
            rStr.Insert('-',0);
        }
        break;
        case 6:                                         // 1-$
        {
            rStr += '-';
            rStr += rSymStr;
        }
        break;
        case 7:                                         // 1$-
        {
            rStr += rSymStr;
            rStr += '-';
        }
        break;
        case 8:                                         // -1 $
        {
            rStr += ' ';
            rStr += rSymStr;
            rStr.Insert('-',0);
        }
        break;
        case 9:                                         // -$ 1
        {
            rStr.Insert(' ',0);
            rStr.Insert( rSymStr, 0);
            rStr.Insert('-',0);
        }
        break;
        case 10:                                        // 1 $-
        {
            rStr += ' ';
            rStr += rSymStr;
            rStr += '-';
        }
        break;
        case 11:                                        // $ -1
        {
            XubString aTmp( rSymStr );
            aTmp += ' ';
            aTmp += '-';
            rStr.Insert( aTmp, 0 );
        }
        break;
        case 12 :                                       // $ 1-
        {
            rStr.Insert(' ', 0);
            rStr.Insert( rSymStr, 0);
            rStr += '-';
        }
        break;
        case 13 :                                       // 1- $
        {
            rStr += '-';
            rStr += ' ';
            rStr += rSymStr;
        }
        break;
        case 14 :                                       // ($ 1)
        {
            rStr.Insert(' ',0);
            rStr.Insert( rSymStr, 0);
            rStr.Insert('(',0);
            rStr += ')';
        }
        break;
        case 15 :                                       // (1 $)
        {
            rStr.Insert('(',0);
            rStr += ' ';
            rStr += rSymStr;
            rStr += ')';
        }
        break;
        default:
            DBG_ERROR("NfCurrencyEntry::CompleteNegativeFormatString: unknown option");
        break;
    }
}


// static
USHORT NfCurrencyEntry::GetEffectivePositiveFormat( USHORT nIntlFormat,
            USHORT nCurrFormat, BOOL bBank )
{
    if ( bBank )
    {
#if NF_BANKSYMBOL_FIX_POSITION
        return 3;
#else
        switch ( nIntlFormat )
        {
            case 0:                                         // $1
                nIntlFormat = 2;                            // $ 1
            break;
            case 1:                                         // 1$
                nIntlFormat = 3;                            // 1 $
            break;
            case 2:                                         // $ 1
            break;
            case 3:                                         // 1 $
            break;
        }
        return nIntlFormat;
#endif
    }
    else
        return nCurrFormat;
}


// nur aufrufen, wenn nCurrFormat wirklich mit Klammern ist
USHORT lcl_MergeNegativeParenthesisFormat( USHORT nIntlFormat, USHORT nCurrFormat )
{
    short nSign = 0;        // -1:=Klammer 0:=links, 1:=mitte, 2:=rechts
    switch ( nIntlFormat )
    {
        case 0:                                         // ($1)
        case 4:                                         // (1$)
        case 14 :                                       // ($ 1)
        case 15 :                                       // (1 $)
            return nCurrFormat;
        break;
        case 1:                                         // -$1
        case 5:                                         // -1$
        case 8:                                         // -1 $
        case 9:                                         // -$ 1
            nSign = 0;
        break;
        case 2:                                         // $-1
        case 6:                                         // 1-$
        case 11 :                                       // $ -1
        case 13 :                                       // 1- $
        break;
            nSign = 1;
        case 3:                                         // $1-
        case 7:                                         // 1$-
        case 10:                                        // 1 $-
        case 12 :                                       // $ 1-
            nSign = 2;
        break;
    }

    switch ( nCurrFormat )
    {
        case 0:                                         // ($1)
            switch ( nSign )
            {
                case 0:
                    return 1;                           // -$1
                break;
                case 1:
                    return 2;                           // $-1
                break;
                case 2:
                    return 3;                           // $1-
                break;
            }
        break;
        case 4:                                         // (1$)
            switch ( nSign )
            {
                case 0:
                    return 5;                           // -1$
                break;
                case 1:
                    return 6;                           // 1-$
                break;
                case 2:
                    return 7;                           // 1$-
                break;
            }
        break;
        case 14 :                                       // ($ 1)
            switch ( nSign )
            {
                case 0:
                    return 9;                           // -$ 1
                break;
                case 1:
                    return 11;                          // $ -1
                break;
                case 2:
                    return 12;                          // $ 1-
                break;
            }
        break;
        case 15 :                                       // (1 $)
            switch ( nSign )
            {
                case 0:
                    return 8;                           // -1 $
                break;
                case 1:
                    return 13;                          // 1- $
                break;
                case 2:
                    return 10;                          // 1 $-
                break;
            }
        break;
    }
    return nCurrFormat;
}


// static
USHORT NfCurrencyEntry::GetEffectiveNegativeFormat( USHORT nIntlFormat,
            USHORT nCurrFormat, BOOL bBank )
{
    if ( bBank )
    {
#if NF_BANKSYMBOL_FIX_POSITION
        return 8;
#else
        switch ( nIntlFormat )
        {
            case 0:                                         // ($1)
//              nIntlFormat = 14;                           // ($ 1)
                nIntlFormat = 9;                            // -$ 1
            break;
            case 1:                                         // -$1
                nIntlFormat = 9;                            // -$ 1
            break;
            case 2:                                         // $-1
                nIntlFormat = 11;                           // $ -1
            break;
            case 3:                                         // $1-
                nIntlFormat = 12;                           // $ 1-
            break;
            case 4:                                         // (1$)
//              nIntlFormat = 15;                           // (1 $)
                nIntlFormat = 8;                            // -1 $
            break;
            case 5:                                         // -1$
                nIntlFormat = 8;                            // -1 $
            break;
            case 6:                                         // 1-$
                nIntlFormat = 13;                           // 1- $
            break;
            case 7:                                         // 1$-
                nIntlFormat = 10;                           // 1 $-
            break;
            case 8:                                         // -1 $
            break;
            case 9:                                         // -$ 1
            break;
            case 10:                                        // 1 $-
            break;
            case 11:                                        // $ -1
            break;
            case 12 :                                       // $ 1-
            break;
            case 13 :                                       // 1- $
            break;
            case 14 :                                       // ($ 1)
//              nIntlFormat = 14;                           // ($ 1)
                nIntlFormat = 9;                            // -$ 1
            break;
            case 15 :                                       // (1 $)
//              nIntlFormat = 15;                           // (1 $)
                nIntlFormat = 8;                            // -1 $
            break;
        }
#endif
    }
    else if ( nIntlFormat != nCurrFormat )
    {
        switch ( nCurrFormat )
        {
            case 0:                                         // ($1)
                nIntlFormat = lcl_MergeNegativeParenthesisFormat(
                    nIntlFormat, nCurrFormat );
            break;
            case 1:                                         // -$1
                nIntlFormat = nCurrFormat;
            break;
            case 2:                                         // $-1
                nIntlFormat = nCurrFormat;
            break;
            case 3:                                         // $1-
                nIntlFormat = nCurrFormat;
            break;
            case 4:                                         // (1$)
                nIntlFormat = lcl_MergeNegativeParenthesisFormat(
                    nIntlFormat, nCurrFormat );
            break;
            case 5:                                         // -1$
                nIntlFormat = nCurrFormat;
            break;
            case 6:                                         // 1-$
                nIntlFormat = nCurrFormat;
            break;
            case 7:                                         // 1$-
                nIntlFormat = nCurrFormat;
            break;
            case 8:                                         // -1 $
                nIntlFormat = nCurrFormat;
            break;
            case 9:                                         // -$ 1
                nIntlFormat = nCurrFormat;
            break;
            case 10:                                        // 1 $-
                nIntlFormat = nCurrFormat;
            break;
            case 11:                                        // $ -1
                nIntlFormat = nCurrFormat;
            break;
            case 12 :                                       // $ 1-
                nIntlFormat = nCurrFormat;
            break;
            case 13 :                                       // 1- $
                nIntlFormat = nCurrFormat;
            break;
            case 14 :                                       // ($ 1)
                nIntlFormat = lcl_MergeNegativeParenthesisFormat(
                    nIntlFormat, nCurrFormat );
            break;
            case 15 :                                       // (1 $)
                nIntlFormat = lcl_MergeNegativeParenthesisFormat(
                    nIntlFormat, nCurrFormat );
            break;
        }
    }
    return nIntlFormat;
}


// we only support default encodings here
// static
sal_Char NfCurrencyEntry::GetEuroSymbol( rtl_TextEncoding eTextEncoding )
{
    switch ( eTextEncoding )
    {
        case RTL_TEXTENCODING_MS_1252 :         // WNT Ansi
        case RTL_TEXTENCODING_ISO_8859_1 :      // UNX for use with TrueType fonts
            return '\x80';
        break;
        case RTL_TEXTENCODING_ISO_8859_15 :     // UNX real
            return '\xA4';
        break;
        case RTL_TEXTENCODING_IBM_850 :         // OS2
            return '\xD5';
        break;
        case RTL_TEXTENCODING_APPLE_ROMAN :     // MAC
            return '\xDB';
        break;
        default:                                // default system
#if WNT
            return '\x80';
#elif OS2
            return '\xD5';
#elif UNX
//          return '\xA4';      // #56121# 0xA4 waere korrekt fuer iso-8859-15
            return '\x80';      // aber Windoze-Code fuer die konvertierten TrueType-Fonts
#elif MAC
            return '\xDB';
#else
#error EuroSymbol is what?
            return '\x80';
#endif
    }
    return '\x80';
}



