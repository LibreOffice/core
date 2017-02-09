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

#include "pfiltdlg.hxx"
#include <svl/zforlist.hxx>
#include <svl/sharedstringpool.hxx>

ScPivotFilterDlg::ScPivotFilterDlg(vcl::Window* pParent, const SfxItemSet& rArgSet,
    SCTAB nSourceTab )
    :   ModalDialog(pParent, "PivotFilterDialog",
            "modules/scalc/ui/pivotfilterdialog.ui" )
    ,

        aStrNone        ( SC_RESSTR(SCSTR_NONE) ),
        aStrEmpty       ( SC_RESSTR(SCSTR_FILTER_EMPTY) ),
        aStrNotEmpty    ( SC_RESSTR(SCSTR_FILTER_NOTEMPTY) ),
        aStrColumn      ( SC_RESSTR(SCSTR_COLUMN) ),

        nWhichQuery     ( rArgSet.GetPool()->GetWhich( SID_QUERY ) ),
        theQueryData    ( static_cast<const ScQueryItem&>(
                           rArgSet.Get( nWhichQuery )).GetQueryData() ),
        pOutItem        ( nullptr ),
        pViewData       ( nullptr ),
        pDoc            ( nullptr ),
        nSrcTab         ( nSourceTab ),     // is not in QueryParam
        nFieldCount     ( 0 )
{
    get(m_pLbField1, "field1");
    get(m_pLbCond1, "cond1");
    get(m_pEdVal1, "val1");
    get(m_pLbConnect1, "connect1");
    get(m_pLbField2, "field2");
    get(m_pLbCond2, "cond2");
    get(m_pEdVal2, "val2");
    get(m_pLbConnect2, "connect2");
    get(m_pLbField3, "field3");
    get(m_pLbCond3, "cond3");
    get(m_pEdVal3, "val3");
    get(m_pBtnCase, "case");
    get(m_pBtnRegExp, "regexp");
    get(m_pBtnUnique, "unique");
    get(m_pFtDbArea, "dbarea");

    for (sal_uInt16 i=0; i<=MAXCOL; i++)
        pEntryLists[i] = nullptr;

    Init( rArgSet );
}

ScPivotFilterDlg::~ScPivotFilterDlg()
{
    disposeOnce();
}

void ScPivotFilterDlg::dispose()
{
    for (sal_uInt16 i=0; i<=MAXCOL; i++)
        delete pEntryLists[i];

    delete pOutItem;
    m_pLbField1.clear();
    m_pLbCond1.clear();
    m_pEdVal1.clear();
    m_pLbConnect1.clear();
    m_pLbField2.clear();
    m_pLbCond2.clear();
    m_pEdVal2.clear();
    m_pLbConnect2.clear();
    m_pLbField3.clear();
    m_pLbCond3.clear();
    m_pEdVal3.clear();
    m_pBtnCase.clear();
    m_pBtnRegExp.clear();
    m_pBtnUnique.clear();
    m_pFtDbArea.clear();
    for (auto& a : aValueEdArr) a.clear();
    for (auto& a : aFieldLbArr) a.clear();
    for (auto& a : aCondLbArr) a.clear();
    ModalDialog::dispose();
}

void ScPivotFilterDlg::Init( const SfxItemSet& rArgSet )
{
    const ScQueryItem& rQueryItem = static_cast<const ScQueryItem&>(
                                    rArgSet.Get( nWhichQuery ));

    m_pBtnCase->SetClickHdl    ( LINK( this, ScPivotFilterDlg, CheckBoxHdl ) );

    m_pLbField1->SetSelectHdl  ( LINK( this, ScPivotFilterDlg, LbSelectHdl ) );
    m_pLbField2->SetSelectHdl  ( LINK( this, ScPivotFilterDlg, LbSelectHdl ) );
    m_pLbField3->SetSelectHdl  ( LINK( this, ScPivotFilterDlg, LbSelectHdl ) );
    m_pLbConnect1->SetSelectHdl( LINK( this, ScPivotFilterDlg, LbSelectHdl ) );
    m_pLbConnect2->SetSelectHdl( LINK( this, ScPivotFilterDlg, LbSelectHdl ) );

    m_pBtnCase->Check( theQueryData.bCaseSens );
    m_pBtnRegExp->Check( theQueryData.eSearchType == utl::SearchParam::SearchType::Regexp );
    m_pBtnUnique->Check( !theQueryData.bDuplicate );

    pViewData   = rQueryItem.GetViewData();
    pDoc        = pViewData ? pViewData->GetDocument() : nullptr;

    // for easier access:
    aFieldLbArr  [0] = m_pLbField1;
    aFieldLbArr  [1] = m_pLbField2;
    aFieldLbArr  [2] = m_pLbField3;
    aValueEdArr  [0] = m_pEdVal1;
    aValueEdArr  [1] = m_pEdVal2;
    aValueEdArr  [2] = m_pEdVal3;
    aCondLbArr   [0] = m_pLbCond1;
    aCondLbArr   [1] = m_pLbCond2;
    aCondLbArr   [2] = m_pLbCond3;

    if ( pViewData && pDoc )
    {
        ScRange         theCurArea ( ScAddress( theQueryData.nCol1,
                                                theQueryData.nRow1,
                                                nSrcTab ),
                                     ScAddress( theQueryData.nCol2,
                                                theQueryData.nRow2,
                                                nSrcTab ) );
        ScDBCollection* pDBColl     = pDoc->GetDBCollection();
        OUString theDbName = STR_DB_LOCAL_NONAME;

         // Check if the passed range is a database range

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
        aBuf.append(" (");
        aBuf.append(theDbName);
        aBuf.append(')');
        m_pFtDbArea->SetText(aBuf.makeStringAndClear());
    }
    else
    {
        m_pFtDbArea->SetText( EMPTY_OUSTRING );
    }

    // Read the field lists and select the entries:

    FillFieldLists();

    for ( SCSIZE i=0; i<3; i++ )
    {
        if ( theQueryData.GetEntry(i).bDoQuery )
        {
            const ScQueryEntry& rEntry = theQueryData.GetEntry(i);
            const ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
            OUString aValStr = rItem.maString.getString();
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
            aFieldLbArr[i]->SelectEntryPos( 0 ); // "none" selected
            aCondLbArr [i]->SelectEntryPos( 0 ); // "=" selected
            UpdateValueList( static_cast<sal_uInt16>(i) );
            aValueEdArr[i]->SetText( EMPTY_OUSTRING );
        }
        aValueEdArr[i]->SetModifyHdl( LINK( this, ScPivotFilterDlg, ValModifyHdl ) );
    }

    // disable/enable logic:

       (m_pLbField1->GetSelectEntryPos() != 0)
    && (m_pLbField2->GetSelectEntryPos() != 0)
        ? m_pLbConnect1->SelectEntryPos( (sal_uInt16)theQueryData.GetEntry(1).eConnect )
        : m_pLbConnect1->SetNoSelection();

       (m_pLbField2->GetSelectEntryPos() != 0)
    && (m_pLbField3->GetSelectEntryPos() != 0)
        ? m_pLbConnect2->SelectEntryPos( (sal_uInt16)theQueryData.GetEntry(2).eConnect )
        : m_pLbConnect2->SetNoSelection();

    if ( m_pLbField1->GetSelectEntryPos() == 0 )
    {
        m_pLbConnect1->Disable();
        m_pLbField2->Disable();
        m_pLbCond2->Disable();
        m_pEdVal2->Disable();
    }
    else if ( m_pLbConnect1->GetSelectEntryCount() == 0 )
    {
        m_pLbField2->Disable();
        m_pLbCond2->Disable();
        m_pEdVal2->Disable();
    }

    if ( m_pLbField2->GetSelectEntryPos() == 0 )
    {
        m_pLbConnect2->Disable();
        m_pLbField3->Disable();
        m_pLbCond3->Disable();
        m_pEdVal3->Disable();
    }
    else if ( m_pLbConnect2->GetSelectEntryCount() == 0 )
    {
        m_pLbField3->Disable();
        m_pLbCond3->Disable();
        m_pEdVal3->Disable();
    }
}

void ScPivotFilterDlg::FillFieldLists()
{
    m_pLbField1->Clear();
    m_pLbField2->Clear();
    m_pLbField3->Clear();
    m_pLbField1->InsertEntry( aStrNone, 0 );
    m_pLbField2->InsertEntry( aStrNone, 0 );
    m_pLbField3->InsertEntry( aStrNone, 0 );

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
                aFieldName = ScGlobal::ReplaceOrAppend( aStrColumn, "%1", ScColToAlpha( col ));
            }
            m_pLbField1->InsertEntry( aFieldName, i );
            m_pLbField2->InsertEntry( aFieldName, i );
            m_pLbField3->InsertEntry( aFieldName, i );
            i++;
        }
        nFieldCount = i;
    }
}

void ScPivotFilterDlg::UpdateValueList( sal_uInt16 nList )
{
    if ( pDoc && nList>0 && nList<=3 )
    {
        ComboBox*   pValList        = aValueEdArr[nList-1].get();
        sal_Int32   nFieldSelPos    = aFieldLbArr[nList-1]->GetSelectEntryPos();
        sal_Int32   nListPos        = 0;
        OUString    aCurValue       = pValList->GetText();

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
                bool bCaseSens = m_pBtnCase->IsChecked();
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

void ScPivotFilterDlg::ClearValueList( sal_uInt16 nList )
{
    if ( nList>0 && nList<=3 )
    {
        ComboBox* pValList = aValueEdArr[nList-1].get();
        pValList->Clear();
        pValList->InsertEntry( aStrNotEmpty, 0 );
        pValList->InsertEntry( aStrEmpty, 1 );
        pValList->SetText( EMPTY_OUSTRING );
    }
}

sal_uInt16 ScPivotFilterDlg::GetFieldSelPos( SCCOL nField )
{
    if ( nField >= theQueryData.nCol1 && nField <= theQueryData.nCol2 )
        return static_cast<sal_uInt16>(nField - theQueryData.nCol1 + 1);
    else
        return 0;
}

const ScQueryItem& ScPivotFilterDlg::GetOutputItem()
{
    ScQueryParam    theParam( theQueryData );
    sal_Int32          nConnect1 = m_pLbConnect1->GetSelectEntryPos();
    sal_Int32          nConnect2 = m_pLbConnect2->GetSelectEntryPos();

    svl::SharedStringPool& rPool = pViewData->GetDocument()->GetSharedStringPool();

    for ( SCSIZE i=0; i<3; i++ )
    {
        const sal_Int32 nField = aFieldLbArr[i]->GetSelectEntryPos();
        ScQueryOp   eOp     = (ScQueryOp)aCondLbArr[i]->GetSelectEntryPos();

        bool bDoThis = (aFieldLbArr[i]->GetSelectEntryPos() != 0);
        theParam.GetEntry(i).bDoQuery = bDoThis;

        if ( bDoThis )
        {
            ScQueryEntry& rEntry = theParam.GetEntry(i);
            ScQueryEntry::Item& rItem = rEntry.GetQueryItem();

            OUString aStrVal = aValueEdArr[i]->GetText();

            /*
             * The dialog returns the specifc field values "empty"/"non empty"
             * as constant in nVal in connection with the bQueryByString switch
             * set to false
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
                rItem.maString = rPool.intern(aStrVal);
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
    theParam.nDestTab   = 0;    // Where do those values come from?
    theParam.nDestCol   = 0;
    theParam.nDestRow   = 0;

    theParam.bDuplicate     = !m_pBtnUnique->IsChecked();
    theParam.bCaseSens      = m_pBtnCase->IsChecked();
    theParam.eSearchType    = m_pBtnRegExp->IsChecked() ? utl::SearchParam::SearchType::Regexp : utl::SearchParam::SearchType::Normal;

    if ( pOutItem ) DELETEZ( pOutItem );
    pOutItem = new ScQueryItem( nWhichQuery, &theParam );

    return *pOutItem;
}

// Handler:

IMPL_LINK( ScPivotFilterDlg, LbSelectHdl, ListBox&, rLb, void )
{

    /*
     * Handling the enable/disable logic based on which ListBox was touched:
     */

    if (&rLb == m_pLbConnect1)
    {
        if ( !m_pLbField2->IsEnabled() )
        {
            m_pLbField2->Enable();
            m_pLbCond2->Enable();
            m_pEdVal2->Enable();
        }
    }
    else if (&rLb == m_pLbConnect2)
    {
        if ( !m_pLbField3->IsEnabled() )
        {
            m_pLbField3->Enable();
            m_pLbCond3->Enable();
            m_pEdVal3->Enable();
        }
    }
    else if (&rLb == m_pLbField1)
    {
        if ( m_pLbField1->GetSelectEntryPos() == 0 )
        {
            m_pLbConnect1->SetNoSelection();
            m_pLbConnect2->SetNoSelection();
            m_pLbField2->SelectEntryPos( 0 );
            m_pLbField3->SelectEntryPos( 0 );
            m_pLbCond2->SelectEntryPos( 0 );
            m_pLbCond3->SelectEntryPos( 0 );
            ClearValueList( 1 );
            ClearValueList( 2 );
            ClearValueList( 3 );

            m_pLbConnect1->Disable();
            m_pLbConnect2->Disable();
            m_pLbField2->Disable();
            m_pLbField3->Disable();
            m_pLbCond2->Disable();
            m_pLbCond3->Disable();
            m_pEdVal2->Disable();
            m_pEdVal3->Disable();
        }
        else
        {
            UpdateValueList( 1 );
            if ( !m_pLbConnect1->IsEnabled() )
            {
                m_pLbConnect1->Enable();
            }
        }
    }
    else if (&rLb == m_pLbField2)
    {
        if ( m_pLbField2->GetSelectEntryPos() == 0 )
        {
            m_pLbConnect2->SetNoSelection();
            m_pLbField3->SelectEntryPos( 0 );
            m_pLbCond3->SelectEntryPos( 0 );
            ClearValueList( 2 );
            ClearValueList( 3 );

            m_pLbConnect2->Disable();
            m_pLbField3->Disable();
            m_pLbCond3->Disable();
            m_pEdVal3->Disable();
        }
        else
        {
            UpdateValueList( 2 );
            if ( !m_pLbConnect2->IsEnabled() )
            {
                m_pLbConnect2->Enable();
            }
        }
    }
    else if (&rLb == m_pLbField3)
    {
        ( m_pLbField3->GetSelectEntryPos() == 0 )
            ? ClearValueList( 3 )
            : UpdateValueList( 3 );
    }
}

IMPL_LINK( ScPivotFilterDlg, CheckBoxHdl, Button*, pBox, void )
{
    // update the value lists when dealing with uppercase/lowercase

    if (pBox == m_pBtnCase)                    // value lists
    {
        for (sal_uInt16 i=0; i<=MAXCOL; i++)
            DELETEZ( pEntryLists[i] );

        OUString aCurVal1 = m_pEdVal1->GetText();
        OUString aCurVal2 = m_pEdVal2->GetText();
        OUString aCurVal3 = m_pEdVal3->GetText();
        UpdateValueList( 1 );
        UpdateValueList( 2 );
        UpdateValueList( 3 );
        m_pEdVal1->SetText( aCurVal1 );
        m_pEdVal2->SetText( aCurVal2 );
        m_pEdVal3->SetText( aCurVal3 );
    }
}

IMPL_LINK( ScPivotFilterDlg, ValModifyHdl, Edit&, rEd, void )
{
    OUString aStrVal = rEd.GetText();
    ListBox* pLb = m_pLbCond1;

    if ( &rEd == m_pEdVal2 ) pLb = m_pLbCond2;
    else if ( &rEd == m_pEdVal3 ) pLb = m_pLbCond3;

    // if ond of the special values "empty"/"non-empty" was chosen only the
    // =-operand makes sense:

    if ( aStrEmpty.equals(aStrVal) || aStrNotEmpty.equals(aStrVal) )
    {
        pLb->SelectEntry(OUString('='));
        pLb->Disable();
    }
    else
        pLb->Enable();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
