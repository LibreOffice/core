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
SdPrintOptions::SdPrintOptions( vcl::Window* pParent, const SfxItemSet& rInAttrs ) :
    SfxTabPage          ( pParent, "prntopts" , "modules/simpress/ui/prntopts.ui" , &rInAttrs )
{
    get( m_pFrmContent , "contentframe" );
    get( m_pCbxDraw , "drawingcb" );
    get( m_pCbxNotes , "notecb" );
    get( m_pCbxHandout , "handoutcb" );
    get( m_pCbxOutline , "outlinecb");
    get( m_pRbtColor , "defaultrb" );
    get( m_pRbtGrayscale , "grayscalerb" );
    get( m_pRbtBlackWhite , "blackwhiterb" );
    get( m_pCbxPagename , "pagenmcb" );
    get( m_pCbxDate , "datecb" );
    get( m_pCbxTime , "timecb" );
    get( m_pCbxHiddenPages , "hiddenpgcb" );
    get( m_pRbtDefault , "pagedefaultrb" );
    get( m_pRbtPagesize , "fittopgrb" );
    get( m_pRbtPagetile , "tilepgrb" );
    get( m_pRbtBooklet , "brouchrb" );
    get( m_pCbxFront , "frontcb" );
    get( m_pCbxBack , "backcb" );
    get( m_pCbxPaperbin , "papertryfrmprntrcb" );

    Link<Button*,void> aLink = LINK( this, SdPrintOptions, ClickBookletHdl );
    m_pRbtDefault->SetClickHdl( aLink );
    m_pRbtPagesize->SetClickHdl( aLink );
    m_pRbtPagetile->SetClickHdl( aLink );
    m_pRbtBooklet->SetClickHdl( aLink );

    aLink = LINK( this, SdPrintOptions, ClickCheckboxHdl );
    m_pCbxDraw->SetClickHdl( aLink );
    m_pCbxNotes->SetClickHdl( aLink );
    m_pCbxHandout->SetClickHdl( aLink );
    m_pCbxOutline->SetClickHdl( aLink );

#ifndef MACOSX
    SetDrawMode();
#endif
}

SdPrintOptions::~SdPrintOptions()
{
    disposeOnce();
}

void SdPrintOptions::dispose()
{
    m_pFrmContent.clear();
    m_pCbxDraw.clear();
    m_pCbxNotes.clear();
    m_pCbxHandout.clear();
    m_pCbxOutline.clear();
    m_pRbtColor.clear();
    m_pRbtGrayscale.clear();
    m_pRbtBlackWhite.clear();
    m_pCbxPagename.clear();
    m_pCbxDate.clear();
    m_pCbxTime.clear();
    m_pCbxHiddenPages.clear();
    m_pRbtDefault.clear();
    m_pRbtPagesize.clear();
    m_pRbtPagetile.clear();
    m_pRbtBooklet.clear();
    m_pCbxFront.clear();
    m_pCbxBack.clear();
    m_pCbxPaperbin.clear();
    SfxTabPage::dispose();
}

bool SdPrintOptions::FillItemSet( SfxItemSet* rAttrs )
{
    if( m_pCbxDraw->IsValueChangedFromSaved() ||
        m_pCbxNotes->IsValueChangedFromSaved() ||
        m_pCbxHandout->IsValueChangedFromSaved() ||
        m_pCbxOutline->IsValueChangedFromSaved() ||
        m_pCbxDate->IsValueChangedFromSaved() ||
        m_pCbxTime->IsValueChangedFromSaved() ||
        m_pCbxPagename->IsValueChangedFromSaved() ||
        m_pCbxHiddenPages->IsValueChangedFromSaved() ||
        m_pRbtPagesize->IsValueChangedFromSaved() ||
        m_pRbtPagetile->IsValueChangedFromSaved() ||
        m_pRbtBooklet->IsValueChangedFromSaved() ||
        m_pCbxFront->IsValueChangedFromSaved() ||
        m_pCbxBack->IsValueChangedFromSaved() ||
        m_pCbxPaperbin->IsValueChangedFromSaved() ||
        m_pRbtColor->IsValueChangedFromSaved() ||
        m_pRbtGrayscale->IsValueChangedFromSaved()||
        m_pRbtBlackWhite->IsValueChangedFromSaved())
    {
        SdOptionsPrintItem aOptions;

        aOptions.GetOptionsPrint().SetDraw( m_pCbxDraw->IsChecked() );
        aOptions.GetOptionsPrint().SetNotes( m_pCbxNotes->IsChecked() );
        aOptions.GetOptionsPrint().SetHandout( m_pCbxHandout->IsChecked() );
        aOptions.GetOptionsPrint().SetOutline( m_pCbxOutline->IsChecked() );
        aOptions.GetOptionsPrint().SetDate( m_pCbxDate->IsChecked() );
        aOptions.GetOptionsPrint().SetTime( m_pCbxTime->IsChecked() );
        aOptions.GetOptionsPrint().SetPagename( m_pCbxPagename->IsChecked() );
        aOptions.GetOptionsPrint().SetHiddenPages( m_pCbxHiddenPages->IsChecked() );
        aOptions.GetOptionsPrint().SetPagesize( m_pRbtPagesize->IsChecked() );
        aOptions.GetOptionsPrint().SetPagetile( m_pRbtPagetile->IsChecked() );
        aOptions.GetOptionsPrint().SetBooklet( m_pRbtBooklet->IsChecked() );
        aOptions.GetOptionsPrint().SetFrontPage( m_pCbxFront->IsChecked() );
        aOptions.GetOptionsPrint().SetBackPage( m_pCbxBack->IsChecked() );
        aOptions.GetOptionsPrint().SetPaperbin( m_pCbxPaperbin->IsChecked() );

        sal_uInt16 nQuality = 0; // Standard, also Color
        if( m_pRbtGrayscale->IsChecked() )
            nQuality = 1;
        if( m_pRbtBlackWhite->IsChecked() )
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
        m_pCbxDraw->Check(              pPrintOpts->GetOptionsPrint().IsDraw() );
        m_pCbxNotes->Check(             pPrintOpts->GetOptionsPrint().IsNotes() );
        m_pCbxHandout->Check(           pPrintOpts->GetOptionsPrint().IsHandout() );
        m_pCbxOutline->Check(           pPrintOpts->GetOptionsPrint().IsOutline() );
        m_pCbxDate->Check(              pPrintOpts->GetOptionsPrint().IsDate() );
        m_pCbxTime->Check(              pPrintOpts->GetOptionsPrint().IsTime() );
        m_pCbxPagename->Check(          pPrintOpts->GetOptionsPrint().IsPagename() );
        m_pCbxHiddenPages->Check(       pPrintOpts->GetOptionsPrint().IsHiddenPages() );
        m_pRbtPagesize->Check(          pPrintOpts->GetOptionsPrint().IsPagesize() );
        m_pRbtPagetile->Check(          pPrintOpts->GetOptionsPrint().IsPagetile() );
        m_pRbtBooklet->Check(           pPrintOpts->GetOptionsPrint().IsBooklet() );
        m_pCbxFront->Check(             pPrintOpts->GetOptionsPrint().IsFrontPage() );
        m_pCbxBack->Check(              pPrintOpts->GetOptionsPrint().IsBackPage() );
        m_pCbxPaperbin->Check(          pPrintOpts->GetOptionsPrint().IsPaperbin() );

        if( !m_pRbtPagesize->IsChecked() &&
            !m_pRbtPagetile->IsChecked() &&
            !m_pRbtBooklet->IsChecked() )
        {
            m_pRbtDefault->Check();
        }

        sal_uInt16 nQuality = pPrintOpts->GetOptionsPrint().GetOutputQuality();
        if( nQuality == 0 )
            m_pRbtColor->Check();
        else if( nQuality == 1 )
            m_pRbtGrayscale->Check();
        else
            m_pRbtBlackWhite->Check();
    }
    m_pCbxDraw->SaveValue();
    m_pCbxNotes->SaveValue();
    m_pCbxHandout->SaveValue();
    m_pCbxOutline->SaveValue();
    m_pCbxDate->SaveValue();
    m_pCbxTime->SaveValue();
    m_pCbxPagename->SaveValue();
    m_pCbxHiddenPages->SaveValue();
    m_pRbtPagesize->SaveValue();
    m_pRbtPagetile->SaveValue();
    m_pRbtBooklet->SaveValue();
    m_pCbxPaperbin->SaveValue();
    m_pRbtColor->SaveValue();
    m_pRbtGrayscale->SaveValue();
    m_pRbtBlackWhite->SaveValue();

    ClickBookletHdl( nullptr );
}

VclPtr<SfxTabPage> SdPrintOptions::Create( TabPageParent pWindow,
                                           const SfxItemSet* rOutAttrs )
{
    return VclPtr<SdPrintOptions>::Create( pWindow.pParent, *rOutAttrs );
}

IMPL_LINK( SdPrintOptions, ClickCheckboxHdl, Button*, pCbx, void )
{
    // there must be at least one of them checked
    if( !m_pCbxDraw->IsChecked() && !m_pCbxNotes->IsChecked() && !m_pCbxOutline->IsChecked() && !m_pCbxHandout->IsChecked() )
        static_cast<CheckBox*>(pCbx)->Check();

    updateControls();
}

IMPL_LINK_NOARG(SdPrintOptions, ClickBookletHdl, Button*, void)
{
    updateControls();
}

void SdPrintOptions::updateControls()
{
    m_pCbxFront->Enable(m_pRbtBooklet->IsChecked());
    m_pCbxBack->Enable(m_pRbtBooklet->IsChecked());

    m_pCbxDate->Enable( !m_pRbtBooklet->IsChecked() );
    m_pCbxTime->Enable( !m_pRbtBooklet->IsChecked() );

    m_pCbxPagename->Enable( !m_pRbtBooklet->IsChecked() && (m_pCbxDraw->IsChecked() || m_pCbxNotes->IsChecked() || m_pCbxOutline->IsChecked()) );
}

void    SdPrintOptions::SetDrawMode()
{
    if(m_pCbxNotes->IsVisible())
    {
        m_pFrmContent->Hide();
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
