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


#include "PreviewRenderer.hxx"

#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#include "drawview.hxx"
#include "sdpage.hxx"
#include "ViewShell.hxx"
#include <vcl/virdev.hxx>
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




//===== PreviewRenderer =======================================================

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
                aPreview = mpPreviewDevice->GetBitmap (
                    mpPreviewDevice->PixelToLogic(Point(0,0)),
                    mpPreviewDevice->PixelToLogic(aSize));

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
        // Set size.
        mpPreviewDevice->SetOutputSizePixel(rPreviewPixelSize);

        // Adjust contrast mode.
        const bool bUseContrast (
            Application::GetSettings().GetStyleSettings().GetHighContrastMode());
        mpPreviewDevice->SetDrawMode (bUseContrast
            ? ViewShell::OUTPUT_DRAWMODE_CONTRAST
            : ViewShell::OUTPUT_DRAWMODE_COLOR);

        // Set a map mode that makes a typical substitution text completely
        // visible.
        MapMode aMapMode (mpPreviewDevice->GetMapMode());
        aMapMode.SetMapUnit(MAP_100TH_MM);
        const double nFinalScale (25.0 * rPreviewPixelSize.Width() / 28000.0);
        aMapMode.SetScaleX(nFinalScale);
        aMapMode.SetScaleY(nFinalScale);
        const sal_Int32 nFrameWidth (mbHasFrame ? snFrameWidth : 0);
        aMapMode.SetOrigin(mpPreviewDevice->PixelToLogic(
            Point(nFrameWidth,nFrameWidth),aMapMode));
        mpPreviewDevice->SetMapMode (aMapMode);

        // Clear the background.
        const Rectangle aPaintRectangle (
            Point(0,0),
            mpPreviewDevice->GetOutputSizePixel());
        mpPreviewDevice->EnableMapMode(sal_False);
        mpPreviewDevice->SetLineColor();
        svtools::ColorConfig aColorConfig;
        mpPreviewDevice->SetFillColor(aColorConfig.GetColorValue(svtools::DOCCOLOR).nColor);
        mpPreviewDevice->DrawRect (aPaintRectangle);
        mpPreviewDevice->EnableMapMode(sal_True);

        // Paint substitution text and a frame around it.
        PaintSubstitutionText (rSubstitutionText);
        PaintFrame();

        const Size aSize (mpPreviewDevice->GetOutputSizePixel());
        aPreview = mpPreviewDevice->GetBitmap (
            mpPreviewDevice->PixelToLogic(Point(0,0)),
            mpPreviewDevice->PixelToLogic(aSize));
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

    // Create view
    ProvideView (pDocShell);
    if (mpView.get() == NULL)
        return false;

    // Adjust contrast mode.
    bool bUseContrast (bObeyHighContrastMode
        && Application::GetSettings().GetStyleSettings().GetHighContrastMode());
    mpPreviewDevice->SetDrawMode (bUseContrast
        ? ViewShell::OUTPUT_DRAWMODE_CONTRAST
        : ViewShell::OUTPUT_DRAWMODE_COLOR);
    mpPreviewDevice->SetSettings(Application::GetSettings());

    // Tell the view to show the given page.
    SdPage* pNonConstPage = const_cast<SdPage*>(pPage);
    if (pPage->IsMasterPage())
    {
        mpView->ShowSdrPage(mpView->GetModel()->GetMasterPage(pPage->GetPageNum()));
    }
    else
    {
        mpView->ShowSdrPage(pNonConstPage);
    }

    // Make sure that a page view exists.
    SdrPageView* pPageView = mpView->GetSdrPageView();

    if (pPageView == NULL)
        return false;

    // #i121224# No need to set SetApplicationBackgroundColor (which is the color
    // of the area 'behind' the page (formally called 'Wiese') since the page previews
    // produced exactly cover the page's area, so it would never be visible. What
    // needs to be set is the ApplicationDocumentColor which is derived from
    // svtools::DOCCOLOR normally
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
    // Paint the page.
    Rectangle aPaintRectangle (Point(0,0), pPage->GetSize());
    Region aRegion (aPaintRectangle);

    // Turn off online spelling and redlining.
    SdrOutliner* pOutliner = NULL;
    sal_uLong nSavedControlWord (0);
    if (mpDocShellOfView!=NULL && mpDocShellOfView->GetDoc()!=NULL)
    {
        pOutliner = &mpDocShellOfView->GetDoc()->GetDrawOutliner();
        nSavedControlWord = pOutliner->GetControlWord();
        pOutliner->SetControlWord((nSavedControlWord & ~EE_CNTRL_ONLINESPELLING));
    }

    // Use a special redirector to prevent PresObj shapes from being painted.
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

    // Restore the previous online spelling and redlining states.
    if (pOutliner != NULL)
        pOutliner->SetControlWord(nSavedControlWord);
}




void PreviewRenderer::PaintSubstitutionText (const OUString& rSubstitutionText)
{
    if (!rSubstitutionText.isEmpty())
    {
        // Set the font size.
        const Font& rOriginalFont (mpPreviewDevice->GetFont());
        Font aFont (mpPreviewDevice->GetSettings().GetStyleSettings().GetAppFont());
        sal_Int32 nHeight (mpPreviewDevice->PixelToLogic(Size(0,snSubstitutionTextSize)).Height());
        aFont.SetHeight(nHeight);
        mpPreviewDevice->SetFont (aFont);

        // Paint the substitution text.
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

        // Restore the font.
        mpPreviewDevice->SetFont (rOriginalFont);
    }
}




void PreviewRenderer::PaintFrame (void)
{
    if (mbHasFrame)
    {
        // Paint a frame arround the preview.
        Rectangle aPaintRectangle (
            Point(0,0),
            mpPreviewDevice->GetOutputSizePixel());
        mpPreviewDevice->EnableMapMode(sal_False);
        mpPreviewDevice->SetLineColor(maFrameColor);
        mpPreviewDevice->SetFillColor();
        mpPreviewDevice->DrawRect(aPaintRectangle);
        mpPreviewDevice->EnableMapMode(sal_True);
     }
}




void PreviewRenderer::SetupOutputSize (
    const SdPage& rPage,
    const Size& rFramePixelSize)
{
    // First set the map mode to some arbitrary scale that is numerically
    // stable.
    MapMode aMapMode (mpPreviewDevice->GetMapMode());
    aMapMode.SetMapUnit(MAP_PIXEL);

    // Adapt it to the desired width.
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
        // We should never get here.
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
        // Destroy the view that is connected to the current doc shell.
        mpView.reset (NULL);

        // Switch our attention, i.e. listening for DYING events, to
        // the new doc shell.
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
    // This works in the slide sorter but prevents the master page
    // background being painted in the list of current master pages in the
    // task manager.
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
        // Adjust contrast mode.
        bool bUseContrast = Application::GetSettings().GetStyleSettings().
            GetHighContrastMode();
        mpPreviewDevice->SetDrawMode (bUseContrast
            ? ViewShell::OUTPUT_DRAWMODE_CONTRAST
            : ViewShell::OUTPUT_DRAWMODE_COLOR);

        // Set output size.
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

        // Paint a frame arround the preview.
        mpPreviewDevice->SetLineColor (maFrameColor);
        mpPreviewDevice->SetFillColor ();
        mpPreviewDevice->DrawRect (Rectangle(Point(0,0), aFrameSize));

        // Paint the bitmap scaled to the desired width.
        BitmapEx aScaledBitmap (rBitmapEx.GetBitmap());
        aScaledBitmap.Scale (aPreviewSize, BMP_SCALE_BESTQUALITY);
        mpPreviewDevice->DrawBitmap (
            Point(1,1),
            aPreviewSize,
            aScaledBitmap.GetBitmap());

        // Get the resulting bitmap.
        aPreview = mpPreviewDevice->GetBitmap (Point(0,0), aFrameSize);
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
            // The doc shell is dying.  Our view uses its item pool and
            // has to be destroyed as well.  The next call to
            // ProvideView will create a new one (for another
            // doc shell, of course.)
            mpView.reset (NULL);
            mpDocShellOfView = NULL;
        }
    }
}




//===== ViewRedirector ========================================================

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
        // not a SdrObject visualisation (maybe e.g. page) or no page
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

} // end of anonymous namespace


} // end of namespace ::sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
