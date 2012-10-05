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

#include <tools/color.hxx>

#define _SVX_NUMFMTSH_CXX
#include <tools/debug.hxx>
#include <i18npool/mslangid.hxx>

#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>

#include <svtools/langtab.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/processfactory.hxx>

#include <svx/numfmtsh.hxx>

#include <limits>

// class SvxNumberFormatShell --------------------------------------------

const double SvxNumberFormatShell::DEFAULT_NUMVALUE = 1234.56789;

// -----------------------------------------------------------------------



SvxNumberFormatShell* SvxNumberFormatShell::Create( SvNumberFormatter* pNumFormatter,
                                              sal_uInt32              nFormatKey,
                                              SvxNumberValueType eNumValType,
                                              const String&      rNumStr )
{
    return new SvxNumberFormatShell(pNumFormatter,nFormatKey,
                                    eNumValType,rNumStr );
}

SvxNumberFormatShell* SvxNumberFormatShell::Create( SvNumberFormatter* pNumFormatter,
                                              sal_uInt32                 nFormatKey,
                                              SvxNumberValueType eNumValType,
                                              double             nNumVal,
                                              const String*      pNumStr )
{
    return new SvxNumberFormatShell(pNumFormatter,nFormatKey,
                                    eNumValType,nNumVal,pNumStr );
}

// -----------------------------------------------------------------------

SvxNumberFormatShell::SvxNumberFormatShell( SvNumberFormatter*  pNumFormatter,
                                            sal_uInt32              nFormatKey,
                                            SvxNumberValueType  eNumValType,
                                            const String&       rNumStr ) :
    pFormatter      ( pNumFormatter ),
    pCurFmtTable    ( NULL ),
    eValType        ( eNumValType ),
    bUndoAddList    ( true ),
    nCurFormatKey   ( nFormatKey ),
    pCurCurrencyEntry(NULL),
    bBankingSymbol  (false),
    nCurCurrencyEntryPos((sal_uInt16) SELPOS_NONE),
    bUseStarFormat  (false)
{
    nValNum = DEFAULT_NUMVALUE;

    switch ( eValType )
    {
        case SVX_VALUE_TYPE_STRING:
            aValStr = rNumStr;
            break;
        case SVX_VALUE_TYPE_NUMBER:
        case SVX_VALUE_TYPE_UNDEFINED:
        default:
            aValStr.Erase();
    }
}

// -----------------------------------------------------------------------

SvxNumberFormatShell::SvxNumberFormatShell( SvNumberFormatter*  pNumFormatter,
                                            sal_uInt32              nFormatKey,
                                            SvxNumberValueType  eNumValType,
                                            double              nNumVal,
                                            const String*       pNumStr ) :
    pFormatter      ( pNumFormatter ),
    pCurFmtTable    ( NULL ),
    eValType        ( eNumValType ),
    bUndoAddList    ( true ),
    nCurFormatKey   ( nFormatKey ),
    pCurCurrencyEntry(NULL),
    bBankingSymbol  (false),
    nCurCurrencyEntryPos((sal_uInt16) SELPOS_NONE),
    bUseStarFormat  (false)
{
    //  #50441# When used in Writer, the SvxNumberInfoItem contains the
    //  original string in addition to the value

    if ( pNumStr )
        aValStr = *pNumStr;

    switch ( eValType )
    {
        case SVX_VALUE_TYPE_NUMBER:
            nValNum = nNumVal;
            break;
        case SVX_VALUE_TYPE_STRING:
        case SVX_VALUE_TYPE_UNDEFINED:
        default:
            nValNum = DEFAULT_NUMVALUE;
    }
}

// -----------------------------------------------------------------------

SvxNumberFormatShell::~SvxNumberFormatShell()
{
    /*
     * An dieser Stelle wird abhaengig davon, ob die
     * hinzugefuegten, benutzerdefinierten als gueltig
     * erklaert wurden (ValidateNewEntries()), die
     * Add-Liste wieder aus dem Zahlenformatierer entfernt.
     *
     * Loeschen von Formaten aus dem Formatierer passiert
     * aus Undo-Gruenden nur in der aufrufenden Instanz.
     */

    if ( bUndoAddList )
    {
        // Hinzugefuegte Formate sind nicht gueltig:
        // => wieder entfernen:

        for ( std::vector<sal_uInt32>::const_iterator it(aAddList.begin()); it != aAddList.end(); ++it )
            pFormatter->DeleteEntry( *it );
    }

    for ( std::vector<String*>::const_iterator it(aCurrencyFormatList.begin());
          it != aCurrencyFormatList.end(); ++it )
        delete *it;
}

// -----------------------------------------------------------------------

size_t SvxNumberFormatShell::GetUpdateDataCount() const
{
    return aDelList.size();
}

// -----------------------------------------------------------------------

void SvxNumberFormatShell::GetUpdateData( sal_uInt32* pDelArray, const sal_uInt32 nSize )
{
    const size_t nListSize = aDelList.size();

    DBG_ASSERT( pDelArray && ( nSize == nListSize ), "Array nicht initialisiert!" );

    if ( pDelArray && ( nSize == nListSize ) )
        for (std::vector<sal_uInt32>::const_iterator it(aDelList.begin()); it != aDelList.end(); ++it )
            *pDelArray++ = *it;
}

// -----------------------------------------------------------------------

void SvxNumberFormatShell::CategoryChanged( sal_uInt16 nCatLbPos,
                                            short& rFmtSelPos,
                                            std::vector<String*>& rFmtEntries )
{
    short nOldCategory = nCurCategory;
    PosToCategory_Impl( nCatLbPos, nCurCategory );
    pCurFmtTable = &( pFormatter->GetEntryTable( nCurCategory,
                                                 nCurFormatKey,
                                                 eCurLanguage ) );
    // reinitialize currency if category newly entered
    if ( nCurCategory == NUMBERFORMAT_CURRENCY && nOldCategory != nCurCategory )
        pCurCurrencyEntry = NULL;
    rFmtSelPos = FillEntryList_Impl( rFmtEntries );
}

// -----------------------------------------------------------------------

void SvxNumberFormatShell::LanguageChanged( LanguageType eLangType,
                                            short& rFmtSelPos,
                                            std::vector<String*>& rFmtEntries )
{
    eCurLanguage = eLangType;
    pCurFmtTable = &(pFormatter->ChangeCL( nCurCategory,
                                           nCurFormatKey,
                                           eCurLanguage ) );
    rFmtSelPos = FillEntryList_Impl( rFmtEntries );
}

// -----------------------------------------------------------------------

void SvxNumberFormatShell::FormatChanged( sal_uInt16  nFmtLbPos,
                                          String& rPreviewStr,
                                          Color*& rpFontColor )
{
    if( static_cast<size_t>(nFmtLbPos) < aCurEntryList.size() )
    {
        nCurFormatKey = aCurEntryList[nFmtLbPos];

        if( nCurFormatKey != NUMBERFORMAT_ENTRY_NOT_FOUND )
        {
            GetPreviewString_Impl( rPreviewStr, rpFontColor );
        }
        else if( nCurCategory == NUMBERFORMAT_CURRENCY )
        {
            if( static_cast<size_t>(nFmtLbPos) < aCurrencyFormatList.size() )
            {
                MakePrevStringFromVal(*aCurrencyFormatList[nFmtLbPos],
                                    rPreviewStr,rpFontColor,nValNum);
            }
        }
    }
}
// -----------------------------------------------------------------------

bool SvxNumberFormatShell::AddFormat( String& rFormat,  xub_StrLen& rErrPos,
                                      sal_uInt16& rCatLbSelPos, short& rFmtSelPos,
                                      std::vector<String*>& rFmtEntries )
{
    bool        bInserted   = false;
    sal_uInt32  nAddKey     = pFormatter->GetEntryKey( rFormat, eCurLanguage );

    if ( nAddKey != NUMBERFORMAT_ENTRY_NOT_FOUND ) // bereits vorhanden?
    {
        ::std::vector<sal_uInt32>::iterator nAt = GetRemoved_Impl( nAddKey );
        if ( nAt != aDelList.end() )
        {
            aDelList.erase( nAt );
            bInserted = true;
        }
        else
        {
            OSL_FAIL( "Doppeltes Format!" );
        }
    }
    else // neues Format
    {
        bInserted = pFormatter->PutEntry( rFormat, rErrPos,
                                          nCurCategory, nAddKey,
                                          eCurLanguage );
        if (bInserted)
        {
            // May be sorted under a different locale if LCID was parsed.
            const SvNumberformat* pEntry = pFormatter->GetEntry( nAddKey);
            if (pEntry)
            {
                LanguageType nLang = pEntry->GetLanguage();
                if (eCurLanguage != nLang)
                {
                    // Current language's list would not show entry, adapt.
                    eCurLanguage = nLang;
                }
            }
        }
    }

    if ( bInserted ) // eingefuegt
    {
        nCurFormatKey = nAddKey;
        DBG_ASSERT( !IsAdded_Impl( nCurFormatKey ), "Doppeltes Format!" );
        aAddList.push_back( nCurFormatKey );

        // aktuelle Tabelle holen
        pCurFmtTable = &(pFormatter->GetEntryTable( nCurCategory,
                                                    nCurFormatKey,
                                                    eCurLanguage ));
        nCurCategory=pFormatter->GetType(nAddKey); //@@ ???
        CategoryToPos_Impl( nCurCategory, rCatLbSelPos );
        rFmtSelPos = FillEntryList_Impl( rFmtEntries );
    }
    else if ( rErrPos != 0 ) // Syntaxfehler
    {
        ;
    }
    else // Doppelt einfuegen nicht moeglich
    {
        OSL_FAIL( "Doppeltes Format!" ); // oder doch?
    }

    return bInserted;
}

// -----------------------------------------------------------------------

bool SvxNumberFormatShell::RemoveFormat( const String& rFormat,
                                         sal_uInt16& rCatLbSelPos,
                                         short& rFmtSelPos,
                                         std::vector<String*>& rFmtEntries )
{
    sal_uInt32 nDelKey = pFormatter->GetEntryKey( rFormat, eCurLanguage );

    DBG_ASSERT( nDelKey != NUMBERFORMAT_ENTRY_NOT_FOUND, "Eintrag nicht gefunden!" );
    DBG_ASSERT( !IsRemoved_Impl( nDelKey ), "Eintrag bereits geloescht!" );

    if ( (nDelKey != NUMBERFORMAT_ENTRY_NOT_FOUND) && !IsRemoved_Impl( nDelKey ) )
    {
        aDelList.push_back( nDelKey );

        ::std::vector<sal_uInt32>::iterator nAt = GetAdded_Impl( nDelKey );
        if( nAt != aAddList.end() )
        {
            aAddList.erase( nAt );
        }

        nCurCategory=pFormatter->GetType(nDelKey);
        pCurFmtTable = &(pFormatter->GetEntryTable( nCurCategory,
                                                    nCurFormatKey,
                                                    eCurLanguage ));

        nCurFormatKey=pFormatter->GetStandardFormat(nCurCategory,
                                            eCurLanguage );

        CategoryToPos_Impl( nCurCategory, rCatLbSelPos );
        rFmtSelPos = FillEntryList_Impl( rFmtEntries );
    }
    return true;
}

// -----------------------------------------------------------------------

void SvxNumberFormatShell::MakeFormat( String& rFormat,
                                       bool bThousand, bool bNegRed,
                                       sal_uInt16 nPrecision, sal_uInt16 nLeadingZeroes,
                                       sal_uInt16 nCurrencyPos)
{
    if( aCurrencyFormatList.size() > static_cast<size_t>(nCurrencyPos) )
    {
        xub_StrLen rErrPos=0;
        std::vector<String*> aFmtEList;

        sal_uInt32 nFound = pFormatter->TestNewString( *aCurrencyFormatList[nCurrencyPos], eCurLanguage );

        if ( nFound == NUMBERFORMAT_ENTRY_NOT_FOUND )
        {
            sal_uInt16 rCatLbSelPos=0;
            short  rFmtSelPos=0;
            AddFormat( *aCurrencyFormatList[nCurrencyPos],rErrPos,rCatLbSelPos,
                    rFmtSelPos,aFmtEList);
        }

        if(rErrPos==0)
        {
            rFormat = pFormatter->GenerateFormat(nCurFormatKey,
                                                 eCurLanguage,
                                                 bThousand, bNegRed,
                                                 nPrecision, nLeadingZeroes);
        }
        for ( std::vector<String*>::const_iterator it(aFmtEList.begin()); it != aFmtEList.end(); ++it )
            delete *it;
    }
    else
    {
        rFormat = pFormatter->GenerateFormat(nCurFormatKey,
                                             eCurLanguage,
                                             bThousand, bNegRed,
                                             nPrecision, nLeadingZeroes);
    }
}

// -----------------------------------------------------------------------

void SvxNumberFormatShell::GetOptions( const String&    rFormat,
                                       bool&            rThousand,
                                       bool&            rNegRed,
                                       sal_uInt16&      rPrecision,
                                       sal_uInt16&      rLeadingZeroes,
                                       sal_uInt16&      rCatLbPos )
{

    sal_uInt32 nFmtKey = pFormatter->GetEntryKey( rFormat, eCurLanguage );

    if(nFmtKey != NUMBERFORMAT_ENTRY_NOT_FOUND)
    {
        if ( nFmtKey != NUMBERFORMAT_ENTRY_NOT_FOUND )
        {
            pFormatter->GetFormatSpecialInfo( nFmtKey,
                                              rThousand, rNegRed,
                                              rPrecision, rLeadingZeroes );

            CategoryToPos_Impl( pFormatter->GetType( nFmtKey ), rCatLbPos );
        }
        else
            rCatLbPos = CAT_USERDEFINED;
    }
    else
    {
        bool bTestBanking = false;
        sal_uInt16 nPos=FindCurrencyTableEntry(rFormat, bTestBanking );

        if(IsInTable(nPos,bTestBanking,rFormat) &&
            pFormatter->GetFormatSpecialInfo( rFormat,rThousand, rNegRed,
                                  rPrecision, rLeadingZeroes,eCurLanguage)==0)
        {
            rCatLbPos = CAT_CURRENCY;
        }
        else
            rCatLbPos = CAT_USERDEFINED;
    }

}

// -----------------------------------------------------------------------

void SvxNumberFormatShell::MakePreviewString( const String& rFormatStr,
                                              String&       rPreviewStr,
                                              Color*&       rpFontColor )
{
    rpFontColor = NULL;

    sal_uIntPtr nExistingFormat = pFormatter->GetEntryKey( rFormatStr, eCurLanguage );
    if ( nExistingFormat == NUMBERFORMAT_ENTRY_NOT_FOUND )
    {
        //  real preview - not implemented in NumberFormatter for text formats

        pFormatter->GetPreviewString( rFormatStr, nValNum, rPreviewStr,
                                      &rpFontColor, eCurLanguage, bUseStarFormat );
    }
    else
    {
        //  format exists

        //  #50441# if a string was set in addition to the value, use it for text formats
        bool bUseText = ( eValType == SVX_VALUE_TYPE_STRING ||
                            ( aValStr.Len() && ( pFormatter->GetType(nExistingFormat) & NUMBERFORMAT_TEXT ) ) );
        if ( bUseText )
            pFormatter->GetOutputString( aValStr, nExistingFormat,
                                         rPreviewStr, &rpFontColor );
        else
            pFormatter->GetOutputString( nValNum, nExistingFormat,
                                         rPreviewStr, &rpFontColor, bUseStarFormat );
    }
}

// -----------------------------------------------------------------------

bool SvxNumberFormatShell::IsUserDefined( const String& rFmtString )
{
    sal_uInt32 nFound = pFormatter->GetEntryKey( rFmtString, eCurLanguage );

    bool bFlag=false;
    if ( nFound != NUMBERFORMAT_ENTRY_NOT_FOUND )
    {
        bFlag=pFormatter->IsUserDefined( rFmtString, eCurLanguage );

        if(bFlag)
        {
            const SvNumberformat* pNumEntry = pFormatter->GetEntry(nFound);

            if(pNumEntry!=NULL && pNumEntry->HasNewCurrency())
            {
                bool bTestBanking;
                sal_uInt16 nPos=FindCurrencyTableEntry(rFmtString,bTestBanking);
                bFlag=!IsInTable(nPos,bTestBanking,rFmtString);
            }
        }
    }
    return bFlag;
}

// -----------------------------------------------------------------------

bool SvxNumberFormatShell::FindEntry( const String& rFmtString, sal_uInt32* pAt /* = NULL */ )
{
    bool bRes=false;
    sal_uInt32 nFound = pFormatter->TestNewString( rFmtString, eCurLanguage );

    if ( nFound == NUMBERFORMAT_ENTRY_NOT_FOUND )
    {
        bool bTestBanking=false;
        sal_uInt16 nPos=FindCurrencyTableEntry(rFmtString, bTestBanking );

        if(IsInTable(nPos,bTestBanking,rFmtString))
        {
            nFound=NUMBERFORMAT_ENTRY_NEW_CURRENCY;
            bRes=true;
        }
    }
    else
    {
        bRes=!IsRemoved_Impl( nFound );
    }

    if ( pAt )
        *pAt = nFound;

    return bRes;
}


// -----------------------------------------------------------------------

void SvxNumberFormatShell::GetInitSettings( sal_uInt16& nCatLbPos,
                                            LanguageType& rLangType,
                                            sal_uInt16& nFmtLbSelPos,
                                            std::vector<String*>& rFmtEntries,
                                            String& rPrevString,
                                            Color*& rpPrevColor )
{
    // -------------------------------------------------------------------
    // Vorbedingung: Zahlenformatierer gefunden
    DBG_ASSERT( pFormatter != NULL, "Zahlenformatierer nicht gefunden!" );

//  sal_uInt16                  nCount      = 0;
    short                   nSelPos     = SELPOS_NONE;
//  SvNumberFormatTable*    pFmtTable   = NULL;

    // Sonderbehandlung fuer undefiniertes Zahlenformat:
    if ( (eValType == SVX_VALUE_TYPE_UNDEFINED) && (nCurFormatKey == 0) )
        PosToCategory_Impl( CAT_ALL, nCurCategory );        // Kategorie = Alle
    else
        nCurCategory = NUMBERFORMAT_UNDEFINED;      // Kategorie = Undefiniert

    pCurFmtTable =  &(pFormatter->GetFirstEntryTable( nCurCategory,
                                                      nCurFormatKey,
                                                      eCurLanguage ));



    CategoryToPos_Impl( nCurCategory, nCatLbPos );
    rLangType = eCurLanguage;

    nSelPos = FillEntryList_Impl( rFmtEntries );

    DBG_ASSERT( nSelPos != SELPOS_NONE, "Leere Formatliste!" );

    nFmtLbSelPos = (nSelPos != SELPOS_NONE) ? (sal_uInt16)nSelPos : 0;
    GetPreviewString_Impl( rPrevString, rpPrevColor );
}

// -----------------------------------------------------------------------

short SvxNumberFormatShell::FillEntryList_Impl( std::vector<String*>& rList )
{
    /* Erstellen einer aktuellen Liste von Format-Eintraegen.
     * Rueckgabewert ist die Listenposition des aktuellen Formates.
     * Ist die Liste leer oder gibt es kein aktuelles Format,
     * so wird SELPOS_NONE geliefert.
     */
    short nSelPos=0;
    sal_uInt16 nPrivCat = CAT_CURRENCY;
    nSelPos=SELPOS_NONE;

    aCurEntryList.clear();

    if(nCurCategory==NUMBERFORMAT_ALL)
    {
        FillEListWithStd_Impl(rList,CAT_NUMBER,nSelPos);
        FillEListWithStd_Impl(rList,CAT_PERCENT,nSelPos);
        FillEListWithStd_Impl(rList,CAT_CURRENCY,nSelPos);
        FillEListWithStd_Impl(rList,CAT_DATE,nSelPos);
        FillEListWithStd_Impl(rList,CAT_TIME,nSelPos);
        FillEListWithStd_Impl(rList,CAT_SCIENTIFIC,nSelPos);
        FillEListWithStd_Impl(rList,CAT_FRACTION,nSelPos);
        FillEListWithStd_Impl(rList,CAT_BOOLEAN,nSelPos);
        FillEListWithStd_Impl(rList,CAT_TEXT,nSelPos);
    }
    else
    {
        CategoryToPos_Impl(nCurCategory, nPrivCat);
        FillEListWithStd_Impl(rList,nPrivCat,nSelPos);
    }

    if( nPrivCat!=CAT_CURRENCY)
        nSelPos=FillEListWithUsD_Impl(rList,nPrivCat,nSelPos);

    return nSelPos;
}

void SvxNumberFormatShell::FillEListWithStd_Impl( std::vector<String*>& rList,
                                                  sal_uInt16 nPrivCat,short &nSelPos )
{
    /* Erstellen einer aktuellen Liste von Format-Eintraegen.
     * Rueckgabewert ist die Listenposition des aktuellen Formates.
     * Ist die Liste leer oder gibt es kein aktuelles Format,
     * so wird SELPOS_NONE geliefert.
     */
    DBG_ASSERT( pCurFmtTable != NULL, "Unbekanntes Zahlenformat!" );

    for ( std::vector<String*>::const_iterator it(aCurrencyFormatList.begin());
          it != aCurrencyFormatList.end(); ++it )
        delete *it;
    aCurrencyFormatList.clear();

    if(nPrivCat==CAT_CURRENCY)
    {
        nSelPos=FillEListWithCurrency_Impl(rList,nSelPos);
    }
    else
    {
        NfIndexTableOffset eOffsetStart;
        NfIndexTableOffset eOffsetEnd;

        switch(nPrivCat)
        {
            case CAT_NUMBER         :eOffsetStart=NF_NUMBER_START;
                                     eOffsetEnd=NF_NUMBER_END;
                                     break;
            case CAT_PERCENT        :eOffsetStart=NF_PERCENT_START;
                                     eOffsetEnd=NF_PERCENT_END;
                                     break;
            case CAT_CURRENCY       :eOffsetStart=NF_CURRENCY_START;
                                     eOffsetEnd=NF_CURRENCY_END;
                                     break;
            case CAT_DATE           :eOffsetStart=NF_DATE_START;
                                     eOffsetEnd=NF_DATE_END;
                                     break;
            case CAT_TIME           :eOffsetStart=NF_TIME_START;
                                     eOffsetEnd=NF_TIME_END;
                                     break;
            case CAT_SCIENTIFIC     :eOffsetStart=NF_SCIENTIFIC_START;
                                     eOffsetEnd=NF_SCIENTIFIC_END;
                                     break;
            case CAT_FRACTION       :eOffsetStart=NF_FRACTION_START;
                                     eOffsetEnd=NF_FRACTION_END;
                                     break;
            case CAT_BOOLEAN        :eOffsetStart=NF_BOOLEAN;
                                     eOffsetEnd=NF_BOOLEAN;
                                     break;
            case CAT_TEXT           :eOffsetStart=NF_TEXT;
                                     eOffsetEnd=NF_TEXT;
                                     break;
            default                 :return;
        }

        nSelPos=FillEListWithFormats_Impl(rList,nSelPos,eOffsetStart,eOffsetEnd);

        if(nPrivCat==CAT_DATE || nPrivCat==CAT_TIME)
        {
            nSelPos=FillEListWithDateTime_Impl(rList,nSelPos);
            //if(nSelPos!=SELPOS_NONE) nSelPos=nTmpPos;
        }
    }
}

short SvxNumberFormatShell::FillEListWithFormats_Impl( std::vector<String*>& rList,
                                                       short nSelPos,
                                                       NfIndexTableOffset eOffsetStart,
                                                       NfIndexTableOffset eOffsetEnd)
{
    /* Erstellen einer aktuellen Liste von Format-Eintraegen.
     * Rueckgabewert ist die Listenposition des aktuellen Formates.
     * Ist die Liste leer oder gibt es kein aktuelles Format,
     * so wird SELPOS_NONE geliefert.
     */
    sal_uInt16  nMyType;

    DBG_ASSERT( pCurFmtTable != NULL, "Unbekanntes Zahlenformat!" );

    const SvNumberformat*   pNumEntry   = pCurFmtTable->empty() ? 0 : pCurFmtTable->begin()->second;
    sal_uInt32          nNFEntry;
    String          aStrComment;
    String          aNewFormNInfo;
    String          aPrevString;
    String          a2PrevString;

    short           nMyCat      = SELPOS_NONE;

    long nIndex;

    for(nIndex=eOffsetStart;nIndex<=eOffsetEnd;nIndex++)
    {
        nNFEntry=pFormatter->GetFormatIndex((NfIndexTableOffset)nIndex,eCurLanguage);

        pNumEntry   = pFormatter->GetEntry(nNFEntry);

        if(pNumEntry==NULL) continue;

        nMyCat=pNumEntry->GetType() & ~NUMBERFORMAT_DEFINED;
        aStrComment=pNumEntry->GetComment();
        CategoryToPos_Impl(nMyCat,nMyType);
        aNewFormNInfo=  pNumEntry->GetFormatstring();

        String *const pStr = new String(aNewFormNInfo);

        if ( nNFEntry == nCurFormatKey )
        {
            nSelPos = ( !IsRemoved_Impl( nNFEntry ) ) ? aCurEntryList.size() : SELPOS_NONE;
        }

        rList.push_back( pStr );
        aCurEntryList.push_back( nNFEntry );
    }

    return nSelPos;
}

short SvxNumberFormatShell::FillEListWithDateTime_Impl( std::vector<String*>& rList,
                                                        short nSelPos)
{
    sal_uInt16  nMyType;

    DBG_ASSERT( pCurFmtTable != NULL, "Unbekanntes Zahlenformat!" );

    const SvNumberformat*   pNumEntry   = pCurFmtTable->empty() ? 0 : pCurFmtTable->begin()->second;
    sal_uInt32          nNFEntry;
    String          aStrComment;
    String          aNewFormNInfo;
    String          aPrevString;
    String          a2PrevString;

    short           nMyCat      = SELPOS_NONE;

    long nIndex;

    for(nIndex=NF_DATETIME_START;nIndex<=NF_DATETIME_END;nIndex++)
    {
        nNFEntry=pFormatter->GetFormatIndex((NfIndexTableOffset)nIndex,eCurLanguage);

        pNumEntry   = pFormatter->GetEntry(nNFEntry);
        if(pNumEntry!=NULL)
        {
            nMyCat=pNumEntry->GetType() & ~NUMBERFORMAT_DEFINED;
            aStrComment=pNumEntry->GetComment();
            CategoryToPos_Impl(nMyCat,nMyType);
            aNewFormNInfo=  pNumEntry->GetFormatstring();

            String *const pStr = new String(aNewFormNInfo);

            if ( nNFEntry == nCurFormatKey )
            {
                nSelPos = ( !IsRemoved_Impl( nNFEntry ) ) ? aCurEntryList.size() : SELPOS_NONE;
            }

            rList.push_back( pStr );
            aCurEntryList.push_back( nNFEntry );
        }
    }

    return nSelPos;
}

short SvxNumberFormatShell::FillEListWithCurrency_Impl( std::vector<String*>& rList,
                                                        short nSelPos)
{
    /* Erstellen einer aktuellen Liste von Format-Eintraegen.
     * Rueckgabewert ist die Listenposition des aktuellen Formates.
     * Ist die Liste leer oder gibt es kein aktuelles Format,
     * so wird SELPOS_NONE geliefert.
     */
    DBG_ASSERT( pCurFmtTable != NULL, "Unbekanntes Zahlenformat!" );

    const NfCurrencyEntry* pTmpCurrencyEntry;
    bool             bTmpBanking;
    XubString        rSymbol;

    bool bFlag=pFormatter->GetNewCurrencySymbolString(nCurFormatKey,rSymbol,
                &pTmpCurrencyEntry,&bTmpBanking);

    if((!bFlag && pCurCurrencyEntry==NULL)  ||
        (bFlag && pTmpCurrencyEntry==NULL && !rSymbol.Len())    ||
        nCurCategory==NUMBERFORMAT_ALL)
    {
        if ( nCurCategory == NUMBERFORMAT_ALL )
            FillEListWithUserCurrencys(rList,nSelPos);
        nSelPos=FillEListWithSysCurrencys(rList,nSelPos);
    }
    else
    {
        nSelPos=FillEListWithUserCurrencys(rList,nSelPos);
    }

    return nSelPos;
}


short SvxNumberFormatShell::FillEListWithSysCurrencys( std::vector<String*>& rList,
                                                       short nSelPos)
{
    /* Erstellen einer aktuellen Liste von Format-Eintraegen.
     * Rueckgabewert ist die Listenposition des aktuellen Formates.
     * Ist die Liste leer oder gibt es kein aktuelles Format,
     * so wird SELPOS_NONE geliefert.
     */
    sal_uInt16  nMyType;

    DBG_ASSERT( pCurFmtTable != NULL, "Unbekanntes Zahlenformat!" );

    const SvNumberformat*   pNumEntry   = pCurFmtTable->empty() ? 0 : pCurFmtTable->begin()->second;
    sal_uInt32          nNFEntry;
    String          aStrComment;
    String          aNewFormNInfo;
    String          aPrevString;
    String          a2PrevString;

    nCurCurrencyEntryPos=0;

    short           nMyCat      = SELPOS_NONE;

    NfIndexTableOffset eOffsetStart=NF_CURRENCY_START;
    NfIndexTableOffset eOffsetEnd=NF_CURRENCY_END;
    long nIndex;

    for(nIndex=eOffsetStart;nIndex<=eOffsetEnd;nIndex++)
    {
        nNFEntry=pFormatter->GetFormatIndex((NfIndexTableOffset)nIndex,eCurLanguage);

        pNumEntry   = pFormatter->GetEntry(nNFEntry);

        if(pNumEntry==NULL) continue;

        nMyCat=pNumEntry->GetType() & ~NUMBERFORMAT_DEFINED;
        aStrComment=pNumEntry->GetComment();
        CategoryToPos_Impl(nMyCat,nMyType);
        aNewFormNInfo=  pNumEntry->GetFormatstring();

        String *const pStr = new String(aNewFormNInfo);

        if ( nNFEntry == nCurFormatKey )
        {
            nSelPos = ( !IsRemoved_Impl( nNFEntry ) ) ? aCurEntryList.size() : SELPOS_NONE;
        }

        rList.push_back( pStr );
        aCurEntryList.push_back( nNFEntry );
    }

    if(nCurCategory!=NUMBERFORMAT_ALL)
    {
        SvNumberFormatTable::iterator it = pCurFmtTable->begin();

        while ( it != pCurFmtTable->end() )
        {
            sal_uInt32 nKey = it->first;
            pNumEntry   = it->second;

            if ( !IsRemoved_Impl( nKey ))
            {
                bool bUserNewCurrency=false;
                if(pNumEntry->HasNewCurrency())
                {
                    const NfCurrencyEntry* pTmpCurrencyEntry;
                    bool            bTmpBanking;
                    XubString       rSymbol;

                    pFormatter->GetNewCurrencySymbolString(nKey,rSymbol,
                        &pTmpCurrencyEntry,&bTmpBanking);

                    bUserNewCurrency=(pTmpCurrencyEntry!=NULL);
                }

                if(!bUserNewCurrency &&(pNumEntry->GetType() & NUMBERFORMAT_DEFINED))
                {
                    nMyCat=pNumEntry->GetType() & ~NUMBERFORMAT_DEFINED;
                    aStrComment=pNumEntry->GetComment();
                    CategoryToPos_Impl(nMyCat,nMyType);
                    aNewFormNInfo=  pNumEntry->GetFormatstring();

                    String *const pStr = new String(aNewFormNInfo);

                    if ( nKey == nCurFormatKey ) nSelPos =aCurEntryList.size();
                    rList.push_back( pStr );
                    aCurEntryList.push_back( nKey );
                }
            }
            ++it;
        }
    }
    return nSelPos;
}

short SvxNumberFormatShell::FillEListWithUserCurrencys( std::vector<String*>& rList,
                                                        short nSelPos)
{
    /* Erstellen einer aktuellen Liste von Format-Eintraegen.
     * Rueckgabewert ist die Listenposition des aktuellen Formates.
     * Ist die Liste leer oder gibt es kein aktuelles Format,
     * so wird SELPOS_NONE geliefert.
     */
    sal_uInt16 nMyType;

    DBG_ASSERT( pCurFmtTable != NULL, "Unbekanntes Zahlenformat!" );

    String          aStrComment;
    String          aNewFormNInfo;
    String          aPrevString;
    String          a2PrevString;
    short           nMyCat = SELPOS_NONE;

    const NfCurrencyEntry* pTmpCurrencyEntry;
    bool            bTmpBanking, bAdaptSelPos;
    XubString       rSymbol;
    XubString       rBankSymbol;

    std::vector<String*>    aList;
    std::vector<sal_uInt32> aKeyList;

    pFormatter->GetNewCurrencySymbolString(nCurFormatKey,rSymbol,
                &pTmpCurrencyEntry,&bTmpBanking);

    XubString rShortSymbol;

    if(pCurCurrencyEntry==NULL)
    {
        // #110398# If no currency format was previously selected (we're not
        // about to add another currency), try to select the initial currency
        // format (nCurFormatKey) that was set in FormatChanged() after
        // matching the format string entered in the dialog.
        bAdaptSelPos = true;
        pCurCurrencyEntry=(NfCurrencyEntry*)pTmpCurrencyEntry;
        bBankingSymbol=bTmpBanking;
        nCurCurrencyEntryPos=FindCurrencyFormat(pTmpCurrencyEntry,bTmpBanking);
    }
    else
    {
        if (pTmpCurrencyEntry == pCurCurrencyEntry)
            bAdaptSelPos = true;
        else
        {
            bAdaptSelPos = false;
            pTmpCurrencyEntry = pCurCurrencyEntry;
        }
        bTmpBanking=bBankingSymbol;
    }

    if(pTmpCurrencyEntry!=NULL)
    {
        rSymbol = pTmpCurrencyEntry->BuildSymbolString(false);
        rBankSymbol = pTmpCurrencyEntry->BuildSymbolString(true);
        rShortSymbol = pTmpCurrencyEntry->BuildSymbolString(bTmpBanking,true);
    }

    SvNumberFormatTable::iterator it = pCurFmtTable->begin();
    while ( it != pCurFmtTable->end() )
    {
        sal_uInt32 nKey = it->first;
        const SvNumberformat* pNumEntry = it->second;

        if ( !IsRemoved_Impl( nKey ) )
        {
            if( pNumEntry->GetType() & NUMBERFORMAT_DEFINED ||
                    pNumEntry->IsAdditionalStandardDefined() )
            {
                nMyCat=pNumEntry->GetType() & ~NUMBERFORMAT_DEFINED;
                aStrComment=pNumEntry->GetComment();
                CategoryToPos_Impl(nMyCat,nMyType);
                aNewFormNInfo=  pNumEntry->GetFormatstring();

                bool bInsFlag = false;
                if ( pNumEntry->HasNewCurrency() )
                {
                    bInsFlag = true;    // merge locale formats into currency selection
                }
                else if( (!bTmpBanking && aNewFormNInfo.Search(rSymbol)!=STRING_NOTFOUND) ||
                   (bTmpBanking && aNewFormNInfo.Search(rBankSymbol)!=STRING_NOTFOUND) )
                {
                    bInsFlag = true;
                }
                else if(aNewFormNInfo.Search(rShortSymbol)!=STRING_NOTFOUND)
                {
                    XubString rTstSymbol;
                    const NfCurrencyEntry* pTstCurrencyEntry;
                    bool bTstBanking;

                    pFormatter->GetNewCurrencySymbolString(nKey,rTstSymbol,
                                &pTstCurrencyEntry,&bTstBanking);

                    if(pTmpCurrencyEntry==pTstCurrencyEntry && bTstBanking==bTmpBanking)
                    {
                        bInsFlag = true;
                    }

                }

                if(bInsFlag)
                {
                    aList.push_back( new String(aNewFormNInfo) );
                    aKeyList.push_back( nKey );
                }
            }
        }
        ++it;
    }

    NfWSStringsDtor aWSStringsDtor;
    sal_uInt16 nDefault;
    if ( pTmpCurrencyEntry && nCurCategory != NUMBERFORMAT_ALL )
    {
        nDefault = pFormatter->GetCurrencyFormatStrings(
            aWSStringsDtor, *pTmpCurrencyEntry, bTmpBanking );
        if ( !bTmpBanking )
            pFormatter->GetCurrencyFormatStrings(
                aWSStringsDtor, *pTmpCurrencyEntry, true );
    }
    else
        nDefault = 0;
    if ( !bTmpBanking && nCurCategory != NUMBERFORMAT_ALL )
    {   // append formats for all currencies defined in the current I18N locale
        const NfCurrencyTable& rCurrencyTable = SvNumberFormatter::GetTheCurrencyTable();
        sal_uInt16 nCurrCount = rCurrencyTable.size();
        LanguageType eLang = MsLangId::getRealLanguage( eCurLanguage );
        for ( sal_uInt16 i=0; i < nCurrCount; ++i )
        {
            const NfCurrencyEntry* pCurr = &rCurrencyTable[i];
            if ( pCurr->GetLanguage() == eLang && pTmpCurrencyEntry != pCurr )
            {
                pFormatter->GetCurrencyFormatStrings( aWSStringsDtor, *pCurr, false );
                pFormatter->GetCurrencyFormatStrings( aWSStringsDtor, *pCurr, true );
            }
        }
    }

    size_t nOldListCount = rList.size();
    for( size_t i = 0, nPos = nOldListCount; i < aWSStringsDtor.size(); ++i )
    {
        bool bFlag = true;
        String aInsStr(aWSStringsDtor[i]);
        size_t j;
        for( j=0; j < aList.size(); ++j )
        {
            const String * pTestStr=aList[j];

            if(*pTestStr==aInsStr)
            {
                bFlag = false;
                break;
            }
        }
        if(bFlag)
        {
            rList.push_back( new String(aInsStr) );
            aCurEntryList.insert( aCurEntryList.begin() + (nPos++), NUMBERFORMAT_ENTRY_NOT_FOUND);
        }
        else
        {
            rList.push_back( aList[j] );
            aList.erase( aList.begin()+j );
            aCurEntryList.insert( aCurEntryList.begin() + (nPos++), aKeyList[j]);
            aKeyList.erase( aKeyList.begin()+j );
        }
    }

    for( size_t i = 0; i < aKeyList.size(); ++i )
    {
        if( aKeyList[i] != NUMBERFORMAT_ENTRY_NOT_FOUND )
        {
            rList.push_back( aList[i] );
            aCurEntryList.push_back( aKeyList[i] );
        }
    }

    for( size_t i = nOldListCount; i < rList.size(); ++i )
    {
        aCurrencyFormatList.push_back( new String(*rList[i]) );

        if ( nSelPos == SELPOS_NONE && bAdaptSelPos && aCurEntryList[i] == nCurFormatKey )
            nSelPos = i;
    }

    if ( nSelPos == SELPOS_NONE && nCurCategory != NUMBERFORMAT_ALL )
        nSelPos = nDefault;

    return nSelPos;
}


short SvxNumberFormatShell::FillEListWithUsD_Impl( std::vector<String*>& rList,
                                                   sal_uInt16 nPrivCat, short nSelPos )
{
    /* Erstellen einer aktuellen Liste von Format-Eintraegen.
     * Rueckgabewert ist die Listenposition des aktuellen Formates.
     * Ist die Liste leer oder gibt es kein aktuelles Format,
     * so wird SELPOS_NONE geliefert.
     */
    sal_uInt16 nMyType;

    DBG_ASSERT( pCurFmtTable != NULL, "Unbekanntes Zahlenformat!" );

    String          aStrComment;
    String          aNewFormNInfo;
    String          aPrevString;
    String          a2PrevString;

    short           nMyCat      = SELPOS_NONE;
    bool            bAdditional = (nPrivCat != CAT_USERDEFINED &&
                                    nCurCategory != NUMBERFORMAT_ALL);

    SvNumberFormatTable::iterator it = pCurFmtTable->begin();
    while ( it != pCurFmtTable->end() )
    {
        sal_uInt32 nKey = it->first;
        const SvNumberformat* pNumEntry = it->second;

        if ( !IsRemoved_Impl( nKey ) )
        {
            if( (pNumEntry->GetType() & NUMBERFORMAT_DEFINED) ||
                    (bAdditional && pNumEntry->IsAdditionalStandardDefined()) )
            {
                nMyCat=pNumEntry->GetType() & ~NUMBERFORMAT_DEFINED;
                aStrComment=pNumEntry->GetComment();
                CategoryToPos_Impl(nMyCat,nMyType);
                aNewFormNInfo=  pNumEntry->GetFormatstring();

                bool bFlag=true;
                if(pNumEntry->HasNewCurrency())
                {
                    bool bTestBanking;
                    sal_uInt16 nPos=FindCurrencyTableEntry(aNewFormNInfo,bTestBanking);
                    bFlag=!IsInTable(nPos,bTestBanking,aNewFormNInfo);
                }
                if(bFlag)
                {
                    String *const pStr = new String(aNewFormNInfo);

                    if ( nKey == nCurFormatKey ) nSelPos = aCurEntryList.size();
                    rList.push_back( pStr );
                    aCurEntryList.push_back( nKey );
                }
            }
        }
        ++it;
    }
    return nSelPos;
}


// -----------------------------------------------------------------------

void SvxNumberFormatShell::GetPreviewString_Impl( String& rString, Color*& rpColor )
{
    rpColor = NULL;

    //  #50441# if a string was set in addition to the value, use it for text formats
    bool bUseText = ( eValType == SVX_VALUE_TYPE_STRING ||
                        ( aValStr.Len() && ( pFormatter->GetType(nCurFormatKey) & NUMBERFORMAT_TEXT ) ) );

    if ( bUseText )
        pFormatter->GetOutputString( aValStr, nCurFormatKey, rString, &rpColor );
    else
        pFormatter->GetOutputString( nValNum, nCurFormatKey, rString, &rpColor, bUseStarFormat );
}

// -----------------------------------------------------------------------

::std::vector<sal_uInt32>::iterator SvxNumberFormatShell::GetRemoved_Impl( size_t nKey )
{
    return ::std::find(aDelList.begin(), aDelList.end(), nKey);
}

// -----------------------------------------------------------------------

bool SvxNumberFormatShell::IsRemoved_Impl( size_t nKey )
{
    return GetRemoved_Impl( nKey ) != aDelList.end();
}

// -----------------------------------------------------------------------

::std::vector<sal_uInt32>::iterator SvxNumberFormatShell::GetAdded_Impl( size_t nKey )
{
    return ::std::find(aAddList.begin(), aAddList.end(), nKey);
}

//------------------------------------------------------------------------

bool SvxNumberFormatShell::IsAdded_Impl( size_t nKey )
{
    return GetAdded_Impl( nKey ) != aAddList.end();
}

// -----------------------------------------------------------------------
// Konvertierungs-Routinen:
// ------------------------

void SvxNumberFormatShell::PosToCategory_Impl( sal_uInt16 nPos, short& rCategory )
{
    // Kategorie ::com::sun::star::form-Positionen abbilden (->Resource)
    switch ( nPos )
    {
        case CAT_USERDEFINED:   rCategory = NUMBERFORMAT_DEFINED;       break;
        case CAT_NUMBER:        rCategory = NUMBERFORMAT_NUMBER;        break;
        case CAT_PERCENT:       rCategory = NUMBERFORMAT_PERCENT;       break;
        case CAT_CURRENCY:      rCategory = NUMBERFORMAT_CURRENCY;      break;
        case CAT_DATE:          rCategory = NUMBERFORMAT_DATE;          break;
        case CAT_TIME:          rCategory = NUMBERFORMAT_TIME;          break;
        case CAT_SCIENTIFIC:    rCategory = NUMBERFORMAT_SCIENTIFIC;    break;
        case CAT_FRACTION:      rCategory = NUMBERFORMAT_FRACTION;      break;
        case CAT_BOOLEAN:       rCategory = NUMBERFORMAT_LOGICAL;       break;
        case CAT_TEXT:          rCategory = NUMBERFORMAT_TEXT;          break;
        case CAT_ALL:
        default:    rCategory = NUMBERFORMAT_ALL; break;
    }
}

// -----------------------------------------------------------------------

void SvxNumberFormatShell::CategoryToPos_Impl( short nCategory, sal_uInt16& rPos )
{
    // Kategorie auf ::com::sun::star::form-Positionen abbilden (->Resource)
    switch ( nCategory )
    {
        case NUMBERFORMAT_DEFINED:      rPos = CAT_USERDEFINED; break;
        case NUMBERFORMAT_NUMBER:       rPos = CAT_NUMBER;      break;
        case NUMBERFORMAT_PERCENT:      rPos = CAT_PERCENT;     break;
        case NUMBERFORMAT_CURRENCY:     rPos = CAT_CURRENCY;    break;
        case NUMBERFORMAT_DATETIME:
        case NUMBERFORMAT_DATE:         rPos = CAT_DATE;        break;
        case NUMBERFORMAT_TIME:         rPos = CAT_TIME;        break;
        case NUMBERFORMAT_SCIENTIFIC:   rPos = CAT_SCIENTIFIC;  break;
        case NUMBERFORMAT_FRACTION:     rPos = CAT_FRACTION;    break;
        case NUMBERFORMAT_LOGICAL:      rPos = CAT_BOOLEAN;     break;
        case NUMBERFORMAT_TEXT:         rPos = CAT_TEXT;        break;
        case NUMBERFORMAT_ALL:
        default:                        rPos = CAT_ALL;
    }
}


/*************************************************************************
#*  Member:     MakePrevStringFromVal                       Datum:19.09.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxNumberFormatShell
#*
#*  Funktion:   Formatiert die Zahl nValue abhaengig von rFormatStr
#*              und speichert das Ergebnis in rPreviewStr.
#*
#*  Input:      FormatString, Farbe, zu formatierende Zahl
#*
#*  Output:     Ausgabestring rPreviewStr
#*
#************************************************************************/

void SvxNumberFormatShell::MakePrevStringFromVal(
        const String& rFormatStr,
        String& rPreviewStr,
        Color*& rpFontColor,
        double  nValue)
{
    rpFontColor = NULL;
    pFormatter->GetPreviewString( rFormatStr, nValue, rPreviewStr, &rpFontColor, eCurLanguage );
}

/*************************************************************************
#*  Member:     GetComment4Entry                            Datum:30.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxNumberFormatShell
#*
#*  Funktion:   Liefert den Kommentar fuer einen gegebenen
#*              Eintrag zurueck.
#*
#*  Input:      Nummer des Eintrags
#*
#*  Output:     Kommentar-String
#*
#************************************************************************/

void SvxNumberFormatShell::SetComment4Entry(short nEntry,String aEntStr)
{
    SvNumberformat *pNumEntry;
    if(nEntry<0) return;
    sal_uInt32  nMyNfEntry=aCurEntryList[nEntry];
    pNumEntry = (SvNumberformat*)pFormatter->GetEntry(nMyNfEntry);
    if(pNumEntry!=NULL) pNumEntry->SetComment(aEntStr);
}

/*************************************************************************
#*  Member:     GetComment4Entry                            Datum:30.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxNumberFormatShell
#*
#*  Funktion:   Liefert den Kommentar fuer einen gegebenen
#*              Eintrag zurueck.
#*
#*  Input:      Nummer des Eintrags
#*
#*  Output:     Kommentar-String
#*
#************************************************************************/

String SvxNumberFormatShell::GetComment4Entry(short nEntry)
{
    const SvNumberformat *pNumEntry;

    if(nEntry < 0)
        return String();

    if( static_cast<size_t>(nEntry) < aCurEntryList.size())
    {
        sal_uInt32  nMyNfEntry=aCurEntryList[nEntry];
        pNumEntry = pFormatter->GetEntry(nMyNfEntry);
        if(pNumEntry!=NULL)
            return pNumEntry->GetComment();
    }

    return String();
}

/*************************************************************************
#*  Member:     GetCategory4Entry                           Datum:30.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxNumberFormatShell
#*
#*  Funktion:   Liefert die Kategorie- Nummer fuer einen gegebenen
#*              Eintrag zurueck.
#*
#*  Input:      Nummer des Eintrags
#*
#*  Output:     Kategorie- Nummer
#*
#************************************************************************/

short SvxNumberFormatShell::GetCategory4Entry(short nEntry)
{
    const SvNumberformat *pNumEntry;
    if(nEntry<0) return 0;

    if( static_cast<size_t>(nEntry) < aCurEntryList.size() )
    {
        sal_uInt32  nMyNfEntry=aCurEntryList[nEntry];

        if(nMyNfEntry!=NUMBERFORMAT_ENTRY_NOT_FOUND)
        {
            pNumEntry = pFormatter->GetEntry(nMyNfEntry);
            sal_uInt16 nMyCat,nMyType;
            if(pNumEntry!=NULL)
            {
                nMyCat=pNumEntry->GetType() & ~NUMBERFORMAT_DEFINED;
                CategoryToPos_Impl(nMyCat,nMyType);

                return (short) nMyType;
            }
            return 0;
        }
        else if( !aCurrencyFormatList.empty() )
        {
            return CAT_CURRENCY;
        }
    }
    return 0;

}

/*************************************************************************
#*  Member:     GetUserDefined4Entry                        Datum:31.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxNumberFormatShell
#*
#*  Funktion:   Liefert die Information, ob ein Eintrag
#*              benutzerspezifisch ist zurueck.
#*
#*  Input:      Nummer des Eintrags
#*
#*  Output:     Benutzerspezifisch?
#*
#************************************************************************/

bool SvxNumberFormatShell::GetUserDefined4Entry(short nEntry)
{
    const SvNumberformat *pNumEntry;
    if(nEntry<0) return 0;

    if( static_cast<size_t>(nEntry) < aCurEntryList.size())
    {
        sal_uInt32  nMyNfEntry=aCurEntryList[nEntry];
        pNumEntry = pFormatter->GetEntry(nMyNfEntry);

        if(pNumEntry!=NULL)
        {
            if((pNumEntry->GetType() & NUMBERFORMAT_DEFINED)>0)
            {
                return true;
            }
        }
    }
    return false;
}


/*************************************************************************
#*  Member:     GetFormat4Entry                             Datum:30.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxNumberFormatShell
#*
#*  Funktion:   Liefert den Format- String fuer einen gegebenen
#*              Eintrag zurueck.
#*
#*  Input:      Nummer des Eintrags
#*
#*  Output:     Format- String
#*
#************************************************************************/

String SvxNumberFormatShell::GetFormat4Entry(short nEntry)
{
    const SvNumberformat *pNumEntry;

    if(nEntry < 0)
        return String();

    if( !aCurrencyFormatList.empty() )
    {
        if( aCurrencyFormatList.size() > static_cast<size_t>(nEntry) )
            return *aCurrencyFormatList[nEntry];
    }
    else
    {
        sal_uInt32  nMyNfEntry=aCurEntryList[nEntry];
        pNumEntry = pFormatter->GetEntry(nMyNfEntry);

        if(pNumEntry!=NULL)
            return pNumEntry->GetFormatstring();
    }
    return String();
}

/*************************************************************************
#*  Member:     GetListPos4Entry                            Datum:31.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxNumberFormatShell
#*
#*  Funktion:   Liefert die Listen- Nummer fuer einen gegebenen
#*              Formatindex zurueck.
#*
#*  Input:      Nummer des Eintrags
#*
#*  Output:     Kategorie- Nummer
#*
#************************************************************************/

short SvxNumberFormatShell::GetListPos4Entry(sal_uInt32 nIdx)
{
    short nSelP=SELPOS_NONE;

    // Check list size against return type limit.
    if( aCurEntryList.size() <= static_cast<size_t>(::std::numeric_limits< short >::max()) )
    {
        for(size_t i=0; i < aCurEntryList.size(); ++i)
        {
            if(aCurEntryList[i]==nIdx)
            {
                nSelP=i;
                break;
            }
        }
    }
    else
    {
        OSL_FAIL("svx::SvxNumberFormatShell::GetListPos4Entry(), list got too large!" );
    }
    return nSelP;
}

short SvxNumberFormatShell::GetListPos4Entry( const String& rFmtString )
{
    sal_uInt32 nAt=0;
    short nSelP=SELPOS_NONE;
    if(FindEntry(rFmtString, &nAt))
    {
        if(NUMBERFORMAT_ENTRY_NOT_FOUND!=nAt && NUMBERFORMAT_ENTRY_NEW_CURRENCY!=nAt)
        {
            nSelP=GetListPos4Entry(nAt);
        }
        else
        {
            for( size_t i=0; i<aCurrencyFormatList.size(); i++ )
            {
                if (rFmtString==*aCurrencyFormatList[i])
                {
                    nSelP = static_cast<short>(i);
                    break;
                }
            }
        }
    }
    return nSelP;
}

String SvxNumberFormatShell::GetStandardName() const
{
    return pFormatter->GetStandardName( eCurLanguage);
}

void SvxNumberFormatShell::GetCurrencySymbols(std::vector<rtl::OUString>& rList, sal_uInt16* pPos)
{
    const NfCurrencyEntry* pTmpCurrencyEntry=SvNumberFormatter::MatchSystemCurrency();

    bool bFlag=(pTmpCurrencyEntry==NULL);

    GetCurrencySymbols(rList, bFlag);

    if(pPos!=NULL)
    {
        const NfCurrencyTable& rCurrencyTable=SvNumberFormatter::GetTheCurrencyTable();
        sal_uInt16 nTableCount=rCurrencyTable.size();

        *pPos=0;
        size_t nCount=aCurCurrencyList.size();

        if(bFlag)
        {
            *pPos=1;
            nCurCurrencyEntryPos=1;
        }
        else
        {
            for(size_t i=1;i<nCount;i++)
            {
                const sal_uInt16 j = aCurCurrencyList[i];
                if (j != (sal_uInt16)-1 && j < nTableCount &&
                        pTmpCurrencyEntry == &rCurrencyTable[j])
                {
                    *pPos=static_cast<sal_uInt16>(i);
                    nCurCurrencyEntryPos=static_cast<sal_uInt16>(i);
                    break;
                }
            }
        }
    }

}

void SvxNumberFormatShell::GetCurrencySymbols(std::vector<rtl::OUString>& rList, bool bFlag)
{
    aCurCurrencyList.clear();

    const NfCurrencyTable& rCurrencyTable=SvNumberFormatter::GetTheCurrencyTable();
    sal_uInt16 nCount=rCurrencyTable.size();

    SvtLanguageTable* pLanguageTable=new SvtLanguageTable;

    sal_uInt16 nStart=1;

    XubString aString( ApplyLreOrRleEmbedding( rCurrencyTable[0].GetSymbol()));
    aString += sal_Unicode(' ');
    aString += ApplyLreOrRleEmbedding( pLanguageTable->GetString( rCurrencyTable[0].GetLanguage()));

    rList.push_back(aString);
    sal_uInt16 nAuto=(sal_uInt16)-1;
    aCurCurrencyList.push_back(nAuto);

    if(bFlag)
    {
        rList.push_back(aString);
        aCurCurrencyList.push_back(0);
        ++nStart;
    }

    CollatorWrapper aCollator( ::comphelper::getProcessServiceFactory());
    aCollator.loadDefaultCollator( Application::GetSettings().GetLocale(), 0);

    const String aTwoSpace( RTL_CONSTASCII_USTRINGPARAM( "  "));

    for(sal_uInt16 i = 1; i < nCount; ++i)
    {
        XubString aStr( ApplyLreOrRleEmbedding( rCurrencyTable[i].GetBankSymbol()));
        aStr += aTwoSpace;
        aStr += ApplyLreOrRleEmbedding( rCurrencyTable[i].GetSymbol());
        aStr += aTwoSpace;
        aStr += ApplyLreOrRleEmbedding( pLanguageTable->GetString( rCurrencyTable[i].GetLanguage()));

        sal_uInt16 j = nStart;
        for(; j < rList.size(); ++j)
            if (aCollator.compareString(aStr, rList[j]) < 0)
                break;  // insert before first greater than

        rList.insert(rList.begin() + j, aStr);
        aCurCurrencyList.insert(aCurCurrencyList.begin() + j, i);
    }

    // Append ISO codes to symbol list.
    // XXX If this is to be changed, various other places would had to be
    // adapted that assume this order!
    sal_uInt16 nCont = rList.size();

    for(sal_uInt16 i = 1; i < nCount; ++i)
    {
        bool bInsert = true;
        rtl::OUString aStr(ApplyLreOrRleEmbedding(rCurrencyTable[i].GetBankSymbol()));

        sal_uInt16 j = nCont;
        for(; j < rList.size() && bInsert; ++j)
        {
            if(rList[j] == aStr)
                bInsert = false;
            else if (aCollator.compareString(aStr, rList[j]) < 0)
                break;  // insert before first greater than
        }
        if(bInsert)
        {
            rList.insert(rList.begin() + j, aStr);
            aCurCurrencyList.insert(aCurCurrencyList.begin()+j, i);
        }
    }

    delete pLanguageTable;
}

void SvxNumberFormatShell::SetCurrencySymbol(sal_uInt16 nPos)
{
    const NfCurrencyTable& rCurrencyTable=SvNumberFormatter::GetTheCurrencyTable();
    sal_uInt16 nCount=rCurrencyTable.size();

    bBankingSymbol=(nPos>=nCount);

    if(nPos<aCurCurrencyList.size())
    {
        sal_uInt16 nCurrencyPos=aCurCurrencyList[nPos];
        if(nCurrencyPos!=(sal_uInt16)-1)
        {
            pCurCurrencyEntry=(NfCurrencyEntry*)&rCurrencyTable[nCurrencyPos];
            nCurCurrencyEntryPos=nPos;
        }
        else
        {
            pCurCurrencyEntry=NULL;
            nCurCurrencyEntryPos=0;
            nCurFormatKey=pFormatter->GetFormatIndex(
                         NF_CURRENCY_1000DEC2_RED, eCurLanguage);
        }
    }
}

sal_uInt32 SvxNumberFormatShell::GetCurrencySymbol()
{
    return nCurCurrencyEntryPos;
}

void SvxNumberFormatShell::SetCurCurrencyEntry(NfCurrencyEntry* pCEntry)
{
    pCurCurrencyEntry=pCEntry;
}

bool SvxNumberFormatShell::IsTmpCurrencyFormat( const String& rFmtString )
{
    sal_uInt32 nFound;
    FindEntry(rFmtString, &nFound);

    if(nFound==NUMBERFORMAT_ENTRY_NEW_CURRENCY)
    {
        return true;
    }
    return false;
}

sal_uInt16 SvxNumberFormatShell::FindCurrencyFormat( const String& rFmtString )
{
    const NfCurrencyTable& rCurrencyTable=SvNumberFormatter::GetTheCurrencyTable();
    sal_uInt16 nCount=rCurrencyTable.size();

    bool bTestBanking=false;

    sal_uInt16 nPos=FindCurrencyTableEntry(rFmtString, bTestBanking);

    if(nPos!=(sal_uInt16)-1)
    {
        sal_uInt16 nStart=0;
        if(bTestBanking && aCurCurrencyList.size()>nPos)
        {
            nStart=nCount;
        }
        for(sal_uInt16 j=nStart;j<aCurCurrencyList.size();j++)
        {
            if(aCurCurrencyList[j]==nPos) return j;
        }
    }
    return (sal_uInt16) -1;
}

sal_uInt16 SvxNumberFormatShell::FindCurrencyTableEntry( const String& rFmtString, bool &bTestBanking )
{
    sal_uInt16 nPos=(sal_uInt16) -1;

    const NfCurrencyTable& rCurrencyTable=SvNumberFormatter::GetTheCurrencyTable();
    sal_uInt16 nCount=rCurrencyTable.size();

    const SvNumberformat* pFormat;
    OUString aSymbol, aExtension;
    sal_uInt32 nFound = pFormatter->TestNewString( rFmtString, eCurLanguage );
    if ( nFound != NUMBERFORMAT_ENTRY_NOT_FOUND &&
            ((pFormat = pFormatter->GetEntry( nFound )) != 0) &&
            pFormat->GetNewCurrencySymbol( aSymbol, aExtension ) )
    {   // eventually match with format locale
        const NfCurrencyEntry* pTmpCurrencyEntry =
            SvNumberFormatter::GetCurrencyEntry( bTestBanking, aSymbol, aExtension,
            pFormat->GetLanguage() );
        if ( pTmpCurrencyEntry )
        {
            for(sal_uInt16 i=0;i<nCount;i++)
            {
                if(pTmpCurrencyEntry==&rCurrencyTable[i])
                {
                    nPos=i;
                    break;
                }
            }
        }
    }
    else
    {   // search symbol string only
        for(sal_uInt16 i=0;i<nCount;i++)
        {
            const NfCurrencyEntry* pTmpCurrencyEntry=&rCurrencyTable[i];
            OUString _aSymbol = pTmpCurrencyEntry->BuildSymbolString(false);
            OUString aBankSymbol = pTmpCurrencyEntry->BuildSymbolString(true);

            if(rFmtString.Search(_aSymbol)!=STRING_NOTFOUND)
            {
                bTestBanking=false;
                nPos=i;
                break;
            }
            else if(rFmtString.Search(aBankSymbol)!=STRING_NOTFOUND)
            {
                bTestBanking=true;
                nPos=i;
                break;
            }
        }
    }

    return nPos;
}

sal_uInt16 SvxNumberFormatShell::FindCurrencyFormat(const NfCurrencyEntry* pTmpCurrencyEntry,bool bTmpBanking)
{
    const NfCurrencyTable& rCurrencyTable=SvNumberFormatter::GetTheCurrencyTable();
    sal_uInt16 nCount=rCurrencyTable.size();

    sal_uInt16 nPos=0;
    for(sal_uInt16 i=0;i<nCount;i++)
    {
        if(pTmpCurrencyEntry==&rCurrencyTable[i])
        {
            nPos=i;
            break;
        }
    }

    sal_uInt16 nStart=0;
    if(bTmpBanking && aCurCurrencyList.size()>nPos)
    {
        nStart=nCount;
    }
    for(sal_uInt16 j=nStart;j<aCurCurrencyList.size();j++)
    {
        if(aCurCurrencyList[j]==nPos) return j;
    }
    return (sal_uInt16) -1;
}

bool SvxNumberFormatShell::IsInTable(sal_uInt16 const nPos,
        bool const bTmpBanking, ::rtl::OUString const& rFmtString)
{
    bool bFlag=false;

    if(nPos!=(sal_uInt16)-1)
    {
        const NfCurrencyTable& rCurrencyTable=SvNumberFormatter::GetTheCurrencyTable();
        sal_uInt16 nCount=rCurrencyTable.size();

        if(nPos<nCount)
        {
            NfWSStringsDtor aWSStringsDtor;

            const NfCurrencyEntry* pTmpCurrencyEntry=&rCurrencyTable[nPos];

            if ( pTmpCurrencyEntry!=NULL)
            {
                pFormatter->GetCurrencyFormatStrings( aWSStringsDtor,
                                *pTmpCurrencyEntry, bTmpBanking );

                for(sal_uInt16 i=0;i<aWSStringsDtor.size();i++)
                {
                    if (aWSStringsDtor[i] == rFmtString)
                    {
                        bFlag=true;
                        break;
                    }
                }
            }
        }
    }

    return bFlag;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
