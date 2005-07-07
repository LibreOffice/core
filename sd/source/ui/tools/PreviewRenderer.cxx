/*************************************************************************
 *
 *  $RCSfile: PreviewRenderer.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-07 13:38:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "PreviewRenderer.hxx"

#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#include "drawview.hxx"
#include "sdpage.hxx"
#include "ViewShell.hxx"
#include <vcl/virdev.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdoutl.hxx>
#include <svx/eeitem.hxx>
#include <tools/link.hxx>


namespace sd {

const int PreviewRenderer::snSubstitutioinTextSize = 5000;
const int PreviewRenderer::snFrameWidth = 1;

PreviewRenderer::PreviewRenderer (OutputDevice* pTemplate)
    : mpPreviewDevice (new VirtualDevice()),
      mpView(NULL),
      mpDocShellOfView(NULL),
      mnWidthOfView(0),
      maFrameColor (svtools::ColorConfig().GetColorValue(
          svtools::DOCBOUNDARIES).nColor)
{
    // Create a virtual device when not already present and
    // initialize it.
    mpPreviewDevice->SetDrawMode (DRAWMODE_DEFAULT);

    if (pTemplate != NULL)
    {
        mpPreviewDevice->SetDigitLanguage (pTemplate->GetDigitLanguage());
        mpPreviewDevice->SetBackground(pTemplate->GetBackground());
    }
    else
        mpPreviewDevice->SetBackground(Wallpaper(COL_WHITE));
}




PreviewRenderer::~PreviewRenderer (void)
{
    if (mpDocShellOfView != NULL)
        EndListening (*mpDocShellOfView);
}




Image PreviewRenderer::RenderPage (
    const SdPage* pPage,
    int nWidth,
    const String& rSubstitutionText)
{
    Size aPageModelSize (pPage->GetSize());
    double nAspectRatio (
        double(aPageModelSize.Width()) / double(aPageModelSize.Height()));
    int nHeight = (int)((nWidth - 2*snFrameWidth) / nAspectRatio
        + 2*snFrameWidth + 0.5);
    return RenderPage (pPage, Size(nWidth,nHeight), rSubstitutionText);
}




Image PreviewRenderer::RenderPage (
    const SdPage* pPage,
    Size aPixelSize,
    const String& rSubstitutionText)
{
    Image aPreview;

    try
    {
        if (Initialize (pPage, aPixelSize))
        {
            PaintPage (pPage);
            PaintSubstitutionText (rSubstitutionText);
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
        OSL_TRACE("%s: caught exception", __FUNCTION__);
    }

    return aPreview;
}




bool PreviewRenderer::Initialize (
    const SdPage* pPage,
    const Size& rPixelSize)
{
    bool bSuccess = false;
    do
    {
        if (pPage == NULL)
            break;

        SdrModel* pModel = pPage->GetModel();
        if (pModel == NULL)
            break;

        SetupOutputSize (pPage, rPixelSize);

        SdDrawDocument* pDocument
            = static_cast<SdDrawDocument*>(pPage->GetModel());
        DrawDocShell* pDocShell = pDocument->GetDocSh();

        // Create view
        ProvideView (pDocShell);
        if (mpView.get() == NULL)
            break;

        // Adjust contrast mode.
        bool bUseContrast = Application::GetSettings().GetStyleSettings().
            GetHighContrastMode();
        mpPreviewDevice->SetDrawMode (bUseContrast
            ? ViewShell::OUTPUT_DRAWMODE_CONTRAST
            : ViewShell::OUTPUT_DRAWMODE_COLOR);

        // Tell the view to show the given page.
        SdPage* pNonConstPage = const_cast<SdPage*>(pPage);
        if (pPage->IsMasterPage())
            mpView->ShowMasterPagePgNum(pPage->GetPageNum(), Point(0, 0));
        else
            mpView->ShowPage (pNonConstPage, Point(0,0));

        // Make sure that a page view exists.
        SdrPageView* pPageView = mpView->GetPageView (pPage);
        if (pPageView == NULL)
            break;
        // Set background color of page view and outliner.
        svtools::ColorConfig aColorConfig;
        pPageView->SetApplicationBackgroundColor(
            pPage->GetBackgroundColor(pPageView));
        SdrOutliner& rOutliner (pDocument->GetDrawOutliner(NULL));
        rOutliner.SetBackgroundColor(pPage->GetBackgroundColor(pPageView));
        rOutliner.SetDefaultLanguage(pDocument->GetLanguage(EE_CHAR_LANGUAGE));
        mpView->SetApplicationBackgroundColor(
            Color(aColorConfig.GetColorValue(svtools::APPBACKGROUND).nColor));

        bSuccess = true;
    }
    while (false);

    return bSuccess;
}




void PreviewRenderer::Cleanup (void)
{
    mpView->HideAllPages();
}



void PreviewRenderer::PaintPage (const SdPage* pPage)
{
    // Paint the page.
    Rectangle aPaintRectangle (Point(0,0), pPage->GetSize());
    Region aRegion (aPaintRectangle);

    try
    {
        mpView->CompleteRedraw (mpPreviewDevice.get(), aRegion);
    }
    catch (const ::com::sun::star::uno::Exception&)
    {
        OSL_TRACE("%s: caught exception", __FUNCTION__);
    }
}




void PreviewRenderer::PaintSubstitutionText (const String& rSubstitutionText)
{
    if (rSubstitutionText.Len() > 0)
    {
        // Set the font size.
        const Font& rOriginalFont (mpPreviewDevice->GetFont());
        Font aFont (rOriginalFont);
        aFont.SetHeight (snSubstitutioinTextSize);
        mpPreviewDevice->SetFont (aFont);

        // Paint the substitution text.
        Rectangle aTextBox (
            Point(0,0),
            mpPreviewDevice->PixelToLogic(
                mpPreviewDevice->GetOutputSizePixel()));
        USHORT nTextStyle =
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
    // Paint a frame arround the preview.
    Rectangle aPaintRectangle (
        Point(0,0),
        mpPreviewDevice->GetOutputSizePixel());
    mpPreviewDevice->EnableMapMode (FALSE);
    mpPreviewDevice->SetLineColor (maFrameColor);
    mpPreviewDevice->SetFillColor ();
    mpPreviewDevice->DrawRect (aPaintRectangle);
    mpPreviewDevice->EnableMapMode (TRUE);
}




void PreviewRenderer::SetupOutputSize (
    const SdPage* pPage,
    const Size& rFramePixelSize)
{
    // First set the map mode to some arbitrary scale that is numerically
    // stable.
    MapMode aMapMode (mpPreviewDevice->GetMapMode());
    aMapMode.SetMapUnit(MAP_100TH_MM);
    double nInitialScale = 1;
    aMapMode.SetScaleX (Fraction(nInitialScale));
    aMapMode.SetScaleY (Fraction(nInitialScale));
    aMapMode.SetOrigin (Point(0,0));

    // Adapt it to the desired width.
    Size aPageModelSize (pPage->GetSize());
    /*    double nAspectRatio (
        double(aPageModelSize.Width()) / double(aPageModelSize.Height()));
    Size aFramePixelSize (nWidth, (int)((nWidth-2) / nAspectRatio + 2 + 0.5));
    */
    Size aOutputSize = mpPreviewDevice->LogicToPixel(
        pPage->GetSize(), aMapMode);
    double nFinalScale (nInitialScale * (rFramePixelSize.Width()-snFrameWidth)
        / aOutputSize.Width());
    aMapMode.SetScaleX (nFinalScale);
    aMapMode.SetScaleY (nFinalScale);
    aMapMode.SetOrigin (mpPreviewDevice->PixelToLogic(
        Point(snFrameWidth,snFrameWidth),aMapMode));

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
    mpView->SetBordVisible(FALSE);
    mpView->SetPageBorderVisible(FALSE);
    mpView->SetPageVisible(TRUE);
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
        aScaledBitmap.Scale (aPreviewSize, BMP_SCALE_INTERPOLATE);
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




void PreviewRenderer::SFX_NOTIFY(SfxBroadcaster& rBC,
    const TypeId& rBCType,
    const SfxHint& rHint,
    const TypeId& rHintType)
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



} // end of namespace ::sd
