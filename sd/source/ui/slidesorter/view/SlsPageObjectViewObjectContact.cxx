/*************************************************************************
 *
 *  $RCSfile: SlsPageObjectViewObjectContact.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-08-04 08:57:40 $
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
 *  The Initial Developer of the Original Ckode is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "view/SlsPageObjectViewObjectContact.hxx"

#include "view/SlideSorterView.hxx"
#include "view/SlsPageObjectViewContact.hxx"
#include "view/SlsPageObject.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "cache/SlsPageCache.hxx"
#include "res_bmp.hrc"
#include "tools/IconCache.hxx"

#include "sdpage.hxx"
#include "sdresid.hxx"
#include "glob.hrc"
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/svdopage.hxx>
#include <svx/xoutx.hxx>
#include <svx/svdpagv.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlnclit.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/lineinfo.hxx>
#include <tools/color.hxx>

using namespace ::sdr::contact;
using namespace ::sd::slidesorter::model;

namespace sd { namespace slidesorter { namespace view {

const sal_Int32 PageObjectViewObjectContact::mnSelectionIndicatorOffset = 4;
const sal_Int32 PageObjectViewObjectContact::mnSelectionIndicatorThickness = 2;
const sal_Int32 PageObjectViewObjectContact::mnFocusIndicatorOffset = 2;
const sal_Int32 PageObjectViewObjectContact::mnFadeEffectIndicatorOffset = 9;
const sal_Int32 PageObjectViewObjectContact::mnFadeEffectIndicatorSize = 14;
const sal_Int32 PageObjectViewObjectContact::mnPageNumberOffset = 9;

PageObjectViewObjectContact::PageObjectViewObjectContact (
    ObjectContact& rObjectContact,
    ViewContact& rViewContact,
    cache::PageCache* pCache)
    : ViewObjectContact (rObjectContact, rViewContact),
      mbIsValid(true),
      mpCache(pCache)
{
    GetPageDescriptor().SetViewObjectContact (this);
}




Rectangle PageObjectViewObjectContact::GetPixelBox (
    DisplayInfo& rDisplayInfo)
{
    const OutputDevice* pDevice = rDisplayInfo.GetOutputDevice();
    DBG_ASSERT (pDevice!=NULL,
        "PageObjectViewObjectContact::CalculatePixelBBox: pOriginalOutDev==NULL");
    return pDevice->LogicToPixel(GetViewContact().GetPaintRectangle());
}




Rectangle PageObjectViewObjectContact::GetPreviewPixelBox (
    DisplayInfo& rDisplayInfo)
{
    const OutputDevice* pDevice = rDisplayInfo.GetOutputDevice();
    DBG_ASSERT (pDevice!=NULL,
        "PageObjectViewObjectContact::CalculatePixelBBox: pOriginalOutDev==NULL");
    return  pDevice->LogicToPixel(
        static_cast<PageObjectViewContact&>(GetViewContact()
            ).GetPageObject().GetCurrentBoundRect());
}




BitmapEx PageObjectViewObjectContact::CreatePreview (DisplayInfo& rDisplayInfo)
{
    if ( ! mbIsValid)
        return BitmapEx();

    BitmapEx aBitmap;
    OutputDevice* pDevice = rDisplayInfo.GetOutputDevice();
    Rectangle aNewSizePixel = GetPreviewPixelBox(rDisplayInfo);

    if( ! aNewSizePixel.IsEmpty())
    {
        // calculate const vars
        const Point aEmptyPoint(0, 0);
        const Rectangle& aPaintRectangle (
            static_cast<PageObjectViewContact&>(GetViewContact()
                ).GetPageObject().GetCurrentBoundRect());
        ExtOutputDevice* pOriginalExtOutDev
            = rDisplayInfo.GetExtendedOutputDevice();
        const Rectangle aPaintRectanglePixel(
            pDevice->LogicToPixel(aPaintRectangle));
        MapMode aMapMode(pDevice->GetMapMode());
        Point aMapModeOrigin(aPaintRectangle.TopLeft());

        // calculate new OffsetTopLeft in pixels
        Point aOffsetTopLeft(
            aNewSizePixel.TopLeft() - aPaintRectanglePixel.TopLeft());

        if(aOffsetTopLeft.X() || aOffsetTopLeft.Y())

        {
            // if there is a pixel offset, calculate the logical offset
            // for later painting
            const Point aLogicalNewSizePixelTopLeft(
                pDevice->PixelToLogic(aNewSizePixel.TopLeft()));
            aOffsetTopLeft = aLogicalNewSizePixelTopLeft
                - aPaintRectangle.TopLeft();

            // also prepare the MapMode origin
            aMapModeOrigin += aOffsetTopLeft;
        }

        // Prepare MapMode for BufferDevice
        aMapMode.SetOrigin (Point (-aMapModeOrigin.X(), -aMapModeOrigin.Y()));

        // prepare VirtualDevice for mask
        VirtualDevice aBufferDevice(*pDevice, 0L, 0L);

        // Set OutputSize and MapMode
        aBufferDevice.SetOutputSizePixel(aNewSizePixel.GetSize());
        aBufferDevice.SetMapMode(aMapMode);

        // set new values at rDisplayInfo
        pOriginalExtOutDev->SetOutDev(&aBufferDevice);
        rDisplayInfo.SetOutputDevice(&aBufferDevice);

        // paint in new OutDev using call to parent
        ViewObjectContact::PaintObject(rDisplayInfo);

        // set back to old OutDev, restore rDisplayInfo
        pOriginalExtOutDev->SetOutDev(pDevice);
        rDisplayInfo.SetOutputDevice(pDevice);

        // get BitmapEx
        aBufferDevice.EnableMapMode(sal_False);
        aBitmap = aBufferDevice.GetBitmapEx(
            aEmptyPoint,
            aBufferDevice.GetOutputSizePixel());
    }

    return aBitmap;
}




BitmapEx PageObjectViewObjectContact::GetPreview (
    DisplayInfo& rDisplayInfo,
    const Rectangle& rNewSizePixel)
{
    BitmapEx aBitmap;

    if (mbIsValid)
    {
        if (mpCache != NULL)
        {
            aBitmap = mpCache->GetPreviewBitmap(
                *this,
                rNewSizePixel.GetSize());
        }
        else
            aBitmap = CreatePreview (rDisplayInfo);
    }

    return aBitmap;
}




void PageObjectViewObjectContact::PaintObject (DisplayInfo& rDisplayInfo)
{
    bool bIsPainted = false;

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
        mbIsInvalidated = sal_False;

        // Save (a part of) the state of the output device.
        ULONG nPreviousDrawMode = pDevice->GetDrawMode();
        const Color aOriginalFillColor (pDevice->GetFillColor());
        const Color aOriginalLineColor (pDevice->GetLineColor());
        Font aOriginalFont (pDevice->GetFont());

        // Set default sans serif font.
        Font aNewFont (OutputDevice::GetDefaultFont (
            DEFAULTFONT_SANS_UNICODE,
            pDevice->GetSettings().GetInternational().GetLanguage(),
            FALSE,
            pDevice));
        Size aFontModelSize (pDevice->PixelToLogic(aNewFont.GetSize()));
        aNewFont.SetSize (aFontModelSize);
        aNewFont.SetTransparent (TRUE);
        pDevice->SetFont (aNewFont);

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




void PageObjectViewObjectContact::PrepareDelete (void)
{
    mbIsValid = false;

    GetPageDescriptor().SetViewObjectContact (NULL);

    if (mpCache != NULL)
        mpCache->ReleasePreviewBitmap(*this);

    ViewObjectContact::PrepareDelete();
}




const SdrPage* PageObjectViewObjectContact::GetPage (void) const
{
    return static_cast<PageObjectViewContact&>(GetViewContact()).GetPage();
}




void PageObjectViewObjectContact::ActionChanged (void)
{
    // Tell the cache to create a new preview bitmap.
    if (mpCache != NULL)
    {
        // We are brave and hope that the cache does the right thing with us
        // even when we are not valid anymore.
        mpCache->InvalidatePreviewBitmap(*this);
    }

    // We have to call the base class, even when we are not in a valid state.
    ViewObjectContact::ActionChanged();
}




void PageObjectViewObjectContact::PaintContent  (
    DisplayInfo& rDisplayInfo)
{
    PaintPreview (rDisplayInfo);
    PaintFrame (rDisplayInfo);
    PaintFocusIndicator (rDisplayInfo);
    PaintSelectionIndicator (rDisplayInfo);
    PaintFadeEffectIndicator (rDisplayInfo);
    PaintPageName (rDisplayInfo);
    PaintPageNumber (rDisplayInfo);
}




void PageObjectViewObjectContact::PaintPreview  (
    DisplayInfo& rDisplayInfo)
{
    Rectangle aNewSizePixel = GetPreviewPixelBox(rDisplayInfo);
    BitmapEx aPreview (GetPreview(rDisplayInfo, aNewSizePixel));

    // Paint using cached bitmap.
    OutputDevice* pDevice = rDisplayInfo.GetOutputDevice();
    const sal_Bool bWasEnabled(pDevice->IsMapModeEnabled());
    pDevice->EnableMapMode(sal_False);
    pDevice->DrawBitmapEx(aNewSizePixel.TopLeft(), aPreview);
    pDevice->EnableMapMode(bWasEnabled);
}




void PageObjectViewObjectContact::PaintFrame (
    DisplayInfo& rDisplayInfo) const
{
    OutputDevice* pDevice = rDisplayInfo.GetOutputDevice();

    Rectangle aFrameBox (pDevice->LogicToPixel(GetModelBoundingBox()));
    pDevice->EnableMapMode(FALSE);
    pDevice->SetFillColor ();
    svtools::ColorConfig aColorConfig;
    Color aColor = aColorConfig.GetColorValue(svtools::DOCBOUNDARIES).nColor;
    pDevice->SetLineColor (aColor);
    pDevice->DrawRect (aFrameBox);
    pDevice->EnableMapMode(TRUE);
}




void PageObjectViewObjectContact::PaintSelectionIndicator (
    DisplayInfo& rDisplayInfo) const
{
    if (GetPageDescriptor().IsSelected())
    {
        OutputDevice* pDevice = rDisplayInfo.GetOutputDevice();

        Rectangle aSelectionFrame (GetModelBoundingBox());

        svtools::ColorConfig aColorConfig;
        Color aColor = aColorConfig.GetColorValue(svtools::FONTCOLOR).nColor;

        // Set default draw mode to be able to correctly draw the selected
        // (and only that) frame.
        ULONG nPreviousDrawMode = pDevice->GetDrawMode();
        pDevice->SetDrawMode (DRAWMODE_DEFAULT);

        Rectangle aInner (pDevice->LogicToPixel(aSelectionFrame));
        const Color aOldFillColor (pDevice->GetFillColor());
        const Color aOldLineColor (pDevice->GetLineColor());

        pDevice->SetFillColor ();
        pDevice->SetLineColor (aColor);

        for (int nOffset=mnSelectionIndicatorOffset;
             nOffset<=mnSelectionIndicatorOffset+mnSelectionIndicatorThickness;
             nOffset++)
        {
            Rectangle aFrame (aInner);
            aFrame.Left() -= nOffset;
            aFrame.Top() -= nOffset;
            aFrame.Right() += nOffset;
            aFrame.Bottom() += nOffset;
            pDevice->DrawRect (pDevice->PixelToLogic(aFrame));
        }

        pDevice->SetLineColor (aOldLineColor);
        pDevice->SetFillColor (aOldFillColor);

        // Restore the previous draw mode.
        pDevice->SetDrawMode (nPreviousDrawMode);
    }
}




void PageObjectViewObjectContact::PaintFocusIndicator (
    DisplayInfo& rDisplayInfo) const
{
    if (GetPageDescriptor().IsFocused())
    {
        OutputDevice* pDevice = rDisplayInfo.GetOutputDevice();
        PageObjectViewContact& rViewContact (
            static_cast<PageObjectViewContact&>(GetViewContact()));
        Rectangle aPagePixelBBox (
            pDevice->LogicToPixel(rViewContact.GetPageRectangle ()));

        aPagePixelBBox.Left() -= mnFocusIndicatorOffset;
        aPagePixelBBox.Top() -= mnFocusIndicatorOffset;
        aPagePixelBBox.Right() += mnFocusIndicatorOffset;
        aPagePixelBBox.Bottom() += mnFocusIndicatorOffset;

        pDevice->EnableMapMode (FALSE);
        pDevice->SetFillColor();
        pDevice->SetLineColor(COL_WHITE);
        pDevice->DrawRect (aPagePixelBBox);

        LineInfo aDottedStyle (LINE_DASH);
        aDottedStyle.SetDashCount (0);
        aDottedStyle.SetDotCount (1);
        aDottedStyle.SetDotLen (1);
        aDottedStyle.SetDistance (1);

        pDevice->SetLineColor(COL_BLACK);
        pDevice->DrawPolyLine (Polygon(aPagePixelBBox), aDottedStyle);

        pDevice->EnableMapMode (TRUE);
    }
}




void PageObjectViewObjectContact::PaintFadeEffectIndicator (
    DisplayInfo& rDisplayInfo,
    bool bHighlight) const
{
    OutputDevice* pDevice = rDisplayInfo.GetOutputDevice();

    Rectangle aIndicatorBox (GetFadeEffectIndicatorArea (pDevice));

    bool bHighContrastMode (
       Application::GetSettings().GetStyleSettings().GetHighContrastMode()!=0);
    if (GetPage() != NULL
        && static_cast<const SdPage*>(GetPage())->GetFadeEffect()
        != ::com::sun::star::presentation::FadeEffect_NONE)
    {
        pDevice->DrawImage (aIndicatorBox.TopLeft(),
            IconCache::Instance().GetIcon (
                bHighContrastMode
                ? BMP_FADE_EFFECT_INDICATOR_H
                : BMP_FADE_EFFECT_INDICATOR));
    }
}




void PageObjectViewObjectContact::PaintPageName (
    DisplayInfo& rDisplayInfo) const
{
    OutputDevice* pDevice = rDisplayInfo.GetOutputDevice();
    Rectangle aPageBox (GetModelBoundingBox ());

    Font aOriginalFont (pDevice->GetFont());
    Font aNewFont (OutputDevice::GetDefaultFont (
        DEFAULTFONT_SANS_UNICODE,
        pDevice->GetSettings().GetInternational().GetLanguage(),
        FALSE,
        pDevice));
    Size aFontModelSize (pDevice->PixelToLogic(aNewFont.GetSize()));
    aNewFont.SetSize (aFontModelSize);
    aNewFont.SetTransparent (TRUE);
    pDevice->SetFont (aNewFont);

    const SdPage* pPage = static_cast<const SdPage*>(GetPage());
    int nPage = (pPage->GetPageNum()-1) / 2;
    // Name der Seite
    Point aPos = aPageBox.BottomLeft();
    Size aPageSize (aPageBox.GetSize());
    Size aSize (pDevice->PixelToLogic (Size (0, mnFadeEffectIndicatorOffset)));
    Rectangle aIndicatorBox (GetFadeEffectIndicatorArea (pDevice));

    aPos.Y() += aSize.Height();
    aPos.X() += 2 * aIndicatorBox.GetWidth();

    Size aTextBoxSize (aPageBox.Right() - aPos.X(), aFontModelSize.Height());

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
    Rectangle aPageBox (GetModelBoundingBox ());

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




Rectangle PageObjectViewObjectContact::GetFadeEffectIndicatorArea (
    OutputDevice* pDevice) const
{
    const SdPage* pPage = static_cast<const SdPage*>(GetPage());
    Rectangle aPageModelBox (GetModelBoundingBox ());
    Size aModelOffset = pDevice->PixelToLogic (
        Size (0, mnFadeEffectIndicatorOffset));

    // Flush left just outside the selection rectangle.
    Rectangle aIndicatorArea (
        Point (
            aPageModelBox.Left(),
            aPageModelBox.Bottom() + aModelOffset.Height()
            ),
        pDevice->PixelToLogic (
            IconCache::Instance().GetIcon(BMP_FADE_EFFECT_INDICATOR)
            .GetSizePixel())
        );

    return aIndicatorArea;
}




Rectangle PageObjectViewObjectContact::GetPageNumberArea (
    OutputDevice* pDevice)
{
    const SdPage* pPage = static_cast<const SdPage*>(GetPage());
    Rectangle aPageModelBox (GetModelBoundingBox ());
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




Rectangle PageObjectViewObjectContact::GetModelBoundingBox (void) const
{
    return static_cast<PageObjectViewContact&>(GetViewContact())
        .GetPageObjectBoundingBox();
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
            mnPageNumberOffset,
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
    Font aNewFont (OutputDevice::GetDefaultFont (
        DEFAULTFONT_SANS_UNICODE,
        pDevice->GetSettings().GetInternational().GetLanguage(),
        FALSE,
        pDevice));
    Size aFontModelSize (pDevice->PixelToLogic(aNewFont.GetSize()));
    aNewFont.SetSize (aFontModelSize);
    aNewFont.SetWeight (WEIGHT_BOLD);
    aNewFont.SetTransparent (TRUE);
    pDevice->SetFont (aNewFont);

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



} } } // end of namespace ::sd::slidesorter::view
