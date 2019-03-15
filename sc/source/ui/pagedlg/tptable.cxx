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

#include <vcl/settings.hxx>
#include <osl/diagnose.h>

#include <tptable.hxx>
#include <global.hxx>
#include <attrib.hxx>
#include <sc.hrc>
#include <bitmaps.hlst>

// Static Data

const sal_uInt16 ScTablePage::pPageTableRanges[] =
{
    ATTR_PAGE_NOTES, ATTR_PAGE_FIRSTPAGENO,
    0
};

static bool lcl_PutVObjModeItem(sal_uInt16  nWhich,
                          SfxItemSet&       rCoreSet,
                          const SfxItemSet& rOldSet,
                          const weld::ToggleButton& rBtn);

static bool lcl_PutScaleItem( sal_uInt16    nWhich,
                       SfxItemSet&          rCoreSet,
                       const SfxItemSet&    rOldSet,
                       const weld::ComboBox& rListBox,
                       sal_uInt16           nLBEntry,
                       const weld::MetricSpinButton& rEd,
                       sal_uInt16           nValue );

static bool lcl_PutScaleItem2( sal_uInt16   nWhich,
                       SfxItemSet&          rCoreSet,
                       const SfxItemSet&    rOldSet,
                       const weld::ComboBox& rListBox,
                       sal_uInt16           nLBEntry,
                       const weld::SpinButton& rEd1,
                       sal_uInt16           nOrigScalePageWidth,
                       const weld::SpinButton& rEd2,
                       sal_uInt16           nOrigScalePageHeight );

static bool lcl_PutScaleItem3( sal_uInt16    nWhich,
                       SfxItemSet&          rCoreSet,
                       const SfxItemSet&    rOldSet,
                       const weld::ComboBox& rListBox,
                       sal_uInt16           nLBEntry,
                       const weld::SpinButton& rEd,
                       sal_uInt16           nValue );

static bool lcl_PutBoolItem( sal_uInt16 nWhich,
                      SfxItemSet&       rCoreSet,
                      const SfxItemSet& rOldSet,
                      bool              bIsChecked,
                      bool              bSavedValue );

#define PAGENO_HDL          LINK(this,ScTablePage,PageNoHdl)
#define PAGEDIR_HDL         LINK(this,ScTablePage,PageDirHdl)

namespace {

bool WAS_DEFAULT(sal_uInt16 w, SfxItemSet const & s)
{ return SfxItemState::DEFAULT==s.GetItemState(w); }

}

#define GET_BOOL(sid,set)   static_cast<const SfxBoolItem&>((set).Get(GetWhich((sid)))).GetValue()
#define GET_USHORT(sid,set) static_cast<const SfxUInt16Item&>((set).Get(GetWhich((sid)))).GetValue()
#define GET_SHOW(sid,set)   ( static_cast<const ScViewObjectModeItem&>((set).Get(GetWhich((sid)))).GetValue() \
                              == VOBJ_MODE_SHOW )
// List box entries "Scaling mode"
#define SC_TPTABLE_SCALE_PERCENT    0
#define SC_TPTABLE_SCALE_TO         1
#define SC_TPTABLE_SCALE_TO_PAGES   2

ScTablePage::ScTablePage(TabPageParent pParent, const SfxItemSet& rCoreAttrs)
    : SfxTabPage(pParent, "modules/scalc/ui/sheetprintpage.ui", "SheetPrintPage", &rCoreAttrs)
    , m_nOrigScalePageWidth(0)
    , m_nOrigScalePageHeight(0)
    , m_xBtnTopDown(m_xBuilder->weld_radio_button("radioBTN_TOPDOWN"))
    , m_xBtnLeftRight(m_xBuilder->weld_radio_button("radioBTN_LEFTRIGHT"))
    , m_xBmpPageDir(m_xBuilder->weld_image("imageBMP_PAGEDIR"))
    , m_xBtnPageNo(m_xBuilder->weld_check_button("checkBTN_PAGENO"))
    , m_xEdPageNo(m_xBuilder->weld_spin_button("spinED_PAGENO"))
    , m_xBtnHeaders(m_xBuilder->weld_check_button("checkBTN_HEADER"))
    , m_xBtnGrid(m_xBuilder->weld_check_button("checkBTN_GRID"))
    , m_xBtnNotes(m_xBuilder->weld_check_button("checkBTN_NOTES"))
    , m_xBtnObjects(m_xBuilder->weld_check_button("checkBTN_OBJECTS"))
    , m_xBtnCharts(m_xBuilder->weld_check_button("checkBTN_CHARTS"))
    , m_xBtnDrawings(m_xBuilder->weld_check_button("checkBTN_DRAWINGS"))
    , m_xBtnFormulas(m_xBuilder->weld_check_button("checkBTN_FORMULAS"))
    , m_xBtnNullVals(m_xBuilder->weld_check_button("checkBTN_NULLVALS"))
    , m_xLbScaleMode(m_xBuilder->weld_combo_box("comboLB_SCALEMODE"))
    , m_xBxScaleAll(m_xBuilder->weld_widget("boxSCALEALL"))
    , m_xEdScaleAll(m_xBuilder->weld_metric_spin_button("spinED_SCALEALL", FieldUnit::PERCENT))
    , m_xGrHeightWidth(m_xBuilder->weld_widget("gridWH"))
    , m_xEdScalePageWidth(m_xBuilder->weld_spin_button("spinED_SCALEPAGEWIDTH"))
    , m_xCbScalePageWidth(m_xBuilder->weld_check_button("labelWP"))
    , m_xEdScalePageHeight(m_xBuilder->weld_spin_button("spinED_SCALEPAGEHEIGHT"))
    , m_xCbScalePageHeight(m_xBuilder->weld_check_button("labelHP"))
    , m_xBxScalePageNum(m_xBuilder->weld_widget("boxNP"))
    , m_xEdScalePageNum(m_xBuilder->weld_spin_button("spinED_SCALEPAGENUM"))
{
    SetExchangeSupport();

    m_xBtnPageNo->connect_toggled(PAGENO_HDL);
    m_xBtnTopDown->connect_toggled(PAGEDIR_HDL);
    m_xBtnLeftRight->connect_toggled(PAGEDIR_HDL);
    m_xLbScaleMode->connect_changed(LINK(this,ScTablePage,ScaleHdl));
    m_xCbScalePageWidth->connect_toggled(LINK(this, ScTablePage, ToggleHdl));
    m_xCbScalePageHeight->connect_toggled(LINK(this, ScTablePage, ToggleHdl));
}

void ScTablePage::ShowImage()
{
    OUString aImg(m_xBtnLeftRight->get_active() ? OUString(BMP_LEFTRIGHT) : OUString(BMP_TOPDOWN));
    m_xBmpPageDir->set_from_icon_name(aImg);
}

ScTablePage::~ScTablePage()
{
    disposeOnce();
}

VclPtr<SfxTabPage> ScTablePage::Create(TabPageParent pParent, const SfxItemSet* rCoreSet)
{
    return VclPtr<ScTablePage>::Create(pParent, *rCoreSet);
}

void ScTablePage::Reset( const SfxItemSet* rCoreSet )
{
    bool    bTopDown = GET_BOOL( SID_SCATTR_PAGE_TOPDOWN, *rCoreSet );
    sal_uInt16  nWhich   = 0;

    // sal_Bool flags
    m_xBtnNotes->set_active( GET_BOOL(SID_SCATTR_PAGE_NOTES,*rCoreSet) );
    m_xBtnGrid->set_active( GET_BOOL(SID_SCATTR_PAGE_GRID,*rCoreSet) );
    m_xBtnHeaders->set_active( GET_BOOL(SID_SCATTR_PAGE_HEADERS,*rCoreSet) );
    m_xBtnFormulas->set_active( GET_BOOL(SID_SCATTR_PAGE_FORMULAS,*rCoreSet) );
    m_xBtnNullVals->set_active( GET_BOOL(SID_SCATTR_PAGE_NULLVALS,*rCoreSet) );
    m_xBtnTopDown->set_active( bTopDown );
    m_xBtnLeftRight->set_active( !bTopDown );

    // first printed page:
    sal_uInt16 nPage = GET_USHORT(SID_SCATTR_PAGE_FIRSTPAGENO,*rCoreSet);
    m_xBtnPageNo->set_active( nPage != 0 );
    m_xEdPageNo->set_value( (nPage != 0) ? nPage : 1 );
    PageNoHdl(nullptr);

    // object representation:
    m_xBtnCharts->set_active( GET_SHOW( SID_SCATTR_PAGE_CHARTS, *rCoreSet ) );
    m_xBtnObjects->set_active( GET_SHOW( SID_SCATTR_PAGE_OBJECTS, *rCoreSet ) );
    m_xBtnDrawings->set_active( GET_SHOW( SID_SCATTR_PAGE_DRAWINGS, *rCoreSet ) );

    // scaling:
    nWhich = GetWhich(SID_SCATTR_PAGE_SCALE);
    if ( rCoreSet->GetItemState( nWhich ) >= SfxItemState::DEFAULT )
    {
        sal_uInt16 nScale = static_cast<const SfxUInt16Item&>(rCoreSet->Get(nWhich)).GetValue();
        if( nScale > 0 )
            m_xLbScaleMode->set_active(SC_TPTABLE_SCALE_PERCENT);
        m_xEdScaleAll->set_value((nScale > 0) ? nScale : 100, FieldUnit::PERCENT);
    }

    nWhich = GetWhich(SID_SCATTR_PAGE_SCALETO);
    if ( rCoreSet->GetItemState( nWhich ) >= SfxItemState::DEFAULT )
    {
        const ScPageScaleToItem& rItem = static_cast< const ScPageScaleToItem& >( rCoreSet->Get( nWhich ) );
        sal_uInt16 nWidth = rItem.GetWidth();
        sal_uInt16 nHeight = rItem.GetHeight();

        /*  width==0 and height==0 is invalid state, used as "not selected".
            Dialog shows width=height=1 then. */
        if (nWidth || nHeight)
            m_xLbScaleMode->set_active(SC_TPTABLE_SCALE_TO);
        else
            nWidth = nHeight = 1;

        if (nWidth)
            m_xEdScalePageWidth->set_value(nWidth);
        else
            m_xEdScalePageWidth->set_text(OUString());

        m_xEdScalePageWidth->set_sensitive(nWidth != 0);
        m_xCbScalePageWidth->set_active(nWidth != 0);

        if(nHeight)
            m_xEdScalePageHeight->set_value(nHeight);
        else
            m_xEdScalePageHeight->set_text(OUString());

        m_xEdScalePageHeight->set_sensitive(nHeight != 0);
        m_xCbScalePageHeight->set_active(nHeight != 0);
    }

    nWhich = GetWhich(SID_SCATTR_PAGE_SCALETOPAGES);
    if ( rCoreSet->GetItemState( nWhich ) >= SfxItemState::DEFAULT )
    {
        sal_uInt16 nPages = static_cast<const SfxUInt16Item&>(rCoreSet->Get(nWhich)).GetValue();
        if( nPages > 0 )
            m_xLbScaleMode->set_active(SC_TPTABLE_SCALE_TO_PAGES);
        m_xEdScalePageNum->set_value( (nPages > 0) ? nPages : 1 );
    }

    if (m_xLbScaleMode->get_active() == 0)
    {
        // fall back to 100%
        OSL_FAIL( "ScTablePage::Reset - missing scaling item" );
        m_xLbScaleMode->set_active(SC_TPTABLE_SCALE_PERCENT);
        m_xEdScaleAll->set_value(100, FieldUnit::PERCENT);
    }

    PageDirHdl(*m_xBtnTopDown);
    ScaleHdl(*m_xLbScaleMode);

    // remember for FillItemSet
    m_xBtnFormulas->save_state();
    m_xBtnNullVals->save_state();
    m_xBtnNotes->save_state();
    m_xBtnGrid->save_state();
    m_xBtnHeaders->save_state();
    m_xBtnTopDown->save_state();
    m_xBtnLeftRight->save_state();
    m_xLbScaleMode->save_value();
    m_xBtnCharts->save_state();
    m_xBtnObjects->save_state();
    m_xBtnDrawings->save_state();
    m_xBtnPageNo->save_state();
    m_xEdPageNo->save_value();
    m_xEdScaleAll->save_value();
    m_nOrigScalePageWidth = m_xEdScalePageWidth->get_sensitive() ? m_xEdScalePageWidth->get_value() : 0;
    m_nOrigScalePageHeight = m_xEdScalePageHeight->get_sensitive() ? m_xEdScalePageHeight->get_value() : 0;
    m_xEdScalePageNum->save_value();
}

bool ScTablePage::FillItemSet( SfxItemSet* rCoreSet )
{
    const SfxItemSet&   rOldSet      = GetItemSet();
    sal_uInt16              nWhichPageNo = GetWhich(SID_SCATTR_PAGE_FIRSTPAGENO);
    bool                bDataChanged = false;

    // sal_Bool flags
    bDataChanged |= lcl_PutBoolItem( GetWhich(SID_SCATTR_PAGE_NOTES),
                                     *rCoreSet, rOldSet,
                                     m_xBtnNotes->get_active(),
                                     m_xBtnNotes->get_saved_state() != TRISTATE_FALSE );

    bDataChanged |= lcl_PutBoolItem( GetWhich(SID_SCATTR_PAGE_GRID),
                                     *rCoreSet, rOldSet,
                                     m_xBtnGrid->get_active(),
                                     m_xBtnGrid->get_saved_state() != TRISTATE_FALSE );

    bDataChanged |= lcl_PutBoolItem( GetWhich(SID_SCATTR_PAGE_HEADERS),
                                     *rCoreSet, rOldSet,
                                     m_xBtnHeaders->get_active(),
                                     m_xBtnHeaders->get_saved_state() != TRISTATE_FALSE );

    bDataChanged |= lcl_PutBoolItem( GetWhich(SID_SCATTR_PAGE_TOPDOWN),
                                     *rCoreSet, rOldSet,
                                     m_xBtnTopDown->get_active(),
                                     m_xBtnTopDown->get_saved_state() != TRISTATE_FALSE );

    bDataChanged |= lcl_PutBoolItem( GetWhich(SID_SCATTR_PAGE_FORMULAS),
                                     *rCoreSet, rOldSet,
                                     m_xBtnFormulas->get_active(),
                                     m_xBtnFormulas->get_saved_state() != TRISTATE_FALSE );

    bDataChanged |= lcl_PutBoolItem( GetWhich(SID_SCATTR_PAGE_NULLVALS),
                                     *rCoreSet, rOldSet,
                                     m_xBtnNullVals->get_active(),
                                     m_xBtnNullVals->get_saved_state() != TRISTATE_FALSE );

    // first printed page:
    bool bUseValue = m_xBtnPageNo->get_active();

    if (   WAS_DEFAULT(nWhichPageNo,rOldSet)
        && (    (!bUseValue && 0 == m_xBtnPageNo->get_saved_state())
            || (   bUseValue && 1 == m_xBtnPageNo->get_saved_state()
                   && ! m_xEdPageNo->get_value_changed_from_saved() ) ) )
    {
            rCoreSet->ClearItem( nWhichPageNo );
    }
    else
    {
        sal_uInt16 nPage = static_cast<sal_uInt16>( m_xBtnPageNo->get_active()
                                    ? m_xEdPageNo->get_value()
                                    : 0 );

        rCoreSet->Put( SfxUInt16Item( nWhichPageNo, nPage ) );
        bDataChanged = true;
    }

    // object representation:
    bDataChanged |= lcl_PutVObjModeItem( GetWhich(SID_SCATTR_PAGE_CHARTS),
                                         *rCoreSet, rOldSet, *m_xBtnCharts );

    bDataChanged |= lcl_PutVObjModeItem( GetWhich(SID_SCATTR_PAGE_OBJECTS),
                                         *rCoreSet, rOldSet, *m_xBtnObjects );

    bDataChanged |= lcl_PutVObjModeItem( GetWhich(SID_SCATTR_PAGE_DRAWINGS),
                                         *rCoreSet, rOldSet, *m_xBtnDrawings );

    // scaling:
    if( !m_xEdScalePageWidth->get_sensitive() && !m_xEdScalePageHeight->get_sensitive() )
    {
        m_xLbScaleMode->set_active(SC_TPTABLE_SCALE_PERCENT);
        m_xEdScaleAll->set_value(100, FieldUnit::PERCENT);
    }

    bDataChanged |= lcl_PutScaleItem( GetWhich(SID_SCATTR_PAGE_SCALE),
                                      *rCoreSet, rOldSet,
                                      *m_xLbScaleMode, SC_TPTABLE_SCALE_PERCENT,
                                      *m_xEdScaleAll, static_cast<sal_uInt16>(m_xEdScaleAll->get_value(FieldUnit::PERCENT)) );

    bDataChanged |= lcl_PutScaleItem2( GetWhich(SID_SCATTR_PAGE_SCALETO),
                                      *rCoreSet, rOldSet,
                                      *m_xLbScaleMode, SC_TPTABLE_SCALE_TO,
                                      *m_xEdScalePageWidth, m_nOrigScalePageWidth,
                                      *m_xEdScalePageHeight, m_nOrigScalePageHeight );

    bDataChanged |= lcl_PutScaleItem3( GetWhich(SID_SCATTR_PAGE_SCALETOPAGES),
                                      *rCoreSet, rOldSet,
                                      *m_xLbScaleMode, SC_TPTABLE_SCALE_TO_PAGES,
                                      *m_xEdScalePageNum, static_cast<sal_uInt16>(m_xEdScalePageNum->get_value()) );

    return bDataChanged;
}

DeactivateRC ScTablePage::DeactivatePage( SfxItemSet* pSetP )
{
    if ( pSetP )
        FillItemSet( pSetP );

    return DeactivateRC::LeavePage;
}

void ScTablePage::DataChanged( const DataChangedEvent& rDCEvt )
{
    if( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
        ShowImage();
    SfxTabPage::DataChanged( rDCEvt );
}

// Handler:

IMPL_LINK_NOARG(ScTablePage, PageDirHdl, weld::ToggleButton&, void)
{
    ShowImage();
}

IMPL_LINK(ScTablePage, PageNoHdl, weld::ToggleButton&, rBtn, void)
{
    PageNoHdl(&rBtn);
}

void ScTablePage::PageNoHdl(const weld::ToggleButton* pBtn)
{
    if (m_xBtnPageNo->get_active())
    {
        m_xEdPageNo->set_sensitive(true);
        if (pBtn)
            m_xEdPageNo->grab_focus();
    }
    else
        m_xEdPageNo->set_sensitive(false);
}

IMPL_LINK_NOARG(ScTablePage, ScaleHdl, weld::ComboBox&, void)
{
    // controls for Box "Reduce/enlarge"
    m_xBxScaleAll->set_visible(m_xLbScaleMode->get_active() == SC_TPTABLE_SCALE_PERCENT);

    // controls for Grid "Scale to width/height"
    m_xGrHeightWidth->set_visible(m_xLbScaleMode->get_active() == SC_TPTABLE_SCALE_TO);

    // controls for Box "Scale to pages"
    m_xBxScalePageNum->set_visible(m_xLbScaleMode->get_active() == SC_TPTABLE_SCALE_TO_PAGES);
}

IMPL_LINK(ScTablePage, ToggleHdl, weld::ToggleButton&, rBox, void)
{
    if (&rBox == m_xCbScalePageWidth.get())
    {
        if (!rBox.get_active())
        {
            m_xEdScalePageWidth->set_text(OUString());
            m_xEdScalePageWidth->set_sensitive(false);
        }
        else
        {
            m_xEdScalePageWidth->set_value(1);
            m_xEdScalePageWidth->set_sensitive(true);
        }
    }
    else
    {
        if (!rBox.get_active())
        {
            m_xEdScalePageHeight->set_text(OUString());
            m_xEdScalePageHeight->set_sensitive(false);
        }
        else
        {
            m_xEdScalePageHeight->set_value(1);
            m_xEdScalePageHeight->set_sensitive(true);
        }
    }
}

// Helper functions for FillItemSet:

static bool lcl_PutBoolItem( sal_uInt16            nWhich,
                     SfxItemSet&        rCoreSet,
                     const SfxItemSet&  rOldSet,
                     bool               bIsChecked,
                     bool               bSavedValue )
{
    bool bDataChanged = (   bSavedValue == bIsChecked
                         && WAS_DEFAULT(nWhich,rOldSet) );

    if ( bDataChanged )
        rCoreSet.ClearItem(nWhich);
    else
        rCoreSet.Put( SfxBoolItem( nWhich, bIsChecked ) );

    return bDataChanged;
}

static bool lcl_PutVObjModeItem( sal_uInt16            nWhich,
                         SfxItemSet&        rCoreSet,
                         const SfxItemSet&  rOldSet,
                         const weld::ToggleButton&    rBtn )
{
    bool bIsChecked   = rBtn.get_active();
    bool bDataChanged =     rBtn.get_saved_state() == (bIsChecked ? 1 : 0)
                         && WAS_DEFAULT(nWhich,rOldSet);

    if ( bDataChanged )
        rCoreSet.ClearItem( nWhich );

    else
        rCoreSet.Put( ScViewObjectModeItem( nWhich, bIsChecked
                                                    ? VOBJ_MODE_SHOW
                                                    : VOBJ_MODE_HIDE ) );
    return bDataChanged;
}

static bool lcl_PutScaleItem( sal_uInt16    nWhich,
                      SfxItemSet&           rCoreSet,
                      const SfxItemSet&     rOldSet,
                      const weld::ComboBox& rListBox,
                      sal_uInt16            nLBEntry,
                      const weld::MetricSpinButton& rEd,
                      sal_uInt16            nValue )
{
    bool bIsSel = (rListBox.get_active() == nLBEntry);
    bool bDataChanged = (rListBox.get_value_changed_from_saved()) ||
                        rEd.get_value_changed_from_saved() ||
                        !WAS_DEFAULT( nWhich, rOldSet );

    if( bDataChanged )
        rCoreSet.Put( SfxUInt16Item( nWhich, bIsSel ? nValue : 0 ) );
    else
        rCoreSet.ClearItem( nWhich );

    return bDataChanged;
}

static bool lcl_PutScaleItem2( sal_uInt16               nWhich,
                      SfxItemSet&           rCoreSet,
                      const SfxItemSet&     rOldSet,
                      const weld::ComboBox& rListBox,
                      sal_uInt16            nLBEntry,
                      const weld::SpinButton& rEd1,
                      sal_uInt16            nOrigScalePageWidth,
                      const weld::SpinButton& rEd2,
                      sal_uInt16            nOrigScalePageHeight )
{
    sal_uInt16 nValue1 = rEd1.get_sensitive() ? rEd1.get_value() : 0;
    sal_uInt16 nValue2 = rEd2.get_sensitive() ? rEd2.get_value() : 0;
    bool bIsSel = (rListBox.get_active() == nLBEntry);
    bool bDataChanged = (rListBox.get_value_changed_from_saved()) ||
                        nValue1 != nOrigScalePageWidth ||
                        nValue1 != nOrigScalePageHeight ||
                        !WAS_DEFAULT( nWhich, rOldSet );

    if( bDataChanged )
    {
        ScPageScaleToItem aItem;
        if( bIsSel )
            aItem.Set( nValue1, nValue2 );
        rCoreSet.Put( aItem );
    }
    else
        rCoreSet.ClearItem( nWhich );

    return bDataChanged;
}

static bool lcl_PutScaleItem3( sal_uInt16    nWhich,
                      SfxItemSet&           rCoreSet,
                      const SfxItemSet&     rOldSet,
                      const weld::ComboBox& rListBox,
                      sal_uInt16            nLBEntry,
                      const weld::SpinButton& rEd,
                      sal_uInt16            nValue )
{
    bool bIsSel = (rListBox.get_active() == nLBEntry);
    bool bDataChanged = (rListBox.get_value_changed_from_saved()) ||
                        rEd.get_value_changed_from_saved() ||
                        !WAS_DEFAULT( nWhich, rOldSet );

    if( bDataChanged )
        rCoreSet.Put( SfxUInt16Item( nWhich, bIsSel ? nValue : 0 ) );
    else
        rCoreSet.ClearItem( nWhich );

    return bDataChanged;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
