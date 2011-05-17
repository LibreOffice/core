/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

#include "PresenterPaneAnimator.hxx"

#include "PresenterAnimation.hxx"
#include "PresenterAnimator.hxx"
#include "PresenterCanvasHelper.hxx"
#include "PresenterController.hxx"
#include "PresenterGeometryHelper.hxx"
#include "PresenterPaintManager.hxx"
#include "PresenterPaneContainer.hxx"
#include "PresenterPaneFactory.hxx"
#include "PresenterSprite.hxx"
#include "PresenterSpritePane.hxx"
#include "PresenterWindowManager.hxx"

#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <boost/bind.hpp>
#include <boost/bind/protect.hpp>
#include <boost/enable_shared_from_this.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;
using ::std::vector;

namespace sdext { namespace presenter {

namespace {

    class PaneGroup;

    class PresenterPaneAnimatorBase
        : public ::boost::enable_shared_from_this<PresenterPaneAnimatorBase>,
          public PresenterPaneAnimator
    {
    public:
        PresenterPaneAnimatorBase (
            const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
            const ::rtl::Reference<PresenterController>& rpPresenterController,
            const bool bAnimate,
            const EndActions& rShowEndActions,
            const EndActions& rEndEndActions);
        virtual ~PresenterPaneAnimatorBase (void);

        typedef ::std::vector< ::boost::function<void()> > EndOperators;

        void ActivatePanes (void);
        void ActivatePane (
            const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId);
        void RestoreFrozenWindows (void);
        void FreezePanes (const Reference<rendering::XSpriteCanvas>& rxCanvas);

    protected:
        ::rtl::Reference<PresenterController> mpPresenterController;
        ::rtl::Reference<PresenterPaneContainer> mpPaneContainer;
        ::rtl::Reference<PresenterWindowManager> mpWindowManager;
        ::std::vector< ::boost::shared_ptr<PaneGroup> > maPaneGroups;
        css::uno::Reference<css::drawing::framework::XResourceId> mxCenterPaneId;
        bool mbDoAnimation;
        EndActions maShowEndActions;
        EndActions maHideEndActions;

        void DeactivatePanes (void);
        void ResizePane (
            const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
            const geometry::RealRectangle2D& rBox);
        void DeactivatePane (
            const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId);
    };


    class UnfoldInCenterAnimator : public PresenterPaneAnimatorBase
    {
    public:
        UnfoldInCenterAnimator (
            const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
            const ::rtl::Reference<PresenterController>& rpPresenterController,
            const bool bAnimate,
            const EndActions& rShowEndActions,
            const EndActions& rEndEndActions);

        virtual ~UnfoldInCenterAnimator (void);

        virtual void ShowPane (void);

        virtual void HidePane (void);

    private:
        geometry::RealRectangle2D maCenterPaneBox;

        void SetupPaneGroups (void);
        geometry::RealRectangle2D MovePanesAway (
            const css::geometry::RealRectangle2D& rFreeCenterArea);
    };


    class MoveInFromBottomAnimator : public PresenterPaneAnimatorBase
    {
    public:
        MoveInFromBottomAnimator(
            const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
            const ::rtl::Reference<PresenterController>& rpPresenterController,
            const bool bAnimate,
            const EndActions& rShowEndActions,
            const EndActions& rEndEndActions);
        virtual ~MoveInFromBottomAnimator (void);

        virtual void ShowPane (void);
        virtual void HidePane (void);

    private:
        ::boost::shared_ptr<PresenterSprite> maNewPaneSprite;
        geometry::RealRectangle2D maCenterPaneBox;

        void CreateShowAnimation (
            const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
            const EndOperators& rpEndOperators,
            const css::uno::Reference<css::rendering::XSpriteCanvas>& rxSpriteCanvas,
            const bool bAnimate,
            const css::geometry::RealPoint2D& rStartLocation,
            const css::geometry::RealPoint2D& rEndLocation);
    };


    class TransparentOverlayAnimator : public PresenterPaneAnimatorBase
    {
    public:
        TransparentOverlayAnimator(
            const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
            const ::rtl::Reference<PresenterController>& rpPresenterController,
            const bool bAnimate,
            const EndActions& rShowEndActions,
            const EndActions& rEndEndActions);
        virtual ~TransparentOverlayAnimator (void);

        virtual void ShowPane (void);
        virtual void HidePane (void);

    private:
        PresenterSprite maBackgroundSprite;
        void CreateBackgroundSprite (void);
    };


    class PaneDescriptor
    {
    public:
        PresenterPaneContainer::SharedPaneDescriptor mpPaneDescriptor;

        PaneDescriptor (const PresenterPaneContainer::SharedPaneDescriptor& rpDescriptor);
        void Restore (void) const;
    private:
        double mnLeft;
        double mnTop;
        double mnRight;
        double mnBottom;
    };

    class MultiAnimation : public PresenterAnimation
    {
    public:
        typedef ::boost::function<void(double)> Animation;
        MultiAnimation (const sal_uInt32 nDuration);
        void AddAnimation (const Animation& rAnimation);
        virtual void Run (const double nProgress, const sal_uInt64 nCurrentTime);
    private:
        vector<Animation> maAnimations;
    };


    class PaneGroup
    {
    public:
        PaneGroup (void);
        ~PaneGroup (void);
        void AddPane (const PresenterPaneContainer::SharedPaneDescriptor& rpPane);
        void CreateSubstitution (const Reference<rendering::XSpriteCanvas>& rxCanvas);
        void ThawPanes (void);
        void Restore (void);
        ::boost::shared_ptr<PresenterSprite> GetSubstitution (void) const;
        css::geometry::RealRectangle2D GetOriginalBoundingBox (void) const;
        css::geometry::RealRectangle2D GetCurrentBoundingBox (void) const;
        void MovePanes (
            const double nXOffset,
            const double nYOffset,
            const ::rtl::Reference<PresenterWindowManager>& rpWindowManager);
        void ActivatePanes (void);
        void DeactivatePanes (void);
        void HidePanes (void);
        void ShowPanes (void);

    private:
        vector<PaneDescriptor> maPanes;
        awt::Rectangle maOriginalBoundingBox;
        css::geometry::RealRectangle2D maCurrentBoundingBox;
        ::boost::shared_ptr<PresenterSprite> mpSubstitution;

    };
    typedef ::boost::shared_ptr<PaneGroup> SharedPaneGroup;

    void InterpolatePosition (
        const ::boost::function<void(geometry::RealPoint2D)>& rSetter,
        double nP,
        const geometry::RealPoint2D rInitialBox,
        const geometry::RealPoint2D rFinalBox);

    template<typename T>
    void InterpolateValue (
        const ::boost::function<void(T)>& rSetter,
        double nP,
        const T aInitialValue,
        const T aFinalValue);

    void SpriteTransform(
        const rtl::Reference<PresenterPaneContainer>& rpPaneContainer,
        const Reference<XResourceId>& rxPaneId,
        const Reference<awt::XWindow>& rxSpriteOwnerWindow,
        const ::boost::shared_ptr<PresenterPaintManager>& rpPaintManager,
        const bool bAppear,
        const double nX,
        const double nInitialTop,
        const double nFinalTop,
        const double nP);

    void SpritePaneMove (
        const rtl::Reference<PresenterPaneContainer>& rpPaneContainer,
        const Reference<XResourceId>& rxPaneId,
        const geometry::RealPoint2D& rLocation);

    geometry::RealPoint2D GetLocation (const geometry::RealRectangle2D& rBox);
    geometry::RealSize2D GetSize (const geometry::RealRectangle2D& rBox);


} // end of anonymous namespace




//=============================================================================


::boost::shared_ptr<PresenterPaneAnimator> CreateUnfoldInCenterAnimator (
    const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
    const ::rtl::Reference<PresenterController>& rpPresenterController,
    const bool bAnimate,
    const EndActions& rShowEndActions,
    const EndActions& rEndEndActions)
{
    return ::boost::shared_ptr<PresenterPaneAnimator>(
        new UnfoldInCenterAnimator(rxPaneId, rpPresenterController, bAnimate,
            rShowEndActions, rEndEndActions));
}




::boost::shared_ptr<PresenterPaneAnimator> CreateMoveInFromBottomAnimator (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
        const ::rtl::Reference<PresenterController>& rpPresenterController,
        const bool bAnimate,
        const EndActions& rShowEndActions,
        const EndActions& rEndEndActions)
{
    return ::boost::shared_ptr<PresenterPaneAnimator>(
        new MoveInFromBottomAnimator(rxPaneId, rpPresenterController, bAnimate,
            rShowEndActions, rEndEndActions));
}




::boost::shared_ptr<PresenterPaneAnimator> CreateTransparentOverlay (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
        const ::rtl::Reference<PresenterController>& rpPresenterController,
        const bool bAnimate,
        const EndActions& rShowEndActions,
        const EndActions& rEndEndActions)
{
    return ::boost::shared_ptr<PresenterPaneAnimator>(
        new TransparentOverlayAnimator(rxPaneId, rpPresenterController, bAnimate,
            rShowEndActions, rEndEndActions));
}




//===== PresenterPaneAnimator =================================================

namespace {

PresenterPaneAnimatorBase::PresenterPaneAnimatorBase (
    const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
    const ::rtl::Reference<PresenterController>& rpPresenterController,
    const bool bAnimate,
    const EndActions& rShowEndActions,
    const EndActions& rHideEndActions)
    : mpPresenterController(rpPresenterController),
      mpPaneContainer(rpPresenterController->GetPaneContainer()),
      mpWindowManager(rpPresenterController->GetWindowManager()),
      maPaneGroups(),
      mxCenterPaneId(rxPaneId),
      mbDoAnimation(bAnimate),
      maShowEndActions(rShowEndActions),
      maHideEndActions(rHideEndActions)
{
}




PresenterPaneAnimatorBase::~PresenterPaneAnimatorBase (void)
{
}




void PresenterPaneAnimatorBase::FreezePanes (const Reference<rendering::XSpriteCanvas>& rxCanvas)
{
    ::std::vector<SharedPaneGroup>::const_iterator iGroup;
    for (iGroup=maPaneGroups.begin(); iGroup!=maPaneGroups.end(); ++iGroup)
    {
        (*iGroup)->CreateSubstitution(rxCanvas);
        (*iGroup)->GetSubstitution()->MoveTo(GetLocation((*iGroup)->GetOriginalBoundingBox()));
    }
}




void PresenterPaneAnimatorBase::ActivatePanes (void)
{
    ActivatePane(mxCenterPaneId);

    ::std::vector<SharedPaneGroup>::const_iterator iGroup;
    for (iGroup=maPaneGroups.begin(); iGroup!=maPaneGroups.end(); ++iGroup)
    {
        (*iGroup)->ShowPanes();
        (*iGroup)->ActivatePanes();
        (*iGroup)->GetSubstitution()->Hide();
    }

    mpWindowManager->Update();
}




void PresenterPaneAnimatorBase::DeactivatePanes (void)
{
    ::std::vector<SharedPaneGroup>::const_iterator iGroup;
    for (iGroup=maPaneGroups.begin(); iGroup!=maPaneGroups.end(); ++iGroup)
    {
        (*iGroup)->GetSubstitution()->Show();
        (*iGroup)->DeactivatePanes();
        (*iGroup)->HidePanes();
    }

    mpWindowManager->Update();
}




void PresenterPaneAnimatorBase::ResizePane (
    const Reference<drawing::framework::XResourceId>& rxPaneId,
    const geometry::RealRectangle2D& rBox)
{
    PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
        mpPaneContainer->FindPaneId(rxPaneId));
    if (pDescriptor.get() != NULL)
    {
        mpWindowManager->SetPanePosSizeAbsolute (
            rxPaneId->getResourceURL(),
            rBox.X1,
            rBox.Y1,
            rBox.X2-rBox.X1,
            rBox.Y2-rBox.Y1);
        mpWindowManager->Layout();
        if ( ! pDescriptor->maSpriteProvider.empty())
        {
            pDescriptor->maSpriteProvider()->Resize(GetSize(rBox));
        }
    }
}




void PresenterPaneAnimatorBase::RestoreFrozenWindows (void)
{
    ::std::vector<SharedPaneGroup>::const_iterator iGroup;
    for (iGroup=maPaneGroups.begin(); iGroup!=maPaneGroups.end(); ++iGroup)
    {
        (*iGroup)->Restore();
        (*iGroup)->ShowPanes();
        (*iGroup)->ActivatePanes();
        (*iGroup)->GetSubstitution()->Hide();
    }
    maPaneGroups.clear();

    ActivatePane(mxCenterPaneId);

    mpWindowManager->Update();
}




void PresenterPaneAnimatorBase::ActivatePane (
    const Reference<drawing::framework::XResourceId>& rxPaneId)
{
    PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
        mpPaneContainer->FindPaneId(rxPaneId));
    if (pDescriptor.get() != NULL)
        pDescriptor->SetActivationState(true);
}




void PresenterPaneAnimatorBase::DeactivatePane (
    const Reference<drawing::framework::XResourceId>& rxPaneId)
{
    PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
        mpPaneContainer->FindPaneId(rxPaneId));
    if (pDescriptor.get() != NULL)
        pDescriptor->SetActivationState(false);
}




//===== UnfoldInCenterAnimator ================================================

UnfoldInCenterAnimator::UnfoldInCenterAnimator (
    const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
    const ::rtl::Reference<PresenterController>& rpPresenterController,
    const bool bAnimate,
    const EndActions& rShowEndActions,
    const EndActions& rEndEndActions)
    : PresenterPaneAnimatorBase(rxPaneId, rpPresenterController, bAnimate,
        rShowEndActions, rEndEndActions)
{
}




UnfoldInCenterAnimator::~UnfoldInCenterAnimator (void)
{
}




void UnfoldInCenterAnimator::ShowPane (void)
{
    OSL_ASSERT(mpWindowManager.get()!=NULL);

    Reference<awt::XWindow> xParentWindow (mpWindowManager->GetParentWindow(), UNO_QUERY);
    if ( ! xParentWindow.is())
        return;

    Reference<rendering::XSpriteCanvas> xCanvas (mpWindowManager->GetParentCanvas(), UNO_QUERY);
    if ( ! xCanvas.is())
        return;

    Reference<rendering::XBitmap> xParentBitmap (xCanvas, UNO_QUERY);
    if ( ! xParentBitmap.is())
        return;

    Reference<rendering::XGraphicDevice> xDevice(xCanvas->getDevice());
    if ( ! xDevice.is())
        return;

    awt::Rectangle aWindowBox (xParentWindow->getPosSize());

    // Create two pane groups that will be moved together.  One contains the
    // notes view, the other group contains all other panes.
    SetupPaneGroups();

    // Setup the places where the two pane groups are moved to.
    maCenterPaneBox
        = MovePanesAway(geometry::RealRectangle2D(0,200,aWindowBox.Width, aWindowBox.Height-200));

    // Setup the final size of the new pane so that it fits into the space
    // between the two pane groups.
    ResizePane(mxCenterPaneId, maCenterPaneBox);

    // Avoid that the center pane updates its previews while being animated.
    DeactivatePane(mxCenterPaneId);

    // Replace the pane groups with sprites that look like the panes but can
    // be moved around much faster.
    FreezePanes(xCanvas);

    // The vertical center of the new pane.
    const double nY0 ((maPaneGroups[0]->GetOriginalBoundingBox().Y2
            + maPaneGroups[1]->GetOriginalBoundingBox().Y1) / 2);

    // Make sure that the new pane is painted once before the animation starts.
    SpriteTransform(
        mpPaneContainer,
        mxCenterPaneId,
        xParentWindow,
        mpPresenterController->GetPaintManager(),
        true,
        maCenterPaneBox.X1,
        nY0,
        maCenterPaneBox.Y1,
        0);
    PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
        mpPaneContainer->FindPaneId(mxCenterPaneId));
    if (pDescriptor.get() != NULL)
    {
        mpPresenterController->GetPaintManager()->Invalidate(
            pDescriptor->mxBorderWindow,
            true);
    }

    // Animate the upper and lower window bitmaps.
    ::boost::shared_ptr<MultiAnimation> pMultiAnimation (
        new MultiAnimation(mbDoAnimation ? 500 : 0));

    // Animate the pane group sprites to be moved up or down.
    vector<SharedPaneGroup>::const_iterator iGroup;
    for (iGroup=maPaneGroups.begin(); iGroup!=maPaneGroups.end(); ++iGroup)
    {
        pMultiAnimation->AddAnimation(
            ::boost::bind(
                InterpolatePosition,
                ::boost::protect(::boost::bind(
                    &PresenterSprite::MoveTo, (*iGroup)->GetSubstitution(), _1)),
                _1,
                GetLocation((*iGroup)->GetOriginalBoundingBox()),
                GetLocation((*iGroup)->GetCurrentBoundingBox())));
    }

    // Animate the new center pane to expand.
    pMultiAnimation->AddAnimation(
        ::boost::bind(
            SpriteTransform,
            mpPaneContainer,
            mxCenterPaneId,
            xParentWindow,
            mpPresenterController->GetPaintManager(),
            true,
            maCenterPaneBox.X1,
            nY0,
            maCenterPaneBox.Y1,
            _1));

    // Call updateScreen after each animation step.
    if (xCanvas.is())
        pMultiAnimation->AddAnimation(
            ::boost::bind(&rendering::XSpriteCanvas::updateScreen, xCanvas, sal_False));

    // Activate the panes when the animation is over.
    pMultiAnimation->AddEndCallback(
        ::boost::bind(&PresenterPaneAnimatorBase::ActivatePanes, shared_from_this()));
    EndActions::const_iterator iAction;
    for (iAction=maShowEndActions.begin(); iAction!=maShowEndActions.end(); ++iAction)
        pMultiAnimation->AddEndCallback(*iAction);

    // Start the animation.
    ::boost::shared_ptr<PresenterAnimator> pAnimator (mpPresenterController->GetAnimator());
    OSL_ASSERT(pAnimator.get()!=NULL);
    pAnimator->AddAnimation(SharedPresenterAnimation(pMultiAnimation));

    mpWindowManager->Update();
}




void UnfoldInCenterAnimator::HidePane (void)
{
    OSL_ASSERT(mpWindowManager.get()!=NULL);

    Reference<awt::XWindow> xParentWindow (mpWindowManager->GetParentWindow(), UNO_QUERY);
    if ( ! xParentWindow.is())
        return;

    DeactivatePanes();
    DeactivatePane(mxCenterPaneId);

    ::boost::shared_ptr<PresenterAnimator> pAnimator (mpPresenterController->GetAnimator());
    const awt::Rectangle aWindowBox (xParentWindow->getPosSize());
    const rendering::ViewState aViewState (
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        NULL);
    const rendering::RenderState aRenderState (
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        NULL,
        Sequence<double>(4),
        rendering::CompositeOperation::SOURCE);

    // Animate the uppder and lower window bitmaps.
    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mpWindowManager->GetParentCanvas(), UNO_QUERY);
    ::boost::shared_ptr<MultiAnimation> pMultiAnimation (new MultiAnimation(mbDoAnimation ? 500 : 1));

    vector<SharedPaneGroup>::const_iterator iGroup;
    for (iGroup=maPaneGroups.begin(); iGroup!=maPaneGroups.end(); ++iGroup)
    {
        pMultiAnimation->AddAnimation(
            ::boost::bind(
                InterpolatePosition,
                ::boost::protect(::boost::bind(
                    &PresenterSprite::MoveTo, (*iGroup)->GetSubstitution(), _1)),
                _1,
                GetLocation((*iGroup)->GetCurrentBoundingBox()),
                GetLocation((*iGroup)->GetOriginalBoundingBox())));
    }

    // Animate the new center pane to collapse.
    const double nY0 ((maPaneGroups[0]->GetOriginalBoundingBox().Y2
            + maPaneGroups[1]->GetOriginalBoundingBox().Y1) / 2);
    pMultiAnimation->AddAnimation(
        ::boost::bind(
            SpriteTransform,
            mpPaneContainer,
            mxCenterPaneId,
            xParentWindow,
            mpPresenterController->GetPaintManager(),
            false,
            maCenterPaneBox.X1,
            nY0,
            maCenterPaneBox.Y1,
            _1));

    if (mbDoAnimation)
    {
        pMultiAnimation->AddAnimation(
            ::boost::bind(&rendering::XSpriteCanvas::updateScreen, xSpriteCanvas, sal_False));
    }
    pMultiAnimation->AddEndCallback(
        ::boost::bind(&PresenterPaneAnimatorBase::RestoreFrozenWindows, shared_from_this()));
    EndActions::const_iterator iAction;
    for (iAction=maHideEndActions.begin(); iAction!=maHideEndActions.end(); ++iAction)
        pMultiAnimation->AddEndCallback(*iAction);

    pAnimator->AddAnimation(SharedPresenterAnimation(pMultiAnimation));
}




void UnfoldInCenterAnimator::SetupPaneGroups (void)
{
    maPaneGroups.clear();

    // Setup the upper pane group.
    SharedPaneGroup pUpperPanes (new PaneGroup());
    pUpperPanes->AddPane(mpPaneContainer->FindPaneURL(
        PresenterPaneFactory::msCurrentSlidePreviewPaneURL));
    pUpperPanes->AddPane(mpPaneContainer->FindPaneURL(
        PresenterPaneFactory::msNextSlidePreviewPaneURL));
    pUpperPanes->AddPane(mpPaneContainer->FindPaneURL(
        PresenterPaneFactory::msToolBarPaneURL));
    pUpperPanes->AddPane(mpPaneContainer->FindPaneURL(
        PresenterPaneFactory::msHelpPaneURL));
    maPaneGroups.push_back(pUpperPanes);

    // Setup the lower pane group.
    SharedPaneGroup pLowerPanes (new PaneGroup());
    pLowerPanes->AddPane(mpPaneContainer->FindPaneURL(
        PresenterPaneFactory::msNotesPaneURL));
    maPaneGroups.push_back(pLowerPanes);
}




geometry::RealRectangle2D UnfoldInCenterAnimator::MovePanesAway (
    const geometry::RealRectangle2D& rFreeCenterArea)
{
    SharedPaneGroup aUpperPanes = maPaneGroups[0];
    SharedPaneGroup aLowerPanes = maPaneGroups[1];

    // Move upper pane group out of the way.
    const double nTop (rFreeCenterArea.Y1);
    const double nUpperVerticalOffset (nTop - aUpperPanes->GetOriginalBoundingBox().Y2);
    aUpperPanes->MovePanes(0, nUpperVerticalOffset, mpWindowManager);

    // Move lower pane group out of the way.
    const double nBottom (rFreeCenterArea.Y2);
    const double nLowerVerticalOffset (nBottom - aLowerPanes->GetOriginalBoundingBox().Y1);
    aLowerPanes->MovePanes(0, nLowerVerticalOffset, mpWindowManager);

    return geometry::RealRectangle2D(
        ::std::min(
            aUpperPanes->GetOriginalBoundingBox().X1,
            aLowerPanes->GetOriginalBoundingBox().X1),
        nTop+20,
        ::std::max(
            aUpperPanes->GetOriginalBoundingBox().X2,
            aLowerPanes->GetOriginalBoundingBox().X2),
        nBottom-20);
}




//===== MoveInFromBottomAnimator ==============================================

MoveInFromBottomAnimator::MoveInFromBottomAnimator (
    const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
    const ::rtl::Reference<PresenterController>& rpPresenterController,
    const bool bAnimate,
    const EndActions& rShowEndActions,
    const EndActions& rEndEndActions)
    : PresenterPaneAnimatorBase(rxPaneId, rpPresenterController, bAnimate,
        rShowEndActions, rEndEndActions),
      maNewPaneSprite()
{
}




MoveInFromBottomAnimator::~MoveInFromBottomAnimator (void)
{
}




void MoveInFromBottomAnimator::ShowPane (void)
{
    OSL_ASSERT(mpWindowManager.get()!=NULL);

    Reference<awt::XWindow> xParentWindow (mpWindowManager->GetParentWindow(), UNO_QUERY);
    if ( ! xParentWindow.is())
        return;

    Reference<rendering::XSpriteCanvas> xCanvas (mpWindowManager->GetParentCanvas(), UNO_QUERY);
    if ( ! xCanvas.is())
        return;

    Reference<rendering::XBitmap> xParentBitmap (xCanvas, UNO_QUERY);
    if ( ! xParentBitmap.is())
        return;

    Reference<rendering::XGraphicDevice> xDevice(xCanvas->getDevice());
    if ( ! xDevice.is())
        return;

    awt::Rectangle aWindowBox (xParentWindow->getPosSize());

    // Create a pane group that just contains the notes view.
    SharedPaneGroup pLowerPanes (new PaneGroup());
    pLowerPanes->AddPane(mpPaneContainer->FindPaneURL(
        PresenterPaneFactory::msNotesPaneURL));
    maPaneGroups.push_back(pLowerPanes);

    // Deactivate the panes that will take place in the animation.
    pLowerPanes->DeactivatePanes();
    DeactivatePane(mxCenterPaneId);

    // Set the size of the new pane.
    maCenterPaneBox = pLowerPanes->GetOriginalBoundingBox();
    ResizePane(mxCenterPaneId, maCenterPaneBox);

    geometry::RealPoint2D aStartLocation (maCenterPaneBox.X1, aWindowBox.Height);
    geometry::RealPoint2D aEndLocation (maCenterPaneBox.X1, maCenterPaneBox.Y1);

    // Get the sprite of the new pane, make it visible and move it to the
    // start location.
    PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
        mpPaneContainer->FindPaneId(mxCenterPaneId));
    if (pDescriptor.get() != NULL)
    {
        if (pDescriptor->mxBorderWindow.is())
            pDescriptor->mxBorderWindow->setVisible(sal_True);

        maNewPaneSprite = pDescriptor->maSpriteProvider();
        if (maNewPaneSprite.get() != NULL)
        {
            maNewPaneSprite->MoveTo(aStartLocation);
            maNewPaneSprite->Show();
        }
        xCanvas->updateScreen(sal_False);
    }

    CreateShowAnimation(
        mxCenterPaneId,
        maShowEndActions,
        xCanvas,
        mbDoAnimation,
        aStartLocation,
        aEndLocation);

    mpWindowManager->Update();
}




void MoveInFromBottomAnimator::HidePane (void)
{
    OSL_ASSERT(mpWindowManager.get()!=NULL);

    Reference<awt::XWindow> xParentWindow (mpWindowManager->GetParentWindow(), UNO_QUERY);
    if ( ! xParentWindow.is())
        return;

    Reference<rendering::XSpriteCanvas> xCanvas (mpWindowManager->GetParentCanvas(), UNO_QUERY);
    if ( ! xCanvas.is())
        return;

    DeactivatePanes();
    DeactivatePane(mxCenterPaneId);

    SharedPaneGroup aPanes (maPaneGroups[0]);

    aPanes->ShowPanes();

    ::boost::shared_ptr<MultiAnimation> pMultiAnimation (
        new MultiAnimation(mbDoAnimation ? 500 : 0));
    awt::Rectangle aWindowBox (xParentWindow->getPosSize());

    // Animate the new center pane to collapse.
    pMultiAnimation->AddAnimation(
        ::boost::bind(
            InterpolatePosition,
            ::boost::protect(::boost::bind(&SpritePaneMove, mpPaneContainer, mxCenterPaneId, _1)),
            _1,
            geometry::RealPoint2D(maCenterPaneBox.X1, maCenterPaneBox.Y1),
            geometry::RealPoint2D(maCenterPaneBox.X1, aWindowBox.Height)));

    if (mbDoAnimation)
    {
        pMultiAnimation->AddAnimation(
            ::boost::bind(&rendering::XSpriteCanvas::updateScreen, xCanvas, sal_False));
    }
    pMultiAnimation->AddEndCallback(
        ::boost::bind(&PresenterPaneAnimatorBase::RestoreFrozenWindows, shared_from_this()));
    EndActions::const_iterator iAction;
    for (iAction=maHideEndActions.begin(); iAction!=maHideEndActions.end(); ++iAction)
        pMultiAnimation->AddEndCallback(*iAction);

    ::boost::shared_ptr<PresenterAnimator> pAnimator (mpPresenterController->GetAnimator());
    pAnimator->AddAnimation(SharedPresenterAnimation(pMultiAnimation));
}




void MoveInFromBottomAnimator::CreateShowAnimation (
    const Reference<drawing::framework::XResourceId>& rxPaneId,
    const EndOperators& rEndOperators,
    const Reference<rendering::XSpriteCanvas>& rxSpriteCanvas,
    const bool bAnimate,
    const geometry::RealPoint2D& rStartLocation,
    const geometry::RealPoint2D& rEndLocation)
{
    // Animate the uppder and lower window bitmaps.
    ::boost::shared_ptr<MultiAnimation> pMultiAnimation (new MultiAnimation(bAnimate ? 500 : 0));

    // Animate new pane to move in from the buttom.
    pMultiAnimation->AddAnimation(
        ::boost::bind(
            InterpolatePosition,
            ::boost::protect(::boost::bind(&SpritePaneMove, mpPaneContainer, rxPaneId, _1)),
            _1,
            rStartLocation,
            rEndLocation));

    // Call updateScreen after each animation step.
    if (rxSpriteCanvas.is())
        pMultiAnimation->AddAnimation(
            ::boost::bind(&rendering::XSpriteCanvas::updateScreen, rxSpriteCanvas, sal_False));

    // Activate the panes when the animation is over.
    pMultiAnimation->AddEndCallback(
        ::boost::bind(&PaneGroup::HidePanes, maPaneGroups[0]));
    pMultiAnimation->AddEndCallback(
        ::boost::bind(&PresenterPaneAnimatorBase::ActivatePane, shared_from_this(), mxCenterPaneId));
    EndActions::const_iterator iAction;
    for (iAction=rEndOperators.begin(); iAction!=rEndOperators.end(); ++iAction)
        pMultiAnimation->AddEndCallback(*iAction);

    // Start the animation.
    ::boost::shared_ptr<PresenterAnimator> pAnimator (mpPresenterController->GetAnimator());
    OSL_ASSERT(pAnimator.get()!=NULL);
    pAnimator->AddAnimation(SharedPresenterAnimation(pMultiAnimation));
}





//===== TransparentOverlayAnimator ============================================

TransparentOverlayAnimator::TransparentOverlayAnimator (
    const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
    const ::rtl::Reference<PresenterController>& rpPresenterController,
    const bool bAnimate,
    const EndActions& rShowEndActions,
    const EndActions& rEndEndActions)
    : PresenterPaneAnimatorBase(
        rxPaneId,
        rpPresenterController,
        bAnimate,
        rShowEndActions,
        rEndEndActions),
      maBackgroundSprite()
{
}




TransparentOverlayAnimator::~TransparentOverlayAnimator (void)
{
}




void TransparentOverlayAnimator::ShowPane (void)
{
    EndActions::const_iterator iAction;
    for (iAction=maShowEndActions.begin(); iAction!=maShowEndActions.end(); ++iAction)
        (*iAction)();

    CreateBackgroundSprite();

    Reference<awt::XWindow> xParentWindow (mpWindowManager->GetParentWindow(), UNO_QUERY);
    if (xParentWindow.is())
    {
        const awt::Rectangle aWindowBox (xParentWindow->getPosSize());
        SharedPaneGroup pAllPanes (new PaneGroup());
        pAllPanes->AddPane(mpPaneContainer->FindPaneURL(
            PresenterPaneFactory::msCurrentSlidePreviewPaneURL));
        pAllPanes->AddPane(mpPaneContainer->FindPaneURL(
            PresenterPaneFactory::msNextSlidePreviewPaneURL));
        pAllPanes->AddPane(mpPaneContainer->FindPaneURL(
            PresenterPaneFactory::msToolBarPaneURL));
        pAllPanes->AddPane(mpPaneContainer->FindPaneURL(
            PresenterPaneFactory::msHelpPaneURL));
        pAllPanes->AddPane(mpPaneContainer->FindPaneURL(
            PresenterPaneFactory::msNotesPaneURL));
        maPaneGroups.push_back(pAllPanes);
        pAllPanes->DeactivatePanes();
        mpWindowManager->Update();
    }

    PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
        mpPaneContainer->FindPaneId(mxCenterPaneId));
    if (pDescriptor.get() != NULL)
    {
        PresenterSpritePane* pPane = dynamic_cast<PresenterSpritePane*>(
            pDescriptor->mxPane.get());
        if (pPane != NULL)
            pPane->ShowTransparentBorder();

        pDescriptor->SetActivationState(true);
        if (pDescriptor->mxBorderWindow.is())
            pDescriptor->mxBorderWindow->setVisible(sal_True);
    }
}




void TransparentOverlayAnimator::HidePane (void)
{
    maPaneGroups[0]->ActivatePanes();
    EndActions::const_iterator iAction;
    for (iAction=maHideEndActions.begin(); iAction!=maHideEndActions.end(); ++iAction)
        (*iAction)();
    mpWindowManager->Update();
}




void TransparentOverlayAnimator::CreateBackgroundSprite (void)
{
    Reference<awt::XWindow> xParentWindow (mpWindowManager->GetParentWindow(), UNO_QUERY);
    Reference<rendering::XSpriteCanvas> xParentCanvas (mpWindowManager->GetParentCanvas(), UNO_QUERY);
    if (xParentWindow.is() && xParentCanvas.is())
    {
        const awt::Rectangle aWindowBox (xParentWindow->getPosSize());
        maBackgroundSprite.SetFactory(xParentCanvas);
        maBackgroundSprite.Resize(
            geometry::RealSize2D(aWindowBox.Width, aWindowBox.Height));
        maBackgroundSprite.MoveTo(
            geometry::RealPoint2D(aWindowBox.X, aWindowBox.Y));
        maBackgroundSprite.SetAlpha(0.5);
        maBackgroundSprite.Show();

        Reference<rendering::XCanvas> xCanvas (maBackgroundSprite.GetCanvas());

        if (xCanvas.is())
        {
            rendering::ViewState aViewState(
                geometry::AffineMatrix2D(1,0,0, 0,1,0),
                NULL);

            rendering::RenderState aRenderState(
                geometry::AffineMatrix2D(1,0,0, 0,1,0),
                NULL,
                Sequence<double>(4),
                rendering::CompositeOperation::SOURCE);
            PresenterCanvasHelper::SetDeviceColor(aRenderState, util::Color(0x80808080));

            Reference<rendering::XPolyPolygon2D> xPolygon (
                PresenterGeometryHelper::CreatePolygon(aWindowBox, xCanvas->getDevice()));
            if (xPolygon.is())
                xCanvas->fillPolyPolygon(
                    xPolygon,
                    aViewState,
                    aRenderState);
        }
    }
}




//===== PaneGroup =============================================================

PaneGroup::PaneGroup (void)
    : maPanes(),
      maOriginalBoundingBox(),
      maCurrentBoundingBox(),
      mpSubstitution(new PresenterSprite())
{
}




PaneGroup::~PaneGroup (void)
{
    mpSubstitution.reset();
}




void PaneGroup::AddPane (const PresenterPaneContainer::SharedPaneDescriptor& rpPane)
{
    OSL_ASSERT(rpPane.get() != NULL);

    if (rpPane->mxBorderWindow.is())
    {
        PaneDescriptor aDescriptor (rpPane);
        maPanes.push_back(aDescriptor);
        maOriginalBoundingBox = PresenterGeometryHelper::Union(
            maOriginalBoundingBox,
            rpPane->mxBorderWindow->getPosSize());
    }
}




void PaneGroup::CreateSubstitution (const Reference<rendering::XSpriteCanvas>& rxCanvas)
{
    // Get the bitmap of the background.
    Reference<rendering::XBitmap> xBackgroundBitmap (rxCanvas, UNO_QUERY);
    if ( ! xBackgroundBitmap.is())
        return;

    // Create the sprite.
    mpSubstitution->SetFactory(rxCanvas);
    mpSubstitution->Resize(
        geometry::RealSize2D(maOriginalBoundingBox.Width, maOriginalBoundingBox.Height));

    // Fill it with the background inside the bounding box.
    const rendering::ViewState aViewState (
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        NULL);
    const rendering::RenderState aRenderState (
        geometry::AffineMatrix2D(1,0,-maOriginalBoundingBox.X, 0,1,-maOriginalBoundingBox.Y),
        NULL,
        Sequence<double>(4),
        rendering::CompositeOperation::SOURCE);

    Reference<rendering::XCanvas> xSpriteCanvas (mpSubstitution->GetCanvas());
    if (xSpriteCanvas.is())
        xSpriteCanvas->drawBitmap(xBackgroundBitmap, aViewState, aRenderState);
}




void PaneGroup::Restore (void)
{
    vector<PaneDescriptor>::iterator iPane;
    for (iPane=maPanes.begin(); iPane!=maPanes.end(); ++iPane)
    {
        iPane->Restore();
    }
}



::boost::shared_ptr<PresenterSprite> PaneGroup::GetSubstitution (void) const
{
    return mpSubstitution;
}




geometry::RealRectangle2D PaneGroup::GetOriginalBoundingBox (void) const
{
    return geometry::RealRectangle2D(
        maOriginalBoundingBox.X,
        maOriginalBoundingBox.Y,
        maOriginalBoundingBox.X + maOriginalBoundingBox.Width,
        maOriginalBoundingBox.Y + maOriginalBoundingBox.Height);
}




geometry::RealRectangle2D PaneGroup::GetCurrentBoundingBox (void) const
{
    return maCurrentBoundingBox;
}




void PaneGroup::MovePanes (
    const double nXOffset,
    const double nYOffset,
    const ::rtl::Reference<PresenterWindowManager>& rpWindowManager
    )
{
    awt::Rectangle aBBox;
    vector<PaneDescriptor>::iterator iPane;
    for (iPane=maPanes.begin(); iPane!=maPanes.end(); ++iPane)
    {
        awt::Rectangle aBox (iPane->mpPaneDescriptor->mxBorderWindow->getPosSize());
        aBox.X += sal_Int32(nXOffset);
        aBox.Y += sal_Int32(nYOffset);
        rpWindowManager->SetPanePosSizeAbsolute(
            iPane->mpPaneDescriptor->mxPaneId->getResourceURL(),
            aBox.X,
            aBox.Y,
            aBox.Width,
            aBox.Height);
        aBBox = PresenterGeometryHelper::Union(aBBox, aBox);
    }
    maCurrentBoundingBox = geometry::RealRectangle2D(
        aBBox.X, aBBox.Y, aBBox.X+aBBox.Width, aBBox.Y+aBBox.Height);
}




void PaneGroup::ActivatePanes (void)
{
    vector<PaneDescriptor>::iterator iPane;
    for (iPane=maPanes.begin(); iPane!=maPanes.end(); ++iPane)
    {
        iPane->mpPaneDescriptor->SetActivationState(true);
    }
}




void PaneGroup::DeactivatePanes (void)
{
    vector<PaneDescriptor>::iterator iPane;
    for (iPane=maPanes.begin(); iPane!=maPanes.end(); ++iPane)
    {
        iPane->mpPaneDescriptor->SetActivationState(false);
    }
}




void PaneGroup::ShowPanes (void)
{
    vector<PaneDescriptor>::iterator iPane;
    for (iPane=maPanes.begin(); iPane!=maPanes.end(); ++iPane)
    {
        iPane->mpPaneDescriptor->mxBorderWindow->setVisible(sal_True);
        iPane->mpPaneDescriptor->mxContentWindow->setVisible(sal_True);
    }
}




void PaneGroup::HidePanes (void)
{
    vector<PaneDescriptor>::iterator iPane;
    for (iPane=maPanes.begin(); iPane!=maPanes.end(); ++iPane)
    {
        iPane->mpPaneDescriptor->mxBorderWindow->setVisible(sal_False);
        iPane->mpPaneDescriptor->mxContentWindow->setVisible(sal_False);
    }
}




//===== PaneDescriptor ========================================================

PaneDescriptor::PaneDescriptor (const PresenterPaneContainer::SharedPaneDescriptor& rpDescriptor)
    : mpPaneDescriptor(rpDescriptor),
      mnLeft(rpDescriptor->mnLeft),
      mnTop(rpDescriptor->mnTop),
      mnRight(rpDescriptor->mnRight),
      mnBottom(rpDescriptor->mnBottom)
{
}




void PaneDescriptor::Restore (void) const
{
    mpPaneDescriptor->mnLeft = mnLeft;
    mpPaneDescriptor->mnTop = mnTop;
    mpPaneDescriptor->mnRight = mnRight;
    mpPaneDescriptor->mnBottom = mnBottom;
}




//===== MultiAnimation ========================================================

MultiAnimation::MultiAnimation (const sal_uInt32 nDuration)
    : PresenterAnimation(0, nDuration, 1000/50),
      maAnimations()
{
}




void MultiAnimation::AddAnimation (const Animation& rAnimation)
{
    maAnimations.push_back(rAnimation);
}




void MultiAnimation::Run (const double nProgress, const sal_uInt64 nCurrentTime)
{
    (void)nCurrentTime;
    vector<Animation>::const_iterator iAnimation (maAnimations.begin());
    vector<Animation>::const_iterator iEnd (maAnimations.end());
    for ( ; iAnimation!=iEnd; ++iAnimation)
        (*iAnimation)(nProgress);
}




//===== functors ==============================================================

void InterpolatePosition (
    const ::boost::function<void(geometry::RealPoint2D)>& rSetter,
    double nP,
    const geometry::RealPoint2D rInitialBox,
    const geometry::RealPoint2D rFinalBox)
{
    const double nQ (1 - nP);

    OSL_TRACE("InterpolatePosition %f\n", nP);
    rSetter(
        geometry::RealPoint2D(
            nQ * rInitialBox.X + nP * rFinalBox.X,
            nQ * rInitialBox.Y + nP * rFinalBox.Y));
}




template<typename T>
    void InterpolateValue (
        const ::boost::function<void(T)>& rSetter,
        double nP,
        const T aInitialValue,
        const T aFinalValue)
{
    const double nQ (1 - nP);

    rSetter(T(nQ * aInitialValue + nP * aFinalValue));
}




void SpriteTransform(
    const rtl::Reference<PresenterPaneContainer>& rpPaneContainer,
    const Reference<XResourceId>& rxPaneId,
    const Reference<awt::XWindow>& rxSpriteOwnerWindow,
    const ::boost::shared_ptr<PresenterPaintManager>& rpPaintManager,
    const bool bAppear,
    const double nX,
    const double nInitialTop,
    const double nFinalTop,
    const double nP)
{
    OSL_ASSERT(rpPaintManager.get()!=NULL);

    PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
        rpPaneContainer->FindPaneId(rxPaneId));
    if (pDescriptor.get() != NULL
        && ! pDescriptor->maSpriteProvider.empty()
        && pDescriptor->mxBorderWindow.is())
    {
        ::boost::shared_ptr<PresenterSprite> pSprite (pDescriptor->maSpriteProvider());
        if (pSprite.get())
        {
            // There seems to be a problem with sprites not correctly
            // invalidating the background when being transformed.  As a
            // workaround invalidate the background in the bounding box of
            // the sprite before the transformation.
            rpPaintManager->Invalidate(
                rxSpriteOwnerWindow,
                awt::Rectangle(
                    sal::static_int_cast<sal_Int32>(pSprite->GetLocation().X),
                    sal::static_int_cast<sal_Int32>(pSprite->GetLocation().Y),
                    sal::static_int_cast<sal_Int32>(pSprite->GetSize().Width),
                    sal::static_int_cast<sal_Int32>(pSprite->GetSize().Height)));

            const double nYScale (bAppear ? nP : 1-nP);
            pSprite->Transform(geometry::AffineMatrix2D(
                1, 0, 0,
                0, nYScale, 0));
            pSprite->MoveTo(
                geometry::RealPoint2D(nX, nYScale*nFinalTop + (1-nYScale)*nInitialTop));
            pSprite->Show();

            pDescriptor->mxBorderWindow->setVisible(sal_True);
        }
    }
}




void SpritePaneMove (
    const rtl::Reference<PresenterPaneContainer>& rpPaneContainer,
    const Reference<XResourceId>& rxPaneId,
    const geometry::RealPoint2D& rLocation)
{
    PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
        rpPaneContainer->FindPaneId(rxPaneId));
    if (pDescriptor.get() != NULL
        && ! pDescriptor->maSpriteProvider.empty()
        && pDescriptor->mxBorderWindow.is())
    {
        ::boost::shared_ptr<PresenterSprite> pSprite (pDescriptor->maSpriteProvider());
        if (pSprite.get() != NULL)
        {
            pDescriptor->mxBorderWindow->setVisible(sal_True);
            pSprite->MoveTo(rLocation);
        }
    }
}




geometry::RealPoint2D GetLocation (const geometry::RealRectangle2D& rBox)
{
    return geometry::RealPoint2D(rBox.X1, rBox.Y1);
}




geometry::RealSize2D GetSize (const geometry::RealRectangle2D& rBox)
{
    return geometry::RealSize2D(rBox.X2-rBox.X1, rBox.Y2-rBox.Y1);
}

} // end of anonymous namespace




} } // end of namespace ::sdext::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
