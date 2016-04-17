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

#include <tools/color.hxx>

#include <tools/debug.hxx>
#include <i18nlangtag/mslangid.hxx>

#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <svl/currencytable.hxx>

#include <svx/numfmtsh.hxx>
#include <svx/tbcontrl.hxx>

#include <limits>

const double SvxNumberFormatShell::DEFAULT_NUMVALUE = 1234.56789;


SvxNumberFormatShell* SvxNumberFormatShell::Create( SvNumberFormatter* pNumFormatter,
                                              sal_uInt32           nFormatKey,
                                              SvxNumberValueType   eNumValType,
                                              const OUString&      rNumStr )
{
    return new SvxNumberFormatShell(pNumFormatter,nFormatKey,
                                    eNumValType,rNumStr );
}

SvxNumberFormatShell* SvxNumberFormatShell::Create( SvNumberFormatter* pNumFormatter,
                                              sal_uInt32         nFormatKey,
                                              SvxNumberValueType eNumValType,
                                              double             nNumVal,
                                              const OUString*    pNumStr )
{
    return new SvxNumberFormatShell(pNumFormatter,nFormatKey,
                                    eNumValType,nNumVal,pNumStr );
}


SvxNumberFormatShell::SvxNumberFormatShell( SvNumberFormatter*  pNumFormatter,
                                            sal_uInt32          nFormatKey,
                                            SvxNumberValueType  eNumValType,
                                            const OUString&     rNumStr )
    : pFormatter ( pNumFormatter )
    , pCurFmtTable ( nullptr )
    , eValType ( eNumValType )
    , bUndoAddList ( true )
    , nCurFormatKey ( nFormatKey )
    , nCurCategory (css::util::NumberFormat::ALL)
    , eCurLanguage (LANGUAGE_NONE)
    , pCurCurrencyEntry(nullptr)
    , bBankingSymbol  (false)
    , nCurCurrencyEntryPos((sal_uInt16) SELPOS_NONE)
    , bUseStarFormat  (false)
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
            aValStr.clear();
    }
}


SvxNumberFormatShell::SvxNumberFormatShell( SvNumberFormatter*  pNumFormatter,
                                            sal_uInt32          nFormatKey,
                                            SvxNumberValueType  eNumValType,
                                            double              nNumVal,
                                            const OUString*     pNumStr )
    : pFormatter ( pNumFormatter )
    , pCurFmtTable ( nullptr )
    , eValType ( eNumValType )
    , bUndoAddList ( true )
    , nCurFormatKey ( nFormatKey )
    , nCurCategory (css::util::NumberFormat::ALL)
    , eCurLanguage (LANGUAGE_NONE)
    , pCurCurrencyEntry(nullptr)
    , bBankingSymbol  (false)
    , nCurCurrencyEntryPos((sal_uInt16) SELPOS_NONE)
    , bUseStarFormat  (false)
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
        // Added formats are invalid => remove them

        for ( std::vector<sal_uInt32>::const_iterator it(aAddList.begin()); it != aAddList.end(); ++it )
            pFormatter->DeleteEntry( *it );
    }
}


size_t SvxNumberFormatShell::GetUpdateDataCount() const
{
    return aDelList.size();
}


void SvxNumberFormatShell::GetUpdateData( sal_uInt32* pDelArray, const sal_uInt32 nSize )
{
    const size_t nListSize = aDelList.size();

    DBG_ASSERT( pDelArray && ( nSize == nListSize ), "Array not initialised!" );

    if ( pDelArray && ( nSize == nListSize ) )
        for (std::vector<sal_uInt32>::const_iterator it(aDelList.begin()); it != aDelList.end(); ++it )
            *pDelArray++ = *it;
}


void SvxNumberFormatShell::CategoryChanged( sal_uInt16 nCatLbPos,
                                            short& rFmtSelPos,
                                            std::vector<OUString>& rFmtEntries )
{
    short nOldCategory = nCurCategory;
    PosToCategory_Impl( nCatLbPos, nCurCategory );
    pCurFmtTable = &( pFormatter->GetEntryTable( nCurCategory,
                                                 nCurFormatKey,
                                                 eCurLanguage ) );
    // reinitialize currency if category newly entered
    if ( nCurCategory == css::util::NumberFormat::CURRENCY && nOldCategory != nCurCategory )
        pCurCurrencyEntry = nullptr;
    rFmtSelPos = FillEntryList_Impl( rFmtEntries );
}


void SvxNumberFormatShell::LanguageChanged( LanguageType eLangType,
                                            short& rFmtSelPos,
                                            std::vector<OUString>& rFmtEntries )
{
    eCurLanguage = eLangType;
    pCurFmtTable = &(pFormatter->ChangeCL( nCurCategory,
                                           nCurFormatKey,
                                           eCurLanguage ) );
    rFmtSelPos = FillEntryList_Impl( rFmtEntries );
}


void SvxNumberFormatShell::FormatChanged( sal_uInt16  nFmtLbPos,
                                          OUString&   rPreviewStr,
                                          Color*&     rpFontColor )
{
    if( static_cast<size_t>(nFmtLbPos) < aCurEntryList.size() )
    {
        nCurFormatKey = aCurEntryList[nFmtLbPos];

        if( nCurFormatKey != NUMBERFORMAT_ENTRY_NOT_FOUND )
        {
            GetPreviewString_Impl( rPreviewStr, rpFontColor );
        }
        else if( nCurCategory == css::util::NumberFormat::CURRENCY )
        {
            if( static_cast<size_t>(nFmtLbPos) < aCurrencyFormatList.size() )
            {
                MakePrevStringFromVal(aCurrencyFormatList[nFmtLbPos],
                                    rPreviewStr,rpFontColor,nValNum);
            }
        }
    }
}


bool SvxNumberFormatShell::AddFormat( OUString& rFormat, sal_Int32& rErrPos,
                                      sal_uInt16& rCatLbSelPos, short& rFmtSelPos,
                                      std::vector<OUString>& rFmtEntries )
{
    bool        bInserted   = false;
    sal_uInt32  nAddKey     = pFormatter->GetEntryKey( rFormat, eCurLanguage );

    if ( nAddKey != NUMBERFORMAT_ENTRY_NOT_FOUND ) // exists already?
    {
        ::std::vector<sal_uInt32>::iterator nAt = GetRemoved_Impl( nAddKey );
        if ( nAt != aDelList.end() )
        {
            aDelList.erase( nAt );
            bInserted = true;
        }
        else
        {
            OSL_FAIL( "duplicate format!" );
        }
    }
    else // new format
    {
        sal_Int32 nPos;
        bInserted = pFormatter->PutEntry( rFormat, nPos,
                                          nCurCategory, nAddKey,
                                          eCurLanguage );
        rErrPos = (nPos >= 0) ? nPos : -1;

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

    if ( bInserted )
    {
        nCurFormatKey = nAddKey;
        DBG_ASSERT( !IsAdded_Impl( nCurFormatKey ), "duplicate format!" );
        aAddList.push_back( nCurFormatKey );

        // get current table
        pCurFmtTable = &(pFormatter->GetEntryTable( nCurCategory,
                                                    nCurFormatKey,
                                                    eCurLanguage ));
        nCurCategory=pFormatter->GetType(nAddKey);
        CategoryToPos_Impl( nCurCategory, rCatLbSelPos );
        rFmtSelPos = FillEntryList_Impl( rFmtEntries );
    }
    else if ( rErrPos != 0 ) // syntax error
    {
        ;
    }
    else // insert twice not possible
    {
        OSL_FAIL( "duplicate format!" );
    }

    return bInserted;
}


bool SvxNumberFormatShell::RemoveFormat( const OUString& rFormat,
                                         sal_uInt16& rCatLbSelPos,
                                         short& rFmtSelPos,
                                         std::vector<OUString>& rFmtEntries )
{
    sal_uInt32 nDelKey = pFormatter->GetEntryKey( rFormat, eCurLanguage );

    DBG_ASSERT( nDelKey != NUMBERFORMAT_ENTRY_NOT_FOUND, "entry not found!" );
    DBG_ASSERT( !IsRemoved_Impl( nDelKey ), "entry already removed!" );

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


void SvxNumberFormatShell::MakeFormat( OUString& rFormat,
                                       bool bThousand, bool bNegRed,
                                       sal_uInt16 nPrecision, sal_uInt16 nLeadingZeroes,
                                       sal_uInt16 nCurrencyPos)
{
    if( aCurrencyFormatList.size() > static_cast<size_t>(nCurrencyPos) )
    {
        sal_Int32 rErrPos=0;
        std::vector<OUString> aFmtEList;

        sal_uInt32 nFound = pFormatter->TestNewString( aCurrencyFormatList[nCurrencyPos], eCurLanguage );

        if ( nFound == NUMBERFORMAT_ENTRY_NOT_FOUND )
        {
            sal_uInt16 rCatLbSelPos =0;
            short      rFmtSelPos = 0;
            AddFormat( aCurrencyFormatList[nCurrencyPos],rErrPos,rCatLbSelPos,
                    rFmtSelPos,aFmtEList);
        }

        if(rErrPos==0)
        {
            rFormat = pFormatter->GenerateFormat(nCurFormatKey,
                                                 eCurLanguage,
                                                 bThousand, bNegRed,
                                                 nPrecision, nLeadingZeroes);
        }
    }
    else
    {
        rFormat = pFormatter->GenerateFormat(nCurFormatKey,
                                             eCurLanguage,
                                             bThousand, bNegRed,
                                             nPrecision, nLeadingZeroes);
    }
}


sal_uInt16 SvxNumberFormatShell::GetFormatIntegerDigits( const OUString&  rFormat ) const
{
    sal_uInt32 nFmtKey = pFormatter->GetEntryKey( rFormat, eCurLanguage );

    return pFormatter->GetFormatIntegerDigits(nFmtKey);
}


void SvxNumberFormatShell::GetOptions( const OUString&  rFormat,
                                       bool&            rThousand,
                                       bool&            rNegRed,
                                       sal_uInt16&      rPrecision,
                                       sal_uInt16&      rLeadingZeroes,
                                       sal_uInt16&      rCatLbPos )
{

    sal_uInt32 nFmtKey = pFormatter->GetEntryKey( rFormat, eCurLanguage );

    if(nFmtKey != NUMBERFORMAT_ENTRY_NOT_FOUND)
    {
        pFormatter->GetFormatSpecialInfo( nFmtKey,
                                          rThousand, rNegRed,
                                          rPrecision, rLeadingZeroes );

        CategoryToPos_Impl( pFormatter->GetType( nFmtKey ), rCatLbPos );
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


void SvxNumberFormatShell::MakePreviewString( const OUString& rFormatStr,
                                              OUString&       rPreviewStr,
                                              Color*&         rpFontColor )
{
    rpFontColor = nullptr;

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
                            ( !aValStr.isEmpty() && ( pFormatter->GetType(nExistingFormat) & css::util::NumberFormat::TEXT ) ) );
        if ( bUseText )
        {
            pFormatter->GetOutputString( aValStr, nExistingFormat,
                                         rPreviewStr, &rpFontColor );
        }
        else
        {
            pFormatter->GetOutputString( nValNum, nExistingFormat,
                                         rPreviewStr, &rpFontColor, bUseStarFormat );
        }
    }
}


bool SvxNumberFormatShell::IsUserDefined( const OUString& rFmtString )
{
    sal_uInt32 nFound = pFormatter->GetEntryKey( rFmtString, eCurLanguage );

    bool bFlag=false;
    if ( nFound != NUMBERFORMAT_ENTRY_NOT_FOUND )
    {
        bFlag=pFormatter->IsUserDefined( rFmtString, eCurLanguage );

        if(bFlag)
        {
            const SvNumberformat* pNumEntry = pFormatter->GetEntry(nFound);

            if(pNumEntry!=nullptr && pNumEntry->HasNewCurrency())
            {
                bool bTestBanking;
                sal_uInt16 nPos=FindCurrencyTableEntry(rFmtString,bTestBanking);
                bFlag=!IsInTable(nPos,bTestBanking,rFmtString);
            }
        }
    }
    return bFlag;
}


bool SvxNumberFormatShell::FindEntry( const OUString& rFmtString, sal_uInt32* pAt /* = NULL */ )
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


void SvxNumberFormatShell::GetInitSettings( sal_uInt16& nCatLbPos,
                                            LanguageType& rLangType,
                                            sal_uInt16& nFmtLbSelPos,
                                            std::vector<OUString>& rFmtEntries,
                                            OUString& rPrevString,
                                            Color*&   rpPrevColor )
{

    // precondition: number formater found
    DBG_ASSERT( pFormatter != nullptr, "Zahlenformatierer nicht gefunden!" );

    short                   nSelPos     = SELPOS_NONE;

    // Sonderbehandlung fuer undefiniertes Zahlenformat:
    if ( (eValType == SVX_VALUE_TYPE_UNDEFINED) && (nCurFormatKey == 0) )
        PosToCategory_Impl( CAT_ALL, nCurCategory );        // Kategorie = Alle
    else
        nCurCategory = css::util::NumberFormat::UNDEFINED;      // Kategorie = Undefiniert

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


short SvxNumberFormatShell::FillEntryList_Impl( std::vector<OUString>& rList )
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

    if(nCurCategory==css::util::NumberFormat::ALL)
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

void SvxNumberFormatShell::FillEListWithStd_Impl( std::vector<OUString>& rList,
                                                  sal_uInt16 nPrivCat,short &nSelPos )
{
    /* Erstellen einer aktuellen Liste von Format-Eintraegen.
     * Rueckgabewert ist die Listenposition des aktuellen Formates.
     * Ist die Liste leer oder gibt es kein aktuelles Format,
     * so wird SELPOS_NONE geliefert.
     */
    DBG_ASSERT( pCurFmtTable != nullptr, "Unbekanntes Zahlenformat!" );

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
        }
    }
}

short SvxNumberFormatShell::FillEListWithFormats_Impl( std::vector<OUString>& rList,
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

    sal_uInt32          nNFEntry;
    OUString            aStrComment;
    OUString            aNewFormNInfo;

    short           nMyCat      = SELPOS_NONE;

    long nIndex;

    for(nIndex=eOffsetStart;nIndex<=eOffsetEnd;nIndex++)
    {
        nNFEntry=pFormatter->GetFormatIndex((NfIndexTableOffset)nIndex,eCurLanguage);

        const SvNumberformat* pNumEntry   = pFormatter->GetEntry(nNFEntry);

        if(pNumEntry==nullptr) continue;

        nMyCat=pNumEntry->GetType() & ~css::util::NumberFormat::DEFINED;
        aStrComment=pNumEntry->GetComment();
        CategoryToPos_Impl(nMyCat,nMyType);
        aNewFormNInfo=  pNumEntry->GetFormatstring();

        if ( nNFEntry == nCurFormatKey )
        {
            nSelPos = ( !IsRemoved_Impl( nNFEntry ) ) ? aCurEntryList.size() : SELPOS_NONE;
        }

        rList.push_back( aNewFormNInfo );
        aCurEntryList.push_back( nNFEntry );
    }

    return nSelPos;
}

short SvxNumberFormatShell::FillEListWithDateTime_Impl( std::vector<OUString>& rList,
                                                        short nSelPos)
{
    sal_uInt16  nMyType;

    sal_uInt32          nNFEntry;
    OUString            aStrComment;
    OUString            aNewFormNInfo;

    short           nMyCat      = SELPOS_NONE;

    for (long nIndex = NF_DATETIME_START; nIndex <= NF_DATETIME_END; ++nIndex)
    {
        nNFEntry=pFormatter->GetFormatIndex((NfIndexTableOffset)nIndex,eCurLanguage);

        const SvNumberformat* pNumEntry   = pFormatter->GetEntry(nNFEntry);
        if(pNumEntry!=nullptr)
        {
            nMyCat=pNumEntry->GetType() & ~css::util::NumberFormat::DEFINED;
            aStrComment=pNumEntry->GetComment();
            CategoryToPos_Impl(nMyCat,nMyType);
            aNewFormNInfo=  pNumEntry->GetFormatstring();

            if ( nNFEntry == nCurFormatKey )
            {
                nSelPos = ( !IsRemoved_Impl( nNFEntry ) ) ? aCurEntryList.size() : SELPOS_NONE;
            }

            rList.push_back( aNewFormNInfo );
            aCurEntryList.push_back( nNFEntry );
        }
    }

    return nSelPos;
}

short SvxNumberFormatShell::FillEListWithCurrency_Impl( std::vector<OUString>& rList,
                                                        short nSelPos)
{
    /* Erstellen einer aktuellen Liste von Format-Eintraegen.
     * Rueckgabewert ist die Listenposition des aktuellen Formates.
     * Ist die Liste leer oder gibt es kein aktuelles Format,
     * so wird SELPOS_NONE geliefert.
     */
    DBG_ASSERT( pCurFmtTable != nullptr, "Unbekanntes Zahlenformat!" );

    const NfCurrencyEntry* pTmpCurrencyEntry;
    bool             bTmpBanking;
    OUString        rSymbol;

    bool bFlag=pFormatter->GetNewCurrencySymbolString(nCurFormatKey,rSymbol,
                                                      &pTmpCurrencyEntry,&bTmpBanking);

    if( (!bFlag && pCurCurrencyEntry==nullptr) ||
        (bFlag && pTmpCurrencyEntry==nullptr && rSymbol.isEmpty()) ||
        (nCurCategory==css::util::NumberFormat::ALL))
    {
        if ( nCurCategory == css::util::NumberFormat::ALL )
            FillEListWithUserCurrencys(rList,nSelPos);
        nSelPos=FillEListWithSysCurrencys(rList,nSelPos);
    }
    else
    {
        nSelPos=FillEListWithUserCurrencys(rList,nSelPos);
    }

    return nSelPos;
}


short SvxNumberFormatShell::FillEListWithSysCurrencys( std::vector<OUString>& rList,
                                                       short nSelPos)
{
    /* Erstellen einer aktuellen Liste von Format-Eintraegen.
     * Rueckgabewert ist die Listenposition des aktuellen Formates.
     * Ist die Liste leer oder gibt es kein aktuelles Format,
     * so wird SELPOS_NONE geliefert.
     */
    sal_uInt16  nMyType;

    DBG_ASSERT( pCurFmtTable != nullptr, "Unbekanntes Zahlenformat!" );

    sal_uInt32          nNFEntry;
    OUString            aStrComment;
    OUString            aNewFormNInfo;

    nCurCurrencyEntryPos=0;

    short           nMyCat      = SELPOS_NONE;

    NfIndexTableOffset eOffsetStart=NF_CURRENCY_START;
    NfIndexTableOffset eOffsetEnd=NF_CURRENCY_END;
    long nIndex;

    for(nIndex=eOffsetStart;nIndex<=eOffsetEnd;nIndex++)
    {
        nNFEntry=pFormatter->GetFormatIndex((NfIndexTableOffset)nIndex,eCurLanguage);

        const SvNumberformat* pNumEntry   = pFormatter->GetEntry(nNFEntry);

        if(pNumEntry==nullptr) continue;

        nMyCat=pNumEntry->GetType() & ~css::util::NumberFormat::DEFINED;
        aStrComment=pNumEntry->GetComment();
        CategoryToPos_Impl(nMyCat,nMyType);
        aNewFormNInfo=  pNumEntry->GetFormatstring();

        if ( nNFEntry == nCurFormatKey )
        {
            nSelPos = ( !IsRemoved_Impl( nNFEntry ) ) ? aCurEntryList.size() : SELPOS_NONE;
        }

        rList.push_back( aNewFormNInfo );
        aCurEntryList.push_back( nNFEntry );
    }

    if(nCurCategory!=css::util::NumberFormat::ALL)
    {
        for( SvNumberFormatTable::const_iterator it = pCurFmtTable->begin(), aEnd = pCurFmtTable->end(); it != aEnd; ++it )
        {
            sal_uInt32 nKey = it->first;
            const SvNumberformat* pNumEntry   = it->second;

            if ( !IsRemoved_Impl( nKey ))
            {
                bool bUserNewCurrency=false;
                if(pNumEntry->HasNewCurrency())
                {
                    const NfCurrencyEntry* pTmpCurrencyEntry;
                    bool            bTmpBanking;
                    OUString       rSymbol;

                    pFormatter->GetNewCurrencySymbolString(nKey,rSymbol,
                                                           &pTmpCurrencyEntry,
                                                           &bTmpBanking);

                    bUserNewCurrency=(pTmpCurrencyEntry!=nullptr);
                }

                if(!bUserNewCurrency &&(pNumEntry->GetType() & css::util::NumberFormat::DEFINED))
                {
                    nMyCat=pNumEntry->GetType() & ~css::util::NumberFormat::DEFINED;
                    aStrComment=pNumEntry->GetComment();
                    CategoryToPos_Impl(nMyCat,nMyType);
                    aNewFormNInfo=  pNumEntry->GetFormatstring();

                    if ( nKey == nCurFormatKey ) nSelPos =aCurEntryList.size();
                    rList.push_back( aNewFormNInfo );
                    aCurEntryList.push_back( nKey );
                }
            }
        }
    }
    return nSelPos;
}

short SvxNumberFormatShell::FillEListWithUserCurrencys( std::vector<OUString>& rList,
                                                        short nSelPos)
{
    /* Erstellen einer aktuellen Liste von Format-Eintraegen.
     * Rueckgabewert ist die Listenposition des aktuellen Formates.
     * Ist die Liste leer oder gibt es kein aktuelles Format,
     * so wird SELPOS_NONE geliefert.
     */
    sal_uInt16 nMyType;

    DBG_ASSERT( pCurFmtTable != nullptr, "Unbekanntes Zahlenformat!" );

    OUString        aStrComment;
    OUString        aNewFormNInfo;
    short           nMyCat = SELPOS_NONE;

    const NfCurrencyEntry* pTmpCurrencyEntry;
    bool            bTmpBanking, bAdaptSelPos;
    OUString        rSymbol;
    OUString       rBankSymbol;

    std::vector<OUString>   aList;
    std::vector<sal_uInt32> aKeyList;

    pFormatter->GetNewCurrencySymbolString(nCurFormatKey,rSymbol,
                                           &pTmpCurrencyEntry,
                                           &bTmpBanking);

    OUString rShortSymbol;

    if(pCurCurrencyEntry==nullptr)
    {
        // #110398# If no currency format was previously selected (we're not
        // about to add another currency), try to select the initial currency
        // format (nCurFormatKey) that was set in FormatChanged() after
        // matching the format string entered in the dialog.
        bAdaptSelPos = true;
        pCurCurrencyEntry = const_cast<NfCurrencyEntry*>(pTmpCurrencyEntry);
        bBankingSymbol = bTmpBanking;
        nCurCurrencyEntryPos = FindCurrencyFormat(pTmpCurrencyEntry,bTmpBanking);
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

    if(pTmpCurrencyEntry!=nullptr)
    {
        rSymbol = pTmpCurrencyEntry->BuildSymbolString(false);
        rBankSymbol = pTmpCurrencyEntry->BuildSymbolString(true);
        rShortSymbol = pTmpCurrencyEntry->BuildSymbolString(bTmpBanking,true);
    }

    for ( SvNumberFormatTable::const_iterator it = pCurFmtTable->begin(), aEnd = pCurFmtTable->end(); it != aEnd; ++it )
    {
        sal_uInt32 nKey = it->first;
        const SvNumberformat* pNumEntry = it->second;

        if ( !IsRemoved_Impl( nKey ) )
        {
            if( pNumEntry->GetType() & css::util::NumberFormat::DEFINED ||
                pNumEntry->IsAdditionalBuiltin() )
            {
                nMyCat=pNumEntry->GetType() & ~css::util::NumberFormat::DEFINED;
                aStrComment = pNumEntry->GetComment();
                CategoryToPos_Impl(nMyCat,nMyType);
                aNewFormNInfo =  pNumEntry->GetFormatstring();

                bool bInsFlag = false;
                if ( pNumEntry->HasNewCurrency() )
                {
                    bInsFlag = true;    // merge locale formats into currency selection
                }
                else if( (!bTmpBanking && aNewFormNInfo.indexOf(rSymbol) >= 0) ||
                         (bTmpBanking && aNewFormNInfo.indexOf(rBankSymbol) >= 0) )
                {
                    bInsFlag = true;
                }
                else if(aNewFormNInfo.indexOf(rShortSymbol) >= 0)
                {
                    OUString rTstSymbol;
                    const NfCurrencyEntry* pTstCurrencyEntry;
                    bool bTstBanking;

                    pFormatter->GetNewCurrencySymbolString(nKey,rTstSymbol,
                                                           &pTstCurrencyEntry,
                                                           &bTstBanking);

                    if(pTmpCurrencyEntry == pTstCurrencyEntry &&
                       bTstBanking == bTmpBanking)
                    {
                        bInsFlag = true;
                    }

                }

                if(bInsFlag)
                {
                    aList.push_back( aNewFormNInfo );
                    aKeyList.push_back( nKey );
                }
            }
        }
    }

    NfWSStringsDtor aWSStringsDtor;
    sal_uInt16 nDefault;
    if ( pTmpCurrencyEntry && nCurCategory != css::util::NumberFormat::ALL )
    {
        nDefault = pFormatter->GetCurrencyFormatStrings(
            aWSStringsDtor, *pTmpCurrencyEntry, bTmpBanking );
        if ( !bTmpBanking )
            pFormatter->GetCurrencyFormatStrings(
                aWSStringsDtor, *pTmpCurrencyEntry, true );
    }
    else
        nDefault = 0;
    if ( !bTmpBanking && nCurCategory != css::util::NumberFormat::ALL )
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
        OUString aInsStr(aWSStringsDtor[i]);
        size_t j;
        for( j=0; j < aList.size(); ++j )
        {
            if(aList[j]==aInsStr)
            {
                bFlag = false;
                break;
            }
        }
        if(bFlag)
        {
            rList.push_back( aInsStr );
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
        aCurrencyFormatList.push_back( rList[i] );

        if ( nSelPos == SELPOS_NONE && bAdaptSelPos && aCurEntryList[i] == nCurFormatKey )
            nSelPos = i;
    }

    if ( nSelPos == SELPOS_NONE && nCurCategory != css::util::NumberFormat::ALL )
        nSelPos = nDefault;

    return nSelPos;
}


short SvxNumberFormatShell::FillEListWithUsD_Impl( std::vector<OUString>& rList,
                                                   sal_uInt16 nPrivCat, short nSelPos )
{
    /* Erstellen einer aktuellen Liste von Format-Eintraegen.
     * Rueckgabewert ist die Listenposition des aktuellen Formates.
     * Ist die Liste leer oder gibt es kein aktuelles Format,
     * so wird SELPOS_NONE geliefert.
     */
    sal_uInt16 nMyType;

    DBG_ASSERT( pCurFmtTable != nullptr, "Unbekanntes Zahlenformat!" );

    OUString        aStrComment;
    OUString        aNewFormNInfo;

    short           nMyCat      = SELPOS_NONE;
    bool            bAdditional = (nPrivCat != CAT_USERDEFINED &&
                                    nCurCategory != css::util::NumberFormat::ALL);

    for( SvNumberFormatTable::const_iterator it = pCurFmtTable->begin(), aEnd = pCurFmtTable->end(); it != aEnd; ++it )
    {
        sal_uInt32 nKey = it->first;
        const SvNumberformat* pNumEntry = it->second;

        if ( !IsRemoved_Impl( nKey ) )
        {
            if( (pNumEntry->GetType() & css::util::NumberFormat::DEFINED) ||
                    (bAdditional && pNumEntry->IsAdditionalBuiltin()) )
            {
                nMyCat=pNumEntry->GetType() & ~css::util::NumberFormat::DEFINED;
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
                    if ( nKey == nCurFormatKey ) nSelPos = aCurEntryList.size();
                    rList.push_back( aNewFormNInfo );
                    aCurEntryList.push_back( nKey );
                }
            }
        }
    }
    return nSelPos;
}


void SvxNumberFormatShell::GetPreviewString_Impl( OUString& rString, Color*& rpColor )
{
    rpColor = nullptr;

    //  #50441# if a string was set in addition to the value, use it for text formats
    bool bUseText = ( eValType == SVX_VALUE_TYPE_STRING ||
                        ( !aValStr.isEmpty() && ( pFormatter->GetType(nCurFormatKey) & css::util::NumberFormat::TEXT ) ) );

    if ( bUseText )
    {
        pFormatter->GetOutputString( aValStr, nCurFormatKey, rString, &rpColor );
    }
    else
    {
        pFormatter->GetOutputString( nValNum, nCurFormatKey, rString, &rpColor, bUseStarFormat );
    }
}


::std::vector<sal_uInt32>::iterator SvxNumberFormatShell::GetRemoved_Impl( size_t nKey )
{
    return ::std::find(aDelList.begin(), aDelList.end(), nKey);
}


bool SvxNumberFormatShell::IsRemoved_Impl( size_t nKey )
{
    return GetRemoved_Impl( nKey ) != aDelList.end();
}


::std::vector<sal_uInt32>::iterator SvxNumberFormatShell::GetAdded_Impl( size_t nKey )
{
    return ::std::find(aAddList.begin(), aAddList.end(), nKey);
}


bool SvxNumberFormatShell::IsAdded_Impl( size_t nKey )
{
    return GetAdded_Impl( nKey ) != aAddList.end();
}

// Konvertierungs-Routinen:
void SvxNumberFormatShell::PosToCategory_Impl(sal_uInt16 nPos, short& rCategory) const
{
    // Kategorie css::form-Positionen abbilden (->Resource)
    switch ( nPos )
    {
        case CAT_USERDEFINED:   rCategory = css::util::NumberFormat::DEFINED;       break;
        case CAT_NUMBER:        rCategory = css::util::NumberFormat::NUMBER;        break;
        case CAT_PERCENT:       rCategory = css::util::NumberFormat::PERCENT;       break;
        case CAT_CURRENCY:      rCategory = css::util::NumberFormat::CURRENCY;      break;
        case CAT_DATE:          rCategory = css::util::NumberFormat::DATE;          break;
        case CAT_TIME:          rCategory = css::util::NumberFormat::TIME;          break;
        case CAT_SCIENTIFIC:    rCategory = css::util::NumberFormat::SCIENTIFIC;    break;
        case CAT_FRACTION:      rCategory = css::util::NumberFormat::FRACTION;      break;
        case CAT_BOOLEAN:       rCategory = css::util::NumberFormat::LOGICAL;       break;
        case CAT_TEXT:          rCategory = css::util::NumberFormat::TEXT;          break;
        case CAT_ALL:
        default:    rCategory = css::util::NumberFormat::ALL; break;
    }
}

void SvxNumberFormatShell::CategoryToPos_Impl(short nCategory, sal_uInt16& rPos) const
{
    // Kategorie auf css::form-Positionen abbilden (->Resource)
    switch ( nCategory )
    {
        case css::util::NumberFormat::DEFINED:      rPos = CAT_USERDEFINED; break;
        case css::util::NumberFormat::NUMBER:       rPos = CAT_NUMBER;      break;
        case css::util::NumberFormat::PERCENT:      rPos = CAT_PERCENT;     break;
        case css::util::NumberFormat::CURRENCY:     rPos = CAT_CURRENCY;    break;
        case css::util::NumberFormat::DATETIME:
        case css::util::NumberFormat::DATE:         rPos = CAT_DATE;        break;
        case css::util::NumberFormat::TIME:         rPos = CAT_TIME;        break;
        case css::util::NumberFormat::SCIENTIFIC:   rPos = CAT_SCIENTIFIC;  break;
        case css::util::NumberFormat::FRACTION:     rPos = CAT_FRACTION;    break;
        case css::util::NumberFormat::LOGICAL:      rPos = CAT_BOOLEAN;     break;
        case css::util::NumberFormat::TEXT:         rPos = CAT_TEXT;        break;
        case css::util::NumberFormat::ALL:
        default:                        rPos = CAT_ALL;
    }
}

/*
 * Funktion:   Formatiert die Zahl nValue abhaengig von rFormatStr
 *             und speichert das Ergebnis in rPreviewStr.
 * Input:      FormatString, Farbe, zu formatierende Zahl
 * Output:     Ausgabestring rPreviewStr
 */
void SvxNumberFormatShell::MakePrevStringFromVal(
        const OUString& rFormatStr,
        OUString& rPreviewStr,
        Color*& rpFontColor,
        double  nValue)
{
    rpFontColor = nullptr;
    pFormatter->GetPreviewString( rFormatStr, nValue, rPreviewStr, &rpFontColor, eCurLanguage );
}

/*
 * Funktion:   Liefert den Kommentar fuer einen gegebenen
 *             Eintrag zurueck.
 * Input:      Nummer des Eintrags
 * Output:     Kommentar-String
 */
void SvxNumberFormatShell::SetComment4Entry(short nEntry, const OUString& aEntStr)
{
    SvNumberformat *pNumEntry;
    if(nEntry<0) return;
    sal_uInt32  nMyNfEntry=aCurEntryList[nEntry];
    pNumEntry = const_cast<SvNumberformat*>(pFormatter->GetEntry(nMyNfEntry));
    if(pNumEntry!=nullptr) pNumEntry->SetComment(aEntStr);
}

/*
 * Funktion:   Liefert den Kommentar fuer einen gegebenen
 *             Eintrag zurueck.
 * Input:      Nummer des Eintrags
 * Output:     Kommentar-String
 */
OUString SvxNumberFormatShell::GetComment4Entry(short nEntry)
{
    if(nEntry < 0)
        return OUString();

    if( static_cast<size_t>(nEntry) < aCurEntryList.size())
    {
        sal_uInt32  nMyNfEntry=aCurEntryList[nEntry];
        const SvNumberformat *pNumEntry = pFormatter->GetEntry(nMyNfEntry);
        if(pNumEntry!=nullptr)
            return pNumEntry->GetComment();
    }

    return OUString();
}

/*
 * Funktion:   Liefert die Kategorie- Nummer fuer einen gegebenen
 *             Eintrag zurueck.
 * Input:      Nummer des Eintrags
 * Output:     Kategorie- Nummer
 */
short SvxNumberFormatShell::GetCategory4Entry(short nEntry) const
{
    if(nEntry<0) return 0;
    if( static_cast<size_t>(nEntry) < aCurEntryList.size() )
    {
        sal_uInt32  nMyNfEntry=aCurEntryList[nEntry];

        if(nMyNfEntry!=NUMBERFORMAT_ENTRY_NOT_FOUND)
        {
            const SvNumberformat *pNumEntry = pFormatter->GetEntry(nMyNfEntry);
            sal_uInt16 nMyCat,nMyType;
            if(pNumEntry!=nullptr)
            {
                nMyCat=pNumEntry->GetType() & ~css::util::NumberFormat::DEFINED;
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

/*
 * Funktion:   Liefert die Information, ob ein Eintrag
 *             benutzerspezifisch ist zurueck.
 * Input:      Nummer des Eintrags
 * Output:     Benutzerspezifisch?
 */
bool SvxNumberFormatShell::GetUserDefined4Entry(short nEntry)
{
    if(nEntry<0) return false;
    if( static_cast<size_t>(nEntry) < aCurEntryList.size())
    {
        sal_uInt32  nMyNfEntry=aCurEntryList[nEntry];
        const SvNumberformat *pNumEntry = pFormatter->GetEntry(nMyNfEntry);

        if(pNumEntry!=nullptr)
        {
            if((pNumEntry->GetType() & css::util::NumberFormat::DEFINED)>0)
            {
                return true;
            }
        }
    }
    return false;
}

/*
 * Funktion:   Liefert den Format- String fuer einen gegebenen
 *             Eintrag zurueck.
 * Input:      Nummer des Eintrags
 * Output:     Format- String
 */
OUString SvxNumberFormatShell::GetFormat4Entry(short nEntry)
{
    if(nEntry < 0)
        return OUString();

    if( !aCurrencyFormatList.empty() )
    {
        if( aCurrencyFormatList.size() > static_cast<size_t>(nEntry) )
            return aCurrencyFormatList[nEntry];
    }
    else
    {
        sal_uInt32  nMyNfEntry=aCurEntryList[nEntry];
        const SvNumberformat *pNumEntry = pFormatter->GetEntry(nMyNfEntry);

        if(pNumEntry!=nullptr)
            return pNumEntry->GetFormatstring();
    }
    return OUString();
}

/*
 * Funktion:   Liefert die Listen- Nummer fuer einen gegebenen
 *             Formatindex zurueck.
 * Input:      Nummer des Eintrags
 * Output:     Kategorie- Nummer
 */
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

short SvxNumberFormatShell::GetListPos4Entry( const OUString& rFmtString )
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
                if (rFmtString==aCurrencyFormatList[i])
                {
                    nSelP = static_cast<short>(i);
                    break;
                }
            }
        }
    }
    return nSelP;
}

OUString SvxNumberFormatShell::GetStandardName() const
{
    return pFormatter->GetStandardName( eCurLanguage);
}

void SvxNumberFormatShell::GetCurrencySymbols(std::vector<OUString>& rList, sal_uInt16* pPos)
{
    const NfCurrencyEntry* pTmpCurrencyEntry=SvNumberFormatter::MatchSystemCurrency();

    bool bFlag=(pTmpCurrencyEntry==nullptr);

    SvxCurrencyToolBoxControl::GetCurrencySymbols( rList, bFlag, aCurCurrencyList );

    if(pPos!=nullptr)
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


void SvxNumberFormatShell::SetCurrencySymbol(sal_uInt32 nPos)
{
    const NfCurrencyTable& rCurrencyTable=SvNumberFormatter::GetTheCurrencyTable();
    sal_uInt16 nCount=rCurrencyTable.size();

    bBankingSymbol=(nPos>=nCount);

    if(nPos<aCurCurrencyList.size())
    {
        sal_uInt16 nCurrencyPos=aCurCurrencyList[nPos];
        if(nCurrencyPos!=(sal_uInt16)-1)
        {
            pCurCurrencyEntry=const_cast<NfCurrencyEntry*>(&rCurrencyTable[nCurrencyPos]);
            nCurCurrencyEntryPos=nPos;
        }
        else
        {
            pCurCurrencyEntry=nullptr;
            nCurCurrencyEntryPos=0;
            nCurFormatKey=pFormatter->GetFormatIndex(
                         NF_CURRENCY_1000DEC2_RED, eCurLanguage);
        }
    }
}


void SvxNumberFormatShell::SetCurCurrencyEntry(NfCurrencyEntry* pCEntry)
{
    pCurCurrencyEntry=pCEntry;
}

bool SvxNumberFormatShell::IsTmpCurrencyFormat( const OUString& rFmtString )
{
    sal_uInt32 nFound;
    FindEntry(rFmtString, &nFound);

    if(nFound==NUMBERFORMAT_ENTRY_NEW_CURRENCY)
    {
        return true;
    }
    return false;
}

sal_uInt16 SvxNumberFormatShell::FindCurrencyFormat( const OUString& rFmtString )
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
        for(size_t j=nStart;j<aCurCurrencyList.size();j++)
        {
            if(aCurCurrencyList[j]==nPos) return j;
        }
    }
    return (sal_uInt16) -1;
}

sal_uInt16 SvxNumberFormatShell::FindCurrencyTableEntry( const OUString& rFmtString, bool &bTestBanking )
{
    sal_uInt16 nPos=(sal_uInt16) -1;

    const NfCurrencyTable& rCurrencyTable=SvNumberFormatter::GetTheCurrencyTable();
    sal_uInt16 nCount=rCurrencyTable.size();

    const SvNumberformat* pFormat;
    OUString aSymbol, aExtension;
    sal_uInt32 nFound = pFormatter->TestNewString( rFmtString, eCurLanguage );
    if ( nFound != NUMBERFORMAT_ENTRY_NOT_FOUND &&
            ((pFormat = pFormatter->GetEntry( nFound )) != nullptr) &&
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

            if(rFmtString.indexOf(_aSymbol) != -1)
            {
                bTestBanking=false;
                nPos=i;
                break;
            }
            else if(rFmtString.indexOf(aBankSymbol) != -1)
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
    for(size_t j=nStart;j<aCurCurrencyList.size();j++)
    {
        if(aCurCurrencyList[j]==nPos) return j;
    }
    return (sal_uInt16) -1;
}

bool SvxNumberFormatShell::IsInTable(sal_uInt16 const nPos,
        bool const bTmpBanking, OUString const& rFmtString)
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

            if ( pTmpCurrencyEntry!=nullptr)
            {
                pFormatter->GetCurrencyFormatStrings( aWSStringsDtor,
                                *pTmpCurrencyEntry, bTmpBanking );

                for(size_t i=0;i<aWSStringsDtor.size();i++)
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
