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

#include <officecfg/Office/Calc.hxx>
#include <tpview.hxx>
#include <global.hxx>
#include <viewopti.hxx>
#include <scresid.hxx>
#include <docsh.hxx>
#include <sc.hrc>
#include <strings.hrc>
#include <units.hrc>
#include <appoptio.hxx>
#include <scmod.hxx>
#include <svl/eitem.hxx>
#include <svtools/unitconv.hxx>
#include <unotools/localedatawrapper.hxx>

ScTpContentOptions::ScTpContentOptions(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet&  rArgSet)
    : SfxTabPage(pPage, pController, "modules/scalc/ui/tpviewpage.ui", "TpViewPage", &rArgSet)
    , m_xGridLB(m_xBuilder->weld_combo_box("grid"))
    , m_xGridImg(m_xBuilder->weld_widget("lockgrid"))
    , m_xBreakCB(m_xBuilder->weld_check_button("break"))
    , m_xBreakImg(m_xBuilder->weld_widget("lockbreak"))
    , m_xGuideLineCB(m_xBuilder->weld_check_button("guideline"))
    , m_xGuideLineImg(m_xBuilder->weld_widget("lockguideline"))
    , m_xFormulaCB(m_xBuilder->weld_check_button("formula"))
    , m_xFormulaImg(m_xBuilder->weld_widget("lockformula"))
    , m_xNilCB(m_xBuilder->weld_check_button("nil"))
    , m_xNilImg(m_xBuilder->weld_widget("locknil"))
    , m_xAnnotCB(m_xBuilder->weld_check_button("annot"))
    , m_xAnnotImg(m_xBuilder->weld_widget("lockannot"))
    , m_xFormulaMarkCB(m_xBuilder->weld_check_button("formulamark"))
    , m_xFormulaMarkImg(m_xBuilder->weld_widget("lockformulamark"))
    , m_xValueCB(m_xBuilder->weld_check_button("value"))
    , m_xValueImg(m_xBuilder->weld_widget("lockvalue"))
    , m_xColRowHighCB(m_xBuilder->weld_check_button("colrowhigh"))
    , m_xColRowHighImg(m_xBuilder->weld_widget("lockcolrowhigh"))
    , m_xAnchorCB(m_xBuilder->weld_check_button("anchor"))
    , m_xAnchorImg(m_xBuilder->weld_widget("lockanchor"))
    , m_xRangeFindCB(m_xBuilder->weld_check_button("rangefind"))
    , m_xRangeFindImg(m_xBuilder->weld_widget("lockrangefind"))
    , m_xObjGrfLB(m_xBuilder->weld_combo_box("objgrf"))
    , m_xObjGrfImg(m_xBuilder->weld_widget("lockobjgrf"))
    , m_xDiagramLB(m_xBuilder->weld_combo_box("diagram"))
    , m_xDiagramImg(m_xBuilder->weld_widget("lockdiagram"))
    , m_xDrawLB(m_xBuilder->weld_combo_box("draw"))
    , m_xDrawImg(m_xBuilder->weld_widget("lockdraw"))
    , m_xSyncZoomCB(m_xBuilder->weld_check_button("synczoom"))
    , m_xSyncZoomImg(m_xBuilder->weld_widget("locksynczoom"))
    , m_xRowColHeaderCB(m_xBuilder->weld_check_button("rowcolheader"))
    , m_xRowColHeaderImg(m_xBuilder->weld_widget("lockrowcolheader"))
    , m_xHScrollCB(m_xBuilder->weld_check_button("hscroll"))
    , m_xHScrollImg(m_xBuilder->weld_widget("lockhscroll"))
    , m_xVScrollCB(m_xBuilder->weld_check_button("vscroll"))
    , m_xVScrollImg(m_xBuilder->weld_widget("lockvscroll"))
    , m_xTblRegCB(m_xBuilder->weld_check_button("tblreg"))
    , m_xTblRegImg(m_xBuilder->weld_widget("locktblreg"))
    , m_xOutlineCB(m_xBuilder->weld_check_button("outline"))
    , m_xOutlineImg(m_xBuilder->weld_widget("lockoutline"))
    , m_xSummaryCB(m_xBuilder->weld_check_button("cbSummary"))
    , m_xSummaryImg(m_xBuilder->weld_widget("lockcbSummary"))
    , m_xThemedCursorRB(m_xBuilder->weld_radio_button("rbThemedCursor"))
    , m_xSystemCursorRB(m_xBuilder->weld_radio_button("rbSystemCursor"))
    , m_xCursorImg(m_xBuilder->weld_widget("lockCursor"))
{
    SetExchangeSupport();
    Link<weld::ComboBox&,void> aSelObjHdl(LINK( this, ScTpContentOptions, SelLbObjHdl ) );
    m_xObjGrfLB->connect_changed(aSelObjHdl);
    m_xDiagramLB->connect_changed(aSelObjHdl);
    m_xDrawLB->connect_changed(aSelObjHdl);
    m_xGridLB->connect_changed( LINK( this, ScTpContentOptions, GridHdl ) );

    Link<weld::Toggleable&, void> aCBHdl(LINK( this, ScTpContentOptions, CBHdl ) );
    m_xFormulaCB->connect_toggled(aCBHdl);
    m_xNilCB->connect_toggled(aCBHdl);
    m_xAnnotCB->connect_toggled(aCBHdl);
    m_xAnnotCB->set_accessible_description(ScResId(STR_A11Y_DESC_ANNOT));
    m_xFormulaMarkCB->connect_toggled(aCBHdl);
    m_xValueCB->connect_toggled(aCBHdl);
    m_xColRowHighCB->connect_toggled(aCBHdl);
    m_xAnchorCB->connect_toggled(aCBHdl);

    m_xVScrollCB->connect_toggled(aCBHdl);
    m_xHScrollCB->connect_toggled(aCBHdl);
    m_xTblRegCB->connect_toggled(aCBHdl);
    m_xOutlineCB->connect_toggled(aCBHdl);
    m_xBreakCB->connect_toggled(aCBHdl);
    m_xGuideLineCB->connect_toggled(aCBHdl);
    m_xRowColHeaderCB->connect_toggled(aCBHdl);
    m_xSummaryCB->connect_toggled(aCBHdl);
    m_xThemedCursorRB->connect_toggled(aCBHdl);
}

ScTpContentOptions::~ScTpContentOptions()
{
}

std::unique_ptr<SfxTabPage> ScTpContentOptions::Create( weld::Container* pPage, weld::DialogController* pController,
                                               const SfxItemSet*     rCoreSet )
{
    return std::make_unique<ScTpContentOptions>(pPage, pController, *rCoreSet);
}

OUString ScTpContentOptions::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { "label4",   "label5", "label3",       "label1",        "grid_label",
                          "lbCursor", "label2", "objgrf_label", "diagram_label", "draw_label" };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString checkButton[]
        = { "formula",   "nil",          "annot",   "formulamark", "value",  "anchor",
            "rangefind", "rowcolheader", "hscroll", "vscroll",     "tblreg", "outline",
            "cbSummary", "synczoom",     "break",   "guideline" };

    for (const auto& check : checkButton)
    {
        if (const auto& pString = m_xBuilder->weld_check_button(check))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
}

bool    ScTpContentOptions::FillItemSet( SfxItemSet* rCoreSet )
{
    bool bRet = false;
    if( m_xFormulaCB->get_state_changed_from_saved() ||
        m_xNilCB->get_state_changed_from_saved() ||
        m_xAnnotCB->get_state_changed_from_saved() ||
        m_xFormulaMarkCB->get_state_changed_from_saved() ||
        m_xValueCB->get_state_changed_from_saved() ||
        m_xAnchorCB->get_state_changed_from_saved() ||
        m_xObjGrfLB->get_value_changed_from_saved() ||
        m_xDiagramLB->get_value_changed_from_saved() ||
        m_xDrawLB->get_value_changed_from_saved() ||
        m_xGridLB->get_value_changed_from_saved() ||
        m_xRowColHeaderCB->get_state_changed_from_saved() ||
        m_xHScrollCB->get_state_changed_from_saved() ||
        m_xVScrollCB->get_state_changed_from_saved() ||
        m_xTblRegCB->get_state_changed_from_saved() ||
        m_xOutlineCB->get_state_changed_from_saved() ||
        m_xBreakCB->get_state_changed_from_saved() ||
        m_xSummaryCB->get_state_changed_from_saved() ||
        m_xThemedCursorRB->get_state_changed_from_saved() ||
        m_xGuideLineCB->get_state_changed_from_saved())
    {
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
    if (m_xColRowHighCB->get_state_changed_from_saved())
    {
        auto pChange(comphelper::ConfigurationChanges::create());
        officecfg::Office::Calc::Content::Display::ColumnRowHighlighting::set(m_xColRowHighCB->get_active(), pChange);
        pChange->commit();
        bRet = true;
    }

    return bRet;
}

void    ScTpContentOptions::Reset( const SfxItemSet* rCoreSet )
{
    if(const ScTpViewItem* pViewItem = rCoreSet->GetItemIfSet(SID_SCVIEWOPTIONS, false))
        m_xLocalOptions.reset( new ScViewOptions( pViewItem->GetViewOptions() ) );
    else
        m_xLocalOptions.reset( new ScViewOptions );
    m_xFormulaCB ->set_active(m_xLocalOptions->GetOption(VOPT_FORMULAS));
    m_xNilCB     ->set_active(m_xLocalOptions->GetOption(VOPT_NULLVALS));
    m_xAnnotCB   ->set_active(m_xLocalOptions->GetOption(VOPT_NOTES));
    m_xFormulaMarkCB->set_active(m_xLocalOptions->GetOption(VOPT_FORMULAS_MARKS));
    m_xValueCB   ->set_active(m_xLocalOptions->GetOption(VOPT_SYNTAX));
    m_xColRowHighCB->set_active(officecfg::Office::Calc::Content::Display::ColumnRowHighlighting::get());
    m_xAnchorCB  ->set_active(m_xLocalOptions->GetOption(VOPT_ANCHOR));

    m_xObjGrfLB  ->set_active( static_cast<sal_uInt16>(m_xLocalOptions->GetObjMode(VOBJ_TYPE_OLE)) );
    m_xDiagramLB ->set_active( static_cast<sal_uInt16>(m_xLocalOptions->GetObjMode(VOBJ_TYPE_CHART)) );
    m_xDrawLB    ->set_active( static_cast<sal_uInt16>(m_xLocalOptions->GetObjMode(VOBJ_TYPE_DRAW)) );

    m_xRowColHeaderCB->set_active( m_xLocalOptions->GetOption(VOPT_HEADER) );
    m_xHScrollCB->set_active( m_xLocalOptions->GetOption(VOPT_HSCROLL) );
    m_xVScrollCB->set_active( m_xLocalOptions->GetOption(VOPT_VSCROLL) );
    m_xTblRegCB ->set_active( m_xLocalOptions->GetOption(VOPT_TABCONTROLS) );
    m_xOutlineCB->set_active( m_xLocalOptions->GetOption(VOPT_OUTLINER) );
    m_xSummaryCB->set_active( m_xLocalOptions->GetOption(VOPT_SUMMARY) );
    if ( m_xLocalOptions->GetOption(VOPT_THEMEDCURSOR) )
        m_xThemedCursorRB->set_active( true );
    else
        m_xSystemCursorRB->set_active( true );

    InitGridOpt();

    m_xBreakCB->set_active( m_xLocalOptions->GetOption(VOPT_PAGEBREAKS) );
    m_xGuideLineCB->set_active( m_xLocalOptions->GetOption(VOPT_HELPLINES) );

    if(const SfxBoolItem* pFinderItem = rCoreSet->GetItemIfSet(SID_SC_INPUT_RANGEFINDER, false))
        m_xRangeFindCB->set_active(pFinderItem->GetValue());
    if(const SfxBoolItem* pZoomItem = rCoreSet->GetItemIfSet(SID_SC_OPT_SYNCZOOM, false))
        m_xSyncZoomCB->set_active(pZoomItem->GetValue());

    bool bReadOnly = officecfg::Office::Calc::Layout::Line::GridLine::isReadOnly() ||
        officecfg::Office::Calc::Layout::Line::GridOnColoredCells::isReadOnly();
    m_xGridLB->set_sensitive(!bReadOnly);
    m_xGridImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Calc::Layout::Line::PageBreak::isReadOnly();
    m_xBreakCB->set_sensitive(!bReadOnly);
    m_xBreakImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Calc::Layout::Line::Guide::isReadOnly();
    m_xGuideLineCB->set_sensitive(!bReadOnly);
    m_xGuideLineImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Calc::Content::Display::Formula::isReadOnly();
    m_xFormulaCB->set_sensitive(!bReadOnly);
    m_xFormulaImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Calc::Content::Display::ZeroValue::isReadOnly();
    m_xNilCB->set_sensitive(!bReadOnly);
    m_xNilImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Calc::Content::Display::NoteTag::isReadOnly();
    m_xAnnotCB->set_sensitive(!bReadOnly);
    m_xAnnotImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Calc::Content::Display::FormulaMark::isReadOnly();
    m_xFormulaMarkCB->set_sensitive(!bReadOnly);
    m_xFormulaMarkImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Calc::Content::Display::ValueHighlighting::isReadOnly();
    m_xValueCB->set_sensitive(!bReadOnly);
    m_xValueImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Calc::Content::Display::ColumnRowHighlighting::isReadOnly();
    m_xColRowHighCB->set_sensitive(!bReadOnly);
    m_xColRowHighImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Calc::Content::Display::Anchor::isReadOnly();
    m_xAnchorCB->set_sensitive(!bReadOnly);
    m_xAnchorImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Calc::Input::ShowReference::isReadOnly();
    m_xRangeFindCB->set_sensitive(!bReadOnly);
    m_xRangeFindImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Calc::Content::Display::ObjectGraphic::isReadOnly();
    m_xObjGrfLB->set_sensitive(!bReadOnly);
    m_xObjGrfImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Calc::Content::Display::Chart::isReadOnly();
    m_xDiagramLB->set_sensitive(!bReadOnly);
    m_xDiagramImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Calc::Content::Display::DrawingObject::isReadOnly();
    m_xDrawLB->set_sensitive(!bReadOnly);
    m_xDrawImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Calc::Layout::Zoom::Synchronize::isReadOnly();
    m_xSyncZoomCB->set_sensitive(!bReadOnly);
    m_xSyncZoomImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Calc::Layout::Window::ColumnRowHeader::isReadOnly();
    m_xRowColHeaderCB->set_sensitive(!bReadOnly);
    m_xRowColHeaderImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Calc::Layout::Window::HorizontalScroll::isReadOnly();
    m_xHScrollCB->set_sensitive(!bReadOnly);
    m_xHScrollImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Calc::Layout::Window::VerticalScroll::isReadOnly();
    m_xVScrollCB->set_sensitive(!bReadOnly);
    m_xVScrollImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Calc::Layout::Window::SheetTab::isReadOnly();
    m_xTblRegCB->set_sensitive(!bReadOnly);
    m_xTblRegImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Calc::Layout::Window::OutlineSymbol::isReadOnly();
    m_xOutlineCB->set_sensitive(!bReadOnly);
    m_xOutlineImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Calc::Layout::Window::SearchSummary::isReadOnly();
    m_xSummaryCB->set_sensitive(!bReadOnly);
    m_xSummaryImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Calc::Layout::Window::ThemedCursor::isReadOnly();
    m_xThemedCursorRB->set_sensitive(!bReadOnly);
    m_xSystemCursorRB->set_sensitive(!bReadOnly);
    m_xCursorImg->set_visible(bReadOnly);

    m_xRangeFindCB->save_state();
    m_xSyncZoomCB->save_state();

    m_xFormulaCB->save_state();
    m_xNilCB->save_state();
    m_xAnnotCB->save_state();
    m_xFormulaMarkCB->save_state();
    m_xValueCB->save_state();
    m_xColRowHighCB->save_state();
    m_xAnchorCB->save_state();
    m_xObjGrfLB->save_value();
    m_xDiagramLB->save_value();
    m_xDrawLB->save_value();
    m_xRowColHeaderCB->save_state();
    m_xHScrollCB->save_state();
    m_xVScrollCB->save_state();
    m_xTblRegCB->save_state();
    m_xOutlineCB->save_state();
    m_xGridLB->save_value();
    m_xBreakCB->save_state();
    m_xGuideLineCB->save_state();
    m_xSummaryCB->save_state();
    m_xThemedCursorRB->save_state();
}

void ScTpContentOptions::ActivatePage( const SfxItemSet& rSet)
{
    if(const ScTpViewItem* pViewItem = rSet.GetItemIfSet(SID_SCVIEWOPTIONS, false))
        *m_xLocalOptions = pViewItem->GetViewOptions();
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

IMPL_LINK( ScTpContentOptions, CBHdl, weld::Toggleable&, rBtn, void )
{
    ScViewOption eOption = VOPT_FORMULAS;
    bool         bChecked = rBtn.get_active();

    if (m_xFormulaCB.get() == &rBtn )   eOption = VOPT_FORMULAS;
    else if ( m_xNilCB.get() == &rBtn )   eOption = VOPT_NULLVALS;
    else if ( m_xAnnotCB.get() == &rBtn )   eOption = VOPT_NOTES;
    else if ( m_xFormulaMarkCB.get() == &rBtn )   eOption = VOPT_FORMULAS_MARKS;
    else if ( m_xValueCB.get() == &rBtn )   eOption = VOPT_SYNTAX;
    else if ( m_xAnchorCB.get() == &rBtn )   eOption = VOPT_ANCHOR;
    else if ( m_xVScrollCB.get()  == &rBtn )   eOption = VOPT_VSCROLL;
    else if ( m_xHScrollCB.get() == &rBtn )   eOption = VOPT_HSCROLL;
    else if ( m_xTblRegCB.get() == &rBtn )   eOption = VOPT_TABCONTROLS;
    else if ( m_xOutlineCB.get() == &rBtn )   eOption = VOPT_OUTLINER;
    else if ( m_xBreakCB.get() == &rBtn )   eOption = VOPT_PAGEBREAKS;
    else if ( m_xGuideLineCB.get() == &rBtn )   eOption = VOPT_HELPLINES;
    else if ( m_xRowColHeaderCB.get() == &rBtn )   eOption = VOPT_HEADER;
    else if ( m_xSummaryCB.get()  == &rBtn )   eOption = VOPT_SUMMARY;
    else if ( m_xThemedCursorRB.get() == &rBtn )   eOption = VOPT_THEMEDCURSOR;

    m_xLocalOptions->SetOption( eOption, bChecked );
}

void ScTpContentOptions::InitGridOpt()
{
    bool    bGrid = m_xLocalOptions->GetOption( VOPT_GRID );
    bool    bGridOnTop = m_xLocalOptions->GetOption( VOPT_GRID_ONTOP );
    sal_Int32   nSelPos = 0;

    if ( bGrid || bGridOnTop )
    {
        if ( !bGridOnTop )
            nSelPos = 0;
        else
            nSelPos = 1;
    }
    else
        nSelPos = 2;

    m_xGridLB->set_active (nSelPos);
}

IMPL_LINK( ScTpContentOptions, GridHdl, weld::ComboBox&, rLb, void )
{
    sal_Int32   nSelPos = rLb.get_active();
    bool    bGrid = ( nSelPos <= 1 );
    bool    bGridOnTop = ( nSelPos == 1 );

    m_xLocalOptions->SetOption( VOPT_GRID, bGrid );
    m_xLocalOptions->SetOption( VOPT_GRID_ONTOP, bGridOnTop );
}

ScTpLayoutOptions::ScTpLayoutOptions(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rArgSet)
    : SfxTabPage(pPage, pController, "modules/scalc/ui/scgeneralpage.ui", "ScGeneralPage", &rArgSet)
    , pDoc(nullptr)
    , m_xUnitLB(m_xBuilder->weld_combo_box("unitlb"))
    , m_xUnitImg(m_xBuilder->weld_widget("lockunitlb"))
    , m_xTabMF(m_xBuilder->weld_metric_spin_button("tabmf", FieldUnit::CM))
    , m_xTabImg(m_xBuilder->weld_widget("locktabmf"))
    , m_xAlwaysRB(m_xBuilder->weld_radio_button("alwaysrb"))
    , m_xRequestRB(m_xBuilder->weld_radio_button("requestrb"))
    , m_xNeverRB(m_xBuilder->weld_radio_button("neverrb"))
    , m_xUpdateLinksImg(m_xBuilder->weld_widget("lockupdatelinks"))
    , m_xAlignCB(m_xBuilder->weld_check_button("aligncb"))
    , m_xAlignImg(m_xBuilder->weld_widget("lockaligncb"))
    , m_xAlignLB(m_xBuilder->weld_combo_box("alignlb"))
    , m_xEditModeCB(m_xBuilder->weld_check_button("editmodecb"))
    , m_xEditModeImg(m_xBuilder->weld_widget("lockeditmodecb"))
    , m_xFormatCB(m_xBuilder->weld_check_button("formatcb"))
    , m_xFormatImg(m_xBuilder->weld_widget("lockformatcb"))
    , m_xExpRefCB(m_xBuilder->weld_check_button("exprefcb"))
    , m_xExpRefImg(m_xBuilder->weld_widget("lockexprefcb"))
    , m_xSortRefUpdateCB(m_xBuilder->weld_check_button("sortrefupdatecb"))
    , m_xSortRefUpdateImg(m_xBuilder->weld_widget("locksortrefupdatecb"))
    , m_xMarkHdrCB(m_xBuilder->weld_check_button("markhdrcb"))
    , m_xMarkHdrImg(m_xBuilder->weld_widget("lockmarkhdrcb"))
    , m_xReplWarnCB(m_xBuilder->weld_check_button("replwarncb"))
    , m_xReplWarnImg(m_xBuilder->weld_widget("lockreplwarncb"))
    , m_xLegacyCellSelectionCB(m_xBuilder->weld_check_button("legacy_cell_selection_cb"))
    , m_xLegacyCellSelectionImg(m_xBuilder->weld_widget("locklegacy_cell"))
    , m_xEnterPasteModeCB(m_xBuilder->weld_check_button("enter_paste_mode_cb"))
    , m_xEnterPasteModeImg(m_xBuilder->weld_widget("lockenter_paste"))
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

OUString ScTpLayoutOptions::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { "label1", "label4", "label5", "label6", "label3" };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString checkButton[] = { "aligncb",   "editmodecb", "enter_paste_mode_cb",
                               "formatcb",  "exprefcb",   "sortrefupdatecb",
                               "markhdrcb", "replwarncb", "legacy_cell_selection_cb" };

    for (const auto& check : checkButton)
    {
        if (const auto& pString = m_xBuilder->weld_check_button(check))
            sAllStrings += pString->get_label() + " ";
    }

    OUString radioButton[] = { "alwaysrb", "requestrb", "neverrb" };

    for (const auto& radio : radioButton)
    {
        if (const auto& pString = m_xBuilder->weld_radio_button(radio))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
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

    bool bReadOnly = false;
    MeasurementSystem eSys = ScGlobal::getLocaleData().getMeasurementSystemEnum();
    if (eSys == MeasurementSystem::Metric)
    {
        bReadOnly = officecfg::Office::Calc::Layout::Other::MeasureUnit::Metric::isReadOnly();
    }
    else
    {
        bReadOnly = officecfg::Office::Calc::Layout::Other::MeasureUnit::NonMetric::isReadOnly();
    }
    m_xUnitLB->set_sensitive(!bReadOnly);
    m_xUnitImg->set_visible(bReadOnly);

    if(const SfxUInt16Item* pTabStopItem = rCoreSet->GetItemIfSet(SID_ATTR_DEFTABSTOP, false))
        m_xTabMF->set_value(m_xTabMF->normalize(pTabStopItem->GetValue()), FieldUnit::TWIP);

    if (eSys == MeasurementSystem::Metric)
    {
        bReadOnly = officecfg::Office::Calc::Layout::Other::TabStop::Metric::isReadOnly();
    }
    else
    {
        bReadOnly = officecfg::Office::Calc::Layout::Other::TabStop::NonMetric::isReadOnly();
    }
    m_xTabMF->set_sensitive(!bReadOnly);
    m_xTabImg->set_visible(bReadOnly);

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

    if (officecfg::Office::Calc::Content::Update::Link::isReadOnly())
    {
        m_xAlwaysRB->set_sensitive(false);
        m_xNeverRB->set_sensitive(false);
        m_xRequestRB->set_sensitive(false);
        m_xUpdateLinksImg->set_visible(true);
    }
    if(const SfxBoolItem* pSelectionItem = rCoreSet->GetItemIfSet(SID_SC_INPUT_SELECTION, false))
        m_xAlignCB->set_active(pSelectionItem->GetValue());

    bReadOnly = officecfg::Office::Calc::Input::MoveSelection::isReadOnly();
    m_xAlignCB->set_sensitive(!bReadOnly);
    m_xAlignImg->set_visible(bReadOnly);

    if(const SfxUInt16Item* pPosItem = rCoreSet->GetItemIfSet(SID_SC_INPUT_SELECTIONPOS, false))
        m_xAlignLB->set_active(pPosItem->GetValue());

    bReadOnly = officecfg::Office::Calc::Input::MoveSelectionDirection::isReadOnly();
    m_xAlignCB->set_sensitive(!bReadOnly);

    if(const SfxBoolItem* pEditModeItem = rCoreSet->GetItemIfSet(SID_SC_INPUT_EDITMODE, false))
        m_xEditModeCB->set_active(pEditModeItem->GetValue());

    bReadOnly = officecfg::Office::Calc::Input::SwitchToEditMode::isReadOnly();
    m_xEditModeCB->set_sensitive(!bReadOnly);
    m_xEditModeImg->set_visible(bReadOnly);

    if(const SfxBoolItem* pExpandItem = rCoreSet->GetItemIfSet(SID_SC_INPUT_FMT_EXPAND, false))
        m_xFormatCB->set_active(pExpandItem->GetValue());

    bReadOnly = officecfg::Office::Calc::Input::ExpandFormatting::isReadOnly();
    m_xFormatCB->set_sensitive(!bReadOnly);
    m_xFormatImg->set_visible(bReadOnly);

    if(const SfxBoolItem* pExpandItem = rCoreSet->GetItemIfSet(SID_SC_INPUT_REF_EXPAND, false))
        m_xExpRefCB->set_active(pExpandItem->GetValue());

    bReadOnly = officecfg::Office::Calc::Input::ExpandReference::isReadOnly();
    m_xExpRefCB->set_sensitive(!bReadOnly);
    m_xExpRefImg->set_visible(bReadOnly);

    if (const SfxBoolItem* pUpdateItem = rCoreSet->GetItemIfSet(SID_SC_OPT_SORT_REF_UPDATE))
        m_xSortRefUpdateCB->set_active(pUpdateItem->GetValue());

    bReadOnly = officecfg::Office::Calc::Input::UpdateReferenceOnSort::isReadOnly();
    m_xSortRefUpdateCB->set_sensitive(!bReadOnly);
    m_xSortRefUpdateImg->set_visible(bReadOnly);

    if(const SfxBoolItem* pHeaderItem = rCoreSet->GetItemIfSet(SID_SC_INPUT_MARK_HEADER, false))
        m_xMarkHdrCB->set_active(pHeaderItem->GetValue());

    bReadOnly = officecfg::Office::Calc::Input::HighlightSelection::isReadOnly();
    m_xMarkHdrCB->set_sensitive(!bReadOnly);
    m_xMarkHdrImg->set_visible(bReadOnly);

    if( const SfxBoolItem* pWarnItem = rCoreSet->GetItemIfSet( SID_SC_INPUT_REPLCELLSWARN, false ) )
        m_xReplWarnCB->set_active( pWarnItem->GetValue() );

    bReadOnly = officecfg::Office::Calc::Input::ReplaceCellsWarning::isReadOnly();
    m_xReplWarnCB->set_sensitive(!bReadOnly);
    m_xReplWarnImg->set_visible(bReadOnly);

    if( const SfxBoolItem* pSelectionItem = rCoreSet->GetItemIfSet( SID_SC_INPUT_LEGACY_CELL_SELECTION, false ) )
        m_xLegacyCellSelectionCB->set_active( pSelectionItem->GetValue() );

    bReadOnly = officecfg::Office::Calc::Input::LegacyCellSelection::isReadOnly();
    m_xLegacyCellSelectionCB->set_sensitive(!bReadOnly);
    m_xLegacyCellSelectionImg->set_visible(bReadOnly);

    if( const SfxBoolItem* pPasteModeItem = rCoreSet->GetItemIfSet( SID_SC_INPUT_ENTER_PASTE_MODE, false ) )
        m_xEnterPasteModeCB->set_active( pPasteModeItem->GetValue() );

    bReadOnly = officecfg::Office::Calc::Input::EnterPasteMode::isReadOnly();
    m_xEnterPasteModeCB->set_sensitive(!bReadOnly);
    m_xEnterPasteModeImg->set_visible(bReadOnly);

    m_xAlignCB->save_state();
    m_xAlignLB->save_value();
    m_xEditModeCB->save_state();
    m_xFormatCB->save_state();

    m_xExpRefCB->save_state();
    m_xSortRefUpdateCB->save_state();
    m_xMarkHdrCB->save_state();
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

IMPL_LINK(ScTpLayoutOptions, AlignHdl, weld::Toggleable&, rBox, void)
{
    m_xAlignLB->set_sensitive(rBox.get_active() &&
        !officecfg::Office::Calc::Input::MoveSelectionDirection::isReadOnly());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
