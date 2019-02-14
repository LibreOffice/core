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

#include <fumorph.hxx>
#include <svx/xfillit.hxx>
#include <svx/xlineit.hxx>
#include <svx/svdpool.hxx>
#include <tools/poly.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdogrp.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/outlobj.hxx>

#include <View.hxx>
#include <ViewShell.hxx>
#include <Window.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <strings.hrc>
#include <sdresid.hxx>

#include <sdabstdlg.hxx>

#include <svx/svditer.hxx>

#include <basegfx/color/bcolor.hxx>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <memory>

using namespace com::sun::star;

namespace sd {

FuMorph::FuMorph (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq )
    :   FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

rtl::Reference<FuPoor> FuMorph::Create(
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq
)
{
    rtl::Reference<FuPoor> xFunc( new FuMorph( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuMorph::DoExecute( SfxRequest& )
{
    const SdrMarkList&  rMarkList = mpView->GetMarkedObjectList();

    if(rMarkList.GetMarkCount() != 2)
        return;

    // create clones
    SdrObject*  pObj1 = rMarkList.GetMark(0)->GetMarkedSdrObj();
    SdrObject*  pObj2 = rMarkList.GetMark(1)->GetMarkedSdrObj();
    SdrObject*  pCloneObj1(pObj1->CloneSdrObject(pObj1->getSdrModelFromSdrObject()));
    SdrObject*  pCloneObj2(pObj2->CloneSdrObject(pObj2->getSdrModelFromSdrObject()));

    // delete text at clone, otherwise we do net get a correct PathObj
    pCloneObj1->SetOutlinerParaObject(nullptr);
    pCloneObj2->SetOutlinerParaObject(nullptr);

    // create path objects
    SdrObject*  pPolyObj1 = pCloneObj1->ConvertToPolyObj(false, false);
    SdrObject*  pPolyObj2 = pCloneObj2->ConvertToPolyObj(false, false);
    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
    ScopedVclPtr<AbstractMorphDlg> pDlg( pFact->CreateMorphDlg(mpWindow ? mpWindow->GetFrameWeld() : nullptr, pObj1, pObj2) );
    if(pPolyObj1 && pPolyObj2 && (pDlg->Execute() == RET_OK))
    {
        B2DPolyPolygonList_impl aPolyPolyList;
        ::basegfx::B2DPolyPolygon aPolyPoly1;
        ::basegfx::B2DPolyPolygon aPolyPoly2;

        pDlg->SaveSettings();

        // #i48168# Not always is the pPolyObj1/pPolyObj2 a SdrPathObj, it may also be a group object
        // containing SdrPathObjs. To get the polygons, I add two iters here
        SdrObjListIter aIter1(*pPolyObj1);
        SdrObjListIter aIter2(*pPolyObj2);

        while(aIter1.IsMore())
        {
            SdrObject* pObj = aIter1.Next();
            if(auto pPathObj = dynamic_cast< SdrPathObj *>( pObj ))
                aPolyPoly1.append(pPathObj->GetPathPoly());
        }

        while(aIter2.IsMore())
        {
            SdrObject* pObj = aIter2.Next();
            if(auto pPathObj = dynamic_cast< SdrPathObj *>( pObj ))
                aPolyPoly2.append(pPathObj->GetPathPoly());
        }

        // perform morphing
        if(aPolyPoly1.count() && aPolyPoly2.count())
        {
            aPolyPoly1 = ::basegfx::utils::correctOrientations(aPolyPoly1);
            aPolyPoly1.removeDoublePoints();
            ::basegfx::B2VectorOrientation eIsClockwise1(::basegfx::utils::getOrientation(aPolyPoly1.getB2DPolygon(0)));

            aPolyPoly2 = ::basegfx::utils::correctOrientations(aPolyPoly2);
            aPolyPoly2.removeDoublePoints();
            ::basegfx::B2VectorOrientation eIsClockwise2(::basegfx::utils::getOrientation(aPolyPoly2.getB2DPolygon(0)));

            // set same orientation
            if(eIsClockwise1 != eIsClockwise2)
                aPolyPoly2.flip();

            // force same poly count
            if(aPolyPoly1.count() < aPolyPoly2.count())
                ImpAddPolys(aPolyPoly1, aPolyPoly2);
            else if(aPolyPoly2.count() < aPolyPoly1.count())
                ImpAddPolys(aPolyPoly2, aPolyPoly1);

            // use orientation flag from dialog
            if(!pDlg->IsOrientationFade())
                aPolyPoly2.flip();

            // force same point counts
            for( sal_uInt32 a(0); a < aPolyPoly1.count(); a++ )
            {
                ::basegfx::B2DPolygon aSub1(aPolyPoly1.getB2DPolygon(a));
                ::basegfx::B2DPolygon aSub2(aPolyPoly2.getB2DPolygon(a));

                if(aSub1.count() < aSub2.count())
                    ImpEqualizePolyPointCount(aSub1, aSub2);
                else if(aSub2.count() < aSub1.count())
                    ImpEqualizePolyPointCount(aSub2, aSub1);

                aPolyPoly1.setB2DPolygon(a, aSub1);
                aPolyPoly2.setB2DPolygon(a, aSub2);
            }

            ImpMorphPolygons(aPolyPoly1, aPolyPoly2, pDlg->GetFadeSteps(), aPolyPolyList);

            OUString aString(mpView->GetDescriptionOfMarkedObjects());
            aString += " " + SdResId(STR_UNDO_MORPHING);

            mpView->BegUndo(aString);
            ImpInsertPolygons(aPolyPolyList, pDlg->IsAttributeFade(), pObj1, pObj2);
            mpView->EndUndo();
        }
    }
    SdrObject::Free( pCloneObj1 );
    SdrObject::Free( pCloneObj2 );

    SdrObject::Free( pPolyObj1 );
    SdrObject::Free( pPolyObj2 );
}

static ::basegfx::B2DPolygon ImpGetExpandedPolygon(
    const ::basegfx::B2DPolygon& rCandidate,
    sal_uInt32 nNum
)
{
    if(rCandidate.count() && nNum && rCandidate.count() != nNum)
    {
        // length of step in dest poly
        ::basegfx::B2DPolygon aRetval;
        const double fStep(::basegfx::utils::getLength(rCandidate) / static_cast<double>(rCandidate.isClosed() ? nNum : nNum - 1));
        double fDestPos(0.0);
        double fSrcPos(0.0);
        sal_uInt32 nSrcPos(0);
        sal_uInt32 nSrcPosNext((nSrcPos + 1 == rCandidate.count()) ? 0L : nSrcPos + 1);
        double fNextSrcLen(::basegfx::B2DVector(rCandidate.getB2DPoint(nSrcPos) - rCandidate.getB2DPoint(nSrcPosNext)).getLength());

        for(sal_uInt32 b(0); b < nNum; b++)
        {
            // calc fDestPos in source
            while(fSrcPos + fNextSrcLen < fDestPos)
            {
                fSrcPos += fNextSrcLen;
                nSrcPos++;
                nSrcPosNext = (nSrcPos + 1 == rCandidate.count()) ? 0L : nSrcPos + 1;
                fNextSrcLen = ::basegfx::B2DVector(rCandidate.getB2DPoint(nSrcPos) - rCandidate.getB2DPoint(nSrcPosNext)).getLength();
            }

            // fDestPos is between fSrcPos and (fSrcPos + fNextSrcLen)
            const double fLenA((fDestPos - fSrcPos) / fNextSrcLen);
            const ::basegfx::B2DPoint aOld1(rCandidate.getB2DPoint(nSrcPos));
            const ::basegfx::B2DPoint aOld2(rCandidate.getB2DPoint(nSrcPosNext));
            ::basegfx::B2DPoint aNewPoint(basegfx::interpolate(aOld1, aOld2, fLenA));
            aRetval.append(aNewPoint);

            // next step
            fDestPos += fStep;
        }

        if(aRetval.count() >= 3)
        {
            aRetval.setClosed(rCandidate.isClosed());
        }

        return aRetval;
    }
    else
    {
        return rCandidate;
    }
}

/**
 * make the point count of the polygons equal in adding points
 */
void FuMorph::ImpEqualizePolyPointCount(
    ::basegfx::B2DPolygon& rSmall,
    const ::basegfx::B2DPolygon& rBig
)
{
    // create poly with equal point count
    const sal_uInt32 nCnt(rBig.count());
    ::basegfx::B2DPolygon aPoly1(ImpGetExpandedPolygon(rSmall, nCnt));

    // create transformation for rBig to do the compare
    const ::basegfx::B2DRange aSrcSize(::basegfx::utils::getRange(rBig));
    const ::basegfx::B2DPoint aSrcPos(aSrcSize.getCenter());
    const ::basegfx::B2DRange aDstSize(::basegfx::utils::getRange(rSmall));
    const ::basegfx::B2DPoint aDstPos(aDstSize.getCenter());

    basegfx::B2DHomMatrix aTrans(basegfx::utils::createTranslateB2DHomMatrix(-aSrcPos.getX(), -aSrcPos.getY()));
    aTrans.scale(aDstSize.getWidth() / aSrcSize.getWidth(), aDstSize.getHeight() / aSrcSize.getHeight());
    aTrans.translate(aDstPos.getX(), aDstPos.getY());

    // transpose points to have smooth linear blending
    ::basegfx::B2DPolygon aPoly2;
    aPoly2.append(::basegfx::B2DPoint(), nCnt);
    sal_uInt32 nInd(ImpGetNearestIndex(aPoly1, aTrans * rBig.getB2DPoint(0)));

    for(sal_uInt32 a(0); a < nCnt; a++)
    {
        aPoly2.setB2DPoint((a + nCnt - nInd) % nCnt, aPoly1.getB2DPoint(a));
    }

    aPoly2.setClosed(rBig.isClosed());
    rSmall = aPoly2;
}

sal_uInt32 FuMorph::ImpGetNearestIndex(
    const ::basegfx::B2DPolygon& rPoly,
    const ::basegfx::B2DPoint& rPos
)
{
    double fMinDist = 0.0;
    sal_uInt32 nActInd = 0;

    for(sal_uInt32 a(0); a < rPoly.count(); a++)
    {
        double fNewDist(::basegfx::B2DVector(rPoly.getB2DPoint(a) - rPos).getLength());

        if(!a || fNewDist < fMinDist)
        {
            fMinDist = fNewDist;
            nActInd = a;
        }
    }

    return nActInd;
}

/**
 * add to a point reduced polys until count is same
 */
void FuMorph::ImpAddPolys(
    ::basegfx::B2DPolyPolygon& rSmaller,
    const ::basegfx::B2DPolyPolygon& rBigger
)
{
    while(rSmaller.count() < rBigger.count())
    {
        const ::basegfx::B2DPolygon& aToBeCopied(rBigger.getB2DPolygon(rSmaller.count()));
        const ::basegfx::B2DRange aToBeCopiedPolySize(::basegfx::utils::getRange(aToBeCopied));
        ::basegfx::B2DPoint aNewPoint(aToBeCopiedPolySize.getCenter());
        ::basegfx::B2DPolygon aNewPoly;

        const ::basegfx::B2DRange aSrcSize(::basegfx::utils::getRange(rBigger.getB2DPolygon(0)));
        const ::basegfx::B2DPoint aSrcPos(aSrcSize.getCenter());
        const ::basegfx::B2DRange aDstSize(::basegfx::utils::getRange(rSmaller.getB2DPolygon(0)));
        const ::basegfx::B2DPoint aDstPos(aDstSize.getCenter());
        aNewPoint = aNewPoint - aSrcPos + aDstPos;

        for(sal_uInt32 a(0); a < aToBeCopied.count(); a++)
        {
            aNewPoly.append(aNewPoint);
        }

        rSmaller.append(aNewPoly);
    }
}

/**
 * create group object with morphed polygons
 */
void FuMorph::ImpInsertPolygons(
    B2DPolyPolygonList_impl& rPolyPolyList3D,
    bool bAttributeFade,
    const SdrObject* pObj1,
    const SdrObject* pObj2
)
{
    Color               aStartFillCol;
    Color               aEndFillCol;
    Color               aStartLineCol;
    Color               aEndLineCol;
    long                nStartLineWidth = 0;
    long                nEndLineWidth = 0;
    SdrPageView*        pPageView = mpView->GetSdrPageView();
    SfxItemPool &       rPool = pObj1->GetObjectItemPool();
    SfxItemSet          aSet1( rPool,svl::Items<SDRATTR_START,SDRATTR_NOTPERSIST_FIRST-1,EE_ITEMS_START,EE_ITEMS_END>{} );
    SfxItemSet          aSet2( aSet1 );
    bool                bLineColor = false;
    bool                bFillColor = false;
    bool                bLineWidth = false;
    bool                bIgnoreLine = false;
    bool                bIgnoreFill = false;

    aSet1.Put(pObj1->GetMergedItemSet());
    aSet2.Put(pObj2->GetMergedItemSet());

    const drawing::LineStyle eLineStyle1 = aSet1.Get(XATTR_LINESTYLE).GetValue();
    const drawing::LineStyle eLineStyle2 = aSet2.Get(XATTR_LINESTYLE).GetValue();
    const drawing::FillStyle eFillStyle1 = aSet1.Get(XATTR_FILLSTYLE).GetValue();
    const drawing::FillStyle eFillStyle2 = aSet2.Get(XATTR_FILLSTYLE).GetValue();

    if ( bAttributeFade )
    {
        if ( ( eLineStyle1 != drawing::LineStyle_NONE ) && ( eLineStyle2 != drawing::LineStyle_NONE ) )
        {
            bLineWidth = bLineColor = true;

            aStartLineCol = aSet1.Get(XATTR_LINECOLOR).GetColorValue();
            aEndLineCol = aSet2.Get(XATTR_LINECOLOR).GetColorValue();

            nStartLineWidth = aSet1.Get(XATTR_LINEWIDTH).GetValue();
            nEndLineWidth = aSet2.Get(XATTR_LINEWIDTH).GetValue();
        }
        else if ( ( eLineStyle1 == drawing::LineStyle_NONE ) && ( eLineStyle2 == drawing::LineStyle_NONE ) )
            bIgnoreLine = true;

        if ( ( eFillStyle1 == drawing::FillStyle_SOLID ) && ( eFillStyle2 == drawing::FillStyle_SOLID ) )
        {
            bFillColor = true;
            aStartFillCol = aSet1.Get(XATTR_FILLCOLOR).GetColorValue();
            aEndFillCol = aSet2.Get(XATTR_FILLCOLOR).GetColorValue();
        }
        else if ( ( eFillStyle1 == drawing::FillStyle_NONE ) && ( eFillStyle2 == drawing::FillStyle_NONE ) )
            bIgnoreFill = true;
    }

    if ( !pPageView )
        return;

    SfxItemSet      aSet( aSet1 );
    SdrObjGroup*    pObjGroup = new SdrObjGroup(mpView->getSdrModelFromSdrView());
    SdrObjList*     pObjList = pObjGroup->GetSubList();
    const size_t    nCount = rPolyPolyList3D.size();
    const double    fStep = 1. / ( nCount + 1 );
    const double    fDelta = nEndLineWidth - nStartLineWidth;
    double          fFactor = fStep;

    aSet.Put( XLineStyleItem( drawing::LineStyle_SOLID ) );
    aSet.Put( XFillStyleItem( drawing::FillStyle_SOLID ) );

    for ( size_t i = 0; i < nCount; i++, fFactor += fStep )
    {
        const ::basegfx::B2DPolyPolygon& rPolyPoly3D = rPolyPolyList3D[ i ];
        SdrPathObj* pNewObj = new SdrPathObj(
            mpView->getSdrModelFromSdrView(),
            OBJ_POLY,
            rPolyPoly3D);

        // line color
        if ( bLineColor )
        {
            const basegfx::BColor aLineColor(basegfx::interpolate(aStartLineCol.getBColor(), aEndLineCol.getBColor(), fFactor));
            aSet.Put( XLineColorItem( "", Color(aLineColor)));
        }
        else if ( bIgnoreLine )
            aSet.Put( XLineStyleItem( drawing::LineStyle_NONE ) );

        // fill color
        if ( bFillColor )
        {
            const basegfx::BColor aFillColor(basegfx::interpolate(aStartFillCol.getBColor(), aEndFillCol.getBColor(), fFactor));
            aSet.Put( XFillColorItem( "", Color(aFillColor)));
        }
        else if ( bIgnoreFill )
            aSet.Put( XFillStyleItem( drawing::FillStyle_NONE ) );

        // line width
        if ( bLineWidth )
            aSet.Put( XLineWidthItem( nStartLineWidth + static_cast<long>( fFactor * fDelta + 0.5 ) ) );

        pNewObj->SetMergedItemSetAndBroadcast(aSet);

        pObjList->InsertObject( pNewObj );
    }

    if ( nCount )
    {
        pObjList->InsertObject(
            pObj1->CloneSdrObject(pObj1->getSdrModelFromSdrObject()),
            0 );
        pObjList->InsertObject(
            pObj2->CloneSdrObject(pObj2->getSdrModelFromSdrObject()) );

        mpView->DeleteMarked();
        mpView->InsertObjectAtView( pObjGroup, *pPageView, SdrInsertFlags:: SETDEFLAYER );
    }
}

/**
 * create single morphed PolyPolygon
 */
::basegfx::B2DPolyPolygon FuMorph::ImpCreateMorphedPolygon(
    const ::basegfx::B2DPolyPolygon& rPolyPolyStart,
    const ::basegfx::B2DPolyPolygon& rPolyPolyEnd,
    double fMorphingFactor
)
{
    ::basegfx::B2DPolyPolygon aNewPolyPolygon;
    const double fFactor = 1.0 - fMorphingFactor;

    for(sal_uInt32 a(0); a < rPolyPolyStart.count(); a++)
    {
        const ::basegfx::B2DPolygon& aPolyStart(rPolyPolyStart.getB2DPolygon(a));
        const ::basegfx::B2DPolygon& aPolyEnd(rPolyPolyEnd.getB2DPolygon(a));
        const sal_uInt32 nCount(aPolyStart.count());
        ::basegfx::B2DPolygon aNewPolygon;

        for(sal_uInt32 b(0); b < nCount; b++)
        {
            const ::basegfx::B2DPoint& aPtStart(aPolyStart.getB2DPoint(b));
            const ::basegfx::B2DPoint& aPtEnd(aPolyEnd.getB2DPoint(b));
            aNewPolygon.append(aPtEnd + ((aPtStart - aPtEnd) * fFactor));
        }

        aNewPolygon.setClosed(aPolyStart.isClosed() && aPolyEnd.isClosed());
        aNewPolyPolygon.append(aNewPolygon);
    }

    return aNewPolyPolygon;
}

/**
 * create morphed PolyPolygons
 */
void FuMorph::ImpMorphPolygons(
    const ::basegfx::B2DPolyPolygon& rPolyPoly1,
    const ::basegfx::B2DPolyPolygon& rPolyPoly2,
    const sal_uInt16 nSteps,
    B2DPolyPolygonList_impl& rPolyPolyList3D
)
{
    if(!nSteps)
        return;

    const ::basegfx::B2DRange aStartPolySize(::basegfx::utils::getRange(rPolyPoly1));
    const ::basegfx::B2DPoint aStartCenter(aStartPolySize.getCenter());
    const ::basegfx::B2DRange aEndPolySize(::basegfx::utils::getRange(rPolyPoly2));
    const ::basegfx::B2DPoint aEndCenter(aEndPolySize.getCenter());
    const ::basegfx::B2DPoint aDelta(aEndCenter - aStartCenter);
    const double fFactor(1.0 / (nSteps + 1));
    double fValue(0.0);

    for(sal_uInt16 i(0); i < nSteps; i++)
    {
        fValue += fFactor;
        ::basegfx::B2DPolyPolygon aNewPolyPoly2D = ImpCreateMorphedPolygon(rPolyPoly1, rPolyPoly2, fValue);

        const ::basegfx::B2DRange aNewPolySize(::basegfx::utils::getRange(aNewPolyPoly2D));
        const ::basegfx::B2DPoint aNewS(aNewPolySize.getCenter());
        const ::basegfx::B2DPoint aRealS(aStartCenter + (aDelta * fValue));
        const ::basegfx::B2DPoint aDiff(aRealS - aNewS);

        aNewPolyPoly2D.transform(basegfx::utils::createTranslateB2DHomMatrix(aDiff));
        rPolyPolyList3D.push_back( std::move(aNewPolyPoly2D) );
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
