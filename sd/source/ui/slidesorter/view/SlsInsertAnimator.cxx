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

#include <view/SlsInsertAnimator.hxx>
#include <controller/SlideSorterController.hxx>
#include <controller/SlsAnimationFunction.hxx>
#include <view/SlideSorterView.hxx>
#include <view/SlsLayouter.hxx>
#include <model/SlideSorterModel.hxx>
#include <model/SlsPageEnumerationProvider.hxx>
#include <SlideSorter.hxx>
#include <Window.hxx>

#include <memory>
#include <set>

namespace sd { namespace slidesorter { namespace view {

namespace {

class PageObjectRun;

class AnimatorAccess
{
public:
    virtual void AddRun (const std::shared_ptr<PageObjectRun>& rRun) = 0;
    virtual void RemoveRun (const std::shared_ptr<PageObjectRun>& rRun) = 0;
    virtual model::SlideSorterModel& GetModel () const = 0;
    virtual view::SlideSorterView& GetView () const = 0;
    virtual std::shared_ptr<controller::Animator> GetAnimator () = 0;
    virtual VclPtr<sd::Window> GetContentWindow () = 0;

protected:
    ~AnimatorAccess() COVERITY_NOEXCEPT_FALSE {}
};

/** Controller of the position offsets of all page objects in one row or one
    column.
*/
class PageObjectRun : public std::enable_shared_from_this<PageObjectRun>
{
public:
    PageObjectRun (
        AnimatorAccess& rAnimatorAccess,
        const sal_Int32 nRunIndex,
        const sal_Int32 nStartIndex,
        const sal_Int32 nEndIndex);

    void operator () (const double nTime);

    void UpdateOffsets(
        const InsertPosition& rInsertPosition,
        const view::Layouter& GetLayouter);
    void ResetOffsets (const controller::Animator::AnimationMode eMode);

    /// Index of the row or column that this run represents.
    sal_Int32 const mnRunIndex;
    /// The index at which to make place for the insertion indicator (-1 for
    /// no indicator).
    sal_Int32 mnLocalInsertIndex;
    /// Index of the first page in the run.
    sal_Int32 const mnStartIndex;
    /// Index of the last page in the run.
    sal_Int32 const mnEndIndex;
    /// Offset of each item in the run at the start of the current animation.
    ::std::vector<Point> maStartOffset;
    /// Target offset of each item in the run at the end of the current animation.
    ::std::vector<Point> maEndOffset;
    /// Time at which the current animation started.
    double mnStartTime;

    class Comparator
    {
        public: bool operator() (
            const std::shared_ptr<PageObjectRun>& rpRunA,
            const std::shared_ptr<PageObjectRun>& rpRunB) const
        {
            return rpRunA->mnRunIndex < rpRunB->mnRunIndex;
        }
    };
private:
    controller::Animator::AnimationId mnAnimationId;
    AnimatorAccess& mrAnimatorAccess;
    ::std::function<double (double)> const maAccelerationFunction;

    void RestartAnimation();
};
typedef std::shared_ptr<PageObjectRun> SharedPageObjectRun;

Point Blend (const Point& rPointA, const Point& rPointB, const double nT)
{
    return Point(
        sal_Int32(rPointA.X() * (1-nT) + rPointB.X() * nT),
        sal_Int32(rPointA.Y() * (1-nT) + rPointB.Y() * nT));
}

} // end of anonymous namespace

class InsertAnimator::Implementation : public AnimatorAccess
{
public:
    explicit Implementation (SlideSorter& rSlideSorter);
    virtual ~Implementation();

    void SetInsertPosition (
        const InsertPosition& rInsertPosition,
        const controller::Animator::AnimationMode eAnimationMode);

    virtual void AddRun (const std::shared_ptr<PageObjectRun>& rRun) override;
    virtual void RemoveRun (const std::shared_ptr<PageObjectRun>& rRun) override;

    virtual model::SlideSorterModel& GetModel() const override { return mrModel; }
    virtual view::SlideSorterView& GetView() const override { return mrView; }
    virtual std::shared_ptr<controller::Animator> GetAnimator() override { return mpAnimator; }
    virtual VclPtr<sd::Window> GetContentWindow() override { return mrSlideSorter.GetContentWindow(); }

private:
    model::SlideSorterModel& mrModel;
    view::SlideSorterView& mrView;
    SlideSorter& mrSlideSorter;
    std::shared_ptr<controller::Animator> mpAnimator;
    typedef ::std::set<SharedPageObjectRun, PageObjectRun::Comparator> RunContainer;
    RunContainer maRuns;
    InsertPosition maInsertPosition;

    SharedPageObjectRun GetRun (
        view::Layouter const & rLayouter,
        const InsertPosition& rInsertPosition);
    RunContainer::const_iterator FindRun (const sal_Int32 nRunIndex) const;
};

//===== InsertAnimator ========================================================

InsertAnimator::InsertAnimator (SlideSorter& rSlideSorter)
    : mpImplementation(new Implementation(rSlideSorter))
{
}

void InsertAnimator::SetInsertPosition (const InsertPosition& rInsertPosition)
{
    mpImplementation->SetInsertPosition(rInsertPosition, controller::Animator::AM_Animated);
}

void InsertAnimator::Reset (const controller::Animator::AnimationMode eMode)
{
    mpImplementation->SetInsertPosition(InsertPosition(), eMode);
}

//===== InsertAnimator::Implementation ========================================

InsertAnimator::Implementation::Implementation (SlideSorter& rSlideSorter)
    : mrModel(rSlideSorter.GetModel()),
      mrView(rSlideSorter.GetView()),
      mrSlideSorter(rSlideSorter),
      mpAnimator(rSlideSorter.GetController().GetAnimator()),
      maRuns(),
      maInsertPosition()
{
}

InsertAnimator::Implementation::~Implementation()
{
    SetInsertPosition(InsertPosition(), controller::Animator::AM_Immediate);
}

void InsertAnimator::Implementation::SetInsertPosition (
    const InsertPosition& rInsertPosition,
    const controller::Animator::AnimationMode eMode)
{
    if (maInsertPosition == rInsertPosition)
        return;

    SharedPageObjectRun pOldRun (GetRun(mrView.GetLayouter(), maInsertPosition));
    SharedPageObjectRun pCurrentRun (GetRun(mrView.GetLayouter(), rInsertPosition));
    maInsertPosition = rInsertPosition;

    // When the new insert position is in a different run then move the page
    // objects in the old run to their default positions.
    if (pOldRun != pCurrentRun && pOldRun)
        pOldRun->ResetOffsets(eMode);

    if (pCurrentRun)
    {
        pCurrentRun->UpdateOffsets(rInsertPosition, mrView.GetLayouter());
    }
}

SharedPageObjectRun InsertAnimator::Implementation::GetRun (
    view::Layouter const & rLayouter,
    const InsertPosition& rInsertPosition)
{
    const sal_Int32 nRow (rInsertPosition.GetRow());
    if (nRow < 0)
        return SharedPageObjectRun();

    RunContainer::const_iterator iRun (maRuns.end());
    if (rLayouter.GetColumnCount() == 1)
    {
        // There is only one run that contains all slides.
        if (maRuns.empty())
            maRuns.insert(std::make_shared<PageObjectRun>(
                *this,
                0,
                0,
                mrModel.GetPageCount()-1));
        iRun = maRuns.begin();
    }
    else
    {
        iRun = FindRun(nRow);
        if (iRun == maRuns.end())
        {
            // Create a new run.
            const sal_Int32 nStartIndex (rLayouter.GetIndex(nRow, 0));
            const sal_Int32 nEndIndex (rLayouter.GetIndex(nRow, rLayouter.GetColumnCount()-1));
            if (nStartIndex <= nEndIndex)
            {
                iRun = maRuns.insert(std::make_shared<PageObjectRun>(
                    *this,
                    nRow,
                    nStartIndex,
                    nEndIndex)).first;
                OSL_ASSERT(iRun != maRuns.end());
            }
        }
    }

    if (iRun != maRuns.end())
        return *iRun;
    else
        return SharedPageObjectRun();
}

InsertAnimator::Implementation::RunContainer::const_iterator
    InsertAnimator::Implementation::FindRun (const sal_Int32 nRunIndex) const
{
    return std::find_if(
        maRuns.begin(),
        maRuns.end(),
        [nRunIndex] (std::shared_ptr<PageObjectRun> const& rRun)
            { return rRun->mnRunIndex == nRunIndex; });
}

void InsertAnimator::Implementation::AddRun (const std::shared_ptr<PageObjectRun>& rRun)
{
    if (rRun)
    {
        maRuns.insert(rRun);
    }
    else
    {
        OSL_ASSERT(rRun);
    }
}

void InsertAnimator::Implementation::RemoveRun (const std::shared_ptr<PageObjectRun>& rRun)
{
    if (rRun)
    {
        // Do not remove runs that show the space for the insertion indicator.
        if (rRun->mnLocalInsertIndex == -1)
        {
            InsertAnimator::Implementation::RunContainer::const_iterator iRun (FindRun(rRun->mnRunIndex));
            if (iRun != maRuns.end())
            {
                OSL_ASSERT(*iRun == rRun);
                maRuns.erase(iRun);
            }
        }
    }
    else
    {
        OSL_ASSERT(rRun);
    }
}

//===== PageObjectRun =========================================================

PageObjectRun::PageObjectRun (
    AnimatorAccess& rAnimatorAccess,
    const sal_Int32 nRunIndex,
    const sal_Int32 nStartIndex,
    const sal_Int32 nEndIndex)
    : mnRunIndex(nRunIndex),
      mnLocalInsertIndex(-1),
      mnStartIndex(nStartIndex),
      mnEndIndex(nEndIndex),
      maStartOffset(),
      maEndOffset(),
      mnStartTime(-1),
      mnAnimationId(controller::Animator::NotAnAnimationId),
      mrAnimatorAccess(rAnimatorAccess),
      maAccelerationFunction(
          controller::AnimationParametricFunction(
              controller::AnimationBezierFunction (0.1,0.7)))
{
    maStartOffset.resize(nEndIndex - nStartIndex + 1);
    maEndOffset.resize(nEndIndex - nStartIndex + 1);
}

void PageObjectRun::UpdateOffsets(
    const InsertPosition& rInsertPosition,
    const view::Layouter& rLayouter)
{
    const bool bIsVertical (rLayouter.GetColumnCount()==1);
    const sal_Int32 nLocalInsertIndex(bIsVertical
        ? rInsertPosition.GetRow()
        : rInsertPosition.GetColumn());
    if (nLocalInsertIndex == mnLocalInsertIndex)
        return;

    mnLocalInsertIndex = nLocalInsertIndex;

    model::SlideSorterModel& rModel (mrAnimatorAccess.GetModel());
    const sal_Int32 nRunLength (mnEndIndex - mnStartIndex + 1);
    for (sal_Int32 nIndex=0; nIndex<nRunLength; ++nIndex)
    {
        model::SharedPageDescriptor pDescriptor(rModel.GetPageDescriptor(nIndex+mnStartIndex));
        if (pDescriptor)
            maStartOffset[nIndex] = pDescriptor->GetVisualState().GetLocationOffset();
        maEndOffset[nIndex] = nIndex < mnLocalInsertIndex
            ? rInsertPosition.GetLeadingOffset()
            : rInsertPosition.GetTrailingOffset();
        if (bIsVertical)
            maEndOffset[nIndex].setX( 0 );
        else
            maEndOffset[nIndex].setY( 0 );
    }
    RestartAnimation();
}

void PageObjectRun::ResetOffsets (const controller::Animator::AnimationMode eMode)
{
    mnLocalInsertIndex = -1;
    const sal_Int32 nRunLength (mnEndIndex - mnStartIndex + 1);
    model::SlideSorterModel& rModel (mrAnimatorAccess.GetModel());
    view::SlideSorterView& rView (mrAnimatorAccess.GetView());
    for (sal_Int32 nIndex=0; nIndex<nRunLength; ++nIndex)
    {
        model::SharedPageDescriptor pDescriptor(rModel.GetPageDescriptor(nIndex+mnStartIndex));
        if (pDescriptor)
        {
            if (eMode == controller::Animator::AM_Animated)
                maStartOffset[nIndex] = pDescriptor->GetVisualState().GetLocationOffset();
            else
            {
                const ::tools::Rectangle aOldBoundingBox (pDescriptor->GetBoundingBox());
                pDescriptor->GetVisualState().SetLocationOffset(Point(0,0));
                rView.RequestRepaint(aOldBoundingBox);
                rView.RequestRepaint(pDescriptor);
            }
        }
        maEndOffset[nIndex] = Point(0,0);
    }
    if (eMode == controller::Animator::AM_Animated)
        RestartAnimation();
    else
        mrAnimatorAccess.RemoveRun(shared_from_this());
}

void PageObjectRun::RestartAnimation()
{
    // Stop the current animation.
    if (mnAnimationId != controller::Animator::NotAnAnimationId)
    {
        mrAnimatorAccess.GetAnimator()->RemoveAnimation(mnAnimationId);
    }

    // Restart the animation.
    mrAnimatorAccess.AddRun(shared_from_this());
    auto sharedThis(shared_from_this());
    mnAnimationId = mrAnimatorAccess.GetAnimator()->AddAnimation(
        [this] (double const val) { (*this)(val); },
        [sharedThis] () { sharedThis->mrAnimatorAccess.RemoveRun(sharedThis); }
        );
}

void PageObjectRun::operator () (const double nGlobalTime)
{
    if (mnStartTime < 0)
        mnStartTime = nGlobalTime;

    double nLocalTime (nGlobalTime - mnStartTime);
    if (nLocalTime > 1.0)
        nLocalTime = 1.0;
    nLocalTime = maAccelerationFunction(nLocalTime);

    model::SlideSorterModel& rModel (mrAnimatorAccess.GetModel());
    view::SlideSorterView& rView (mrAnimatorAccess.GetView());
    for (sal_Int32 nIndex=mnStartIndex; nIndex<=mnEndIndex; ++nIndex)
    {
        model::SharedPageDescriptor pDescriptor (rModel.GetPageDescriptor(nIndex));
        if ( ! pDescriptor)
            continue;
        const ::tools::Rectangle aOldBoundingBox (pDescriptor->GetBoundingBox());
        pDescriptor->GetVisualState().SetLocationOffset(
            Blend(
                maStartOffset[nIndex-mnStartIndex],
                maEndOffset[nIndex-mnStartIndex],
                nLocalTime));

        // Request a repaint of the old and new bounding box (which largely overlap.)
        rView.RequestRepaint(aOldBoundingBox);
        rView.RequestRepaint(pDescriptor);
    }

    // Call Flush to make
    // a) animations a bit more smooth and
    // b) on Mac without the Flush a Reset of the page locations is not properly
    // visualized when the mouse leaves the window during drag-and-drop.
    mrAnimatorAccess.GetContentWindow()->Flush();
}

} } } // end of namespace ::sd::slidesorter::view

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
