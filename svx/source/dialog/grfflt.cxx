/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <tools/shl.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>

#include <svx/dialmgr.hxx>
#include <svx/grfflt.hxx>
#include <svx/dialogs.hrc>
#include <svx/svxdlg.hxx>

// --------------------
// - SvxGraphicFilter -
// --------------------

ULONG SvxGraphicFilter::ExecuteGrfFilterSlot( SfxRequest& rReq, GraphicObject& rFilterObject )
{
    const Graphic&  rGraphic = rFilterObject.GetGraphic();
    ULONG           nRet = SVX_GRAPHICFILTER_UNSUPPORTED_GRAPHICTYPE;

    if( rGraphic.GetType() == GRAPHIC_BITMAP )
    {
        SfxViewFrame*   pViewFrame = SfxViewFrame::Current();
        SfxObjectShell* pShell = pViewFrame ? pViewFrame->GetObjectShell() : NULL;
        Window*         pWindow = ( pViewFrame && pViewFrame->GetViewShell() ) ? pViewFrame->GetViewShell()->GetWindow() : NULL;
        Graphic         aGraphic;

        switch( rReq.GetSlot() )
        {
            case( SID_GRFFILTER_INVERT ):
            {
                if( pShell )
                    pShell->SetWaitCursor( TRUE );

                if( rGraphic.IsAnimated() )
                {
                    Animation aAnimation( rGraphic.GetAnimation() );

                    if( aAnimation.Invert() )
                        aGraphic = aAnimation;
                }
                else
                {
                    BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

                    if( aBmpEx.Invert() )
                        aGraphic = aBmpEx;
                }

                if( pShell )
                    pShell->SetWaitCursor( FALSE );
            }
            break;

            case( SID_GRFFILTER_SMOOTH ):
            {
                if( pShell )
                    pShell->SetWaitCursor( TRUE );

                if( rGraphic.IsAnimated() )
                {
                    Animation aAnimation( rGraphic.GetAnimation() );

                    if( aAnimation.Filter( BMP_FILTER_SMOOTH ) )
                        aGraphic = aAnimation;
                }
                else
                {
                    BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

                    if( aBmpEx.Filter( BMP_FILTER_SMOOTH ) )
                        aGraphic = aBmpEx;
                }

                if( pShell )
                    pShell->SetWaitCursor( FALSE );
            }
            break;

            case( SID_GRFFILTER_SHARPEN ):
            {
                if( pShell )
                    pShell->SetWaitCursor( TRUE );

                if( rGraphic.IsAnimated() )
                {
                    Animation aAnimation( rGraphic.GetAnimation() );

                    if( aAnimation.Filter( BMP_FILTER_SHARPEN ) )
                        aGraphic = aAnimation;
                }
                else
                {
                    BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

                    if( aBmpEx.Filter( BMP_FILTER_SHARPEN ) )
                        aGraphic = aBmpEx;
                }

                if( pShell )
                    pShell->SetWaitCursor( FALSE );
            }
            break;

            case( SID_GRFFILTER_REMOVENOISE ):
            {
                if( pShell )
                    pShell->SetWaitCursor( TRUE );

                if( rGraphic.IsAnimated() )
                {
                    Animation aAnimation( rGraphic.GetAnimation() );

                    if( aAnimation.Filter( BMP_FILTER_REMOVENOISE ) )
                        aGraphic = aAnimation;
                }
                else
                {
                    BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

                    if( aBmpEx.Filter( BMP_FILTER_REMOVENOISE ) )
                        aGraphic = aBmpEx;
                }

                if( pShell )
                    pShell->SetWaitCursor( FALSE );
            }
            break;

            case( SID_GRFFILTER_SOBEL ):
            {
                if( pShell )
                    pShell->SetWaitCursor( TRUE );

                if( rGraphic.IsAnimated() )
                {
                    Animation aAnimation( rGraphic.GetAnimation() );

                    if( aAnimation.Filter( BMP_FILTER_SOBEL_GREY ) )
                        aGraphic = aAnimation;
                }
                else
                {
                    BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

                    if( aBmpEx.Filter( BMP_FILTER_SOBEL_GREY ) )
                        aGraphic = aBmpEx;
                }

                if( pShell )
                    pShell->SetWaitCursor( FALSE );
            }
            break;

            case( SID_GRFFILTER_MOSAIC ):
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if(pFact)
                {
                    AbstractGraphicFilterDialog* aDlg = pFact->CreateGraphicFilterMosaic( pWindow, rGraphic, 4, 4, FALSE, RID_SVX_GRFFILTER_DLG_MOSAIC);
                    DBG_ASSERT(aDlg, "Dialogdiet fail!");
                    if( aDlg->Execute() == RET_OK )
                        aGraphic = aDlg->GetFilteredGraphic( rGraphic, 1.0, 1.0 );
                    delete aDlg;
                }
            }
            break;

            case( SID_GRFFILTER_EMBOSS  ):
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if(pFact)
                {
                    AbstractGraphicFilterDialog* aDlg = pFact->CreateGraphicFilterEmboss( pWindow, rGraphic, RP_MM, RID_SVX_GRFFILTER_DLG_EMBOSS );
                    DBG_ASSERT(aDlg, "Dialogdiet fail!");
                    if( aDlg->Execute() == RET_OK )
                        aGraphic = aDlg->GetFilteredGraphic( rGraphic, 1.0, 1.0 );
                    delete aDlg;
                }
            }
            break;

            case( SID_GRFFILTER_POSTER  ):
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if(pFact)
                {
                    AbstractGraphicFilterDialog* aDlg = pFact->CreateGraphicFilterPosterSepia( pWindow, rGraphic, 16, RID_SVX_GRFFILTER_DLG_POSTER );
                    DBG_ASSERT(aDlg, "Dialogdiet fail!");
                    if( aDlg->Execute() == RET_OK )
                        aGraphic = aDlg->GetFilteredGraphic( rGraphic, 1.0, 1.0 );
                    delete aDlg;
                }
            }
            break;

            case( SID_GRFFILTER_POPART  ):
            {
                if( pShell )
                    pShell->SetWaitCursor( TRUE );

                if( rGraphic.IsAnimated() )
                {
                    Animation aAnimation( rGraphic.GetAnimation() );

                    if( aAnimation.Filter( BMP_FILTER_POPART ) )
                        aGraphic = aAnimation;
                }
                else
                {
                    BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

                    if( aBmpEx.Filter( BMP_FILTER_POPART ) )
                        aGraphic = aBmpEx;
                }

                if( pShell )
                    pShell->SetWaitCursor( FALSE );
            }
            break;

            case( SID_GRFFILTER_SEPIA ):
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if(pFact)
                {
                    AbstractGraphicFilterDialog* aDlg = pFact->CreateGraphicFilterPosterSepia( pWindow, rGraphic, 10, RID_SVX_GRFFILTER_DLG_SEPIA );
                    DBG_ASSERT(aDlg, "Dialogdiet fail!");
                    if( aDlg->Execute() == RET_OK )
                        aGraphic = aDlg->GetFilteredGraphic( rGraphic, 1.0, 1.0 );
                    delete aDlg;
                }
            }
            break;

            case( SID_GRFFILTER_SOLARIZE ):
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if(pFact)
                {
                    AbstractGraphicFilterDialog* aDlg = pFact->CreateGraphicFilterSolarize( pWindow, rGraphic, 128, FALSE, RID_SVX_GRFFILTER_DLG_SOLARIZE );
                    DBG_ASSERT(aDlg, "Dialogdiet fail!");
                    if( aDlg->Execute() == RET_OK )
                        aGraphic = aDlg->GetFilteredGraphic( rGraphic, 1.0, 1.0 );
                    delete aDlg;
                }
            }
            break;

            case SID_GRFFILTER :
            {
                // do nothing; no error
                nRet = SVX_GRAPHICFILTER_ERRCODE_NONE;
                break;
            }

            default:
            {
                OSL_FAIL( "SvxGraphicFilter: selected filter slot not yet implemented" );
                nRet = SVX_GRAPHICFILTER_UNSUPPORTED_SLOT;
            }
            break;
        }

        if( aGraphic.GetType() != GRAPHIC_NONE )
        {
            rFilterObject.SetGraphic( aGraphic );
            nRet = SVX_GRAPHICFILTER_ERRCODE_NONE;
        }
    }

    return nRet;
}

// -----------------------------------------------------------------------------

void SvxGraphicFilter::DisableGraphicFilterSlots( SfxItemSet& rSet )
{
    if( SFX_ITEM_AVAILABLE <= rSet.GetItemState( SID_GRFFILTER ) )
        rSet.DisableItem( SID_GRFFILTER );

    if( SFX_ITEM_AVAILABLE <= rSet.GetItemState( SID_GRFFILTER_INVERT ) )
        rSet.DisableItem( SID_GRFFILTER_INVERT );

    if( SFX_ITEM_AVAILABLE <= rSet.GetItemState( SID_GRFFILTER_SMOOTH ) )
        rSet.DisableItem( SID_GRFFILTER_SMOOTH );

    if( SFX_ITEM_AVAILABLE <= rSet.GetItemState( SID_GRFFILTER_SHARPEN ) )
        rSet.DisableItem( SID_GRFFILTER_SHARPEN );

    if( SFX_ITEM_AVAILABLE <= rSet.GetItemState( SID_GRFFILTER_REMOVENOISE ) )
        rSet.DisableItem( SID_GRFFILTER_REMOVENOISE );

    if( SFX_ITEM_AVAILABLE <= rSet.GetItemState( SID_GRFFILTER_SOBEL ) )
        rSet.DisableItem( SID_GRFFILTER_SOBEL );

    if( SFX_ITEM_AVAILABLE <= rSet.GetItemState( SID_GRFFILTER_MOSAIC ) )
        rSet.DisableItem( SID_GRFFILTER_MOSAIC );

    if( SFX_ITEM_AVAILABLE <= rSet.GetItemState( SID_GRFFILTER_EMBOSS ) )
        rSet.DisableItem( SID_GRFFILTER_EMBOSS );

    if( SFX_ITEM_AVAILABLE <= rSet.GetItemState( SID_GRFFILTER_POSTER ) )
        rSet.DisableItem( SID_GRFFILTER_POSTER );

    if( SFX_ITEM_AVAILABLE <= rSet.GetItemState( SID_GRFFILTER_POPART ) )
        rSet.DisableItem( SID_GRFFILTER_POPART );

    if( SFX_ITEM_AVAILABLE <= rSet.GetItemState( SID_GRFFILTER_SEPIA ) )
        rSet.DisableItem( SID_GRFFILTER_SEPIA );

    if( SFX_ITEM_AVAILABLE <= rSet.GetItemState( SID_GRFFILTER_SOLARIZE ) )
        rSet.DisableItem( SID_GRFFILTER_SOLARIZE );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
