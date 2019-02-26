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

#include <sfx2/app.hxx>
#include <svtools/ruler.hxx>
#include <svx/dialogs.hrc>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>

#include <editeng/lrspitem.hxx>
#include <tabstpge.hxx>
#include <svx/dlgutil.hxx>
#include <sfx2/module.hxx>
#include <svl/cjkoptions.hxx>
#include <unotools/localedatawrapper.hxx>
#include <sfx2/request.hxx>
#include <svl/intitem.hxx>

constexpr FieldUnit eDefUnit = FieldUnit::MM_100TH;

const sal_uInt16 SvxTabulatorTabPage::pRanges[] =
{
    SID_ATTR_TABSTOP,
    SID_ATTR_TABSTOP_OFFSET,
    0
};

static void FillUpWithDefTabs_Impl( long nDefDist, SvxTabStopItem& rTabs )
{
    if( rTabs.Count() )
        return;
    {
        SvxTabStop aSwTabStop( nDefDist, SvxTabAdjust::Default );
        rTabs.Insert( aSwTabStop );
    }
}

void TabWin_Impl::Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle&)
{
    // Paint tabulators
    Point aPoint;
    Size aSize(GetOutputSizePixel());
    aPoint.setX( aSize.Width() / 2 );
    aPoint.setY( aSize.Height() / 2 );
    Ruler::DrawTab(rRenderContext, rRenderContext.GetSettings().GetStyleSettings().GetFontColor(), aPoint, nTabStyle);
}

SvxTabulatorTabPage::SvxTabulatorTabPage(TabPageParent pParent, const SfxItemSet& rAttr)
    : SfxTabPage(pParent, "cui/ui/paratabspage.ui", "ParagraphTabsPage", &rAttr)
    , aCurrentTab(0)
    , aNewTabs(0, 0, SvxTabAdjust::Left, GetWhich(SID_ATTR_TABSTOP))
    , nDefDist(0)
    , m_xTabSpin(m_xBuilder->weld_metric_spin_button("SP_TABPOS", FieldUnit::CM))
    , m_xTabBox(m_xBuilder->weld_entry_tree_view("tabgrid", "ED_TABPOS", "LB_TABPOS"))
    , m_xCenterTab(m_xBuilder->weld_radio_button("radiobuttonBTN_TABTYPE_CENTER"))
    , m_xDezTab(m_xBuilder->weld_radio_button("radiobuttonBTN_TABTYPE_DECIMAL"))
    , m_xDezChar(m_xBuilder->weld_entry("entryED_TABTYPE_DECCHAR"))
    , m_xDezCharLabel(m_xBuilder->weld_label("labelFT_TABTYPE_DECCHAR"))
    // lower radio buttons
    , m_xNoFillChar(m_xBuilder->weld_radio_button("radiobuttonBTN_FILLCHAR_NO"))
    , m_xFillPoints(m_xBuilder->weld_radio_button("radiobuttonBTN_FILLCHAR_POINTS"))
    , m_xFillDashLine(m_xBuilder->weld_radio_button("radiobuttonBTN_FILLCHAR_DASHLINE"))
    , m_xFillSolidLine(m_xBuilder->weld_radio_button("radiobuttonBTN_FILLCHAR_UNDERSCORE"))
    , m_xFillSpecial(m_xBuilder->weld_radio_button("radiobuttonBTN_FILLCHAR_OTHER"))
    , m_xFillChar(m_xBuilder->weld_entry("entryED_FILLCHAR_OTHER"))
    // button bar
    , m_xNewBtn(m_xBuilder->weld_button("buttonBTN_NEW"))
    , m_xDelAllBtn(m_xBuilder->weld_button("buttonBTN_DELALL"))
    , m_xDelBtn(m_xBuilder->weld_button("buttonBTN_DEL"))
    , m_xTypeFrame(m_xBuilder->weld_container("frameFL_TABTYPE"))
    , m_xFillFrame(m_xBuilder->weld_container("frameFL_FILLCHAR"))
    // the tab images
    , m_xLeftWin(new weld::CustomWeld(*m_xBuilder, "drawingareaWIN_TABLEFT", m_aLeftWin))
    , m_xRightWin(new weld::CustomWeld(*m_xBuilder, "drawingareaWIN_TABRIGHT", m_aRightWin))
    , m_xCenterWin(new weld::CustomWeld(*m_xBuilder, "drawingareaWIN_TABCENTER", m_aCenterWin))
    , m_xDezWin(new weld::CustomWeld(*m_xBuilder, "drawingareaWIN_TABDECIMAL", m_aDezWin))
{
    m_aLeftWin.SetTabStyle(sal_uInt16(RULER_TAB_LEFT|WB_HORZ));
    m_aRightWin.SetTabStyle(sal_uInt16(RULER_TAB_RIGHT|WB_HORZ));
    m_aCenterWin.SetTabStyle(sal_uInt16(RULER_TAB_CENTER|WB_HORZ));
    m_aDezWin.SetTabStyle(sal_uInt16(RULER_TAB_DECIMAL|WB_HORZ));
    //upper radiobuttons
    SvtCJKOptions aCJKOptions;
    m_xLeftTab = m_xBuilder->weld_radio_button(aCJKOptions.IsAsianTypographyEnabled() ? "radiobuttonST_LEFTTAB_ASIAN" : "radiobuttonBTN_TABTYPE_LEFT");
    m_xRightTab = m_xBuilder->weld_radio_button(aCJKOptions.IsAsianTypographyEnabled() ? "radiobuttonST_RIGHTTAB_ASIAN" : "radiobuttonBTN_TABTYPE_RIGHT");
    m_xLeftTab->show();
    m_xRightTab->show();

    // This page needs ExchangeSupport
    SetExchangeSupport();

    // Set metric
    FieldUnit eFUnit = GetModuleFieldUnit( rAttr );
    SetFieldUnit(*m_xTabSpin, eFUnit);

    // Initialize buttons
    m_xNewBtn->connect_clicked(LINK(this,SvxTabulatorTabPage, NewHdl_Impl));
    m_xDelBtn->connect_clicked(LINK(this,SvxTabulatorTabPage, DelHdl_Impl));
    m_xDelAllBtn->connect_clicked(LINK(this,SvxTabulatorTabPage, DelAllHdl_Impl));

    Link<weld::Button&,void> aLink = LINK(this, SvxTabulatorTabPage, TabTypeCheckHdl_Impl);
    m_xLeftTab->connect_clicked(aLink);
    m_xRightTab->connect_clicked(aLink);
    m_xDezTab->connect_clicked(aLink);
    m_xCenterTab->connect_clicked(aLink);

    m_xDezChar->connect_focus_out(LINK(this,  SvxTabulatorTabPage, GetDezCharHdl_Impl));
    m_xDezChar->set_sensitive(false);
    m_xDezCharLabel->set_sensitive(false);

    aLink = LINK(this, SvxTabulatorTabPage, FillTypeCheckHdl_Impl);
    m_xNoFillChar->connect_clicked(aLink);
    m_xFillPoints->connect_clicked(aLink);
    m_xFillDashLine->connect_clicked(aLink);
    m_xFillSolidLine->connect_clicked(aLink);
    m_xFillSpecial->connect_clicked(aLink);
    m_xFillChar->connect_focus_out(LINK(this,  SvxTabulatorTabPage, GetFillCharHdl_Impl));
    m_xFillChar->set_sensitive(false);

    m_xTabBox->connect_row_activated(LINK(this, SvxTabulatorTabPage, SelectHdl_Impl));
    m_xTabBox->connect_changed(LINK(this, SvxTabulatorTabPage, ModifyHdl_Impl));
    m_xTabBox->connect_focus_out(LINK(this,  SvxTabulatorTabPage, ReformatHdl_Impl));

    // Get the default decimal char from the system
    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    aCurrentTab.GetDecimal() = rLocaleWrapper.getNumDecimalSep()[0];
}

SvxTabulatorTabPage::~SvxTabulatorTabPage()
{
    disposeOnce();
}

void SvxTabulatorTabPage::dispose()
{
    m_xDezWin.reset();
    m_xCenterWin.reset();
    m_xRightWin.reset();
    m_xLeftWin.reset();
    m_xFillChar.reset();
    m_xDezChar.reset();
    m_xTabBox.reset();
    SfxTabPage::dispose();
}

bool SvxTabulatorTabPage::FillItemSet(SfxItemSet* rSet)
{
    bool bModified = false;

    // Put the controls' values in here
    if (m_xNewBtn->get_sensitive())
        NewHdl_Impl(nullptr);

    // Call the LoseFocus-Handler first
    GetDezCharHdl_Impl(*m_xDezChar);
    GetFillCharHdl_Impl(*m_xFillChar);

    FillUpWithDefTabs_Impl(nDefDist, aNewTabs);
    SfxItemPool* pPool = rSet->GetPool();
    MapUnit eUnit = pPool->GetMetric(GetWhich(SID_ATTR_TABSTOP));
    const SfxPoolItem* pOld = GetOldItem(*rSet, SID_ATTR_TABSTOP);

    if (MapUnit::Map100thMM != eUnit)
    {
        // If the ItemSet contains a LRSpaceItem with negative first line indent,
        // the TabStopItem needs to have a DefTab at position 0.
        const SfxPoolItem* pLRSpace;
        // If not in the new set, then maybe in the old one
        if (SfxItemState::SET != rSet->GetItemState(GetWhich(SID_ATTR_LRSPACE), true, &pLRSpace))
            pLRSpace = GetOldItem(*rSet, SID_ATTR_LRSPACE);

        if (pLRSpace && static_cast<const SvxLRSpaceItem*>(pLRSpace)->GetTextFirstLineOfst() < 0)
        {
            SvxTabStop aNull(0, SvxTabAdjust::Default);
            aNewTabs.Insert(aNull);
        }

        SvxTabStopItem aTmp(aNewTabs);
        aTmp.Remove(0, aTmp.Count());

        for (sal_uInt16 i = 0; i < aNewTabs.Count(); ++i)
        {
            SvxTabStop aTmpStop = aNewTabs[i];
            aTmpStop.GetTabPos() = LogicToLogic(aTmpStop.GetTabPos(), MapUnit::Map100thMM, eUnit);
            aTmp.Insert(aTmpStop);
        }

        if (!pOld || *static_cast<const SvxTabStopItem*>(pOld) != aTmp)
        {
            rSet->Put(aTmp);
            bModified = true;
        }
    }
    else if (!pOld || *static_cast<const SvxTabStopItem*>(pOld) != aNewTabs)
    {
        rSet->Put(aNewTabs);
        bModified = true;
    }

    return bModified;
}

VclPtr<SfxTabPage> SvxTabulatorTabPage::Create(TabPageParent pParent, const SfxItemSet* rSet)
{
    return VclPtr<SvxTabulatorTabPage>::Create(pParent, *rSet);
}

void SvxTabulatorTabPage::Reset(const SfxItemSet* rSet)
{
    SfxItemPool* pPool = rSet->GetPool();
    MapUnit eUnit = pPool->GetMetric(GetWhich(SID_ATTR_TABSTOP));

    // Current tabs
    const SfxPoolItem* pItem = GetItem(*rSet, SID_ATTR_TABSTOP);

    if (pItem)
    {
        if (MapUnit::Map100thMM != eUnit)
        {
            SvxTabStopItem aTmp(*static_cast<const SvxTabStopItem*>(pItem));
            aNewTabs.Remove(0, aNewTabs.Count());

            for (sal_uInt16 i = 0; i < aTmp.Count(); ++i)
            {
                SvxTabStop aTmpStop = aTmp[i];
                aTmpStop.GetTabPos() = LogicToLogic(aTmpStop.GetTabPos(), eUnit, MapUnit::Map100thMM);
                aNewTabs.Insert(aTmpStop);
            }
        }
        else
            aNewTabs = *static_cast<const SvxTabStopItem*>(pItem);
    }
    else
        aNewTabs.Remove(0, aNewTabs.Count());

    // Default tab distance
    nDefDist = SVX_TAB_DEFDIST;
    pItem = GetItem(*rSet, SID_ATTR_TABSTOP_DEFAULTS);

    if (pItem)
        nDefDist = LogicToLogic(long(static_cast<const SfxUInt16Item*>(pItem)->GetValue()), eUnit, MapUnit::Map100thMM);

    // Tab pos currently selected
    sal_uInt16 nTabPos = 0;
    pItem = GetItem(*rSet, SID_ATTR_TABSTOP_POS);

    if (pItem)
        nTabPos = static_cast<const SfxUInt16Item*>(pItem)->GetValue();

    InitTabPos_Impl(nTabPos);
}

void SvxTabulatorTabPage::DisableControls(const TabulatorDisableFlags nFlag)
{
    if (TabulatorDisableFlags::TypeLeft & nFlag)
    {
        m_xLeftTab->set_sensitive(false);
        m_xLeftWin->set_sensitive(false);
    }
    if (TabulatorDisableFlags::TypeRight & nFlag)
    {
        m_xRightTab->set_sensitive(false);
        m_xRightWin->set_sensitive(false);
    }
    if (TabulatorDisableFlags::TypeCenter & nFlag)
    {
        m_xCenterTab->set_sensitive(false);
        m_xCenterWin->set_sensitive(false);
    }
    if (TabulatorDisableFlags::TypeDecimal & nFlag)
    {
        m_xDezTab->set_sensitive(false);
        m_xDezWin->set_sensitive(false);
        m_xDezCharLabel->set_sensitive(false);
        m_xDezChar->set_sensitive(false);
    }
    if (TabulatorDisableFlags::TypeMask & nFlag)
        m_xTypeFrame->set_sensitive(false);
    if (TabulatorDisableFlags::FillNone & nFlag)
        m_xNoFillChar->set_sensitive(false);
    if (TabulatorDisableFlags::FillPoint & nFlag)
        m_xFillPoints->set_sensitive(false);
    if (TabulatorDisableFlags::FillDashLine & nFlag)
        m_xFillDashLine->set_sensitive(false);
    if (TabulatorDisableFlags::FillSolidLine & nFlag)
        m_xFillSolidLine->set_sensitive(false);
    if (TabulatorDisableFlags::FillSpecial & nFlag)
    {
        m_xFillSpecial->set_sensitive(false);
        m_xFillChar->set_sensitive(false);
    }
    if (TabulatorDisableFlags::FillMask & nFlag)
        m_xFillFrame->set_sensitive(false);
}

DeactivateRC SvxTabulatorTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( _pSet );
    return DeactivateRC::LeavePage;
}

void SvxTabulatorTabPage::InitTabPos_Impl( sal_uInt16 nTabPos )
{
    m_xTabBox->clear();

    long nOffset = 0;
    const SfxPoolItem* pItem = nullptr;
    if (GetItemSet().GetItemState(SID_ATTR_TABSTOP_OFFSET, true, &pItem) == SfxItemState::SET)
    {
        nOffset = static_cast<const SfxInt32Item*>(pItem)->GetValue();
        MapUnit eUnit = GetItemSet().GetPool()->GetMetric(GetWhich(SID_ATTR_TABSTOP));
        nOffset = OutputDevice::LogicToLogic(nOffset, eUnit, MapUnit::Map100thMM);
    }

    // Correct current TabPos and default tabs
    for ( sal_uInt16 i = 0; i < aNewTabs.Count(); i++ )
    {
        if ( aNewTabs[i].GetAdjustment() != SvxTabAdjust::Default )
        {
            m_xTabSpin->set_value(m_xTabSpin->normalize(
                aNewTabs[i].GetTabPos() + nOffset ), eDefUnit);
            m_xTabBox->append_text(m_xTabSpin->get_text());
        }
        else
            aNewTabs.Remove( i-- );
    }

    // Select current tabulator
    const sal_uInt16 nSize = aNewTabs.Count();

    if ( nTabPos >= nSize )
        nTabPos = 0;

    // Switch off all RadioButtons for a start
    m_xLeftTab->set_active(true);
    m_xNoFillChar->set_active(true);

    if (m_xTabBox->get_count() > 0)
    {
        m_xTabBox->set_active(nTabPos);
        aCurrentTab = aNewTabs[nTabPos];

        SetFillAndTabType_Impl();
        m_xNewBtn->set_sensitive(false);
        m_xDelBtn->set_sensitive(true);
    }
    else
    {   // If no entry, 0 is the default value
        m_xTabSpin->set_value(0, eDefUnit);
        m_xTabBox->set_entry_text(m_xTabSpin->get_text());

        m_xNewBtn->set_sensitive(true);
        m_xDelBtn->set_sensitive(false);
    }
}

void SvxTabulatorTabPage::SetFillAndTabType_Impl()
{
    weld::RadioButton* pTypeBtn = nullptr;
    weld::RadioButton* pFillBtn = nullptr;

    m_xDezChar->set_sensitive(false);
    m_xDezCharLabel->set_sensitive(false);

    if ( aCurrentTab.GetAdjustment() == SvxTabAdjust::Left )
        pTypeBtn = m_xLeftTab.get();
    else if ( aCurrentTab.GetAdjustment() == SvxTabAdjust::Right )
        pTypeBtn = m_xRightTab.get();
    else if ( aCurrentTab.GetAdjustment() == SvxTabAdjust::Decimal )
    {
        pTypeBtn = m_xDezTab.get();
        m_xDezChar->set_sensitive(true);
        m_xDezCharLabel->set_sensitive(true);
        m_xDezChar->set_text(OUString(aCurrentTab.GetDecimal()));
    }
    else if ( aCurrentTab.GetAdjustment() == SvxTabAdjust::Center )
        pTypeBtn = m_xCenterTab.get();

    if (pTypeBtn)
        pTypeBtn->set_active(true);

    m_xFillChar->set_sensitive(false);
    m_xFillChar->set_text("");

    if ( aCurrentTab.GetFill() == ' ' )
        pFillBtn = m_xNoFillChar.get();
    else if ( aCurrentTab.GetFill() == '-' )
        pFillBtn = m_xFillDashLine.get();
    else if ( aCurrentTab.GetFill() == '_' )
        pFillBtn = m_xFillSolidLine.get();
    else if ( aCurrentTab.GetFill() == '.' )
        pFillBtn = m_xFillPoints.get();
    else
    {
        pFillBtn = m_xFillSpecial.get();
        m_xFillChar->set_sensitive(true);
        m_xFillChar->set_text(OUString(aCurrentTab.GetFill()));
    }
    pFillBtn->set_active(true);
}

IMPL_LINK(SvxTabulatorTabPage, NewHdl_Impl, weld::Button&, rBtn, void)
{
    NewHdl_Impl(&rBtn);
}

void SvxTabulatorTabPage::NewHdl_Impl(const weld::Button* pBtn)
{
    // Add a new one and select it
    // Get the value from the display
    ReformatHdl_Impl(*m_xTabBox);
    m_xTabSpin->set_text(m_xTabBox->get_active_text());
    auto nVal = m_xTabSpin->denormalize(m_xTabSpin->get_value(eDefUnit));

    // If the pBtn == 0 && the value == 0 then do not create a tab, because we create via OK
    if (nVal == 0 && pBtn == nullptr)
        return;

    long nOffset = 0;
    const SfxPoolItem* pItem = nullptr;

    if ( GetItemSet().GetItemState( SID_ATTR_TABSTOP_OFFSET, true, &pItem ) ==
         SfxItemState::SET )
    {
        nOffset = static_cast<const SfxInt32Item*>(pItem)->GetValue();
        MapUnit eUnit = GetItemSet().GetPool()->GetMetric( GetWhich( SID_ATTR_TABSTOP ) );
        nOffset = OutputDevice::LogicToLogic( nOffset, eUnit, MapUnit::Map100thMM  );
    }
    const long nReal = nVal - nOffset;
    sal_Int32 nSize = m_xTabBox->get_count();

    sal_Int32 i;
    for( i = 0; i < nSize; i++ )
    {
        if ( nReal < aNewTabs[i].GetTabPos() )
            break;
    }

    // Make ListBox entry
    m_xTabSpin->set_value(m_xTabSpin->normalize(nVal), eDefUnit);
    m_xTabBox->insert_text(i, m_xTabSpin->get_text());

    aCurrentTab.GetTabPos() = nReal;
    SvxTabAdjust eAdj = SvxTabAdjust::Left;

    if (m_xRightTab->get_active())
        eAdj = SvxTabAdjust::Right;
    else if (m_xCenterTab->get_active())
        eAdj = SvxTabAdjust::Center;
    else if (m_xDezTab->get_active())
        eAdj = SvxTabAdjust::Decimal;

    aCurrentTab.GetAdjustment() = eAdj;
    aNewTabs.Insert( aCurrentTab );

    m_xNewBtn->set_sensitive(false);
    m_xDelBtn->set_sensitive(true);
    m_xTabBox->grab_focus();

    // Set the selection into the position Edit
    m_xTabBox->select_entry_region(0, -1);
}

int SvxTabulatorTabPage::FindCurrentTab()
{
    return m_xTabBox->find_text(FormatTab());
}

IMPL_LINK_NOARG(SvxTabulatorTabPage, DelHdl_Impl, weld::Button&, void)
{
    int nPos = FindCurrentTab();
    if (nPos == -1)
        return;

    if (m_xTabBox->get_count() == 1)
    {
        DelAllHdl_Impl(*m_xDelAllBtn);
        return;
    }

    // Delete Tab
    m_xTabBox->remove(nPos);
    aNewTabs.Remove( nPos );

    // Reset aCurrentTab
    const sal_uInt16 nSize = aNewTabs.Count();

    if ( nSize > 0 )
    {
        // Correct Pos
        nPos = ( ( nSize - 1 ) >= nPos) ? nPos : nPos - 1;
        m_xTabBox->set_active(nPos);
        aCurrentTab = aNewTabs[nPos];
    }

    // If no Tabs Enable Disable Controls
    if (m_xTabBox->get_count() == 0)
    {
        m_xDelBtn->set_sensitive(false);
        m_xNewBtn->set_sensitive(true);
        m_xTabBox->grab_focus();
    }
}

IMPL_LINK_NOARG(SvxTabulatorTabPage, DelAllHdl_Impl, weld::Button&, void)
{
    if ( aNewTabs.Count() )
    {
        aNewTabs = SvxTabStopItem( 0 );
        InitTabPos_Impl();
    }
}

IMPL_LINK(SvxTabulatorTabPage, TabTypeCheckHdl_Impl, weld::Button&, rBox, void)
{
    SvxTabAdjust eAdj;
    m_xDezChar->set_sensitive(false);
    m_xDezCharLabel->set_sensitive(false);
    m_xDezChar->set_text("");

    if (&rBox == m_xLeftTab.get())
        eAdj = SvxTabAdjust::Left;
    else if (&rBox == m_xRightTab.get())
        eAdj = SvxTabAdjust::Right;
    else if (&rBox == m_xCenterTab.get())
        eAdj = SvxTabAdjust::Center;
    else
    {
        eAdj = SvxTabAdjust::Decimal;
        m_xDezChar->set_sensitive(true);
        m_xDezCharLabel->set_sensitive(true);
        m_xDezChar->set_text(OUString(aCurrentTab.GetDecimal()));
    }

    aCurrentTab.GetAdjustment() = eAdj;
    int nPos = FindCurrentTab();
    if (nPos != -1)
    {
        aNewTabs.Remove( nPos );
        aNewTabs.Insert( aCurrentTab );
    }
}

IMPL_LINK(SvxTabulatorTabPage, FillTypeCheckHdl_Impl, weld::Button&, rBox, void)
{
    sal_uInt8 cFill = ' ';
    m_xFillChar->set_text( "" );
    m_xFillChar->set_sensitive(false);

    if (&rBox == m_xFillSpecial.get())
        m_xFillChar->set_sensitive(true);
    else if (&rBox == m_xNoFillChar.get())
        cFill = ' ';
    else if (&rBox == m_xFillSolidLine.get())
        cFill = '_';
    else if (&rBox == m_xFillPoints.get())
        cFill = '.';
    else if (&rBox == m_xFillDashLine.get())
        cFill = '-';

    aCurrentTab.GetFill() = cFill;
    int nPos = FindCurrentTab();
    if (nPos != -1)
    {
        aNewTabs.Remove( nPos );
        aNewTabs.Insert( aCurrentTab );
    }
}

IMPL_LINK_NOARG(SvxTabulatorTabPage, GetFillCharHdl_Impl, weld::Widget&, void)
{
    OUString aChar(m_xFillChar->get_text());
    if ( !aChar.isEmpty() )
        aCurrentTab.GetFill() = aChar[0];

    const int nPos = FindCurrentTab();
    if (nPos != -1)
    {
        aNewTabs.Remove( nPos );
        aNewTabs.Insert( aCurrentTab );
    }
}

IMPL_LINK_NOARG(SvxTabulatorTabPage, GetDezCharHdl_Impl, weld::Widget&, void)
{
    OUString aChar(m_xDezChar->get_text());
    if ( !aChar.isEmpty() && ( aChar[0] >= ' '))
        aCurrentTab.GetDecimal() = aChar[0];

    const int nPos = FindCurrentTab();
    if (nPos != -1)
    {
        aNewTabs.Remove( nPos );
        aNewTabs.Insert( aCurrentTab );
    }
}

IMPL_LINK_NOARG(SvxTabulatorTabPage, SelectHdl_Impl, weld::TreeView&, void)
{
    const int nPos = FindCurrentTab();
    if (nPos != -1)
    {
        aCurrentTab = aNewTabs[nPos];
        m_xNewBtn->set_sensitive(false);
        SetFillAndTabType_Impl();
    }
}

OUString SvxTabulatorTabPage::FormatTab()
{
    m_xTabSpin->set_text(m_xTabBox->get_active_text());
    m_xTabSpin->reformat();
    return m_xTabSpin->get_text();
}

IMPL_LINK_NOARG(SvxTabulatorTabPage, ReformatHdl_Impl, weld::Widget&, void)
{
    m_xTabBox->set_entry_text(FormatTab());
}

IMPL_LINK_NOARG(SvxTabulatorTabPage, ModifyHdl_Impl, weld::ComboBox&, void)
{
    const int nPos = FindCurrentTab();
    if (nPos != -1)
    {
        aCurrentTab = aNewTabs[nPos];
        SetFillAndTabType_Impl();

        m_xTabSpin->set_text(m_xTabBox->get_active_text());
        aCurrentTab.GetTabPos() = m_xTabSpin->denormalize(m_xTabSpin->get_value(eDefUnit));
        m_xNewBtn->set_sensitive(false);
        m_xDelBtn->set_sensitive(true);
        return;
    }
    m_xNewBtn->set_sensitive(true);
    m_xDelBtn->set_sensitive(false);
}

void SvxTabulatorTabPage::PageCreated(const SfxAllItemSet& aSet)
{
    const SfxUInt16Item* pControlItem = aSet.GetItem<SfxUInt16Item>(SID_SVXTABULATORTABPAGE_DISABLEFLAGS, false);
    if (pControlItem)
        DisableControls(static_cast<TabulatorDisableFlags>(pControlItem->GetValue()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
