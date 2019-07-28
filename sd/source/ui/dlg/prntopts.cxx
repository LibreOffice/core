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

/**
 *  dialog to adjust print options
 */
SdPrintOptions::SdPrintOptions(TabPageParent pParent, const SfxItemSet& rInAttrs)
    : SfxTabPage(pParent, "modules/simpress/ui/prntopts.ui", "prntopts", &rInAttrs)
    , m_xFrmContent(m_xBuilder->weld_frame("contentframe"))
    , m_xCbxDraw(m_xBuilder->weld_check_button("drawingcb"))
    , m_xCbxNotes(m_xBuilder->weld_check_button("notecb"))
    , m_xCbxHandout(m_xBuilder->weld_check_button("handoutcb"))
    , m_xCbxOutline(m_xBuilder->weld_check_button("outlinecb"))
    , m_xRbtColor(m_xBuilder->weld_radio_button("defaultrb"))
    , m_xRbtGrayscale(m_xBuilder->weld_radio_button("grayscalerb"))
    , m_xRbtBlackWhite(m_xBuilder->weld_radio_button("blackwhiterb"))
    , m_xCbxPagename(m_xBuilder->weld_check_button("pagenmcb"))
    , m_xCbxDate(m_xBuilder->weld_check_button("datecb"))
    , m_xCbxTime(m_xBuilder->weld_check_button("timecb"))
    , m_xCbxHiddenPages(m_xBuilder->weld_check_button("hiddenpgcb"))
    , m_xRbtDefault(m_xBuilder->weld_radio_button("pagedefaultrb"))
    , m_xRbtPagesize(m_xBuilder->weld_radio_button("fittopgrb"))
    , m_xRbtPagetile(m_xBuilder->weld_radio_button("tilepgrb"))
    , m_xRbtBooklet(m_xBuilder->weld_radio_button("brouchrb"))
    , m_xCbxFront(m_xBuilder->weld_check_button("frontcb"))
    , m_xCbxBack(m_xBuilder->weld_check_button("backcb"))
    , m_xCbxPaperbin(m_xBuilder->weld_check_button("papertryfrmprntrcb"))
{
    Link<weld::ToggleButton&,void> aLink = LINK( this, SdPrintOptions, ClickBookletHdl );
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
    const SdOptionsPrintItem* pPrintOpts = nullptr;
    if( SfxItemState::SET == rAttrs->GetItemState( ATTR_OPTIONS_PRINT, false,
                            reinterpret_cast<const SfxPoolItem**>(&pPrintOpts) ) )
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

VclPtr<SfxTabPage> SdPrintOptions::Create( TabPageParent pParent,
                                           const SfxItemSet* rOutAttrs )
{
    return VclPtr<SdPrintOptions>::Create( pParent, *rOutAttrs );
}

IMPL_LINK(SdPrintOptions, ClickCheckboxHdl, weld::ToggleButton&, rCbx, void)
{
    // there must be at least one of them checked
    if( !m_xCbxDraw->get_active() && !m_xCbxNotes->get_active() && !m_xCbxOutline->get_active() && !m_xCbxHandout->get_active() )
        rCbx.set_active(true);

    updateControls();
}

IMPL_LINK_NOARG(SdPrintOptions, ClickBookletHdl, weld::ToggleButton&, void)
{
    updateControls();
}

void SdPrintOptions::updateControls()
{
    m_xCbxFront->set_sensitive(m_xRbtBooklet->get_active());
    m_xCbxBack->set_sensitive(m_xRbtBooklet->get_active());

    m_xCbxDate->set_sensitive( !m_xRbtBooklet->get_active() );
    m_xCbxTime->set_sensitive( !m_xRbtBooklet->get_active() );

    m_xCbxPagename->set_sensitive( !m_xRbtBooklet->get_active() && (m_xCbxDraw->get_active() || m_xCbxNotes->get_active() || m_xCbxOutline->get_active()) );
}

void    SdPrintOptions::SetDrawMode()
{
    if (m_xCbxNotes->get_visible())
    {
        m_xFrmContent->hide();
    }
}

void SdPrintOptions::PageCreated (const SfxAllItemSet&
#ifdef MACOSX
                                  aSet
#endif
                                  )
{
#ifdef MACOSX
    const SfxUInt32Item* pFlagItem = aSet.GetItem<SfxUInt32Item>(SID_SDMODE_FLAG, false);
    if (pFlagItem)
    {
        sal_uInt32 nFlags=pFlagItem->GetValue();
        if ( ( nFlags & SD_DRAW_MODE ) == SD_DRAW_MODE )
            SetDrawMode();
    }
#else
    SetDrawMode();
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
