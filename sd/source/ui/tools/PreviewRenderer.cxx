/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "PreviewRenderer.hxx"

#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#include "drawview.hxx"
#include "sdpage.hxx"
#include "ViewShell.hxx"
#include <vcl/virdev.hxx>
#include <vcl/settings.hxx>

#include <svx/svdpagv.hxx>
#include <svx/svdoutl.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editstat.hxx>
#include <vcl/svapp.hxx>
#include <tools/diagnose_ex.h>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/viewcontact.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


namespace sd {

const int PreviewRenderer::snSubstitutionTextSize = 11;
const int PreviewRenderer::snFrameWidth = 1;

namespace {
    /** This incarnation of the ViewObjectContactRedirector filters away all
        PageObj objects, unconditionally.
    */
    class ViewRedirector : public ::sdr::contact::ViewObjectContactRedirector
    {
    public:
        ViewRedirector (void);
        virtual ~ViewRedirector (void);
        virtual drawinglayer::primitive2d::Primitive2DSequence createRedirectedPrimitive2DSequence(
            const sdr::contact::ViewObjectContact& rOriginal,
                const sdr::contact::DisplayInfo& rDisplayInfo);
    };
}






PreviewRenderer::PreviewRenderer (
    OutputDevice* pTemplate,
    const bool bHasFrame)
    : mpPreviewDevice (new VirtualDevice()),
      mpView(NULL),
      mpDocShellOfView(NULL),
      maFrameColor (svtools::ColorConfig().GetColorValue(svtools::DOCBOUNDARIES).nColor),
      mbHasFrame(bHasFrame)
{
    if (pTemplate != NULL)
    {
        mpPreviewDevice->SetDigitLanguage (pTemplate->GetDigitLanguage());
        mpPreviewDevice->SetBackground(pTemplate->GetBackground());
    }
    else
    {
        mpPreviewDevice->SetBackground(Wallpaper(
            Application::GetSettings().GetStyleSettings().GetWindowColor()));
    }
}




PreviewRenderer::~PreviewRenderer (void)
{
    if (mpDocShellOfView != NULL)
        EndListening (*mpDocShellOfView);
}




Image PreviewRenderer::RenderPage (
    const SdPage* pPage,
    const sal_Int32 nWidth,
    const OUString& rSubstitutionText,
    const bool bObeyHighContrastMode,
    const bool bDisplayPresentationObjects)
{
    if (pPage != NULL)
    {
        const Size aPageModelSize (pPage->GetSize());
        const double nAspectRatio (
            double(aPageModelSize.Width()) / double(aPageModelSize.Height()));
        const sal_Int32 nFrameWidth (mbHasFrame ? snFrameWidth : 0);
        const sal_Int32 nHeight (sal::static_int_cast<sal_Int32>(
            (nWidth - 2*nFrameWidth) / nAspectRatio + 2*nFrameWidth + 0.5));
        return RenderPage (
            pPage,
            Size(nWidth,nHeight),
            rSubstitutionText,
            bObeyHighContrastMode,
            bDisplayPresentationObjects);
    }
    else
        return Image();
}




Image PreviewRenderer::RenderPage (
    const SdPage* pPage,
    Size aPixelSize,
    const OUString& rSubstitutionText,
    const bool bObeyHighContrastMode,
    const bool bDisplayPresentationObjects)
{
    Image aPreview;

    if (pPage != NULL)
    {
        try
        {
            if (Initialize(pPage, aPixelSize, bObeyHighContrastMode))
            {
                PaintPage(pPage, bDisplayPresentationObjects);
                PaintSubstitutionText(rSubstitutionText);
                PaintFrame();

                Size aSize (mpPreviewDevice->GetOutputSizePixel());
                aPreview = Image(mpPreviewDevice->GetBitmap (
                    mpPreviewDevice->PixelToLogic(Point(0,0)),
                    mpPreviewDevice->PixelToLogic(aSize)));

                Cleanup();
            }
        }
        catch (const com::sun::star::uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    return aPreview;
}




Image PreviewRenderer::RenderSubstitution (
    const Size& rPreviewPixelSize,
    const OUString& rSubstitutionText)
{
    Image aPreview;

    try
    {
        
        mpPreviewDevice->SetOutputSizePixel(rPreviewPixelSize);

        
        const bool bUseContrast (
            Application::GetSettings().GetStyleSettings().GetHighContrastMode());
        mpPreviewDevice->SetDrawMode (bUseContrast
            ? ViewShell::OUTPUT_DRAWMODE_CONTRAST
            : ViewShell::OUTPUT_DRAWMODE_COLOR);

        
        
        MapMode aMapMode (mpPreviewDevice->GetMapMode());
        aMapMode.SetMapUnit(MAP_100TH_MM);
        const double nFinalScale (25.0 * rPreviewPixelSize.Width() / 28000.0);
        aMapMode.SetScaleX(nFinalScale);
        aMapMode.SetScaleY(nFinalScale);
        const sal_Int32 nFrameWidth (mbHasFrame ? snFrameWidth : 0);
        aMapMode.SetOrigin(mpPreviewDevice->PixelToLogic(
            Point(nFrameWidth,nFrameWidth),aMapMode));
        mpPreviewDevice->SetMapMode (aMapMode);

        
        const Rectangle aPaintRectangle (
            Point(0,0),
            mpPreviewDevice->GetOutputSizePixel());
        mpPreviewDevice->EnableMapMode(false);
        mpPreviewDevice->SetLineColor();
        svtools::ColorConfig aColorConfig;
        mpPreviewDevice->SetFillColor(aColorConfig.GetColorValue(svtools::DOCCOLOR).nColor);
        mpPreviewDevice->DrawRect (aPaintRectangle);
        mpPreviewDevice->EnableMapMode(true);

        
        PaintSubstitutionText (rSubstitutionText);
        PaintFrame();

        const Size aSize (mpPreviewDevice->GetOutputSizePixel());
        aPreview = Image(mpPreviewDevice->GetBitmap(
            mpPreviewDevice->PixelToLogic(Point(0,0)),
            mpPreviewDevice->PixelToLogic(aSize)));
    }
    catch (const com::sun::star::uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return aPreview;
}




bool PreviewRenderer::Initialize (
    const SdPage* pPage,
    const Size& rPixelSize,
    const bool bObeyHighContrastMode)
{
    if (pPage == NULL)
        return false;

    SdrModel* pModel = pPage->GetModel();
    if (pModel == NULL)
        return false;

    SetupOutputSize(*pPage, rPixelSize);

    SdDrawDocument* pDocument
        = static_cast<SdDrawDocument*>(pPage->GetModel());
    DrawDocShell* pDocShell = pDocument->GetDocSh();

    
    ProvideView (pDocShell);
    if (mpView.get() == NULL)
        return false;

    
    bool bUseContrast (bObeyHighContrastMode
        && Application::GetSettings().GetStyleSettings().GetHighContrastMode());
    mpPreviewDevice->SetDrawMode (bUseContrast
        ? ViewShell::OUTPUT_DRAWMODE_CONTRAST
        : ViewShell::OUTPUT_DRAWMODE_COLOR);
    mpPreviewDevice->SetSettings(Application::GetSettings());

    
    SdPage* pNonConstPage = const_cast<SdPage*>(pPage);
    if (pPage->IsMasterPage())
    {
        mpView->ShowSdrPage(mpView->GetModel()->GetMasterPage(pPage->GetPageNum()));
    }
    else
    {
        mpView->ShowSdrPage(pNonConstPage);
    }

    
    SdrPageView* pPageView = mpView->GetSdrPageView();

    if (pPageView == NULL)
        return false;

    
    
    
    
    
    Color aApplicationDocumentColor;

    if (pPageView->GetApplicationDocumentColor() == COL_AUTO)
    {
        svtools::ColorConfig aColorConfig;
        aApplicationDocumentColor = aColorConfig.GetColorValue( svtools::DOCCOLOR ).nColor;
    }
    else
    {
        aApplicationDocumentColor = pPageView->GetApplicationDocumentColor();
    }

    pPageView->SetApplicationDocumentColor(aApplicationDocumentColor);
    SdrOutliner& rOutliner(pDocument->GetDrawOutliner(NULL));
    rOutliner.SetBackgroundColor(aApplicationDocumentColor);
    rOutliner.SetDefaultLanguage(pDocument->GetLanguage(EE_CHAR_LANGUAGE));
    mpPreviewDevice->SetBackground(Wallpaper(aApplicationDocumentColor));
    mpPreviewDevice->Erase();

    return true;
}




void PreviewRenderer::Cleanup (void)
{
    mpView->HideSdrPage();
}




void PreviewRenderer::PaintPage (
    const SdPage* pPage,
    const bool bDisplayPresentationObjects)
{
    
    Rectangle aPaintRectangle (Point(0,0), pPage->GetSize());
    Region aRegion (aPaintRectangle);

    
    SdrOutliner* pOutliner = NULL;
    sal_uLong nSavedControlWord (0);
    if (mpDocShellOfView!=NULL && mpDocShellOfView->GetDoc()!=NULL)
    {
        pOutliner = &mpDocShellOfView->GetDoc()->GetDrawOutliner();
        nSavedControlWord = pOutliner->GetControlWord();
        pOutliner->SetControlWord((nSavedControlWord & ~EE_CNTRL_ONLINESPELLING));
    }

    
    boost::scoped_ptr<ViewRedirector> pRedirector;
    if ( ! bDisplayPresentationObjects)
        pRedirector.reset(new ViewRedirector());

    try
    {
        mpView->CompleteRedraw(mpPreviewDevice.get(), aRegion, pRedirector.get());
    }
    catch (const ::com::sun::star::uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    
    if (pOutliner != NULL)
        pOutliner->SetControlWord(nSavedControlWord);
}




void PreviewRenderer::PaintSubstitutionText (const OUString& rSubstitutionText)
{
    if (!rSubstitutionText.isEmpty())
    {
        
        const Font& rOriginalFont (mpPreviewDevice->GetFont());
        Font aFont (mpPreviewDevice->GetSettings().GetStyleSettings().GetAppFont());
        sal_Int32 nHeight (mpPreviewDevice->PixelToLogic(Size(0,snSubstitutionTextSize)).Height());
        aFont.SetHeight(nHeight);
        mpPreviewDevice->SetFont (aFont);

        
        Rectangle aTextBox (
            Point(0,0),
            mpPreviewDevice->PixelToLogic(
                mpPreviewDevice->GetOutputSizePixel()));
        sal_uInt16 nTextStyle =
            TEXT_DRAW_CENTER
            | TEXT_DRAW_VCENTER
            | TEXT_DRAW_MULTILINE
            | TEXT_DRAW_WORDBREAK;
        mpPreviewDevice->DrawText (aTextBox, rSubstitutionText, nTextStyle);

        
        mpPreviewDevice->SetFont (rOriginalFont);
    }
}




void PreviewRenderer::PaintFrame (void)
{
    if (mbHasFrame)
    {
        
        Rectangle aPaintRectangle (
            Point(0,0),
            mpPreviewDevice->GetOutputSizePixel());
        mpPreviewDevice->EnableMapMode(false);
        mpPreviewDevice->SetLineColor(maFrameColor);
        mpPreviewDevice->SetFillColor();
        mpPreviewDevice->DrawRect(aPaintRectangle);
        mpPreviewDevice->EnableMapMode(true);
     }
}




void PreviewRenderer::SetupOutputSize (
    const SdPage& rPage,
    const Size& rFramePixelSize)
{
    
    
    MapMode aMapMode (mpPreviewDevice->GetMapMode());
    aMapMode.SetMapUnit(MAP_PIXEL);

    
    const Size aPageModelSize (rPage.GetSize());
    if (aPageModelSize.Width()>0 || aPageModelSize.Height()>0)
    {
        const sal_Int32 nFrameWidth (mbHasFrame ? snFrameWidth : 0);
        aMapMode.SetScaleX(
            Fraction(rFramePixelSize.Width()-2*nFrameWidth-1, aPageModelSize.Width()));
        aMapMode.SetScaleY(
            Fraction(rFramePixelSize.Height()-2*nFrameWidth-1, aPageModelSize.Height()));
        aMapMode.SetOrigin(mpPreviewDevice->PixelToLogic(Point(nFrameWidth,nFrameWidth),aMapMode));
    }
    else
    {
        
        OSL_ASSERT(false);
        aMapMode.SetScaleX(1.0);
        aMapMode.SetScaleY(1.0);
    }
    mpPreviewDevice->SetMapMode (aMapMode);
    mpPreviewDevice->SetOutputSizePixel(rFramePixelSize);
}




void PreviewRenderer::ProvideView (DrawDocShell* pDocShell)
{
    if (pDocShell != mpDocShellOfView)
    {
        
        mpView.reset();

        
        
        if (mpDocShellOfView != NULL)
            EndListening (*mpDocShellOfView);
        mpDocShellOfView = pDocShell;
        if (mpDocShellOfView != NULL)
            StartListening (*mpDocShellOfView);
    }
    if (mpView.get() == NULL)
    {
        mpView.reset (new DrawView (pDocShell, mpPreviewDevice.get(), NULL));
    }
    mpView->SetPreviewRenderer(true);
#if 1
    mpView->SetPageVisible(false);
    mpView->SetPageBorderVisible(true);
    mpView->SetBordVisible(false);
    mpView->SetGridVisible(false);
    mpView->SetHlplVisible(false);
    mpView->SetGlueVisible(false);

#else
    
    
    
    mpView->SetPagePaintingAllowed(false);
#endif
}




Image PreviewRenderer::ScaleBitmap (
    const BitmapEx& rBitmapEx,
    int nWidth)
{
    Image aPreview;

    do
    {
        
        bool bUseContrast = Application::GetSettings().GetStyleSettings().
            GetHighContrastMode();
        mpPreviewDevice->SetDrawMode (bUseContrast
            ? ViewShell::OUTPUT_DRAWMODE_CONTRAST
            : ViewShell::OUTPUT_DRAWMODE_COLOR);

        
        Size aSize (rBitmapEx.GetSizePixel());
        if (aSize.Width() <= 0)
            break;
        Size aFrameSize (
            nWidth,
            (long)((nWidth*1.0 * aSize.Height()) / aSize.Width() + 0.5));
        Size aPreviewSize (aFrameSize.Width()-2,aFrameSize.Height()-2);
        MapMode aMapMode (mpPreviewDevice->GetMapMode());
        aMapMode.SetMapUnit(MAP_PIXEL);
        aMapMode.SetOrigin (Point());
        aMapMode.SetScaleX (1.0);
        aMapMode.SetScaleY (1.0);
        mpPreviewDevice->SetMapMode (aMapMode);
        mpPreviewDevice->SetOutputSize (aFrameSize);

        
        mpPreviewDevice->SetLineColor (maFrameColor);
        mpPreviewDevice->SetFillColor ();
        mpPreviewDevice->DrawRect (Rectangle(Point(0,0), aFrameSize));

        
        BitmapEx aScaledBitmap (rBitmapEx.GetBitmap());
        aScaledBitmap.Scale (aPreviewSize, BMP_SCALE_BESTQUALITY);
        mpPreviewDevice->DrawBitmap (
            Point(1,1),
            aPreviewSize,
            aScaledBitmap.GetBitmap());

        
        aPreview = Image(mpPreviewDevice->GetBitmap(Point(0,0), aFrameSize));
    }
    while (false);

    return aPreview;
}




void PreviewRenderer::Notify(SfxBroadcaster&, const SfxHint& rHint)
{
    if (rHint.IsA(TYPE(SfxSimpleHint))
        && mpDocShellOfView != NULL)
    {
        const SfxSimpleHint* pSimpleHint = PTR_CAST(SfxSimpleHint, &rHint);
        if (pSimpleHint != NULL
            && pSimpleHint->GetId() == SFX_HINT_DYING)
        {
            
            
            
            
            mpView.reset();
            mpDocShellOfView = NULL;
        }
    }
}






namespace {

ViewRedirector::ViewRedirector (void)
{
}




ViewRedirector::~ViewRedirector (void)
{
}




drawinglayer::primitive2d::Primitive2DSequence ViewRedirector::createRedirectedPrimitive2DSequence(
    const sdr::contact::ViewObjectContact& rOriginal,
    const sdr::contact::DisplayInfo& rDisplayInfo)
{
    SdrObject* pObject = rOriginal.GetViewContact().TryToGetSdrObject();

    if (pObject==NULL || pObject->GetPage() == NULL)
    {
        
        return sdr::contact::ViewObjectContactRedirector::createRedirectedPrimitive2DSequence(
            rOriginal,
            rDisplayInfo);
    }

    const bool bDoCreateGeometry (pObject->GetPage()->checkVisibility( rOriginal, rDisplayInfo, true));

    if ( ! bDoCreateGeometry
        && (pObject->GetObjInventor() != SdrInventor || pObject->GetObjIdentifier() != OBJ_PAGE))
    {
        return drawinglayer::primitive2d::Primitive2DSequence();
    }

    if (pObject->IsEmptyPresObj())
        return drawinglayer::primitive2d::Primitive2DSequence();

    return sdr::contact::ViewObjectContactRedirector::createRedirectedPrimitive2DSequence(
        rOriginal,
        rDisplayInfo);
}

} 


} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
