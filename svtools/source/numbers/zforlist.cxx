/*************************************************************************
 *
 *  $RCSfile: zforlist.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: er $ $Date: 2001-03-07 19:11:18 $
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
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _UNOTOOLS_NUMBERFORMATCODEWRAPPER_HXX
#include <unotools/numberformatcodewrapper.hxx>
#endif
#ifndef _UNOTOOLS_CALENDARWRAPPER_HXX
#include <unotools/calendarwrapper.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_KNUMBERFORMATUSAGE_HPP_
#include <com/sun/star/i18n/KNumberFormatUsage.hpp>
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


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;


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
SV_IMPL_PTRARR( NfWSStringsDtor, String* );

// ob das BankSymbol immer am Ende ist (1 $;-1 $) oder sprachabhaengig
#define NF_BANKSYMBOL_FIX_POSITION 1

/***********************Funktionen SvNumberFormatter**************************/

SvNumberFormatter::SvNumberFormatter(
            const Reference< XMultiServiceFactory >& xSMgr,
            LanguageType eLang )
        :
        xServiceManager( xSMgr )
{
    ImpConstruct( eLang );
}


SvNumberFormatter::SvNumberFormatter( LanguageType eLang )
{
    ImpConstruct( eLang );
}


SvNumberFormatter::~SvNumberFormatter()
{
    SvNumberformat* pEntry = aFTable.First();
    while (pEntry)
    {
        delete pEntry;
        pEntry = aFTable.Next();
    }
    delete pFormatTable;
    delete pCalendar;
    delete pLocaleData;
    delete pCharClass;
    delete pIntl;
    delete pStringScanner;
    delete pFormatScanner;
    ClearMergeTable();
    delete pMergeTable;
}


void SvNumberFormatter::ImpConstruct( LanguageType eLang )
{
    if ( eLang == LANGUAGE_DONTKNOW )
        eLang = UNKNOWN_SUBSTITUTE;
    SysLnge = eLang;
    ActLnge = eLang;
    eEvalDateFormat = NF_EVALDATEFORMAT_INTL;
    nDefaultSystemCurrencyFormat = NUMBERFORMAT_ENTRY_NOT_FOUND;
    nDefaultCurrencyFormat = NUMBERFORMAT_ENTRY_NOT_FOUND;

    aLocale = ConvertLanguageToLocale( eLang );
    pCharClass = new CharClass( xServiceManager, aLocale );
    pLocaleData = new LocaleDataWrapper( xServiceManager, aLocale );
    pCalendar = new CalendarWrapper( xServiceManager );
    pCalendar->loadDefaultCalendar( aLocale );

    if ( !International::IsFormatAvailable( eLang ) )
        eLang = UNKNOWN_SUBSTITUTE;
    pIntl = new International( eLang );

    pStringScanner = new ImpSvNumberInputScan( this );
    pFormatScanner = new ImpSvNumberformatScan( this );
    pFormatTable = NULL;
    ImpGenerateFormats( 0, FALSE );     // 0 .. 999 for System formats
    MaxCLOffset = 0;
    pMergeTable = new SvULONGTable;
    bNoZero = FALSE;
    pColorLink = NULL;
}


void SvNumberFormatter::ChangeIntl(LanguageType eLnge)
{
    if (ActLnge != eLnge)
    {
        ActLnge = eLnge;
        nDefaultCurrencyFormat = NUMBERFORMAT_ENTRY_NOT_FOUND;

        aLocale = ConvertLanguageToLocale( eLnge );
        pCharClass->setLocale( aLocale );
        pLocaleData->setLocale( aLocale );
        pCalendar->loadDefaultCalendar( aLocale );

        delete pIntl;
        if ( !International::IsFormatAvailable( eLnge ) )
            eLnge = UNKNOWN_SUBSTITUTE;
        pIntl = new International( eLnge );

        pFormatScanner->ChangeIntl();
        pStringScanner->ChangeIntl();
    }
}


// static
::com::sun::star::lang::Locale SvNumberFormatter::ConvertLanguageToLocale( LanguageType eLang )
{
    if ( eLang == LANGUAGE_DONTKNOW )
        eLang = UNKNOWN_SUBSTITUTE;
    if ( eLang == LANGUAGE_SYSTEM )
        eLang = International::GetRealLanguage( eLang );
    String aLanguage, aCountry, aVariant;
    ConvertLanguageToIsoNames( eLang, aLanguage, aCountry );
    return Locale( aLanguage, aCountry, aVariant );
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

void SvNumberFormatter::ImpChangeSysCL( LanguageType eLnge, BOOL bLoadingSO5 )
{
    if (eLnge == LANGUAGE_DONTKNOW)
        eLnge = UNKNOWN_SUBSTITUTE;
    if (eLnge != SysLnge)
    {
        SysLnge = eLnge;
        ChangeIntl(eLnge);
        SvNumberformat* pEntry = aFTable.First();
        while (pEntry)                          // delete old formats
        {
            pEntry = (SvNumberformat*) aFTable.Remove(aFTable.GetCurKey());
            delete pEntry;
            pEntry = (SvNumberformat*) aFTable.First();
        }
        ImpGenerateFormats( 0, bLoadingSO5 );   // new standard formats
    }
    else if ( bLoadingSO5 )
    {   // delete additional standard formats
        ULONG nKey;
        aFTable.Seek( SV_MAX_ANZ_STANDARD_FORMATE + 1 );
        while ( (nKey = aFTable.GetCurKey()) > SV_MAX_ANZ_STANDARD_FORMATE &&
                nKey < SV_COUNTRY_LANGUAGE_OFFSET )
        {
            SvNumberformat* pEntry = (SvNumberformat*) aFTable.Remove( nKey );
            delete pEntry;
        }
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

BOOL SvNumberFormatter::PutEntry(String& rString,
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
            SvNumberformat* pStdFormat =
                     (SvNumberformat*) aFTable.Get(CLOffset + ZF_STANDARD);
            ULONG nPos = CLOffset + pStdFormat->GetLastInsertKey();
            if (nPos - CLOffset >= SV_COUNTRY_LANGUAGE_OFFSET)
            {
                Sound::Beep();
                DBG_ERROR("SvNumberFormatter:: Zu viele Formate pro CL");
                delete p_Entry;
            }
            else if (!aFTable.Insert(nPos+1,p_Entry))
                delete p_Entry;
            else
            {
                bCheck = TRUE;
                nKey = nPos+1;
                pStdFormat->SetLastInsertKey((USHORT) (nKey-CLOffset));
            }
        }
    }
    else
        delete p_Entry;
    return bCheck;
}

BOOL SvNumberFormatter::PutandConvertEntry(String& rString,
                                           xub_StrLen& nCheckPos,
                                           short& nType,
                                           ULONG& nKey,
                                           LanguageType eLnge,
                                           LanguageType eNewLnge)
{
    BOOL bRes;
    if (eNewLnge == LANGUAGE_DONTKNOW)
        eNewLnge = UNKNOWN_SUBSTITUTE;

    pFormatScanner->SetConvertMode(eLnge, eNewLnge);
    bRes = PutEntry(rString, nCheckPos, nType, nKey, eLnge);
    pFormatScanner->SetConvertMode(FALSE);
    return bRes;
}


BOOL SvNumberFormatter::PutandConvertEntrySystem(String& rString,
                                           xub_StrLen& nCheckPos,
                                           short& nType,
                                           ULONG& nKey,
                                           LanguageType eLnge,
                                           LanguageType eNewLnge)
{
    BOOL bRes;
    if (eNewLnge == LANGUAGE_DONTKNOW)
        eNewLnge = UNKNOWN_SUBSTITUTE;

    pFormatScanner->SetConvertMode(eLnge, eNewLnge, TRUE);
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
    ImpChangeSysCL( eLnge, TRUE );

    rStream >> nPos;
    while (nPos != NUMBERFORMAT_ENTRY_NOT_FOUND)
    {
        rStream >> eDummy >> eLge;
        eLnge = (LanguageType) eLge;
        ImpGenerateCL( eLnge, TRUE );           // ggfs. neue Standardformate anlegen

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
                pConverter = new SvNumberFormatter( xServiceManager, eSysLang );
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
                        pConverter = new SvNumberFormatter( xServiceManager, eSysLang );
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
                                    pConverter = new SvNumberFormatter( xServiceManager, eSysLang );
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

    // generate additional i18n standard formats for all used locales
    LanguageType eOldLanguage = ActLnge;
    NumberFormatCodeWrapper aNumberFormatCode( xServiceManager, GetLocale() );
    SvUShorts aList;
    GetUsedLanguages( aList );
    USHORT nCount = aList.Count();
    for ( USHORT j=0; j<nCount; j++ )
    {
        LanguageType eLang = aList[j];
        ChangeIntl( eLang );
        ULONG CLOffset = ImpGetCLOffset( eLang );
        ImpGenerateAdditionalFormats( CLOffset, aNumberFormatCode, TRUE );
    }
    ChangeIntl( eOldLanguage );

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

String SvNumberFormatter::GetKeyword( LanguageType eLnge, USHORT nIndex )
{
    ChangeIntl(eLnge);
    const String* pTable = pFormatScanner->GetKeyword();
    if ( pTable && nIndex < NF_KEYWORD_ENTRIES_COUNT )
        return pTable[nIndex];

    DBG_ERROR("GetKeyword: invalid index");
    return String();
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

ULONG SvNumberFormatter::ImpIsEntry(const String& rString,
                                       ULONG nCLOffset,
                                       LanguageType eLnge)
{
#ifndef NF_COMMENT_IN_FORMATSTRING
#error NF_COMMENT_IN_FORMATSTRING not defined (zformat.hxx)
#endif
#if NF_COMMENT_IN_FORMATSTRING
    String aStr( rString );
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
            String aFormat( pEntry->GetFormatstring() );
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

ULONG SvNumberFormatter::ImpGenerateCL( LanguageType eLnge, BOOL bLoadingSO5 )
{
    ChangeIntl(eLnge);
    ULONG CLOffset = ImpGetCLOffset(ActLnge);
    if (CLOffset > MaxCLOffset)
    {   // new CL combination
#ifndef PRODUCT
        Locale aLoadedLocale = pLocaleData->getLoadedLocale();
        if ( aLoadedLocale.Language != aLocale.Language || aLoadedLocale.Country != aLocale.Country )
        {
            ByteString aMsg( RTL_CONSTASCII_STRINGPARAM( "Locales don't match:" ) );
            DBG_ERRORFILE( pLocaleData->AppendLocaleInfo( aMsg ).GetBuffer() );
        }
        // test XML locale data FormatElement entries
        {
            uno::Sequence< i18n::FormatElement > xSeq = pLocaleData->getAllFormats();
            // A test for completeness of formatindex="0" ... formatindex="47"
            // is not needed here since it is done in ImpGenerateFormats().

            // Test for dupes of formatindex="..."
            for ( sal_Int32 j = 0; j < xSeq.getLength(); j++ )
            {
                sal_Int16 nIdx = xSeq[j].formatIndex;
                ByteString aDupes;
                for ( sal_Int32 i = 0; i < xSeq.getLength(); i++ )
                {
                    if ( i != j && xSeq[i].formatIndex == nIdx )
                    {
                        aDupes += ByteString::CreateFromInt32( i );
                        aDupes += ' ';
                    }
                }
                if ( aDupes.Len() )
                {
                    ByteString aMsg( RTL_CONSTASCII_STRINGPARAM( "XML locale data FormatElement formatindex dupe: " ) );
                    aMsg += ByteString::CreateFromInt32( nIdx );
                    aMsg.Append( RTL_CONSTASCII_STRINGPARAM( "\nFormatElements: " ) );
                    aMsg += ByteString::CreateFromInt32( j );
                    aMsg += ' ';
                    aMsg += aDupes;
                    DBG_ERRORFILE( pLocaleData->AppendLocaleInfo( aMsg ).GetBuffer() );
                }
            }
        }
#endif
        MaxCLOffset += SV_COUNTRY_LANGUAGE_OFFSET;
        ImpGenerateFormats( MaxCLOffset, bLoadingSO5 );
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
    if ( pFormatTable )
        pFormatTable->Clear();
    else
        pFormatTable = new SvNumberFormatTable;
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

BOOL SvNumberFormatter::IsNumberFormat(const String& sString,
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
                return ImpGetDefaultCurrencyFormat();
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
                                           String& sOutString)
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
                                        String& sOutString,
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

void SvNumberFormatter::GetOutputString(String& sString,
                                        ULONG nFIndex,
                                        String& sOutString,
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

BOOL SvNumberFormatter::GetPreviewString(const String& sFormatString,
                                         double fPreviewNumber,
                                         String& sOutString,
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
    String sTmpString = sFormatString;
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

BOOL SvNumberFormatter::GetPreviewStringGuess( const String& sFormatString,
                                         double fPreviewNumber,
                                         String& sOutString,
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
    String sTmpString;

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

ULONG SvNumberFormatter::TestNewString(const String& sFormatString,
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
    String sTmpString = sFormatString;
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

SvNumberformat* SvNumberFormatter::ImpInsertFormat(
            const ::com::sun::star::i18n::NumberFormatCode& rCode,
            ULONG nPos, BOOL bAfterLoadingSO5 )
{
    String aCodeStr( rCode.Code );
    if ( rCode.Index < NF_INDEX_TABLE_ENTRIES &&
            rCode.Usage == ::com::sun::star::i18n::KNumberFormatUsage::CURRENCY &&
            rCode.Index != NF_CURRENCY_1000DEC2_CCC )
    {   // strip surrounding [$...] on automatic currency
        if ( aCodeStr.SearchAscii( "[$" ) != STRING_NOTFOUND )
            aCodeStr = SvNumberformat::StripNewCurrencyDelimiters( aCodeStr, FALSE );
        else
        {
#ifndef PRODUCT
            if ( rCode.Index != NF_CURRENCY_1000DEC2_CCC )
            {
                ByteString aMsg( "ImpInsertFormat: no [$...] on currency format code, index " );
                aMsg += ByteString::CreateFromInt32( rCode.Index );
                aMsg += ":\n";
                aMsg += ByteString( String( rCode.Code ), RTL_TEXTENCODING_UTF8 );
                DBG_ERRORFILE( (pLocaleData->AppendLocaleInfo( aMsg )).GetBuffer() );
            }
#endif
        }
    }
    xub_StrLen nCheckPos = 0;
    SvNumberformat* pFormat = new SvNumberformat(aCodeStr,
                                                 pFormatScanner,
                                                 pStringScanner,
                                                 nCheckPos,
                                                 ActLnge);
    if ( !pFormat || nCheckPos > 0 )
    {
#ifndef PRODUCT
        ByteString aMsg( "ImpInsertFormat: bad format code, index " );
        aMsg += ByteString::CreateFromInt32( rCode.Index );
        aMsg += '\n';
        aMsg += ByteString( String( rCode.Code ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( (pLocaleData->AppendLocaleInfo( aMsg )).GetBuffer() );
#endif
        delete pFormat;
        return NULL;
    }
    if ( rCode.Index >= NF_INDEX_TABLE_ENTRIES )
    {
        ULONG nCLOffset = nPos - (nPos % SV_COUNTRY_LANGUAGE_OFFSET);
        ULONG nKey = ImpIsEntry( aCodeStr, nCLOffset, ActLnge );
        if ( nKey != NUMBERFORMAT_ENTRY_NOT_FOUND )
        {
#ifndef PRODUCT
            if ( !bAfterLoadingSO5 )
            {   // If bAfterLoadingSO5 there will definitely be some dupes,
                // don't cry. But we need this test for verification of locale
                // data if not loading old SO5 documents.
                 ByteString aMsg( "ImpInsertFormat: dup format code, index " );
                aMsg += ByteString::CreateFromInt32( rCode.Index );
                aMsg += '\n';
                aMsg += ByteString( String( rCode.Code ), RTL_TEXTENCODING_UTF8 );
                DBG_ERRORFILE( (pLocaleData->AppendLocaleInfo( aMsg )).GetBuffer() );
            }
#endif
            delete pFormat;
            return NULL;
        }
        else if ( nPos - nCLOffset >= SV_COUNTRY_LANGUAGE_OFFSET )
        {
#ifndef PRODUCT
            ByteString aMsg( "ImpInsertFormat: too many format codes, index " );
            aMsg += ByteString::CreateFromInt32( rCode.Index );
            aMsg += '\n';
            aMsg += ByteString( String( rCode.Code ), RTL_TEXTENCODING_UTF8 );
            DBG_ERRORFILE( (pLocaleData->AppendLocaleInfo( aMsg )).GetBuffer() );
#endif
            delete pFormat;
            return NULL;
        }
    }
    if ( !aFTable.Insert( nPos, pFormat ) )
    {
#ifndef PRODUCT
        ByteString aMsg( "ImpInsertFormat: can't insert number format key pos: " );
        aMsg += ByteString::CreateFromInt32( nPos );
        aMsg.Append( RTL_CONSTASCII_STRINGPARAM( ", code index " ) );
        aMsg += ByteString::CreateFromInt32( rCode.Index );
        aMsg += '\n';
        aMsg += ByteString( String( rCode.Code ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( (pLocaleData->AppendLocaleInfo( aMsg )).GetBuffer() );
#endif
        delete pFormat;
        return NULL;
    }
    if ( rCode.Default )
        pFormat->SetStandard();
    if ( rCode.DefaultName.getLength() )
        pFormat->SetComment( rCode.DefaultName );
    return pFormat;
}

SvNumberformat* SvNumberFormatter::ImpInsertNewStandardFormat(
            const ::com::sun::star::i18n::NumberFormatCode& rCode,
            ULONG nPos, USHORT nVersion, BOOL bAfterLoadingSO5 )
{
    SvNumberformat* pNewFormat = ImpInsertFormat( rCode, nPos, bAfterLoadingSO5 );
    if (pNewFormat)
        pNewFormat->SetNewStandardDefined( nVersion );
        // so that it gets saved, displayed properly, and converted by old versions
    return pNewFormat;
}

void SvNumberFormatter::GetFormatSpecialInfo(ULONG nFormat,
                                             BOOL& bThousand,
                                             BOOL& IsRed,
                                             USHORT& nPrecision,
                                             USHORT& nAnzLeading)

{
    const SvNumberformat* pFormat = aFTable.Get(nFormat);
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
    const SvNumberformat* pFormat = aFTable.Get( nFormat );
    if ( pFormat )
        return pFormat->GetFormatPrecision();
    else
        return pFormatScanner->GetStandardPrec();
}


String SvNumberFormatter::GetFormatDecimalSep( ULONG nFormat ) const
{
    const SvNumberformat* pFormat = aFTable.Get( nFormat );
    if ( !pFormat || pFormat->GetLanguage() == ActLnge )
        return pLocaleData->getNumDecimalSep();

    ::com::sun::star::lang::Locale aSaveLocale( pLocaleData->getLocale() );
    pLocaleData->setLocale( ConvertLanguageToLocale( pFormat->GetLanguage() ) );
    String aRet( pLocaleData->getNumDecimalSep() );
    pLocaleData->setLocale( aSaveLocale );
    return aRet;
}


ULONG SvNumberFormatter::GetFormatSpecialInfo( const String& rFormatString,
            BOOL& bThousand, BOOL& IsRed, USHORT& nPrecision,
            USHORT& nAnzLeading, LanguageType eLnge )

{
    xub_StrLen nCheckPos = 0;
    if (eLnge == LANGUAGE_DONTKNOW)
        eLnge = SysLnge;
    ChangeIntl(eLnge);                                  // ggfs. austauschen
    eLnge = ActLnge;
    String aTmpStr( rFormatString );
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
            "SetIndexTable: theIndexTable[nTabOff] already occupied" );
        theIndexTable[nTabOff] = nIndOff;
    }
    return nIndOff;
}


sal_Int32 SvNumberFormatter::ImpGetFormatCodeIndex(
            ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode >& rSeq,
            const NfIndexTableOffset nTabOff )
{
    const sal_Int32 nLen = rSeq.getLength();
    for ( sal_Int32 j=0; j<nLen; j++ )
    {
        if ( rSeq[j].Index == nTabOff )
            return j;
    }
#ifndef PRODUCT
    if ( nTabOff < NF_CURRENCY_START || NF_CURRENCY_END < nTabOff
      || nTabOff == NF_CURRENCY_1000INT || nTabOff == NF_CURRENCY_1000INT_RED
      || nTabOff == NF_CURRENCY_1000DEC2_CCC )
    {   // currency entries with decimals might not exist, e.g. Italian Lira
        ByteString aMsg( RTL_CONSTASCII_STRINGPARAM( "ImpGetFormatCodeIndex: not found: " ) );
        aMsg += ByteString::CreateFromInt32( nTabOff );
        DBG_ERRORFILE( (pLocaleData->AppendLocaleInfo( aMsg )).GetBuffer() );
    }
#endif
    if ( nLen )
    {
        sal_Int32 j;
        // look for a preset default
        for ( j=0; j<nLen; j++ )
        {
            if ( rSeq[j].Default )
                return j;
        }
        // currencies are special, not all format codes must exist, but all
        // builtin number format key index positions must have a format assigned
        if ( NF_CURRENCY_START <= nTabOff && nTabOff <= NF_CURRENCY_END )
        {
            // look for a format with decimals
            for ( j=0; j<nLen; j++ )
            {
                if ( rSeq[j].Index == NF_CURRENCY_1000DEC2 )
                    return j;
            }
            // last resort: look for a format without decimals
            for ( j=0; j<nLen; j++ )
            {
                if ( rSeq[j].Index == NF_CURRENCY_1000INT )
                    return j;
            }
        }
    }
    else
    {   // we need at least _some_ format
        rSeq.realloc(1);
        rSeq[0] = ::com::sun::star::i18n::NumberFormatCode();
        String aTmp( '0' );
        aTmp += GetDecimalSep();
        aTmp.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "############" ) );
        rSeq[0].Code = aTmp;
    }
    return 0;
}


void SvNumberFormatter::ImpGenerateFormats( ULONG CLOffset, BOOL bLoadingSO5 )
{
    using namespace ::com::sun::star;

    if ( !bIndexTableInitialized )
    {
        for ( USHORT j=0; j<NF_INDEX_TABLE_ENTRIES; j++ )
        {
            theIndexTable[j] = NUMBERFORMAT_ENTRY_NOT_FOUND;
        }
    }
    BOOL bOldConvertMode = pFormatScanner->GetConvertMode();
    if (bOldConvertMode)
        pFormatScanner->SetConvertMode(FALSE);      // switch off for this function

    NumberFormatCodeWrapper aNumberFormatCode( xServiceManager, GetLocale() );

    xub_StrLen nCheckPos = 0;
    SvNumberformat* pNewFormat = NULL;
    String aFormatCode;
    sal_Int32 nIdx;
    sal_Bool bDefault;

    // Counter for additional builtin formats not fitting into the first 10
    // of a category (TLOT:=The Legacy Of Templin), altogether about 20 formats.
    // Has to be incremented on each ImpInsertNewStandardformat, new formats
    // must be appended, not inserted!
    USHORT nNewExtended = ZF_STANDARD_NEWEXTENDED;

    // General
    aFormatCode = pFormatScanner->GetStandardName();
    SvNumberformat* pStdFormat = new SvNumberformat( aFormatCode,
        pFormatScanner, pStringScanner, nCheckPos, ActLnge );
    pStdFormat->SetType( NUMBERFORMAT_NUMBER );
    pStdFormat->SetStandard();
    if ( !aFTable.Insert(
            CLOffset + SetIndexTable( NF_NUMBER_STANDARD, ZF_STANDARD ),
            pStdFormat ) )
    {
#ifndef PRODUCT
         ByteString aMsg( "ImpGenerateFormats: General format not insertable, nothing will work" );
        DBG_ERRORFILE( (pLocaleData->AppendLocaleInfo( aMsg )).GetBuffer() );
#endif
        delete pStdFormat;
        pStdFormat = NULL;
    }
    else
        pStdFormat->SetLastInsertKey( SV_MAX_ANZ_STANDARD_FORMATE );

    // Boolean
    aFormatCode = pFormatScanner->GetBooleanString();
    pNewFormat = new SvNumberformat( aFormatCode,
        pFormatScanner, pStringScanner, nCheckPos, ActLnge );
    pNewFormat->SetType(NUMBERFORMAT_LOGICAL);
    pNewFormat->SetStandard();
    if ( !aFTable.Insert(
            CLOffset + SetIndexTable( NF_BOOLEAN, ZF_STANDARD_LOGICAL ),
            pNewFormat))
        delete pNewFormat;

    // Text
    aFormatCode = '@';
    pNewFormat = new SvNumberformat( aFormatCode,
        pFormatScanner, pStringScanner, nCheckPos, ActLnge );
    pNewFormat->SetType(NUMBERFORMAT_TEXT);
    pNewFormat->SetStandard();
    if ( !aFTable.Insert(
            CLOffset + SetIndexTable( NF_TEXT, ZF_STANDARD_TEXT ),
            pNewFormat))
        delete pNewFormat;



    // Number
    uno::Sequence< i18n::NumberFormatCode > aFormatSeq
        = aNumberFormatCode.getAllFormatCode( i18n::KNumberFormatUsage::FIXED_NUMBER );

    // 0
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_NUMBER_INT );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_NUMBER_INT, ZF_STANDARD+1 ));

    // 0.00
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_NUMBER_DEC2 );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_NUMBER_DEC2, ZF_STANDARD+2 ));

    // #,##0
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_NUMBER_1000INT );
    ImpInsertFormat( aFormatSeq[nIdx],
            CLOffset + SetIndexTable( NF_NUMBER_1000INT, ZF_STANDARD+3 ));

    // #,##0.00
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_NUMBER_1000DEC2 );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_NUMBER_1000DEC2, ZF_STANDARD+4 ));

    // #.##0,00 System country/language dependent   since number formatter version 6
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_NUMBER_SYSTEM );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_NUMBER_SYSTEM, ZF_STANDARD+5 ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );


    // Percent number
    aFormatSeq = aNumberFormatCode.getAllFormatCode( i18n::KNumberFormatUsage::PERCENT_NUMBER );

    // 0%
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_PERCENT_INT );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_PERCENT_INT, ZF_STANDARD_PERCENT ));

    // 0.00%
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_PERCENT_DEC2 );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_PERCENT_DEC2, ZF_STANDARD_PERCENT+1 ));



    // Currency (no default standard option => no TRUE at ImpInsertFormat)
    aFormatSeq = aNumberFormatCode.getAllFormatCode( i18n::KNumberFormatUsage::CURRENCY );

    // #,##0
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_CURRENCY_1000INT );
    bDefault = aFormatSeq[nIdx].Default;
    aFormatSeq[nIdx].Default = sal_False;
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_CURRENCY_1000INT, ZF_STANDARD_CURRENCY ));
    aFormatSeq[nIdx].Default = bDefault;

    // #,##0.00
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_CURRENCY_1000DEC2 );
    bDefault = aFormatSeq[nIdx].Default;
    aFormatSeq[nIdx].Default = sal_False;
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_CURRENCY_1000DEC2, ZF_STANDARD_CURRENCY+1 ));
        //! MUST be ZF_STANDARD_CURRENCY+1 for Calc currency function, e.g. DM()
    aFormatSeq[nIdx].Default = bDefault;

    // #,##0 negative red
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_CURRENCY_1000INT_RED );
    bDefault = aFormatSeq[nIdx].Default;
    aFormatSeq[nIdx].Default = sal_False;
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_CURRENCY_1000INT_RED, ZF_STANDARD_CURRENCY+2 ));
    aFormatSeq[nIdx].Default = bDefault;

    // #,##0.00 negative red
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_CURRENCY_1000DEC2_RED );
    bDefault = aFormatSeq[nIdx].Default;
    aFormatSeq[nIdx].Default = sal_False;
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_CURRENCY_1000DEC2_RED, ZF_STANDARD_CURRENCY+3 ));
    aFormatSeq[nIdx].Default = bDefault;

    // #,##0.00 USD   since number formatter version 3
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_CURRENCY_1000DEC2_CCC );
    bDefault = aFormatSeq[nIdx].Default;
    aFormatSeq[nIdx].Default = sal_False;
    pNewFormat = ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_CURRENCY_1000DEC2_CCC, ZF_STANDARD_CURRENCY+4 ));
    if ( pNewFormat )
        pNewFormat->SetUsed(TRUE);      // must be saved for older versions
    aFormatSeq[nIdx].Default = bDefault;

    // #.##0,--   since number formatter version 6
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_CURRENCY_1000DEC2_DASHED );
    bDefault = aFormatSeq[nIdx].Default;
    aFormatSeq[nIdx].Default = sal_False;
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_CURRENCY_1000DEC2_DASHED, ZF_STANDARD_CURRENCY+5 ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );
    aFormatSeq[nIdx].Default = bDefault;

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



    // Date
    aFormatSeq = aNumberFormatCode.getAllFormatCode( i18n::KNumberFormatUsage::DATE );

    // DD.MM.YY   System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYSTEM_SHORT );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_SYSTEM_SHORT, ZF_STANDARD_DATE ));

    // NN DD.MMM YY
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_DEF_NNDDMMMYY );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_DEF_NNDDMMMYY, ZF_STANDARD_DATE+1 ));

    // DD.MM.YY   def/System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYS_MMYY );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_SYS_MMYY, ZF_STANDARD_DATE+2 ));

    // DD MMM
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYS_DDMMM );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_SYS_DDMMM, ZF_STANDARD_DATE+3 ));

    // MMMM
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_MMMM );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_MMMM, ZF_STANDARD_DATE+4 ));

    // QQ YY
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_QQJJ );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_QQJJ, ZF_STANDARD_DATE+5 ));

    // DD.MM.YYYY   since number formatter version 2, was DD.MM.[YY]YY
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYS_DDMMYYYY );
    pNewFormat = ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_SYS_DDMMYYYY, ZF_STANDARD_DATE+6 ));
    if ( pNewFormat )
        pNewFormat->SetUsed(TRUE);      // must be saved for older versions

    // DD.MM.YY   def/System, since number formatter version 6
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYS_DDMMYY );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_SYS_DDMMYY, ZF_STANDARD_DATE+7 ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );

    // NNN, D. MMMM YYYY   System
    // Long day of week: "NNNN" instead of "NNN," because of compatibility
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYSTEM_LONG );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_SYSTEM_LONG, ZF_STANDARD_DATE+8 ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );

    // Hard coded but system (regional settings) delimiters dependent long date formats
    // since numberformatter version 6

    // D. MMM YY   def/System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYS_DMMMYY );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_SYS_DMMMYY, ZF_STANDARD_DATE+9 ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );

    //! Unfortunally TLOT intended only 10 builtin formats per category, more
    //! would overwrite the next category (ZF_STANDARD_TIME) :-((
    //! Therefore they are inserted with nNewExtended++ (which is also limited)

    // D. MMM YYYY   def/System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYS_DMMMYYYY );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_SYS_DMMMYYYY, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );

    // D. MMMM YYYY   def/System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYS_DMMMMYYYY );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_SYS_DMMMMYYYY, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );

    // NN, D. MMM YY   def/System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYS_NNDMMMYY );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_SYS_NNDMMMYY, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );

    // NN, D. MMMM YYYY   def/System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYS_NNDMMMMYYYY );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_SYS_NNDMMMMYYYY, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );

    // NNN, D. MMMM YYYY   def/System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYS_NNNNDMMMMYYYY );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_SYS_NNNNDMMMMYYYY, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );

    // Hard coded DIN (Deutsche Industrie Norm) and EN (European Norm) date formats

    // D. MMM. YYYY   DIN/EN
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_DIN_DMMMYYYY );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_DIN_DMMMYYYY, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );

    // D. MMMM YYYY   DIN/EN
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_DIN_DMMMMYYYY );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_DIN_DMMMMYYYY, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );

    // MM-DD   DIN/EN
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_DIN_MMDD );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_DIN_MMDD, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );

    // YY-MM-DD   DIN/EN
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_DIN_YYMMDD );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_DIN_YYMMDD, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );

    // YYYY-MM-DD   DIN/EN
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_DIN_YYYYMMDD );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_DIN_YYYYMMDD, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );



    // Time
    aFormatSeq = aNumberFormatCode.getAllFormatCode( i18n::KNumberFormatUsage::TIME );

    // HH:MM
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_TIME_HHMM );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_TIME_HHMM, ZF_STANDARD_TIME ));

    // HH:MM:SS
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_TIME_HHMMSS );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_TIME_HHMMSS, ZF_STANDARD_TIME+1 ));

    // HH:MM AM/PM
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_TIME_HHMMAMPM );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_TIME_HHMMAMPM, ZF_STANDARD_TIME+2 ));

    // HH:MM:SS AM/PM
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_TIME_HHMMSSAMPM );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_TIME_HHMMSSAMPM, ZF_STANDARD_TIME+3 ));

    // [HH]:MM:SS
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_TIME_HH_MMSS );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_TIME_HH_MMSS, ZF_STANDARD_TIME+4 ));

    // MM:SS,00
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_TIME_MMSS00 );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_TIME_MMSS00, ZF_STANDARD_TIME+5 ));

    // [HH]:MM:SS,00
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_TIME_HH_MMSS00 );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_TIME_HH_MMSS00, ZF_STANDARD_TIME+6 ),
        SV_NUMBERFORMATTER_VERSION_NF_TIME_HH_MMSS00 );



    // DateTime
    aFormatSeq = aNumberFormatCode.getAllFormatCode( i18n::KNumberFormatUsage::DATE_TIME );

    // DD.MM.YY HH:MM   System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATETIME_SYSTEM_SHORT_HHMM );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATETIME_SYSTEM_SHORT_HHMM, ZF_STANDARD_DATETIME ));

    // DD.MM.YYYY HH:MM:SS   System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATETIME_SYS_DDMMYYYY_HHMMSS );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATETIME_SYS_DDMMYYYY_HHMMSS, ZF_STANDARD_DATETIME+1 ),
        SV_NUMBERFORMATTER_VERSION_NF_DATETIME_SYS_DDMMYYYY_HHMMSS );



    // Scientific number
    aFormatSeq = aNumberFormatCode.getAllFormatCode( i18n::KNumberFormatUsage::SCIENTIFIC_NUMBER );

    // 0.00E+000
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_SCIENTIFIC_000E000 );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_SCIENTIFIC_000E000, ZF_STANDARD_SCIENTIFIC ));

    // 0.00E+00
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_SCIENTIFIC_000E00 );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_SCIENTIFIC_000E00, ZF_STANDARD_SCIENTIFIC+1 ));



    // Fraction number (no default option)
    i18n::NumberFormatCode aSingleFormatCode;

     // # ?/?
    aSingleFormatCode.Code = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "# ?/?" ) );
    String s25( RTL_CONSTASCII_USTRINGPARAM( "# ?/?" ) );           // # ?/?
    ImpInsertFormat( aSingleFormatCode,
        CLOffset + SetIndexTable( NF_FRACTION_1, ZF_STANDARD_FRACTION ));

    // # ??/??
    //! "??/" would be interpreted by the compiler as a trigraph for '\'
    aSingleFormatCode.Code = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "# ?\?/?\?" ) );
    ImpInsertFormat( aSingleFormatCode,
        CLOffset + SetIndexTable( NF_FRACTION_2, ZF_STANDARD_FRACTION+1 ));

    // Week of year   must be appended here because of nNewExtended
    const String* pKeyword = pFormatScanner->GetKeyword();
    aSingleFormatCode.Code = pKeyword[NF_KEY_WW];
    ImpInsertNewStandardFormat( aSingleFormatCode,
        CLOffset + SetIndexTable( NF_DATE_WW, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NF_DATE_WW );



    bIndexTableInitialized = TRUE;
    DBG_ASSERT( nNewExtended <= ZF_STANDARD_NEWEXTENDEDMAX,
        "ImpGenerateFormats: overflow of nNewExtended standard formats" );

    // Now all additional format codes provided by I18N, but only if not
    // loading from old SO5 file format, then they are appended last.
    if ( !bLoadingSO5 )
        ImpGenerateAdditionalFormats( CLOffset, aNumberFormatCode, FALSE );

    if (bOldConvertMode)
        pFormatScanner->SetConvertMode(TRUE);
}


void SvNumberFormatter::ImpGenerateAdditionalFormats( ULONG CLOffset,
            NumberFormatCodeWrapper& rNumberFormatCode, BOOL bAfterLoadingSO5 )
{
    using namespace ::com::sun::star;

    SvNumberformat* pStdFormat =
        (SvNumberformat*) aFTable.Get( CLOffset + ZF_STANDARD );
    if ( !pStdFormat )
    {
        DBG_ERRORFILE( "ImpGenerateAdditionalFormats: no GENERAL format" );
        return ;
    }
    ULONG nPos = CLOffset + pStdFormat->GetLastInsertKey();
    rNumberFormatCode.setLocale( GetLocale() );
    sal_Int32 j;

    // All currencies, this time with [$...] which was stripped in
    // ImpGenerateFormats for old "automatic" currency formats.
    uno::Sequence< i18n::NumberFormatCode > aFormatSeq =
        rNumberFormatCode.getAllFormatCode( i18n::KNumberFormatUsage::CURRENCY );
    sal_Int32 nCodes = aFormatSeq.getLength();
    for ( j = 0; j < nCodes; j++ )
    {
        if ( nPos - CLOffset >= SV_COUNTRY_LANGUAGE_OFFSET )
        {
            DBG_ERRORFILE( "ImpGenerateAdditionalFormats: too many formats" );
            break;  // for
        }
        if ( aFormatSeq[j].Index < NF_INDEX_TABLE_ENTRIES &&
                aFormatSeq[j].Index != NF_CURRENCY_1000DEC2_CCC )
        {   // Insert only if not already inserted, but internal index must be
            // above so ImpInsertFormat can distinguish it.
            aFormatSeq[j].Index += nCodes + NF_INDEX_TABLE_ENTRIES;
            if ( ImpInsertNewStandardFormat( aFormatSeq[j], nPos+1,
                    SV_NUMBERFORMATTER_VERSION_ADDITIONAL_I18N_FORMATS,
                    bAfterLoadingSO5 ) )
                nPos++;
        }
    }

    // all additional format codes provided by I18N that are not old standard index
    aFormatSeq = rNumberFormatCode.getAllFormatCodes();
    nCodes = aFormatSeq.getLength();
    for ( j = 0; j < nCodes; j++ )
    {
        if ( nPos - CLOffset >= SV_COUNTRY_LANGUAGE_OFFSET )
        {
            DBG_ERRORFILE( "ImpGenerateAdditionalFormats: too many formats" );
            break;  // for
        }
        if ( aFormatSeq[j].Index >= NF_INDEX_TABLE_ENTRIES )
            if ( ImpInsertNewStandardFormat( aFormatSeq[j], nPos+1,
                    SV_NUMBERFORMATTER_VERSION_ADDITIONAL_I18N_FORMATS,
                    bAfterLoadingSO5 ) )
                nPos++;
    }

    pStdFormat->SetLastInsertKey( (USHORT)(nPos - CLOffset) );
}


void SvNumberFormatter::ImpGetPosCurrFormat(String& sPosStr)
{
    NfCurrencyEntry::CompletePositiveFormatString( sPosStr,
        pLocaleData->getCurrSymbol(), pLocaleData->getCurrPositiveFormat() );
}

void SvNumberFormatter::ImpGetNegCurrFormat(String& sNegStr)
{
    NfCurrencyEntry::CompleteNegativeFormatString( sNegStr,
        pLocaleData->getCurrSymbol(), pLocaleData->getCurrNegativeFormat() );
}

void SvNumberFormatter::GenerateFormat(String& sString,
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

    const String& rThSep = pLocaleData->getNumThousandSep();
    if (nAnzLeading == 0)
    {
        if (!bThousand)
            sString += '#';
        else
        {
            sString += '#';
            sString += rThSep;
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
                sString.Insert( rThSep, 0 );
            sString.Insert('0',0);
        }
        if (bThousand && nAnzLeading < 4)
        {
            for (i = nAnzLeading; i < 4; i++)
            {
                if (bThousand && i%3 == 0)
                    sString.Insert( rThSep, 0 );
                sString.Insert('#',0);
            }
        }
    }
    if (nPrecision > 0)
    {
                sString += pLocaleData->getNumDecimalSep();
                for (i = 0; i < nPrecision; i++)
                    sString += '0';
    }
    if (eType == NUMBERFORMAT_PERCENT)
        sString += '%';
    else if (eType == NUMBERFORMAT_CURRENCY)
    {
        String sNegStr = sString;
        String aCurr;
        const NfCurrencyEntry* pEntry;
        BOOL bBank;
        if ( GetNewCurrencySymbolString( nIndex, aCurr, &pEntry, &bBank ) )
        {
            if ( pEntry )
            {
                USHORT nPosiForm = NfCurrencyEntry::GetEffectivePositiveFormat(
                    pLocaleData->getCurrPositiveFormat(),
                    pEntry->GetPositiveFormat(), bBank );
                USHORT nNegaForm = NfCurrencyEntry::GetEffectiveNegativeFormat(
                    pLocaleData->getCurrNegativeFormat(),
                    pEntry->GetNegativeFormat(), bBank );
                pEntry->CompletePositiveFormatString( sString, bBank,
                    nPosiForm );
                pEntry->CompleteNegativeFormatString( sNegStr, bBank,
                    nNegaForm );
            }
            else
            {   // wir gehen von einem Banksymbol aus
                USHORT nPosiForm = NfCurrencyEntry::GetEffectivePositiveFormat(
                    pLocaleData->getCurrPositiveFormat(),
                    pLocaleData->getCurrPositiveFormat(), TRUE );
                USHORT nNegaForm = NfCurrencyEntry::GetEffectiveNegativeFormat(
                    pLocaleData->getCurrNegativeFormat(),
                    pLocaleData->getCurrNegativeFormat(), TRUE );
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
        String sTmpStr = sString;
        sTmpStr += ';';
        sTmpStr += '[';
        sTmpStr += pFormatScanner->GetRedString();
        sTmpStr += ']';
        sTmpStr += '-';
        sTmpStr +=sString;
        sString = sTmpStr;
    }
}

BOOL SvNumberFormatter::IsUserDefined(const String& sStr,
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

ULONG SvNumberFormatter::GetEntryKey(const String& sStr,
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
                    Sound::Beep();
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
        ImpInitCurrencyTable();
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
    if ( nDefaultSystemCurrencyFormat == NUMBERFORMAT_ENTRY_NOT_FOUND )
    {
        xub_StrLen nCheck;
        short nType;
        NfWSStringsDtor aCurrList;
        USHORT nDefault = GetCurrencyFormatStrings( aCurrList,
            GetCurrencyEntry( LANGUAGE_SYSTEM ), FALSE );
        DBG_ASSERT( aCurrList.Count(), "where is the NewCurrency System standard format?!?" );
        // if already loaded or user defined nDefaultSystemCurrencyFormat
        // will be set to the right value
        PutEntry( *aCurrList.GetObject( nDefault ), nCheck, nType,
            nDefaultSystemCurrencyFormat, LANGUAGE_SYSTEM );
        DBG_ASSERT( nCheck == 0, "NewCurrency CheckError" );
        DBG_ASSERT( nDefaultSystemCurrencyFormat != NUMBERFORMAT_ENTRY_NOT_FOUND,
            "nDefaultSystemCurrencyFormat == NUMBERFORMAT_ENTRY_NOT_FOUND" );
    }
    return nDefaultSystemCurrencyFormat;
}


ULONG SvNumberFormatter::ImpGetDefaultCurrencyFormat()
{
    if ( nDefaultCurrencyFormat == NUMBERFORMAT_ENTRY_NOT_FOUND )
    {
        ULONG CLOffset = ImpGetCLOffset( ActLnge );
        // look for a defined standard
        ULONG nStopKey = CLOffset + SV_COUNTRY_LANGUAGE_OFFSET;
        ULONG nKey;
        aFTable.Seek( CLOffset );
        while ( (nKey = aFTable.GetCurKey()) >= CLOffset && nKey < nStopKey )
        {
            const SvNumberformat* pEntry =
                (const SvNumberformat*) aFTable.GetCurObject();
            if ( pEntry->IsStandard() && (pEntry->GetType() & NUMBERFORMAT_CURRENCY) )
            {
                nDefaultCurrencyFormat = nKey;
                break;  // while
            }
            aFTable.Next();
        }

        if ( nDefaultCurrencyFormat == NUMBERFORMAT_ENTRY_NOT_FOUND )
        {   // none found, create one
            xub_StrLen nCheck;
            short nType;
            NfWSStringsDtor aCurrList;
            USHORT nDefault = GetCurrencyFormatStrings( aCurrList,
                GetCurrencyEntry( ActLnge ), FALSE );
            DBG_ASSERT( aCurrList.Count(), "where is the NewCurrency standard format?" );
            if ( aCurrList.Count() )
            {
                // if already loaded or user defined nDefaultSystemCurrencyFormat
                // will be set to the right value
                PutEntry( *aCurrList.GetObject( nDefault ), nCheck, nType,
                    nDefaultCurrencyFormat, ActLnge );
                DBG_ASSERT( nCheck == 0, "NewCurrency CheckError" );
                DBG_ASSERT( nDefaultCurrencyFormat != NUMBERFORMAT_ENTRY_NOT_FOUND,
                    "nDefaultCurrencyFormat == NUMBERFORMAT_ENTRY_NOT_FOUND" );
            }
            // old automatic currency format as a last resort
            if ( nDefaultCurrencyFormat == NUMBERFORMAT_ENTRY_NOT_FOUND )
                nDefaultCurrencyFormat = CLOffset + ZF_STANDARD_CURRENCY+3;
            else
            {   // mark as standard so that it is found next time
                SvNumberformat* pEntry = aFTable.Get( nDefaultCurrencyFormat );
                if ( pEntry )
                    pEntry->SetStandard();
            }
        }
    }
    return nDefaultCurrencyFormat;
}


// try to make it inline if possible since this a loop body
// TRUE: continue; FALSE: break loop, if pFoundEntry==NULL dupe found
#ifdef PRODUCT
inline
#endif
    BOOL SvNumberFormatter::ImpLookupCurrencyEntryLoopBody(
        const NfCurrencyEntry*& pFoundEntry, BOOL& bFoundBank,
        const NfCurrencyEntry* pData, USHORT nPos, const String& rSymbol ) const
{
    BOOL bFound;
    if ( pData->GetSymbol() == rSymbol )
    {
        bFound = TRUE;
        bFoundBank = FALSE;
    }
    else if ( pData->GetBankSymbol() == rSymbol )
    {
        bFound = TRUE;
        bFoundBank = TRUE;
    }
    else
        bFound = FALSE;
    if ( bFound )
    {
        if ( pFoundEntry && pFoundEntry != pData )
        {
            pFoundEntry = NULL;
            return FALSE;   // break loop, not unique
        }
        if ( nPos == 0 )
        {   // first entry is SYSTEM
            pFoundEntry = MatchSystemCurrency();
            if ( pFoundEntry )
                return FALSE;   // break loop
                // even if there are more matching entries
                // this one is propably the one we are looking for
            else
                pFoundEntry = pData;
        }
        else
            pFoundEntry = pData;
    }
    return TRUE;
}


BOOL SvNumberFormatter::GetNewCurrencySymbolString( ULONG nFormat,
            String& rStr, const NfCurrencyEntry** ppEntry /* = NULL */,
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
        String aSymbol, aExtension;
        if ( pFormat->GetNewCurrencySymbol( aSymbol, aExtension ) )
        {
            if ( ppEntry )
            {
                BOOL bFoundBank = FALSE;
                // we definiteley need an entry matching the format code string
                const NfCurrencyEntry* pFoundEntry = GetCurrencyEntry(
                    bFoundBank, aSymbol, aExtension, pFormat->GetLanguage(),
                    TRUE );
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
                if ( aExtension.Len() )
                    rStr += aExtension;
                rStr += ']';
            }
            return TRUE;
        }
    }
    return FALSE;
}


const NfCurrencyEntry* SvNumberFormatter::GetCurrencyEntry( BOOL & bFoundBank,
            const String& rSymbol, const String& rExtension,
            LanguageType eFormatLanguage, BOOL bOnlyStringLanguage ) const
{
    xub_StrLen nExtLen = rExtension.Len();
    LanguageType eExtLang;
    if ( nExtLen )
    {
        sal_Int32 nExtLang = ::rtl::OUString( rExtension ).toInt32( 16 );
        if ( !nExtLang )
            eExtLang = LANGUAGE_DONTKNOW;
        else
            eExtLang = (LanguageType) ((nExtLang < 0) ?
                -nExtLang : nExtLang);
    }
    else
        eExtLang = LANGUAGE_DONTKNOW;
    const NfCurrencyEntry* pFoundEntry = NULL;
    const NfCurrencyTable& rTable = GetTheCurrencyTable();
    USHORT nCount = rTable.Count();
    BOOL bCont = TRUE;

    // first try with given extension language/country
    if ( nExtLen )
    {
        const NfCurrencyEntryPtr* ppData = rTable.GetData();
        for ( USHORT j = 0; j < nCount && bCont; j++, ppData++ )
        {
            LanguageType eLang = (*ppData)->GetLanguage();
            if ( eLang == eExtLang ||
                    ((eExtLang == LANGUAGE_DONTKNOW) &&
                    (eLang == LANGUAGE_SYSTEM))
                )
            {
                bCont = ImpLookupCurrencyEntryLoopBody( pFoundEntry, bFoundBank,
                    *ppData, j, rSymbol );
            }
        }
    }

    // ok?
    if ( pFoundEntry || !bCont || (bOnlyStringLanguage && nExtLen) )
        return pFoundEntry;

    if ( !bOnlyStringLanguage )
    {
        // now try the language/country of the number format
        const NfCurrencyEntryPtr* ppData = rTable.GetData();
        for ( USHORT j = 0; j < nCount && bCont; j++, ppData++ )
        {
            LanguageType eLang = (*ppData)->GetLanguage();
            if ( eLang == eFormatLanguage ||
                    ((eFormatLanguage == LANGUAGE_DONTKNOW) &&
                    (eLang == LANGUAGE_SYSTEM))
                )
            {
                bCont = ImpLookupCurrencyEntryLoopBody( pFoundEntry, bFoundBank,
                    *ppData, j, rSymbol );
            }
        }

        // ok?
        if ( pFoundEntry || !bCont )
            return pFoundEntry;
    }

    // then try without language/country
    {
        const NfCurrencyEntryPtr* ppData = rTable.GetData();
        for ( USHORT j = 0; j < nCount && bCont; j++, ppData++ )
        {
            bCont = ImpLookupCurrencyEntryLoopBody( pFoundEntry, bFoundBank,
                *ppData, j, rSymbol );
        }
    }

    return pFoundEntry;
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
// seems that there really are some currencies which differ, e.g. YugoDinar
        DBG_ERRORFILE( aStr.GetBuffer() );
#endif
    }
}
#endif


void SvNumberFormatter::ImpInitCurrencyTable() const
{
    // racing condition possible:
    // while ( !bCurrencyTableInitialized )
    //      ImpInitCurrencyTable();
    static BOOL bInitializing = FALSE;
    if ( bCurrencyTableInitialized || bInitializing )
        return ;
    bInitializing = TRUE;

    Locale aSaveLocale( pLocaleData->getLocale() );
    LanguageType eSysLang = System::GetLanguage();
    NfCurrencyEntryPtr pEntry;

    // first entry is SYSTEM
    pLocaleData->setLocale( ConvertLanguageToLocale( eSysLang ) );
    pEntry = new NfCurrencyEntry( *pLocaleData, LANGUAGE_SYSTEM );
    theCurrencyTable.Insert( pEntry, 0 );
    // insert country independent EURo at second position
    pEntry = new NfCurrencyEntry;
    pEntry->SetEuro();
    theCurrencyTable.Insert( pEntry, nCurrencyTableEuroPosition );
    USHORT nCurrencyPos = nCurrencyTableEuroPosition + 1;

    ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale > xLoc =
        LocaleDataWrapper::getInstalledLocaleNames();
    sal_Int32 nLocaleCount = xLoc.getLength();
    for ( sal_Int32 nLocale = 0; nLocale < nLocaleCount; nLocale++ )
    {
        LanguageType eLang = ConvertIsoNamesToLanguage( xLoc[nLocale].Language,
            xLoc[nLocale].Country );
#ifdef DEBUG
        LanguageType eReal = International::GetRealLanguage( eLang );
        LanguageType eNeut = International::GetNeutralLanguage( eLang );
        if ( eReal != eLang )
            BOOL bBreak = TRUE;
        if ( eNeut != eLang )
            BOOL bBreak = TRUE;
#endif
        pLocaleData->setLocale( xLoc[nLocale] );
        // one default currency for each locale
        pEntry = new NfCurrencyEntry( *pLocaleData, eLang );
        // but don't insert multiple EURo entries
        BOOL bInsert = !pEntry->IsEuro();
        if ( !bInsert )
            delete pEntry;
        else
        {
#ifndef PRODUCT
            lcl_CheckCurrencySymbolPosition( *pEntry );
#endif
            theCurrencyTable.Insert( pEntry, nCurrencyPos++ );
            if ( !nSystemCurrencyPosition && pEntry->GetLanguage() == eSysLang )
                nSystemCurrencyPosition = nCurrencyPos-1;
        }

        // all available currencies for each locale
        Sequence< Currency > aCurrSeq = pLocaleData->getAllCurrencies();
        sal_Int32 nCurrencyCount = aCurrSeq.getLength();
        if ( nCurrencyCount > 1 )
        {
            sal_Int32 nCurrency;
            for ( nCurrency = 0; nCurrency < nCurrencyCount; nCurrency++ )
            {
                pEntry = new NfCurrencyEntry( aCurrSeq[nCurrency], *pLocaleData, eLang );
                // no dupes, and don't insert multiple EURo entries
                BOOL bInsert = !pEntry->IsEuro();
                if ( bInsert )
                {
                    NfCurrencyEntry const * const * pData = theCurrencyTable.GetData();
                    USHORT n = theCurrencyTable.Count();
                    pData++;        // skip first SYSTEM entry
                    for ( USHORT j=1; j<n; j++ )
                    {
                        if ( *(*pData++) == *pEntry )
                        {
                            bInsert = FALSE;
                            break;  // for
                        }
                    }
                }
                if ( !bInsert )
                    delete pEntry;
                else
                {
                    theCurrencyTable.Insert( pEntry, nCurrencyPos++ );
                    if ( !nSystemCurrencyPosition && pEntry->GetLanguage() == eSysLang )
                        nSystemCurrencyPosition = nCurrencyPos-1;
                }
            }
        }
    }
    pLocaleData->setLocale( aSaveLocale );
    // first entry is System
    DBG_ASSERT( nSystemCurrencyPosition, "system language_country locale not in I18N format tables" );
    if ( nSystemCurrencyPosition )
    {
        if ( theCurrencyTable[nSystemCurrencyPosition]->GetSymbol() !=
                theCurrencyTable[0]->GetSymbol() )
        {
            if ( theCurrencyTable[0]->IsEuro() )
                nSystemCurrencyPosition = nCurrencyTableEuroPosition;
            else
                nSystemCurrencyPosition = 0;
        }
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
        String aPositiveBank, aNegativeBank;
        rCurr.BuildPositiveFormatString( aPositiveBank, TRUE, *pLocaleData, 1 );
        rCurr.BuildNegativeFormatString( aNegativeBank, TRUE, *pLocaleData, 1 );

        WSStringPtr pFormat1 = new String( aPositiveBank );
        *pFormat1 += ';';
        WSStringPtr pFormat2 = new String( *pFormat1 );

        String aRed( '[' );
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
        String aPositive, aNegative, aPositiveNoDec, aNegativeNoDec,
            aPositiveDashed, aNegativeDashed;
        WSStringPtr pFormat1, pFormat2, pFormat3, pFormat4, pFormat5;

        String aRed( '[' );
        aRed += pFormatScanner->GetRedString();
        aRed += ']';

        rCurr.BuildPositiveFormatString( aPositive, FALSE, *pLocaleData, 1 );
        rCurr.BuildNegativeFormatString( aNegative, FALSE, *pLocaleData, 1 );
        if ( rCurr.GetDigits() )
        {
            rCurr.BuildPositiveFormatString( aPositiveNoDec, FALSE, *pLocaleData, 0 );
            rCurr.BuildNegativeFormatString( aNegativeNoDec, FALSE, *pLocaleData, 0 );
            rCurr.BuildPositiveFormatString( aPositiveDashed, FALSE, *pLocaleData, 2 );
            rCurr.BuildNegativeFormatString( aNegativeDashed, FALSE, *pLocaleData, 2 );

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


NfCurrencyEntry::NfCurrencyEntry( const LocaleDataWrapper& rLocaleData, LanguageType eLang )
{
    aSymbol         = rLocaleData.getCurrSymbol();
    aBankSymbol     = rLocaleData.getCurrBankSymbol();
    eLanguage       = eLang;
    nPositiveFormat = rLocaleData.getCurrPositiveFormat();
    nNegativeFormat = rLocaleData.getCurrNegativeFormat();
    nDigits         = rLocaleData.getCurrDigits();
    cZeroChar       = rLocaleData.getCurrZeroChar();
}


NfCurrencyEntry::NfCurrencyEntry( const ::com::sun::star::i18n::Currency & rCurr,
            const LocaleDataWrapper& rLocaleData, LanguageType eLang )
{
    aSymbol         = rCurr.Symbol;
    aBankSymbol     = rCurr.BankSymbol;
    eLanguage       = eLang;
//!TODO: shouldn't the format information be provided by the Currency itself?
    nPositiveFormat = rLocaleData.getCurrPositiveFormat();
    nNegativeFormat = rLocaleData.getCurrNegativeFormat();
    nDigits         = rLocaleData.getCurrDigits();
    cZeroChar       = rLocaleData.getCurrZeroChar();
}


BOOL NfCurrencyEntry::operator==( const NfCurrencyEntry& r ) const
{
    return aSymbol      == r.aSymbol
        && aBankSymbol  == r.aBankSymbol
        && eLanguage    == r.eLanguage
        ;
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
    String aEuro( NfCurrencyEntry::GetEuroSymbol() );
    return aSymbol == aEuro;
}


void NfCurrencyEntry::ApplyVariableInformation( const NfCurrencyEntry& r )
{
    nPositiveFormat = r.nPositiveFormat;
    nNegativeFormat = r.nNegativeFormat;
    nDigits         = r.nDigits;
    cZeroChar       = r.cZeroChar;
}


void NfCurrencyEntry::BuildSymbolString( String& rStr, BOOL bBank,
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
            rStr += String::CreateFromInt32( sal_Int32( eLanguage ), 16 ).ToUpperAscii();
        }
    }
    rStr += ']';
}


void NfCurrencyEntry::Impl_BuildFormatStringNumChars( String& rStr,
            const LocaleDataWrapper& rLoc, USHORT nDecimalFormat ) const
{
    rStr.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "###0" ) );
    rStr.Insert( rLoc.getNumThousandSep(), 1 );
    if ( nDecimalFormat && nDigits )
    {
        rStr += rLoc.getNumDecimalSep();
        rStr.Expand( rStr.Len() + nDigits, (nDecimalFormat == 2 ? '-' : cZeroChar) );
    }
}


void NfCurrencyEntry::BuildPositiveFormatString( String& rStr, BOOL bBank,
            const LocaleDataWrapper& rLoc, USHORT nDecimalFormat ) const
{
    Impl_BuildFormatStringNumChars( rStr, rLoc, nDecimalFormat );
    USHORT nPosiForm = NfCurrencyEntry::GetEffectivePositiveFormat(
        rLoc.getCurrPositiveFormat(), nPositiveFormat, bBank );
    CompletePositiveFormatString( rStr, bBank, nPosiForm );
}


void NfCurrencyEntry::BuildNegativeFormatString( String& rStr, BOOL bBank,
            const LocaleDataWrapper& rLoc, USHORT nDecimalFormat ) const
{
    Impl_BuildFormatStringNumChars( rStr, rLoc, nDecimalFormat );
    USHORT nNegaForm = NfCurrencyEntry::GetEffectiveNegativeFormat(
        rLoc.getCurrNegativeFormat(), nNegativeFormat, bBank );
    CompleteNegativeFormatString( rStr, bBank, nNegaForm );
}


void NfCurrencyEntry::CompletePositiveFormatString( String& rStr, BOOL bBank,
            USHORT nPosiForm ) const
{
    String aSymStr;
    BuildSymbolString( aSymStr, bBank );
    NfCurrencyEntry::CompletePositiveFormatString( rStr, aSymStr, nPosiForm );
}


void NfCurrencyEntry::CompleteNegativeFormatString( String& rStr, BOOL bBank,
            USHORT nNegaForm ) const
{
    String aSymStr;
    BuildSymbolString( aSymStr, bBank );
    NfCurrencyEntry::CompleteNegativeFormatString( rStr, aSymStr, nNegaForm );
}


// static
void NfCurrencyEntry::CompletePositiveFormatString( String& rStr,
        const String& rSymStr, USHORT nPositiveFormat )
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
void NfCurrencyEntry::CompleteNegativeFormatString( String& rStr,
        const String& rSymStr, USHORT nNegativeFormat )
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
            String aTmp( rSymStr );
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



