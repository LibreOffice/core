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

#include <vcl/msgbox.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>

#include <svx/dialmgr.hxx>
#include <svx/grfflt.hxx>
#include <svx/strings.hrc>
#include <svx/svxids.hrc>
#include <svx/svxdlg.hxx>
#include <memory>


sal_uIntPtr SvxGraphicFilter::ExecuteGrfFilterSlot( SfxRequest& rReq, GraphicObject& rFilterObject )
{
    const Graphic&  rGraphic = rFilterObject.GetGraphic();
    sal_uIntPtr         nRet = SVX_GRAPHICFILTER_UNSUPPORTED_GRAPHICTYPE;

    if( rGraphic.GetType() == GraphicType::Bitmap )
    {
        SfxViewFrame*   pViewFrame = SfxViewFrame::Current();
        SfxObjectShell* pShell = pViewFrame ? pViewFrame->GetObjectShell() : nullptr;
        vcl::Window*         pWindow = ( pViewFrame && pViewFrame->GetViewShell() ) ? pViewFrame->GetViewShell()->GetWindow() : nullptr;
        Graphic         aGraphic;

        switch( rReq.GetSlot() )
        {
            case SID_GRFFILTER_INVERT:
            {
                if( pShell )
                    pShell->SetWaitCursor( true );

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
                    pShell->SetWaitCursor( false );
            }
            break;

            case SID_GRFFILTER_SMOOTH:
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if(pFact)
                {
                    ScopedVclPtr<AbstractGraphicFilterDialog> aDlg(pFact->CreateGraphicFilterSmooth(pWindow, rGraphic, 0.7));
                    DBG_ASSERT(aDlg, "Dialog creation failed!");
                    if( aDlg->Execute() == RET_OK )
                        aGraphic = aDlg->GetFilteredGraphic( rGraphic, 1.0, 1.0 );
                }
            }
            break;

            case SID_GRFFILTER_SHARPEN:
            {
                if( pShell )
                    pShell->SetWaitCursor( true );

                if( rGraphic.IsAnimated() )
                {
                    Animation aAnimation( rGraphic.GetAnimation() );

                    if( aAnimation.Filter( BmpFilter::Sharpen ) )
                        aGraphic = aAnimation;
                }
                else
                {
                    BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

                    if( aBmpEx.Filter( BmpFilter::Sharpen ) )
                        aGraphic = aBmpEx;
                }

                if( pShell )
                    pShell->SetWaitCursor( false );
            }
            break;

            case SID_GRFFILTER_REMOVENOISE:
            {
                if( pShell )
                    pShell->SetWaitCursor( true );

                if( rGraphic.IsAnimated() )
                {
                    Animation aAnimation( rGraphic.GetAnimation() );

                    if( aAnimation.Filter( BmpFilter::RemoveNoise ) )
                        aGraphic = aAnimation;
                }
                else
                {
                    BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

                    if( aBmpEx.Filter( BmpFilter::RemoveNoise ) )
                        aGraphic = aBmpEx;
                }

                if( pShell )
                    pShell->SetWaitCursor( false );
            }
            break;

            case SID_GRFFILTER_SOBEL:
            {
                if( pShell )
                    pShell->SetWaitCursor( true );

                if( rGraphic.IsAnimated() )
                {
                    Animation aAnimation( rGraphic.GetAnimation() );

                    if( aAnimation.Filter( BmpFilter::SobelGrey ) )
                        aGraphic = aAnimation;
                }
                else
                {
                    BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

                    if( aBmpEx.Filter( BmpFilter::SobelGrey ) )
                        aGraphic = aBmpEx;
                }

                if( pShell )
                    pShell->SetWaitCursor( false );
            }
            break;

            case SID_GRFFILTER_MOSAIC:
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if(pFact)
                {
                    ScopedVclPtr<AbstractGraphicFilterDialog> aDlg(pFact->CreateGraphicFilterMosaic(pWindow, rGraphic));
                    DBG_ASSERT(aDlg, "Dialog creation failed!");
                    if( aDlg->Execute() == RET_OK )
                        aGraphic = aDlg->GetFilteredGraphic( rGraphic, 1.0, 1.0 );
                }
            }
            break;

            case SID_GRFFILTER_EMBOSS:
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if(pFact)
                {
                    ScopedVclPtr<AbstractGraphicFilterDialog> aDlg(pFact->CreateGraphicFilterEmboss(pWindow, rGraphic));
                    DBG_ASSERT(aDlg, "Dialog creation failed!");
                    if( aDlg->Execute() == RET_OK )
                        aGraphic = aDlg->GetFilteredGraphic( rGraphic, 1.0, 1.0 );
                }
            }
            break;

            case SID_GRFFILTER_POSTER:
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if(pFact)
                {
                    ScopedVclPtr<AbstractGraphicFilterDialog> aDlg(pFact->CreateGraphicFilterPoster(pWindow, rGraphic));
                    DBG_ASSERT(aDlg, "Dialog creation failed!");
                    if( aDlg->Execute() == RET_OK )
                        aGraphic = aDlg->GetFilteredGraphic( rGraphic, 1.0, 1.0 );
                }
            }
            break;

            case SID_GRFFILTER_POPART:
            {
                if( pShell )
                    pShell->SetWaitCursor( true );

                if( rGraphic.IsAnimated() )
                {
                    Animation aAnimation( rGraphic.GetAnimation() );

                    if( aAnimation.Filter( BmpFilter::PopArt ) )
                        aGraphic = aAnimation;
                }
                else
                {
                    BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

                    if( aBmpEx.Filter( BmpFilter::PopArt ) )
                        aGraphic = aBmpEx;
                }

                if( pShell )
                    pShell->SetWaitCursor( false );
            }
            break;

            case SID_GRFFILTER_SEPIA:
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if(pFact)
                {
                    ScopedVclPtr<AbstractGraphicFilterDialog> aDlg(pFact->CreateGraphicFilterSepia(pWindow, rGraphic));
                    DBG_ASSERT(aDlg, "Dialog creation failed!");
                    if( aDlg->Execute() == RET_OK )
                        aGraphic = aDlg->GetFilteredGraphic( rGraphic, 1.0, 1.0 );
                }
            }
            break;

            case SID_GRFFILTER_SOLARIZE:
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if(pFact)
                {
                    ScopedVclPtr<AbstractGraphicFilterDialog> aDlg(pFact->CreateGraphicFilterSolarize(pWindow, rGraphic));
                    DBG_ASSERT(aDlg, "Dialog creation failed!");
                    if( aDlg->Execute() == RET_OK )
                        aGraphic = aDlg->GetFilteredGraphic( rGraphic, 1.0, 1.0 );
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

        if( aGraphic.GetType() != GraphicType::NONE )
        {
            rFilterObject.SetGraphic( aGraphic );
            nRet = SVX_GRAPHICFILTER_ERRCODE_NONE;
        }
    }

    return nRet;
}


void SvxGraphicFilter::DisableGraphicFilterSlots( SfxItemSet& rSet )
{
    if( SfxItemState::DEFAULT <= rSet.GetItemState( SID_GRFFILTER ) )
        rSet.DisableItem( SID_GRFFILTER );

    if( SfxItemState::DEFAULT <= rSet.GetItemState( SID_GRFFILTER_INVERT ) )
        rSet.DisableItem( SID_GRFFILTER_INVERT );

    if( SfxItemState::DEFAULT <= rSet.GetItemState( SID_GRFFILTER_SMOOTH ) )
        rSet.DisableItem( SID_GRFFILTER_SMOOTH );

    if( SfxItemState::DEFAULT <= rSet.GetItemState( SID_GRFFILTER_SHARPEN ) )
        rSet.DisableItem( SID_GRFFILTER_SHARPEN );

    if( SfxItemState::DEFAULT <= rSet.GetItemState( SID_GRFFILTER_REMOVENOISE ) )
        rSet.DisableItem( SID_GRFFILTER_REMOVENOISE );

    if( SfxItemState::DEFAULT <= rSet.GetItemState( SID_GRFFILTER_SOBEL ) )
        rSet.DisableItem( SID_GRFFILTER_SOBEL );

    if( SfxItemState::DEFAULT <= rSet.GetItemState( SID_GRFFILTER_MOSAIC ) )
        rSet.DisableItem( SID_GRFFILTER_MOSAIC );

    if( SfxItemState::DEFAULT <= rSet.GetItemState( SID_GRFFILTER_EMBOSS ) )
        rSet.DisableItem( SID_GRFFILTER_EMBOSS );

    if( SfxItemState::DEFAULT <= rSet.GetItemState( SID_GRFFILTER_POSTER ) )
        rSet.DisableItem( SID_GRFFILTER_POSTER );

    if( SfxItemState::DEFAULT <= rSet.GetItemState( SID_GRFFILTER_POPART ) )
        rSet.DisableItem( SID_GRFFILTER_POPART );

    if( SfxItemState::DEFAULT <= rSet.GetItemState( SID_GRFFILTER_SEPIA ) )
        rSet.DisableItem( SID_GRFFILTER_SEPIA );

    if( SfxItemState::DEFAULT <= rSet.GetItemState( SID_GRFFILTER_SOLARIZE ) )
        rSet.DisableItem( SID_GRFFILTER_SOLARIZE );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
