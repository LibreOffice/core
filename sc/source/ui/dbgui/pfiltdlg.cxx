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

#include <viewdata.hxx>
#include <document.hxx>
#include <uiitems.hxx>
#include <global.hxx>
#include <globalnames.hxx>
#include <dbdata.hxx>
#include <scresid.hxx>
#include <queryentry.hxx>
#include <filterentries.hxx>

#include <sc.hrc>
#include <strings.hrc>

#include <pfiltdlg.hxx>
#include <svl/sharedstringpool.hxx>
#include <osl/diagnose.h>

ScPivotFilterDlg::ScPivotFilterDlg(weld::Window* pParent, const SfxItemSet& rArgSet,
                                   SCTAB nSourceTab )
    : GenericDialogController(pParent, "modules/scalc/ui/pivotfilterdialog.ui", "PivotFilterDialog")
    , aStrNone(ScResId(SCSTR_NONE))
    , aStrEmpty(ScResId(SCSTR_FILTER_EMPTY))
    , aStrNotEmpty(ScResId(SCSTR_FILTER_NOTEMPTY))
    , aStrColumn(ScResId(SCSTR_COLUMN))
    , nWhichQuery(rArgSet.GetPool()->GetWhich(SID_QUERY))
    , theQueryData(static_cast<const ScQueryItem&>(rArgSet.Get(nWhichQuery)).GetQueryData())
    , pViewData(nullptr)
    , pDoc(nullptr)
    , nSrcTab(nSourceTab)     // is not in QueryParam
    , m_xLbField1(m_xBuilder->weld_combo_box("field1"))
    , m_xLbCond1(m_xBuilder->weld_combo_box("cond1"))
    , m_xEdVal1(m_xBuilder->weld_combo_box("val1"))
    , m_xLbConnect1(m_xBuilder->weld_combo_box("connect1"))
    , m_xLbField2(m_xBuilder->weld_combo_box("field2"))
    , m_xLbCond2(m_xBuilder->weld_combo_box("cond2"))
    , m_xEdVal2(m_xBuilder->weld_combo_box("val2"))
    , m_xLbConnect2(m_xBuilder->weld_combo_box("connect2"))
    , m_xLbField3(m_xBuilder->weld_combo_box("field3"))
    , m_xLbCond3(m_xBuilder->weld_combo_box("cond3"))
    , m_xEdVal3(m_xBuilder->weld_combo_box("val3"))
    , m_xBtnCase(m_xBuilder->weld_check_button("case"))
    , m_xBtnRegExp(m_xBuilder->weld_check_button("regexp"))
    , m_xBtnUnique(m_xBuilder->weld_check_button("unique"))
    , m_xFtDbArea(m_xBuilder->weld_label("dbarea"))
{
    Init( rArgSet );
}

ScPivotFilterDlg::~ScPivotFilterDlg()
{
}

void ScPivotFilterDlg::Init( const SfxItemSet& rArgSet )
{
    const ScQueryItem& rQueryItem = static_cast<const ScQueryItem&>(
                                    rArgSet.Get( nWhichQuery ));

    m_xBtnCase->connect_clicked    ( LINK( this, ScPivotFilterDlg, CheckBoxHdl ) );

    m_xLbField1->connect_changed  ( LINK( this, ScPivotFilterDlg, LbSelectHdl ) );
    m_xLbField2->connect_changed  ( LINK( this, ScPivotFilterDlg, LbSelectHdl ) );
    m_xLbField3->connect_changed  ( LINK( this, ScPivotFilterDlg, LbSelectHdl ) );
    m_xLbConnect1->connect_changed( LINK( this, ScPivotFilterDlg, LbSelectHdl ) );
    m_xLbConnect2->connect_changed( LINK( this, ScPivotFilterDlg, LbSelectHdl ) );

    m_xBtnCase->set_active( theQueryData.bCaseSens );
    m_xBtnRegExp->set_active( theQueryData.eSearchType == utl::SearchParam::SearchType::Regexp );
    m_xBtnUnique->set_active( !theQueryData.bDuplicate );

    pViewData   = rQueryItem.GetViewData();
    pDoc        = pViewData ? &pViewData->GetDocument() : nullptr;

    // for easier access:
    aFieldLbArr  [0] = m_xLbField1.get();
    aFieldLbArr  [1] = m_xLbField2.get();
    aFieldLbArr  [2] = m_xLbField3.get();
    aValueEdArr  [0] = m_xEdVal1.get();
    aValueEdArr  [1] = m_xEdVal2.get();
    aValueEdArr  [2] = m_xEdVal3.get();
    aCondLbArr   [0] = m_xLbCond1.get();
    aCondLbArr   [1] = m_xLbCond2.get();
    aCondLbArr   [2] = m_xLbCond3.get();

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

        OUString sLabel = " (" + theDbName + ")";
        m_xFtDbArea->set_label(sLabel);
    }
    else
    {
        m_xFtDbArea->set_label(EMPTY_OUSTRING);
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
            sal_uInt16  nCondPos     = static_cast<sal_uInt16>(rEntry.eOp);
            sal_uInt16  nFieldSelPos = GetFieldSelPos( static_cast<SCCOL>(rEntry.nField) );

            aFieldLbArr[i]->set_active( nFieldSelPos );
            aCondLbArr [i]->set_active( nCondPos );
            UpdateValueList( static_cast<sal_uInt16>(i+1) );
            aValueEdArr[i]->set_entry_text(aValStr);
            if (aValStr == aStrEmpty || aValStr == aStrNotEmpty)
                aCondLbArr[i]->set_sensitive(false);
        }
        else
        {
            aFieldLbArr[i]->set_active( 0 ); // "none" selected
            aCondLbArr [i]->set_active( 0 ); // "=" selected
            UpdateValueList( static_cast<sal_uInt16>(i) );
            aValueEdArr[i]->set_entry_text(EMPTY_OUSTRING);
        }
        aValueEdArr[i]->connect_changed( LINK( this, ScPivotFilterDlg, ValModifyHdl ) );
    }

    // disable/enable logic:

    if (m_xLbField1->get_active() != 0 && m_xLbField2->get_active() != 0)
        m_xLbConnect1->set_active( static_cast<sal_uInt16>(theQueryData.GetEntry(1).eConnect) );
    else
        m_xLbConnect1->set_active(-1);

    if (m_xLbField2->get_active() != 0 && m_xLbField3->get_active() != 0)
        m_xLbConnect2->set_active( static_cast<sal_uInt16>(theQueryData.GetEntry(2).eConnect) );
    else
        m_xLbConnect2->set_active(-1);

    if (m_xLbField1->get_active() == 0)
    {
        m_xLbConnect1->set_sensitive(false);
        m_xLbField2->set_sensitive(false);
        m_xLbCond2->set_sensitive(false);
        m_xEdVal2->set_sensitive(false);
    }
    else if (m_xLbConnect1->get_active() == -1)
    {
        m_xLbField2->set_sensitive(false);
        m_xLbCond2->set_sensitive(false);
        m_xEdVal2->set_sensitive(false);
    }

    if (m_xLbField2->get_active() == 0)
    {
        m_xLbConnect2->set_sensitive(false);
        m_xLbField3->set_sensitive(false);
        m_xLbCond3->set_sensitive(false);
        m_xEdVal3->set_sensitive(false);
    }
    else if (m_xLbConnect2->get_active() == -1)
    {
        m_xLbField3->set_sensitive(false);
        m_xLbCond3->set_sensitive(false);
        m_xEdVal3->set_sensitive(false);
    }
}

void ScPivotFilterDlg::FillFieldLists()
{
    m_xLbField1->clear();
    m_xLbField2->clear();
    m_xLbField3->clear();
    m_xLbField1->append_text(aStrNone);
    m_xLbField2->append_text(aStrNone);
    m_xLbField3->append_text(aStrNone);

    if ( !pDoc )
        return;

    OUString  aFieldName;
    SCTAB   nTab        = nSrcTab;
    SCCOL   nFirstCol   = theQueryData.nCol1;
    SCROW   nFirstRow   = theQueryData.nRow1;
    SCCOL   nMaxCol     = theQueryData.nCol2;
    SCCOL   col = 0;

    for ( col=nFirstCol; col<=nMaxCol; col++ )
    {
        aFieldName = pDoc->GetString(col, nFirstRow, nTab);
        if ( aFieldName.isEmpty() )
        {
            aFieldName = ScGlobal::ReplaceOrAppend( aStrColumn, "%1", ScColToAlpha( col ));
        }
        m_xLbField1->append_text(aFieldName);
        m_xLbField2->append_text(aFieldName);
        m_xLbField3->append_text(aFieldName);
    }
}

void ScPivotFilterDlg::UpdateValueList( sal_uInt16 nList )
{
    if ( !(pDoc && nList>0 && nList<=3) )
        return;

    weld::ComboBox* pValList        = aValueEdArr[nList-1];
    sal_Int32   nFieldSelPos    = aFieldLbArr[nList-1]->get_active();
    OUString    aCurValue       = pValList->get_active_text();

    pValList->clear();
    pValList->append_text(aStrNotEmpty);
    pValList->append_text(aStrEmpty);

    if ( pDoc && nFieldSelPos )
    {
        SCCOL nColumn = theQueryData.nCol1 + static_cast<SCCOL>(nFieldSelPos) - 1;
        if (!m_pEntryLists[nColumn])
        {
            weld::WaitObject aWaiter(m_xDialog.get());

            SCTAB   nTab        = nSrcTab;
            SCROW   nFirstRow   = theQueryData.nRow1;
            SCROW   nLastRow    = theQueryData.nRow2;
            nFirstRow++;
            bool bCaseSens = m_xBtnCase->get_active();
            m_pEntryLists[nColumn].reset( new ScFilterEntries);
            pDoc->GetFilterEntriesArea(
                nColumn, nFirstRow, nLastRow, nTab, bCaseSens, *m_pEntryLists[nColumn]);
        }

        const ScFilterEntries* pColl = m_pEntryLists[nColumn].get();
        for (const auto& rEntry : *pColl)
        {
            pValList->append_text(rEntry.GetString());
        }
    }
    pValList->set_entry_text(aCurValue);
}

void ScPivotFilterDlg::ClearValueList( sal_uInt16 nList )
{
    if ( nList>0 && nList<=3 )
    {
        weld::ComboBox* pValList = aValueEdArr[nList-1];
        pValList->clear();
        pValList->append_text(aStrNotEmpty);
        pValList->append_text(aStrEmpty);
        pValList->set_entry_text(EMPTY_OUSTRING);
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
    sal_Int32          nConnect1 = m_xLbConnect1->get_active();
    sal_Int32          nConnect2 = m_xLbConnect2->get_active();

    svl::SharedStringPool& rPool = pViewData->GetDocument().GetSharedStringPool();

    for ( SCSIZE i=0; i<3; i++ )
    {
        const sal_Int32 nField = aFieldLbArr[i]->get_active();
        ScQueryOp   eOp     = static_cast<ScQueryOp>(aCondLbArr[i]->get_active());

        bool bDoThis = (aFieldLbArr[i]->get_active() != 0);
        theParam.GetEntry(i).bDoQuery = bDoThis;

        if ( bDoThis )
        {
            ScQueryEntry& rEntry = theParam.GetEntry(i);
            ScQueryEntry::Item& rItem = rEntry.GetQueryItem();

            OUString aStrVal = aValueEdArr[i]->get_active_text();

            /*
             * The dialog returns the specific field values "empty"/"non empty"
             * as constant in nVal in connection with the bQueryByString switch
             * set to false
             */
            if ( aStrVal == aStrEmpty )
            {
                OSL_ASSERT(eOp == SC_EQUAL);
                rEntry.SetQueryByEmpty();
            }
            else if ( aStrVal == aStrNotEmpty )
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

    theParam.GetEntry(1).eConnect = (nConnect1 != -1)
                                    ? static_cast<ScQueryConnect>(nConnect1)
                                    : SC_AND;
    theParam.GetEntry(2).eConnect = (nConnect2 != -1)
                                    ? static_cast<ScQueryConnect>(nConnect2)
                                    : SC_AND;

    theParam.bInplace   = false;
    theParam.nDestTab   = 0;    // Where do those values come from?
    theParam.nDestCol   = 0;
    theParam.nDestRow   = 0;

    theParam.bDuplicate     = !m_xBtnUnique->get_active();
    theParam.bCaseSens      = m_xBtnCase->get_active();
    theParam.eSearchType    = m_xBtnRegExp->get_active() ? utl::SearchParam::SearchType::Regexp : utl::SearchParam::SearchType::Normal;

    pOutItem.reset( new ScQueryItem( nWhichQuery, &theParam ) );

    return *pOutItem;
}

// Handler:

IMPL_LINK( ScPivotFilterDlg, LbSelectHdl, weld::ComboBox&, rLb, void )
{
    /*
     * Handling the enable/disable logic based on which ListBox was touched:
     */
    if (&rLb == m_xLbConnect1.get())
    {
        if ( !m_xLbField2->get_sensitive() )
        {
            m_xLbField2->set_sensitive(true);
            m_xLbCond2->set_sensitive(true);
            m_xEdVal2->set_sensitive(true);
        }
    }
    else if (&rLb == m_xLbConnect2.get())
    {
        if ( !m_xLbField3->get_sensitive() )
        {
            m_xLbField3->set_sensitive(true);
            m_xLbCond3->set_sensitive(true);
            m_xEdVal3->set_sensitive(true);
        }
    }
    else if (&rLb == m_xLbField1.get())
    {
        if ( m_xLbField1->get_active() == 0 )
        {
            m_xLbConnect1->set_active(-1);
            m_xLbConnect2->set_active(-1);
            m_xLbField2->set_active( 0 );
            m_xLbField3->set_active( 0 );
            m_xLbCond2->set_active( 0 );
            m_xLbCond3->set_active( 0 );
            ClearValueList( 1 );
            ClearValueList( 2 );
            ClearValueList( 3 );

            m_xLbConnect1->set_sensitive(false);
            m_xLbConnect2->set_sensitive(false);
            m_xLbField2->set_sensitive(false);
            m_xLbField3->set_sensitive(false);
            m_xLbCond2->set_sensitive(false);
            m_xLbCond3->set_sensitive(false);
            m_xEdVal2->set_sensitive(false);
            m_xEdVal3->set_sensitive(false);
        }
        else
        {
            UpdateValueList( 1 );
            if ( !m_xLbConnect1->get_sensitive() )
            {
                m_xLbConnect1->set_sensitive(true);
            }
        }
    }
    else if (&rLb == m_xLbField2.get())
    {
        if ( m_xLbField2->get_active() == 0 )
        {
            m_xLbConnect2->set_active(-1);
            m_xLbField3->set_active( 0 );
            m_xLbCond3->set_active( 0 );
            ClearValueList( 2 );
            ClearValueList( 3 );

            m_xLbConnect2->set_sensitive(false);
            m_xLbField3->set_sensitive(false);
            m_xLbCond3->set_sensitive(false);
            m_xEdVal3->set_sensitive(false);
        }
        else
        {
            UpdateValueList( 2 );
            if (!m_xLbConnect2->get_sensitive())
            {
                m_xLbConnect2->set_sensitive(true);
            }
        }
    }
    else if (&rLb == m_xLbField3.get())
    {
        if (m_xLbField3->get_active() == 0)
            ClearValueList(3);
        else
            UpdateValueList(3);
    }
}

IMPL_LINK(ScPivotFilterDlg, CheckBoxHdl, weld::Button&, rBox, void)
{
    // update the value lists when dealing with uppercase/lowercase

    if (&rBox != m_xBtnCase.get())                    // value lists
        return;

    for (auto& a : m_pEntryLists)
        a.reset();

    OUString aCurVal1 = m_xEdVal1->get_active_text();
    OUString aCurVal2 = m_xEdVal2->get_active_text();
    OUString aCurVal3 = m_xEdVal3->get_active_text();
    UpdateValueList( 1 );
    UpdateValueList( 2 );
    UpdateValueList( 3 );
    m_xEdVal1->set_entry_text(aCurVal1);
    m_xEdVal2->set_entry_text(aCurVal2);
    m_xEdVal3->set_entry_text(aCurVal3);
}

IMPL_LINK( ScPivotFilterDlg, ValModifyHdl, weld::ComboBox&, rEd, void )
{
    OUString aStrVal = rEd.get_active_text();
    weld::ComboBox* pLb = m_xLbCond1.get();

    if ( &rEd == m_xEdVal2.get() ) pLb = m_xLbCond2.get();
    else if ( &rEd == m_xEdVal3.get() ) pLb = m_xLbCond3.get();

    // if cond of the special values "empty"/"non-empty" was chosen only the
    // =-operand makes sense:

    if ( aStrEmpty == aStrVal || aStrNotEmpty == aStrVal )
    {
        pLb->set_active_text(OUString('='));
        pLb->set_sensitive(false);
    }
    else
        pLb->set_sensitive(true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
