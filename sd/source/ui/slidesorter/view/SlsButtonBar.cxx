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
#include "model/SlideSorterModel.hxx"
#include "view/SlsTheme.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsToolTip.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsSlotManager.hxx"
#include "controller/SlsCurrentSlideManager.hxx"
#include "controller/SlsPageSelector.hxx"
#include "controller/SlsAnimator.hxx"
#include "controller/SlsAnimationFunction.hxx"
#include "app.hrc"
#include "drawdoc.hxx"
#include <svx/svxids.hrc>
#include <sfx2/dispatch.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/virdev.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <com/sun/star/presentation/XPresentation2.hpp>
#include <boost/bind.hpp>

using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::presentation::XPresentation2;

namespace sd { namespace slidesorter { namespace view {

/** Base class for the painter of the background bar onto which the buttons
    are painted.  It also provides some size information.
*/
class ButtonBar::BackgroundTheme
{
public:
    BackgroundTheme(
        const ::boost::shared_ptr<Theme>& rpTheme,
        const ::std::vector<SharedButton>& rButtons);
    /** Set the preview bounding box, the maximal area in which to display
        buttons.  A call to this method triggers a call to Layout().
    */
    void SetPreviewBoundingBox (const Rectangle& rPreviewBoundingBox);
    Button::IconSize GetIconSize (void) const;

    virtual BitmapEx CreateBackground (
        const OutputDevice& rTemplateDevice,
        const bool bIsButtonDown) const = 0;
    virtual Point GetBackgroundLocation (void) = 0;
    virtual Rectangle GetButtonArea (void) = 0;

protected:
    ::boost::shared_ptr<Theme> mpTheme;
    Rectangle maPreviewBoundingBox;
    Size maMinimumLargeButtonAreaSize;
    Size maMinimumMediumButtonAreaSize;
    Size maMinimumSmallButtonAreaSize;
    Button::IconSize meIconSize;

    virtual void Layout (void) = 0;

private:
    void UpdateMinimumIconSizes(const ::std::vector<SharedButton>& rButtons);
};


namespace {
    /** Rectangular button bar that covers the whole width of the preview.
    */
    class RectangleBackgroundTheme : public ButtonBar::BackgroundTheme
    {
    public:
        RectangleBackgroundTheme(
            const ::boost::shared_ptr<Theme>& rpTheme,
            const ::std::vector<SharedButton>& rButtons);
        virtual BitmapEx CreateBackground (
            const OutputDevice& rTemplateDevice,
            const bool bIsButtonDown) const;
        virtual Point GetBackgroundLocation (void);
        virtual Rectangle GetButtonArea (void);
    protected:
        virtual void Layout (void);
    private:
        sal_Int32 mnBarHeight;
    };

    /** Button bar is composed of three images, the left and right end of
        the bar and the center image.  Buttons are only placed over the
        center image.  The center image is painted as is, it is not scaled.
    */
    class BitmapBackgroundTheme : public ButtonBar::BackgroundTheme
    {
    public:
        BitmapBackgroundTheme(
            const ::boost::shared_ptr<Theme>& rpTheme,
            const ::std::vector<SharedButton>& rButtons);
        virtual BitmapEx CreateBackground (
            const OutputDevice& rTemplateDevice,
            const bool bIsButtonDown) const;
        virtual Point GetBackgroundLocation (void);
        virtual Rectangle GetButtonArea (void);
    protected:
        virtual void Layout (void);
    private:
        Rectangle maButtonArea;
        Point maBackgroundLocation;
    };

    /** The source mask is essentially multiplied with the given alpha value.
        The result is writen to the result mask.
    */
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
                    const sal_uInt8 nValue (255 - pSourceBitmap->GetPixel(nY, nX).GetBlueOrIndex());
                    const sal_uInt8 nNewValue (nValue * (1-nAlpha));
                    pBitmap->SetPixel(nY, nX, 255-nNewValue);
                }
        }
    }

} // end of anonymous namespace


//===== ButtonBar::Lock =======================================================

ButtonBar::Lock::Lock (SlideSorter& rSlideSorter)
    : mrButtonBar(rSlideSorter.GetView().GetButtonBar())
{
    mrButtonBar.AcquireLock();
}




ButtonBar::Lock::~Lock (void)
{
    mrButtonBar.ReleaseLock();
}




//===== ButtonBar =============================================================

ButtonBar::ButtonBar (SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter),
      maPageObjectSize(0,0),
      maButtonBoundingBox(),
      maBackgroundLocation(),
      mpDescriptor(),
      mbIsExcluded(false),
      mpButtonUnderMouse(),
      mpDownButton(),
      maRegularButtons(),
      maExcludedButtons(),
      maNormalBackground(),
      maButtonDownBackground(),
      mbIsMouseOverBar(false),
      mpBackgroundTheme(),
      mnLockCount(0)
{
    HandleDataChangeEvent();
}




ButtonBar::~ButtonBar (void)
{
}




void ButtonBar::ProcessButtonDownEvent (
    const model::SharedPageDescriptor& rpDescriptor,
    const Point aMouseModelLocation)
{
    SetButtonUnderMouse(GetButtonAt(aMouseModelLocation));
    if (mpButtonUnderMouse)
        mpButtonUnderMouse->SetState(Button::State_Down);
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
        mpButtonUnderMouse->SetState(Button::State_Hover);
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

        if (mpButtonUnderMouse)
        {
            // When the mouse button is down, mark the button under the
            // mouse only as pressed when it is the same button the mouse
            // button was pressed over, and where the button release would
            // lead to a click action.
            if (bIsMouseButtonDown)
            {
                if (mpButtonUnderMouse==mpDownButton)
                    bButtonStateHasChanged = mpButtonUnderMouse->SetState(Button::State_Down);
            }
            else
                bButtonStateHasChanged = mpButtonUnderMouse->SetState(Button::State_Hover);
        }
    }

    // Show a quick help text when the mouse is over a button.
    if (bButtonHasChanged)
    {
        SharedSdWindow pWindow (mrSlideSorter.GetContentWindow());
        if (pWindow)
        {
            if (mpButtonUnderMouse)
                mrSlideSorter.GetView().GetToolTip().ShowHelpText(mpButtonUnderMouse->GetHelpText());
            else
                mrSlideSorter.GetView().GetToolTip().ShowDefaultHelpText();
        }
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
            mbIsExcluded = mpDescriptor->HasState(model::PageDescriptor::ST_Excluded);
        else
            mbIsExcluded = false;
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
        {
            if (rButtons[sal_uInt32(nIndex)]->GetBoundingBox().IsInside(aLocalLocation))
            {
                if (rButtons[sal_uInt32(nIndex)]->IsEnabled())
                    return rButtons[sal_uInt32(nIndex)];
                else
                    return SharedButton();
            }
        }
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
            mpButtonUnderMouse->SetState(Button::State_Normal);

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
    if ( ! rpDescriptor)
        return;

    const double nButtonBarAlpha (rpDescriptor->GetVisualState().GetButtonBarAlpha());
    if (nButtonBarAlpha >= 1)
        return;

    LayoutButtons(rpDescriptor->GetBoundingBox().GetSize());

    const Point aOffset (rpDescriptor->GetBoundingBox().TopLeft());

    // Paint the background.
    PaintButtonBackground(rDevice, rpDescriptor, aOffset);

    // Paint the buttons.
    const ::std::vector<SharedButton>& rButtons (
        rpDescriptor->HasState(model::PageDescriptor::ST_Excluded)
            ? maExcludedButtons
            : maRegularButtons);


    const double nButtonAlpha (rpDescriptor->GetVisualState().GetButtonAlpha());
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
    const model::SharedPageDescriptor& rpDescriptor,
    const Point aOffset)
{
    BitmapEx* pBitmap = NULL;
    if (maButtonDownBackground.IsEmpty() || maNormalBackground.IsEmpty())
    {
        if (mpBackgroundTheme)
        {
            maButtonDownBackground = mpBackgroundTheme->CreateBackground(rDevice, true);
            maNormalBackground = mpBackgroundTheme->CreateBackground(rDevice, false);
        }
    }
    if (mpButtonUnderMouse && mpButtonUnderMouse->IsDown())
        pBitmap = &maButtonDownBackground;
    else
        pBitmap = &maNormalBackground;
    if (pBitmap != NULL)
    {
        AlphaMask aMask (pBitmap->GetSizePixel());
        AdaptTransparency(
            aMask,
            pBitmap->GetAlpha(),
            rpDescriptor->GetVisualState().GetButtonBarAlpha());
        rDevice.DrawBitmapEx(maBackgroundLocation+aOffset, BitmapEx(pBitmap->GetBitmap(), aMask));
    }
}




bool ButtonBar::IsMouseOverBar (const Point aModelLocation) const
{
    if ( ! mpDescriptor || ! mpDescriptor->GetBoundingBox().IsInside(aModelLocation))
        return false;

    if ( ! maButtonBoundingBox.IsInside(aModelLocation - mpDescriptor->GetBoundingBox().TopLeft()))
        return false;

    return true;
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

        if (mpBackgroundTheme)
        {
            mpBackgroundTheme->SetPreviewBoundingBox(
                mrSlideSorter.GetView().GetLayouter().GetPageObjectLayouter()->GetBoundingBox(
                    Point(0,0),
                    PageObjectLayouter::Preview,
                    PageObjectLayouter::ModelCoordinateSystem));
            LayoutButtons();
        }

        // Release the background bitmaps so that on the next paint
        // they are created anew in the right size.
        maNormalBackground.SetEmpty();
        maButtonDownBackground.SetEmpty();
    }
}




bool ButtonBar::LayoutButtons (void)
{
    const sal_Int32 nGap (mrSlideSorter.GetTheme()->GetIntegerValue(Theme::Integer_ButtonGap));
    const sal_Int32 nBorder (mrSlideSorter.GetTheme()->GetIntegerValue(Theme::Integer_ButtonBorder));

    const Button::IconSize eIconSize (mpBackgroundTheme->GetIconSize());

    // Tell buttons which size they are.
    for (sal_uInt32 nIndex=0; nIndex<maExcludedButtons.size(); ++nIndex)
        maExcludedButtons[nIndex]->SetIconSize(eIconSize);
    for (sal_uInt32 nIndex=0; nIndex<maRegularButtons.size(); ++nIndex)
        maRegularButtons[nIndex]->SetIconSize(eIconSize);

    // Determine maximal height and total width of the buttons.
    // Start with the buttons used for the excluded state.
    sal_Int32 nMaximumHeight (0);
    sal_Int32 nExcludedTotalWidth ((maExcludedButtons.size()-1) * nGap + 2*nBorder);
    for (sal_uInt32 nIndex=0; nIndex<maExcludedButtons.size(); ++nIndex)
    {
        const Size aSize (maExcludedButtons[nIndex]->GetSize());
        if (aSize.Height() > nMaximumHeight)
            nMaximumHeight = aSize.Height();
        nExcludedTotalWidth += aSize.Width();
    }

    // Do the same for the regular buttons.
    sal_Int32 nRegularTotalWidth ((maRegularButtons.size()-1) * nGap + 2*nBorder);
    for (sal_uInt32 nIndex=0; nIndex<maRegularButtons.size(); ++nIndex)
    {
        const Size aSize (maRegularButtons[nIndex]->GetSize());
        if (aSize.Height() > nMaximumHeight)
            nMaximumHeight = aSize.Height();
        nRegularTotalWidth += aSize.Width();
    }
    nMaximumHeight += 2*nBorder;

    // Set up the bounding box of the button bar.
    maButtonBoundingBox = mpBackgroundTheme->GetButtonArea();
    maBackgroundLocation = mpBackgroundTheme->GetBackgroundLocation();
    if (mrSlideSorter.GetTheme()->GetIntegerValue(Theme::Integer_ButtonPaintType) == 1)
    {
        // Center the buttons.
        maButtonBoundingBox.Left() += (maButtonBoundingBox.GetWidth() - nRegularTotalWidth)/2;
        maButtonBoundingBox.Right() = maButtonBoundingBox.Left() + nRegularTotalWidth - 1;
    }

    // Place the buttons.
    Rectangle aBox (maButtonBoundingBox);
    aBox.Right() -= nBorder;
    for (sal_Int32 nIndex=maRegularButtons.size()-1; nIndex>=0; --nIndex)
    {
        maRegularButtons[nIndex]->Place(aBox);
        aBox.Right() = maRegularButtons[nIndex]->GetBoundingBox().Left() - nGap;
    }

    // For slides excluded from the show there is only one icon placed
    // exactly like the second of the regular icons.
    if (maRegularButtons.size()>=2 && maExcludedButtons.size()>=1)
    {
        aBox = maRegularButtons[1]->GetBoundingBox();
        maExcludedButtons[0]->Place(aBox);
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




void ButtonBar::RequestFadeIn (
    const model::SharedPageDescriptor& rpDescriptor,
    const bool bAnimate)
{
    if ( ! rpDescriptor)
        return;
    if (mnLockCount > 0)
        return;

    const double nMinAlpha (0);
    if ( ! bAnimate)
    {
        rpDescriptor->GetVisualState().SetButtonAlpha(nMinAlpha);
        rpDescriptor->GetVisualState().SetButtonBarAlpha(nMinAlpha);
    }
    else
        StartFadeAnimation(rpDescriptor, nMinAlpha, true);
}




void ButtonBar::RequestFadeOut (
    const model::SharedPageDescriptor& rpDescriptor,
    const bool bAnimate)
{
    if ( ! rpDescriptor)
        return;
    if (mnLockCount > 0)
        return;

    const double nMaxAlpha (1);
    if ( ! bAnimate)
    {
        rpDescriptor->GetVisualState().SetButtonAlpha(nMaxAlpha);
        rpDescriptor->GetVisualState().SetButtonBarAlpha(nMaxAlpha);
    }
    else
        StartFadeAnimation(rpDescriptor, nMaxAlpha, false);
}




bool ButtonBar::IsVisible (const model::SharedPageDescriptor& rpDescriptor)
{
    const double nMaxAlpha (1);
    return rpDescriptor && rpDescriptor->GetVisualState().GetButtonBarAlpha() < nMaxAlpha;
}




void ButtonBar::HandleDataChangeEvent (void)
{
    maExcludedButtons.clear();
    maExcludedButtons.push_back(::boost::shared_ptr<Button>(new UnhideButton(mrSlideSorter)));

    maRegularButtons.clear();
    maRegularButtons.push_back(::boost::shared_ptr<Button>(new StartShowButton(mrSlideSorter)));
    maRegularButtons.push_back(::boost::shared_ptr<Button>(new HideButton(mrSlideSorter)));
    maRegularButtons.push_back(::boost::shared_ptr<Button>(new DuplicateButton(mrSlideSorter)));

    mpBackgroundTheme.reset(
        new BitmapBackgroundTheme(
            mrSlideSorter.GetTheme(),
            maRegularButtons));

    // Force layout on next Paint().
    maPageObjectSize = Size(0,0);
}




void ButtonBar::StartFadeAnimation (
    const model::SharedPageDescriptor& rpDescriptor,
    const double nTargetAlpha,
    const bool bFadeIn)
{
    model::SharedPageDescriptor pDescriptor (rpDescriptor);

    const double nCurrentButtonAlpha (pDescriptor->GetVisualState().GetButtonAlpha());
    const double nCurrentButtonBarAlpha (pDescriptor->GetVisualState().GetButtonBarAlpha());

    // Stop a running animation.
    const controller::Animator::AnimationId nId (
        pDescriptor->GetVisualState().GetButtonAlphaAnimationId());
    if (nId != controller::Animator::NotAnAnimationId)
        mrSlideSorter.GetController().GetAnimator()->RemoveAnimation(nId);

    // Prepare the blending functors that translate [0,1] animation
    // times into alpha values of buttons and button bar.
    const ::boost::function<double(double)> aButtonBlendFunctor (
        ::boost::bind(
            controller::AnimationFunction::Blend,
            nCurrentButtonAlpha,
            nTargetAlpha,
            ::boost::bind(controller::AnimationFunction::Linear, _1)));
    const ::boost::function<double(double)> aButtonBarBlendFunctor (
        ::boost::bind(
            controller::AnimationFunction::Blend,
            nCurrentButtonBarAlpha,
            nTargetAlpha,
            ::boost::bind(controller::AnimationFunction::Linear, _1)));

    // Delay the fade in a little bit when the buttons are not visible at
    // all so that we do not leave a trail of half-visible buttons when the
    // mouse is moved across the screen.  No delay on fade out or when the
    // buttons are already showing.  Fade out is faster than fade in.
    const double nDelay (nCurrentButtonBarAlpha>0 && nCurrentButtonBarAlpha<1
        ? 0
        : (mrSlideSorter.GetTheme()->GetIntegerValue(bFadeIn
            ?  Theme::Integer_ButtonFadeInDelay
            :  Theme::Integer_ButtonFadeOutDelay)));
    const double nDuration (mrSlideSorter.GetTheme()->GetIntegerValue(bFadeIn
            ?  Theme::Integer_ButtonFadeInDuration
            :  Theme::Integer_ButtonFadeOutDuration));
    pDescriptor->GetVisualState().SetButtonAlphaAnimationId(
        mrSlideSorter.GetController().GetAnimator()->AddAnimation(
            ::boost::bind(
                controller::AnimationFunction::ApplyButtonAlphaChange,
                pDescriptor,
                ::boost::ref(mrSlideSorter.GetView()),
                ::boost::bind(aButtonBlendFunctor, _1),
                ::boost::bind(aButtonBarBlendFunctor, _1)),
            nDelay,
            nDuration,
            ::boost::bind(
                &model::VisualState::SetButtonAlphaAnimationId,
                ::boost::ref(pDescriptor->GetVisualState()),
                controller::Animator::NotAnAnimationId)
            ));
}




void ButtonBar::AcquireLock (void)
{
    if (mnLockCount == 0 && mpDescriptor)
        RequestFadeOut(mpDescriptor, true);

    ++mnLockCount;
}




void ButtonBar::ReleaseLock (void)
{
    --mnLockCount;

    if (mnLockCount == 0 && mpDescriptor)
        RequestFadeIn(mpDescriptor, true);
}




//===== BackgroundTheme =====================================================

ButtonBar::BackgroundTheme::BackgroundTheme (
    const ::boost::shared_ptr<Theme>& rpTheme,
    const ::std::vector<SharedButton>& rButtons)
    : mpTheme(rpTheme)
{
    UpdateMinimumIconSizes(rButtons);
}




void ButtonBar::BackgroundTheme::SetPreviewBoundingBox (const Rectangle& rPreviewBoundingBox)
{
    maPreviewBoundingBox = rPreviewBoundingBox;
    Layout();
}




void ButtonBar::BackgroundTheme::UpdateMinimumIconSizes (
    const ::std::vector<SharedButton>& rButtons)
{
    OSL_ASSERT(mpTheme);

    sal_Int32 nMaximumHeightLarge (0);
    sal_Int32 nMaximumHeightMedium (0);
    sal_Int32 nMaximumHeightSmall (0);
    const sal_Int32 nGap (mpTheme->GetIntegerValue(Theme::Integer_ButtonGap));
    const sal_Int32 nBorder (mpTheme->GetIntegerValue(Theme::Integer_ButtonBorder));
    sal_Int32 nTotalWidthLarge ((rButtons.size()-1) * nGap + 2*nBorder);
    sal_Int32 nTotalWidthMedium ((rButtons.size()-1) * nGap + 2*nBorder);
    sal_Int32 nTotalWidthSmall ((rButtons.size()-1) * nGap + 2*nBorder);
    for (sal_uInt32 nIndex=0; nIndex<rButtons.size(); ++nIndex)
    {
        // Update large size.
        Size aSize = rButtons[nIndex]->GetSize(Button::IconSize_Large);
        if (aSize.Height() > nMaximumHeightLarge)
            nMaximumHeightLarge = aSize.Height();
        nTotalWidthLarge += aSize.Width();

        // Update medium size.
        aSize = rButtons[nIndex]->GetSize(Button::IconSize_Medium);
        if (aSize.Height() > nMaximumHeightMedium)
            nMaximumHeightMedium = aSize.Height();
        nTotalWidthMedium += aSize.Width();

        // Update small size.
        aSize = rButtons[nIndex]->GetSize(Button::IconSize_Small);
        if (aSize.Height() > nMaximumHeightSmall)
            nMaximumHeightSmall = aSize.Height();
        nTotalWidthSmall += aSize.Width();
    }
    maMinimumLargeButtonAreaSize = Size(nTotalWidthLarge, nMaximumHeightLarge+2*nBorder);
    maMinimumMediumButtonAreaSize = Size(nTotalWidthMedium, nMaximumHeightMedium+2*nBorder);
    maMinimumSmallButtonAreaSize = Size(nTotalWidthSmall, nMaximumHeightSmall+2*nBorder);
}




Button::IconSize ButtonBar::BackgroundTheme::GetIconSize (void) const
{
    return meIconSize;
}




//===== RectangleBackgroundTheme ============================================

RectangleBackgroundTheme::RectangleBackgroundTheme (
    const ::boost::shared_ptr<Theme>& rpTheme,
    const ::std::vector<SharedButton>& rButtons)
    : BackgroundTheme(rpTheme, rButtons),
      mnBarHeight(0)
{
}




BitmapEx RectangleBackgroundTheme::CreateBackground (
    const OutputDevice& rTemplateDevice,
    const bool bIsButtonDown) const
{
    OSL_ASSERT(mpTheme);

    // Setup background color.
    Color aTopFillColor (mpTheme->GetGradientColor(
        Theme::Gradient_ButtonBackground,
        Theme::Fill1));
    Color aTopBorderColor (mpTheme->GetGradientColor(
        Theme::Gradient_ButtonBackground,
        Theme::Border1));
    Color aBottomFillColor (mpTheme->GetGradientColor(
        Theme::Gradient_ButtonBackground,
        Theme::Fill2));
    Color aBottomBorderColor (mpTheme->GetGradientColor(
        Theme::Gradient_ButtonBackground,
        Theme::Border2));
    if (bIsButtonDown)
    {
        aTopFillColor.DecreaseLuminance(50);
        aTopBorderColor.DecreaseLuminance(50);
        aBottomFillColor.DecreaseLuminance(50);
        aBottomBorderColor.DecreaseLuminance(50);
    }

    const int nWidth (maPreviewBoundingBox.GetWidth()+2);
    const int nHeight (mnBarHeight);
    const int nCenter (nHeight / 2);

    VirtualDevice aDevice (rTemplateDevice, 0, 8);
    aDevice.SetOutputSizePixel(Size(nWidth,nHeight));

    // Fill upper and lower half.
    aDevice.SetLineColor();
    aDevice.SetFillColor(aTopFillColor);
    aDevice.DrawRect(Rectangle(0,0,nWidth-1,nCenter));
    aDevice.SetFillColor(aBottomFillColor);
    aDevice.DrawRect(Rectangle(0,nCenter,nWidth-1,nHeight-1));

    // Draw border.
    aDevice.SetFillColor();
    aDevice.SetLineColor(aTopBorderColor);
    aDevice.DrawLine(Point(0,nCenter),Point(0,0));
    aDevice.DrawLine(Point(0,0), Point(nWidth-1,0));
    aDevice.DrawLine(Point(nWidth-1,0),Point(nWidth-1,nCenter));
    aDevice.SetLineColor(aBottomBorderColor);
    aDevice.DrawLine(Point(0,nCenter),Point(0,nHeight-1));
    aDevice.DrawLine(Point(0,nHeight-1), Point(nWidth-1,nHeight-1));
    aDevice.DrawLine(Point(nWidth-1,nHeight-1),Point(nWidth-1,nCenter));

    return aDevice.GetBitmapEx(Point(0,0), Size(nWidth,nHeight));
}




Point RectangleBackgroundTheme::GetBackgroundLocation (void)
{
    return Point(
        maPreviewBoundingBox.Left()-1,
        maPreviewBoundingBox.Bottom() - mnBarHeight + 2);
}




Rectangle RectangleBackgroundTheme::GetButtonArea (void)
{
    return Rectangle(
        maPreviewBoundingBox.Left(),
        maPreviewBoundingBox.Bottom() - mnBarHeight + 2,
        maPreviewBoundingBox.Right(),
        maPreviewBoundingBox.Bottom());
}




void RectangleBackgroundTheme::Layout (void)
{
    if (maPreviewBoundingBox.GetWidth() < maMinimumLargeButtonAreaSize.Width())
        if (maPreviewBoundingBox.GetWidth() < maMinimumMediumButtonAreaSize.Width())
        {
            meIconSize = Button::IconSize_Small;
            mnBarHeight = maMinimumSmallButtonAreaSize.Height();
        }
        else
        {
            meIconSize = Button::IconSize_Medium;
            mnBarHeight = maMinimumMediumButtonAreaSize.Height();
        }
    else
    {
        meIconSize = Button::IconSize_Large;
        mnBarHeight = maMinimumLargeButtonAreaSize.Height();
    }
}




//===== BitmapBackgroundTheme =================================================

BitmapBackgroundTheme::BitmapBackgroundTheme (
    const ::boost::shared_ptr<Theme>& rpTheme,
    const ::std::vector<SharedButton>& rButtons)
    : BackgroundTheme(rpTheme, rButtons),
      maButtonArea(),
      maBackgroundLocation()
{
}




BitmapEx BitmapBackgroundTheme::CreateBackground (
    const OutputDevice& rTemplateDevice,
    const bool bIsButtonDown) const
{
    (void)rTemplateDevice;
    (void)bIsButtonDown;

    OSL_ASSERT(mpTheme);

    // Get images.
    switch (meIconSize)
    {
        case Button::IconSize_Large:
        default:
            return mpTheme->GetIcon(Theme::Icon_ButtonBarLarge);

        case Button::IconSize_Medium:
            return mpTheme->GetIcon(Theme::Icon_ButtonBarMedium);

        case Button::IconSize_Small:
            return mpTheme->GetIcon(Theme::Icon_ButtonBarSmall);
    }
}




Point BitmapBackgroundTheme::GetBackgroundLocation (void)
{
    return maBackgroundLocation;
}




Rectangle BitmapBackgroundTheme::GetButtonArea (void)
{
    return maButtonArea;
}




void BitmapBackgroundTheme::Layout (void)
{
    Size aImageSize (mpTheme->GetIcon(Theme::Icon_ButtonBarLarge).GetSizePixel());
    if (aImageSize.Width() >= maPreviewBoundingBox.GetWidth())
    {
        aImageSize = mpTheme->GetIcon(Theme::Icon_ButtonBarMedium).GetSizePixel();
        if (aImageSize.Width() >= maPreviewBoundingBox.GetWidth())
        {
            meIconSize = Button::IconSize_Small;
            aImageSize = mpTheme->GetIcon(Theme::Icon_ButtonBarSmall).GetSizePixel();
        }
        else
            meIconSize = Button::IconSize_Medium;
    }
    else
    {
        meIconSize = Button::IconSize_Large;
    }

    maBackgroundLocation = Point(
        maPreviewBoundingBox.Left()
            + (maPreviewBoundingBox.GetWidth()-aImageSize.Width())/2,
        maPreviewBoundingBox.Bottom() - aImageSize.Height());
    maButtonArea = Rectangle(maBackgroundLocation, aImageSize);
}




//===== Button ================================================================

Button::Button (
    SlideSorter& rSlideSorter,
    const ::rtl::OUString& rsHelpText)
    : mrSlideSorter(rSlideSorter),
      meState(State_Normal),
      maBoundingBox(),
      msHelpText(rsHelpText),
      mbIsActive(false),
      meIconSize(IconSize_Large)
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
    return mbIsActive && meState==State_Down;
}




void Button::SetActiveState (const bool bIsActive)
{
    mbIsActive = bIsActive;
}




bool Button::IsActive (void) const
{
    return mbIsActive;
}




void Button::SetIconSize (const IconSize eIconSize)
{
    meIconSize = eIconSize;
}




Button::IconSize Button::GetIconSize (void) const
{
    return meIconSize;
}




bool Button::IsEnabled (void) const
{
    return true;
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




void TextButton::Place (const Rectangle aButtonBarBox)
{
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
        if (meState == State_Normal)
            rDevice.SetTextColor(rpTheme->GetColor(Theme::Color_ButtonText));
        else
            rDevice.SetTextColor(rpTheme->GetColor(Theme::Color_ButtonTextHover));
        Rectangle aBox (maBoundingBox);
        aBox += aOffset;
        rDevice.DrawText(aBox, msText, TEXT_DRAW_CENTER | TEXT_DRAW_VCENTER);
    }
}




Size TextButton::GetSize (void) const
{
    return Size();
}




Size TextButton::GetSize (const Button::IconSize) const
{
    return Size();
}




//===== ImageButon ============================================================

ImageButton::ImageButton (
    SlideSorter& rSlideSorter,
    const BitmapEx& rLargeIcon,
    const BitmapEx& rLargeHoverIcon,
    const BitmapEx& rMediumIcon,
    const BitmapEx& rMediumHoverIcon,
    const BitmapEx& rSmallIcon,
    const BitmapEx& rSmallHoverIcon,
    const ::rtl::OUString& rsHelpText)
    : Button(rSlideSorter, rsHelpText),
      maLargeIcon(rLargeIcon),
      maLargeHoverIcon(rLargeHoverIcon.IsEmpty() ? rLargeIcon : rLargeHoverIcon),
      maMediumIcon(rMediumIcon),
      maMediumHoverIcon(rMediumHoverIcon.IsEmpty() ? rMediumIcon : rMediumHoverIcon),
      maSmallIcon(rSmallIcon),
      maSmallHoverIcon(rSmallHoverIcon.IsEmpty() ? rSmallIcon : rSmallHoverIcon)
{
}




void ImageButton::Place (const Rectangle aButtonBarBox)
{
    const sal_Int32 nWidth (GetSize().Width());
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

    const sal_uInt16 nSavedAntialiasingMode (rDevice.GetAntialiasing());
    rDevice.SetAntialiasing(nSavedAntialiasingMode | ANTIALIASING_ENABLE_B2DDRAW);

    rDevice.SetLineColor();

    // Choose icon.
    BitmapEx aIcon;
    switch (meIconSize)
    {
        case IconSize_Large:
        default:
            if (meState == State_Normal)
                aIcon = maLargeIcon;
            else
                aIcon = maLargeHoverIcon;
            break;

        case IconSize_Medium:
            if (meState == State_Normal)
                aIcon = maMediumIcon;
            else
                aIcon = maMediumHoverIcon;
            break;

        case IconSize_Small:
            if (meState == State_Normal)
                aIcon = maSmallIcon;
            else
                aIcon = maSmallHoverIcon;
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
    return GetSize(meIconSize);
}




Size ImageButton::GetSize (const Button::IconSize eIconSize) const
{
    switch (eIconSize)
    {
        case IconSize_Large:
        default:
            return maLargeIcon.GetSizePixel();

        case IconSize_Medium:
            return maMediumIcon.GetSizePixel();

        case IconSize_Small:
            return maSmallIcon.GetSizePixel();
    }
}




//===== UnhideButton ==========================================================

UnhideButton::UnhideButton (SlideSorter& rSlideSorter)
    : ImageButton(
        rSlideSorter,
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command2BLarge),
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command2BLargeHover),
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command2BMedium),
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command2BMediumHover),
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command2BSmall),
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command2BSmallHover),
        rSlideSorter.GetTheme()->GetString(Theme::String_Command2B))
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
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command1Large),
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command1LargeHover),
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command1Medium),
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command1MediumHover),
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command1Small),
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command1SmallHover),
        rSlideSorter.GetTheme()->GetString(Theme::String_Command1))
{
}




bool StartShowButton::IsEnabled (void) const
{
    ViewShell* pViewShell = mrSlideSorter.GetViewShell();
    if (pViewShell == NULL)
        return false;
    SfxDispatcher* pDispatcher = pViewShell->GetDispatcher();
    if (pDispatcher == NULL)
        return false;

    const SfxPoolItem* pState = NULL;
    const SfxItemState eState (pDispatcher->QueryState(SID_PRESENTATION, pState));
    return (eState & SFX_ITEM_DISABLED) == 0;
}




void StartShowButton::ProcessClick (const model::SharedPageDescriptor& rpDescriptor)
{
    // Hide the tool tip early, while the slide show still intializes.
    mrSlideSorter.GetView().GetToolTip().SetPage(model::SharedPageDescriptor());

    Reference< XPresentation2 > xPresentation(
        mrSlideSorter.GetModel().GetDocument()->getPresentation());
    if (xPresentation.is())
    {
        Sequence<PropertyValue> aProperties (1);
        aProperties[0].Name = ::rtl::OUString::createFromAscii("FirstPage");
        const ::rtl::OUString sName (rpDescriptor->GetPage()->GetName());
        aProperties[0].Value = Any(sName);
        xPresentation->startWithArguments(aProperties);
    }
}




//===== HideButton ============================================================

HideButton::HideButton (SlideSorter& rSlideSorter)
    : ImageButton(
        rSlideSorter,
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command2Large),
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command2LargeHover),
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command2Medium),
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command2MediumHover),
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command2Small),
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command2SmallHover),
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
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command3Large),
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command3LargeHover),
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command3Medium),
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command3MediumHover),
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command3Small),
        rSlideSorter.GetTheme()->GetIcon(Theme::Icon_Command3SmallHover),
        rSlideSorter.GetTheme()->GetString(Theme::String_Command3))
{
}




bool DuplicateButton::IsEnabled (void) const
{
    ViewShell* pViewShell = mrSlideSorter.GetViewShell();
    if (pViewShell == NULL)
        return false;
    SfxDispatcher* pDispatcher = pViewShell->GetDispatcher();
    if (pDispatcher == NULL)
        return false;

    const SfxPoolItem* pState = NULL;
    const SfxItemState eState (pDispatcher->QueryState(
        SID_DUPLICATE_PAGE,
        pState));
    return (eState & SFX_ITEM_DISABLED) == 0;
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
    if (mrSlideSorter.GetViewShell() != NULL
        && mrSlideSorter.GetViewShell()->GetDispatcher() != NULL)
    {
        mrSlideSorter.GetViewShell()->GetDispatcher()->Execute(
            SID_DUPLICATE_PAGE,
            SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
    }
}



} } } // end of namespace ::sd::slidesorter::view
