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

#include <vcl/BitmapSharpenFilter.hxx>
#include <vcl/BitmapMedianFilter.hxx>
#include <vcl/BitmapSobelGreyFilter.hxx>
#include <vcl/BitmapPopArtFilter.hxx>

#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>

#include <svx/grfflt.hxx>
#include <svx/svxids.hrc>
#include <svx/svxdlg.hxx>
#include <memory>


SvxGraphicFilterResult SvxGraphicFilter::ExecuteGrfFilterSlot( SfxRequest const & rReq, GraphicObject& rFilterObject )
{
    const Graphic&  rGraphic = rFilterObject.GetGraphic();
    SvxGraphicFilterResult nRet = SvxGraphicFilterResult::UnsupportedGraphicType;

    if( rGraphic.GetType() == GraphicType::Bitmap )
    {
        SfxViewFrame*   pViewFrame = SfxViewFrame::Current();
        SfxObjectShell* pShell = pViewFrame ? pViewFrame->GetObjectShell() : nullptr;
        weld::Window*   pFrameWeld = (pViewFrame && pViewFrame->GetViewShell()) ? pViewFrame->GetViewShell()->GetFrameWeld() : nullptr;
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
                ScopedVclPtr<AbstractGraphicFilterDialog> aDlg(pFact->CreateGraphicFilterSmooth(pFrameWeld, rGraphic, 0.7));
                if( aDlg->Execute() == RET_OK )
                    aGraphic = aDlg->GetFilteredGraphic( rGraphic, 1.0, 1.0 );
            }
            break;

            case SID_GRFFILTER_SHARPEN:
            {
                if( pShell )
                    pShell->SetWaitCursor( true );

                if( rGraphic.IsAnimated() )
                {
                    Animation aAnimation( rGraphic.GetAnimation() );

                    if (BitmapFilter::Filter(aAnimation, BitmapSharpenFilter()))
                        aGraphic = aAnimation;
                }
                else
                {
                    BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

                    if (BitmapFilter::Filter(aBmpEx, BitmapSharpenFilter()))
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

                    if (BitmapFilter::Filter(aAnimation, BitmapMedianFilter()))
                        aGraphic = aAnimation;
                }
                else
                {
                    BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

                    if (BitmapFilter::Filter(aBmpEx, BitmapMedianFilter()))
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

                    if (BitmapFilter::Filter(aAnimation, BitmapSobelGreyFilter()))
                        aGraphic = aAnimation;
                }
                else
                {
                    BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

                    if (BitmapFilter::Filter(aBmpEx, BitmapSobelGreyFilter()))
                        aGraphic = aBmpEx;
                }

                if( pShell )
                    pShell->SetWaitCursor( false );
            }
            break;

            case SID_GRFFILTER_MOSAIC:
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                ScopedVclPtr<AbstractGraphicFilterDialog> aDlg(pFact->CreateGraphicFilterMosaic(pFrameWeld, rGraphic));
                if( aDlg->Execute() == RET_OK )
                    aGraphic = aDlg->GetFilteredGraphic( rGraphic, 1.0, 1.0 );
            }
            break;

            case SID_GRFFILTER_EMBOSS:
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                ScopedVclPtr<AbstractGraphicFilterDialog> aDlg(pFact->CreateGraphicFilterEmboss(pFrameWeld, rGraphic));
                if( aDlg->Execute() == RET_OK )
                    aGraphic = aDlg->GetFilteredGraphic( rGraphic, 1.0, 1.0 );
            }
            break;

            case SID_GRFFILTER_POSTER:
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                ScopedVclPtr<AbstractGraphicFilterDialog> aDlg(pFact->CreateGraphicFilterPoster(pFrameWeld, rGraphic));
                if( aDlg->Execute() == RET_OK )
                    aGraphic = aDlg->GetFilteredGraphic( rGraphic, 1.0, 1.0 );
            }
            break;

            case SID_GRFFILTER_POPART:
            {
                if( pShell )
                    pShell->SetWaitCursor( true );

                if( rGraphic.IsAnimated() )
                {
                    Animation aAnimation( rGraphic.GetAnimation() );

                    if (BitmapFilter::Filter(aAnimation, BitmapPopArtFilter()))
                        aGraphic = aAnimation;
                }
                else
                {
                    BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

                    if (BitmapFilter::Filter(aBmpEx, BitmapPopArtFilter()))
                        aGraphic = aBmpEx;
                }

                if( pShell )
                    pShell->SetWaitCursor( false );
            }
            break;

            case SID_GRFFILTER_SEPIA:
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                ScopedVclPtr<AbstractGraphicFilterDialog> aDlg(pFact->CreateGraphicFilterSepia(pFrameWeld, rGraphic));
                if( aDlg->Execute() == RET_OK )
                    aGraphic = aDlg->GetFilteredGraphic( rGraphic, 1.0, 1.0 );
            }
            break;

            case SID_GRFFILTER_SOLARIZE:
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                ScopedVclPtr<AbstractGraphicFilterDialog> aDlg(pFact->CreateGraphicFilterSolarize(pFrameWeld, rGraphic));
                if( aDlg->Execute() == RET_OK )
                    aGraphic = aDlg->GetFilteredGraphic( rGraphic, 1.0, 1.0 );
            }
            break;

            case SID_GRFFILTER :
            {
                // do nothing; no error
                nRet = SvxGraphicFilterResult::NONE;
                break;
            }

            default:
            {
                OSL_FAIL( "SvxGraphicFilter: selected filter slot not yet implemented" );
                nRet = SvxGraphicFilterResult::UnsupportedSlot;
            }
            break;
        }

        if( aGraphic.GetType() != GraphicType::NONE )
        {
            rFilterObject.SetGraphic( aGraphic );
            nRet = SvxGraphicFilterResult::NONE;
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
