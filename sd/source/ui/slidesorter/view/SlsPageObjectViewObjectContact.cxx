/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsPageObjectViewObjectContact.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:44:53 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "precompiled_sd.hxx"

#include "view/SlsPageObjectViewObjectContact.hxx"

#include "controller/SlsProperties.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsPageObjectViewContact.hxx"
#include "view/SlsPageObject.hxx"
#include "view/SlsFontProvider.hxx"
#include "view/SlsPageNotificationObjectContact.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "cache/SlsPageCache.hxx"
#include "cache/SlsPageCacheManager.hxx"
#include "res_bmp.hrc"
#include "tools/IconCache.hxx"
#include "PreviewRenderer.hxx"

#include "sdpage.hxx"
#include "sdresid.hxx"
#include "glob.hrc"
#include "drawdoc.hxx"
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/svdopage.hxx>
#include <svx/xoutx.hxx>
#include <svx/svdpagv.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/svdoutl.hxx>
#include <svx/sdrpagewindow.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/lineinfo.hxx>
#include <tools/color.hxx>
#include <boost/shared_ptr.hpp>
#include <com/sun/star/uno/Exception.hpp>

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

using namespace ::sdr::contact;
using namespace ::sd::slidesorter::model;


namespace sd { namespace slidesorter { namespace view {


const sal_Int32 PageObjectViewObjectContact::mnSelectionIndicatorOffset = 2;
const sal_Int32 PageObjectViewObjectContact::mnSelectionIndicatorThickness = 3;
const sal_Int32 PageObjectViewObjectContact::mnFocusIndicatorOffset = 3;
const sal_Int32 PageObjectViewObjectContact::mnFadeEffectIndicatorOffset = 9;
const sal_Int32 PageObjectViewObjectContact::mnFadeEffectIndicatorSize = 14;
const sal_Int32 PageObjectViewObjectContact::mnPageNumberOffset = 9;
const sal_Int32 PageObjectViewObjectContact::mnMouseOverEffectOffset = 3;
const sal_Int32 PageObjectViewObjectContact::mnMouseOverEffectThickness = 1;

PageObjectViewObjectContact::PageObjectViewObjectContact (
    ObjectContact& rObjectContact,
    ViewContact& rViewContact,
    const ::boost::shared_ptr<cache::PageCache>& rpCache,
    const ::boost::shared_ptr<controller::Properties>& rpProperties)
    : ViewObjectContact (rObjectContact, rViewContact),
      mpPageDescriptor(GetPageDescriptor()),
      mbIsValid(true),
      mbInPrepareDelete(false),
      mbIsBackgroundColorUpdatePending(true),
      mpCache(rpCache),
      mpNotifier(NULL),
      mpProperties(rpProperties),
      maBackgroundColor()
{
    SharedPageDescriptor pDescriptor (GetPageDescriptor());
    OSL_ASSERT(pDescriptor.get()!=NULL);
    if (pDescriptor.get() != NULL)
        pDescriptor->SetViewObjectContact(this);
}




PageObjectViewObjectContact::~PageObjectViewObjectContact (void)
{
    if (mpCache.get() != NULL)
    {
        mpCache->ReleasePreviewBitmap(GetPage());
    }

    if (mpNotifier.get() != NULL)
    {
        mbInPrepareDelete = true;
        mpNotifier->PrepareDelete();
        mpNotifier.reset();
    }
}




void PageObjectViewObjectContact::SetCache (const ::boost::shared_ptr<cache::PageCache>& rpCache)
{
    mpCache = rpCache;
}




Rectangle PageObjectViewObjectContact::GetBoundingBox (
    OutputDevice& rDevice,
    BoundingBoxType eType,
    CoordinateSystem eCoordinateSystem) const
{
    // Most of the bounding boxes are based on the bounding box of the preview.
    Rectangle aBoundingBox (static_cast<PageObjectViewContact&>(GetViewContact()
            ).GetPageObject().GetCurrentBoundRect());

    CoordinateSystem eCurrentCoordinateSystem (ModelCoordinateSystem);
    switch(eType)
    {
        case PageObjectBoundingBox:
            aBoundingBox = GetViewContact().GetPaintRectangle();
            break;
        case PreviewBoundingBox:
            // The aBoundingBox already has the right value.
            break;
        case MouseOverIndicatorBoundingBox:
        {
            const sal_Int32 nBorderWidth (mnMouseOverEffectOffset+mnMouseOverEffectThickness);
            const Size aBorderSize (rDevice.PixelToLogic(Size(nBorderWidth,nBorderWidth)));
            aBoundingBox.Left() -= aBorderSize.Width();
            aBoundingBox.Top() -= aBorderSize.Height();
            aBoundingBox.Right() += aBorderSize.Width();
            aBoundingBox.Bottom() += aBorderSize.Height();
            break;
        }
        case FocusIndicatorBoundingBox:
        {
            const sal_Int32 nBorderWidth (mnFocusIndicatorOffset+1);
            const Size aBorderSize (rDevice.PixelToLogic(Size(nBorderWidth,nBorderWidth)));
            aBoundingBox.Left() -= aBorderSize.Width();
            aBoundingBox.Top() -= aBorderSize.Height();
            aBoundingBox.Right() += aBorderSize.Width();
            aBoundingBox.Bottom() += aBorderSize.Height();
            break;
        }
        case SelectionIndicatorBoundingBox:
        {
            const sal_Int32 nBorderWidth(mnSelectionIndicatorOffset+mnSelectionIndicatorThickness);
            const Size aBorderSize (rDevice.PixelToLogic(Size(nBorderWidth,nBorderWidth)));
            aBoundingBox.Left() -= aBorderSize.Width();
            aBoundingBox.Top() -= aBorderSize.Height();
            aBoundingBox.Right() += aBorderSize.Width();
            aBoundingBox.Bottom() += aBorderSize.Height();
            break;
        }
        case PageNumberBoundingBox:
        {
            Size aModelOffset = rDevice.PixelToLogic(Size(mnPageNumberOffset,mnPageNumberOffset));
            Size aNumberSize (GetPageDescriptor()->GetPageNumberAreaModelSize());
            aBoundingBox = Rectangle (
                Point (
                    aBoundingBox.Left() - aModelOffset.Width() - aNumberSize.Width(),
                    aBoundingBox.Top()),
                aNumberSize);
            break;
        }

        case NameBoundingBox:
            break;

        case FadeEffectIndicatorBoundingBox:
            Size aModelOffset = rDevice.PixelToLogic(Size (0, mnFadeEffectIndicatorOffset));
            // Flush left just outside the selection rectangle.
            aBoundingBox = Rectangle (
                Point (
                    aBoundingBox.Left(),
                    aBoundingBox.Bottom() + aModelOffset.Height()
                    ),
                rDevice.PixelToLogic (
                    IconCache::Instance().GetIcon(BMP_FADE_EFFECT_INDICATOR).GetSizePixel())
                );
            break;
    }

    // Make sure the bounding box uses the requested coordinate system.
    if (eCurrentCoordinateSystem != eCoordinateSystem)
    {
        if (eCoordinateSystem == ModelCoordinateSystem)
            aBoundingBox = Rectangle(
                rDevice.PixelToLogic(aBoundingBox.TopLeft()),
                rDevice.PixelToLogic(aBoundingBox.GetSize()));
        else
            aBoundingBox = Rectangle(
                rDevice.LogicToPixel(aBoundingBox.TopLeft()),
                rDevice.LogicToPixel(aBoundingBox.GetSize()));
    }
    return aBoundingBox;
}




BitmapEx PageObjectViewObjectContact::CreatePreview (OutputDevice& rDevice) const
{
    const SdPage* pPage = static_cast<const SdPage*>(GetPage());
    Rectangle aPreviewPixelBox (GetBoundingBox(rDevice,PreviewBoundingBox,PixelCoordinateSystem));

    PreviewRenderer aRenderer(&rDevice);
    Image aPreview (aRenderer.RenderPage(
        pPage,
        aPreviewPixelBox.GetSize(),
        String()));

    return aPreview.GetBitmapEx();
}




BitmapEx PageObjectViewObjectContact::GetPreview (
    OutputDevice& rDevice,
    const Rectangle& rNewSizePixel)
{
    BitmapEx aBitmap;

    try
    {
        if (mbIsValid)
        {
            if (mpCache != NULL)
            {
                aBitmap = mpCache->GetPreviewBitmap(
                    GetPage(),
                    rNewSizePixel.GetSize());
                mpCache->SetPreciousFlag(GetPage(), true);
            }
            else
                aBitmap = CreatePreview(rDevice);
        }
    }
    catch (const ::com::sun::star::uno::Exception&)
    {
        OSL_TRACE("PageObjectViewObjectContact::GetPreview: caught exception");
    }

    return aBitmap;
}




void PageObjectViewObjectContact::PaintObject (DisplayInfo& rDisplayInfo)
{
    try
    {
        // Make sure that our notification object contact exists...
        if (mpNotifier.get() == NULL)
        {
            SdrPage* pPage = const_cast<SdrPage*>(GetPage());
            mpNotifier.reset(new PageNotificationObjectContact(*pPage, *this));
        }
        // ...and that it has a valid contact object hierarchy.
        mpNotifier->EnsureValidDrawHierarchy(rDisplayInfo);

        OutputDevice* pDevice = rDisplayInfo.GetOutputDevice();
        // Check if buffering can and shall be done.
        if (pDevice != NULL
            && !rDisplayInfo.OutputToPrinter()
            && !rDisplayInfo.OutputToRecordingMetaFile()
            && rDisplayInfo.IsBufferingAllowed()
            && mbIsValid)
        {
            // Transform the page borders from pixel coordinates to model
            // coordinates and tell the view contact object about them.  It
            // would be nice if we had to do this only once (better yet not at
            // all).
            GetViewContact().InvalidatePaintRectangle();

            // Set state flags. This overrides results from a contained
            // call to PaintObject(..).
            mbIsPainted = sal_True;

            PaintContent(*pDevice);

            // set painted rectangle
            maPaintedRectangle = GetViewContact().GetPaintRectangle();
        }
        else
        {
            // paint normal, bitmap not available. Call parent.
            ViewObjectContact::PaintObject(rDisplayInfo);
        }
    }
    catch (const com::sun::star::uno::Exception&)
    {
        OSL_TRACE("PageObjectViewObjectContact::PaintObject: caught exception");
        // Even though the object was not painted completely we set the
        // state flags as if that has happened.  We do this in order to
        // avoid frequent repaints because when painting failed this time it
        // may fail the next time, too.
        mbIsPainted = sal_True;
    }
}




void PageObjectViewObjectContact::PrepareDelete (void)
{
    mbIsValid = false;
    mbInPrepareDelete = true;

    GetPageDescriptor()->SetViewObjectContact(NULL);

    if (mpCache != NULL)
        mpCache->ReleasePreviewBitmap(GetPage());

    ViewObjectContact::PrepareDelete();

    mbInPrepareDelete = false;
}




const SdrPage* PageObjectViewObjectContact::GetPage (void) const
{
    return static_cast<PageObjectViewContact&>(GetViewContact()).GetPage();
}




void PageObjectViewObjectContact::ActionChanged (void)
{
    // Even when we are called from PrepareDelete we still have to invalide
    // the preview bitmap in the cache.
    const SdrPage* pPage = GetPage();
    SdDrawDocument* pDocument = dynamic_cast<SdDrawDocument*>(pPage->GetModel());
    if (mpCache!=NULL && pPage!=NULL && pDocument!=NULL)
    {
        cache::PageCacheManager::Instance()->InvalidatePreviewBitmap(
            pDocument->getUnoModel(),
            GetPage());
    }

    ViewObjectContact::ActionChanged();

    mbIsBackgroundColorUpdatePending = true;
}




void PageObjectViewObjectContact::PaintContent  (OutputDevice& rDevice)
{
    // Save (a part of) the state of the output device.
    const ULONG nPreviousDrawMode (rDevice.GetDrawMode());
    const Color aOriginalFillColor (rDevice.GetFillColor());
    const Color aOriginalLineColor (rDevice.GetLineColor());
    const Font aOriginalFont (rDevice.GetFont());

    // Set default font.
    rDevice.SetFont(*FontProvider::Instance().GetFont(rDevice));

    // Do the actual painting.
    PaintBackground(rDevice);
    PaintPreview(rDevice);
    PaintFrame(rDevice);
    PaintFadeEffectIndicator(rDevice);
    PaintPageName(rDevice);
    PaintPageNumber(rDevice);

    // Restore old device state.
    rDevice.SetFont(aOriginalFont);
    rDevice.SetLineColor(aOriginalLineColor);
    rDevice.SetFillColor(aOriginalFillColor);
    rDevice.SetDrawMode(nPreviousDrawMode);
}




void PageObjectViewObjectContact::PaintBackground  (OutputDevice& rDevice) const
{
    if (mpProperties.get()!=NULL
        && mpProperties->IsHighlightCurrentSlide()
        && GetPageDescriptor()->IsCurrentPage())
    {
        Rectangle aOuterBox (GetBoundingBox(rDevice,PageObjectBoundingBox,PixelCoordinateSystem));
        Rectangle aInnerBox (GetBoundingBox(rDevice,PreviewBoundingBox,PixelCoordinateSystem));

        const Color aOldFillColor (rDevice.GetFillColor());
        const Color aOldLineColor (rDevice.GetLineColor());
        const sal_Bool bWasEnabled(rDevice.IsMapModeEnabled());

        rDevice.SetLineColor();
        rDevice.SetFillColor(GetColor(rDevice, CS_BACKGROUND));
        rDevice.EnableMapMode(sal_False);

        // Paint the background without painting over the preview.
        rDevice.DrawRect(
            Rectangle(aOuterBox.Left(),aOuterBox.Top(),aInnerBox.Left(),aOuterBox.Bottom()));
        rDevice.DrawRect(
            Rectangle(aInnerBox.Left(),aOuterBox.Top(),aInnerBox.Right(),aInnerBox.Top()));
        rDevice.DrawRect(
            Rectangle(aInnerBox.Right(),aOuterBox.Top(),aOuterBox.Right(),aInnerBox.Bottom()));
        rDevice.DrawRect(
            Rectangle(aInnerBox.Left(),aInnerBox.Bottom(),aOuterBox.Right(),aOuterBox.Bottom()));

        // Draw the frame around the background.
        rDevice.SetLineColor(GetColor(rDevice, CS_SELECTION));
        rDevice.SetFillColor();
        rDevice.DrawRect(aOuterBox);

        // Erase the corner pixel to have somewhat rounded corners.
        const Color aCornerColor (GetColor(rDevice, CS_WINDOW));
        Point aCorner (aOuterBox.TopLeft());
        rDevice.DrawPixel (aCorner, aCornerColor);
        aCorner = aOuterBox.TopRight();
        rDevice.DrawPixel (aCorner, aCornerColor);
        aCorner = aOuterBox.BottomLeft();
        rDevice.DrawPixel (aCorner, aCornerColor);
        aCorner = aOuterBox.BottomRight();
        rDevice.DrawPixel (aCorner, aCornerColor);

        rDevice.SetFillColor(aOldFillColor);
        rDevice.SetLineColor(aOldLineColor);
        rDevice.EnableMapMode(bWasEnabled);
    }
}




void PageObjectViewObjectContact::PaintPreview  (OutputDevice& rDevice)
{
    Rectangle aNewSizePixel (GetBoundingBox(rDevice,PreviewBoundingBox,PixelCoordinateSystem));
    BitmapEx aPreview (GetPreview(rDevice, aNewSizePixel));

    // Paint using cached bitmap.
    const sal_Bool bWasEnabled(rDevice.IsMapModeEnabled());
    rDevice.EnableMapMode(sal_False);
    rDevice.DrawBitmapEx(aNewSizePixel.TopLeft(), aPreview);
    rDevice.EnableMapMode(bWasEnabled);
}




void PageObjectViewObjectContact::PaintFrame (
    OutputDevice& rDevice,
    bool bShowMouseOverEffect) const
{
    PaintBorder (rDevice);
    PaintSelectionIndicator (rDevice);
    PaintMouseOverEffect (rDevice, bShowMouseOverEffect);
    // else the mouse over effect is not visible when the selection
    // indicator is painted already.
    PaintFocusIndicator (rDevice,
        GetPageDescriptor()->IsSelected() || ! bShowMouseOverEffect);
}




void PageObjectViewObjectContact::PaintBorder (
    OutputDevice& rDevice) const
{
    Rectangle aFrameBox (GetBoundingBox(rDevice, PreviewBoundingBox, PixelCoordinateSystem));
    aFrameBox.Left() -= 1;
    aFrameBox.Top() -= 1;
    aFrameBox.Right() += 1;
    aFrameBox.Bottom() += 1;
    rDevice.EnableMapMode(FALSE);
    rDevice.SetFillColor();
    svtools::ColorConfig aColorConfig;
    Color aColor = aColorConfig.GetColorValue(svtools::FONTCOLOR).nColor;
    rDevice.SetLineColor(aColor);
    rDevice.DrawRect(aFrameBox);
    rDevice.EnableMapMode(TRUE);
}




void PageObjectViewObjectContact::PaintSelectionIndicator (
    OutputDevice& rDevice) const
{
    if ( ! GetPageDescriptor()->IsSelected())
        return;

    if (mpProperties.get()!=NULL && ! mpProperties->IsShowSelection())
        return;

    const Color aOldFillColor (rDevice.GetFillColor());
    const Color aOldLineColor (rDevice.GetLineColor());

    // Determine colors for the frame and the background and mix them to
    // obtain a third color that is used for an antialiasing effect.
    Color aFrameColor (GetColor(rDevice, CS_SELECTION));
    Color aBackgroundColor (GetColor(rDevice, CS_BACKGROUND));
    Color aCornerColor (aFrameColor);
    aCornerColor.Merge (aBackgroundColor, 128);

    // Set default draw mode to be able to correctly draw the selected
    // (and only that) frame.
    ULONG nPreviousDrawMode = rDevice.GetDrawMode();
    rDevice.SetDrawMode (DRAWMODE_DEFAULT);

    Rectangle aInner (GetBoundingBox(rDevice,PreviewBoundingBox,PixelCoordinateSystem));
    rDevice.EnableMapMode (FALSE);

    rDevice.SetFillColor();
    rDevice.SetLineColor(aFrameColor);

    // Paint the frame.
    for (int nOffset=mnSelectionIndicatorOffset;
         nOffset<mnSelectionIndicatorOffset+mnSelectionIndicatorThickness;
         nOffset++)
    {
        Rectangle aFrame (aInner);
        aFrame.Left() -= nOffset;
        aFrame.Top() -= nOffset;
        aFrame.Right() += nOffset;
        aFrame.Bottom() += nOffset;
        rDevice.DrawRect (aFrame);
    }

    // Paint the four corner pixels in backround color for a rounded
    // effect.
    int nFrameWidth (mnSelectionIndicatorOffset
        + mnSelectionIndicatorThickness - 1);
    Rectangle aOuter (aInner);
    aOuter.Left() -= nFrameWidth;
    aOuter.Top() -= nFrameWidth;
    aOuter.Right() += nFrameWidth;
    aOuter.Bottom() += nFrameWidth;
    Point aCorner (aOuter.TopLeft());
    rDevice.DrawPixel (aCorner, aBackgroundColor);
    rDevice.DrawPixel (Point(aCorner.X()+1,aCorner.Y()),aBackgroundColor);
    rDevice.DrawPixel (Point(aCorner.X(),aCorner.Y()+1),aBackgroundColor);
    rDevice.DrawPixel (Point(aCorner.X()+2,aCorner.Y()),aCornerColor);
    rDevice.DrawPixel (Point(aCorner.X(),aCorner.Y()+2),aCornerColor);
    aCorner = aOuter.TopRight();
    rDevice.DrawPixel (aCorner, aBackgroundColor);
    rDevice.DrawPixel (Point(aCorner.X()-1,aCorner.Y()),aBackgroundColor);
    rDevice.DrawPixel (Point(aCorner.X(),aCorner.Y()+1),aBackgroundColor);
    rDevice.DrawPixel (Point(aCorner.X()-2,aCorner.Y()), aCornerColor);
    rDevice.DrawPixel (Point(aCorner.X(),aCorner.Y()+2), aCornerColor);
    aCorner = aOuter.BottomLeft();
    rDevice.DrawPixel (aCorner, aBackgroundColor);
    rDevice.DrawPixel (Point(aCorner.X()+1,aCorner.Y()),aBackgroundColor);
    rDevice.DrawPixel (Point(aCorner.X(),aCorner.Y()-1),aBackgroundColor);
    rDevice.DrawPixel (Point(aCorner.X()+2,aCorner.Y()), aCornerColor);
    rDevice.DrawPixel (Point(aCorner.X(),aCorner.Y()-2), aCornerColor);
    aCorner = aOuter.BottomRight();
    rDevice.DrawPixel (aCorner, aBackgroundColor);
    rDevice.DrawPixel (Point(aCorner.X()-1,aCorner.Y()),aBackgroundColor);
    rDevice.DrawPixel (Point(aCorner.X(),aCorner.Y()-1),aBackgroundColor);
    rDevice.DrawPixel (Point(aCorner.X()-2,aCorner.Y()), aCornerColor);
    rDevice.DrawPixel (Point(aCorner.X(),aCorner.Y()-2), aCornerColor);

    rDevice.EnableMapMode (TRUE);

    // Restore old values.
    rDevice.SetLineColor (aOldLineColor);
    rDevice.SetFillColor (aOldFillColor);
    rDevice.SetDrawMode (nPreviousDrawMode);
}




void PageObjectViewObjectContact::PaintMouseOverEffect (
    OutputDevice& rDevice,
    bool bVisible) const
{
    // When the selection frame is painted the mouse over frame is not
    // visible and does not have to be painted.
    if (GetPageDescriptor()->IsSelected())
        if (mpProperties.get()!=NULL && mpProperties->IsShowSelection())
            return;

    ULONG nPreviousDrawMode = rDevice.GetDrawMode();
    rDevice.SetDrawMode (DRAWMODE_DEFAULT);
    Rectangle aInner (GetBoundingBox(rDevice,PreviewBoundingBox,PixelCoordinateSystem));
    rDevice.EnableMapMode (FALSE);

    Color aSelectionColor (GetColor(rDevice, CS_SELECTION));
    Color aBackgroundColor (GetColor(rDevice, CS_BACKGROUND));
    Color aFrameColor (bVisible ? aSelectionColor : aBackgroundColor);
    Color aCornerColor (aBackgroundColor);

    rDevice.SetFillColor ();
    rDevice.SetLineColor (aFrameColor);

    // Paint the frame.
    for (int nOffset=mnMouseOverEffectOffset;
         nOffset<mnMouseOverEffectOffset+mnMouseOverEffectThickness;
         nOffset++)
    {
        Rectangle aFrame (aInner);
        aFrame.Left() -= nOffset;
        aFrame.Top() -= nOffset;
        aFrame.Right() += nOffset;
        aFrame.Bottom() += nOffset;
        rDevice.DrawRect (rDevice.PixelToLogic(aFrame));
    }

    // Paint the four corner pixels in backround color for a rounded effect.
    int nFrameWidth (mnMouseOverEffectOffset
        + mnMouseOverEffectThickness - 1);
    Rectangle aOuter (aInner);
    aOuter.Left() -= nFrameWidth;
    aOuter.Top() -= nFrameWidth;
    aOuter.Right() += nFrameWidth;
    aOuter.Bottom() += nFrameWidth;
    Point aCorner (aOuter.TopLeft());

    rDevice.DrawPixel (aCorner, aCornerColor);
    aCorner = aOuter.TopRight();
    rDevice.DrawPixel (aCorner, aCornerColor);
    aCorner = aOuter.BottomLeft();
    rDevice.DrawPixel (aCorner, aCornerColor);
    aCorner = aOuter.BottomRight();
    rDevice.DrawPixel (aCorner, aCornerColor);

    rDevice.EnableMapMode (TRUE);
    rDevice.SetDrawMode(nPreviousDrawMode);
}




void PageObjectViewObjectContact::PaintFocusIndicator (
    OutputDevice& rDevice,
    bool bEraseBackground) const
{
    (void)bEraseBackground;

    if (GetPageDescriptor()->IsSelected()
        && mpProperties.get()!=NULL
        && ! mpProperties->IsShowFocus())
    {
        return;
    }

    if (GetPageDescriptor()->IsFocused())
    {
        Rectangle aPagePixelBBox (GetBoundingBox(rDevice,PreviewBoundingBox,PixelCoordinateSystem));

        aPagePixelBBox.Left() -= mnFocusIndicatorOffset;
        aPagePixelBBox.Top() -= mnFocusIndicatorOffset;
        aPagePixelBBox.Right() += mnFocusIndicatorOffset;
        aPagePixelBBox.Bottom() += mnFocusIndicatorOffset;

        PaintDottedRectangle(rDevice, aPagePixelBBox);
    }
}




void PageObjectViewObjectContact::PaintFadeEffectIndicator (OutputDevice& rDevice) const
{
    if (GetPage() != NULL
        && static_cast<const SdPage*>(GetPage())->getTransitionType() > 0)
    {
        Rectangle aIndicatorBox (
            GetBoundingBox(rDevice, FadeEffectIndicatorBoundingBox, ModelCoordinateSystem));

        USHORT nIconId (BMP_FADE_EFFECT_INDICATOR);
        if (rDevice.GetSettings().GetStyleSettings().GetHighContrastMode()!=0)
            nIconId = BMP_FADE_EFFECT_INDICATOR_H;

        rDevice.DrawImage (
            aIndicatorBox.TopLeft(),
            IconCache::Instance().GetIcon(nIconId));
    }
}




void PageObjectViewObjectContact::PaintPageName (OutputDevice& rDevice) const
{
    Rectangle aPageBox (GetBoundingBox(rDevice, PreviewBoundingBox, ModelCoordinateSystem));

    Font aOriginalFont (rDevice.GetFont());
    rDevice.SetFont(*FontProvider::Instance().GetFont(rDevice));

    const SdPage* pPage = static_cast<const SdPage*>(GetPage());
    int nPage = (pPage->GetPageNum()-1) / 2;
    // Name der Seite
    Point aPos = aPageBox.BottomLeft();
    const Size aSize (rDevice.PixelToLogic (Size (0, mnFadeEffectIndicatorOffset)));
    const Rectangle aIndicatorBox (
        GetBoundingBox(rDevice, FadeEffectIndicatorBoundingBox, ModelCoordinateSystem));

    aPos.Y() += aSize.Height();
    aPos.X() += 2 * aIndicatorBox.GetWidth();

    Size aTextBoxSize (aPageBox.Right() - aPos.X(), rDevice.GetFont().GetSize().Height());

    String sName (const_cast<SdPage*>(pPage)->GetName());
    if (sName.Len() == 0)
    {
        sName = String (SdResId(STR_PAGE));
        sName += String::CreateFromInt32 (nPage + 1);
    }

    USHORT nTextStyle
        = TEXT_DRAW_RIGHT
        | TEXT_DRAW_NEWSELLIPSIS;

    rDevice.SetTextColor(GetColor(rDevice, CS_TEXT));
    rDevice.DrawText(Rectangle(aPos,aTextBoxSize), sName, nTextStyle);

    rDevice.SetFont(aOriginalFont);
}




void PageObjectViewObjectContact::PaintPageNumber (OutputDevice& rDevice) const
{
    const Rectangle aPageBox (GetBoundingBox(rDevice, PreviewBoundingBox, ModelCoordinateSystem));

    const SdPage* pPage = static_cast<const SdPage*>(GetPage());
    const sal_Int32 nPageNumber (mpPageDescriptor->GetPageIndex() + 1);
    const String sPageNumber (String::CreateFromInt32 (nPageNumber));
    Point aPos = aPageBox.TopLeft();
    Rectangle aBox (GetBoundingBox(rDevice, PageNumberBoundingBox, ModelCoordinateSystem));

    // Paint the page number centered in its box.
    // TODO: What when the page number is wider than the page number box?
    const USHORT nTextStyle = TEXT_DRAW_CENTER | TEXT_DRAW_VCENTER;
    const Rectangle aTextBox (rDevice.GetTextRect (aBox, sPageNumber, nTextStyle));
    rDevice.SetFillColor();
    rDevice.SetTextColor(GetColor(rDevice, CS_TEXT));
    rDevice.DrawText(aTextBox, sPageNumber, nTextStyle);

    // Paint box arround the page number.  Strike through when slide is
    // excluded from the presentation
    if (pPage->IsExcluded())
    {
        // Make the box a little bit larger at the left so that the digits
        // do not touch the border.
        const Size aOffset (rDevice.PixelToLogic(Size(1,0)));
        aBox.Left() -= aOffset.Width();

        rDevice.SetLineColor(
            rDevice.GetSettings().GetStyleSettings().GetActiveColor());
        rDevice.DrawRect(aBox);
        rDevice.DrawLine(aBox.TopLeft(), aBox.BottomRight());
    }
}




SvBorder PageObjectViewObjectContact::CalculatePageModelBorder (
    OutputDevice* pDevice,
    int nPageCount)
{
    SvBorder aModelBorder;

    if (pDevice != NULL)
    {
        // 1. Initialize the border with the values that do not depend on
        // the device.
        Size aTopLeftBorders (pDevice->PixelToLogic (Size (
            mnPageNumberOffset+1,
            mnSelectionIndicatorOffset + mnSelectionIndicatorThickness)));
        Size aBottomRightBorders (pDevice->PixelToLogic (Size (
            mnSelectionIndicatorOffset + mnSelectionIndicatorThickness,
            mnFadeEffectIndicatorOffset)));
        aModelBorder = SvBorder (
            aTopLeftBorders.Width(),
            aTopLeftBorders.Height(),
            aBottomRightBorders.Width(),
            aBottomRightBorders.Height());


        // 2. Add the device dependent values.

        // Calculate the area of the page number.
        Size aPageNumberModelSize (
            CalculatePageNumberAreaModelSize (pDevice, nPageCount));

        // Update the border.
        aModelBorder.Left() += aPageNumberModelSize.Width();
        // The height of the page number area is the same as the height of
        // the page name area.
        aModelBorder.Bottom() += aPageNumberModelSize.Height();
    }

    return aModelBorder;
}




Size PageObjectViewObjectContact::CalculatePageNumberAreaModelSize (
    OutputDevice* pDevice,
    int nPageCount)
{
    // Set the correct font.
    Font aOriginalFont (pDevice->GetFont());
    pDevice->SetFont(*FontProvider::Instance().GetFont(*pDevice));

    String sPageNumberTemplate;
    if (nPageCount < 10)
        sPageNumberTemplate = String::CreateFromAscii("9");
    else if (nPageCount < 100)
        sPageNumberTemplate = String::CreateFromAscii("99");
    else if (nPageCount < 200)
        // Just for the case that 1 is narrower than 9.
        sPageNumberTemplate = String::CreateFromAscii("199");
    else if (nPageCount < 1000)
        sPageNumberTemplate = String::CreateFromAscii("999");
    else
        sPageNumberTemplate = String::CreateFromAscii("9999");
    // More then 9999 pages are not handled.

    Size aSize (
        pDevice->GetTextWidth (sPageNumberTemplate),
        pDevice->GetTextHeight ());

    pDevice->SetFont (aOriginalFont);

    return aSize;
}




model::SharedPageDescriptor
    PageObjectViewObjectContact::GetPageDescriptor (void) const
{
    PageObjectViewContact& rViewContact (
        static_cast<PageObjectViewContact&>(GetViewContact()));
    PageObject& rPageObject (
        static_cast<PageObject&>(rViewContact.GetPageObject()));
    return rPageObject.GetDescriptor();
}




Color PageObjectViewObjectContact::GetColor (
    const OutputDevice& rDevice,
    const ColorSpec eSpec,
    const double nOpacity) const
{
    (void)rDevice;
    if (mbIsBackgroundColorUpdatePending)
    {
        mbIsBackgroundColorUpdatePending = false;
        maBackgroundColor = mpProperties->GetBackgroundColor();
    }

    Color aColor;

    switch (eSpec)
    {
        case CS_SELECTION:
            aColor = mpProperties->GetSelectionColor();
            break;

        case CS_BACKGROUND:
            if (mpProperties.get()!=NULL
                && mpProperties->IsHighlightCurrentSlide()
                && GetPageDescriptor()->IsCurrentPage())
            {
                aColor = mpProperties->GetHighlightColor();
            }
            else
                aColor = maBackgroundColor;
            break;

        case CS_WINDOW:
            aColor = maBackgroundColor;
            break;

        case CS_TEXT:
        default:
            aColor = mpProperties->GetTextColor();
            break;
    }
    aColor.Merge(maBackgroundColor, BYTE(255*(nOpacity) + 0.5));
    return aColor;
}




Color PageObjectViewObjectContact::GetBackgroundColor (
    const OutputDevice& rDevice) const
{
    Color aBackgroundColor (COL_AUTO);
    bool bBackgroundColorSet (false);

    if ( ! bBackgroundColorSet)
    {
        aBackgroundColor = rDevice.GetSettings().GetStyleSettings().GetWindowColor();
    }

    aBackgroundColor.SetTransparency(0);
    return aBackgroundColor;
}




void PageObjectViewObjectContact::PaintDottedRectangle (
    OutputDevice& rDevice,
    const Rectangle& rRectangle,
    const DashType eDashType)
{
    const Color aOriginalFillColor (rDevice.GetFillColor());
    const Color aOriginalLineColor (rDevice.GetLineColor());

    // The dots or dashes are painted indepently of the zoom factor because
    // selection or focus rectangles are not part of the model.
    const bool bOriginalMapModeState(rDevice.IsMapModeEnabled());
    rDevice.EnableMapMode(FALSE);

    // Paint a solid rectangle first.
    rDevice.SetFillColor();
    rDevice.SetLineColor(COL_WHITE);
    rDevice.DrawRect(rRectangle);

    // Now paint the dots or dashes.
    LineInfo aDottedStyle (LINE_DASH);
    aDottedStyle.SetDashCount(0);
    aDottedStyle.SetDotCount(1);
    switch (eDashType)
    {
        case Dotted:
        default:
            aDottedStyle.SetDotLen(1);
            aDottedStyle.SetDistance(1);
            break;

        case Dashed:
            aDottedStyle.SetDotLen(3);
            aDottedStyle.SetDistance(3);
            break;
    }

    rDevice.SetLineColor(COL_BLACK);
    rDevice.DrawPolyLine(Polygon(rRectangle), aDottedStyle);

    // Restore the original state.
    rDevice.EnableMapMode(bOriginalMapModeState);
    rDevice.SetFillColor(aOriginalFillColor);
    rDevice.SetLineColor(aOriginalLineColor);
}


} } } // end of namespace ::sd::slidesorter::view
