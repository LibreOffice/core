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

#include <PreviewRenderer.hxx>

#include <DrawDocShell.hxx>
#include <drawdoc.hxx>
#include <drawview.hxx>
#include <sdpage.hxx>
#include <ViewShell.hxx>
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

#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd {

const int PreviewRenderer::snSubstitutionTextSize = 11;
const int PreviewRenderer::snFrameWidth = 1;

namespace {
    /** This incarnation of the ViewObjectContactRedirector filters away all
        PageObj objects, unconditionally.
    */
    class ViewRedirector : public sdr::contact::ViewObjectContactRedirector
    {
    public:
        ViewRedirector();

        virtual drawinglayer::primitive2d::Primitive2DContainer createRedirectedPrimitive2DSequence(
            const sdr::contact::ViewObjectContact& rOriginal,
            const sdr::contact::DisplayInfo& rDisplayInfo) override;
    };
}

//===== PreviewRenderer =======================================================

PreviewRenderer::PreviewRenderer (
    const bool bHasFrame)
    : mpPreviewDevice (VclPtr<VirtualDevice>::Create()),
      mpDocShellOfView(nullptr),
      maFrameColor (svtools::ColorConfig().GetColorValue(svtools::DOCBOUNDARIES).nColor),
      mbHasFrame(bHasFrame)
{
    mpPreviewDevice->SetBackground(Wallpaper(
        Application::GetSettings().GetStyleSettings().GetWindowColor()));
}

PreviewRenderer::~PreviewRenderer()
{
    if (mpDocShellOfView != nullptr)
        EndListening (*mpDocShellOfView);
}

Image PreviewRenderer::RenderPage (
    const SdPage* pPage,
    const sal_Int32 nWidth)
{
    if (pPage != nullptr)
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
            false/*bObeyHighContrastMode*/);
    }
    else
        return Image();
}

Image PreviewRenderer::RenderPage (
    const SdPage* pPage,
    Size aPixelSize,
    const bool bObeyHighContrastMode,
    const bool bDisplayPresentationObjects)
{
    Image aPreview;

    if (pPage != nullptr)
    {
        try
        {
            if (Initialize(pPage, aPixelSize, bObeyHighContrastMode))
            {
                PaintPage(pPage, bDisplayPresentationObjects);
                PaintSubstitutionText("");
                PaintFrame();

                Size aSize (mpPreviewDevice->GetOutputSizePixel());
                aPreview = Image(mpPreviewDevice->GetBitmapEx(
                    mpPreviewDevice->PixelToLogic(Point(0,0)),
                    mpPreviewDevice->PixelToLogic(aSize)));

                mpView->HideSdrPage();
            }
        }
        catch (const css::uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("sd.tools");
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
            ? sd::OUTPUT_DRAWMODE_CONTRAST
            : sd::OUTPUT_DRAWMODE_COLOR);

        // Set a map mode that makes a typical substitution text completely
        // visible.
        MapMode aMapMode (mpPreviewDevice->GetMapMode());
        aMapMode.SetMapUnit(MapUnit::Map100thMM);
        Fraction aFinalScale(25 * rPreviewPixelSize.Width(), 28000);
        aMapMode.SetScaleX(aFinalScale);
        aMapMode.SetScaleY(aFinalScale);
        const sal_Int32 nFrameWidth (mbHasFrame ? snFrameWidth : 0);
        aMapMode.SetOrigin(mpPreviewDevice->PixelToLogic(
            Point(nFrameWidth,nFrameWidth),aMapMode));
        mpPreviewDevice->SetMapMode (aMapMode);

        // Clear the background.
        const ::tools::Rectangle aPaintRectangle (
            Point(0,0),
            mpPreviewDevice->GetOutputSizePixel());
        mpPreviewDevice->EnableMapMode(false);
        mpPreviewDevice->SetLineColor();
        svtools::ColorConfig aColorConfig;
        mpPreviewDevice->SetFillColor(aColorConfig.GetColorValue(svtools::DOCCOLOR).nColor);
        mpPreviewDevice->DrawRect (aPaintRectangle);
        mpPreviewDevice->EnableMapMode();

        // Paint substitution text and a frame around it.
        PaintSubstitutionText (rSubstitutionText);
        PaintFrame();

        const Size aSize (mpPreviewDevice->GetOutputSizePixel());
        aPreview = Image(mpPreviewDevice->GetBitmapEx(
            mpPreviewDevice->PixelToLogic(Point(0,0)),
            mpPreviewDevice->PixelToLogic(aSize)));
    }
    catch (const css::uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("sd.tools");
    }

    return aPreview;
}

bool PreviewRenderer::Initialize (
    const SdPage* pPage,
    const Size& rPixelSize,
    const bool bObeyHighContrastMode)
{
    if (pPage == nullptr)
        return false;

    SetupOutputSize(*pPage, rPixelSize);
    SdDrawDocument& rDocument(static_cast< SdDrawDocument& >(pPage->getSdrModelFromSdrPage()));
    DrawDocShell* pDocShell = rDocument.GetDocSh();

    // Create view
    ProvideView (pDocShell);
    if (mpView == nullptr)
        return false;

    // Adjust contrast mode.
    bool bUseContrast (bObeyHighContrastMode
        && Application::GetSettings().GetStyleSettings().GetHighContrastMode());
    mpPreviewDevice->SetDrawMode (bUseContrast
        ? sd::OUTPUT_DRAWMODE_CONTRAST
        : sd::OUTPUT_DRAWMODE_COLOR);
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

    if (pPageView == nullptr)
        return false;

    // #i121224# No need to set SetApplicationBackgroundColor (which is the color
    // of the area 'behind' the page (formerly called 'Wiese') since the page previews
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
    SdrOutliner& rOutliner(rDocument.GetDrawOutliner());
    rOutliner.SetBackgroundColor(aApplicationDocumentColor);
    rOutliner.SetDefaultLanguage(rDocument.GetLanguage(EE_CHAR_LANGUAGE));
    mpPreviewDevice->SetBackground(Wallpaper(aApplicationDocumentColor));
    mpPreviewDevice->Erase();

    return true;
}

void PreviewRenderer::PaintPage (
    const SdPage* pPage,
    const bool bDisplayPresentationObjects)
{
    // Paint the page.
    ::tools::Rectangle aPaintRectangle (Point(0,0), pPage->GetSize());
    vcl::Region aRegion (aPaintRectangle);

    // Turn off online spelling and redlining.
    SdrOutliner* pOutliner = nullptr;
    EEControlBits nSavedControlWord = EEControlBits::NONE;
    if (mpDocShellOfView!=nullptr && mpDocShellOfView->GetDoc()!=nullptr)
    {
        pOutliner = &mpDocShellOfView->GetDoc()->GetDrawOutliner();
        nSavedControlWord = pOutliner->GetControlWord();
        pOutliner->SetControlWord(nSavedControlWord & ~EEControlBits::ONLINESPELLING);
    }

    // Use a special redirector to prevent PresObj shapes from being painted.
    std::unique_ptr<ViewRedirector> pRedirector;
    if ( ! bDisplayPresentationObjects)
        pRedirector.reset(new ViewRedirector());

    try
    {
        mpView->CompleteRedraw(mpPreviewDevice.get(), aRegion, pRedirector.get());
    }
    catch (const css::uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("sd.tools");
    }

    // Restore the previous online spelling and redlining states.
    if (pOutliner != nullptr)
        pOutliner->SetControlWord(nSavedControlWord);
}

void PreviewRenderer::PaintSubstitutionText (const OUString& rSubstitutionText)
{
    if (rSubstitutionText.isEmpty())
        return;

    // Set the font size.
    const vcl::Font& rOriginalFont (mpPreviewDevice->GetFont());
    vcl::Font aFont (mpPreviewDevice->GetSettings().GetStyleSettings().GetAppFont());
    sal_Int32 nHeight (mpPreviewDevice->PixelToLogic(Size(0,snSubstitutionTextSize)).Height());
    aFont.SetFontHeight(nHeight);
    mpPreviewDevice->SetFont (aFont);

    // Paint the substitution text.
    ::tools::Rectangle aTextBox (
        Point(0,0),
        mpPreviewDevice->PixelToLogic(
            mpPreviewDevice->GetOutputSizePixel()));
    DrawTextFlags const nTextStyle =
        DrawTextFlags::Center
        | DrawTextFlags::VCenter
        | DrawTextFlags::MultiLine
        | DrawTextFlags::WordBreak;
    mpPreviewDevice->DrawText (aTextBox, rSubstitutionText, nTextStyle);

    // Restore the font.
    mpPreviewDevice->SetFont (rOriginalFont);
}

void PreviewRenderer::PaintFrame()
{
    if (mbHasFrame)
    {
        // Paint a frame around the preview.
        ::tools::Rectangle aPaintRectangle (
            Point(0,0),
            mpPreviewDevice->GetOutputSizePixel());
        mpPreviewDevice->EnableMapMode(false);
        mpPreviewDevice->SetLineColor(maFrameColor);
        mpPreviewDevice->SetFillColor();
        mpPreviewDevice->DrawRect(aPaintRectangle);
        mpPreviewDevice->EnableMapMode();
     }
}

void PreviewRenderer::SetupOutputSize (
    const SdPage& rPage,
    const Size& rFramePixelSize)
{
    // First set the map mode to some arbitrary scale that is numerically
    // stable.
    MapMode aMapMode (mpPreviewDevice->GetMapMode());
    aMapMode.SetMapUnit(MapUnit::MapPixel);

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
        aMapMode.SetScaleX(Fraction(1.0));
        aMapMode.SetScaleY(Fraction(1.0));
    }
    mpPreviewDevice->SetMapMode (aMapMode);
    mpPreviewDevice->SetOutputSizePixel(rFramePixelSize);
}

void PreviewRenderer::ProvideView (DrawDocShell* pDocShell)
{
    if (pDocShell != mpDocShellOfView)
    {
        // Destroy the view that is connected to the current doc shell.
        mpView.reset();

        // Switch our attention, i.e. listening for DYING events, to
        // the new doc shell.
        if (mpDocShellOfView != nullptr)
            EndListening (*mpDocShellOfView);
        mpDocShellOfView = pDocShell;
        if (mpDocShellOfView != nullptr)
            StartListening (*mpDocShellOfView);
    }
    if (mpView == nullptr)
    {
        mpView.reset (new DrawView (pDocShell, mpPreviewDevice.get(), nullptr));
    }
    mpView->SetPreviewRenderer(true);
#if 1
    mpView->SetPageVisible(false);
    mpView->SetPageBorderVisible();
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
            ? sd::OUTPUT_DRAWMODE_CONTRAST
            : sd::OUTPUT_DRAWMODE_COLOR);

        // Set output size.
        Size aSize (rBitmapEx.GetSizePixel());
        if (aSize.Width() <= 0)
            break;
        Size aFrameSize (
            nWidth,
            static_cast<long>((nWidth*1.0 * aSize.Height()) / aSize.Width() + 0.5));
        Size aPreviewSize (aFrameSize.Width()-2,aFrameSize.Height()-2);
        MapMode aMapMode (mpPreviewDevice->GetMapMode());
        aMapMode.SetMapUnit(MapUnit::MapPixel);
        aMapMode.SetOrigin (Point());
        aMapMode.SetScaleX (Fraction(1.0));
        aMapMode.SetScaleY (Fraction(1.0));
        mpPreviewDevice->SetMapMode (aMapMode);
        mpPreviewDevice->SetOutputSize (aFrameSize);

        // Paint a frame around the preview.
        mpPreviewDevice->SetLineColor (maFrameColor);
        mpPreviewDevice->SetFillColor ();
        mpPreviewDevice->DrawRect (::tools::Rectangle(Point(0,0), aFrameSize));

        // Paint the bitmap scaled to the desired width.
        BitmapEx aScaledBitmap (rBitmapEx.GetBitmap());
        aScaledBitmap.Scale (aPreviewSize, BmpScaleFlag::BestQuality);
        mpPreviewDevice->DrawBitmap (
            Point(1,1),
            aPreviewSize,
            aScaledBitmap.GetBitmap());

        // Get the resulting bitmap.
        aPreview = Image(mpPreviewDevice->GetBitmapEx(Point(0,0), aFrameSize));
    }
    while (false);

    return aPreview;
}

void PreviewRenderer::Notify(SfxBroadcaster&, const SfxHint& rHint)
{
    if (!mpDocShellOfView)
        return;

    if (rHint.GetId() == SfxHintId::Dying)
    {
        // The doc shell is dying.  Our view uses its item pool and
        // has to be destroyed as well.  The next call to
        // ProvideView will create a new one (for another
        // doc shell, of course.)
        mpView.reset();
        mpDocShellOfView = nullptr;
    }
}

//===== ViewRedirector ========================================================

namespace {

ViewRedirector::ViewRedirector()
{
}

drawinglayer::primitive2d::Primitive2DContainer ViewRedirector::createRedirectedPrimitive2DSequence(
    const sdr::contact::ViewObjectContact& rOriginal,
    const sdr::contact::DisplayInfo& rDisplayInfo)
{
    SdrObject* pObject = rOriginal.GetViewContact().TryToGetSdrObject();

    if (pObject==nullptr || pObject->getSdrPageFromSdrObject() == nullptr)
    {
        // not a SdrObject visualisation (maybe e.g. page) or no page
        return sdr::contact::ViewObjectContactRedirector::createRedirectedPrimitive2DSequence(
            rOriginal,
            rDisplayInfo);
    }

    const bool bDoCreateGeometry (pObject->getSdrPageFromSdrObject()->checkVisibility( rOriginal, rDisplayInfo, true));

    if ( ! bDoCreateGeometry
        && (pObject->GetObjInventor() != SdrInventor::Default || pObject->GetObjIdentifier() != OBJ_PAGE))
    {
        return drawinglayer::primitive2d::Primitive2DContainer();
    }

    if (pObject->IsEmptyPresObj())
        return drawinglayer::primitive2d::Primitive2DContainer();

    return sdr::contact::ViewObjectContactRedirector::createRedirectedPrimitive2DSequence(
        rOriginal,
        rDisplayInfo);
}

} // end of anonymous namespace

} // end of namespace ::sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
