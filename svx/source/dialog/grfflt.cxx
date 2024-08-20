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

#include <osl/diagnose.h>
#include <svx/grfflt.hxx>
#include <svx/svxids.hrc>
#include <svx/svxdlg.hxx>


static void handleGraphicFilterDialog(const VclPtr<AbstractGraphicFilterDialog>& pDlg,
        const Graphic& aInputGraphic,
        const std::function<void(const Graphic&)>& f);

void SvxGraphicFilter::ExecuteGrfFilterSlot( SfxRequest const & rReq,
                        const GraphicObject& rInputObject,
                        const std::function<void(const Graphic&)>& f)
{
    Graphic aInputGraphic = rInputObject.GetGraphic();

    if( aInputGraphic.GetType() != GraphicType::Bitmap )
        return;

    SfxViewFrame*   pViewFrame = SfxViewFrame::Current();
    SfxObjectShell* pShell = pViewFrame ? pViewFrame->GetObjectShell() : nullptr;
    weld::Window*   pFrameWeld = (pViewFrame && pViewFrame->GetViewShell()) ? pViewFrame->GetViewShell()->GetFrameWeld() : nullptr;

    switch( rReq.GetSlot() )
    {
        case SID_GRFFILTER_INVERT:
        {
            Graphic aOutputGraphic;

            if( pShell )
                pShell->SetWaitCursor( true );

            if( aInputGraphic.IsAnimated() )
            {
                Animation aAnimation( aInputGraphic.GetAnimation() );

                if( aAnimation.Invert() )
                    aOutputGraphic = aAnimation;
            }
            else
            {
                BitmapEx aBmpEx( aInputGraphic.GetBitmapEx() );

                if( aBmpEx.Invert() )
                    aOutputGraphic = aBmpEx;
            }

            if( pShell )
                pShell->SetWaitCursor( false );

            if( aOutputGraphic.GetType() != GraphicType::NONE )
                f(aOutputGraphic);
        }
        break;

        case SID_GRFFILTER_SMOOTH:
        {
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            handleGraphicFilterDialog(pFact->CreateGraphicFilterSmooth(pFrameWeld, aInputGraphic, 0.7), aInputGraphic, f);
        }
        break;

        case SID_GRFFILTER_SHARPEN:
        {
            Graphic aOutputGraphic;

            if( pShell )
                pShell->SetWaitCursor( true );

            if( aInputGraphic.IsAnimated() )
            {
                Animation aAnimation( aInputGraphic.GetAnimation() );

                if (BitmapFilter::Filter(aAnimation, BitmapSharpenFilter()))
                    aOutputGraphic = aAnimation;
            }
            else
            {
                BitmapEx aBmpEx( aInputGraphic.GetBitmapEx() );

                if (BitmapFilter::Filter(aBmpEx, BitmapSharpenFilter()))
                    aOutputGraphic = aBmpEx;
            }

            if( pShell )
                pShell->SetWaitCursor( false );

            if( aOutputGraphic.GetType() != GraphicType::NONE )
                f(aOutputGraphic);
        }
        break;

        case SID_GRFFILTER_REMOVENOISE:
        {
            Graphic aOutputGraphic;

            if( pShell )
                pShell->SetWaitCursor( true );

            if( aInputGraphic.IsAnimated() )
            {
                Animation aAnimation( aInputGraphic.GetAnimation() );

                if (BitmapFilter::Filter(aAnimation, BitmapMedianFilter()))
                    aOutputGraphic = aAnimation;
            }
            else
            {
                BitmapEx aBmpEx( aInputGraphic.GetBitmapEx() );

                if (BitmapFilter::Filter(aBmpEx, BitmapMedianFilter()))
                    aOutputGraphic = aBmpEx;
            }

            if( pShell )
                pShell->SetWaitCursor( false );

            if( aOutputGraphic.GetType() != GraphicType::NONE )
                f(aOutputGraphic);
        }
        break;

        case SID_GRFFILTER_SOBEL:
        {
            Graphic aOutputGraphic;

            if( pShell )
                pShell->SetWaitCursor( true );

            if( aInputGraphic.IsAnimated() )
            {
                Animation aAnimation( aInputGraphic.GetAnimation() );

                if (BitmapFilter::Filter(aAnimation, BitmapSobelGreyFilter()))
                    aOutputGraphic = aAnimation;
            }
            else
            {
                BitmapEx aBmpEx( aInputGraphic.GetBitmapEx() );

                if (BitmapFilter::Filter(aBmpEx, BitmapSobelGreyFilter()))
                    aOutputGraphic = aBmpEx;
            }

            if( pShell )
                pShell->SetWaitCursor( false );

            if( aOutputGraphic.GetType() != GraphicType::NONE )
                f(aOutputGraphic);
        }
        break;

        case SID_GRFFILTER_MOSAIC:
        {
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            handleGraphicFilterDialog(pFact->CreateGraphicFilterMosaic(pFrameWeld, aInputGraphic), aInputGraphic, f);
        }
        break;

        case SID_GRFFILTER_EMBOSS:
        {
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            handleGraphicFilterDialog(pFact->CreateGraphicFilterEmboss(pFrameWeld, aInputGraphic), aInputGraphic, f);
        }
        break;

        case SID_GRFFILTER_POSTER:
        {
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            handleGraphicFilterDialog(pFact->CreateGraphicFilterPoster(pFrameWeld, aInputGraphic), aInputGraphic, f);
        }
        break;

        case SID_GRFFILTER_POPART:
        {
            Graphic aOutputGraphic;

            if( pShell )
                pShell->SetWaitCursor( true );

            if( aInputGraphic.IsAnimated() )
            {
                Animation aAnimation( aInputGraphic.GetAnimation() );

                if (BitmapFilter::Filter(aAnimation, BitmapPopArtFilter()))
                    aOutputGraphic = aAnimation;
            }
            else
            {
                BitmapEx aBmpEx( aInputGraphic.GetBitmapEx() );

                if (BitmapFilter::Filter(aBmpEx, BitmapPopArtFilter()))
                    aOutputGraphic = aBmpEx;
            }

            if( pShell )
                pShell->SetWaitCursor( false );

            if( aOutputGraphic.GetType() != GraphicType::NONE )
                f(aOutputGraphic);
        }
        break;

        case SID_GRFFILTER_SEPIA:
        {
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            handleGraphicFilterDialog(pFact->CreateGraphicFilterSepia(pFrameWeld, aInputGraphic), aInputGraphic, f);
        }
        break;

        case SID_GRFFILTER_SOLARIZE:
        {
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            handleGraphicFilterDialog(pFact->CreateGraphicFilterSolarize(pFrameWeld, aInputGraphic), aInputGraphic, f);
        }
        break;

        case SID_GRFFILTER :
        {
            // do nothing; no error
            return;
        }

        default:
        {
            OSL_FAIL( "SvxGraphicFilter: selected filter slot not yet implemented" );
            return;
        }
    }
}

static void handleGraphicFilterDialog(const VclPtr<AbstractGraphicFilterDialog>& pDlg,
        const Graphic& aInputGraphic,
        const std::function<void(const Graphic&)>& f)
{
    pDlg->StartExecuteAsync(
        [pDlg, aInputGraphic, f] (sal_Int32 nResult)->void
        {
            if (nResult == RET_OK)
                f(pDlg->GetFilteredGraphic(aInputGraphic, 1.0, 1.0));
            pDlg->disposeOnce();
        }
    );
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
