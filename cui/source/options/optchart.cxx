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

#include "optchart.hxx"
#include <svx/SvxColorValueSet.hxx>
#include <vcl/virdev.hxx>
#include <vcl/weld.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <svx/svxids.hrc>
#include <osl/diagnose.h>
#include <officecfg/Office/Common.hxx>
#include <officecfg/Office/Chart.hxx>

void SvxDefaultColorOptPage::InsertColorEntry(const XColorEntry& rEntry, sal_Int32 nPos)
{
    const Color& rColor = rEntry.GetColor();
    const OUString& rStr = rEntry.GetName();

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Size aImageSize = rStyleSettings.GetListBoxPreviewDefaultPixelSize();

    ScopedVclPtrInstance<VirtualDevice> xDevice;
    xDevice->SetOutputSize(aImageSize);
    const ::tools::Rectangle aRect(Point(0, 0), aImageSize);
    xDevice->SetFillColor(rColor);
    xDevice->SetLineColor(rStyleSettings.GetDisableColor());
    xDevice->DrawRect(aRect);

    m_xLbChartColors->insert(nullptr, nPos, &rStr, nullptr,
                             nullptr, xDevice.get(), false, nullptr);

    if (nPos == -1)
        aColorList.push_back( rColor );
    else
    {
        ImpColorList::iterator it = aColorList.begin();
        std::advance( it, nPos );
        aColorList.insert( it, rColor );
    }
}

void SvxDefaultColorOptPage::RemoveColorEntry(sal_Int32 nPos)
{
    m_xLbChartColors->remove(nPos);
    ImpColorList::iterator it = aColorList.begin();
    std::advance(it, nPos);
    aColorList.erase(it);
}

void SvxDefaultColorOptPage::ClearColorEntries()
{
    aColorList.clear();
    m_xLbChartColors->clear();
}

void SvxDefaultColorOptPage::ModifyColorEntry(const XColorEntry& rEntry, sal_Int32 nPos)
{
    RemoveColorEntry(nPos);
    InsertColorEntry(rEntry, nPos);
}

void SvxDefaultColorOptPage::FillBoxChartColorLB()
{
    if (!m_SvxChartColorTableUniquePtr)
        return;

    m_xLbChartColors->freeze();
    ClearColorEntries();
    const tools::Long nCount(m_SvxChartColorTableUniquePtr->size());
    for (tools::Long i = 0; i < nCount; ++i)
        InsertColorEntry((*m_SvxChartColorTableUniquePtr)[i]);
    m_xLbChartColors->thaw();
}

SvxDefaultColorOptPage::SvxDefaultColorOptPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, u"cui/ui/optchartcolorspage.ui"_ustr, u"OptChartColorsPage"_ustr, &rInAttrs)
    , m_xLbChartColors(m_xBuilder->weld_tree_view(u"colors"_ustr))
    , m_xLbPaletteSelector(m_xBuilder->weld_combo_box(u"paletteselector"_ustr))
    , m_xPBDefault(m_xBuilder->weld_button(u"default"_ustr))
    , m_xPBAdd(m_xBuilder->weld_button(u"add"_ustr))
    , m_xPBRemove(m_xBuilder->weld_button(u"delete"_ustr))
    , m_xValSetColorBox(new SvxColorValueSet(m_xBuilder->weld_scrolled_window(u"tablewin"_ustr, true)))
    , m_xValSetColorBoxWin(new weld::CustomWeld(*m_xBuilder, u"table"_ustr, *m_xValSetColorBox))
{
    m_xLbChartColors->set_size_request(-1, m_xLbChartColors->get_height_rows(16));

    if (officecfg::Office::Chart::DefaultColor::Series::isReadOnly())
    {
        m_xPBDefault->set_sensitive(false);
        m_xPBAdd->set_sensitive(false);
        m_xPBRemove->set_sensitive(false);
        m_xValSetColorBoxWin->set_sensitive(false);
    }

    m_xPBDefault->connect_clicked( LINK( this, SvxDefaultColorOptPage, ResetToDefaults ) );
    m_xPBAdd->connect_clicked( LINK( this, SvxDefaultColorOptPage, AddChartColor ) );
    m_xPBRemove->connect_clicked( LINK( this, SvxDefaultColorOptPage, RemoveChartColor ) );
    m_xValSetColorBox->SetSelectHdl( LINK( this, SvxDefaultColorOptPage, BoxClickedHdl ) );
    m_xLbPaletteSelector->connect_changed( LINK( this, SvxDefaultColorOptPage, SelectPaletteLbHdl ) );

    m_xValSetColorBox->SetStyle( m_xValSetColorBox->GetStyle()
                                    | WB_ITEMBORDER | WB_NAMEFIELD | WB_VSCROLL );

    if ( const SvxChartColorTableItem* pEditOptionsItem = rInAttrs.GetItemIfSet( SID_SCH_EDITOPTIONS, false ) )
    {
        m_SvxChartColorTableUniquePtr = std::make_unique<SvxChartColorTable>(
            pEditOptionsItem->GetColorList());
    }
    else
    {
        m_SvxChartColorTableUniquePtr = std::make_unique<SvxChartColorTable>();
        m_SvxChartColorTableUniquePtr->useDefault();
        SvxChartOptions::SetDefaultColors(*m_SvxChartColorTableUniquePtr);
    }

    Construct();
}

SvxDefaultColorOptPage::~SvxDefaultColorOptPage()
{
    m_xValSetColorBoxWin.reset();
    m_xValSetColorBox.reset();
}

void SvxDefaultColorOptPage::Construct()
{
    FillBoxChartColorLB();
    FillPaletteLB();

    m_xLbChartColors->select( 0 );
}

std::unique_ptr<SfxTabPage> SvxDefaultColorOptPage::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrs )
{
    return std::make_unique<SvxDefaultColorOptPage>( pPage, pController, *rAttrs );
}

OUString SvxDefaultColorOptPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { u"label20"_ustr, u"label1"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString buttons[] = { u"add"_ustr, u"delete"_ustr, u"default"_ustr };

    for (const auto& btn : buttons)
    {
        if (const auto& pString = m_xBuilder->weld_button(btn))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
}

bool SvxDefaultColorOptPage::FillItemSet( SfxItemSet* rOutAttrs )
{
    if( m_SvxChartColorTableUniquePtr )
    {
        rOutAttrs->Put(SvxChartColorTableItem(SID_SCH_EDITOPTIONS, *m_SvxChartColorTableUniquePtr));
    }

    return true;
}

void SvxDefaultColorOptPage::Reset( const SfxItemSet* )
{
    m_xLbChartColors->select( 0 );
}

void SvxDefaultColorOptPage::FillPaletteLB()
{
    m_xLbPaletteSelector->clear();
    std::vector<OUString> aPaletteList = aPaletteManager.GetPaletteList();
    for (auto const& palette : aPaletteList)
        m_xLbPaletteSelector->append_text(palette);

    OUString aPaletteName(officecfg::Office::Common::UserColors::PaletteName::get());
    m_xLbPaletteSelector->set_active_text(aPaletteName);
    if (m_xLbPaletteSelector->get_active() != -1)
        SelectPaletteLbHdl( *m_xLbPaletteSelector );
}

void SvxDefaultColorOptPage::SaveChartOptions()
{
    if (m_SvxChartColorTableUniquePtr)
        SvxChartOptions::SetDefaultColors(*m_SvxChartColorTableUniquePtr);
}

// event handlers


// ResetToDefaults
IMPL_LINK_NOARG(SvxDefaultColorOptPage, ResetToDefaults, weld::Button&, void)
{
    if( m_SvxChartColorTableUniquePtr )
    {
        m_SvxChartColorTableUniquePtr->useDefault();

        FillBoxChartColorLB();

        m_xLbChartColors->grab_focus();
        m_xLbChartColors->select( 0 );
        m_xPBRemove->set_sensitive(true);
    }
}

// AddChartColor
IMPL_LINK_NOARG(SvxDefaultColorOptPage, AddChartColor, weld::Button&, void)
{
    if( m_SvxChartColorTableUniquePtr )
    {
        Color const black( 0x00, 0x00, 0x00 );

        m_SvxChartColorTableUniquePtr->append(
            XColorEntry(black, SvxChartColorTable::getDefaultName(m_SvxChartColorTableUniquePtr->size())));

        FillBoxChartColorLB();
        m_xLbChartColors->grab_focus();
        m_xLbChartColors->select(m_SvxChartColorTableUniquePtr->size() - 1);
        m_xPBRemove->set_sensitive(true);
    }
}

// RemoveChartColor
IMPL_LINK_NOARG( SvxDefaultColorOptPage, RemoveChartColor, weld::Button&, void )
{
    sal_Int32 nIndex = m_xLbChartColors->get_selected_index();
    if (nIndex == -1)
        return;

    if( !m_SvxChartColorTableUniquePtr )
        return;

    OSL_ENSURE(m_SvxChartColorTableUniquePtr->size() > 1, "don't delete the last chart color");

    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), u"cui/ui/querydeletechartcolordialog.ui"_ustr));
    std::unique_ptr<weld::MessageDialog> xQuery(xBuilder->weld_message_dialog(u"QueryDeleteChartColorDialog"_ustr));

    if (RET_YES != xQuery->run())
        return;

    m_SvxChartColorTableUniquePtr->remove(nIndex);

    FillBoxChartColorLB();

    m_xLbChartColors->grab_focus();

    if (nIndex == m_xLbChartColors->n_children() && m_xLbChartColors->n_children() > 0)
        m_xLbChartColors->select(m_SvxChartColorTableUniquePtr->size() - 1);
    else if (m_xLbChartColors->n_children() > 0)
        m_xLbChartColors->select( nIndex );
    else
        m_xPBRemove->set_sensitive(true);
}

IMPL_LINK_NOARG( SvxDefaultColorOptPage, SelectPaletteLbHdl, weld::ComboBox&, void)
{
    sal_Int32 nPos = m_xLbPaletteSelector->get_active();
    aPaletteManager.SetPalette( nPos );
    aPaletteManager.ReloadColorSet( *m_xValSetColorBox );
    m_xValSetColorBox->Resize();
}

IMPL_LINK_NOARG(SvxDefaultColorOptPage, BoxClickedHdl, ValueSet*, void)
{
    sal_Int32 nIdx = m_xLbChartColors->get_selected_index();
    if (nIdx != -1)
    {
        const XColorEntry aEntry(m_xValSetColorBox->GetItemColor(m_xValSetColorBox->GetSelectedItemId()), m_xLbChartColors->get_selected_text());

        ModifyColorEntry(aEntry, nIdx);
        m_SvxChartColorTableUniquePtr->replace(nIdx, aEntry);

        m_xLbChartColors->select(nIdx);  // reselect entry
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
