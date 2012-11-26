/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/svdsnpv.hxx>
#include <math.h>
#include <svx/svdetc.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdpage.hxx>
#include <svx/svditer.hxx>
#include <svx/sdr/overlay/overlayobjectlist.hxx>
#include <svx/sdr/overlay/overlaycrosshair.hxx>
#include <svx/sdr/overlay/overlayhelpline.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/svdlegacy.hxx>
#include <svx/sdrpaintwindow.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

class ImplPageOriginOverlay
{
    // The OverlayObjects
    ::sdr::overlay::OverlayObjectList               maObjects;

    // The current position in logical coodinates
    basegfx::B2DPoint                               maPosition;

public:
    ImplPageOriginOverlay(const SdrPaintView& rView, const basegfx::B2DPoint& rStartPos);
    ~ImplPageOriginOverlay();

    void SetPosition(const basegfx::B2DPoint& rNewPosition);
};

ImplPageOriginOverlay::ImplPageOriginOverlay(const SdrPaintView& rView, const basegfx::B2DPoint& rStartPos)
:   maPosition(rStartPos)
{
    for(sal_uInt32 a(0L); a < rView.PaintWindowCount(); a++)
    {
        SdrPaintWindow* pCandidate = rView.GetPaintWindow(a);
        ::sdr::overlay::OverlayManager* pTargetOverlay = pCandidate->GetOverlayManager();

        if(pTargetOverlay)
        {
            ::sdr::overlay::OverlayCrosshairStriped* aNew = new ::sdr::overlay::OverlayCrosshairStriped(
                maPosition);
            pTargetOverlay->add(*aNew);
            maObjects.append(*aNew);
        }
    }
}

ImplPageOriginOverlay::~ImplPageOriginOverlay()
{
    // The OverlayObjects are cleared using the destructor of OverlayObjectList.
    // That destructor calls clear() at the list which removes all objects from the
    // OverlayManager and deletes them.
}

void ImplPageOriginOverlay::SetPosition(const basegfx::B2DPoint& rNewPosition)
{
    if(rNewPosition != maPosition)
    {
        // apply to OverlayObjects
        for(sal_uInt32 a(0); a < maObjects.count(); a++)
        {
            sdr::overlay::OverlayCrosshairStriped* pCandidate =
                static_cast< sdr::overlay::OverlayCrosshairStriped* >(&maObjects.getOverlayObject(a));

            if(pCandidate)
            {
                pCandidate->setBasePosition(rNewPosition);
            }
        }

        // remember new position
        maPosition = rNewPosition;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

class ImplHelpLineOverlay
{
    // The OverlayObjects
    ::sdr::overlay::OverlayObjectList               maObjects;

    // The current position in logical coodinates
    basegfx::B2DPoint                               maPosition;

    // HelpLine specific stuff
    SdrPageView*                                    mpPageView;
    sal_uInt32                                      mnHelpLineNumber;
    SdrHelpLineKind                                 meHelpLineKind;

public:
    ImplHelpLineOverlay(
        const SdrPaintView& rView,
        const basegfx::B2DPoint& rStartPos,
        SdrPageView* pPageView,
        sal_uInt32 nHelpLineNumber,
        SdrHelpLineKind eKind);
    ~ImplHelpLineOverlay();

    void SetPosition(const basegfx::B2DPoint& rNewPosition);

    // access to HelpLine specific stuff
    SdrPageView* GetPageView() const { return mpPageView; }
    sal_uInt32 GetHelpLineNumber() const { return mnHelpLineNumber; }
    SdrHelpLineKind GetHelpLineKind() const { return meHelpLineKind; }
};

ImplHelpLineOverlay::ImplHelpLineOverlay(
    const SdrPaintView& rView,
    const basegfx::B2DPoint& rStartPos,
    SdrPageView* pPageView,
    sal_uInt32 nHelpLineNumber,
    SdrHelpLineKind eKind)
:   maPosition(rStartPos),
    mpPageView(pPageView),
    mnHelpLineNumber(nHelpLineNumber),
    meHelpLineKind(eKind)
{
    for(sal_uInt32 a(0L); a < rView.PaintWindowCount(); a++)
    {
        SdrPaintWindow* pCandidate = rView.GetPaintWindow(a);
        ::sdr::overlay::OverlayManager* pTargetOverlay = pCandidate->GetOverlayManager();

        if(pTargetOverlay)
        {
            ::sdr::overlay::OverlayHelplineStriped* aNew = new ::sdr::overlay::OverlayHelplineStriped(
                maPosition, meHelpLineKind);
            pTargetOverlay->add(*aNew);
            maObjects.append(*aNew);
        }
    }
}

ImplHelpLineOverlay::~ImplHelpLineOverlay()
{
    // The OverlayObjects are cleared using the destructor of OverlayObjectList.
    // That destructor calls clear() at the list which removes all objects from the
    // OverlayManager and deletes them.
}

void ImplHelpLineOverlay::SetPosition(const basegfx::B2DPoint& rNewPosition)
{
    if(rNewPosition != maPosition)
    {
        // apply to OverlayObjects
        // apply to OverlayObjects
        for(sal_uInt32 a(0); a < maObjects.count(); a++)
        {
            sdr::overlay::OverlayHelplineStriped* pCandidate =
                static_cast< sdr::overlay::OverlayHelplineStriped* >(&maObjects.getOverlayObject(a));

            if(pCandidate)
            {
                pCandidate->setBasePosition(rNewPosition);
            }
        }

        // remember new position
        maPosition = rNewPosition;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrSnapView::SdrSnapView(SdrModel& rModel1, OutputDevice* pOut)
:   SdrPaintView(rModel1, pOut),
    mpPageOriginOverlay(0),
    mpHelpLineOverlay(0),
//  aMagnSiz(),
    mfSnapWdtX(0.0),
    mfSnapWdtY(0.0),
    mnDiscreteMagneticSnap(4),
    mnSnapAngle(1500),
    mnEliminatePolyPointLimitAngle(0),
    meCrookMode(SDRCROOK_ROTATE),
    mbSnapEnabled(true),
    mbGridSnap(true),
    mbBorderSnap(true),
    mbHelplineSnap(true),
    mbOFrameSnap(true),
    mbOPointSnap(false),
    mbOConnectorSnap(true),
    mbMoveSnapOnlyTopLeft(false),
    mbOrthogonal(false),
    mbBigOrthogonal(true),
    mbAngleSnap(false),
    mbMoveOnlyDragging(false),
    mbSlantButShear(false),
    mbCrookNoContortion(false),
    mbHelplinesFixed(false),
    mbEliminatePolyPoints(false)
{
    BrkSetPageOrg();
    BrkDragHelpLine();
}

SdrSnapView::~SdrSnapView()
{
    BrkSetPageOrg();
    BrkDragHelpLine();
}

bool SdrSnapView::IsAction() const
{
    return IsSetPageOrg() || IsDragHelpLine();
}

void SdrSnapView::MovAction(const basegfx::B2DPoint& rPnt)
{
    if (IsSetPageOrg())
    {
        MovSetPageOrg(rPnt);
    }

    if (IsDragHelpLine())
    {
        MovDragHelpLine(rPnt);
    }
}

void SdrSnapView::EndAction()
{
    if(IsSetPageOrg())
    {
        EndSetPageOrg();
    }

    if (IsDragHelpLine())
    {
        EndDragHelpLine();
    }
}

void SdrSnapView::BckAction()
{
    BrkSetPageOrg();
    BrkDragHelpLine();
}

void SdrSnapView::BrkAction()
{
    BrkSetPageOrg();
    BrkDragHelpLine();
}

basegfx::B2DRange SdrSnapView::TakeActionRange() const
{
    if(IsSetPageOrg() || IsDragHelpLine())
    {
        return basegfx::B2DRange(GetDragStat().GetNow());
    }

    return basegfx::B2DRange();
}

namespace
{
    basegfx::B2DPoint impCheckNewSnapped(
        const basegfx::B2DPoint& rPnt,
        const basegfx::B2DPoint& rSnapped,
        double& rfSmallestDistanceX,
        double& rfSmallestDistanceY)
    {
        basegfx::B2DPoint aRetval(rPnt);
        const double fNewDistX(fabs(rSnapped.getX() - rPnt.getX()));
        const double fNewDistY(fabs(rSnapped.getY() - rPnt.getY()));

        if(fNewDistX < rfSmallestDistanceX)
        {
            rfSmallestDistanceX = fNewDistX;
            aRetval.setX(rSnapped.getX());
        }

        if(fNewDistY < rfSmallestDistanceY)
        {
            rfSmallestDistanceY = fNewDistY;
            aRetval.setY(rSnapped.getY());
        }

        return aRetval;
    }

    basegfx::B2DPoint impSnapPos_Helpline(
        const basegfx::B2DPoint& rPnt,
        double& rfSmallestDistanceX,
        double& rfSmallestDistanceY,
        const SdrHelpLineList& rHelpLineList,
        const basegfx::B2DVector &rMagneticSnapLogic)
    {
        basegfx::B2DPoint aRetval(rPnt);
        const sal_uInt32 nAnz(rHelpLineList.GetCount());

        for(sal_uInt32 i(0); i < nAnz; i++)
        {
            const SdrHelpLine& rHL = rHelpLineList[i];

            if(SDRHELPLINE_VERTICAL == rHL.GetKind())
            {
                if(basegfx::fTools::less(fabs(rHL.GetPos().getX() - rPnt.getX()), rMagneticSnapLogic.getX()))
                {
                    const basegfx::B2DPoint aSnapped(rHL.GetPos().getX(), rPnt.getY());
                    aRetval = impCheckNewSnapped(rPnt, aSnapped, rfSmallestDistanceX, rfSmallestDistanceY);
                }
            }
            else if(SDRHELPLINE_HORIZONTAL == rHL.GetKind())
            {
                if(basegfx::fTools::less(fabs(rHL.GetPos().getY() - rPnt.getY()), rMagneticSnapLogic.getY()))
                {
                    const basegfx::B2DPoint aSnapped(rPnt.getX(), rHL.GetPos().getY());
                    aRetval = impCheckNewSnapped(rPnt, aSnapped, rfSmallestDistanceX, rfSmallestDistanceY);
                }
            }
            else
            {
                const basegfx::B2DVector aDiff(absolute(rPnt - rHL.GetPos()));

                if(basegfx::fTools::less(aDiff.getX(), rMagneticSnapLogic.getX()) && basegfx::fTools::less(aDiff.getY(), rMagneticSnapLogic.getY()))
                {
                    aRetval = impCheckNewSnapped(rPnt, rHL.GetPos(), rfSmallestDistanceX, rfSmallestDistanceY);
                }
            }
        }

        return aRetval;
    }

    basegfx::B2DPoint impSnapPos_PageBounds(
        const basegfx::B2DPoint& rPnt,
        double& rfSmallestDistanceX,
        double& rfSmallestDistanceY,
        const SdrPage& rPage,
        const basegfx::B2DVector &rMagneticSnapLogic)
    {
        basegfx::B2DPoint aRetval(rPnt);
        const basegfx::B2DRange aPageRange(basegfx::B2DPoint(0.0, 0.0), rPage.GetPageScale());

        // left border
        if(basegfx::fTools::less(fabs(aPageRange.getMinX() - rPnt.getX()), rMagneticSnapLogic.getX()))
        {
            const basegfx::B2DPoint aSnapped(aPageRange.getMinX(), rPnt.getY());
            aRetval = impCheckNewSnapped(rPnt, aSnapped, rfSmallestDistanceX, rfSmallestDistanceY);
        }

        // right border
        if(basegfx::fTools::less(fabs(aPageRange.getMaxX() - rPnt.getX()), rMagneticSnapLogic.getX()))
        {
            const basegfx::B2DPoint aSnapped(aPageRange.getMaxX(), rPnt.getY());
            aRetval = impCheckNewSnapped(rPnt, aSnapped, rfSmallestDistanceX, rfSmallestDistanceY);
        }

        // top border
        if(basegfx::fTools::less(fabs(aPageRange.getMinY() - rPnt.getY()), rMagneticSnapLogic.getY()))
        {
            const basegfx::B2DPoint aSnapped(rPnt.getX(), aPageRange.getMinY());
            aRetval = impCheckNewSnapped(rPnt, aSnapped, rfSmallestDistanceX, rfSmallestDistanceY);
        }

        // bottom border
        if(basegfx::fTools::less(fabs(aPageRange.getMaxY() - rPnt.getY()), rMagneticSnapLogic.getY()))
        {
            const basegfx::B2DPoint aSnapped(rPnt.getX(), aPageRange.getMaxY());
            aRetval = impCheckNewSnapped(rPnt, aSnapped, rfSmallestDistanceX, rfSmallestDistanceY);
        }

        const basegfx::B2DRange aInnerPageRange(rPage.GetInnerPageRange());

        if(!aPageRange.equal(aInnerPageRange))
        {
            // left border
            if(basegfx::fTools::less(fabs(aInnerPageRange.getMinX() - rPnt.getX()), rMagneticSnapLogic.getX()))
            {
                const basegfx::B2DPoint aSnapped(aInnerPageRange.getMinX(), rPnt.getY());
                aRetval = impCheckNewSnapped(rPnt, aSnapped, rfSmallestDistanceX, rfSmallestDistanceY);
            }

            // right border
            if(basegfx::fTools::less(fabs(aInnerPageRange.getMaxX() - rPnt.getX()), rMagneticSnapLogic.getX()))
            {
                const basegfx::B2DPoint aSnapped(aInnerPageRange.getMaxX(), rPnt.getY());
                aRetval = impCheckNewSnapped(rPnt, aSnapped, rfSmallestDistanceX, rfSmallestDistanceY);
            }

            // top border
            if(basegfx::fTools::less(fabs(aInnerPageRange.getMinY() - rPnt.getY()), rMagneticSnapLogic.getY()))
            {
                const basegfx::B2DPoint aSnapped(rPnt.getX(), aInnerPageRange.getMinY());
                aRetval = impCheckNewSnapped(rPnt, aSnapped, rfSmallestDistanceX, rfSmallestDistanceY);
            }

            // bottom border
            if(basegfx::fTools::less(fabs(aInnerPageRange.getMaxY() - rPnt.getY()), rMagneticSnapLogic.getY()))
            {
                const basegfx::B2DPoint aSnapped(rPnt.getX(), aInnerPageRange.getMaxY());
                aRetval = impCheckNewSnapped(rPnt, aSnapped, rfSmallestDistanceX, rfSmallestDistanceY);
            }
        }

        return aRetval;
    }

    basegfx::B2DPoint impSnapPos_ObjectFrame(
        const basegfx::B2DPoint& rPnt,
        double& rfSmallestDistanceX,
        double& rfSmallestDistanceY,
        const SdrPage& rPage,
        const basegfx::B2DVector &rMagneticSnapLogic,
        bool bOFrameSnap,
        bool bOPointSnap,
        const SdrView* pSdrView)
    {
        basegfx::B2DPoint aRetval(rPnt);
        SdrObjListIter aIter(rPage, IM_DEEPNOGROUPS, true);

        while(aIter.IsMore())
        {
            const SdrObject* pCandidate = aIter.Next();
            const basegfx::B2DRange& rCandidateRange = pCandidate->getObjectRange(pSdrView);

            if(!rCandidateRange.isInside(aRetval))
            {
                basegfx::B2DRange aCandidateRange(rCandidateRange);

                aCandidateRange.expand(aCandidateRange.getMinimum() - rMagneticSnapLogic);
                aCandidateRange.expand(aCandidateRange.getMaximum() + rMagneticSnapLogic);

                if(!aCandidateRange.isInside(aRetval))
                {
                    continue;
                }
            }

            if(bOPointSnap)
            {
                const sal_uInt32 nAnz(pCandidate->GetSnapPointCount());

                for(sal_uInt32 i(0); i < nAnz; i++)
                {
                    const basegfx::B2DPoint aTestPoint(pCandidate->GetSnapPoint(i));
                    const basegfx::B2DVector aDiff(absolute(rPnt - aTestPoint));

                    if(basegfx::fTools::less(aDiff.getX(), rMagneticSnapLogic.getX()) && basegfx::fTools::less(aDiff.getY(), rMagneticSnapLogic.getY()))
                    {
                        aRetval = impCheckNewSnapped(rPnt, aTestPoint, rfSmallestDistanceX, rfSmallestDistanceY);
                    }
                }
            }

            if(bOFrameSnap)
            {
                const basegfx::B2DRange aCandidateRange(sdr::legacy::GetSnapRange(*pCandidate));
                bool bInside(aCandidateRange.isInside(rPnt));

                if(!bInside)
                {
                    basegfx::B2DRange aGrownCandidateRange(aCandidateRange);

                    aGrownCandidateRange.expand(aGrownCandidateRange.getMinimum() - rMagneticSnapLogic);
                    aGrownCandidateRange.expand(aGrownCandidateRange.getMaximum() + rMagneticSnapLogic);

                    bInside = aGrownCandidateRange.isInside(rPnt);
                }

                if(bInside)
                {
                    // left border
                    if(basegfx::fTools::less(fabs(aCandidateRange.getMinX() - rPnt.getX()), rMagneticSnapLogic.getX()))
                    {
                        const basegfx::B2DPoint aSnapped(aCandidateRange.getMinX(), rPnt.getY());
                        aRetval = impCheckNewSnapped(rPnt, aSnapped, rfSmallestDistanceX, rfSmallestDistanceY);
                    }

                    // right border
                    if(basegfx::fTools::less(fabs(aCandidateRange.getMaxX() - rPnt.getX()), rMagneticSnapLogic.getX()))
                    {
                        const basegfx::B2DPoint aSnapped(aCandidateRange.getMaxX(), rPnt.getY());
                        aRetval = impCheckNewSnapped(rPnt, aSnapped, rfSmallestDistanceX, rfSmallestDistanceY);
                    }

                    // top border
                    if(basegfx::fTools::less(fabs(aCandidateRange.getMinY() - rPnt.getY()), rMagneticSnapLogic.getY()))
                    {
                        const basegfx::B2DPoint aSnapped(rPnt.getX(), aCandidateRange.getMinY());
                        aRetval = impCheckNewSnapped(rPnt, aSnapped, rfSmallestDistanceX, rfSmallestDistanceY);
                    }

                    // bottom border
                    if(basegfx::fTools::less(fabs(aCandidateRange.getMaxY() - rPnt.getY()), rMagneticSnapLogic.getY()))
                    {
                        const basegfx::B2DPoint aSnapped(rPnt.getX(), aCandidateRange.getMaxY());
                        aRetval = impCheckNewSnapped(rPnt, aSnapped, rfSmallestDistanceX, rfSmallestDistanceY);
                    }
                }
            }
        }

        return aRetval;
    }
} // end of anonymous namespace

basegfx::B2DPoint SdrSnapView::GetSnapPos(const basegfx::B2DPoint& rPnt, sal_uInt16* pSnapFlags) const
{
    basegfx::B2DPoint aSnapPoint(rPnt);
    bool bSnappedX(false);
    bool bSnappedY(false);

    if(GetSdrPageView() && IsSnapEnabled())
    {
        const basegfx::B2DVector aMagneticSnapDiscrete(GetDiscreteMagneticSnap(), GetDiscreteMagneticSnap());
        const basegfx::B2DVector aMagneticSnapLogic(GetFirstOutputDevice()
            ? GetFirstOutputDevice()->GetInverseViewTransformation() * aMagneticSnapDiscrete
            : basegfx::B2DVector(0.0, 0.0));
        double fSmallestDistanceX(DBL_MAX);
        double fSmallestDistanceY(DBL_MAX);
        const SdrHelpLineList& rHelpLineList = GetSdrPageView()->GetHelpLines();

        if(rHelpLineList.GetCount() && IsHlplVisible() && IsHelplineSnap() && !IsDragHelpLine())
        {
            aSnapPoint = impSnapPos_Helpline(rPnt, fSmallestDistanceX, fSmallestDistanceY, GetSdrPageView()->GetHelpLines(), aMagneticSnapLogic);
        }

        const SdrPage& rPage = GetSdrPageView()->getSdrPageFromSdrPageView();

        if(IsBordVisible() && IsBorderSnap())
        {
            aSnapPoint = impSnapPos_PageBounds(rPnt, fSmallestDistanceX, fSmallestDistanceY, rPage, aMagneticSnapLogic);
        }

        if(IsOFrameSnap() || IsOPointSnap())
        {
            aSnapPoint = impSnapPos_ObjectFrame(
                rPnt,
                fSmallestDistanceX,
                fSmallestDistanceY,
                rPage,
                aMagneticSnapLogic,
                IsOFrameSnap(),
                IsOPointSnap(),
                getAsSdrView());
        }

        bSnappedX = (DBL_MAX != fSmallestDistanceX);
        bSnappedY = (DBL_MAX != fSmallestDistanceY);

        if(IsGridSnap() && (!bSnappedX || !bSnappedY))
        {
            const basegfx::B2DPoint aUnsnappedGridPos(rPnt - GetSdrPageView()->GetPageOrigin());
            const basegfx::B2DPoint aSnappedGridPos(
                bSnappedX ? aUnsnappedGridPos.getX() : basegfx::snapToNearestMultiple(aUnsnappedGridPos.getX(), GetSnapGridWidthX()),
                bSnappedY ? aUnsnappedGridPos.getY() : basegfx::snapToNearestMultiple(aUnsnappedGridPos.getY(), GetSnapGridWidthY()));

            aSnapPoint = aSnappedGridPos + GetSdrPageView()->GetPageOrigin();
            bSnappedX = bSnappedY = true;
        }
    }

    if(pSnapFlags)
    {
        *pSnapFlags = SDRSNAP_NOTSNAPPED;

        if(bSnappedX)
        {
            *pSnapFlags |= SDRSNAP_XSNAPPED;
        }

        if(bSnappedY)
        {
            *pSnapFlags |= SDRSNAP_YSNAPPED;
        }
    }

    return aSnapPoint;
}

bool SdrSnapView::BegSetPageOrg(const basegfx::B2DPoint& rPnt)
{
    BrkAction();

    DBG_ASSERT(0L == mpPageOriginOverlay, "SdrSnapView::BegSetPageOrg: There exists a ImplPageOriginOverlay (!)");
    mpPageOriginOverlay = new ImplPageOriginOverlay(*this, rPnt);
    GetDragStat().Reset(GetSnapPos(rPnt));

    return true;
}

void SdrSnapView::MovSetPageOrg(const basegfx::B2DPoint& rPnt)
{
    if(IsSetPageOrg())
    {
        GetDragStat().NextMove(GetSnapPos(rPnt));
        DBG_ASSERT(mpPageOriginOverlay, "SdrSnapView::MovSetPageOrg: no ImplPageOriginOverlay (!)");
        mpPageOriginOverlay->SetPosition(GetDragStat().GetNow());
    }
}

bool SdrSnapView::EndSetPageOrg()
{
    bool bRet(false);

    if(IsSetPageOrg())
    {
        SdrPageView* pPV = GetSdrPageView();

        if(pPV)
        {
            pPV->SetPageOrigin(GetDragStat().GetNow());
            bRet = true;
        }

        // cleanup
        BrkSetPageOrg();
    }

    return bRet;
}

void SdrSnapView::BrkSetPageOrg()
{
    if(IsSetPageOrg())
    {
        DBG_ASSERT(mpPageOriginOverlay, "SdrSnapView::BrkSetPageOrg: no ImplPageOriginOverlay (!)");
        delete mpPageOriginOverlay;
        mpPageOriginOverlay = 0;
    }
}

bool SdrSnapView::PickHelpLine(const basegfx::B2DPoint& rPnt, double fTol, sal_uInt32& rnHelpLineNum) const
{
    if(GetSdrPageView())
    {
        const sal_uInt32 nIndex(GetSdrPageView()->GetHelpLines().HLHitTest(rPnt, fTol));

        if(SDRHELPLINE_NOTFOUND != nIndex)
        {
            rnHelpLineNum=nIndex;

            return true;
        }
    }

    return false;
}

// start HelpLine drag for new HelpLine
bool SdrSnapView::BegDragHelpLine(sal_uInt16 nHelpLineNum)
{
    bool bRet(false);

    if(!AreHelplinesFixed())
    {
        BrkAction();

        if(GetSdrPageView() && nHelpLineNum < GetSdrPageView()->GetHelpLines().GetCount())
        {
            const SdrHelpLineList& rHelpLines = GetSdrPageView()->GetHelpLines();
            const SdrHelpLine& rHelpLine = rHelpLines[nHelpLineNum];
            basegfx::B2DPoint aHelpLinePos(rHelpLine.GetPos()); // + GetSdrPageView()->GetOffset();

            DBG_ASSERT(0 == mpHelpLineOverlay, "SdrSnapView::BegDragHelpLine: There exists a ImplHelpLineOverlay (!)");
            mpHelpLineOverlay = new ImplHelpLineOverlay(*this, aHelpLinePos, GetSdrPageView(), nHelpLineNum, rHelpLine.GetKind());

            GetDragStat().Reset(GetSnapPos(aHelpLinePos));
            const double fTolerance(GetFirstOutputDevice()
                ? basegfx::B2DVector(GetFirstOutputDevice()->GetInverseViewTransformation() * basegfx::B2DVector(3.0, 0.0)).getLength()
                : 0.0);
            GetDragStat().SetMinMove(fTolerance);

            bRet = true;
        }
    }

    return bRet;
}

// start HelpLine drag with existing HelpLine
bool SdrSnapView::BegDragHelpLine(const basegfx::B2DPoint& rPnt, SdrHelpLineKind eNewKind)
{
    bool bRet(false);

    BrkAction();

    if(GetSdrPageView())
    {
        DBG_ASSERT(0L == mpHelpLineOverlay, "SdrSnapView::BegDragHelpLine: There exists a ImplHelpLineOverlay (!)");
        mpHelpLineOverlay = new ImplHelpLineOverlay(*this, rPnt, 0, 0, eNewKind);
        GetDragStat().Reset(GetSnapPos(rPnt));
        bRet = true;
    }

    return bRet;
}

Pointer SdrSnapView::GetDraggedHelpLinePointer() const
{
    if(IsDragHelpLine())
    {
        switch(mpHelpLineOverlay->GetHelpLineKind())
        {
            case SDRHELPLINE_VERTICAL :
                return Pointer(POINTER_ESIZE);
            case SDRHELPLINE_HORIZONTAL :
                return Pointer(POINTER_SSIZE);
            default :
                return Pointer(POINTER_MOVE);
        }
    }

    return Pointer(POINTER_MOVE);
}

void SdrSnapView::MovDragHelpLine(const basegfx::B2DPoint& rPnt)
{
    if(IsDragHelpLine() && GetDragStat().CheckMinMoved(rPnt))
    {
        const basegfx::B2DPoint aPnt(GetSnapPos(rPnt));

        if(!aPnt.equal(GetDragStat().GetNow()))
        {
            GetDragStat().NextMove(aPnt);
            DBG_ASSERT(mpHelpLineOverlay, "SdrSnapView::MovDragHelpLine: no ImplHelpLineOverlay (!)");
            mpHelpLineOverlay->SetPosition(GetDragStat().GetNow());
        }
    }
}

bool SdrSnapView::EndDragHelpLine()
{
    bool bRet(false);

    if(IsDragHelpLine())
    {
        if(GetDragStat().IsMinMoved())
        {
            SdrPageView* pPageView = mpHelpLineOverlay->GetPageView();

            if(pPageView)
            {
                // moved existing one
                const SdrHelpLineList& rHelpLines = pPageView->GetHelpLines();
                SdrHelpLine aChangedHelpLine = rHelpLines[mpHelpLineOverlay->GetHelpLineNumber()];
                aChangedHelpLine.SetPos(GetDragStat().GetNow());
                pPageView->SetHelpLine(mpHelpLineOverlay->GetHelpLineNumber(), aChangedHelpLine);

                bRet = true;
            }
            else
            {
                // create new one
                pPageView = GetSdrPageView();

                if(pPageView)
                {
                    SdrHelpLine aNewHelpLine(mpHelpLineOverlay->GetHelpLineKind(), GetDragStat().GetNow());
                    pPageView->InsertHelpLine(aNewHelpLine);

                    bRet = true;
                }
            }
        }

        // cleanup
        BrkDragHelpLine();
    }

    return bRet;
}

void SdrSnapView::BrkDragHelpLine()
{
    if(IsDragHelpLine())
    {
        DBG_ASSERT(mpHelpLineOverlay, "SdrSnapView::EndDragHelpLine: no ImplHelpLineOverlay (!)");
        delete mpHelpLineOverlay;
        mpHelpLineOverlay = 0L;
    }
}

//////////////////////////////////////////////////////////////////////////////
// eof
