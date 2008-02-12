/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsPageObjectViewObjectContact.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: vg $ $Date: 2008-02-12 16:29:06 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "view/SlsPageObjectViewObjectContact.hxx"

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
#include <svx/sdr/contact/objectcontactofpageview.hxx>
#include <svx/svdopage.hxx>
#include <svx/xoutx.hxx>
#include <svx/svdpagv.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/svdoutl.hxx>
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


const sal_Int32 PageObjectViewObjectContact::mnSelectionIndicatorOffset = 1;
const sal_Int32 PageObjectViewObjectContact::mnSelectionIndicatorThickness = 3;
const sal_Int32 PageObjectViewObjectContact::mnFocusIndicatorOffset = 2;
const sal_Int32 PageObjectViewObjectContact::mnFadeEffectIndicatorOffset = 9;
const sal_Int32 PageObjectViewObjectContact::mnFadeEffectIndicatorSize = 14;
const sal_Int32 PageObjectViewObjectContact::mnPageNumberOffset = 9;
const sal_Int32 PageObjectViewObjectContact::mnMouseOverEffectOffset = 2;
const sal_Int32 PageObjectViewObjectContact::mnMouseOverEffectThickness = 1;

PageObjectViewObjectContact::PageObjectViewObjectContact (
    ObjectContact& rObjectContact,
    ViewContact& rViewContact,
    const ::boost::shared_ptr<cache::PageCache>& rpCache)
    : ViewObjectContact (rObjectContact, rViewContact),
      mbIsValid(true),
      mpCache(rpCache),
      mpNotifier(NULL),
      mbInPrepareDelete(false)
{
    GetPageDescriptor().SetViewObjectContact (this);
}




PageObjectViewObjectContact::~PageObjectViewObjectContact (void)
{
    if (mpCache.get() != NULL)
    {
        mpCache->ReleasePreviewBitmap(*this);
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
            Size aNumberSize (GetPageDescriptor().GetPageNumberAreaModelSize());
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




BitmapEx PageObjectViewObjectContact::CreatePreview (DisplayInfo& rDisplayInfo)
{
    const SdPage* pPage = static_cast<const SdPage*>(GetPage());
    OutputDevice* pDevice = rDisplayInfo.GetOutputDevice();
    Rectangle aPreviewPixelBox (GetBoundingBox(*pDevice,PreviewBoundingBox,PixelCoordinateSystem));

    PreviewRenderer aRenderer (pDevice);
    Image aPreview (aRenderer.RenderPage(
        pPage,
        aPreviewPixelBox.GetSize(),
        String()));

    return aPreview.GetBitmapEx();
}




BitmapEx PageObjectViewObjectContact::GetPreview (
    DisplayInfo& rDisplayInfo,
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
                    *this,
                    rNewSizePixel.GetSize());
                mpCache->SetPreciousFlag(*this, true);
            }
            else
                aBitmap = CreatePreview (rDisplayInfo);
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

            // Save (a part of) the state of the output device.
            ULONG nPreviousDrawMode = pDevice->GetDrawMode();
            const Color aOriginalFillColor (pDevice->GetFillColor());
            const Color aOriginalLineColor (pDevice->GetLineColor());
            Font aOriginalFont (pDevice->GetFont());

            // Set default font.
            pDevice->SetFont(*FontProvider::Instance().GetFont(*pDevice));

            PaintContent (rDisplayInfo);

            // Restore old device state.
            pDevice->SetFont (aOriginalFont);
            pDevice->SetLineColor (aOriginalLineColor);
            pDevice->SetFillColor (aOriginalFillColor);
            pDevice->SetDrawMode (nPreviousDrawMode);

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

    GetPageDescriptor().SetViewObjectContact (NULL);

    if (mpCache != NULL)
        mpCache->ReleasePreviewBitmap(*this);

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
            pDocument,
            GetPage());
    }

    ViewObjectContact::ActionChanged();
}




void PageObjectViewObjectContact::PaintContent  (
    DisplayInfo& rDisplayInfo)
{
    PaintPreview (rDisplayInfo);
    PaintFrame (*rDisplayInfo.GetOutputDevice());
    PaintFadeEffectIndicator (rDisplayInfo);
    PaintPageName (rDisplayInfo);
    PaintPageNumber (rDisplayInfo);
}




void PageObjectViewObjectContact::PaintPreview  (
    DisplayInfo& rDisplayInfo)
{
    OutputDevice* pDevice = rDisplayInfo.GetOutputDevice();
    if (pDevice != NULL)
    {
        Rectangle aNewSizePixel (
            GetBoundingBox(*pDevice,PreviewBoundingBox,PixelCoordinateSystem));
        BitmapEx aPreview (GetPreview(rDisplayInfo, aNewSizePixel));

        // Paint using cached bitmap.
        const sal_Bool bWasEnabled(pDevice->IsMapModeEnabled());
        pDevice->EnableMapMode(sal_False);
        pDevice->DrawBitmapEx(aNewSizePixel.TopLeft(), aPreview);
        pDevice->EnableMapMode(bWasEnabled);
    }
}




void PageObjectViewObjectContact::PaintFrame (
    OutputDevice& rDevice,
    bool bShowMouseOverEffect) const
{
    PaintBorder (rDevice);
    PaintSelectionIndicator (rDevice);
    if ( ! GetPageDescriptor().IsSelected())
        PaintMouseOverEffect (rDevice, bShowMouseOverEffect);
    // else the mouse over effect is not visible when the selection
    // indicator is painted already.
    PaintFocusIndicator (rDevice,
        GetPageDescriptor().IsSelected() || ! bShowMouseOverEffect);
}




void PageObjectViewObjectContact::PaintBorder (
    OutputDevice& rDevice) const
{
    Rectangle aFrameBox (GetBoundingBox(rDevice, PreviewBoundingBox, PixelCoordinateSystem));
    rDevice.EnableMapMode(FALSE);
    rDevice.SetFillColor ();
    svtools::ColorConfig aColorConfig;
    Color aColor = aColorConfig.GetColorValue(svtools::FONTCOLOR).nColor;
    rDevice.SetLineColor (aColor);
    rDevice.DrawRect (aFrameBox);
    rDevice.EnableMapMode(TRUE);
}




void PageObjectViewObjectContact::PaintSelectionIndicator (
    OutputDevice& rDevice) const
{
    if (GetPageDescriptor().IsSelected())
    {
        const Color aOldFillColor (rDevice.GetFillColor());
        const Color aOldLineColor (rDevice.GetLineColor());

        // Determine colors for the frame and the background and mix them to
        // obtain a third color that is used for an antialiasing effect.
        svtools::ColorConfig aColorConfig;
        Color aFrameColor (
            rDevice.GetSettings().GetStyleSettings().GetMenuHighlightColor());
        Color aBackgroundColor (
            rDevice.GetSettings().GetStyleSettings().GetWindowColor());
        Color aCornerColor (aFrameColor);
        aCornerColor.Merge (aBackgroundColor, 128);

        // Set default draw mode to be able to correctly draw the selected
        // (and only that) frame.
        ULONG nPreviousDrawMode = rDevice.GetDrawMode();
        rDevice.SetDrawMode (DRAWMODE_DEFAULT);

        Rectangle aInner (GetBoundingBox(rDevice,PreviewBoundingBox,PixelCoordinateSystem));
        rDevice.EnableMapMode (FALSE);

        rDevice.SetFillColor ();
        rDevice.SetLineColor (aFrameColor);

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
}




void PageObjectViewObjectContact::PaintMouseOverEffect (
    OutputDevice& rDevice,
    bool bVisible) const
{
    ULONG nPreviousDrawMode = rDevice.GetDrawMode();
    rDevice.SetDrawMode (DRAWMODE_DEFAULT);
    Rectangle aInner (GetBoundingBox(rDevice,PreviewBoundingBox,PixelCoordinateSystem));
    rDevice.EnableMapMode (FALSE);

    svtools::ColorConfig aColorConfig;
    Color aSelectionColor (
        rDevice.GetSettings().GetStyleSettings().GetMenuHighlightColor());
    Color aBackgroundColor (
        rDevice.GetSettings().GetStyleSettings().GetWindowColor());
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

    if (GetPageDescriptor().IsFocused())
    {
        Rectangle aPagePixelBBox (
            GetBoundingBox(rDevice,PreviewBoundingBox,PixelCoordinateSystem));

        aPagePixelBBox.Left() -= mnFocusIndicatorOffset;
        aPagePixelBBox.Top() -= mnFocusIndicatorOffset;
        aPagePixelBBox.Right() += mnFocusIndicatorOffset;
        aPagePixelBBox.Bottom() += mnFocusIndicatorOffset;

        PaintDottedRectangle(rDevice, aPagePixelBBox);
    }
}




void PageObjectViewObjectContact::PaintFadeEffectIndicator (
    DisplayInfo& rDisplayInfo,
    bool ) const
{
    if (GetPage() != NULL
        && static_cast<const SdPage*>(GetPage())->getTransitionType() > 0)
    {
        OutputDevice* pDevice = rDisplayInfo.GetOutputDevice();
        if (pDevice != NULL)
        {
            Rectangle aIndicatorBox (
                GetBoundingBox(*pDevice, FadeEffectIndicatorBoundingBox, ModelCoordinateSystem));

            USHORT nIconId (BMP_FADE_EFFECT_INDICATOR);
            if (pDevice->GetSettings().GetStyleSettings().GetHighContrastMode()!=0)
                nIconId = BMP_FADE_EFFECT_INDICATOR_H;

            pDevice->DrawImage (
                aIndicatorBox.TopLeft(),
                IconCache::Instance().GetIcon(nIconId));
        }
    }
}




void PageObjectViewObjectContact::PaintPageName (
    DisplayInfo& rDisplayInfo) const
{
    OutputDevice* pDevice = rDisplayInfo.GetOutputDevice();
    Rectangle aPageBox (GetBoundingBox(*pDevice, PreviewBoundingBox, ModelCoordinateSystem));

    Font aOriginalFont (pDevice->GetFont());
    pDevice->SetFont(*FontProvider::Instance().GetFont(*pDevice));

    const SdPage* pPage = static_cast<const SdPage*>(GetPage());
    int nPage = (pPage->GetPageNum()-1) / 2;
    // Name der Seite
    Point aPos = aPageBox.BottomLeft();
    Size aPageSize (aPageBox.GetSize());
    Size aSize (pDevice->PixelToLogic (Size (0, mnFadeEffectIndicatorOffset)));
    Rectangle aIndicatorBox (
        GetBoundingBox(*pDevice, FadeEffectIndicatorBoundingBox, ModelCoordinateSystem));

    aPos.Y() += aSize.Height();
    aPos.X() += 2 * aIndicatorBox.GetWidth();

    Size aTextBoxSize (aPageBox.Right() - aPos.X(), pDevice->GetFont().GetSize().Height());

    String sName (const_cast<SdPage*>(pPage)->GetName());
    if (sName.Len() == 0)
    {
        sName = String (SdResId(STR_PAGE));
        sName += String::CreateFromInt32 (nPage + 1);
    }

    USHORT nTextStyle
        = TEXT_DRAW_RIGHT
        | TEXT_DRAW_NEWSELLIPSIS;

    pDevice->DrawText (Rectangle(aPos,aTextBoxSize), sName, nTextStyle);

    pDevice->SetFont (aOriginalFont);
}




void PageObjectViewObjectContact::PaintPageNumber (
    DisplayInfo& rDisplayInfo)
{
    OutputDevice* pDevice = rDisplayInfo.GetOutputDevice();
    Rectangle aPageBox (GetBoundingBox(*pDevice, PreviewBoundingBox, ModelCoordinateSystem));

    const SdPage* pPage = static_cast<const SdPage*>(GetPage());
    int nPageNumber ((pPage->GetPageNum()-1) / 2 + 1);
    String sPageNumber (String::CreateFromInt32 (nPageNumber));
    Point aPos = aPageBox.TopLeft();
    Rectangle aBox (GetPageNumberArea (pDevice));

    // Paint the page number centered in its box.
    // TODO: What when the page number is wider than the page number box?
    USHORT nTextStyle = TEXT_DRAW_CENTER | TEXT_DRAW_VCENTER;
    Rectangle aTextBox (pDevice->GetTextRect (aBox, sPageNumber, nTextStyle));
    //    int nLeft = aTextBox.Left();
    //    int nTop = aBox.Top() + (aBox.Top() - aTextBox.GetHeight()) / 2;
    pDevice->SetFillColor ();
    svtools::ColorConfig aColorConfig;
    pDevice->SetLineColor (
            pDevice->GetSettings().GetStyleSettings().GetActiveTextColor());
    pDevice->DrawText (aTextBox, sPageNumber, nTextStyle);

    // Paint box arround the page number.  Strike through when slide is
    // excluded from the presentation
    if (pPage->IsExcluded())
    {
        // Make the box a little bit larger at the left so that the digits
        // do not touch the border.
        Size aOffset (pDevice->PixelToLogic(Size(1,0)));
        aBox.Left() -= aOffset.Width();

        pDevice->SetLineColor (
            pDevice->GetSettings().GetStyleSettings().GetActiveColor());
        pDevice->DrawRect (aBox);
        pDevice->DrawLine (aBox.TopLeft(), aBox.BottomRight());
    }
}




Rectangle PageObjectViewObjectContact::GetPageNumberArea (
    OutputDevice* pDevice)
{
    Rectangle aPageModelBox (GetBoundingBox(*pDevice, PreviewBoundingBox, ModelCoordinateSystem));
    Size aModelOffset = pDevice->PixelToLogic (
        Size (mnPageNumberOffset, mnPageNumberOffset));
    Size aNumberSize (GetPageDescriptor().GetPageNumberAreaModelSize());

    Rectangle aPageNumberArea (
        Point (
            aPageModelBox.Left() - aModelOffset.Width() - aNumberSize.Width(),
            aPageModelBox.Top()),
        aNumberSize);

    return aPageNumberArea;
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




model::PageDescriptor&
    PageObjectViewObjectContact::GetPageDescriptor (void) const
{
    PageObjectViewContact& rViewContact (
        static_cast<PageObjectViewContact&>(GetViewContact()));
    PageObject& rPageObject (
        static_cast<PageObject&>(rViewContact.GetPageObject()));
    return rPageObject.GetDescriptor();
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
