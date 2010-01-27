/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlsViewCacheContext.hxx,v $
 *
 * $Revision: 1.3 $
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

#include "view/SlsInsertAnimator.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsAnimationFunction.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsLayouter.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageEnumerationProvider.hxx"

#include <set>
#include <boost/bind.hpp>


namespace sd { namespace slidesorter { namespace view {

namespace {

class PageObjectRun;

class AnimatorAccess
{
public:
    virtual void RemoveRun (PageObjectRun* pRun) = 0;
    virtual model::SlideSorterModel& GetModel (void) const = 0;
    virtual view::SlideSorterView& GetView (void) const = 0;
    virtual ::boost::shared_ptr<controller::Animator> GetAnimator (void) = 0;
};


/** Controller of the position offsets of all page objects in one row or one
    column.
*/
class PageObjectRun
{
public:
    PageObjectRun (
        AnimatorAccess& rAnimatorAccess,
        const sal_Int32 nRunIndex,
        const sal_Int32 nStartIndex,
        const sal_Int32 nEndIndex);
    ~PageObjectRun (void);

    void operator () (const double nTime);

    void UpdateOffsets(
        const sal_Int32 nInsertIndex,
        const Point& rLeadingOffset,
        const Point& rTrailingOffset);

    /// Index of the row or column that this run represents.
    sal_Int32 mnRunIndex;
    /// The index at which to make place for the insertion indicator (-1 for
    /// no indicator).
    sal_Int32 mnInsertIndex;
    /// Index of the first page in the run.
    sal_Int32 mnStartIndex;
    /// Index of the last page in the run.
    sal_Int32 mnEndIndex;
    /// Offset of each item in the run at the start of the current animation.
    ::std::vector<Point> maStartOffset;
    /// Target offset of each item in the run at the end of the current animation.
    ::std::vector<Point> maEndOffset;
    /// Time at which the current animation started.
    double mnStartTime;

    class Comparator
    {
        public: bool operator() (
            const ::boost::shared_ptr<PageObjectRun>& rpRunA,
            const ::boost::shared_ptr<PageObjectRun>& rpRunB) const
        {
            return rpRunA->mnRunIndex < rpRunB->mnRunIndex;
        }
    };
private:
    controller::Animator::AnimationId mnAnimationId;
    AnimatorAccess& mrAnimatorAccess;
    ::boost::function<double(double)> maAccelerationFunction;

    void RestartAnimation (void);
};
typedef ::boost::shared_ptr<PageObjectRun> SharedPageObjectRun;


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
    Implementation (SlideSorter& rSlideSorter);
    virtual ~Implementation (void);

    void SetInsertPosition (
        const sal_Int32 nPageIndex,
        const bool bInsertBefore);

    void Reset (void);

    virtual void RemoveRun (PageObjectRun* pRun);

    virtual model::SlideSorterModel& GetModel (void) const { return mrModel; }
    virtual view::SlideSorterView& GetView (void) const { return mrView; }
    virtual ::boost::shared_ptr<controller::Animator> GetAnimator (void) { return mpAnimator; }

private:
    model::SlideSorterModel& mrModel;
    view::SlideSorterView& mrView;
    ::boost::shared_ptr<controller::Animator> mpAnimator;
    typedef ::std::set<SharedPageObjectRun, PageObjectRun::Comparator> RunContainer;
    RunContainer maRuns;
    /// The current insertion index. The special value -1 means that there
    /// is no current insertion index.
    sal_Int32 mnCurrentInsertPosition;
    bool mbCurrentInsertBefore;

    void StopAnimation (void);
    SharedPageObjectRun GetRun (
        view::Layouter& rLayouter,
        const sal_Int32 nPageIndex,
        const bool bInsertBefore,
        const bool bCreate = true);
    RunContainer::iterator FindRun (const sal_Int32 nRunIndex) const;
};





//===== InsertAnimator ========================================================

InsertAnimator::InsertAnimator (SlideSorter& rSlideSorter)
    : mpImplementation(new Implementation(rSlideSorter))
{
}




void InsertAnimator::SetInsertPosition (
    const sal_Int32 nPageIndex,
    const bool bInsertBefore)
{
    mpImplementation->SetInsertPosition(nPageIndex, bInsertBefore);
}




void InsertAnimator::Reset (void)
{
    mpImplementation->Reset();
}




//===== InsertAnimator::Implementation ========================================

InsertAnimator::Implementation::Implementation (SlideSorter& rSlideSorter)
    : mrModel(rSlideSorter.GetModel()),
      mrView(rSlideSorter.GetView()),
      mpAnimator(rSlideSorter.GetController().GetAnimator()),
      maRuns(),
      mnCurrentInsertPosition(-1),
      mbCurrentInsertBefore(false)
{
}




InsertAnimator::Implementation::~Implementation (void)
{
    Reset();
}




void InsertAnimator::Implementation::SetInsertPosition (
    const sal_Int32 nPageIndex,
    const bool bInsertBefore)
{
    if (nPageIndex==mnCurrentInsertPosition && bInsertBefore==mbCurrentInsertBefore)
        return;

    SharedPageObjectRun pOldRun (
        GetRun(mrView.GetLayouter(), mnCurrentInsertPosition, mbCurrentInsertBefore));
    SharedPageObjectRun pCurrentRun (
        GetRun(mrView.GetLayouter(), nPageIndex, bInsertBefore));
    mnCurrentInsertPosition = nPageIndex;
    mbCurrentInsertBefore = bInsertBefore;

    // When the new insert position is in a different run then move the page
    // objects in the old run to their default positions.
    if (pOldRun != pCurrentRun)
    {
        if (pOldRun)
        {
            pOldRun->UpdateOffsets(-1, Point(0,0), Point(0,0));
            maRuns.insert(pOldRun);
        }
    }

    if (pCurrentRun)
    {
        const sal_Int32 nColumnCount (mrView.GetLayouter().GetColumnCount());
        pCurrentRun->UpdateOffsets(
            mnCurrentInsertPosition,
            nColumnCount > 1 ? Point(-20,0) : Point(0,-20),
            nColumnCount > 1 ? Point(+20,0) : Point(0,+20));
        maRuns.insert(pCurrentRun);
    }
}




void InsertAnimator::Implementation::Reset (void)
{
    SetInsertPosition(-1, false);
}




SharedPageObjectRun InsertAnimator::Implementation::GetRun (
    view::Layouter& rLayouter,
    const sal_Int32 nPageIndex,
    const bool bInsertBefore,
    const bool bCreate)
{
    if (nPageIndex < 0)
        return SharedPageObjectRun();

    RunContainer::iterator iRun (maRuns.end());
    if (rLayouter.GetColumnCount() == 1)
    {
        // There is only one run that contains all slides.
        if (maRuns.empty() && bCreate)
            maRuns.insert(SharedPageObjectRun(new PageObjectRun(
                *this,
                0,
                0,
                mrModel.GetPageCount()-1)));
        iRun = maRuns.begin();
    }
    else
    {
        // Look up the row that contains the insert position (take into
        // acount the flag that states whether the indicator is to place
        // before or after the page object at that position.)
        int nIndex (nPageIndex);
        if ( ! bInsertBefore && nPageIndex > 0)
            --nIndex;
        const sal_Int32 nRow (rLayouter.GetRow(nIndex));
        iRun = FindRun(nRow);
        if (iRun == maRuns.end() && bCreate)
        {
            // Create a new run.
            const sal_Int32 nStartIndex (rLayouter.GetIndex(nRow,0));
            const sal_Int32 nEndIndex (rLayouter.GetIndex(nRow, rLayouter.GetColumnCount()-1));
            if (nStartIndex <= nEndIndex)
            {
                iRun = maRuns.insert(SharedPageObjectRun(new PageObjectRun(
                    *this,
                    nRow,
                    nStartIndex,
                    nEndIndex))).first;
                OSL_ASSERT(iRun != maRuns.end());
            }
        }
    }

    if (iRun != maRuns.end())
        return *iRun;
    else
        return SharedPageObjectRun();
}




InsertAnimator::Implementation::RunContainer::iterator
    InsertAnimator::Implementation::FindRun (const sal_Int32 nRunIndex) const
{
    return std::find_if(
        maRuns.begin(),
        maRuns.end(),
        ::boost::bind(
            ::std::equal_to<sal_Int32>(),
            ::boost::bind(&PageObjectRun::mnRunIndex, _1),
            nRunIndex));
}




void InsertAnimator::Implementation::RemoveRun (PageObjectRun* pRun)
{
    if (pRun != NULL)
    {
        // Do not remove runs that show the space for the insertion indicator.
        if (pRun->mnInsertIndex == -1)
            maRuns.erase(FindRun(pRun->mnRunIndex));
    }
    else
    {
        OSL_ASSERT(pRun!=NULL);
    }
}





//===== PageObjectRun =========================================================

PageObjectRun::PageObjectRun (
    AnimatorAccess& rAnimatorAccess,
    const sal_Int32 nRunIndex,
    const sal_Int32 nStartIndex,
    const sal_Int32 nEndIndex)
    : mnRunIndex(nRunIndex),
      mnInsertIndex(-1),
      mnStartIndex(nStartIndex),
      mnEndIndex(nEndIndex),
      maStartOffset(),
      maEndOffset(),
      mnStartTime(-1),
      mnAnimationId(controller::Animator::NotAnAnimationId),
      mrAnimatorAccess(rAnimatorAccess),
      maAccelerationFunction(
          controller::AnimationParametricFunction(
              controller::AnimationBezierFunction (0.1,0.6)))
{
    maStartOffset.resize(nEndIndex - nStartIndex + 1);
    maEndOffset.resize(nEndIndex - nStartIndex + 1);
}




PageObjectRun::~PageObjectRun (void)
{
}




void PageObjectRun::UpdateOffsets(
    const sal_Int32 nInsertIndex,
    const Point& rLeadingOffset,
    const Point& rTrailingOffset)
{
    if (nInsertIndex != mnInsertIndex)
    {
        mnInsertIndex = nInsertIndex;

        model::SlideSorterModel& rModel (mrAnimatorAccess.GetModel());
        for (sal_Int32 nIndex=mnStartIndex; nIndex<=mnEndIndex; ++nIndex)
        {
            model::SharedPageDescriptor pDescriptor (rModel.GetPageDescriptor(nIndex));
            if (pDescriptor)
                maStartOffset[nIndex-mnStartIndex] = pDescriptor->GetVisualState().GetLocationOffset();
            maEndOffset[nIndex-mnStartIndex] = nIndex < nInsertIndex
                ? rLeadingOffset
                : rTrailingOffset;
        }
        RestartAnimation();
    }
}




void PageObjectRun::RestartAnimation (void)
{
    // Stop the current animation.
    if (mnAnimationId != controller::Animator::NotAnAnimationId)
    {
        mrAnimatorAccess.GetAnimator()->RemoveAnimation(mnAnimationId);
    }

    // Restart the animation.
    mnAnimationId = mrAnimatorAccess.GetAnimator()->AddAnimation(
        ::boost::ref(*this),
        300,
        ::boost::bind(&AnimatorAccess::RemoveRun, ::boost::ref(mrAnimatorAccess), this));
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
        const Rectangle aOldBoundingBox (pDescriptor->GetBoundingBox());
        pDescriptor->GetVisualState().SetLocationOffset(
            Blend(
                maStartOffset[nIndex-mnStartIndex],
                maEndOffset[nIndex-mnStartIndex],
                nLocalTime));
        rView.RequestRepaint(aOldBoundingBox);
        rView.RequestRepaint(pDescriptor);
    }
}




} } } // end of namespace ::sd::slidesorter::view
