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

#include <tpview.hxx>
#include <global.hxx>
#include <viewopti.hxx>
#include <scresid.hxx>
#include <docsh.hxx>
#include <sc.hrc>
#include <units.hrc>
#include <appoptio.hxx>
#include <scmod.hxx>
#include <svl/eitem.hxx>
#include <svx/colorbox.hxx>
#include <svtools/unitconv.hxx>

ScTpContentOptions::ScTpContentOptions( vcl::Window*         pParent,
                             const SfxItemSet&  rArgSet ) :
    SfxTabPage(pParent, "TpViewPage", "modules/scalc/ui/tpviewpage.ui", &rArgSet)
{
    get(pGridLB,"grid");
    get(pColorFT,"color_label");
    get(pColorLB,"color");
    get(pBreakCB,"break");
    get(pGuideLineCB,"guideline");

    get(pFormulaCB,"formula");
    get(pNilCB,"nil");
    get(pAnnotCB,"annot");
    get(pValueCB,"value");
    get(pAnchorCB,"anchor");
    get(pClipMarkCB,"clipmark");
    get(pRangeFindCB,"rangefind");

    get(pObjGrfLB,"objgrf");
    get(pDiagramLB,"diagram");
    get(pDrawLB,"draw");

    get(pSyncZoomCB,"synczoom");

    get(pRowColHeaderCB,"rowcolheader");
    get(pHScrollCB,"hscroll");
    get(pVScrollCB,"vscroll");
    get(pTblRegCB,"tblreg");
    get(pOutlineCB,"outline");
    get(pSummaryCB,"cbSummary");

    SetExchangeSupport();
    Link<ListBox&,void> aSelObjHdl(LINK( this, ScTpContentOptions, SelLbObjHdl ) );
    pObjGrfLB->  SetSelectHdl(aSelObjHdl);
    pDiagramLB-> SetSelectHdl(aSelObjHdl);
    pDrawLB->    SetSelectHdl(aSelObjHdl);
    pGridLB->    SetSelectHdl( LINK( this, ScTpContentOptions, GridHdl ) );

    Link<Button*, void> aCBHdl(LINK( this, ScTpContentOptions, CBHdl ) );
    pFormulaCB  ->SetClickHdl(aCBHdl);
    pNilCB      ->SetClickHdl(aCBHdl);
    pAnnotCB    ->SetClickHdl(aCBHdl);
    pValueCB    ->SetClickHdl(aCBHdl);
    pAnchorCB   ->SetClickHdl(aCBHdl);
    pClipMarkCB ->SetClickHdl(aCBHdl);

    pVScrollCB  ->SetClickHdl(aCBHdl);
    pHScrollCB  ->SetClickHdl(aCBHdl);
    pTblRegCB   ->SetClickHdl(aCBHdl);
    pOutlineCB  ->SetClickHdl(aCBHdl);
    pBreakCB    ->SetClickHdl(aCBHdl);
    pGuideLineCB->SetClickHdl(aCBHdl);
    pRowColHeaderCB->SetClickHdl(aCBHdl);
    pSummaryCB->SetClickHdl(aCBHdl);

    pColorLB->SetSlotId(SID_ATTR_CHAR_COLOR);
    pColorLB->SetAutoDisplayColor(SC_STD_GRIDCOLOR);
}

ScTpContentOptions::~ScTpContentOptions()
{
    disposeOnce();
}

void ScTpContentOptions::dispose()
{
    pLocalOptions.reset();
    pGridLB.clear();
    pColorFT.clear();
    pColorLB.clear();
    pBreakCB.clear();
    pGuideLineCB.clear();
    pFormulaCB.clear();
    pNilCB.clear();
    pAnnotCB.clear();
    pValueCB.clear();
    pAnchorCB.clear();
    pClipMarkCB.clear();
    pRangeFindCB.clear();
    pObjGrfLB.clear();
    pDiagramLB.clear();
    pDrawLB.clear();
    pSyncZoomCB.clear();
    pRowColHeaderCB.clear();
    pHScrollCB.clear();
    pVScrollCB.clear();
    pTblRegCB.clear();
    pOutlineCB.clear();
    pSummaryCB.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> ScTpContentOptions::Create( TabPageParent pParent,
                                               const SfxItemSet*     rCoreSet )
{
    return VclPtr<ScTpContentOptions>::Create(pParent.pParent, *rCoreSet);
}

bool    ScTpContentOptions::FillItemSet( SfxItemSet* rCoreSet )
{
    bool bRet = false;
    if( pFormulaCB ->IsValueChangedFromSaved() ||
        pNilCB     ->IsValueChangedFromSaved() ||
        pAnnotCB   ->IsValueChangedFromSaved() ||
        pValueCB   ->IsValueChangedFromSaved() ||
        pAnchorCB  ->IsValueChangedFromSaved() ||
        pClipMarkCB->IsValueChangedFromSaved() ||
        pObjGrfLB  ->IsValueChangedFromSaved() ||
        pDiagramLB ->IsValueChangedFromSaved() ||
        pDrawLB    ->IsValueChangedFromSaved() ||
        pGridLB        ->IsValueChangedFromSaved() ||
        pRowColHeaderCB->IsValueChangedFromSaved() ||
        pHScrollCB     ->IsValueChangedFromSaved() ||
        pVScrollCB     ->IsValueChangedFromSaved() ||
        pTblRegCB      ->IsValueChangedFromSaved() ||
        pOutlineCB     ->IsValueChangedFromSaved() ||
        pColorLB       ->IsValueChangedFromSaved() ||
        pBreakCB       ->IsValueChangedFromSaved() ||
        pSummaryCB     ->IsValueChangedFromSaved() ||
        pGuideLineCB   ->IsValueChangedFromSaved())
    {
        NamedColor aNamedColor = pColorLB->GetSelectedEntry();
        if (aNamedColor.first == COL_AUTO)
        {
            aNamedColor.first = SC_STD_GRIDCOLOR;
            aNamedColor.second.clear();
        }
        pLocalOptions->SetGridColor(aNamedColor.first, aNamedColor.second);
        rCoreSet->Put(ScTpViewItem(*pLocalOptions));
        bRet = true;
    }
    if(pRangeFindCB->IsValueChangedFromSaved())
    {
        rCoreSet->Put(SfxBoolItem(SID_SC_INPUT_RANGEFINDER, pRangeFindCB->IsChecked()));
        bRet = true;
    }
    if(pSyncZoomCB->IsValueChangedFromSaved())
    {
        rCoreSet->Put(SfxBoolItem(SID_SC_OPT_SYNCZOOM, pSyncZoomCB->IsChecked()));
        bRet = true;
    }

    return bRet;
}

void    ScTpContentOptions::Reset( const SfxItemSet* rCoreSet )
{
    const SfxPoolItem* pItem;
    if(SfxItemState::SET == rCoreSet->GetItemState(SID_SCVIEWOPTIONS, false , &pItem))
        pLocalOptions.reset( new ScViewOptions(
                            static_cast<const ScTpViewItem*>(pItem)->GetViewOptions() ) );
    else
        pLocalOptions.reset( new ScViewOptions );
    pFormulaCB ->Check(pLocalOptions->GetOption(VOPT_FORMULAS));
    pNilCB     ->Check(pLocalOptions->GetOption(VOPT_NULLVALS));
    pAnnotCB   ->Check(pLocalOptions->GetOption(VOPT_NOTES));
    pValueCB   ->Check(pLocalOptions->GetOption(VOPT_SYNTAX));
    pAnchorCB  ->Check(pLocalOptions->GetOption(VOPT_ANCHOR));
    pClipMarkCB->Check(pLocalOptions->GetOption(VOPT_CLIPMARKS));

    pObjGrfLB  ->SelectEntryPos( static_cast<sal_uInt16>(pLocalOptions->GetObjMode(VOBJ_TYPE_OLE)) );
    pDiagramLB ->SelectEntryPos( static_cast<sal_uInt16>(pLocalOptions->GetObjMode(VOBJ_TYPE_CHART)) );
    pDrawLB    ->SelectEntryPos( static_cast<sal_uInt16>(pLocalOptions->GetObjMode(VOBJ_TYPE_DRAW)) );

    pRowColHeaderCB->Check( pLocalOptions->GetOption(VOPT_HEADER) );
    pHScrollCB->Check( pLocalOptions->GetOption(VOPT_HSCROLL) );
    pVScrollCB->Check( pLocalOptions->GetOption(VOPT_VSCROLL) );
    pTblRegCB ->Check( pLocalOptions->GetOption(VOPT_TABCONTROLS) );
    pOutlineCB->Check( pLocalOptions->GetOption(VOPT_OUTLINER) );
    pSummaryCB->Check( pLocalOptions->GetOption(VOPT_SUMMARY) );

    InitGridOpt();

    pBreakCB->Check( pLocalOptions->GetOption(VOPT_PAGEBREAKS) );
    pGuideLineCB->Check( pLocalOptions->GetOption(VOPT_HELPLINES) );

    if(SfxItemState::SET == rCoreSet->GetItemState(SID_SC_INPUT_RANGEFINDER, false, &pItem))
        pRangeFindCB->Check(static_cast<const SfxBoolItem*>(pItem)->GetValue());
    if(SfxItemState::SET == rCoreSet->GetItemState(SID_SC_OPT_SYNCZOOM, false, &pItem))
        pSyncZoomCB->Check(static_cast<const SfxBoolItem*>(pItem)->GetValue());

    pRangeFindCB->SaveValue();
    pSyncZoomCB->SaveValue();

    pFormulaCB->SaveValue();
    pNilCB->SaveValue();
    pAnnotCB->SaveValue();
    pValueCB->SaveValue();
    pAnchorCB->SaveValue();
    pClipMarkCB->SaveValue();
    pObjGrfLB->SaveValue();
    pDiagramLB->SaveValue();
    pDrawLB->SaveValue();
    pRowColHeaderCB->SaveValue();
    pHScrollCB->SaveValue();
    pVScrollCB->SaveValue();
    pTblRegCB->SaveValue();
    pOutlineCB->SaveValue();
    pGridLB->SaveValue();
    pColorLB->SaveValue();
    pBreakCB->SaveValue();
    pGuideLineCB->SaveValue();
    pSummaryCB->SaveValue();
}

void ScTpContentOptions::ActivatePage( const SfxItemSet& rSet)
{
    const SfxPoolItem* pItem;
    if(SfxItemState::SET == rSet.GetItemState(SID_SCVIEWOPTIONS, false , &pItem))
        *pLocalOptions = static_cast<const ScTpViewItem*>(pItem)->GetViewOptions();
}

DeactivateRC ScTpContentOptions::DeactivatePage( SfxItemSet* pSetP )
{
    if(pSetP)
        FillItemSet(pSetP);
    return DeactivateRC::LeavePage;
}

IMPL_LINK( ScTpContentOptions, SelLbObjHdl, ListBox&, rLb, void )
{
    const sal_Int32 nSelPos = rLb.GetSelectedEntryPos();
    ScVObjMode  eMode   = ScVObjMode(nSelPos);
    ScVObjType  eType   = VOBJ_TYPE_OLE;

    if ( &rLb == pDiagramLB )
        eType = VOBJ_TYPE_CHART;
    else if ( &rLb == pDrawLB )
        eType = VOBJ_TYPE_DRAW;

    pLocalOptions->SetObjMode( eType, eMode );
}

IMPL_LINK( ScTpContentOptions, CBHdl, Button*, pBtn, void )
{
    ScViewOption eOption = VOPT_FORMULAS;
    bool         bChecked = static_cast<CheckBox*>(pBtn)->IsChecked();

    if (      pFormulaCB   == pBtn )   eOption = VOPT_FORMULAS;
    else if ( pNilCB       == pBtn )   eOption = VOPT_NULLVALS;
    else if ( pAnnotCB     == pBtn )   eOption = VOPT_NOTES;
    else if ( pValueCB     == pBtn )   eOption = VOPT_SYNTAX;
    else if ( pAnchorCB    == pBtn )   eOption = VOPT_ANCHOR;
    else if ( pClipMarkCB  == pBtn )   eOption = VOPT_CLIPMARKS;
    else if ( pVScrollCB       == pBtn )   eOption = VOPT_VSCROLL;
    else if ( pHScrollCB       == pBtn )   eOption = VOPT_HSCROLL;
    else if ( pTblRegCB        == pBtn )   eOption = VOPT_TABCONTROLS;
    else if ( pOutlineCB       == pBtn )   eOption = VOPT_OUTLINER;
    else if ( pBreakCB         == pBtn )   eOption = VOPT_PAGEBREAKS;
    else if ( pGuideLineCB     == pBtn )   eOption = VOPT_HELPLINES;
    else if ( pRowColHeaderCB  == pBtn )   eOption = VOPT_HEADER;
    else if ( pSummaryCB  == pBtn )   eOption = VOPT_SUMMARY;

    pLocalOptions->SetOption( eOption, bChecked );
}

void ScTpContentOptions::InitGridOpt()
{
    bool    bGrid = pLocalOptions->GetOption( VOPT_GRID );
    bool    bGridOnTop = pLocalOptions->GetOption( VOPT_GRID_ONTOP );
    sal_Int32   nSelPos = 0;

    if ( bGrid || bGridOnTop )
    {
        pColorFT->Enable();
        pColorLB->Enable();
        if ( !bGridOnTop )
            nSelPos = 0;
        else
            nSelPos = 1;
    }
    else
    {
        pColorFT->Disable();
        pColorLB->Disable();
        nSelPos = 2;
    }

    pGridLB->SelectEntryPos (nSelPos);

    //  select grid color entry
    OUString  aName;
    Color     aCol    = pLocalOptions->GetGridColor( &aName );

    if (aName.trim().isEmpty() && aCol == SC_STD_GRIDCOLOR)
        aCol = COL_AUTO;

    pColorLB->SelectEntry(std::make_pair(aCol, aName));
}

IMPL_LINK( ScTpContentOptions, GridHdl, ListBox&, rLb, void )
{
    sal_Int32   nSelPos = rLb.GetSelectedEntryPos();
    bool    bGrid = ( nSelPos <= 1 );
    bool    bGridOnTop = ( nSelPos == 1 );

    pColorFT->Enable(bGrid);
    pColorLB->Enable(bGrid);
    pLocalOptions->SetOption( VOPT_GRID, bGrid );
    pLocalOptions->SetOption( VOPT_GRID_ONTOP, bGridOnTop );
}

ScTpLayoutOptions::ScTpLayoutOptions(TabPageParent pParent, const SfxItemSet& rArgSet)
    : SfxTabPage(pParent, "modules/scalc/ui/scgeneralpage.ui", "ScGeneralPage", &rArgSet)
    , pDoc(nullptr)
    , m_xUnitLB(m_xBuilder->weld_combo_box("unitlb"))
    , m_xTabMF(m_xBuilder->weld_metric_spin_button("tabmf", FieldUnit::CM))
    , m_xAlwaysRB(m_xBuilder->weld_radio_button("alwaysrb"))
    , m_xRequestRB(m_xBuilder->weld_radio_button("requestrb"))
    , m_xNeverRB(m_xBuilder->weld_radio_button("neverrb"))
    , m_xAlignCB(m_xBuilder->weld_check_button("aligncb"))
    , m_xAlignLB(m_xBuilder->weld_combo_box("alignlb"))
    , m_xEditModeCB(m_xBuilder->weld_check_button("editmodecb"))
    , m_xFormatCB(m_xBuilder->weld_check_button("formatcb"))
    , m_xExpRefCB(m_xBuilder->weld_check_button("exprefcb"))
    , m_xSortRefUpdateCB(m_xBuilder->weld_check_button("sortrefupdatecb"))
    , m_xMarkHdrCB(m_xBuilder->weld_check_button("markhdrcb"))
    , m_xTextFmtCB(m_xBuilder->weld_check_button("textfmtcb"))
    , m_xReplWarnCB(m_xBuilder->weld_check_button("replwarncb"))
    , m_xLegacyCellSelectionCB(m_xBuilder->weld_check_button("legacy_cell_selection_cb"))
{
    SetExchangeSupport();

    m_xUnitLB->connect_changed( LINK( this, ScTpLayoutOptions, MetricHdl ) );
    m_xAlignCB->connect_toggled(LINK(this, ScTpLayoutOptions, AlignHdl));

    for (size_t i = 0; i < SAL_N_ELEMENTS(SCSTR_UNIT); ++i)
    {
        OUString sMetric = ScResId(SCSTR_UNIT[i].first);
        FieldUnit eFUnit = SCSTR_UNIT[i].second;

        switch ( eFUnit )
        {
            case FieldUnit::MM:
            case FieldUnit::CM:
            case FieldUnit::POINT:
            case FieldUnit::PICA:
            case FieldUnit::INCH:
            {
                // only use these metrics
                m_xUnitLB->append(OUString::number(static_cast<sal_uInt32>(eFUnit)), sMetric);
            }
            break;
            default:
            {
                // added to avoid warnings
            }
        }
    }
}

ScTpLayoutOptions::~ScTpLayoutOptions()
{
}

VclPtr<SfxTabPage> ScTpLayoutOptions::Create( TabPageParent pParent,
                                              const SfxItemSet*   rCoreSet )
{
    VclPtrInstance<ScTpLayoutOptions> pNew( pParent, *rCoreSet );
    ScDocShell* pDocSh = dynamic_cast< ScDocShell *>( SfxObjectShell::Current() );

    if(pDocSh!=nullptr)
        pNew->pDoc = &pDocSh->GetDocument();
    return pNew;
}

bool    ScTpLayoutOptions::FillItemSet( SfxItemSet* rCoreSet )
{
    bool bRet = true;
    if (m_xUnitLB->get_value_changed_from_saved())
    {
        const sal_Int32 nMPos = m_xUnitLB->get_active();
        sal_uInt16 nFieldUnit = m_xUnitLB->get_id(nMPos).toUInt32();
        rCoreSet->Put( SfxUInt16Item( SID_ATTR_METRIC, nFieldUnit ) );
        bRet = true;
    }

    if (m_xTabMF->get_value_changed_from_saved())
    {
        rCoreSet->Put(SfxUInt16Item(SID_ATTR_DEFTABSTOP,
                    sal::static_int_cast<sal_uInt16>( m_xTabMF->denormalize(m_xTabMF->get_value(FieldUnit::TWIP)) )));
        bRet = true;
    }

    ScLkUpdMode nSet=LM_ALWAYS;

    if (m_xRequestRB->get_active())
    {
        nSet=LM_ON_DEMAND;
    }
    else if (m_xNeverRB->get_active())
    {
        nSet=LM_NEVER;
    }

    if (m_xRequestRB->get_state_changed_from_saved() ||
        m_xNeverRB->get_state_changed_from_saved() )
    {
        if(pDoc)
            pDoc->SetLinkMode(nSet);
        ScAppOptions aAppOptions=SC_MOD()->GetAppOptions();
        aAppOptions.SetLinkMode(nSet );
        SC_MOD()->SetAppOptions(aAppOptions);
        bRet = true;
    }
    if (m_xAlignCB->get_state_changed_from_saved())
    {
        rCoreSet->Put(SfxBoolItem(SID_SC_INPUT_SELECTION, m_xAlignCB->get_active()));
        bRet = true;
    }

    if (m_xAlignLB->get_value_changed_from_saved())
    {
        rCoreSet->Put(SfxUInt16Item(SID_SC_INPUT_SELECTIONPOS, m_xAlignLB->get_active()));
        bRet = true;
    }

    if (m_xEditModeCB->get_state_changed_from_saved())
    {
        rCoreSet->Put(SfxBoolItem(SID_SC_INPUT_EDITMODE, m_xEditModeCB->get_active()));
        bRet = true;
    }

    if (m_xFormatCB->get_state_changed_from_saved())
    {
        rCoreSet->Put(SfxBoolItem(SID_SC_INPUT_FMT_EXPAND, m_xFormatCB->get_active()));
        bRet = true;
    }

    if (m_xExpRefCB->get_state_changed_from_saved())
    {
        rCoreSet->Put(SfxBoolItem(SID_SC_INPUT_REF_EXPAND, m_xExpRefCB->get_active()));
        bRet = true;
    }

    if (m_xSortRefUpdateCB->get_state_changed_from_saved())
    {
        rCoreSet->Put(SfxBoolItem(SID_SC_OPT_SORT_REF_UPDATE, m_xSortRefUpdateCB->get_active()));
        bRet = true;
    }

    if (m_xMarkHdrCB->get_state_changed_from_saved())
    {
        rCoreSet->Put(SfxBoolItem(SID_SC_INPUT_MARK_HEADER, m_xMarkHdrCB->get_active()));
        bRet = true;
    }

    if (m_xTextFmtCB->get_state_changed_from_saved())
    {
        rCoreSet->Put(SfxBoolItem(SID_SC_INPUT_TEXTWYSIWYG, m_xTextFmtCB->get_active()));
        bRet = true;
    }

    if (m_xReplWarnCB->get_state_changed_from_saved())
    {
        rCoreSet->Put( SfxBoolItem( SID_SC_INPUT_REPLCELLSWARN, m_xReplWarnCB->get_active() ) );
        bRet = true;
    }

    if (m_xLegacyCellSelectionCB->get_state_changed_from_saved())
    {
        rCoreSet->Put( SfxBoolItem( SID_SC_INPUT_LEGACY_CELL_SELECTION, m_xLegacyCellSelectionCB->get_active() ) );
        bRet = true;
    }

    return bRet;
}

void    ScTpLayoutOptions::Reset( const SfxItemSet* rCoreSet )
{
    m_xUnitLB->set_active(-1);
    if ( rCoreSet->GetItemState( SID_ATTR_METRIC ) >= SfxItemState::DEFAULT )
    {
        const SfxUInt16Item& rItem = rCoreSet->Get( SID_ATTR_METRIC );
        FieldUnit eFieldUnit = static_cast<FieldUnit>(rItem.GetValue());

        for (sal_Int32 i = 0, nEntryCount = m_xUnitLB->get_count(); i < nEntryCount; ++i)
        {
            if (m_xUnitLB->get_id(i).toUInt32() == static_cast<sal_uInt32>(eFieldUnit))
            {
                m_xUnitLB->set_active(i);
                break;
            }
        }
        ::SetFieldUnit(*m_xTabMF, eFieldUnit);
    }
    m_xUnitLB->save_value();

    const SfxPoolItem* pItem;
    if(SfxItemState::SET == rCoreSet->GetItemState(SID_ATTR_DEFTABSTOP, false, &pItem))
        m_xTabMF->set_value(m_xTabMF->normalize(static_cast<const SfxUInt16Item*>(pItem)->GetValue()), FieldUnit::TWIP);
    m_xTabMF->save_value();

    m_xUnitLB->save_value();
    m_xTabMF->save_value();

    ScLkUpdMode nSet=LM_UNKNOWN;

    if(pDoc!=nullptr)
    {
        nSet=pDoc->GetLinkMode();
    }

    if(nSet==LM_UNKNOWN)
    {
        ScAppOptions aAppOptions=SC_MOD()->GetAppOptions();
        nSet=aAppOptions.GetLinkMode();
    }

    switch(nSet)
    {
        case LM_ALWAYS:     m_xAlwaysRB->set_active(true);    break;
        case LM_NEVER:      m_xNeverRB->set_active(true);    break;
        case LM_ON_DEMAND:  m_xRequestRB->set_active(true);    break;
        default:
        {
            // added to avoid warnings
        }
    }
    if(SfxItemState::SET == rCoreSet->GetItemState(SID_SC_INPUT_SELECTION, false, &pItem))
        m_xAlignCB->set_active(static_cast<const SfxBoolItem*>(pItem)->GetValue());

    if(SfxItemState::SET == rCoreSet->GetItemState(SID_SC_INPUT_SELECTIONPOS, false, &pItem))
        m_xAlignLB->set_active(static_cast<const SfxUInt16Item*>(pItem)->GetValue());

    if(SfxItemState::SET == rCoreSet->GetItemState(SID_SC_INPUT_EDITMODE, false, &pItem))
        m_xEditModeCB->set_active(static_cast<const SfxBoolItem*>(pItem)->GetValue());

    if(SfxItemState::SET == rCoreSet->GetItemState(SID_SC_INPUT_FMT_EXPAND, false, &pItem))
        m_xFormatCB->set_active(static_cast<const SfxBoolItem*>(pItem)->GetValue());

    if(SfxItemState::SET == rCoreSet->GetItemState(SID_SC_INPUT_REF_EXPAND, false, &pItem))
        m_xExpRefCB->set_active(static_cast<const SfxBoolItem*>(pItem)->GetValue());

    if (rCoreSet->HasItem(SID_SC_OPT_SORT_REF_UPDATE, &pItem))
        m_xSortRefUpdateCB->set_active(static_cast<const SfxBoolItem*>(pItem)->GetValue());

    if(SfxItemState::SET == rCoreSet->GetItemState(SID_SC_INPUT_MARK_HEADER, false, &pItem))
        m_xMarkHdrCB->set_active(static_cast<const SfxBoolItem*>(pItem)->GetValue());

    if(SfxItemState::SET == rCoreSet->GetItemState(SID_SC_INPUT_TEXTWYSIWYG, false, &pItem))
        m_xTextFmtCB->set_active(static_cast<const SfxBoolItem*>(pItem)->GetValue());

    if( SfxItemState::SET == rCoreSet->GetItemState( SID_SC_INPUT_REPLCELLSWARN, false, &pItem ) )
        m_xReplWarnCB->set_active( static_cast<const SfxBoolItem*>(pItem)->GetValue() );

    if( SfxItemState::SET == rCoreSet->GetItemState( SID_SC_INPUT_LEGACY_CELL_SELECTION, false, &pItem ) )
        m_xLegacyCellSelectionCB->set_active( static_cast<const SfxBoolItem*>(pItem)->GetValue() );

    m_xAlignCB->save_state();
    m_xAlignLB->save_value();
    m_xEditModeCB->save_state();
    m_xFormatCB->save_state();

    m_xExpRefCB->save_state();
    m_xSortRefUpdateCB->save_state();
    m_xMarkHdrCB->save_state();
    m_xTextFmtCB->save_state();
    m_xReplWarnCB->save_state();

    m_xLegacyCellSelectionCB->save_state();

    AlignHdl(*m_xAlignCB);

    m_xAlwaysRB->save_state();
    m_xNeverRB->save_state();
    m_xRequestRB->save_state();
}

void ScTpLayoutOptions::ActivatePage( const SfxItemSet& /* rCoreSet */ )
{
}

DeactivateRC ScTpLayoutOptions::DeactivatePage( SfxItemSet* pSetP )
{
    if(pSetP)
        FillItemSet(pSetP);
    return DeactivateRC::LeavePage;
}

IMPL_LINK_NOARG(ScTpLayoutOptions, MetricHdl, weld::ComboBox&, void)
{
    const sal_Int32 nMPos = m_xUnitLB->get_active();
    if (nMPos != -1)
    {
        FieldUnit eFieldUnit = static_cast<FieldUnit>(m_xUnitLB->get_id(nMPos).toUInt32());
        sal_Int64 nVal =
            m_xTabMF->denormalize( m_xTabMF->get_value( FieldUnit::TWIP ) );
        ::SetFieldUnit( *m_xTabMF, eFieldUnit );
        m_xTabMF->set_value( m_xTabMF->normalize( nVal ), FieldUnit::TWIP );
    }
}

IMPL_LINK(ScTpLayoutOptions, AlignHdl, weld::ToggleButton&, rBox, void)
{
    m_xAlignLB->set_sensitive(rBox.get_active());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
