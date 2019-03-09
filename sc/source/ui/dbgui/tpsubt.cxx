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

#include <scitems.hxx>
#include <uiitems.hxx>
#include <global.hxx>
#include <userlist.hxx>
#include <viewdata.hxx>
#include <document.hxx>
#include <scresid.hxx>
#include <sc.hrc>
#include <strings.hrc>
#include <subtotals.hrc>

#include <tpsubt.hxx>
#include <memory>

// Subtotals group tabpage:

ScTpSubTotalGroup::ScTpSubTotalGroup(TabPageParent pParent, const SfxItemSet& rArgSet)
    : SfxTabPage(pParent, "modules/scalc/ui/subtotalgrppage.ui", "SubTotalGrpPage", &rArgSet)
    , aStrNone(ScResId(SCSTR_NONE))
    , aStrColumn(ScResId(SCSTR_COLUMN))
    , pViewData(nullptr)
    , pDoc(nullptr)
    , nWhichSubTotals(rArgSet.GetPool()->GetWhich(SID_SUBTOTALS))
    , rSubTotalData(static_cast<const ScSubTotalItem&>(rArgSet.Get(nWhichSubTotals)).GetSubTotalData())
    , nFieldCount(0)
    , mxLbGroup(m_xBuilder->weld_combo_box("group_by"))
    , mxLbColumns(m_xBuilder->weld_tree_view("columns"))
    , mxLbFunctions(m_xBuilder->weld_tree_view("functions"))
{
    for (size_t i = 0; i < SAL_N_ELEMENTS(SCSTR_SUBTOTALS); ++i)
        mxLbFunctions->append_text(ScResId(SCSTR_SUBTOTALS[i]));

    auto nHeight = mxLbColumns->get_height_rows(14);
    mxLbColumns->set_size_request(-1, nHeight);
    mxLbFunctions->set_size_request(-1, nHeight);

    std::vector<int> aWidths;
    aWidths.push_back(mxLbColumns->get_checkbox_column_width());
    mxLbColumns->set_column_fixed_widths(aWidths);

    Init();
}

ScTpSubTotalGroup::~ScTpSubTotalGroup()
{
    disposeOnce();
}

void ScTpSubTotalGroup::Init()
{
    const ScSubTotalItem& rSubTotalItem = static_cast<const ScSubTotalItem&>(
                                          GetItemSet().Get( nWhichSubTotals ));

    pViewData   = rSubTotalItem.GetViewData();
    pDoc        = pViewData ? pViewData->GetDocument() : nullptr;

    OSL_ENSURE( pViewData && pDoc, "ViewData or Document not found :-(" );

    mxLbGroup->connect_changed( LINK( this, ScTpSubTotalGroup, SelectListBoxHdl ) );
    mxLbColumns->connect_changed( LINK( this, ScTpSubTotalGroup, SelectTreeListBoxHdl ) );
    mxLbColumns->connect_toggled( LINK( this, ScTpSubTotalGroup, CheckHdl ) );
    mxLbFunctions->connect_changed( LINK( this, ScTpSubTotalGroup, SelectTreeListBoxHdl) );

    nFieldArr[0] = 0;
    FillListBoxes();
}

bool ScTpSubTotalGroup::DoReset( sal_uInt16             nGroupNo,
                                 const SfxItemSet&  rArgSet  )
{
    sal_uInt16 nGroupIdx = 0;

    OSL_ENSURE( (nGroupNo<=3) && (nGroupNo>0), "Invalid group" );

    if ( (nGroupNo > 3) || (nGroupNo == 0) )
        return false;
    else
        nGroupIdx = nGroupNo-1;

    // first we have to clear the listboxes...
    for (int nLbEntry = 0, nCount = mxLbColumns->n_children(); nLbEntry < nCount; ++nLbEntry)
    {
        mxLbColumns->set_toggle(nLbEntry, false, 0);
        mxLbColumns->set_id(nLbEntry, "0");
    }
    mxLbFunctions->select(0);

    ScSubTotalParam theSubTotalData( static_cast<const ScSubTotalItem&>(
                                      rArgSet.Get( nWhichSubTotals )).
                                            GetSubTotalData() );

    if ( theSubTotalData.bGroupActive[nGroupIdx] )
    {
        SCCOL           nField      = theSubTotalData.nField[nGroupIdx];
        SCCOL           nSubTotals  = theSubTotalData.nSubTotals[nGroupIdx];
        SCCOL*          pSubTotals  = theSubTotalData.pSubTotals[nGroupIdx];
        ScSubTotalFunc* pFunctions  = theSubTotalData.pFunctions[nGroupIdx];

        mxLbGroup->set_active( GetFieldSelPos( nField )+1 );

        sal_uInt16 nFirstChecked = 0;
        for ( sal_uInt16 i=0; i<nSubTotals; i++ )
        {
            sal_uInt16  nCheckPos = GetFieldSelPos( pSubTotals[i] );

            mxLbColumns->set_toggle(nCheckPos, true, 0);
            mxLbColumns->set_id(nCheckPos, OUString::number(FuncToLbPos(pFunctions[i])));

            if (i == 0 || nCheckPos < nFirstChecked)
                nFirstChecked = nCheckPos;
        }
        // Select the first checked field from the top.
        mxLbColumns->select(nFirstChecked);
    }
    else
    {
        mxLbGroup->set_active( (nGroupNo == 1) ? 1 : 0 );
        mxLbColumns->select( 0 );
        mxLbFunctions->select( 0 );
    }

    return true;
}

namespace
{
    int GetCheckedEntryCount(weld::TreeView& rTreeView)
    {
        int nRet = 0;
        for (sal_Int32 i=0, nEntryCount = rTreeView.n_children(); i < nEntryCount; ++i)
        {
            if (rTreeView.get_toggle(i, 0))
                ++nRet;
        }
        return nRet;
    }
}

bool ScTpSubTotalGroup::DoFillItemSet( sal_uInt16       nGroupNo,
                                       SfxItemSet&  rArgSet  )
{
    sal_uInt16 nGroupIdx = 0;

    OSL_ENSURE( (nGroupNo<=3) && (nGroupNo>0), "Invalid group" );
    OSL_ENSURE(    (mxLbGroup->get_count() > 0)
                && (mxLbColumns->n_children() > 0)
                && (mxLbFunctions->n_children() > 0),
                "Non-initialized Lists" );

    if (  (nGroupNo > 3) || (nGroupNo == 0)
        || (mxLbGroup->get_count() == 0)
        || (mxLbColumns->n_children() == 0)
        || (mxLbFunctions->n_children() == 0)
       )
        return false;
    else
        nGroupIdx = nGroupNo-1;

    ScSubTotalParam theSubTotalData;            // read out, if already partly filled
    const SfxItemSet* pExample = GetDialogExampleSet();
    if (pExample)
    {
        const SfxPoolItem* pItem;
        if (pExample->GetItemState(nWhichSubTotals, true, &pItem) == SfxItemState::SET)
            theSubTotalData = static_cast<const ScSubTotalItem*>(pItem)->GetSubTotalData();
    }

    std::unique_ptr<ScSubTotalFunc[]> pFunctions;
    std::unique_ptr<SCCOL[]>          pSubTotals;
    const sal_Int32 nGroup      = mxLbGroup->get_active();
    const sal_Int32 nEntryCount = mxLbColumns->n_children();
    const sal_Int32 nCheckCount = GetCheckedEntryCount(*mxLbColumns);

    theSubTotalData.nCol1                   = rSubTotalData.nCol1;
    theSubTotalData.nRow1                   = rSubTotalData.nRow1;
    theSubTotalData.nCol2                   = rSubTotalData.nCol2;
    theSubTotalData.nRow2                   = rSubTotalData.nRow2;
    theSubTotalData.bGroupActive[nGroupIdx] = (nGroup != 0);
    theSubTotalData.nField[nGroupIdx]       = (nGroup != 0)
                                                ? nFieldArr[nGroup-1]
                                                : static_cast<SCCOL>(0);

    if ( nEntryCount>0 && nCheckCount>0 && nGroup!=0 )
    {
        sal_uInt16 nFunction    = 0;

        pSubTotals.reset(new SCCOL          [nCheckCount]);
        pFunctions.reset(new ScSubTotalFunc [nCheckCount]);

        for ( sal_Int32 i=0, nCheck=0; i<nEntryCount; i++ )
        {
            if (mxLbColumns->get_toggle(i, 0))
            {
                OSL_ENSURE( nCheck <= nCheckCount,
                            "Range error :-(" );
                nFunction = mxLbColumns->get_id(i).toUInt32();
                pSubTotals[nCheck] = nFieldArr[i];
                pFunctions[nCheck] = LbPosToFunc( nFunction );
                nCheck++;
            }
        }
        theSubTotalData.SetSubTotals( nGroupNo,      // group number
                                      pSubTotals.get(),
                                      pFunctions.get(),
                                      nCheckCount ); // number of array elements

    }

    rArgSet.Put( ScSubTotalItem( SCITEM_SUBTDATA, &theSubTotalData ) );

    return true;
}

void ScTpSubTotalGroup::FillListBoxes()
{
    OSL_ENSURE( pViewData && pDoc, "ViewData or Document not found :-/" );

    if ( pViewData && pDoc )
    {
        SCCOL   nFirstCol   = rSubTotalData.nCol1;
        SCROW   nFirstRow   = rSubTotalData.nRow1;
        SCTAB   nTab        = pViewData->GetTabNo();
        SCCOL   nMaxCol     = rSubTotalData.nCol2;
        SCCOL   col;
        OUString  aFieldName;

        mxLbGroup->clear();
        mxLbColumns->clear();
        mxLbGroup->insert_text(0, aStrNone );

        sal_uInt16 i=0;
        for ( col=nFirstCol; col<=nMaxCol && i<SC_MAXFIELDS; col++ )
        {
            aFieldName = pDoc->GetString(col, nFirstRow, nTab);
            if ( aFieldName.isEmpty() )
            {
                aFieldName = ScGlobal::ReplaceOrAppend( aStrColumn, "%1", ScColToAlpha( col ));
            }
            nFieldArr[i] = col;
            mxLbGroup->insert_text(i+1, aFieldName);
            mxLbColumns->insert(i);
            mxLbColumns->set_toggle(i, false, 0);
            mxLbColumns->set_text(i, aFieldName, 1);
            mxLbColumns->set_id(i, "0");
            i++;
        }
        // subsequent initialization of the constant:
        nFieldCount = i;
    }
}

sal_uInt16 ScTpSubTotalGroup::GetFieldSelPos( SCCOL nField )
{
    sal_uInt16  nFieldPos   = 0;
    bool    bFound      = false;

    for ( sal_uInt16 n=0; n<nFieldCount && !bFound; n++ )
    {
        if ( nFieldArr[n] == nField )
        {
            nFieldPos = n;
            bFound = true;
        }
    }

    return nFieldPos;
}

ScSubTotalFunc ScTpSubTotalGroup::LbPosToFunc( sal_uInt16 nPos )
{
    switch ( nPos )
    {
//      case  0:    return SUBTOTAL_FUNC_NONE;
        case  2:    return SUBTOTAL_FUNC_AVE;
        case  6:    return SUBTOTAL_FUNC_CNT;
        case  1:    return SUBTOTAL_FUNC_CNT2;
        case  3:    return SUBTOTAL_FUNC_MAX;
        case  4:    return SUBTOTAL_FUNC_MIN;
        case  5:    return SUBTOTAL_FUNC_PROD;
        case  7:    return SUBTOTAL_FUNC_STD;
        case  8:    return SUBTOTAL_FUNC_STDP;
        case  0:    return SUBTOTAL_FUNC_SUM;
        case  9:    return SUBTOTAL_FUNC_VAR;
        case 10:    return SUBTOTAL_FUNC_VARP;
        default:
            OSL_FAIL( "ScTpSubTotalGroup::LbPosToFunc" );
            return SUBTOTAL_FUNC_NONE;
    }
}

sal_uInt16 ScTpSubTotalGroup::FuncToLbPos( ScSubTotalFunc eFunc )
{
    switch ( eFunc )
    {
//      case SUBTOTAL_FUNC_NONE:    return 0;
        case SUBTOTAL_FUNC_AVE:     return 2;
        case SUBTOTAL_FUNC_CNT:     return 6;
        case SUBTOTAL_FUNC_CNT2:    return 1;
        case SUBTOTAL_FUNC_MAX:     return 3;
        case SUBTOTAL_FUNC_MIN:     return 4;
        case SUBTOTAL_FUNC_PROD:    return 5;
        case SUBTOTAL_FUNC_STD:     return 7;
        case SUBTOTAL_FUNC_STDP:    return 8;
        case SUBTOTAL_FUNC_SUM:     return 0;
        case SUBTOTAL_FUNC_VAR:     return 9;
        case SUBTOTAL_FUNC_VARP:    return 10;
        default:
            OSL_FAIL( "ScTpSubTotalGroup::FuncToLbPos" );
            return 0;
    }
}

// Handler:

IMPL_LINK(ScTpSubTotalGroup, SelectTreeListBoxHdl, weld::TreeView&, rLb, void)
{
    SelectHdl(&rLb);
}

IMPL_LINK(ScTpSubTotalGroup, SelectListBoxHdl, weld::ComboBox&, rLb, void)
{
    SelectHdl(&rLb);
}

void ScTpSubTotalGroup::SelectHdl(const weld::Widget *pLb)
{
    const sal_Int32 nColumn = mxLbColumns->get_selected_index();
    if (nColumn != -1)
    {
        const sal_Int32 nFunction   = mxLbFunctions->get_selected_index();
        sal_uInt16      nOldFunction  = mxLbColumns->get_id(nColumn).toUInt32();

        if ( pLb == mxLbColumns.get() )
        {
            mxLbFunctions->select(nOldFunction);
        }
        else if ( pLb == mxLbFunctions.get() )
        {
            mxLbColumns->set_id(nColumn, OUString::number(nFunction));
            mxLbColumns->set_toggle(nColumn, true, 0);
        }
    }
}

IMPL_LINK( ScTpSubTotalGroup, CheckHdl, const row_col&, rRowCol, void )
{
    mxLbColumns->select(rRowCol.first);
    SelectHdl(mxLbColumns.get());
}

// Derived Group TabPages:

VclPtr<SfxTabPage> ScTpSubTotalGroup1::Create( TabPageParent pParent,
                                                 const SfxItemSet*  rArgSet )
{
    return VclPtr<ScTpSubTotalGroup1>::Create( pParent, *rArgSet );
}

VclPtr<SfxTabPage> ScTpSubTotalGroup2::Create( TabPageParent pParent,
                                       const SfxItemSet*    rArgSet )
{
    return VclPtr<ScTpSubTotalGroup2>::Create( pParent, *rArgSet );
}

VclPtr<SfxTabPage> ScTpSubTotalGroup3::Create( TabPageParent pParent,
                                       const SfxItemSet*    rArgSet )
{
    return VclPtr<ScTpSubTotalGroup3>::Create( pParent, *rArgSet );
}

ScTpSubTotalGroup1::ScTpSubTotalGroup1( TabPageParent pParent, const SfxItemSet& rArgSet ) :
    ScTpSubTotalGroup( pParent, rArgSet )
{}

ScTpSubTotalGroup2::ScTpSubTotalGroup2( TabPageParent pParent, const SfxItemSet& rArgSet ) :
    ScTpSubTotalGroup( pParent, rArgSet )
{}

ScTpSubTotalGroup3::ScTpSubTotalGroup3( TabPageParent pParent, const SfxItemSet& rArgSet ) :
    ScTpSubTotalGroup( pParent, rArgSet )
{}

#define RESET(i) (ScTpSubTotalGroup::DoReset( (i), *rArgSet ))
void ScTpSubTotalGroup1::Reset( const SfxItemSet* rArgSet ) { RESET(1); }
void ScTpSubTotalGroup2::Reset( const SfxItemSet* rArgSet ) { RESET(2); }
void ScTpSubTotalGroup3::Reset( const SfxItemSet* rArgSet ) { RESET(3); }
#undef RESET

#define FILLSET(i) (ScTpSubTotalGroup::DoFillItemSet( (i), *rArgSet ))
bool ScTpSubTotalGroup1::FillItemSet( SfxItemSet* rArgSet ) { return FILLSET(1); }
bool ScTpSubTotalGroup2::FillItemSet( SfxItemSet* rArgSet ) { return FILLSET(2); }
bool ScTpSubTotalGroup3::FillItemSet( SfxItemSet* rArgSet ) { return FILLSET(3); }
#undef FILL

// options tab page:

ScTpSubTotalOptions::ScTpSubTotalOptions(TabPageParent pParent, const SfxItemSet& rArgSet)

        :   SfxTabPage      ( pParent,
                              "modules/scalc/ui/subtotaloptionspage.ui", "SubTotalOptionsPage",
                              &rArgSet ),
            pViewData       ( nullptr ),
            pDoc            ( nullptr ),
            nWhichSubTotals ( rArgSet.GetPool()->GetWhich( SID_SUBTOTALS ) ),
            rSubTotalData   ( static_cast<const ScSubTotalItem&>(
                              rArgSet.Get( nWhichSubTotals )).
                                GetSubTotalData() )
    , m_xBtnPagebreak(m_xBuilder->weld_check_button("pagebreak"))
    , m_xBtnCase(m_xBuilder->weld_check_button("case"))
    , m_xBtnSort(m_xBuilder->weld_check_button("sort"))
    , m_xFlSort(m_xBuilder->weld_label("label2"))
    , m_xBtnAscending(m_xBuilder->weld_radio_button("ascending"))
    , m_xBtnDescending(m_xBuilder->weld_radio_button("descending"))
    , m_xBtnFormats(m_xBuilder->weld_check_button("formats"))
    , m_xBtnUserDef(m_xBuilder->weld_check_button("btnuserdef"))
    , m_xLbUserDef(m_xBuilder->weld_combo_box("lbuserdef"))
{
    Init();
}

ScTpSubTotalOptions::~ScTpSubTotalOptions()
{
    disposeOnce();
}

void ScTpSubTotalOptions::Init()
{
    const ScSubTotalItem& rSubTotalItem = static_cast<const ScSubTotalItem&>(
                                          GetItemSet().Get( nWhichSubTotals ));

    pViewData   = rSubTotalItem.GetViewData();
    pDoc        = pViewData ? pViewData->GetDocument() : nullptr;

    OSL_ENSURE( pViewData && pDoc, "ViewData or Document not found!" );

    m_xBtnSort->connect_clicked( LINK( this, ScTpSubTotalOptions, CheckHdl ) );
    m_xBtnUserDef->connect_clicked( LINK( this, ScTpSubTotalOptions, CheckHdl ) );

    FillUserSortListBox();
}

VclPtr<SfxTabPage> ScTpSubTotalOptions::Create(TabPageParent pParent,
                                               const SfxItemSet* rArgSet)
{
    return VclPtr<ScTpSubTotalOptions>::Create(pParent, *rArgSet);
}

void ScTpSubTotalOptions::Reset( const SfxItemSet* /* rArgSet */ )
{
    m_xBtnPagebreak->set_active( rSubTotalData.bPagebreak );
    m_xBtnCase->set_active( rSubTotalData.bCaseSens );
    m_xBtnFormats->set_active( rSubTotalData.bIncludePattern );
    m_xBtnSort->set_active( rSubTotalData.bDoSort );
    m_xBtnAscending->set_active( rSubTotalData.bAscending );
    m_xBtnDescending->set_active( !rSubTotalData.bAscending );

    if ( rSubTotalData.bUserDef )
    {
        m_xBtnUserDef->set_active(true);
        m_xLbUserDef->set_sensitive(true);
        m_xLbUserDef->set_active(rSubTotalData.nUserIndex);
    }
    else
    {
        m_xBtnUserDef->set_active( false );
        m_xLbUserDef->set_sensitive(false);
        m_xLbUserDef->set_active(0);
    }

    CheckHdl(*m_xBtnSort);
}

bool ScTpSubTotalOptions::FillItemSet( SfxItemSet* rArgSet )
{
    ScSubTotalParam theSubTotalData;            // read out, if already partly filled
    const SfxItemSet* pExample = GetDialogExampleSet();
    if (pExample)
    {
        const SfxPoolItem* pItem;
        if (pExample->GetItemState(nWhichSubTotals, true, &pItem) == SfxItemState::SET)
            theSubTotalData = static_cast<const ScSubTotalItem*>(pItem)->GetSubTotalData();
    }

    theSubTotalData.bPagebreak      = m_xBtnPagebreak->get_active();
    theSubTotalData.bReplace        = true;
    theSubTotalData.bCaseSens       = m_xBtnCase->get_active();
    theSubTotalData.bIncludePattern = m_xBtnFormats->get_active();
    theSubTotalData.bDoSort         = m_xBtnSort->get_active();
    theSubTotalData.bAscending      = m_xBtnAscending->get_active();
    theSubTotalData.bUserDef        = m_xBtnUserDef->get_active();
    theSubTotalData.nUserIndex      = (m_xBtnUserDef->get_active())
                                    ? m_xLbUserDef->get_active()
                                    : 0;

    rArgSet->Put( ScSubTotalItem( nWhichSubTotals, &theSubTotalData ) );

    return true;
}

void ScTpSubTotalOptions::FillUserSortListBox()
{
    ScUserList* pUserLists = ScGlobal::GetUserList();

    m_xLbUserDef->freeze();
    m_xLbUserDef->clear();
    if ( pUserLists )
    {
        size_t nCount = pUserLists->size();
        for ( size_t i=0; i<nCount; ++i )
            m_xLbUserDef->append_text((*pUserLists)[i].GetString() );
    }
    m_xLbUserDef->thaw();
}

// Handler:

IMPL_LINK(ScTpSubTotalOptions, CheckHdl, weld::Button&, rBox, void)
{
    if (&rBox == m_xBtnSort.get())
    {
        if ( m_xBtnSort->get_active() )
        {
            m_xFlSort->set_sensitive(true);
            m_xBtnFormats->set_sensitive(true);
            m_xBtnUserDef->set_sensitive(true);
            m_xBtnAscending->set_sensitive(true);
            m_xBtnDescending->set_sensitive(true);

            if ( m_xBtnUserDef->get_active() )
                m_xLbUserDef->set_sensitive(true);
        }
        else
        {
            m_xFlSort->set_sensitive(false);
            m_xBtnFormats->set_sensitive(false);
            m_xBtnUserDef->set_sensitive(false);
            m_xBtnAscending->set_sensitive(false);
            m_xBtnDescending->set_sensitive(false);
            m_xLbUserDef->set_sensitive(false);
        }
    }
    else if (&rBox == m_xBtnUserDef.get())
    {
        if ( m_xBtnUserDef->get_active() )
        {
            m_xLbUserDef->set_sensitive(true);
            m_xLbUserDef->grab_focus();
        }
        else
            m_xLbUserDef->set_sensitive(false);
    }
}

ScTpSubTotalGroup1::~ScTpSubTotalGroup1()
{
}

ScTpSubTotalGroup2::~ScTpSubTotalGroup2()
{
}

ScTpSubTotalGroup3::~ScTpSubTotalGroup3()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
