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

#include <sdattr.hrc>
#include <optsitem.hxx>
#include <prntopts.hxx>
#include <app.hrc>
#include <svl/intitem.hxx>
#include <officecfg/Office/Impress.hxx>
#include <officecfg/Office/Draw.hxx>

/**
 *  dialog to adjust print options
 */
SdPrintOptions::SdPrintOptions(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, u"modules/simpress/ui/prntopts.ui"_ustr, u"prntopts"_ustr, &rInAttrs)
    , m_bDrawMode(false)
    , m_xFrmContent(m_xBuilder->weld_frame(u"contentframe"_ustr))
    , m_xCbxDraw(m_xBuilder->weld_check_button(u"drawingcb"_ustr))
    , m_xCbxNotes(m_xBuilder->weld_check_button(u"notecb"_ustr))
    , m_xCbxHandout(m_xBuilder->weld_check_button(u"handoutcb"_ustr))
    , m_xCbxOutline(m_xBuilder->weld_check_button(u"outlinecb"_ustr))
    , m_xRbtColor(m_xBuilder->weld_radio_button(u"defaultrb"_ustr))
    , m_xRbtGrayscale(m_xBuilder->weld_radio_button(u"grayscalerb"_ustr))
    , m_xRbtBlackWhite(m_xBuilder->weld_radio_button(u"blackwhiterb"_ustr))
    , m_xRbQualityImg(m_xBuilder->weld_widget(u"lockquality"_ustr))
    , m_xCbxPagename(m_xBuilder->weld_check_button(u"pagenmcb"_ustr))
    , m_xCbxPagenameImg(m_xBuilder->weld_widget(u"lockpagenmcb"_ustr))
    , m_xCbxDate(m_xBuilder->weld_check_button(u"datecb"_ustr))
    , m_xCbxDateImg(m_xBuilder->weld_widget(u"lockdatecb"_ustr))
    , m_xCbxTime(m_xBuilder->weld_check_button(u"timecb"_ustr))
    , m_xCbxTimeImg(m_xBuilder->weld_widget(u"locktimecb"_ustr))
    , m_xCbxHiddenPages(m_xBuilder->weld_check_button(u"hiddenpgcb"_ustr))
    , m_xCbxHiddenPagesImg(m_xBuilder->weld_widget(u"lockhiddenpgcb"_ustr))
    , m_xRbtDefault(m_xBuilder->weld_radio_button(u"pagedefaultrb"_ustr))
    , m_xRbtPagesize(m_xBuilder->weld_radio_button(u"fittopgrb"_ustr))
    , m_xRbtPagetile(m_xBuilder->weld_radio_button(u"tilepgrb"_ustr))
    , m_xRbtBooklet(m_xBuilder->weld_radio_button(u"brouchrb"_ustr))
    , m_xGridPageOpt(m_xBuilder->weld_widget(u"pageoptions"_ustr))
    , m_xRbtPageOptImg(m_xBuilder->weld_widget(u"lockpageoptions"_ustr))
    , m_xCbxFront(m_xBuilder->weld_check_button(u"frontcb"_ustr))
    , m_xCbxFrontImg(m_xBuilder->weld_widget(u"lockfrontcb"_ustr))
    , m_xCbxBack(m_xBuilder->weld_check_button(u"backcb"_ustr))
    , m_xCbxBackImg(m_xBuilder->weld_widget(u"lockbackcb"_ustr))
    , m_xCbxPaperbin(m_xBuilder->weld_check_button(u"papertryfrmprntrcb"_ustr))
    , m_xCbxPaperbinImg(m_xBuilder->weld_widget(u"lockpapertryfrmprntrcb"_ustr))
{
    Link<weld::Toggleable&,void> aLink = LINK( this, SdPrintOptions, ClickBookletHdl );
    m_xRbtDefault->connect_toggled( aLink );
    m_xRbtPagesize->connect_toggled( aLink );
    m_xRbtPagetile->connect_toggled( aLink );
    m_xRbtBooklet->connect_toggled( aLink );

    aLink = LINK( this, SdPrintOptions, ClickCheckboxHdl );
    m_xCbxDraw->connect_toggled( aLink );
    m_xCbxNotes->connect_toggled( aLink );
    m_xCbxHandout->connect_toggled( aLink );
    m_xCbxOutline->connect_toggled( aLink );

#ifndef MACOSX
    SetDrawMode();
#endif
}

SdPrintOptions::~SdPrintOptions()
{
}

OUString SdPrintOptions::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { u"label3"_ustr, u"label2"_ustr, u"printlbl"_ustr, u"contentlbl"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString checkButton[] = { u"frontcb"_ustr, u"backcb"_ustr,    u"papertryfrmprntrcb"_ustr, u"pagenmcb"_ustr,
                               u"datecb"_ustr,  u"timecb"_ustr,    u"hiddenpgcb"_ustr,         u"drawingcb"_ustr,
                               u"notecb"_ustr,  u"handoutcb"_ustr, u"outlinecb"_ustr };

    for (const auto& check : checkButton)
    {
        if (const auto& pString = m_xBuilder->weld_check_button(check))
            sAllStrings += pString->get_label() + " ";
    }

    OUString radioButton[] = { u"pagedefaultrb"_ustr, u"fittopgrb"_ustr,   u"tilepgrb"_ustr,    u"brouchrb"_ustr,
                               u"defaultrb"_ustr,     u"grayscalerb"_ustr, u"blackwhiterb"_ustr };

    for (const auto& radio : radioButton)
    {
        if (const auto& pString = m_xBuilder->weld_radio_button(radio))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
}

bool SdPrintOptions::FillItemSet( SfxItemSet* rAttrs )
{
    if( m_xCbxDraw->get_state_changed_from_saved() ||
        m_xCbxNotes->get_state_changed_from_saved() ||
        m_xCbxHandout->get_state_changed_from_saved() ||
        m_xCbxOutline->get_state_changed_from_saved() ||
        m_xCbxDate->get_state_changed_from_saved() ||
        m_xCbxTime->get_state_changed_from_saved() ||
        m_xCbxPagename->get_state_changed_from_saved() ||
        m_xCbxHiddenPages->get_state_changed_from_saved() ||
        m_xRbtPagesize->get_state_changed_from_saved() ||
        m_xRbtPagetile->get_state_changed_from_saved() ||
        m_xRbtBooklet->get_state_changed_from_saved() ||
        m_xCbxFront->get_state_changed_from_saved() ||
        m_xCbxBack->get_state_changed_from_saved() ||
        m_xCbxPaperbin->get_state_changed_from_saved() ||
        m_xRbtColor->get_state_changed_from_saved() ||
        m_xRbtGrayscale->get_state_changed_from_saved()||
        m_xRbtBlackWhite->get_state_changed_from_saved())
    {
        SdOptionsPrintItem aOptions;

        aOptions.GetOptionsPrint().SetDraw( m_xCbxDraw->get_active() );
        aOptions.GetOptionsPrint().SetNotes( m_xCbxNotes->get_active() );
        aOptions.GetOptionsPrint().SetHandout( m_xCbxHandout->get_active() );
        aOptions.GetOptionsPrint().SetOutline( m_xCbxOutline->get_active() );
        aOptions.GetOptionsPrint().SetDate( m_xCbxDate->get_active() );
        aOptions.GetOptionsPrint().SetTime( m_xCbxTime->get_active() );
        aOptions.GetOptionsPrint().SetPagename( m_xCbxPagename->get_active() );
        aOptions.GetOptionsPrint().SetHiddenPages( m_xCbxHiddenPages->get_active() );
        aOptions.GetOptionsPrint().SetPagesize( m_xRbtPagesize->get_active() );
        aOptions.GetOptionsPrint().SetPagetile( m_xRbtPagetile->get_active() );
        aOptions.GetOptionsPrint().SetBooklet( m_xRbtBooklet->get_active() );
        aOptions.GetOptionsPrint().SetFrontPage( m_xCbxFront->get_active() );
        aOptions.GetOptionsPrint().SetBackPage( m_xCbxBack->get_active() );
        aOptions.GetOptionsPrint().SetPaperbin( m_xCbxPaperbin->get_active() );

        sal_uInt16 nQuality = 0; // Standard, also Color
        if( m_xRbtGrayscale->get_active() )
            nQuality = 1;
        if( m_xRbtBlackWhite->get_active() )
            nQuality = 2;
        aOptions.GetOptionsPrint().SetOutputQuality( nQuality );

        rAttrs->Put( aOptions );

        return true;
    }
    return false;
}

void SdPrintOptions::Reset( const SfxItemSet* rAttrs )
{
    const SdOptionsPrintItem* pPrintOpts = rAttrs->GetItemIfSet( ATTR_OPTIONS_PRINT, false);
    if( pPrintOpts )
    {
        m_xCbxDraw->set_active(              pPrintOpts->GetOptionsPrint().IsDraw() );
        m_xCbxNotes->set_active(             pPrintOpts->GetOptionsPrint().IsNotes() );
        m_xCbxHandout->set_active(           pPrintOpts->GetOptionsPrint().IsHandout() );
        m_xCbxOutline->set_active(           pPrintOpts->GetOptionsPrint().IsOutline() );
        m_xCbxDate->set_active(              pPrintOpts->GetOptionsPrint().IsDate() );
        m_xCbxTime->set_active(              pPrintOpts->GetOptionsPrint().IsTime() );
        m_xCbxPagename->set_active(          pPrintOpts->GetOptionsPrint().IsPagename() );
        m_xCbxHiddenPages->set_active(       pPrintOpts->GetOptionsPrint().IsHiddenPages() );
        m_xRbtPagesize->set_active(          pPrintOpts->GetOptionsPrint().IsPagesize() );
        m_xRbtPagetile->set_active(          pPrintOpts->GetOptionsPrint().IsPagetile() );
        m_xRbtBooklet->set_active(           pPrintOpts->GetOptionsPrint().IsBooklet() );
        m_xCbxFront->set_active(             pPrintOpts->GetOptionsPrint().IsFrontPage() );
        m_xCbxBack->set_active(              pPrintOpts->GetOptionsPrint().IsBackPage() );
        m_xCbxPaperbin->set_active(          pPrintOpts->GetOptionsPrint().IsPaperbin() );

        if( !m_xRbtPagesize->get_active() &&
            !m_xRbtPagetile->get_active() &&
            !m_xRbtBooklet->get_active() )
        {
            m_xRbtDefault->set_active(true);
        }

        sal_uInt16 nQuality = pPrintOpts->GetOptionsPrint().GetOutputQuality();
        if( nQuality == 0 )
            m_xRbtColor->set_active(true);
        else if( nQuality == 1 )
            m_xRbtGrayscale->set_active(true);
        else
            m_xRbtBlackWhite->set_active(true);
    }

    bool bReadOnly = false;
    if (m_bDrawMode)
    {
        bReadOnly = officecfg::Office::Draw::Print::Page::PageSize::isReadOnly() ||
            officecfg::Office::Draw::Print::Page::PageTile::isReadOnly() ||
            officecfg::Office::Draw::Print::Page::Booklet::isReadOnly();
    }
    else
    {
        bReadOnly = officecfg::Office::Impress::Print::Page::PageSize::isReadOnly() ||
            officecfg::Office::Impress::Print::Page::PageTile::isReadOnly() ||
            officecfg::Office::Impress::Print::Page::Booklet::isReadOnly();
    }
    m_xGridPageOpt->set_sensitive(!bReadOnly);
    m_xRbtPageOptImg->set_visible(bReadOnly);

    bReadOnly = m_bDrawMode ? officecfg::Office::Draw::Print::Page::BookletFront::isReadOnly() :
        officecfg::Office::Impress::Print::Page::BookletFront::isReadOnly();
    m_xCbxFront->set_sensitive(!bReadOnly);
    m_xCbxFrontImg->set_visible(bReadOnly);

    bReadOnly = m_bDrawMode ? officecfg::Office::Draw::Print::Page::BookletBack::isReadOnly() :
        officecfg::Office::Impress::Print::Page::BookletBack::isReadOnly();
    m_xCbxBack->set_sensitive(!bReadOnly);
    m_xCbxBackImg->set_visible(bReadOnly);

    bReadOnly = m_bDrawMode ? officecfg::Office::Draw::Print::Other::FromPrinterSetup::isReadOnly() :
        officecfg::Office::Impress::Print::Other::FromPrinterSetup::isReadOnly();
    m_xCbxPaperbin->set_sensitive(!bReadOnly);
    m_xCbxPaperbinImg->set_visible(bReadOnly);

    bReadOnly = m_bDrawMode ? officecfg::Office::Draw::Print::Other::PageName::isReadOnly() :
        officecfg::Office::Impress::Print::Other::PageName::isReadOnly();
    m_xCbxPagename->set_sensitive(!bReadOnly);
    m_xCbxPagenameImg->set_visible(bReadOnly);

    bReadOnly = m_bDrawMode ? officecfg::Office::Draw::Print::Other::Date::isReadOnly() :
        officecfg::Office::Impress::Print::Other::Date::isReadOnly();
    m_xCbxDate->set_sensitive(!bReadOnly);
    m_xCbxDateImg->set_visible(bReadOnly);

    bReadOnly = m_bDrawMode ? officecfg::Office::Draw::Print::Other::Time::isReadOnly() :
        officecfg::Office::Impress::Print::Other::Time::isReadOnly();
    m_xCbxTime->set_sensitive(!bReadOnly);
    m_xCbxTimeImg->set_visible(bReadOnly);

    bReadOnly = m_bDrawMode ? officecfg::Office::Draw::Print::Other::HiddenPage::isReadOnly() :
        officecfg::Office::Impress::Print::Other::HiddenPage::isReadOnly();
    m_xCbxHiddenPages->set_sensitive(!bReadOnly);
    m_xCbxHiddenPagesImg->set_visible(bReadOnly);

    bReadOnly = m_bDrawMode ? officecfg::Office::Draw::Print::Other::Quality::isReadOnly() :
        officecfg::Office::Impress::Print::Other::Quality::isReadOnly();
    m_xRbtColor->set_sensitive(!bReadOnly);
    m_xRbtGrayscale->set_sensitive(!bReadOnly);
    m_xRbtBlackWhite->set_sensitive(!bReadOnly);
    m_xRbQualityImg->set_visible(bReadOnly);

    m_xCbxDraw->save_state();
    m_xCbxNotes->save_state();
    m_xCbxHandout->save_state();
    m_xCbxOutline->save_state();
    m_xCbxDate->save_state();
    m_xCbxTime->save_state();
    m_xCbxPagename->save_state();
    m_xCbxHiddenPages->save_state();
    m_xRbtPagesize->save_state();
    m_xRbtPagetile->save_state();
    m_xRbtBooklet->save_state();
    m_xCbxPaperbin->save_state();
    m_xRbtColor->save_state();
    m_xRbtGrayscale->save_state();
    m_xRbtBlackWhite->save_state();

    updateControls();
}

std::unique_ptr<SfxTabPage> SdPrintOptions::Create( weld::Container* pPage, weld::DialogController* pController,
                                           const SfxItemSet* rOutAttrs )
{
    return std::make_unique<SdPrintOptions>( pPage, pController, *rOutAttrs );
}

IMPL_LINK(SdPrintOptions, ClickCheckboxHdl, weld::Toggleable&, rCbx, void)
{
    // there must be at least one of them checked
    if( !m_xCbxDraw->get_active() && !m_xCbxNotes->get_active() && !m_xCbxOutline->get_active() && !m_xCbxHandout->get_active() )
        rCbx.set_active(true);

    updateControls();
}

IMPL_LINK_NOARG(SdPrintOptions, ClickBookletHdl, weld::Toggleable&, void)
{
    updateControls();
}

void SdPrintOptions::updateControls()
{
    bool bReadOnly = m_bDrawMode ? officecfg::Office::Draw::Print::Page::BookletFront::isReadOnly() :
        officecfg::Office::Impress::Print::Page::BookletFront::isReadOnly();
    m_xCbxFront->set_sensitive(m_xRbtBooklet->get_active() && !bReadOnly);

    bReadOnly = m_bDrawMode ? officecfg::Office::Draw::Print::Page::BookletBack::isReadOnly() :
        officecfg::Office::Impress::Print::Page::BookletBack::isReadOnly();
    m_xCbxBack->set_sensitive(m_xRbtBooklet->get_active() && !bReadOnly);

    bReadOnly = m_bDrawMode ? officecfg::Office::Draw::Print::Other::Date::isReadOnly() :
        officecfg::Office::Impress::Print::Other::Date::isReadOnly();
    m_xCbxDate->set_sensitive(!m_xRbtBooklet->get_active() && !bReadOnly);

    bReadOnly = m_bDrawMode ? officecfg::Office::Draw::Print::Other::Time::isReadOnly() :
        officecfg::Office::Impress::Print::Other::Time::isReadOnly();
    m_xCbxTime->set_sensitive(!m_xRbtBooklet->get_active() && !bReadOnly);

    bReadOnly = m_bDrawMode ? officecfg::Office::Draw::Print::Other::PageName::isReadOnly() :
        officecfg::Office::Impress::Print::Other::PageName::isReadOnly();
    m_xCbxPagename->set_sensitive( !m_xRbtBooklet->get_active() && !bReadOnly &&
        (m_xCbxDraw->get_active() || m_xCbxNotes->get_active() || m_xCbxOutline->get_active()) );
}

void    SdPrintOptions::SetDrawMode()
{
    if (m_xCbxNotes->get_visible())
    {
        m_xFrmContent->hide();
    }
}

void SdPrintOptions::PageCreated (const SfxAllItemSet& aSet)
{
    const SfxUInt32Item* pFlagItem = aSet.GetItem<SfxUInt32Item>(SID_SDMODE_FLAG, false);
    if (pFlagItem)
    {
        sal_uInt32 nFlags=pFlagItem->GetValue();
        if ( ( nFlags & SD_DRAW_MODE ) == SD_DRAW_MODE )
            m_bDrawMode = true;
    }
#ifdef MACOSX
    if (m_bDrawMode)
        SetDrawMode();
#else
    SetDrawMode();
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
