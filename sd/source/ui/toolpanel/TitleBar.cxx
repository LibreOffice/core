/*************************************************************************
 *
 *  $RCSfile: TitleBar.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:39:14 $
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

#include "TitleBar.hxx"

#include "ControlContainerDescriptor.hxx"

#ifndef _SV_DECOVIEW_HXX
#include <vcl/decoview.hxx>
#endif
#include <vcl/window.hxx>
#include <vcl/virdev.hxx>
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SDRESID_HXX
#include "sdresid.hxx"
#endif
#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif
#include <vcl/lineinfo.hxx>
#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef _XOUTX_HXX
#include <svx/xoutx.hxx>
#endif
#ifndef _SVX_XDASH_HXX
#include <svx/xdash.hxx>
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#ifndef _SVX_XLNDSIT_HXX
#include <svx/xlndsit.hxx>
#endif
#ifndef _SVX_XLINEIT0_HXX
#include <svx/xlineit0.hxx>
#endif
#include <svx/svdobj.hxx>
#include <svx/svdpool.hxx>
#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif
#ifndef _SVX_XLNCLIT_HXX
#include <svx/xlnclit.hxx>
#endif
#ifndef SVX_XFILLIT0_HXX
#include <svx/xfillit0.hxx>
#endif
#include "res_bmp.hrc"


namespace sd { namespace toolpanel {

const int TitleBar::snIndentationWidth = 16;
Image TitleBar::saTriangleRight;
Image TitleBar::saTriangleRightHC;
Image TitleBar::saTriangleDown;
Image TitleBar::saTriangleDownHC;
Image TitleBar::saExpanded;
Image TitleBar::saExpandedHC;
Image TitleBar::saCollapsed;
Image TitleBar::saCollapsedHC;
bool TitleBar::sbImagesInitialized = false;


TitleBar::TitleBar (
    ::Window* pParent,
    const String& rsTitle,
    TitleBarType eType,
    bool bIsExpandable)
    : ::Window (pParent),
      TreeNode (this),
      msTitle(rsTitle),
      meType (eType),
      mbExpanded (false),
      mbFocused (false),
      mbMouseOver (false),
      mpDevice (new VirtualDevice (*this)),
      mbIsExpandable (bIsExpandable)
{
    EnableMapMode (FALSE);
    InitializeImages();

    SetBackground (Wallpaper());

    // Change the mouse pointer shape so that it acts as a mouse over effect.
    switch (meType)
    {
        case TBT_WINDOW_TITLE:
            break;

        case TBT_CONTROL_TITLE:
        case TBT_SUB_CONTROL_HEADLINE:
            if (mbIsExpandable)
                SetPointer (POINTER_REFHAND);
            break;
    }
}




TitleBar::~TitleBar (void)
{
}




Size TitleBar::GetPreferredSize (void)
{
    int nWidth = GetOutputSizePixel().Width();
    Rectangle aTitleBarBox (
        CalculateTitleBarBox(
            CalculateTextBoundingBox(nWidth, true),
            nWidth));

    return aTitleBarBox.GetSize();
}




sal_Int32 TitleBar::GetPreferredWidth (sal_Int32 nHeight)
{
    Rectangle aTitleBarBox (
        CalculateTitleBarBox(
            CalculateTextBoundingBox(0, true),
            0));
    return aTitleBarBox.GetWidth();
}




sal_Int32 TitleBar::GetPreferredHeight (sal_Int32 nWidth)
{
    Rectangle aTitleBarBox (
        CalculateTitleBarBox(
            CalculateTextBoundingBox(nWidth, true),
            nWidth));

    return aTitleBarBox.GetHeight();
}




bool TitleBar::IsResizable (void)
{
    return true;
}




::Window* TitleBar::GetWindow (void)
{
    return this;
}




sal_Int32 TitleBar::GetMinimumWidth (void)
{
    return 20;
}




void TitleBar::Paint (const Rectangle& rBoundingBox)
{
    mpDevice->SetMapMode(GetMapMode());
    mpDevice->SetOutputSize (GetOutputSizePixel());

    switch (meType)
    {
        case TBT_WINDOW_TITLE:
            PaintWindowTitleBar ();
            break;

        case TBT_CONTROL_TITLE:
            PaintPanelControlTitle ();
            break;

        case TBT_SUB_CONTROL_HEADLINE:
            PaintSubPanelHeadLineBar ();
            break;
    }

    DrawOutDev (
        Point(0,0),
        GetOutputSizePixel(),
        Point(0,0),
        GetOutputSizePixel(),
        *mpDevice);

    ::Window::Paint (rBoundingBox);
}




void TitleBar::Expand (bool bFlag)
{
    mbExpanded = bFlag;
    Invalidate ();
}




void TitleBar::Collapse (void)
{
    Expand (false);
}




void TitleBar::SetFocus (bool bFlag)
{
    mbFocused = bFlag;
    Invalidate ();
}




void TitleBar::SetMouseOver (bool bFlag)
{
    if (bFlag != mbMouseOver)
    {
        mbMouseOver = bFlag;
        //        Invalidate();
    }
}




Image* TitleBar::GetExpansionIndicator (void) const
{
    Image* pIndicator = NULL;
    bool bHighContrastMode (
        GetSettings().GetStyleSettings().GetHighContrastMode() != 0);
    if (mbIsExpandable)
    {
        switch (meType)
        {
            case TBT_CONTROL_TITLE:
                if (mbExpanded)
                    if (bHighContrastMode)
                        pIndicator = &saTriangleDownHC;
                    else
                        pIndicator = &saTriangleDown;
                else
                    if (bHighContrastMode)
                        pIndicator = &saTriangleRightHC;
                    else
                        pIndicator = &saTriangleRight;

                break;

            case TBT_SUB_CONTROL_HEADLINE:
                if (mbExpanded)
                    if (bHighContrastMode)
                        pIndicator = &saExpandedHC;
                    else
                        pIndicator = &saExpanded;
                else
                    if (bHighContrastMode)
                        pIndicator = &saCollapsedHC;
                    else
                        pIndicator = &saCollapsed;
                break;

            default:
            case TBT_WINDOW_TITLE:
                pIndicator = NULL;
                break;
        }
    }

    return pIndicator;
}




void TitleBar::PaintPanelControlTitle (void)
{
    int nWidth (GetOutputSizePixel().Width());
    Rectangle aTextBox (CalculateTextBoundingBox (nWidth, true));
    PaintBackground(CalculateTitleBarBox(aTextBox, nWidth));
    Rectangle aFocusBox (PaintExpansionIndicator (aTextBox));
    PaintText (aTextBox);
    aFocusBox.Union (aTextBox);
    aFocusBox.Left() += 2;
    PaintFocusIndicator (aFocusBox);
    PaintMouseOverIndicator (aTextBox);
}




void TitleBar::PaintWindowTitleBar (void)
{
    Rectangle aTextBox (CalculateTextBoundingBox (
        GetOutputSizePixel().Width(),
        true));

    PaintText (aTextBox);
    PaintFocusIndicator (aTextBox);
}




void TitleBar::PaintSubPanelHeadLineBar (void)
{
    int nWidth (GetOutputSizePixel().Width());
    Rectangle aTextBox (CalculateTextBoundingBox (nWidth, true));

    Rectangle aTitleBarBox (CalculateTitleBarBox(aTextBox, nWidth));
    int nVerticalOffset = -aTitleBarBox.Top();
    aTitleBarBox.Top() += nVerticalOffset;
    aTitleBarBox.Bottom() += nVerticalOffset;
    aTextBox.Top() += nVerticalOffset;
    aTextBox.Bottom() += nVerticalOffset;

    PaintBackground (aTitleBarBox);
    Rectangle aFocusBox (PaintExpansionIndicator (aTextBox));
    PaintText (aTextBox);

    aFocusBox.Union (aTextBox);
    aFocusBox.Left() -= 2;
    aFocusBox.Right() += 1;
    PaintFocusIndicator (aFocusBox);
    PaintMouseOverIndicator (aTextBox);
}




void TitleBar::PaintFocusIndicator (const Rectangle& rTextBox)
{
    if (mbFocused)
    {
        Rectangle aTextPixelBox (mpDevice->LogicToPixel (rTextBox));
        mpDevice->EnableMapMode (FALSE);
        Rectangle aBox (rTextBox);
        aBox.Top() -= 1;
        aBox.Bottom() += 1;

        mpDevice->SetFillColor ();

        //        mpDevice->SetLineColor(COL_WHITE);
        mpDevice->DrawRect (aTextPixelBox);

        LineInfo aDottedStyle (LINE_DASH);
        aDottedStyle.SetDashCount (0);
        aDottedStyle.SetDotCount (1);
        aDottedStyle.SetDotLen (1);
        aDottedStyle.SetDistance (1);

        mpDevice->SetLineColor (COL_BLACK);
        mpDevice->DrawPolyLine (Polygon(aTextPixelBox), aDottedStyle);
        mpDevice->EnableMapMode (FALSE);
    }
    else
        HideFocus ();
}




void TitleBar::PaintMouseOverIndicator (const Rectangle& rTextBox)
{
    if (mbMouseOver)
    {
        Rectangle aBox (rTextBox);
        // Show the line below the focus rectangle (which is painted
        // after and over the mouse over indicator.)
        //        aBox.Bottom() += 2;
        //        DrawLine (aBox.BottomLeft(), aBox.BottomRight());
    }
}




Rectangle TitleBar::PaintExpansionIndicator (const Rectangle& rTextBox)
{
    const Image* pImage = GetExpansionIndicator();
    if (pImage != NULL)
    {
        Point aPosition (
            0,
            rTextBox.Top()
            + (GetTextHeight() - pImage->GetSizePixel().Height()) / 2);
        if (meType == TBT_SUB_CONTROL_HEADLINE)
            aPosition.X() += 3;
        mpDevice->DrawImage (aPosition, *pImage);

        return Rectangle (aPosition, pImage->GetSizePixel());
    }
    else
        return Rectangle (Point(0,0), Size(0,0));
}




void TitleBar::PaintText (const Rectangle& rTextBox)
{
    mpDevice->DrawText (rTextBox, msTitle, GetTextStyle());
}




USHORT TitleBar::GetTextStyle (void)
{
    return TEXT_DRAW_LEFT
        | TEXT_DRAW_TOP
        | TEXT_DRAW_MULTILINE
        | TEXT_DRAW_WORDBREAK;
}




void TitleBar::PaintBackground (const Rectangle& rTitleBarBox)
{
    // Fill a slightly rounded rectangle.
    Color aFillColor (GetFillColor());
    Color aLineColor (GetLineColor());

    switch (meType)
    {
        case TBT_CONTROL_TITLE:
        {
            mpDevice->SetFillColor (
                GetSettings().GetStyleSettings().GetDialogColor());
            mpDevice->DrawRect(rTitleBarBox);

            mpDevice->SetFillColor();
            mpDevice->SetLineColor (
                GetSettings().GetStyleSettings().GetLightColor());
            mpDevice->DrawLine(
                rTitleBarBox.TopLeft(),rTitleBarBox.TopRight());
            mpDevice->DrawLine(
                rTitleBarBox.TopLeft(),rTitleBarBox.BottomLeft());

            mpDevice->SetLineColor (
                GetSettings().GetStyleSettings().GetShadowColor());
            mpDevice-> DrawLine(
                rTitleBarBox.BottomLeft(), rTitleBarBox.BottomRight());
            mpDevice->DrawLine(
                rTitleBarBox.TopRight(), rTitleBarBox.BottomRight());
        }
        break;

        case TBT_SUB_CONTROL_HEADLINE:
        {
            Color aColor (GetSettings().GetStyleSettings().GetDialogColor());
            if (mbExpanded)
            {
                // Make the color a little bit darker.
                aColor.SetRed(((UINT16)aColor.GetRed()) * 8 / 10);
                aColor.SetGreen(((UINT16)aColor.GetGreen()) * 8 / 10);
                aColor.SetBlue(((UINT16)aColor.GetBlue()) * 8 / 10);
            }

            mpDevice->SetFillColor (aColor);
            mpDevice->SetLineColor ();
            mpDevice->DrawRect (rTitleBarBox);

            // Erase the four corner pixels to make the rectangle appear
            // rounded.
            mpDevice->SetLineColor (
                GetSettings().GetStyleSettings().GetWindowColor());
            mpDevice->DrawPixel (
                rTitleBarBox.TopLeft());
            mpDevice->DrawPixel (
                Point(rTitleBarBox.Right(), rTitleBarBox.Top()));
            mpDevice->DrawPixel (
                Point(rTitleBarBox.Left(), rTitleBarBox.Bottom()));
            mpDevice->DrawPixel (
                Point(rTitleBarBox.Right(), rTitleBarBox.Bottom()));
        }
        break;

        default:
        case TBT_WINDOW_TITLE:
            break;
    }
}




Rectangle TitleBar::CalculateTextBoundingBox (
    int nAvailableWidth,
    bool bEmphasizeExpanded)
{
    // Show the title of expanded controls in bold font.
    const Font& rOriginalFont (GetFont());
    Font aFont (rOriginalFont);
    if (bEmphasizeExpanded && mbExpanded)
        aFont.SetWeight (WEIGHT_BOLD);
    else
        aFont.SetWeight (WEIGHT_NORMAL);
    mpDevice->SetFont (aFont);

    // Use the natural width of the text when no width is given.
    if (nAvailableWidth == 0)
        nAvailableWidth = GetTextWidth (msTitle);

    Rectangle aTextBox (
        Point(0,0),
        Size (nAvailableWidth,
            Application::GetSettings().GetStyleSettings().GetTitleHeight()));
    aTextBox.Top() += (aTextBox.GetHeight() - GetTextHeight()) / 2;
    if (GetExpansionIndicator() != NULL)
        aTextBox.Left() += snIndentationWidth;
    else
        aTextBox.Left() += 3;
    aTextBox.Right() -= 1;

   aTextBox = mpDevice->GetTextRect (aTextBox, msTitle, GetTextStyle());

   return aTextBox;
}




Rectangle TitleBar::CalculateTitleBarBox (
    const Rectangle& rTextBox,
    int nWidth)
{
    Rectangle aTitleBarBox (rTextBox);

   switch (meType)
   {
       case TBT_WINDOW_TITLE:
           aTitleBarBox.Bottom() += aTitleBarBox.Top();
           aTitleBarBox.Top() = 0;
           break;

        case TBT_CONTROL_TITLE:
           aTitleBarBox.Bottom() += aTitleBarBox.Top();
           aTitleBarBox.Top() = 0;
           break;

        case TBT_SUB_CONTROL_HEADLINE:
           aTitleBarBox.Top() -= 3;
           aTitleBarBox.Bottom() += 3;
           break;

   }
    aTitleBarBox.Left() = 0;
    if (aTitleBarBox.GetWidth() < nWidth)
        aTitleBarBox.Right() = nWidth-1;

    return aTitleBarBox;
}




void TitleBar::MouseMove (const MouseEvent& rEvent)
{
    Point aRelativePosition = rEvent.GetPosPixel() - GetPosPixel();
    Size aSize = GetSizePixel();
    SetMouseOver (
        aRelativePosition.X() >= 0
        && aRelativePosition.Y() >= 0
        && aRelativePosition.X() < aSize.Width()
        && aRelativePosition.Y() < aSize.Height());
}




void TitleBar::MouseButtonDown (const MouseEvent& rMEvt)
{
    Window::MouseButtonDown (rMEvt);
}




void TitleBar::InitializeImages (void)
{
    if ( ! sbImagesInitialized)
    {
        saTriangleRight = Image(BitmapEx(SdResId (BMP_TRIANGLE_RIGHT)));
        saTriangleRightHC = Image(BitmapEx(SdResId (BMP_TRIANGLE_RIGHT_H)));
        saTriangleDown = Image(BitmapEx(SdResId (BMP_TRIANGLE_DOWN)));
        saTriangleDownHC = Image(BitmapEx(SdResId (BMP_TRIANGLE_DOWN_H)));
        saExpanded = Image(BitmapEx(SdResId(BMP_COLLAPSE)));
        saCollapsed = Image(BitmapEx(SdResId(BMP_EXPAND)));
        saExpandedHC = Image(BitmapEx(SdResId(BMP_COLLAPSE_H)));
        saCollapsedHC = Image(BitmapEx(SdResId(BMP_EXPAND_H)));
        sbImagesInitialized = true;
    }
}


} } // end of namespace ::sd::toolpanel
