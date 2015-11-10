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

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include "slideshow.hxx"
#include <sfx2/objsh.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/virdev.hxx>
#include <com/sun/star/presentation/FadeEffect.hpp>
#include <fadedef.h>
#include <vcl/ctrl.hxx>
#include <vcl/builder.hxx>
#include <vcl/settings.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdorect.hxx>

#include "docprev.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "ViewShell.hxx"
#include "ViewShellBase.hxx"
#include "drawview.hxx"
#include "sdpage.hxx"
#include "sfx2/viewfrm.hxx"
#include <vcl/svapp.hxx>
#include <vcl/builderfactory.hxx>

using ::com::sun::star::drawing::XDrawPage;
using ::com::sun::star::animations::XAnimationNode;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

const int SdDocPreviewWin::FRAME = 4;

void SdDocPreviewWin::SetObjectShell( SfxObjectShell* pObj, sal_uInt16 nShowPage )
{
    mpObj = pObj;
    mnShowPage = nShowPage;
    if (mxSlideShow.is())
    {
        mxSlideShow->end();
        mxSlideShow.clear();
    }
    updateViewSettings();
}

VCL_BUILDER_DECL_FACTORY(SdDocPreviewWin)
{
    WinBits nWinStyle = 0;

    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;

    rRet = VclPtr<SdDocPreviewWin>::Create(pParent, nWinStyle);
}

SdDocPreviewWin::SdDocPreviewWin( vcl::Window* pParent, const WinBits nStyle )
: Control(pParent, nStyle), pMetaFile( nullptr ), bInEffect(false), mpObj(nullptr), mnShowPage(0)
{
    SetBorderStyle( WindowBorderStyle::MONO );
    svtools::ColorConfig aColorConfig;
    Wallpaper aEmpty;
    SetBackground( aEmpty );
}

SdDocPreviewWin::~SdDocPreviewWin()
{
    disposeOnce();
}

void SdDocPreviewWin::dispose()
{
    delete pMetaFile;
    Control::dispose();
}

Size SdDocPreviewWin::GetOptimalSize() const
{
    return LogicToPixel(Size(122, 96), MAP_APPFONT);
}

void SdDocPreviewWin::Resize()
{
    Invalidate();
    if( mxSlideShow.is() )
        mxSlideShow->resize( GetSizePixel() );
}

void SdDocPreviewWin::CalcSizeAndPos( GDIMetaFile* pFile, Size& rSize, Point& rPoint )
{
    Size aTmpSize = pFile ? pFile->GetPrefSize() : Size(1,1 );
    long nWidth = rSize.Width() - 2*FRAME;
    long nHeight = rSize.Height() - 2*FRAME;
    if( nWidth < 0 ) nWidth = 0;
    if( nHeight < 0 ) nHeight = 0;

    double dRatio=((double)aTmpSize.Width())/aTmpSize.Height();
    double dRatioPreV = nHeight ? (((double) nWidth ) / nHeight) : 0.0;

    if (dRatio > dRatioPreV)
    {
        rSize=Size(nWidth, (sal_uInt16)(nWidth/dRatio));
        rPoint=Point( 0, (sal_uInt16)((nHeight-rSize.Height())/2));
    }
    else
    {
        rSize=Size((sal_uInt16)(nHeight*dRatio), nHeight);
        rPoint=Point((sal_uInt16)((nWidth-rSize.Width())/2),0);
    }
}

void SdDocPreviewWin::ImpPaint( GDIMetaFile* pFile, OutputDevice* pVDev )
{
    Point aPoint;
    Size aSize = pVDev->GetOutputSize();
    Point bPoint(aSize.Width()-2*FRAME, aSize.Height()-2*FRAME );
    CalcSizeAndPos( pFile, aSize, aPoint );
    bPoint -= aPoint;
    aPoint += Point( FRAME, FRAME );

    svtools::ColorConfig aColorConfig;

    pVDev->SetLineColor();
    pVDev->SetFillColor( Color( aColorConfig.GetColorValue( svtools::APPBACKGROUND ).nColor ) );
    pVDev->DrawRect(Rectangle( Point(0,0 ), pVDev->GetOutputSize()));
    if( pFile )
    {
        pVDev->SetFillColor( maDocumentColor );
        pVDev->DrawRect(Rectangle(aPoint, aSize));
        pFile->WindStart();
        pFile->Play( pVDev, aPoint, aSize  );
    }
}

void SdDocPreviewWin::Paint( vcl::RenderContext& /*rRenderContext*/, const Rectangle& rRect )
{
    if( (!mxSlideShow.is()) || (!mxSlideShow->isRunning() ) )
    {
        SvtAccessibilityOptions aAccOptions;
        bool bUseContrast = aAccOptions.GetIsForPagePreviews() && Application::GetSettings().GetStyleSettings().GetHighContrastMode();
        SetDrawMode( bUseContrast
            ? ::sd::OUTPUT_DRAWMODE_CONTRAST
            : ::sd::OUTPUT_DRAWMODE_COLOR );

        ImpPaint( pMetaFile, this );
    }
    else
    {
        mxSlideShow->paint( rRect );
    }
}

void SdDocPreviewWin::startPreview()
{
    ::sd::DrawDocShell* pDocShell = dynamic_cast< ::sd::DrawDocShell * >( mpObj );
    if( pDocShell )
    {
        SdDrawDocument* pDoc = pDocShell->GetDoc();

        if( pDoc )
        {
            SdPage* pPage = pDoc->GetSdPage( mnShowPage, PK_STANDARD );

            if( pPage && (pPage->getTransitionType() != 0) )
            {
                if( !mxSlideShow.is() )
                    mxSlideShow = sd::SlideShow::Create( pDoc );

                Reference< XDrawPage > xDrawPage( pPage->getUnoPage(), UNO_QUERY );
                Reference< XAnimationNode > xAnimationNode;

                mxSlideShow->startPreview( xDrawPage, xAnimationNode, this );
            }
        }
    }
}

bool SdDocPreviewWin::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == MouseNotifyEvent::MOUSEBUTTONDOWN )
    {
        const MouseEvent* pMEvt = rNEvt.GetMouseEvent();
        if ( pMEvt->IsLeft() )
        {
            if( rNEvt.GetWindow() == this )
            {
                aClickHdl.Call(*this);
            }
        }
    }

    return Control::Notify( rNEvt );
}

void SdDocPreviewWin::updateViewSettings()
{
    ::sd::DrawDocShell* pDocShell = dynamic_cast< ::sd::DrawDocShell *>( mpObj );
    SdDrawDocument* pDoc = pDocShell?pDocShell->GetDoc():nullptr;

    SvtAccessibilityOptions aAccOptions;
    bool bUseWhiteColor = !aAccOptions.GetIsForPagePreviews() && GetSettings().GetStyleSettings().GetHighContrastMode();
    if( bUseWhiteColor )
    {
        maDocumentColor = Color( COL_WHITE );
    }
    else
    {
        svtools::ColorConfig aColorConfig;
        maDocumentColor = Color( aColorConfig.GetColorValue( svtools::DOCCOLOR ).nColor );
    }

    GDIMetaFile* pMtf = nullptr;

    if(pDoc)
    {
        SdPage * pPage = pDoc->GetSdPage( mnShowPage, PK_STANDARD );
        if( pPage )
        {
            SdrOutliner& rOutl = pDoc->GetDrawOutliner();
            Color aOldBackgroundColor = rOutl.GetBackgroundColor();
            rOutl.SetBackgroundColor( maDocumentColor );

            pMtf = new GDIMetaFile;

            ScopedVclPtrInstance< VirtualDevice > pVDev;

            const Fraction      aFrac( pDoc->GetScaleFraction() );
            const MapMode       aMap( pDoc->GetScaleUnit(), Point(), aFrac, aFrac );

            pVDev->SetMapMode( aMap );

            // Disable output, as we only want to record a metafile
            pVDev->EnableOutput( false );

            pMtf->Record( pVDev );

            ::sd::DrawView* pView = new ::sd::DrawView(pDocShell, this, nullptr);

            const Size aSize( pPage->GetSize() );

            pView->SetBordVisible( false );
            pView->SetPageVisible( false );
            pView->ShowSdrPage( pPage );

            const Point aNewOrg( pPage->GetLftBorder(), pPage->GetUppBorder() );
            const Size aNewSize( aSize.Width() - pPage->GetLftBorder() - pPage->GetRgtBorder(),
                                  aSize.Height() - pPage->GetUppBorder() - pPage->GetLwrBorder() );
            const Rectangle aClipRect( aNewOrg, aNewSize );
            MapMode         aVMap( aMap );

            pVDev->Push();
            aVMap.SetOrigin( Point( -aNewOrg.X(), -aNewOrg.Y() ) );
            pVDev->SetRelativeMapMode( aVMap );
            pVDev->IntersectClipRegion( aClipRect );

        // Use new StandardCheckVisisbilityRedirector
        StandardCheckVisisbilityRedirector aRedirector;
        const Rectangle aRedrawRectangle( Point(), aNewSize );
        vcl::Region aRedrawRegion(aRedrawRectangle);
        pView->SdrPaintView::CompleteRedraw(pVDev,aRedrawRegion,&aRedirector);

            pVDev->Pop();

            pMtf->Stop();
            pMtf->WindStart();
            pMtf->SetPrefMapMode( aMap );
            pMtf->SetPrefSize( aNewSize );

            rOutl.SetBackgroundColor( aOldBackgroundColor );

            delete pView;
        }
    }

    delete pMetaFile;
    pMetaFile = pMtf;

    Invalidate();
}

void SdDocPreviewWin::Notify(SfxBroadcaster&, const SfxHint& rHint)
{
    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if( pSimpleHint && pSimpleHint->GetId() == SFX_HINT_COLORS_CHANGED )
    {
        updateViewSettings();
    }
}
void SdDocPreviewWin::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        updateViewSettings();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
