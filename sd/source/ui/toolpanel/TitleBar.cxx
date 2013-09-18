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


#include "taskpane/TitleBar.hxx"

#include "ControlContainerDescriptor.hxx"
#include "tools/IconCache.hxx"
#include "AccessibleTreeNode.hxx"
#include <vcl/decoview.hxx>
#include <vcl/window.hxx>
#include <vcl/virdev.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include "sdresid.hxx"
#include <vcl/bitmap.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/bitmapex.hxx>
#include <tools/color.hxx>
#include <svx/xdash.hxx>
#include <svl/itemset.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpool.hxx>
#include <svtools/colorcfg.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xfillit0.hxx>
#include "res_bmp.hrc"


namespace sd { namespace toolpanel {

const int TitleBar::snIndentationWidth = 16;

TitleBar::TitleBar ( ::Window* pParent, const OUString& rsTitle, TitleBarType eType, bool bIsExpandable)
: ::Window (pParent, WB_TABSTOP)
, TreeNode(this)
, meType(eType)
, msTitle(rsTitle)
, mbExpanded(false)
, mbFocused(false)
, mpDevice(new VirtualDevice (*this))
, mbIsExpandable (bIsExpandable)
{
    EnableMapMode (sal_False);

    SetBackground (Wallpaper());

    // Change the mouse pointer shape so that it acts as a mouse over effect.
    switch (meType)
    {
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




sal_Int32 TitleBar::GetPreferredWidth (sal_Int32 )
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
    mpDevice->SetSettings(GetSettings());
    mpDevice->SetDrawMode(GetDrawMode());

    switch (meType)
    {
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




bool TitleBar::Expand (bool bFlag)
{
    bool bExpansionStateChanged (bFlag!=IsExpanded());
    mbExpanded = bFlag;
    Invalidate ();
    return bExpansionStateChanged;
}




bool TitleBar::IsExpanded (void) const
{
    return mbExpanded;
}


void TitleBar::SetEnabledState(bool bFlag)
{
    if(bFlag)
        Enable();
    else
        Disable();
    Invalidate ();
}




void TitleBar::GetFocus()
{
    mbFocused = true;
    Invalidate();
}




void TitleBar::LoseFocus()
{
    mbFocused = false;
    Invalidate();
}




bool TitleBar::HasExpansionIndicator (void) const
{
    bool bHasExpansionIndicator (false);
    if (mbIsExpandable)
    {
        switch (meType)
        {
            case TBT_SUB_CONTROL_HEADLINE:
                bHasExpansionIndicator = true;
                break;
        }
    }
    return bHasExpansionIndicator;
}




Image TitleBar::GetExpansionIndicator (void) const
{
    Image aIndicator;
    if (mbIsExpandable)
    {
        sal_uInt16 nResourceId = 0;
        switch (meType)
        {
            case TBT_SUB_CONTROL_HEADLINE:
                if (mbExpanded)
                    nResourceId = BMP_COLLAPSE;
                else
                    nResourceId = BMP_EXPAND;

                aIndicator = IconCache::Instance().GetIcon(nResourceId);
                break;
        }
    }

    return aIndicator;
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
}




void TitleBar::PaintFocusIndicator (const Rectangle& rTextBox)
{
    if (mbFocused)
    {
        Rectangle aTextPixelBox (mpDevice->LogicToPixel (rTextBox));
        mpDevice->EnableMapMode (sal_False);
        Rectangle aBox (rTextBox);
        aBox.Top() -= 1;
        aBox.Bottom() += 1;

        mpDevice->SetFillColor ();

        mpDevice->DrawRect (aTextPixelBox);

        LineInfo aDottedStyle (LINE_DASH);
        aDottedStyle.SetDashCount (0);
        aDottedStyle.SetDotCount (1);
        aDottedStyle.SetDotLen (1);
        aDottedStyle.SetDistance (1);

        mpDevice->SetLineColor (COL_BLACK);
        mpDevice->DrawPolyLine (Polygon(aTextPixelBox), aDottedStyle);
        mpDevice->EnableMapMode (sal_False);
    }
    else
        HideFocus ();
}




Rectangle TitleBar::PaintExpansionIndicator (const Rectangle& rTextBox)
{
    Rectangle aExpansionIndicatorArea;

    if (HasExpansionIndicator())
    {
        Image aImage = GetExpansionIndicator();
        int nHeight (aImage.GetSizePixel().Height());
        if (nHeight > 0)
        {
            Point aPosition (
                0,
                rTextBox.Top() + (GetTextHeight() - nHeight) / 2);
            if (meType == TBT_SUB_CONTROL_HEADLINE)
                aPosition.X() += 3;
            mpDevice->DrawImage (aPosition, aImage);

            aExpansionIndicatorArea = Rectangle (
                aPosition, aImage.GetSizePixel());
        }
    }

    return aExpansionIndicatorArea;
}




void TitleBar::PaintText (const Rectangle& rTextBox)
{
    mpDevice->DrawText (rTextBox, msTitle, GetTextStyle());
}




sal_uInt16 TitleBar::GetTextStyle (void)
{
     if(IsEnabled())
     {
         return TEXT_DRAW_LEFT
             | TEXT_DRAW_TOP
             | TEXT_DRAW_MULTILINE
             | TEXT_DRAW_WORDBREAK;
     }
     else
     {
         return TEXT_DRAW_DISABLE;
     }
}



void TitleBar::PaintBackground (const Rectangle& rTitleBarBox)
{
    // Fill a slightly rounded rectangle.
    switch (meType)
    {
        case TBT_SUB_CONTROL_HEADLINE:
        {
            Color aColor (GetSettings().GetStyleSettings().GetDialogColor());
            if (mbExpanded)
            {
                // Make the color a little bit darker.
                aColor.SetRed(sal_uInt8(((sal_uInt16)aColor.GetRed()) * 8 / 10));
                aColor.SetGreen(sal_uInt8(((sal_uInt16)aColor.GetGreen()) * 8 / 10));
                aColor.SetBlue(sal_uInt8(((sal_uInt16)aColor.GetBlue()) * 8 / 10));
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
            GetSettings().GetStyleSettings().GetTitleHeight()));
    aTextBox.Top() += (aTextBox.GetHeight() - GetTextHeight()) / 2;
    if (HasExpansionIndicator())
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




void TitleBar::MouseMove (const MouseEvent& )
{
}




void TitleBar::MouseButtonDown (const MouseEvent& )
{
    // Do not forward to parent window so that the mouse button handler of
    // the docking window is not invoked.
}




void TitleBar::MouseButtonUp (const MouseEvent& )
{
    // Do not forward to parent window so that the mouse button handler of
    // the docking window is not invoked.
}




void TitleBar::DataChanged (const DataChangedEvent& rEvent)
{
    ::Window::DataChanged (rEvent);

    switch (rEvent.GetType())
    {
        case DATACHANGED_SETTINGS:
            if ((rEvent.GetFlags() & SETTINGS_STYLE) == 0)
                break;
            SetSettings(Application::GetSettings());
            mpDevice.reset(new VirtualDevice (*this));

            // fall through.

        case DATACHANGED_FONTS:
        case DATACHANGED_FONTSUBSTITUTION:
        {
            const StyleSettings& rStyleSettings (GetSettings().GetStyleSettings());

            // Font.
            Font aFont = rStyleSettings.GetAppFont();
            if (IsControlFont())
                aFont.Merge(GetControlFont());
            SetZoomedPointFont(aFont);

            // Color.
            Color aColor;
            if (IsControlForeground())
                aColor = GetControlForeground();
            else
                aColor = rStyleSettings.GetButtonTextColor();
            SetTextColor(aColor);
            SetTextFillColor();

            Resize();
            Invalidate();
        }
        break;
    }
}




::com::sun::star::uno::Reference<
    ::com::sun::star::accessibility::XAccessible > TitleBar::CreateAccessibleObject (
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>& )
{
    return new ::accessibility::AccessibleTreeNode(
        *this,
        GetTitle(),
        GetTitle(),
        ::com::sun::star::accessibility::AccessibleRole::LABEL);
}


} } // end of namespace ::sd::toolpanel

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
