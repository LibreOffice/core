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

#include "sdattr.hxx"
#include "optsitem.hxx"
#include "prntopts.hrc"
#include "sdresid.hxx"
#include "prntopts.hxx"
#include "app.hrc"
#include <svl/intitem.hxx>
#include <sfx2/request.hxx>

/**
 *  dialog to adjust print options
 */
SdPrintOptions::SdPrintOptions( Window* pParent, const SfxItemSet& rInAttrs ) :
        SfxTabPage          ( pParent, SdResId( TP_PRINT_OPTIONS ), rInAttrs ),

        aGrpPrint               ( this, SdResId( GRP_PRINT ) ),
        aCbxDraw                ( this, SdResId( CBX_DRAW ) ),
        aCbxNotes               ( this, SdResId( CBX_NOTES ) ),
        aCbxHandout             ( this, SdResId( CBX_HANDOUTS ) ),
        aCbxOutline             ( this, SdResId( CBX_OUTLINE ) ),

        aSeparator1FL            ( this, SdResId( FL_SEPARATOR1 ) ),
        aGrpOutput              ( this, SdResId( GRP_OUTPUT ) ),
        aRbtColor               ( this, SdResId( RBT_COLOR ) ),
        aRbtGrayscale           ( this, SdResId( RBT_GRAYSCALE ) ),
        aRbtBlackWhite          ( this, SdResId( RBT_BLACKWHITE ) ),

        aGrpPrintExt            ( this, SdResId( GRP_PRINT_EXT ) ),
        aCbxPagename            ( this, SdResId( CBX_PAGENAME ) ),
        aCbxDate                ( this, SdResId( CBX_DATE ) ),
        aCbxTime                ( this, SdResId( CBX_TIME ) ),
        aCbxHiddenPages         ( this, SdResId( CBX_HIDDEN_PAGES ) ),

        aSeparator2FL            ( this, SdResId( FL_SEPARATOR2 ) ),
        aGrpPageoptions         ( this, SdResId( GRP_PAGE ) ),
        aRbtDefault             ( this, SdResId( RBT_DEFAULT ) ),
        aRbtPagesize            ( this, SdResId( RBT_PAGESIZE ) ),
        aRbtPagetile            ( this, SdResId( RBT_PAGETILE ) ),
        aRbtBooklet             ( this, SdResId( RBT_BOOKLET ) ),
        aCbxFront               ( this, SdResId( CBX_FRONT ) ),
        aCbxBack                ( this, SdResId( CBX_BACK ) ),

        aCbxPaperbin            ( this, SdResId( CBX_PAPERBIN ) ),

        rOutAttrs               ( rInAttrs )
{
    FreeResource();

    Link aLink = LINK( this, SdPrintOptions, ClickBookletHdl );
    aRbtDefault.SetClickHdl( aLink );
    aRbtPagesize.SetClickHdl( aLink );
    aRbtPagetile.SetClickHdl( aLink );
    aRbtBooklet.SetClickHdl( aLink );

    aLink = LINK( this, SdPrintOptions, ClickCheckboxHdl );
    aCbxDraw.SetClickHdl( aLink );
    aCbxNotes.SetClickHdl( aLink );
    aCbxHandout.SetClickHdl( aLink );
    aCbxOutline.SetClickHdl( aLink );

#ifndef MACOSX
    SetDrawMode();
#endif

    aCbxFront.SetAccessibleRelationLabeledBy( &aRbtBooklet );
    aCbxBack.SetAccessibleRelationLabeledBy( &aRbtBooklet );
}

// -----------------------------------------------------------------------

SdPrintOptions::~SdPrintOptions()
{
}

// -----------------------------------------------------------------------

sal_Bool SdPrintOptions::FillItemSet( SfxItemSet& rAttrs )
{
    if( aCbxDraw.GetSavedValue() != aCbxDraw.IsChecked() ||
        aCbxNotes.GetSavedValue() != aCbxNotes.IsChecked() ||
        aCbxHandout.GetSavedValue() != aCbxHandout.IsChecked() ||
        aCbxOutline.GetSavedValue() != aCbxOutline.IsChecked() ||
        aCbxDate.GetSavedValue() != aCbxDate.IsChecked() ||
        aCbxTime.GetSavedValue() != aCbxTime.IsChecked() ||
        aCbxPagename.GetSavedValue() != aCbxPagename.IsChecked() ||
        aCbxHiddenPages.GetSavedValue() != aCbxHiddenPages.IsChecked() ||
        aRbtPagesize.GetSavedValue() != aRbtPagesize.IsChecked() ||
        aRbtPagetile.GetSavedValue() != aRbtPagetile.IsChecked() ||
        aRbtBooklet.GetSavedValue() != aRbtBooklet.IsChecked() ||
        aCbxFront.GetSavedValue() != aCbxFront.IsChecked() ||
        aCbxBack.GetSavedValue() != aCbxBack.IsChecked() ||
        aCbxPaperbin.GetSavedValue() != aCbxPaperbin.IsChecked() ||
        aRbtColor.GetSavedValue() != aRbtColor.IsChecked() ||
        aRbtGrayscale.GetSavedValue() != aRbtGrayscale.IsChecked() ||
        aRbtBlackWhite.GetSavedValue() != aRbtBlackWhite.IsChecked() )
    {
        SdOptionsPrintItem aOptions( ATTR_OPTIONS_PRINT );

        aOptions.GetOptionsPrint().SetDraw( aCbxDraw.IsChecked() );
        aOptions.GetOptionsPrint().SetNotes( aCbxNotes.IsChecked() );
        aOptions.GetOptionsPrint().SetHandout( aCbxHandout.IsChecked() );
        aOptions.GetOptionsPrint().SetOutline( aCbxOutline.IsChecked() );
        aOptions.GetOptionsPrint().SetDate( aCbxDate.IsChecked() );
        aOptions.GetOptionsPrint().SetTime( aCbxTime.IsChecked() );
        aOptions.GetOptionsPrint().SetPagename( aCbxPagename.IsChecked() );
        aOptions.GetOptionsPrint().SetHiddenPages( aCbxHiddenPages.IsChecked() );
        aOptions.GetOptionsPrint().SetPagesize( aRbtPagesize.IsChecked() );
        aOptions.GetOptionsPrint().SetPagetile( aRbtPagetile.IsChecked() );
        aOptions.GetOptionsPrint().SetBooklet( aRbtBooklet.IsChecked() );
        aOptions.GetOptionsPrint().SetFrontPage( aCbxFront.IsChecked() );
        aOptions.GetOptionsPrint().SetBackPage( aCbxBack.IsChecked() );
        aOptions.GetOptionsPrint().SetPaperbin( aCbxPaperbin.IsChecked() );

        sal_uInt16 nQuality = 0; // Standard, also Color
        if( aRbtGrayscale.IsChecked() )
            nQuality = 1;
        if( aRbtBlackWhite.IsChecked() )
            nQuality = 2;
        aOptions.GetOptionsPrint().SetOutputQuality( nQuality );

        rAttrs.Put( aOptions );

        return( sal_True );
    }
    return( sal_False );
}

// -----------------------------------------------------------------------

void SdPrintOptions::Reset( const SfxItemSet& rAttrs )
{
    const SdOptionsPrintItem* pPrintOpts = NULL;
    if( SFX_ITEM_SET == rAttrs.GetItemState( ATTR_OPTIONS_PRINT, sal_False,
                            (const SfxPoolItem**) &pPrintOpts ) )
    {
        aCbxDraw.Check(              pPrintOpts->GetOptionsPrint().IsDraw() );
        aCbxNotes.Check(             pPrintOpts->GetOptionsPrint().IsNotes() );
        aCbxHandout.Check(           pPrintOpts->GetOptionsPrint().IsHandout() );
        aCbxOutline.Check(           pPrintOpts->GetOptionsPrint().IsOutline() );
        aCbxDate.Check(              pPrintOpts->GetOptionsPrint().IsDate() );
        aCbxTime.Check(              pPrintOpts->GetOptionsPrint().IsTime() );
        aCbxPagename.Check(          pPrintOpts->GetOptionsPrint().IsPagename() );
        aCbxHiddenPages.Check(       pPrintOpts->GetOptionsPrint().IsHiddenPages() );
        aRbtPagesize.Check(          pPrintOpts->GetOptionsPrint().IsPagesize() );
        aRbtPagetile.Check(          pPrintOpts->GetOptionsPrint().IsPagetile() );
        aRbtBooklet.Check(           pPrintOpts->GetOptionsPrint().IsBooklet() );
        aCbxFront.Check(             pPrintOpts->GetOptionsPrint().IsFrontPage() );
        aCbxBack.Check(              pPrintOpts->GetOptionsPrint().IsBackPage() );
        aCbxPaperbin.Check(          pPrintOpts->GetOptionsPrint().IsPaperbin() );

        if( !aRbtPagesize.IsChecked() &&
            !aRbtPagetile.IsChecked() &&
            !aRbtBooklet.IsChecked() )
        {
            aRbtDefault.Check();
        }

        sal_uInt16 nQuality = pPrintOpts->GetOptionsPrint().GetOutputQuality();
        if( nQuality == 0 )
            aRbtColor.Check();
        else if( nQuality == 1 )
            aRbtGrayscale.Check();
        else
            aRbtBlackWhite.Check();
    }
    aCbxDraw.SaveValue();
    aCbxNotes.SaveValue();
    aCbxHandout.SaveValue();
    aCbxOutline.SaveValue();
    aCbxDate.SaveValue();
    aCbxTime.SaveValue();
    aCbxPagename.SaveValue();
    aCbxHiddenPages.SaveValue();
    aRbtPagesize.SaveValue();
    aRbtPagetile.SaveValue();
    aRbtBooklet.SaveValue();
    aCbxPaperbin.SaveValue();
    aRbtColor.SaveValue();
    aRbtGrayscale.SaveValue();
    aRbtBlackWhite.SaveValue();

    ClickBookletHdl( NULL );
}

// -----------------------------------------------------------------------

SfxTabPage* SdPrintOptions::Create( Window* pWindow,
                const SfxItemSet& rOutAttrs )
{
    return( new SdPrintOptions( pWindow, rOutAttrs ) );
}

//-----------------------------------------------------------------------

IMPL_LINK( SdPrintOptions, ClickCheckboxHdl, CheckBox *, pCbx )
{
    // there must be at least one of them checked
    if( !aCbxDraw.IsChecked() && !aCbxNotes.IsChecked() && !aCbxOutline.IsChecked() && !aCbxHandout.IsChecked() )
        pCbx->Check();

    updateControls();
    return 0;
}

//-----------------------------------------------------------------------

IMPL_LINK_NOARG(SdPrintOptions, ClickBookletHdl)
{
    updateControls();
    return 0;
}

void SdPrintOptions::updateControls()
{
    aCbxFront.Enable(aRbtBooklet.IsChecked());
    aCbxBack.Enable(aRbtBooklet.IsChecked());

    aCbxDate.Enable( !aRbtBooklet.IsChecked() );
    aCbxTime.Enable( !aRbtBooklet.IsChecked() );

    aCbxPagename.Enable( !aRbtBooklet.IsChecked() && (aCbxDraw.IsChecked() || aCbxNotes.IsChecked() || aCbxOutline.IsChecked()) );
}

static void lcl_MoveRB_Impl(Window& rBtn, long nXDiff)
{
    Point aPos(rBtn.GetPosPixel());
    aPos.X() -= nXDiff;
    rBtn.SetPosPixel(aPos);
}

void    SdPrintOptions::SetDrawMode()
{
    if(aCbxNotes.IsVisible())
    {
        aCbxNotes.Hide();
        aCbxHandout.Hide();
        aCbxOutline.Hide();
        aCbxDraw.Hide();
        aGrpPrint.Hide();

        aSeparator1FL.Hide();
        long nXDiff = aGrpOutput.GetPosPixel().X() - aGrpPrint.GetPosPixel().X();
        lcl_MoveRB_Impl(aRbtColor, nXDiff);
        lcl_MoveRB_Impl(aRbtGrayscale, nXDiff);
        lcl_MoveRB_Impl(aRbtBlackWhite, nXDiff);
        lcl_MoveRB_Impl(aGrpOutput, nXDiff);

        long nWidth =  aGrpOutput.GetSizePixel().Width() + nXDiff;
        Size aSize(aGrpOutput.GetSizePixel());
        aSize.Width() = nWidth;
        aGrpOutput.SetSizePixel(aSize);
    }
}

void SdPrintOptions::PageCreated (SfxAllItemSet
#ifdef MACOSX
                                  aSet
#endif
                                  )
{
#ifdef MACOSX
    SFX_ITEMSET_ARG (&aSet,pFlagItem,SfxUInt32Item,SID_SDMODE_FLAG,sal_False);
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
