/*************************************************************************
 *
 *  $RCSfile: numfmtsh.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2001-03-09 18:09:39 $
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

// include ---------------------------------------------------------------

#ifndef _SV_COLOR_HXX
#include <vcl/color.hxx>
#endif
#pragma hdrstop

#define _SVX_NUMFMTSH_CXX
#define _SVSTDARR_STRINGSDTOR

#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

#define _ZFORLIST_DECLARE_TABLE
#ifndef _ZFORLIST_HXX //autogen
#include <svtools/zforlist.hxx>
#endif

#ifndef _ZFORMAT_HXX //autogen
#include <svtools/zformat.hxx>
#endif

#ifndef _SVX_LANGTAB_HXX //autogen
#include <langtab.hxx>
#endif

#include "numfmtsh.hxx"
// class SvxNumberFormatShell --------------------------------------------

const double SvxNumberFormatShell::DEFAULT_NUMVALUE = 1234.56789;

SV_IMPL_PTRARR( NfShCurrencyEntries, NfCurrencyEntry* );

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

#define _INIT \
    pFormatter      ( pNumFormatter ),  \
    pCurFmtTable    ( NULL ),           \
    nCurFormatKey   ( nFormatKey ),     \
    nInitFormatKey  ( nFormatKey ),     \
    eValType        ( eNumValType ),    \
    pCurCurrencyEntry(NULL),            \
    bBankingSymbol  (sal_False),            \
    nCurCurrencyEntryPos((sal_uInt16) SELPOS_NONE), \
    bUndoAddList    ( sal_True )

// -----------------------------------------------------------------------

SvxNumberFormatShell::SvxNumberFormatShell( SvNumberFormatter*  pNumFormatter,
                                            sal_uInt32              nFormatKey,
                                            SvxNumberValueType  eNumValType,
                                            const String&       rNumStr )
    :   _INIT
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
                                            const String*       pNumStr )
    :   _INIT
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

        for ( sal_uInt16 i = 0; i < aAddList.Count(); ++i )
            pFormatter->DeleteEntry( aAddList[i] );
    }

    //--------------------------------
    // Add-/Remove-Listen leerraeumen:
    //--------------------------------
    aAddList.Remove( 0, aAddList.Count() );
    aDelList.Remove( 0, aAddList.Count() );

    if(aCurrencyFormatList.Count()>0)
        aCurrencyFormatList.DeleteAndDestroy(0,aCurrencyFormatList.Count());
}

// -----------------------------------------------------------------------

sal_uInt32 SvxNumberFormatShell::GetUpdateDataCount() const
{
    return aDelList.Count();
}

// -----------------------------------------------------------------------

void SvxNumberFormatShell::GetUpdateData( sal_uInt32* pDelArray, const sal_uInt32 nSize )
{
    const sal_uInt32 nCount = aDelList.Count();

    DBG_ASSERT( pDelArray && ( nSize == nCount ), "Array nicht initialisiert!" );

    if ( pDelArray && ( nSize == nCount ) )
        for ( sal_uInt16 i = 0; i < aDelList.Count(); ++i )
            *pDelArray++ = aDelList[i];
}

// -----------------------------------------------------------------------

void SvxNumberFormatShell::CategoryChanged( sal_uInt16     nCatLbPos,
                                            short&     rFmtSelPos,
                                            SvStrings& rFmtEntries )
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
                                            short&       rFmtSelPos,
                                            SvStrings&   rFmtEntries )
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
                                          Color&  rFontColor )
{
    //nCurFormatKey = pCurFmtTable->GetKey( pCurFmtTable->GetObject( nFmtLbPos ) );

    if(nFmtLbPos<aCurEntryList.Count())
    {
        nCurFormatKey=aCurEntryList[nFmtLbPos];

        if(nCurFormatKey!=NUMBERFORMAT_ENTRY_NOT_FOUND)
        {
            GetPreviewString_Impl( rPreviewStr, rFontColor );
        }
        else if(nCurCategory==NUMBERFORMAT_CURRENCY)
        {
            if(nFmtLbPos<aCurrencyFormatList.Count())
            {
                //nCurFormatKey=nFmtLbPos;
                MakePrevStringFromVal(*aCurrencyFormatList[nFmtLbPos],
                                    rPreviewStr,rFontColor,nValNum);
            }
        }
    }
}
// -----------------------------------------------------------------------

sal_Bool SvxNumberFormatShell::AddFormat( String& rFormat,  sal_uInt32& rErrPos,
                                      sal_uInt16& rCatLbSelPos, short& rFmtSelPos,
                                      SvStrings& rFmtEntries )
{
    sal_Bool    bInserted   = sal_False;
    sal_uInt32  nAddKey     = pFormatter->GetEntryKey( rFormat, eCurLanguage );

    if ( nAddKey != NUMBERFORMAT_ENTRY_NOT_FOUND ) // bereits vorhanden?
    {
        if ( IsRemoved_Impl( nAddKey ) )
        {
            // Key suchen und loeschen
            sal_Bool    bFound  = sal_False;
            sal_uInt16  nAt     = 0;

            for ( sal_uInt16 i = 0; !bFound && i < aDelList.Count(); ++i )
            {
                if ( aDelList[i] == nAddKey )
                {
                    bFound  = sal_True;
                    nAt     = i;
                }
            }
            DBG_ASSERT( bFound, "Key not found" );
            aDelList.Remove( nAt );
            bInserted = sal_True;
        }
        else
            DBG_ERROR( "Doppeltes Format!" );
    }
    else // neues Format
    {
        xub_StrLen nErrPos = (xub_StrLen)rErrPos;
        bInserted = pFormatter->PutEntry( rFormat, nErrPos,
                                          nCurCategory, nAddKey,
                                          eCurLanguage );
        rErrPos = sal_uInt32(nErrPos);
    }

    if ( bInserted ) // eingefuegt
    {
        nCurFormatKey = nAddKey;
        DBG_ASSERT( !IsAdded_Impl( nCurFormatKey ), "Doppeltes Format!" );
        aAddList.Insert( nCurFormatKey, aAddList.Count() );

        // aktuelle Tabelle holen
        pCurFmtTable = &(pFormatter->GetEntryTable( nCurCategory,
                                                    nCurFormatKey,
                                                    eCurLanguage ));
        nCurCategory=pFormatter->GetType(nAddKey); //@@ ???
        CategoryToPos_Impl( nCurCategory, rCatLbSelPos );
        rFmtSelPos = FillEntryList_Impl( rFmtEntries );
    }
    else if ( rErrPos != 0 ) // Syntaxfehler
        ;
    else // Doppelt einfuegen nicht moeglich
        DBG_ERROR( "Doppeltes Format!" ); // oder doch?

    return bInserted;
}

// -----------------------------------------------------------------------

sal_Bool SvxNumberFormatShell::RemoveFormat( const String&  rFormat,
                                         sal_uInt16&        rCatLbSelPos,
                                         short&         rFmtSelPos,
                                         SvStrings&     rFmtEntries )
{
    sal_uInt32 nDelKey = pFormatter->GetEntryKey( rFormat, eCurLanguage );

    DBG_ASSERT( nDelKey != NUMBERFORMAT_ENTRY_NOT_FOUND, "Eintrag nicht gefunden!" );
    DBG_ASSERT( !IsRemoved_Impl( nDelKey ), "Eintrag bereits geloescht!" );

    if ( (nDelKey != NUMBERFORMAT_ENTRY_NOT_FOUND) && !IsRemoved_Impl( nDelKey ) )
    {
        aDelList.Insert( nDelKey, aDelList.Count() );

        if ( IsAdded_Impl( nDelKey ) )
        {
            // Key suchen und loeschen
            sal_Bool    bFound  = sal_False;
            sal_uInt16  nAt     = 0;

            for ( sal_uInt16 i = 0; !bFound && i < aAddList.Count(); ++i )
            {
                if ( aAddList[i] == nDelKey )
                {
                    bFound  = sal_True;
                    nAt     = i;
                }
            }
            DBG_ASSERT( bFound, "Key not found" );
            aAddList.Remove( nAt );
        }

        nCurCategory=pFormatter->GetType(nDelKey); //@@ 01.10.97
        pCurFmtTable = &(pFormatter->GetEntryTable( nCurCategory,
                                                    nCurFormatKey,
                                                    eCurLanguage ));

        nCurFormatKey=pFormatter->GetStandardFormat(nCurCategory,
                                            eCurLanguage );

        CategoryToPos_Impl( nCurCategory, rCatLbSelPos );
        rFmtSelPos = FillEntryList_Impl( rFmtEntries );
        //rFmtSelPos = (short) nCurFormatKey; //@@ 01.10.97
    }
    return sal_True;
}

// -----------------------------------------------------------------------

void SvxNumberFormatShell::MakeFormat( String& rFormat,
                                       sal_Bool bThousand, sal_Bool bNegRed,
                                       sal_uInt16 nPrecision, sal_uInt16 nLeadingZeroes,
                                       sal_uInt16 nCurrencyPos)
{
    if(aCurrencyFormatList.Count()>nCurrencyPos)
    {
         sal_uInt32 rErrPos=0;
        sal_uInt16 rCatLbSelPos=0;
        short  rFmtSelPos=0;
        SvStrings aFmtEList;

        sal_uInt32 nFound = pFormatter->TestNewString( *aCurrencyFormatList[nCurrencyPos], eCurLanguage );

        if ( nFound == NUMBERFORMAT_ENTRY_NOT_FOUND )
        {
            AddFormat( *aCurrencyFormatList[nCurrencyPos],rErrPos,rCatLbSelPos,
                    rFmtSelPos,aFmtEList);
        }

        if(rErrPos==0)
        {
            pFormatter->GenerateFormat( rFormat, nCurFormatKey,
                                        eCurLanguage,
                                        bThousand, bNegRed,
                                        nPrecision, nLeadingZeroes );
        }
        aFmtEList.DeleteAndDestroy(0,aFmtEList.Count());
    }
    else
    {
        pFormatter->GenerateFormat( rFormat, nCurFormatKey,
                                    eCurLanguage,
                                    bThousand, bNegRed,
                                    nPrecision, nLeadingZeroes );
    }
}

// -----------------------------------------------------------------------

void SvxNumberFormatShell::GetOptions( const String&    rFormat,
                                       sal_Bool&            rThousand,
                                       sal_Bool&            rNegRed,
                                       sal_uInt16&          rPrecision,
                                       sal_uInt16&          rLeadingZeroes,
                                       sal_uInt16&          rCatLbPos )
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
        sal_Bool bTestBanking=sal_False;
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
                                              Color&        rFontColor )
{
    Color* pColor = NULL;

    ULONG nExistingFormat = pFormatter->GetEntryKey( rFormatStr, eCurLanguage );
    if ( nExistingFormat == NUMBERFORMAT_ENTRY_NOT_FOUND )
    {
        //  real preview - not implemented in NumberFormatter for text formats

        pFormatter->GetPreviewString( rFormatStr, nValNum, rPreviewStr,
                                      &pColor, eCurLanguage );
    }
    else
    {
        //  format exists

        //  #50441# if a string was set in addition to the value, use it for text formats
        BOOL bUseText = ( eValType == SVX_VALUE_TYPE_STRING ||
                            ( aValStr.Len() && ( pFormatter->GetType(nExistingFormat) & NUMBERFORMAT_TEXT ) ) );
        if ( bUseText )
            pFormatter->GetOutputString( aValStr, nExistingFormat,
                                         rPreviewStr, &pColor );
        else
            pFormatter->GetOutputString( nValNum, nExistingFormat,
                                         rPreviewStr, &pColor );
    }

    if ( pColor )
        rFontColor = *pColor;
    else
        rFontColor = Color( COL_BLACK );
}

// -----------------------------------------------------------------------

sal_Bool SvxNumberFormatShell::IsUserDefined( const String& rFmtString )
{
    sal_uInt32 nFound = pFormatter->GetEntryKey( rFmtString, eCurLanguage );

    sal_Bool bFlag=sal_False;
    if ( nFound != NUMBERFORMAT_ENTRY_NOT_FOUND )
    {
        bFlag=pFormatter->IsUserDefined( rFmtString, eCurLanguage );

        if(bFlag)
        {
            const SvNumberformat* pNumEntry = pFormatter->GetEntry(nFound);

            if(pNumEntry!=NULL && pNumEntry->HasNewCurrency())
            {
                sal_Bool bTestBanking;
                sal_uInt16 nPos=FindCurrencyTableEntry(rFmtString,bTestBanking);
                bFlag=!IsInTable(nPos,bTestBanking,rFmtString);
            }
        }
    }
    return bFlag;
}

// -----------------------------------------------------------------------

sal_Bool SvxNumberFormatShell::FindEntry( const String& rFmtString, sal_uInt32* pAt /* = NULL */ )
{
    sal_Bool bRes=sal_False;
    sal_uInt32 nFound = pFormatter->TestNewString( rFmtString, eCurLanguage );

    if ( nFound == NUMBERFORMAT_ENTRY_NOT_FOUND )
    {
        sal_Bool bTestBanking=sal_False;
        sal_uInt16 nPos=FindCurrencyTableEntry(rFmtString, bTestBanking );

        if(IsInTable(nPos,bTestBanking,rFmtString))
        {
            nFound=NUMBERFORMAT_ENTRY_NEW_CURRENCY;
            bRes=sal_True;
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

void SvxNumberFormatShell::GetInitSettings( sal_uInt16&       nCatLbPos,
                                            LanguageType& rLangType,
                                            sal_uInt16&       nFmtLbSelPos,
                                            SvStrings&    rFmtEntries,
                                            String&       rPrevString,
                                            Color&        rPrevColor )
{
    // -------------------------------------------------------------------
    // Vorbedingung: Zahlenformatierer gefunden
    DBG_ASSERT( pFormatter != NULL, "Zahlenformatierer nicht gefunden!" );

    sal_uInt16                  nCount      = 0;
    short                   nSelPos     = SELPOS_NONE;
    SvNumberFormatTable*    pFmtTable   = NULL;

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
    GetPreviewString_Impl( rPrevString, rPrevColor );
}

// -----------------------------------------------------------------------

short SvxNumberFormatShell::FillEntryList_Impl( SvStrings& rList )
{
    /* Erstellen einer aktuellen Liste von Format-Eintraegen.
     * Rueckgabewert ist die Listenposition des aktuellen Formates.
     * Ist die Liste leer oder gibt es kein aktuelles Format,
     * so wird SELPOS_NONE geliefert.
     */
    short nSelPos=0;
    aCurEntryList.Remove(nSelPos,aCurEntryList.Count());
    sal_uInt16          nPrivCat;
    nSelPos=SELPOS_NONE;

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

void SvxNumberFormatShell::FillEListWithStd_Impl( SvStrings& rList,sal_uInt16 nPrivCat,short &nSelPos )
{
    /* Erstellen einer aktuellen Liste von Format-Eintraegen.
     * Rueckgabewert ist die Listenposition des aktuellen Formates.
     * Ist die Liste leer oder gibt es kein aktuelles Format,
     * so wird SELPOS_NONE geliefert.
     */
    DBG_ASSERT( pCurFmtTable != NULL, "Unbekanntes Zahlenformat!" );

    if(aCurrencyFormatList.Count()>0)
        aCurrencyFormatList.DeleteAndDestroy(0,aCurrencyFormatList.Count());

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
                                     break;
        }

        nSelPos=FillEListWithFormats_Impl(rList,nSelPos,eOffsetStart,eOffsetEnd);

        if(nPrivCat==CAT_DATE || nPrivCat==CAT_TIME)
        {
            nSelPos=FillEListWithDateTime_Impl(rList,nSelPos);
            //if(nSelPos!=SELPOS_NONE) nSelPos=nTmpPos;
        }
    }
}

short SvxNumberFormatShell::FillEListWithFormats_Impl( SvStrings& rList,short nSelPos,
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

    const SvNumberformat*   pNumEntry   = pCurFmtTable->First();
    sal_uInt16          nCount      = 0;
    sal_uInt32          nNFEntry;
    String          aStrComment;
    String          aNewFormNInfo;
    String          aPrevString;
    String          a2PrevString;
    Color           aColor;

    short           nMyCat      = SELPOS_NONE;
    short           nIq=0;

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

        const StringPtr pStr = new String(aNewFormNInfo);

        if ( nNFEntry == nCurFormatKey )
        {
            nSelPos = ( !IsRemoved_Impl( nNFEntry ) ) ? aCurEntryList.Count() : SELPOS_NONE;
        }

        rList.Insert( pStr,rList.Count());
        aCurEntryList.Insert( nNFEntry, aCurEntryList.Count() );
    }

    return nSelPos;
}

short SvxNumberFormatShell::FillEListWithDateTime_Impl( SvStrings& rList,short nSelPos)
{
    sal_uInt16  nMyType;

    DBG_ASSERT( pCurFmtTable != NULL, "Unbekanntes Zahlenformat!" );

    const SvNumberformat*   pNumEntry   = pCurFmtTable->First();
    sal_uInt16          nCount      = 0;
    sal_uInt32          nNFEntry;
    String          aStrComment;
    String          aNewFormNInfo;
    String          aPrevString;
    String          a2PrevString;
    Color           aColor;

    short           nMyCat      = SELPOS_NONE;
    short           nIq=0;

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

            const StringPtr pStr = new String(aNewFormNInfo);

            if ( nNFEntry == nCurFormatKey )
            {
                nSelPos = ( !IsRemoved_Impl( nNFEntry ) ) ? aCurEntryList.Count() : SELPOS_NONE;
            }

            rList.Insert( pStr,rList.Count());
            aCurEntryList.Insert( nNFEntry, aCurEntryList.Count() );
        }
    }

    return nSelPos;
}

short SvxNumberFormatShell::FillEListWithCurrency_Impl( SvStrings& rList,short nSelPos)
{
    /* Erstellen einer aktuellen Liste von Format-Eintraegen.
     * Rueckgabewert ist die Listenposition des aktuellen Formates.
     * Ist die Liste leer oder gibt es kein aktuelles Format,
     * so wird SELPOS_NONE geliefert.
     */
    DBG_ASSERT( pCurFmtTable != NULL, "Unbekanntes Zahlenformat!" );

    const NfCurrencyEntry* pTmpCurrencyEntry;
    sal_Bool             bTmpBanking;
    XubString        rSymbol;

    sal_Bool bFlag=pFormatter->GetNewCurrencySymbolString(nCurFormatKey,rSymbol,
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


short SvxNumberFormatShell::FillEListWithSysCurrencys( SvStrings& rList,short nSelPos)
{
    /* Erstellen einer aktuellen Liste von Format-Eintraegen.
     * Rueckgabewert ist die Listenposition des aktuellen Formates.
     * Ist die Liste leer oder gibt es kein aktuelles Format,
     * so wird SELPOS_NONE geliefert.
     */
    sal_uInt16  nMyType;

    DBG_ASSERT( pCurFmtTable != NULL, "Unbekanntes Zahlenformat!" );

    const SvNumberformat*   pNumEntry   = pCurFmtTable->First();
    sal_uInt16          nCount      = 0;
    sal_uInt32          nNFEntry;
    String          aStrComment;
    String          aNewFormNInfo;
    String          aPrevString;
    String          a2PrevString;
    Color           aColor;

    nCurCurrencyEntryPos=0;

    short           nMyCat      = SELPOS_NONE;
    short           nIq=0;

    NfIndexTableOffset eOffsetStart=NF_CURRENCY_START;
    NfIndexTableOffset eOffsetEnd=NF_CURRENCY_END;;
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

        const StringPtr pStr = new String(aNewFormNInfo);

        if ( nNFEntry == nCurFormatKey )
        {
            nSelPos = ( !IsRemoved_Impl( nNFEntry ) ) ? aCurEntryList.Count() : SELPOS_NONE;
        }

        rList.Insert( pStr,rList.Count());
        aCurEntryList.Insert( nNFEntry, aCurEntryList.Count() );
    }

    if(nCurCategory!=NUMBERFORMAT_ALL)
    {
        pNumEntry   = pCurFmtTable->First();
        nCount      = 0;
        while ( pNumEntry )
        {
            sal_uInt32 nKey = pCurFmtTable->GetCurKey();

            nCount++;

            if ( !IsRemoved_Impl( nKey ))
            {
                sal_Bool bUserNewCurrency=sal_False;
                if(pNumEntry->HasNewCurrency())
                {
                    const NfCurrencyEntry* pTmpCurrencyEntry;
                    sal_Bool            bTmpBanking;
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

                    const StringPtr pStr = new String(aNewFormNInfo);

                    if ( nKey == nCurFormatKey ) nSelPos =aCurEntryList.Count();
                    rList.Insert( pStr,rList.Count());
                    aCurEntryList.Insert( nKey, aCurEntryList.Count() );
                }
            }
            pNumEntry = pCurFmtTable->Next();
        }
    }
    return nSelPos;
}

short SvxNumberFormatShell::FillEListWithUserCurrencys( SvStrings& rList,short nSelPos)
{
    /* Erstellen einer aktuellen Liste von Format-Eintraegen.
     * Rueckgabewert ist die Listenposition des aktuellen Formates.
     * Ist die Liste leer oder gibt es kein aktuelles Format,
     * so wird SELPOS_NONE geliefert.
     */
    sal_uInt16 nMyType;

    DBG_ASSERT( pCurFmtTable != NULL, "Unbekanntes Zahlenformat!" );

    sal_uInt16          nCount      = 0;
    String          aStrComment;
    String          aNewFormNInfo;
    String          aPrevString;
    String          a2PrevString;
    Color           aColor;
    short           nMyCat = SELPOS_NONE;
    short           nIq=0;

    const NfCurrencyEntry* pTmpCurrencyEntry;
    sal_Bool            bTmpBanking;
    XubString       rSymbol;
    XubString       rBankSymbol;

    SvStrings       aList;
    SvULongs        aKeyList;

    sal_Bool bFlag=pFormatter->GetNewCurrencySymbolString(nCurFormatKey,rSymbol,
                &pTmpCurrencyEntry,&bTmpBanking);

    XubString rShortSymbol;

    if(pCurCurrencyEntry==NULL)
    {
        pCurCurrencyEntry=(NfCurrencyEntry*)pTmpCurrencyEntry;
        bBankingSymbol=bTmpBanking;
        nCurCurrencyEntryPos=FindCurrencyFormat(pTmpCurrencyEntry,bTmpBanking);
    }
    else
    {
        pTmpCurrencyEntry=pCurCurrencyEntry;
        bTmpBanking=bBankingSymbol;
    }

    if(pTmpCurrencyEntry!=NULL)
    {
        pTmpCurrencyEntry->BuildSymbolString(rSymbol,sal_False);
        pTmpCurrencyEntry->BuildSymbolString(rBankSymbol,sal_True);
        pTmpCurrencyEntry->BuildSymbolString(rShortSymbol,bTmpBanking,sal_True);
    }

    const SvNumberformat*   pNumEntry   = pCurFmtTable->First();

    while ( pNumEntry )
    {
        sal_uInt32 nKey = pCurFmtTable->GetCurKey();

        nCount++;

        if ( !IsRemoved_Impl( nKey ) )
        {
            if( pNumEntry->GetType() & NUMBERFORMAT_DEFINED ||
                    pNumEntry->IsAdditionalStandardDefined() )
            {
                nMyCat=pNumEntry->GetType() & ~NUMBERFORMAT_DEFINED;
                aStrComment=pNumEntry->GetComment();
                CategoryToPos_Impl(nMyCat,nMyType);
                aNewFormNInfo=  pNumEntry->GetFormatstring();

                sal_Bool bInsFlag=sal_False;
                if ( pNumEntry->HasNewCurrency() )
                    bInsFlag = sal_True;    // merge locale formats into currency selection
                else if(!bTmpBanking && aNewFormNInfo.Search(rSymbol)!=STRING_NOTFOUND||
                   bTmpBanking && aNewFormNInfo.Search(rBankSymbol)!=STRING_NOTFOUND)
                {
                    bInsFlag=sal_True;
                }
                else if(aNewFormNInfo.Search(rShortSymbol)!=STRING_NOTFOUND)
                {
                    XubString rTstSymbol;
                    const NfCurrencyEntry* pTstCurrencyEntry;
                    sal_Bool bTstBanking;

                    sal_Bool bTstFlag=pFormatter->GetNewCurrencySymbolString(nKey,rTstSymbol,
                                &pTstCurrencyEntry,&bTstBanking);

                    if(pTmpCurrencyEntry==pTstCurrencyEntry && bTstBanking==bTmpBanking)
                    {
                        bInsFlag=sal_True;
                    }

                }

                if(bInsFlag)
                {
                    const StringPtr pStr = new String(aNewFormNInfo);

                    aList.Insert( pStr,aList.Count());
                    aKeyList.Insert( nKey, aKeyList.Count() );
                }
            }
        }
        pNumEntry = pCurFmtTable->Next();
    }

    NfWSStringsDtor aWSStringsDtor;
    sal_uInt16 nDefault;
    if ( pTmpCurrencyEntry && nCurCategory != NUMBERFORMAT_ALL )
        nSelPos = nDefault = pFormatter->GetCurrencyFormatStrings(
            aWSStringsDtor, *pTmpCurrencyEntry, bTmpBanking );
    else
        nDefault = 0;

    sal_uInt16 i,nPos;
    sal_uInt16 nOldListCount = rList.Count();
    for( i=0, nPos=nOldListCount; i<aWSStringsDtor.Count(); i++)
    {
        sal_Bool bFlag=sal_True;
        String aInsStr(*aWSStringsDtor[i]);
        sal_uInt16 j;
        for(j=0;j<aList.Count();j++)
        {
            const StringPtr pTestStr=aList[j];

            if(*pTestStr==aInsStr)
            {
                bFlag=sal_False;
                break;
            }
        }
        if(bFlag)
        {
            rList.Insert(new String(aInsStr),nPos);
            aCurEntryList.Insert( NUMBERFORMAT_ENTRY_NOT_FOUND, nPos++);
        }
        else
        {
            rList.Insert(aList[j],nPos);
            aList.Remove(j);
            aCurEntryList.Insert( aKeyList[j],nPos++);
            aKeyList.Remove(j);
        }
    }

    for(i=0;i<aKeyList.Count();i++)
    {
        if(aKeyList[i]!=NUMBERFORMAT_ENTRY_NOT_FOUND)
        {
            rList.Insert(aList[i],rList.Count());
            aCurEntryList.Insert( aKeyList[i],aCurEntryList.Count());
        }
    }

    for(i=nOldListCount;i<rList.Count();i++)
    {
        aCurrencyFormatList.Insert(new String(*rList[i]),aCurrencyFormatList.Count());

        if ( nSelPos == SELPOS_NONE && aCurEntryList[i] == nCurFormatKey )
            nSelPos = i;
    }

    if ( nSelPos == SELPOS_NONE && nCurCategory != NUMBERFORMAT_ALL )
        nSelPos = nDefault;

    return nSelPos;
}


short SvxNumberFormatShell::FillEListWithUsD_Impl( SvStrings& rList, sal_uInt16 nPrivCat, short nSelPos)
{
    /* Erstellen einer aktuellen Liste von Format-Eintraegen.
     * Rueckgabewert ist die Listenposition des aktuellen Formates.
     * Ist die Liste leer oder gibt es kein aktuelles Format,
     * so wird SELPOS_NONE geliefert.
     */
    sal_uInt16 nMyType;

    DBG_ASSERT( pCurFmtTable != NULL, "Unbekanntes Zahlenformat!" );

    const SvNumberformat*   pNumEntry   = pCurFmtTable->First();
    sal_uInt16          nCount      = 0;
    String          aStrComment;
    String          aNewFormNInfo;
    String          aPrevString;
    String          a2PrevString;
    Color           aColor;


    short           nMyCat      = SELPOS_NONE;
    short           nIq=0;
    sal_Bool        bAdditional = (nPrivCat != CAT_USERDEFINED &&
                                    nCurCategory != NUMBERFORMAT_ALL);

    while ( pNumEntry )
    {
        sal_uInt32 nKey = pCurFmtTable->GetCurKey();

        nCount++;

        if ( !IsRemoved_Impl( nKey ) )
        {
            if( (pNumEntry->GetType() & NUMBERFORMAT_DEFINED) ||
                    (bAdditional && pNumEntry->IsAdditionalStandardDefined()) )
            {
                nMyCat=pNumEntry->GetType() & ~NUMBERFORMAT_DEFINED;
                aStrComment=pNumEntry->GetComment();
                CategoryToPos_Impl(nMyCat,nMyType);
                aNewFormNInfo=  pNumEntry->GetFormatstring();

                sal_Bool bFlag=sal_True;
                if(pNumEntry->HasNewCurrency())
                {
                    sal_Bool bTestBanking;
                    sal_uInt16 nPos=FindCurrencyTableEntry(aNewFormNInfo,bTestBanking);
                    bFlag=!IsInTable(nPos,bTestBanking,aNewFormNInfo);
                }
                if(bFlag)
                {
                    const StringPtr pStr = new String(aNewFormNInfo);

                    if ( nKey == nCurFormatKey ) nSelPos =aCurEntryList.Count();
                    rList.Insert( pStr,rList.Count());
                    aCurEntryList.Insert( nKey, aCurEntryList.Count() );
                }
            }
        }
        pNumEntry = pCurFmtTable->Next();
    }
    return nSelPos;
}


// -----------------------------------------------------------------------

void SvxNumberFormatShell::GetPreviewString_Impl( String& rString,
                                                  Color& rColor )
{
    Color* pColor = NULL;

    //  #50441# if a string was set in addition to the value, use it for text formats
    BOOL bUseText = ( eValType == SVX_VALUE_TYPE_STRING ||
                        ( aValStr.Len() && ( pFormatter->GetType(nCurFormatKey) & NUMBERFORMAT_TEXT ) ) );

    if ( bUseText )
        pFormatter->GetOutputString( aValStr, nCurFormatKey,
                                     rString, &pColor );
    else
        pFormatter->GetOutputString( nValNum, nCurFormatKey,
                                     rString, &pColor );

    if ( pColor )
        rColor = *pColor;
    else
        rColor = Color( COL_BLACK );
}

// -----------------------------------------------------------------------

sal_Bool SvxNumberFormatShell::IsRemoved_Impl( sal_uInt32 nKey )
{
    sal_Bool bFound = sal_False;
    for ( sal_uInt16 i = 0; !bFound && i < aDelList.Count(); ++i )
        if ( aDelList[i] == nKey )
            bFound = sal_True;
    return bFound;
}

// -----------------------------------------------------------------------

sal_Bool SvxNumberFormatShell::IsAdded_Impl( sal_uInt32 nKey )
{
    sal_Bool bFound = sal_False;
    for ( sal_uInt16 i = 0; !bFound && i < aAddList.Count(); ++i )
        if ( aAddList[i] == nKey )
            bFound = sal_True;
    return bFound;
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

void SvxNumberFormatShell::MakePrevStringFromVal( const String& rFormatStr,
                                                String& rPreviewStr,
                                                Color&  rFontColor,
                                                double  nValue)
{
    Color* pColor = NULL;

    pFormatter->GetPreviewString( rFormatStr, nValue, rPreviewStr,
                                  &pColor, eCurLanguage );
    if ( pColor )
        rFontColor = *pColor;
    else
        rFontColor = Color( COL_BLACK );
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

    if(nEntry<aCurEntryList.Count())
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

    if(nEntry<aCurEntryList.Count())
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
        else if(aCurrencyFormatList.Count()>0)
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

sal_Bool SvxNumberFormatShell::GetUserDefined4Entry(short nEntry)
{
    const SvNumberformat *pNumEntry;
    if(nEntry<0) return 0;
    if(nEntry<aCurEntryList.Count())
    {
        sal_uInt32  nMyNfEntry=aCurEntryList[nEntry];
        pNumEntry = pFormatter->GetEntry(nMyNfEntry);

        if(pNumEntry!=NULL)
        {
            if((pNumEntry->GetType() & NUMBERFORMAT_DEFINED)>0)
            {
                return sal_True;
            }
        }
    }
    return sal_False;
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

    if(aCurrencyFormatList.Count()>0)
    {
        if(aCurrencyFormatList.Count()>nEntry)
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
    for(int i=0;i<aCurEntryList.Count();i++)
    {
        if(aCurEntryList[i]==nIdx)
        {
            nSelP=i;
            break;
        }
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
            if(aCurrencyFormatList.Count()>0)
            {
                for(sal_uInt16 i=0;i<aCurrencyFormatList.Count();i++)
                {
                    if (rFmtString==*aCurrencyFormatList[i])
                    {
                        nSelP=i;
                        break;
                    }
                }
            }
        }
    }
    return nSelP;
}

void SvxNumberFormatShell::GetCurrencySymbols(SvStringsDtor& rList,const XubString& rStrEurope, sal_uInt16* pPos)
{

    const NfCurrencyEntry* pTmpCurrencyEntry=pFormatter->MatchSystemCurrency();

    sal_Bool bFlag=(pTmpCurrencyEntry==NULL);

    GetCurrencySymbols(rList,rStrEurope, bFlag);

    if(pPos!=NULL)
    {
        const NfCurrencyTable& rCurrencyTable=pFormatter->GetTheCurrencyTable();
        sal_uInt16 nCount=rCurrencyTable.Count();

        *pPos=0;
        nCount=aCurCurrencyList.Count();

        if(bFlag)
        {
            *pPos=1;
            nCurCurrencyEntryPos=1;
        }
        else
        {
            for(sal_uInt16 i=1;i<nCount;i++)
            {
                if(aCurCurrencyList[i]!=(sal_uInt16)-1 &&
                    pTmpCurrencyEntry==rCurrencyTable[aCurCurrencyList[i]])
                {
                    *pPos=i;
                    nCurCurrencyEntryPos=i;
                    break;
                }
            }
        }
    }

}

void SvxNumberFormatShell::GetCurrencySymbols(SvStringsDtor& rList,const XubString& rStrEurope, sal_Bool bFlag)
{
    aCurCurrencyList.Remove(0,aCurCurrencyList.Count());

    const NfCurrencyTable& rCurrencyTable=pFormatter->GetTheCurrencyTable();
    sal_uInt16 nCount=rCurrencyTable.Count();

    SvxLanguageTable* pLanguageTable=new SvxLanguageTable;

    sal_uInt16 nStart=2;
    sal_uInt16 i,j;

    XubString aString(rCurrencyTable[0]->GetSymbol());
    aString += sal_Unicode(' ');
    aString += pLanguageTable->GetString(rCurrencyTable[0]->GetLanguage());

    WSStringPtr pStr = new XubString(aString);
    rList.Insert( pStr,rList.Count());
    sal_uInt16 nAuto=(sal_uInt16)-1;
    aCurCurrencyList.Insert(nAuto,aCurCurrencyList.Count());

    if(bFlag)
    {
        pStr = new XubString(aString);
        rList.Insert( pStr,rList.Count());
        aCurCurrencyList.Insert((sal_uInt16)0,aCurCurrencyList.Count());
        nStart=3;
    }

    aString = rCurrencyTable[1]->GetSymbol();
    aString += sal_Unicode(' ');
    aString += rStrEurope;

    pStr = new XubString(aString);
    rList.Insert( pStr,rList.Count());
    aCurCurrencyList.Insert((sal_uInt16)1,aCurCurrencyList.Count());

    for(i=2;i<nCount;i++)
    {
        XubString aString(rCurrencyTable[i]->GetSymbol());
        aString += sal_Unicode(' ');
        aString += pLanguageTable->GetString(rCurrencyTable[i]->GetLanguage());

        pStr = new XubString(aString);

        for(j=nStart;j<rList.Count();j++)
        {
            const StringPtr pTestStr=rList[j];

            if(*pTestStr>aString) break;
        }
        rList.Insert( pStr,j);
        aCurCurrencyList.Insert(i,j);
    }

    pStr = new XubString(rCurrencyTable[1]->GetBankSymbol());
    rList.Insert( pStr,rList.Count());
    aCurCurrencyList.Insert(1,aCurCurrencyList.Count());

    sal_uInt16 nCont=nCount+1;
    if(bFlag)nCont++;

    for(i=2;i<nCount;i++)
    {
        sal_Bool bTest=sal_True;
        pStr = new XubString(rCurrencyTable[i]->GetBankSymbol());

        for(j=nCont;j<rList.Count();j++)
        {
            const StringPtr pTestStr=rList[j];

            if(*pTestStr==*pStr)
                bTest=sal_False;
            else
                if(*pTestStr>*pStr) break;
        }
        if(bTest)
        {
            rList.Insert( pStr,j);
            aCurCurrencyList.Insert(i,j);
        }
    }
}

void SvxNumberFormatShell::GetCurrencyFormats(SvStrings& aListDtor)
{
    if(pCurCurrencyEntry!=NULL)
    {
        NfWSStringsDtor aWSStringsDtor;
        sal_uInt16 nDefault = pFormatter->GetCurrencyFormatStrings( aWSStringsDtor,
            *pCurCurrencyEntry, bBankingSymbol );

        for(sal_uInt16 i=0;i<aWSStringsDtor.Count();i++)
        {
            aListDtor.Insert(new String(*aWSStringsDtor[i]),aListDtor.Count());
        }
    }
}

sal_Bool SvxNumberFormatShell::IsBankingSymbol(sal_uInt16 nPos)
{
    const NfCurrencyTable& rCurrencyTable=pFormatter->GetTheCurrencyTable();
    sal_uInt16 nCount=rCurrencyTable.Count();

    return (nPos>nCount);
}

void SvxNumberFormatShell::SetCurrencySymbol(sal_uInt16 nPos)
{
    const NfCurrencyTable& rCurrencyTable=pFormatter->GetTheCurrencyTable();
    sal_uInt16 nCount=rCurrencyTable.Count();

    bBankingSymbol=(nPos>=nCount);

    if(nPos<aCurCurrencyList.Count())
    {
        sal_uInt16 nCurrencyPos=aCurCurrencyList[nPos];
        if(nCurrencyPos!=(sal_uInt16)-1)
        {
            pCurCurrencyEntry=rCurrencyTable[nCurrencyPos];
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

NfCurrencyEntry* SvxNumberFormatShell::GetCurCurrencyEntry()
{
    return pCurCurrencyEntry;
}

void SvxNumberFormatShell::SetCurCurrencyEntry(NfCurrencyEntry* pCEntry)
{
    pCurCurrencyEntry=pCEntry;
}

sal_Bool SvxNumberFormatShell::IsTmpCurrencyFormat( const String& rFmtString )
{
    sal_uInt32 nFound;
    sal_Bool bRes=FindEntry(rFmtString, &nFound);

    if(nFound==NUMBERFORMAT_ENTRY_NEW_CURRENCY)
    {
        return sal_True;
    }
    return sal_False;
}

sal_uInt16 SvxNumberFormatShell::FindCurrencyFormat( const String& rFmtString )
{
    const NfCurrencyTable& rCurrencyTable=pFormatter->GetTheCurrencyTable();
    sal_uInt16 nCount=rCurrencyTable.Count();

    sal_Bool bTestBanking=sal_False;

    sal_uInt16 nPos=FindCurrencyTableEntry(rFmtString, bTestBanking);

    sal_uInt16 nStart=0;

    if(nPos!=(sal_uInt16)-1)
    {
        if(bTestBanking && aCurCurrencyList.Count()>nPos)
        {
            nStart=nCount;
        }
        for(sal_uInt16 j=nStart;j<aCurCurrencyList.Count();j++)
        {
            if(aCurCurrencyList[j]==nPos) return j;
        }
    }
    return (sal_uInt16) -1;
}

sal_uInt16 SvxNumberFormatShell::FindCurrencyTableEntry( const String& rFmtString, sal_Bool &bTestBanking )
{
    sal_uInt16 nPos=(sal_uInt16) -1;

    const NfCurrencyTable& rCurrencyTable=pFormatter->GetTheCurrencyTable();
    sal_uInt16 nCount=rCurrencyTable.Count();

    const SvNumberformat* pFormat;
    String aSymbol, aExtension;
    sal_uInt32 nFound = pFormatter->TestNewString( rFmtString, eCurLanguage );
    if ( nFound != NUMBERFORMAT_ENTRY_NOT_FOUND &&
            (pFormat = pFormatter->GetEntry( nFound )) &&
            pFormat->GetNewCurrencySymbol( aSymbol, aExtension ) )
    {   // eventually match with format locale
        const NfCurrencyEntry* pTmpCurrencyEntry =
            pFormatter->GetCurrencyEntry( bTestBanking, aSymbol, aExtension,
            pFormat->GetLanguage() );
        if ( pTmpCurrencyEntry )
        {
            for(sal_uInt16 i=0;i<nCount;i++)
            {
                if(pTmpCurrencyEntry==rCurrencyTable[i])
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
            const NfCurrencyEntry* pTmpCurrencyEntry=rCurrencyTable[i];
            XubString aSymbol, aBankSymbol;
            pTmpCurrencyEntry->BuildSymbolString(aSymbol,sal_False);
            pTmpCurrencyEntry->BuildSymbolString(aBankSymbol,sal_True);

            if(rFmtString.Search(aSymbol)!=STRING_NOTFOUND)
            {
                bTestBanking=sal_False;
                nPos=i;
                break;
            }
            else if(rFmtString.Search(aBankSymbol)!=STRING_NOTFOUND)
            {
                bTestBanking=sal_True;
                nPos=i;
                break;
            }
        }
    }

    return nPos;
}

sal_uInt16 SvxNumberFormatShell::FindCurrencyFormat(const NfCurrencyEntry* pTmpCurrencyEntry,sal_Bool bTmpBanking)
{
    const NfCurrencyTable& rCurrencyTable=pFormatter->GetTheCurrencyTable();
    sal_uInt16 nCount=rCurrencyTable.Count();

    sal_Bool bTestBanking=sal_False;
    sal_uInt16 nPos=0;
    for(sal_uInt16 i=0;i<nCount;i++)
    {
        if(pTmpCurrencyEntry==rCurrencyTable[i])
        {
            nPos=i;
            break;
        }
    }

    sal_uInt16 nStart=0;
    if(bTmpBanking && aCurCurrencyList.Count()>nPos)
    {
        nStart=nCount;
    }
    for(sal_uInt16 j=nStart;j<aCurCurrencyList.Count();j++)
    {
        if(aCurCurrencyList[j]==nPos) return j;
    }
    return (sal_uInt16) -1;
}

sal_Bool SvxNumberFormatShell::IsInTable(sal_uInt16 nPos,sal_Bool bTmpBanking,const String &rFmtString)
{
    sal_Bool bFlag=sal_False;

    if(nPos!=(sal_uInt16)-1)
    {
        const NfCurrencyTable& rCurrencyTable=pFormatter->GetTheCurrencyTable();
        sal_uInt16 nCount=rCurrencyTable.Count();

        if(nPos<nCount)
        {
            NfWSStringsDtor aWSStringsDtor;
            sal_uInt16 nDefault;

            const NfCurrencyEntry* pTmpCurrencyEntry=rCurrencyTable[nPos];

            if ( pTmpCurrencyEntry!=NULL)
            {
                nDefault = pFormatter->GetCurrencyFormatStrings( aWSStringsDtor,
                                *pTmpCurrencyEntry, bTmpBanking );

                for(sal_uInt16 i=0;i<aWSStringsDtor.Count();i++)
                {
                    if(*aWSStringsDtor[i]==rFmtString)
                    {
                        bFlag=sal_True;
                        break;
                    }
                }
            }
        }
    }

    return bFlag;
}

