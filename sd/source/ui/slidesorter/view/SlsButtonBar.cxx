/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "precompiled_sd.hxx"

#include "view/SlsButtonBar.hxx"

#include "SlideSorter.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "view/SlsTheme.hxx"
#include "view/SlideSorterView.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsSlotManager.hxx"
#include "controller/SlsCurrentSlideManager.hxx"
#include "controller/SlsPageSelector.hxx"
#include "app.hrc"
#include <svx/svxids.hrc>
#include <sfx2/dispatch.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/virdev.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

namespace sd { namespace slidesorter { namespace view {

namespace {
    void AdaptTransparency (AlphaMask& rMask, const AlphaMask& rSourceMask, const double nAlpha)
    {
        BitmapWriteAccess* pBitmap = rMask.AcquireWriteAccess();
        const BitmapReadAccess* pSourceBitmap = const_cast<AlphaMask&>(rSourceMask).AcquireReadAccess();

        if (pBitmap!=NULL && pSourceBitmap!=NULL)
        {
            const sal_Int32 nWidth (pBitmap->Width());
            const sal_Int32 nHeight (pBitmap->Height());

            for (sal_Int32 nY = 0; nY<nHeight; ++nY)
                for (sal_Int32 nX = 0; nX<nWidth; ++nX)
                {
                    const BYTE nValue (255 - pSourceBitmap->GetPixel(nY, nX).GetBlueOrIndex());
                    const BYTE nNewValue (nValue * (1-nAlpha));
                    pBitmap->SetPixel(nY, nX, 255-nNewValue);
                }
        }
    }
}




//===== ButtonBar =============================================================

ButtonBar::ButtonBar (SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter),
      maPageObjectSize(0,0),
      maBoundingBox(),
      mpDescriptor(),
      mbIsExcluded(false),
      mpButtonUnderMouse(),
      mpDownButton(),
      maRegularButtons(),
      maExcludedButtons(),
      maNormalBackground(),
      maButtonDownBackground(),
      mbIsMouseOverBar(false)
{
    maExcludedButtons.push_back(::boost::shared_ptr<Button>(new UnhideButton(mrSlideSorter)));

    maRegularButtons.push_back(::boost::shared_ptr<Button>(new StartShowButton(mrSlideSorter)));
    maRegularButtons.push_back(::boost::shared_ptr<Button>(new HideButton(mrSlideSorter)));
    maRegularButtons.push_back(::boost::shared_ptr<Button>(new DuplicateButton(mrSlideSorter)));
}




void ButtonBar::ProcessButtonDownEvent (
    const model::SharedPageDescriptor& rpDescriptor,
    const Point aMouseModelLocation)
{
    SetButtonUnderMouse(GetButtonAt(aMouseModelLocation));
    if (mpButtonUnderMouse)
        mpButtonUnderMouse->SetState(Button::ST_Down);
    mpDownButton = mpButtonUnderMouse;

    mrSlideSorter.GetView().RequestRepaint(rpDescriptor);
}




void ButtonBar::ProcessButtonUpEvent (
    const model::SharedPageDescriptor& rpDescriptor,
    const Point aMouseModelLocation)
{
    SetButtonUnderMouse(GetButtonAt(aMouseModelLocation));
    if (mpButtonUnderMouse)
    {
        mpButtonUnderMouse->SetState(Button::ST_Hover);
        if (mpButtonUnderMouse == mpDownButton)
        {
            // This is done only when the buttons are sufficiently visible.
            if (mpDescriptor->GetVisualState().GetButtonAlpha()<0.7)
            {
                mpButtonUnderMouse->ProcessClick(mpDescriptor);
                mbIsExcluded = mpDescriptor->HasState(model::PageDescriptor::ST_Excluded);
                ProcessMouseMotionEvent (rpDescriptor, aMouseModelLocation, false);
            }
        }
    }
    mpDownButton.reset();
    mrSlideSorter.GetView().RequestRepaint(rpDescriptor);
}




void ButtonBar::ProcessMouseMotionEvent (
    const model::SharedPageDescriptor& rpDescriptor,
    const Point aMouseModelLocation,
    const bool bIsMouseButtonDown)
{
    model::SharedPageDescriptor pOldDescriptor (mpDescriptor);
    bool bPageHasChanged (false);
    bool bButtonHasChanged (false);
    bool bButtonStateHasChanged (false);

    // Update the page object for which to manage the buttons.
    bPageHasChanged = SetPage(rpDescriptor);
    mbIsMouseOverBar = IsMouseOverBar(aMouseModelLocation);

    // Update button under mouse.
    if (rpDescriptor)
    {
        bButtonHasChanged = SetButtonUnderMouse(GetButtonAt(aMouseModelLocation));

        // Update the button state.
        if (mpButtonUnderMouse)
        {
            // When the mouse button is down, mark the button under
            // the mouse only as pressed when it is the same button
            // the mouse button was pressed over, and where the button
            // release would lead to a click action.
            if (bIsMouseButtonDown)
            {
                if (mpButtonUnderMouse==mpDownButton)
                    bButtonStateHasChanged = mpButtonUnderMouse->SetState(Button::ST_Down);
            }
            else
                bButtonStateHasChanged = mpButtonUnderMouse->SetState(Button::ST_Hover);
        }
    }

    // Show a quick help text when the mouse is over a button.
    if (bButtonHasChanged)
    {
        SharedSdWindow pWindow (mrSlideSorter.GetContentWindow());
        if (pWindow)
            if (mpButtonUnderMouse)
                mrSlideSorter.GetView().SetHelpText(mpButtonUnderMouse->GetHelpText(), false);
            else
                mrSlideSorter.GetView().SetHelpText(mrSlideSorter.GetView().GetDefaultHelpText(),true);
    }

    if (bPageHasChanged || bButtonHasChanged || bButtonStateHasChanged)
    {
        if (pOldDescriptor)
            mrSlideSorter.GetView().RequestRepaint(pOldDescriptor);
        if (mpDescriptor && pOldDescriptor!=mpDescriptor)
            mrSlideSorter.GetView().RequestRepaint(mpDescriptor);
    }
}




void ButtonBar::ResetPage (void)
{
    SetPage(model::SharedPageDescriptor());
}




bool ButtonBar::SetPage (const model::SharedPageDescriptor& rpDescriptor)
{
    if (mpDescriptor != rpDescriptor)
    {
        mpDescriptor = rpDescriptor;

        if (mpDescriptor)
        {
            mbIsExcluded = mpDescriptor->HasState(model::PageDescriptor::ST_Excluded);

            LayoutButtons(rpDescriptor->GetBoundingBox().GetSize());
        }
        else
        {
            mbIsExcluded = false;
        }
        SetButtonUnderMouse();
        mpDownButton.reset();

        return true;
    }
    else
        return false;
}




sal_Int32 ButtonBar::GetButtonCount (const bool bIsExcluded) const
{
    if (bIsExcluded)
        return maExcludedButtons.size();
    else
        return maRegularButtons.size();
}




::boost::shared_ptr<Button> ButtonBar::GetButton (
    const bool bIsExcluded,
    const sal_Int32 nIndex) const
{
    const ::std::vector<boost::shared_ptr<Button> >& rButtons (bIsExcluded
        ? maExcludedButtons
        : maRegularButtons);

    if (nIndex<0 || sal_uInt32(nIndex)>=rButtons.size())
    {
        OSL_ASSERT(nIndex<0 || sal_uInt32(nIndex)>=rButtons.size());
        return ::boost::shared_ptr<Button>();
    }
    else
        return rButtons[sal_uInt32(nIndex)];
}




SharedButton ButtonBar::GetButtonAt (const Point aModelLocation)
{
    if (IsMouseOverBar(aModelLocation))
    {
        const Point aLocalLocation (aModelLocation - mpDescriptor->GetBoundingBox().TopLeft());
        ::std::vector<SharedButton>& rButtons (
            mbIsExcluded ? maExcludedButtons : maRegularButtons);
        for (sal_uInt32 nIndex=0; nIndex<rButtons.size(); ++nIndex)
            if (rButtons[sal_uInt32(nIndex)]->GetBoundingBox().IsInside(aLocalLocation))
                return rButtons[sal_uInt32(nIndex)];
    }

    return SharedButton();
}




bool ButtonBar::IsMouseOverBar (void) const
{
    return mbIsMouseOverBar;
}




bool ButtonBar::SetButtonUnderMouse (const SharedButton& rButton)
{
    if (mpButtonUnderMouse != rButton)
    {
        if (mpButtonUnderMouse)
            mpButtonUnderMouse->SetState(Button::ST_Normal);

        mpButtonUnderMouse = rButton;

        return true;
    }
    else
        return false;
}




void ButtonBar::Paint (
    OutputDevice& rDevice,
    const model::SharedPageDescriptor& rpDescriptor)
{
    if ( ! mpDescriptor || mpDescriptor!=rpDescriptor)
        return;

    const double nButtonAlpha (mpDescriptor->GetVisualState().GetButtonAlpha());
    if (nButtonAlpha >= 1)
        return;

    const Point aOffset (rpDescriptor->GetBoundingBox().TopLeft());

    // Paint the background.
    PaintButtonBackground(rDevice, aOffset);

    // Paint the buttons.
    const ::std::vector<SharedButton>& rButtons (
        rpDescriptor->HasState(model::PageDescriptor::ST_Excluded)
            ? maExcludedButtons
            : maRegularButtons);


    for (sal_uInt32 nIndex=0; nIndex<rButtons.size(); ++nIndex)
        rButtons[nIndex]->Paint(
            rDevice,
            aOffset,
            nButtonAlpha,
            mrSlideSorter.GetTheme());
}




bool ButtonBar::IsMouseOverButton (void) const
{
    return mpButtonUnderMouse;
}




void ButtonBar::PaintButtonBackground (
    OutputDevice& rDevice,
    const Point aOffset)
{
    Bitmap* pBitmap = NULL;
    if (mpButtonUnderMouse && mpButtonUnderMouse->IsDown())
    {
        if (maButtonDownBackground.IsEmpty())
            maButtonDownBackground = CreateBackground(rDevice, maBoundingBox.GetSize(), true);
        pBitmap = &maButtonDownBackground;
    }
    else
    {
        if (maNormalBackground.IsEmpty())
            maNormalBackground = CreateBackground(rDevice, maBoundingBox.GetSize(), false);
        pBitmap = &maNormalBackground;
    }

    if (pBitmap != NULL)
    {
        BYTE aTransparency (mpDescriptor->GetVisualState().GetButtonBarAlpha()*255);
        AlphaMask aMask (pBitmap->GetSizePixel(), &aTransparency);
        rDevice.DrawBitmapEx(maBoundingBox.TopLeft()+aOffset, BitmapEx(*pBitmap, aMask));
    }
}




Bitmap ButtonBar::CreateBackground (
    const OutputDevice& rTemplateDevice,
    const Size aSize,
    const bool bIsButtonDown) const
{
    VirtualDevice aDevice (rTemplateDevice);
    aDevice.SetOutputSizePixel(aSize);

    // Setup background color.
    Color aTopFillColor (mrSlideSorter.GetTheme()->GetGradientColor(
        Theme::Gradient_ButtonBackground,
        Theme::Fill1));
    Color aTopBorderColor (mrSlideSorter.GetTheme()->GetGradientColor(
        Theme::Gradient_ButtonBackground,
        Theme::Border1));
    Color aBottomFillColor (mrSlideSorter.GetTheme()->GetGradientColor(
        Theme::Gradient_ButtonBackground,
        Theme::Fill2));
    Color aBottomBorderColor (mrSlideSorter.GetTheme()->GetGradientColor(
        Theme::Gradient_ButtonBackground,
        Theme::Border2));
    if (bIsButtonDown)
    {
        aTopFillColor.DecreaseLuminance(50);
        aTopBorderColor.DecreaseLuminance(50);
        aBottomFillColor.DecreaseLuminance(50);
        aBottomBorderColor.DecreaseLuminance(50);
    }

    const int nCenter (aSize.Height() / 2);

    // Fill upper and lower half.
    aDevice.SetLineColor();
    aDevice.SetFillColor(aTopFillColor);
    aDevice.DrawRect(Rectangle(0,0,aSize.Width()-1,nCenter));
    aDevice.SetFillColor(aBottomFillColor);
    aDevice.DrawRect(Rectangle(0,nCenter,aSize.Width()-1,aSize.Height()-1));

    // Draw border.
    aDevice.SetFillColor();
    aDevice.SetLineColor(aTopBorderColor);
    aDevice.DrawLine(Point(0,nCenter),Point(0,0));
    aDevice.DrawLine(Point(0,0), Point(aSize.Width()-1,0));
    aDevice.DrawLine(Point(aSize.Width()-1,0),Point(aSize.Width()-1,nCenter));
    aDevice.SetLineColor(aBottomBorderColor);
    aDevice.DrawLine(Point(0,nCenter),Point(0,aSize.Height()-1));
    aDevice.DrawLine(Point(0,aSize.Height()-1), Point(aSize.Width()-1,aSize.Height()-1));
    aDevice.DrawLine(Point(aSize.Width()-1,aSize.Height()-1),Point(aSize.Width()-1,nCenter));

    return aDevice.GetBitmap(Point(0,0), aSize);
}




bool ButtonBar::IsMouseOverBar (const Point aModelLocation) const
{
    if ( ! mpDescriptor || ! mpDescriptor->GetBoundingBox().IsInside(aModelLocation))
        return false;

    return maBoundingBox.IsInside(aModelLocation - mpDescriptor->GetBoundingBox().TopLeft());
}




void ButtonBar::RequestLayout (void)
{
    maPageObjectSize = Size(0,0);
}




void ButtonBar::LayoutButtons (const Size aPageObjectSize)
{
    if (maPageObjectSize != aPageObjectSize)
    {
        maPageObjectSize = aPageObjectSize;

        if ( ! LayoutButtons(aPageObjectSize, false))
            LayoutButtons(aPageObjectSize, true);

        // Release the background bitmaps so that on the next paint
        // they are created anew in the right size.
        maNormalBackground.SetEmpty();
        maButtonDownBackground.SetEmpty();
    }
}




bool ButtonBar::LayoutButtons (
    const Size aPageObjectSize,
    const bool bIsSmall)
{
    (void)aPageObjectSize;

    // Tell buttons which size they are.
    for (sal_uInt32 nIndex=0; nIndex<maExcludedButtons.size(); ++nIndex)
        maExcludedButtons[nIndex]->SetIsSmall(bIsSmall);
    for (sal_uInt32 nIndex=0; nIndex<maRegularButtons.size(); ++nIndex)
        maRegularButtons[nIndex]->SetIsSmall(bIsSmall);

    // Determine maximal height of the buttons.
    // Start with the buttons used for the excluded state.
    sal_Int32 nMaximumHeight (0);
    for (sal_uInt32 nIndex=0; nIndex<maExcludedButtons.size(); ++nIndex)
    {
        const Size aSize (maExcludedButtons[nIndex]->GetSize());
        if (aSize.Height() > nMaximumHeight)
            nMaximumHeight = aSize.Height();
    }

    // Do the same for the regular buttons.
    for (sal_uInt32 nIndex=0; nIndex<maRegularButtons.size(); ++nIndex)
    {
        const Size aSize (maRegularButtons[nIndex]->GetSize());
        if (aSize.Height() > nMaximumHeight)
            nMaximumHeight = aSize.Height();
    }
    nMaximumHeight += 2*mrSlideSorter.GetTheme()->GetIntegerValue(Theme::Integer_ButtonBorder);

    // Set up the bounding box of the button bar.
    maBoundingBox
        = mrSlideSorter.GetView().GetLayouter().GetPageObjectLayouter()->GetBoundingBox(
            Point(0,0),
            PageObjectLayouter::Preview,
            PageObjectLayouter::ModelCoordinateSystem);

    maBoundingBox.Top() = maBoundingBox.Bottom()- nMaximumHeight;
    maBoundingBox.Left() -= 1;
    maBoundingBox.Right() += 1;
    maBoundingBox.Bottom() += 1;

    // Place the buttons.
    Rectangle aBox (maBoundingBox);
    const sal_Int32 nGap (mrSlideSorter.GetTheme()->GetIntegerValue(Theme::Integer_ButtonGap));
    if (maExcludedButtons.size() > 1)
        aBox.Right() -= mrSlideSorter.GetTheme()->GetIntegerValue(Theme::Integer_ButtonBorder);
    for (sal_Int32 nIndex=maExcludedButtons.size()-1; nIndex>=0; --nIndex)
    {
        maExcludedButtons[nIndex]->Place(aBox, maExcludedButtons.size()-1-nIndex);
        aBox.Right() = maExcludedButtons[nIndex]->GetBoundingBox().Left() - nGap;
    }
    aBox = maBoundingBox;
    aBox.Right() -= mrSlideSorter.GetTheme()->GetIntegerValue(Theme::Integer_ButtonBorder);
    for (sal_Int32 nIndex=maRegularButtons.size()-1; nIndex>=0; --nIndex)
    {
        maRegularButtons[nIndex]->Place(aBox, maRegularButtons.size()-1-nIndex);
        aBox.Right() = maRegularButtons[nIndex]->GetBoundingBox().Left() - nGap;
    }

    // We return true only when there is no inactive button.
    for (sal_uInt32 nIndex=0; nIndex<maExcludedButtons.size(); ++nIndex)
        if ( ! maExcludedButtons[nIndex]->IsActive())
            return false;
    for (sal_uInt32 nIndex=0; nIndex<maRegularButtons.size(); ++nIndex)
        if ( ! maRegularButtons[nIndex]->IsActive())
            return false;
    return true;
}




//===== Button ================================================================

Button::Button (
    SlideSorter& rSlideSorter,
    const ::rtl::OUString& rsHelpText)
    : mrSlideSorter(rSlideSorter),
      meState(ST_Normal),
      maBoundingBox(),
      msHelpText(rsHelpText),
      mbIsActive(false),
      mbIsSmall(false)
{
}




Button::~Button (void)
{
}




bool Button::SetState (const State eState)
{
    if (meState != eState)
    {
        meState = eState;
        return true;
    }
    else
        return false;
}




Button::State Button::GetState (void) const
{
    return meState;
}




Rectangle Button::GetBoundingBox (void) const
{
    if (mbIsActive)
        return maBoundingBox;
    else
        return Rectangle();
}




::rtl::OUString Button::GetHelpText (void) const
{
    if (mbIsActive)
        return msHelpText;
    else
        return ::rtl::OUString();
}




bool Button::IsDown (void) const
{
    return mbIsActive && meState==ST_Down;
}




void Button::SetActiveState (const bool bIsActive)
{
    mbIsActive = bIsActive;
}




bool Button::IsActive (void) const
{
    return mbIsActive;
}




void Button::SetIsSmall (const bool bIsSmall)
{
    mbIsSmall = bIsSmall;
}




//===== TextButton ============================================================

TextButton::TextButton (
    SlideSorter& rSlideSorter,
    const ::rtl::OUString& rsText,
    const ::rtl::OUString& rsHelpText)
    : Button(rSlideSorter, rsHelpText),
      msText(rsText)
{
}




void TextButton::Place (const Rectangle aButtonBarBox, const sal_Int32 nIndex)
{
    (void)nIndex;
    maBoundingBox = aButtonBarBox;
    SetActiveState(true);
}




void TextButton::Paint (
    OutputDevice& rDevice,
    const Point aOffset,
    const double nAlpha,
    const ::boost::shared_ptr<Theme>& rpTheme) const
{
    (void)nAlpha;

    if (mbIsActive)
    {
        // Paint text over the button background.
        rDevice.SetTextColor(rpTheme->GetColor(Theme::ButtonText));
        Rectangle aBox (maBoundingBox);
        aBox += aOffset;
        rDevice.DrawText(aBox, msText, TEXT_DRAW_CENTER | TEXT_DRAW_VCENTER);
    }
}




Size TextButton::GetSize (void) const
{
    return Size();
}




//===== ImageButon ============================================================

ImageButton::ImageButton (
    SlideSorter& rSlideSorter,
    const BitmapEx& rRegularIcon,
    const BitmapEx& rHoverIcon,
    const BitmapEx& rPressedIcon,
    const BitmapEx& rSmallIcon,
    const BitmapEx& rSmallHoverIcon,
    const BitmapEx& rSmallPressedIcon,
    const ::rtl::OUString& rsHelpText)
    : Button(rSlideSorter, rsHelpText),
      maNormalIcon(rRegularIcon),
      maHoverIcon(rHoverIcon.IsEmpty() ? rRegularIcon : rHoverIcon),
      maDownIcon(rPressedIcon.IsEmpty() ? rRegularIcon : rPressedIcon),
      maSmallIcon(rSmallIcon),
      maSmallHoverIcon(rSmallHoverIcon.IsEmpty() ? rSmallIcon : rSmallHoverIcon),
      maSmallDownIcon(rSmallPressedIcon.IsEmpty() ? rSmallIcon : rSmallPressedIcon)
{
}




void ImageButton::Place (
    const Rectangle aButtonBarBox,
    const sal_Int32 nIndex)
{
    (void)nIndex;
    const sal_Int32 nWidth (mbIsSmall
        ? maSmallIcon.GetSizePixel().Width()
        : maNormalIcon.GetSizePixel().Width());
    maBoundingBox = Rectangle(
        aButtonBarBox.Right() - nWidth,
        aButtonBarBox.Top(),
        aButtonBarBox.Right(),
        aButtonBarBox.Bottom());
    SetActiveState(aButtonBarBox.IsInside(maBoundingBox));
}




void ImageButton::Paint (
    OutputDevice& rDevice,
    const Point aOffset,
    const double nAlpha,
    const ::boost::shared_ptr<Theme>& rpTheme) const
{
    (void)rpTheme;

    if ( ! mbIsActive)
        return;

    const USHORT nSavedAntialiasingMode (rDevice.GetAntialiasing());
    rDevice.SetAntialiasing(nSavedAntialiasingMode | ANTIALIASING_ENABLE_B2DDRAW);

    rDevice.SetLineColor();

    // Choose icon.
    BitmapEx aIcon;
    switch (meState)
    {
        case ST_Normal:
            if (mbIsSmall)
                aIcon = maSmallIcon;
            else
                aIcon = maNormalIcon;
            break;

        case ST_Hover:
            if (mbIsSmall)
                aIcon = maSmallHoverIcon;
            else
                aIcon = maHoverIcon;
            break;

        case ST_Down:
            if (mbIsSmall)
                aIcon = maSmallDownIcon;
            else
                aIcon = maDownIcon;
            break;
    }

    // Paint icon.
    if ( ! aIcon.IsEmpty())
    {
        AlphaMask aMask (aIcon.GetSizePixel());
        AdaptTransparency(aMask, aIcon.GetAlpha(), nAlpha);
        rDevice.DrawBitmapEx(
            Point(
                maBoundingBox.Left()
                    + aOffset.X()
                    + (maBoundingBox.GetWidth()-aIcon.GetSizePixel().Width())/2,
                maBoundingBox.Top()
                    + aOffset.Y()
                    + (maBoundingBox.GetHeight()-aIcon.GetSizePixel().Height())/2),
            BitmapEx(aIcon.GetBitmap(), aMask));
    }

    rDevice.SetAntialiasing(nSavedAntialiasingMode);
}




Size ImageButton::GetSize (void) const
{
    if (mbIsSmall)
        return maSmallIcon.GetSizePixel();
    else
        return maNormalIcon.GetSizePixel();
}




//===== UnhideButton ==========================================================

UnhideButton::UnhideButton (SlideSorter& rSlideSorter)
    : TextButton(
        rSlideSorter,
        rSlideSorter.GetTheme()->GetString(Theme::String_Unhide),
        rSlideSorter.GetTheme()->GetString(Theme::String_Command2))
{
}




void UnhideButton::ProcessClick (const model::SharedPageDescriptor& rpDescriptor)
{
    if ( ! rpDescriptor)
        return;
    mrSlideSorter.GetController().GetSlotManager()->ChangeSlideExclusionState(
        (rpDescriptor->HasState(model::PageDescriptor::ST_Selected)
            ? model::SharedPageDescriptor()
            : rpDescriptor),
        false);
}




//===== StartSlideShowButton ==================================================

StartShowButton::StartShowButton (SlideSorter& rSlideSorter)
    : ImageButton(
        rSlideSorter,
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command1Regular),
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command1Hover),
        BitmapEx(),
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command1Small),
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command1SmallHover),
        BitmapEx(),
        rSlideSorter.GetTheme()->GetString(Theme::String_Command1))
{
}




void StartShowButton::ProcessClick (const model::SharedPageDescriptor& rpDescriptor)
{
    mrSlideSorter.GetController().GetCurrentSlideManager()->SwitchCurrentSlide(
        rpDescriptor);
    if (mrSlideSorter.GetViewShell() != NULL
        && mrSlideSorter.GetViewShell()->GetDispatcher() != NULL)
    {
        mrSlideSorter.GetViewShell()->GetDispatcher()->Execute(
            SID_PRESENTATION,
            SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
    }
}




//===== HideButton ============================================================

HideButton::HideButton (SlideSorter& rSlideSorter)
    : ImageButton(
        rSlideSorter,
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command2Regular),
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command2Hover),
        BitmapEx(),
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command2Small),
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command2SmallHover),
        BitmapEx(),
        rSlideSorter.GetTheme()->GetString(Theme::String_Command2))
{
}




void HideButton::ProcessClick (const model::SharedPageDescriptor& rpDescriptor)
{
    if ( ! rpDescriptor)
        return;
    mrSlideSorter.GetController().GetSlotManager()->ChangeSlideExclusionState(
        (rpDescriptor->HasState(model::PageDescriptor::ST_Selected)
            ? model::SharedPageDescriptor()
            : rpDescriptor),
        true);
}




//===== DuplicateButton =======================================================

DuplicateButton::DuplicateButton (SlideSorter& rSlideSorter)
    : ImageButton(
        rSlideSorter,
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command3Regular),
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command3Hover),
        BitmapEx(),
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command3Small),
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command3SmallHover),
        BitmapEx(),
        rSlideSorter.GetTheme()->GetString(Theme::String_Command3))
{
}




void DuplicateButton::ProcessClick (const model::SharedPageDescriptor& rpDescriptor)
{
    if ( ! rpDescriptor)
        return;

    mrSlideSorter.GetView().SetPageUnderMouse(model::SharedPageDescriptor(),false);

    // When the page under the button is not selected then set the
    // selection to just this page.
    if ( ! rpDescriptor->HasState(model::PageDescriptor::ST_Selected))
    {
        mrSlideSorter.GetController().GetPageSelector().DeselectAllPages();
        mrSlideSorter.GetController().GetPageSelector().SelectPage(rpDescriptor);
    }
    // Duplicate the selected pages.  Insert the new pages right
    // after the current selection and select them
    if (mrSlideSorter.GetViewShell() != NULL)
        mrSlideSorter.GetViewShell()->GetDispatcher()->Execute(
            SID_DUPLICATE_PAGE,
            SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
}





} } } // end of namespace ::sd::slidesorter::view
