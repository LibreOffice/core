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


#undef SC_DLLIMPLEMENTATION



#include <vcl/waitobj.hxx>

#include "viewdata.hxx"
#include "document.hxx"
#include "uiitems.hxx"
#include "global.hxx"
#include "globalnames.hxx"
#include "dbdata.hxx"
#include "scresid.hxx"
#include "queryentry.hxx"
#include "typedstrdata.hxx"

#include "sc.hrc"
#include "filter.hrc"
#include "globstr.hrc"

#define _PFILTDLG_CXX
#include "pfiltdlg.hxx"
#undef _PFILTDLG_CXX
#include <svl/zforlist.hxx>

//==================================================================

ScPivotFilterDlg::ScPivotFilterDlg( Window*             pParent,
                                    const SfxItemSet&   rArgSet,
                                    SCTAB               nSourceTab )

    :   ModalDialog ( pParent, ScResId( RID_SCDLG_PIVOTFILTER ) ),
        //
        aFlCriteria     ( this, ScResId( FL_CRITERIA ) ),
        aLbField1       ( this, ScResId( LB_FIELD1 ) ),
        aLbCond1        ( this, ScResId( LB_COND1 ) ),
        aEdVal1         ( this, ScResId( ED_VAL1 ) ),
        aLbConnect1     ( this, ScResId( LB_OP1 ) ),
        aLbField2       ( this, ScResId( LB_FIELD2 ) ),
        aLbCond2        ( this, ScResId( LB_COND2 ) ),
        aEdVal2         ( this, ScResId( ED_VAL2 ) ),
        aLbConnect2     ( this, ScResId( LB_OP2 ) ),
        aLbField3       ( this, ScResId( LB_FIELD3 ) ),
        aLbCond3        ( this, ScResId( LB_COND3 ) ),
        aEdVal3         ( this, ScResId( ED_VAL3 ) ),
        aFtConnect      ( this, ScResId( FT_OP ) ),
        aFtField        ( this, ScResId( FT_FIELD ) ),
        aFtCond         ( this, ScResId( FT_COND ) ),
        aFtVal          ( this, ScResId( FT_VAL ) ),
        aFlOptions      ( this, ScResId( FL_OPTIONS ) ),
        aBtnCase        ( this, ScResId( BTN_CASE ) ),
        aBtnRegExp      ( this, ScResId( BTN_REGEXP ) ),
        aBtnUnique      ( this, ScResId( BTN_UNIQUE ) ),
        aFtDbAreaLabel  ( this, ScResId( FT_DBAREA_LABEL ) ),
        aFtDbArea       ( this, ScResId( FT_DBAREA ) ),
        aBtnOk          ( this, ScResId( BTN_OK ) ),
        aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp        ( this, ScResId( BTN_HELP ) ),
        aBtnMore        ( this, ScResId( BTN_MORE ) ),
        aStrUndefined   ( SC_RESSTR(SCSTR_UNDEFINED) ),
        aStrNone        ( SC_RESSTR(SCSTR_NONE) ),
        aStrEmpty       ( SC_RESSTR(SCSTR_FILTER_EMPTY) ),
        aStrNotEmpty    ( SC_RESSTR(SCSTR_FILTER_NOTEMPTY) ),
        aStrRow         ( SC_RESSTR(SCSTR_ROW) ),
        aStrColumn      ( SC_RESSTR(SCSTR_COLUMN) ),
        //
        nWhichQuery     ( rArgSet.GetPool()->GetWhich( SID_QUERY ) ),
        theQueryData    ( ((const ScQueryItem&)
                           rArgSet.Get( nWhichQuery )).GetQueryData() ),
        pOutItem        ( NULL ),
        pViewData       ( NULL ),
        pDoc            ( NULL ),
        nSrcTab         ( nSourceTab ),     // ist nicht im QueryParam
        nFieldCount     ( 0 )
{
    for (sal_uInt16 i=0; i<=MAXCOL; i++)
        pEntryLists[i] = NULL;

    Init( rArgSet );
    FreeResource();
}

//------------------------------------------------------------------------

ScPivotFilterDlg::~ScPivotFilterDlg()
{
    for (sal_uInt16 i=0; i<=MAXCOL; i++)
        delete pEntryLists[i];

    if ( pOutItem )
        delete pOutItem;
}

//------------------------------------------------------------------------

void ScPivotFilterDlg::Init( const SfxItemSet& rArgSet )
{
    const ScQueryItem& rQueryItem = (const ScQueryItem&)
                                    rArgSet.Get( nWhichQuery );

    aBtnCase.SetClickHdl    ( LINK( this, ScPivotFilterDlg, CheckBoxHdl ) );

    aLbField1.SetSelectHdl  ( LINK( this, ScPivotFilterDlg, LbSelectHdl ) );
    aLbField2.SetSelectHdl  ( LINK( this, ScPivotFilterDlg, LbSelectHdl ) );
    aLbField3.SetSelectHdl  ( LINK( this, ScPivotFilterDlg, LbSelectHdl ) );
    aLbConnect1.SetSelectHdl( LINK( this, ScPivotFilterDlg, LbSelectHdl ) );
    aLbConnect2.SetSelectHdl( LINK( this, ScPivotFilterDlg, LbSelectHdl ) );

    aBtnMore.AddWindow( &aBtnCase );
    aBtnMore.AddWindow( &aBtnRegExp );
    aBtnMore.AddWindow( &aBtnUnique );
    aBtnMore.AddWindow( &aFtDbAreaLabel );
    aBtnMore.AddWindow( &aFtDbArea );
    aBtnMore.AddWindow( &aFlOptions );

    aBtnCase    .Check( theQueryData.bCaseSens );
    aBtnRegExp  .Check( theQueryData.bRegExp );
    aBtnUnique  .Check( !theQueryData.bDuplicate );

    pViewData   = rQueryItem.GetViewData();
    pDoc        = pViewData ? pViewData->GetDocument() : NULL;

    // fuer leichteren Zugriff:
    aFieldLbArr  [0] = &aLbField1;
    aFieldLbArr  [1] = &aLbField2;
    aFieldLbArr  [2] = &aLbField3;
    aValueEdArr  [0] = &aEdVal1;
    aValueEdArr  [1] = &aEdVal2;
    aValueEdArr  [2] = &aEdVal3;
    aCondLbArr   [0] = &aLbCond1;
    aCondLbArr   [1] = &aLbCond2;
    aCondLbArr   [2] = &aLbCond3;

    if ( pViewData && pDoc )
    {
        ScRange         theCurArea ( ScAddress( theQueryData.nCol1,
                                                theQueryData.nRow1,
                                                nSrcTab ),
                                     ScAddress( theQueryData.nCol2,
                                                theQueryData.nRow2,
                                                nSrcTab ) );
        ScDBCollection* pDBColl     = pDoc->GetDBCollection();
        OUString theDbName = OUString(STR_DB_LOCAL_NONAME);

        /*
         * Ueberpruefen, ob es sich bei dem uebergebenen
         * Bereich um einen Datenbankbereich handelt:
         */

        if ( pDBColl )
        {
            ScAddress&  rStart  = theCurArea.aStart;
            ScAddress&  rEnd    = theCurArea.aEnd;
            ScDBData*   pDBData = pDBColl->GetDBAtArea( rStart.Tab(),
                                                        rStart.Col(), rStart.Row(),
                                                        rEnd.Col(),   rEnd.Row() );
            if ( pDBData )
                theDbName = pDBData->GetName();
        }

        OUStringBuffer aBuf;
        aBuf.appendAscii(" (");
        aBuf.append(theDbName);
        aBuf.append(sal_Unicode(')'));
        aFtDbArea.SetText(aBuf.makeStringAndClear());
    }
    else
    {
        aFtDbArea.SetText( EMPTY_STRING );
    }

    // Feldlisten einlesen und Eintraege selektieren:

    FillFieldLists();

    for ( SCSIZE i=0; i<3; i++ )
    {
        if ( theQueryData.GetEntry(i).bDoQuery )
        {
            const ScQueryEntry& rEntry = theQueryData.GetEntry(i);
            const ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
            OUString aValStr = rItem.maString;
            if (rEntry.IsQueryByEmpty())
                aValStr = aStrEmpty;
            else if (rEntry.IsQueryByNonEmpty())
                aValStr = aStrNotEmpty;
            sal_uInt16  nCondPos     = (sal_uInt16)rEntry.eOp;
            sal_uInt16  nFieldSelPos = GetFieldSelPos( static_cast<SCCOL>(rEntry.nField) );

            aFieldLbArr[i]->SelectEntryPos( nFieldSelPos );
            aCondLbArr [i]->SelectEntryPos( nCondPos );
            UpdateValueList( static_cast<sal_uInt16>(i+1) );
            aValueEdArr[i]->SetText( aValStr );
            if (aValStr.equals(aStrEmpty) || aValStr.equals(aStrNotEmpty))
                aCondLbArr[i]->Disable();
        }
        else
        {
            aFieldLbArr[i]->SelectEntryPos( 0 ); // "keiner" selektieren
            aCondLbArr [i]->SelectEntryPos( 0 ); // "=" selektieren
            UpdateValueList( static_cast<sal_uInt16>(i) );
            aValueEdArr[i]->SetText( EMPTY_STRING );
        }
        aValueEdArr[i]->SetModifyHdl( LINK( this, ScPivotFilterDlg, ValModifyHdl ) );
    }

    // Disable/Enable Logik:

       (aLbField1.GetSelectEntryPos() != 0)
    && (aLbField2.GetSelectEntryPos() != 0)
        ? aLbConnect1.SelectEntryPos( (sal_uInt16)theQueryData.GetEntry(1).eConnect )
        : aLbConnect1.SetNoSelection();

       (aLbField2.GetSelectEntryPos() != 0)
    && (aLbField3.GetSelectEntryPos() != 0)
        ? aLbConnect2.SelectEntryPos( (sal_uInt16)theQueryData.GetEntry(2).eConnect )
        : aLbConnect2.SetNoSelection();

    if ( aLbField1.GetSelectEntryPos() == 0 )
    {
        aLbConnect1.Disable();
        aLbField2.Disable();
        aLbCond2.Disable();
        aEdVal2.Disable();
    }
    else if ( aLbConnect1.GetSelectEntryCount() == 0 )
    {
        aLbField2.Disable();
        aLbCond2.Disable();
        aEdVal2.Disable();
    }

    if ( aLbField2.GetSelectEntryPos() == 0 )
    {
        aLbConnect2.Disable();
        aLbField3.Disable();
        aLbCond3.Disable();
        aEdVal3.Disable();
    }
    else if ( aLbConnect2.GetSelectEntryCount() == 0 )
    {
        aLbField3.Disable();
        aLbCond3.Disable();
        aEdVal3.Disable();
    }
}

//------------------------------------------------------------------------

void ScPivotFilterDlg::FillFieldLists()
{
    aLbField1.Clear();
    aLbField2.Clear();
    aLbField3.Clear();
    aLbField1.InsertEntry( aStrNone, 0 );
    aLbField2.InsertEntry( aStrNone, 0 );
    aLbField3.InsertEntry( aStrNone, 0 );

    if ( pDoc )
    {
        OUString  aFieldName;
        SCTAB   nTab        = nSrcTab;
        SCCOL   nFirstCol   = theQueryData.nCol1;
        SCROW   nFirstRow   = theQueryData.nRow1;
        SCCOL   nMaxCol     = theQueryData.nCol2;
        SCCOL   col = 0;
        sal_uInt16  i=1;

        for ( col=nFirstCol; col<=nMaxCol; col++ )
        {
            aFieldName = pDoc->GetString(col, nFirstRow, nTab);
            if ( aFieldName.isEmpty() )
            {
               OUStringBuffer aBuf;
                aBuf.append(aStrColumn);
                aFieldName = aBuf.makeStringAndClear().replaceAll("%1", ScColToAlpha( col ));
            }
            aLbField1.InsertEntry( aFieldName, i );
            aLbField2.InsertEntry( aFieldName, i );
            aLbField3.InsertEntry( aFieldName, i );
            i++;
        }
        nFieldCount = i;
    }
}

//------------------------------------------------------------------------

void ScPivotFilterDlg::UpdateValueList( sal_uInt16 nList )
{
    if ( pDoc && nList>0 && nList<=3 )
    {
        ComboBox*   pValList        = aValueEdArr[nList-1];
        sal_uInt16      nFieldSelPos    = aFieldLbArr[nList-1]->GetSelectEntryPos();
        sal_uInt16      nListPos        = 0;
        OUString     aCurValue       = pValList->GetText();

        pValList->Clear();
        pValList->InsertEntry( aStrNotEmpty, 0 );
        pValList->InsertEntry( aStrEmpty, 1 );
        nListPos = 2;

        if ( pDoc && nFieldSelPos )
        {
            SCCOL nColumn = theQueryData.nCol1 + static_cast<SCCOL>(nFieldSelPos) - 1;
            if (!pEntryLists[nColumn])
            {
                WaitObject aWaiter( this );

                SCTAB   nTab        = nSrcTab;
                SCROW   nFirstRow   = theQueryData.nRow1;
                SCROW   nLastRow    = theQueryData.nRow2;
                nFirstRow++;
                bool bHasDates = false;
                bool bCaseSens = aBtnCase.IsChecked();
                pEntryLists[nColumn] = new std::vector<ScTypedStrData>;
                pDoc->GetFilterEntriesArea(
                    nColumn, nFirstRow, nLastRow, nTab, bCaseSens, *pEntryLists[nColumn], bHasDates);
            }

            std::vector<ScTypedStrData>* pColl = pEntryLists[nColumn];
            std::vector<ScTypedStrData>::const_iterator it = pColl->begin(), itEnd = pColl->end();
            for (; it != itEnd; ++it)
            {
                pValList->InsertEntry(it->GetString(), nListPos);
                nListPos++;
            }
        }
        pValList->SetText( aCurValue );
    }
}

//------------------------------------------------------------------------

void ScPivotFilterDlg::ClearValueList( sal_uInt16 nList )
{
    if ( nList>0 && nList<=3 )
    {
        ComboBox* pValList = aValueEdArr[nList-1];
        pValList->Clear();
        pValList->InsertEntry( aStrNotEmpty, 0 );
        pValList->InsertEntry( aStrEmpty, 1 );
        pValList->SetText( EMPTY_STRING );
    }
}

//------------------------------------------------------------------------

sal_uInt16 ScPivotFilterDlg::GetFieldSelPos( SCCOL nField )
{
    if ( nField >= theQueryData.nCol1 && nField <= theQueryData.nCol2 )
        return static_cast<sal_uInt16>(nField - theQueryData.nCol1 + 1);
    else
        return 0;
}

//------------------------------------------------------------------------

const ScQueryItem& ScPivotFilterDlg::GetOutputItem()
{
    ScQueryParam    theParam( theQueryData );
    sal_uInt16          nConnect1 = aLbConnect1.GetSelectEntryPos();
    sal_uInt16          nConnect2 = aLbConnect2.GetSelectEntryPos();

    for ( SCSIZE i=0; i<3; i++ )
    {
        sal_uInt16      nField  = aFieldLbArr[i]->GetSelectEntryPos();
        ScQueryOp   eOp     = (ScQueryOp)aCondLbArr[i]->GetSelectEntryPos();

        sal_Bool bDoThis = (aFieldLbArr[i]->GetSelectEntryPos() != 0);
        theParam.GetEntry(i).bDoQuery = bDoThis;

        if ( bDoThis )
        {
            ScQueryEntry& rEntry = theParam.GetEntry(i);
            ScQueryEntry::Item& rItem = rEntry.GetQueryItem();

            OUString aStrVal = aValueEdArr[i]->GetText();

            /*
             * Dialog liefert die ausgezeichneten Feldwerte "leer"/"nicht leer"
             * als Konstanten in nVal in Verbindung mit dem Schalter
             * bQueryByString auf FALSE.
             */
            if ( aStrVal.equals(aStrEmpty) )
            {
                OSL_ASSERT(eOp == SC_EQUAL);
                rEntry.SetQueryByEmpty();
            }
            else if ( aStrVal.equals(aStrNotEmpty) )
            {
                OSL_ASSERT(eOp == SC_EQUAL);
                rEntry.SetQueryByNonEmpty();
            }
            else
            {
                rItem.maString = aStrVal;
                rItem.mfVal = 0.0;
                rItem.meType = ScQueryEntry::ByString;
            }

            rEntry.nField   = nField ? (theQueryData.nCol1 +
                    static_cast<SCCOL>(nField) - 1) : static_cast<SCCOL>(0);
            rEntry.eOp      = eOp;
        }
    }

    theParam.GetEntry(1).eConnect = (nConnect1 != LISTBOX_ENTRY_NOTFOUND)
                                    ? (ScQueryConnect)nConnect1
                                    : SC_AND;
    theParam.GetEntry(2).eConnect = (nConnect2 != LISTBOX_ENTRY_NOTFOUND)
                                    ? (ScQueryConnect)nConnect2
                                    : SC_AND;

    theParam.bInplace   = false;
    theParam.nDestTab   = 0;    // Woher kommen diese Werte?
    theParam.nDestCol   = 0;
    theParam.nDestRow   = 0;

    theParam.bDuplicate     = !aBtnUnique.IsChecked();
    theParam.bCaseSens      = aBtnCase.IsChecked();
    theParam.bRegExp        = aBtnRegExp.IsChecked();

    if ( pOutItem ) DELETEZ( pOutItem );
    pOutItem = new ScQueryItem( nWhichQuery, &theParam );

    return *pOutItem;
}

//------------------------------------------------------------------------
// Handler:
//------------------------------------------------------------------------

IMPL_LINK( ScPivotFilterDlg, LbSelectHdl, ListBox*, pLb )
{
    /*
     * Behandlung der Enable/Disable-Logik,
     * abhaengig davon, welche ListBox angefasst wurde:
     */

    if ( pLb == &aLbConnect1 )
    {
        if ( !aLbField2.IsEnabled() )
        {
            aLbField2.Enable();
            aLbCond2.Enable();
            aEdVal2.Enable();
        }
    }
    else if ( pLb == &aLbConnect2 )
    {
        if ( !aLbField3.IsEnabled() )
        {
            aLbField3.Enable();
            aLbCond3.Enable();
            aEdVal3.Enable();
        }
    }
    else if ( pLb == &aLbField1 )
    {
        if ( aLbField1.GetSelectEntryPos() == 0 )
        {
            aLbConnect1.SetNoSelection();
            aLbConnect2.SetNoSelection();
            aLbField2.SelectEntryPos( 0 );
            aLbField3.SelectEntryPos( 0 );
            aLbCond2.SelectEntryPos( 0 );
            aLbCond3.SelectEntryPos( 0 );
            ClearValueList( 1 );
            ClearValueList( 2 );
            ClearValueList( 3 );

            aLbConnect1.Disable();
            aLbConnect2.Disable();
            aLbField2.Disable();
            aLbField3.Disable();
            aLbCond2.Disable();
            aLbCond3.Disable();
            aEdVal2.Disable();
            aEdVal3.Disable();
        }
        else
        {
            UpdateValueList( 1 );
            if ( !aLbConnect1.IsEnabled() )
            {
                aLbConnect1.Enable();
            }
        }
    }
    else if ( pLb == &aLbField2 )
    {
        if ( aLbField2.GetSelectEntryPos() == 0 )
        {
            aLbConnect2.SetNoSelection();
            aLbField3.SelectEntryPos( 0 );
            aLbCond3.SelectEntryPos( 0 );
            ClearValueList( 2 );
            ClearValueList( 3 );

            aLbConnect2.Disable();
            aLbField3.Disable();
            aLbCond3.Disable();
            aEdVal3.Disable();
        }
        else
        {
            UpdateValueList( 2 );
            if ( !aLbConnect2.IsEnabled() )
            {
                aLbConnect2.Enable();
            }
        }
    }
    else if ( pLb == &aLbField3 )
    {
        ( aLbField3.GetSelectEntryPos() == 0 )
            ? ClearValueList( 3 )
            : UpdateValueList( 3 );
    }

    return 0;
}

//----------------------------------------------------------------------------

IMPL_LINK( ScPivotFilterDlg, CheckBoxHdl, CheckBox*, pBox )
{
    //  bei Gross-/Kleinschreibung die Werte-Listen aktualisieren

    if ( pBox == &aBtnCase )                    // Wertlisten
    {
        for (sal_uInt16 i=0; i<=MAXCOL; i++)
            DELETEZ( pEntryLists[i] );

        OUString aCurVal1 = aEdVal1.GetText();
        OUString aCurVal2 = aEdVal2.GetText();
        OUString aCurVal3 = aEdVal3.GetText();
        UpdateValueList( 1 );
        UpdateValueList( 2 );
        UpdateValueList( 3 );
        aEdVal1.SetText( aCurVal1 );
        aEdVal2.SetText( aCurVal2 );
        aEdVal3.SetText( aCurVal3 );
    }

    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( ScPivotFilterDlg, ValModifyHdl, ComboBox*, pEd )
{
    if ( pEd )
    {
        OUString aStrVal = pEd->GetText();
        ListBox* pLb    = &aLbCond1;

             if ( pEd == &aEdVal2 ) pLb = &aLbCond2;
        else if ( pEd == &aEdVal3 ) pLb = &aLbCond3;

        // wenn einer der Sonderwerte leer/nicht-leer
        // gewaehlt wird, so macht nur der =-Operator Sinn:

        if ( aStrEmpty.equals(aStrVal) || aStrNotEmpty.equals(aStrVal) )
        {
            pLb->SelectEntry(OUString('='));
            pLb->Disable();
        }
        else
            pLb->Enable();
    }

    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
