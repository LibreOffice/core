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

ScTpContentOptions::ScTpContentOptions(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet&  rArgSet)
    : SfxTabPage(pPage, pController, "modules/scalc/ui/tpviewpage.ui", "TpViewPage", &rArgSet)
    , m_xGridLB(m_xBuilder->weld_combo_box("grid"))
    , m_xColorFT(m_xBuilder->weld_label("color_label"))
    , m_xColorLB(new ColorListBox(m_xBuilder->weld_menu_button("color"),
                [this]{ return GetDialogController()->getDialog(); }))
    , m_xBreakCB(m_xBuilder->weld_check_button("break"))
    , m_xGuideLineCB(m_xBuilder->weld_check_button("guideline"))
    , m_xFormulaCB(m_xBuilder->weld_check_button("formula"))
    , m_xNilCB(m_xBuilder->weld_check_button("nil"))
    , m_xAnnotCB(m_xBuilder->weld_check_button("annot"))
    , m_xValueCB(m_xBuilder->weld_check_button("value"))
    , m_xAnchorCB(m_xBuilder->weld_check_button("anchor"))
    , m_xClipMarkCB(m_xBuilder->weld_check_button("clipmark"))
    , m_xRangeFindCB(m_xBuilder->weld_check_button("rangefind"))
    , m_xObjGrfLB(m_xBuilder->weld_combo_box("objgrf"))
    , m_xDiagramLB(m_xBuilder->weld_combo_box("diagram"))
    , m_xDrawLB(m_xBuilder->weld_combo_box("draw"))
    , m_xSyncZoomCB(m_xBuilder->weld_check_button("synczoom"))
    , m_xRowColHeaderCB(m_xBuilder->weld_check_button("rowcolheader"))
    , m_xHScrollCB(m_xBuilder->weld_check_button("hscroll"))
    , m_xVScrollCB(m_xBuilder->weld_check_button("vscroll"))
    , m_xTblRegCB(m_xBuilder->weld_check_button("tblreg"))
    , m_xOutlineCB(m_xBuilder->weld_check_button("outline"))
    , m_xSummaryCB(m_xBuilder->weld_check_button("cbSummary"))
{
    SetExchangeSupport();
    Link<weld::ComboBox&,void> aSelObjHdl(LINK( this, ScTpContentOptions, SelLbObjHdl ) );
    m_xObjGrfLB->connect_changed(aSelObjHdl);
    m_xDiagramLB->connect_changed(aSelObjHdl);
    m_xDrawLB->connect_changed(aSelObjHdl);
    m_xGridLB->connect_changed( LINK( this, ScTpContentOptions, GridHdl ) );

    Link<weld::ToggleButton&, void> aCBHdl(LINK( this, ScTpContentOptions, CBHdl ) );
    m_xFormulaCB->connect_toggled(aCBHdl);
    m_xNilCB->connect_toggled(aCBHdl);
    m_xAnnotCB->connect_toggled(aCBHdl);
    m_xValueCB->connect_toggled(aCBHdl);
    m_xAnchorCB->connect_toggled(aCBHdl);
    m_xClipMarkCB->connect_toggled(aCBHdl);

    m_xVScrollCB->connect_toggled(aCBHdl);
    m_xHScrollCB->connect_toggled(aCBHdl);
    m_xTblRegCB->connect_toggled(aCBHdl);
    m_xOutlineCB->connect_toggled(aCBHdl);
    m_xBreakCB->connect_toggled(aCBHdl);
    m_xGuideLineCB->connect_toggled(aCBHdl);
    m_xRowColHeaderCB->connect_toggled(aCBHdl);
    m_xSummaryCB->connect_toggled(aCBHdl);

    m_xColorLB->SetSlotId(SID_ATTR_CHAR_COLOR);
    m_xColorLB->SetAutoDisplayColor(SC_STD_GRIDCOLOR);
}

ScTpContentOptions::~ScTpContentOptions()
{
    m_xColorLB.reset();
}

std::unique_ptr<SfxTabPage> ScTpContentOptions::Create( weld::Container* pPage, weld::DialogController* pController,
                                               const SfxItemSet*     rCoreSet )
{
    return std::make_unique<ScTpContentOptions>(pPage, pController, *rCoreSet);
}

bool    ScTpContentOptions::FillItemSet( SfxItemSet* rCoreSet )
{
    bool bRet = false;
    if( m_xFormulaCB->get_state_changed_from_saved() ||
        m_xNilCB->get_state_changed_from_saved() ||
        m_xAnnotCB->get_state_changed_from_saved() ||
        m_xValueCB->get_state_changed_from_saved() ||
        m_xAnchorCB->get_state_changed_from_saved() ||
        m_xClipMarkCB->get_state_changed_from_saved() ||
        m_xObjGrfLB->get_value_changed_from_saved() ||
        m_xDiagramLB->get_value_changed_from_saved() ||
        m_xDrawLB->get_value_changed_from_saved() ||
        m_xGridLB->get_value_changed_from_saved() ||
        m_xRowColHeaderCB->get_state_changed_from_saved() ||
        m_xHScrollCB->get_state_changed_from_saved() ||
        m_xVScrollCB->get_state_changed_from_saved() ||
        m_xTblRegCB->get_state_changed_from_saved() ||
        m_xOutlineCB->get_state_changed_from_saved() ||
        m_xColorLB->IsValueChangedFromSaved() ||
        m_xBreakCB->get_state_changed_from_saved() ||
        m_xSummaryCB->get_state_changed_from_saved() ||
        m_xGuideLineCB->get_state_changed_from_saved())
    {
        NamedColor aNamedColor = m_xColorLB->GetSelectedEntry();
        if (aNamedColor.first == COL_AUTO)
        {
            aNamedColor.first = SC_STD_GRIDCOLOR;
            aNamedColor.second.clear();
        }
        m_xLocalOptions->SetGridColor(aNamedColor.first, aNamedColor.second);
        rCoreSet->Put(ScTpViewItem(*m_xLocalOptions));
        bRet = true;
    }
    if(m_xRangeFindCB->get_state_changed_from_saved())
    {
        rCoreSet->Put(SfxBoolItem(SID_SC_INPUT_RANGEFINDER, m_xRangeFindCB->get_active()));
        bRet = true;
    }
    if(m_xSyncZoomCB->get_state_changed_from_saved())
    {
        rCoreSet->Put(SfxBoolItem(SID_SC_OPT_SYNCZOOM, m_xSyncZoomCB->get_active()));
        bRet = true;
    }

    return bRet;
}

void    ScTpContentOptions::Reset( const SfxItemSet* rCoreSet )
{
    const SfxPoolItem* pItem;
    if(SfxItemState::SET == rCoreSet->GetItemState(SID_SCVIEWOPTIONS, false , &pItem))
        m_xLocalOptions.reset( new ScViewOptions(
                            static_cast<const ScTpViewItem*>(pItem)->GetViewOptions() ) );
    else
        m_xLocalOptions.reset( new ScViewOptions );
    m_xFormulaCB ->set_active(m_xLocalOptions->GetOption(VOPT_FORMULAS));
    m_xNilCB     ->set_active(m_xLocalOptions->GetOption(VOPT_NULLVALS));
    m_xAnnotCB   ->set_active(m_xLocalOptions->GetOption(VOPT_NOTES));
    m_xValueCB   ->set_active(m_xLocalOptions->GetOption(VOPT_SYNTAX));
    m_xAnchorCB  ->set_active(m_xLocalOptions->GetOption(VOPT_ANCHOR));
    m_xClipMarkCB->set_active(m_xLocalOptions->GetOption(VOPT_CLIPMARKS));

    m_xObjGrfLB  ->set_active( static_cast<sal_uInt16>(m_xLocalOptions->GetObjMode(VOBJ_TYPE_OLE)) );
    m_xDiagramLB ->set_active( static_cast<sal_uInt16>(m_xLocalOptions->GetObjMode(VOBJ_TYPE_CHART)) );
    m_xDrawLB    ->set_active( static_cast<sal_uInt16>(m_xLocalOptions->GetObjMode(VOBJ_TYPE_DRAW)) );

    m_xRowColHeaderCB->set_active( m_xLocalOptions->GetOption(VOPT_HEADER) );
    m_xHScrollCB->set_active( m_xLocalOptions->GetOption(VOPT_HSCROLL) );
    m_xVScrollCB->set_active( m_xLocalOptions->GetOption(VOPT_VSCROLL) );
    m_xTblRegCB ->set_active( m_xLocalOptions->GetOption(VOPT_TABCONTROLS) );
    m_xOutlineCB->set_active( m_xLocalOptions->GetOption(VOPT_OUTLINER) );
    m_xSummaryCB->set_active( m_xLocalOptions->GetOption(VOPT_SUMMARY) );

    InitGridOpt();

    m_xBreakCB->set_active( m_xLocalOptions->GetOption(VOPT_PAGEBREAKS) );
    m_xGuideLineCB->set_active( m_xLocalOptions->GetOption(VOPT_HELPLINES) );

    if(SfxItemState::SET == rCoreSet->GetItemState(SID_SC_INPUT_RANGEFINDER, false, &pItem))
        m_xRangeFindCB->set_active(static_cast<const SfxBoolItem*>(pItem)->GetValue());
    if(SfxItemState::SET == rCoreSet->GetItemState(SID_SC_OPT_SYNCZOOM, false, &pItem))
        m_xSyncZoomCB->set_active(static_cast<const SfxBoolItem*>(pItem)->GetValue());

    m_xRangeFindCB->save_state();
    m_xSyncZoomCB->save_state();

    m_xFormulaCB->save_state();
    m_xNilCB->save_state();
    m_xAnnotCB->save_state();
    m_xValueCB->save_state();
    m_xAnchorCB->save_state();
    m_xClipMarkCB->save_state();
    m_xObjGrfLB->save_value();
    m_xDiagramLB->save_value();
    m_xDrawLB->save_value();
    m_xRowColHeaderCB->save_state();
    m_xHScrollCB->save_state();
    m_xVScrollCB->save_state();
    m_xTblRegCB->save_state();
    m_xOutlineCB->save_state();
    m_xGridLB->save_value();
    m_xColorLB->SaveValue();
    m_xBreakCB->save_state();
    m_xGuideLineCB->save_state();
    m_xSummaryCB->save_state();
}

void ScTpContentOptions::ActivatePage( const SfxItemSet& rSet)
{
    const SfxPoolItem* pItem;
    if(SfxItemState::SET == rSet.GetItemState(SID_SCVIEWOPTIONS, false , &pItem))
        *m_xLocalOptions = static_cast<const ScTpViewItem*>(pItem)->GetViewOptions();
}

DeactivateRC ScTpContentOptions::DeactivatePage( SfxItemSet* pSetP )
{
    if(pSetP)
        FillItemSet(pSetP);
    return DeactivateRC::LeavePage;
}

IMPL_LINK( ScTpContentOptions, SelLbObjHdl, weld::ComboBox&, rLb, void )
{
    const sal_Int32 nSelPos = rLb.get_active();
    ScVObjMode  eMode   = ScVObjMode(nSelPos);
    ScVObjType  eType   = VOBJ_TYPE_OLE;

    if ( &rLb == m_xDiagramLB.get() )
        eType = VOBJ_TYPE_CHART;
    else if ( &rLb == m_xDrawLB.get() )
        eType = VOBJ_TYPE_DRAW;

    m_xLocalOptions->SetObjMode( eType, eMode );
}

IMPL_LINK( ScTpContentOptions, CBHdl, weld::ToggleButton&, rBtn, void )
{
    ScViewOption eOption = VOPT_FORMULAS;
    bool         bChecked = rBtn.get_active();

    if (m_xFormulaCB.get() == &rBtn )   eOption = VOPT_FORMULAS;
    else if ( m_xNilCB.get() == &rBtn )   eOption = VOPT_NULLVALS;
    else if ( m_xAnnotCB.get() == &rBtn )   eOption = VOPT_NOTES;
    else if ( m_xValueCB.get() == &rBtn )   eOption = VOPT_SYNTAX;
    else if ( m_xAnchorCB.get() == &rBtn )   eOption = VOPT_ANCHOR;
    else if ( m_xClipMarkCB.get() == &rBtn )   eOption = VOPT_CLIPMARKS;
    else if ( m_xVScrollCB.get()  == &rBtn )   eOption = VOPT_VSCROLL;
    else if ( m_xHScrollCB.get() == &rBtn )   eOption = VOPT_HSCROLL;
    else if ( m_xTblRegCB.get() == &rBtn )   eOption = VOPT_TABCONTROLS;
    else if ( m_xOutlineCB.get() == &rBtn )   eOption = VOPT_OUTLINER;
    else if ( m_xBreakCB.get() == &rBtn )   eOption = VOPT_PAGEBREAKS;
    else if ( m_xGuideLineCB.get() == &rBtn )   eOption = VOPT_HELPLINES;
    else if ( m_xRowColHeaderCB.get() == &rBtn )   eOption = VOPT_HEADER;
    else if ( m_xSummaryCB.get()  == &rBtn )   eOption = VOPT_SUMMARY;

    m_xLocalOptions->SetOption( eOption, bChecked );
}

void ScTpContentOptions::InitGridOpt()
{
    bool    bGrid = m_xLocalOptions->GetOption( VOPT_GRID );
    bool    bGridOnTop = m_xLocalOptions->GetOption( VOPT_GRID_ONTOP );
    sal_Int32   nSelPos = 0;

    if ( bGrid || bGridOnTop )
    {
        m_xColorFT->set_sensitive(true);
        m_xColorLB->set_sensitive(true);
        if ( !bGridOnTop )
            nSelPos = 0;
        else
            nSelPos = 1;
    }
    else
    {
        m_xColorFT->set_sensitive(false);
        m_xColorLB->set_sensitive(false);
        nSelPos = 2;
    }

    m_xGridLB->set_active (nSelPos);

    //  select grid color entry
    OUString  aName;
    Color     aCol    = m_xLocalOptions->GetGridColor( &aName );

    if (aName.trim().isEmpty() && aCol == SC_STD_GRIDCOLOR)
        aCol = COL_AUTO;

    m_xColorLB->SelectEntry(std::make_pair(aCol, aName));
}

IMPL_LINK( ScTpContentOptions, GridHdl, weld::ComboBox&, rLb, void )
{
    sal_Int32   nSelPos = rLb.get_active();
    bool    bGrid = ( nSelPos <= 1 );
    bool    bGridOnTop = ( nSelPos == 1 );

    m_xColorFT->set_sensitive(bGrid);
    m_xColorLB->set_sensitive(bGrid);
    m_xLocalOptions->SetOption( VOPT_GRID, bGrid );
    m_xLocalOptions->SetOption( VOPT_GRID_ONTOP, bGridOnTop );
}

ScTpLayoutOptions::ScTpLayoutOptions(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rArgSet)
    : SfxTabPage(pPage, pController, "modules/scalc/ui/scgeneralpage.ui", "ScGeneralPage", &rArgSet)
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
    , m_xEnterPasteModeCB(m_xBuilder->weld_check_button("enter_paste_mode_cb"))
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

std::unique_ptr<SfxTabPage> ScTpLayoutOptions::Create( weld::Container* pPage, weld::DialogController* pController,
                                              const SfxItemSet*   rCoreSet )
{
    auto xNew = std::make_unique<ScTpLayoutOptions>(pPage, pController, *rCoreSet);

    ScDocShell* pDocSh = dynamic_cast< ScDocShell *>( SfxObjectShell::Current() );
    if (pDocSh!=nullptr)
        xNew->pDoc = &pDocSh->GetDocument();
    return xNew;
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

    if (m_xEnterPasteModeCB->get_state_changed_from_saved())
    {
        rCoreSet->Put( SfxBoolItem( SID_SC_INPUT_ENTER_PASTE_MODE, m_xEnterPasteModeCB->get_active() ) );
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

    if( SfxItemState::SET == rCoreSet->GetItemState( SID_SC_INPUT_ENTER_PASTE_MODE, false, &pItem ) )
        m_xEnterPasteModeCB->set_active( static_cast<const SfxBoolItem*>(pItem)->GetValue() );

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
    m_xEnterPasteModeCB->save_state();

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
