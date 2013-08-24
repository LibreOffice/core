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

#include "fumorph.hxx"
#include <svx/xfillit.hxx>
#include <svx/xlineit.hxx>
#include <vcl/msgbox.hxx>
#include <svx/svdpool.hxx>
#include <tools/poly.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdogrp.hxx>
#include <editeng/eeitem.hxx>

#include "View.hxx"
#include "ViewShell.hxx"
#include "Window.hxx"
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include "strings.hrc"
#include "sdresid.hxx"

#include "sdabstdlg.hxx"

// #i48168#
#include <svx/svditer.hxx>

#include <basegfx/color/bcolor.hxx>

namespace sd {

#define  ITEMVALUE( ItemSet, Id, Cast ) ( ( (const Cast&) (ItemSet).Get( (Id) ) ).GetValue() )
TYPEINIT1( FuMorph, FuPoor );


FuMorph::FuMorph (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq )
    :   FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuMorph::Create(
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq
)
{
    FunctionReference xFunc( new FuMorph( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuMorph::DoExecute( SfxRequest& )
{
    const SdrMarkList&  rMarkList = mpView->GetMarkedObjectList();

    if(rMarkList.GetMarkCount() == 2)
    {
        // create clones
        SdrObject*  pObj1 = rMarkList.GetMark(0)->GetMarkedSdrObj();
        SdrObject*  pObj2 = rMarkList.GetMark(1)->GetMarkedSdrObj();
        SdrObject*  pCloneObj1 = pObj1->Clone();
        SdrObject*  pCloneObj2 = pObj2->Clone();

        // delete text at clone, otherwise we do net get a correct PathObj
        pCloneObj1->SetOutlinerParaObject(NULL);
        pCloneObj2->SetOutlinerParaObject(NULL);

        // create path objects
        SdrObject*  pPolyObj1 = pCloneObj1->ConvertToPolyObj(sal_False, sal_False);
        SdrObject*  pPolyObj2 = pCloneObj2->ConvertToPolyObj(sal_False, sal_False);
        SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
        AbstractMorphDlg* pDlg = pFact ? pFact->CreateMorphDlg( static_cast< ::Window*>(mpWindow), pObj1, pObj2 ) : 0;
        if(pPolyObj1 && pPolyObj2 && pDlg && (pDlg->Execute() == RET_OK))
        {
            B2DPolyPolygonList_impl aPolyPolyList;
            ::basegfx::B2DPolyPolygon aPolyPoly1;
            ::basegfx::B2DPolyPolygon aPolyPoly2;

            pDlg->SaveSettings();

            // #i48168# Not always is the pPolyObj1/pPolyObj2 a SdrPathObj, it may also be a group object
            // containing SdrPathObjs. To get the polygons, i add two iters here
            SdrObjListIter aIter1(*pPolyObj1);
            SdrObjListIter aIter2(*pPolyObj2);

            while(aIter1.IsMore())
            {
                SdrObject* pObj = aIter1.Next();
                if(pObj && pObj->ISA(SdrPathObj))
                    aPolyPoly1.append(((SdrPathObj*)pObj)->GetPathPoly());
            }

            while(aIter2.IsMore())
            {
                SdrObject* pObj = aIter2.Next();
                if(pObj && pObj->ISA(SdrPathObj))
                    aPolyPoly2.append(((SdrPathObj*)pObj)->GetPathPoly());
            }

            // perform morphing
            if(aPolyPoly1.count() && aPolyPoly2.count())
            {
                aPolyPoly1 = ::basegfx::tools::correctOrientations(aPolyPoly1);
                aPolyPoly1.removeDoublePoints();
                ::basegfx::B2VectorOrientation eIsClockwise1(::basegfx::tools::getOrientation(aPolyPoly1.getB2DPolygon(0L)));

                aPolyPoly2 = ::basegfx::tools::correctOrientations(aPolyPoly2);
                aPolyPoly2.removeDoublePoints();
                ::basegfx::B2VectorOrientation eIsClockwise2(::basegfx::tools::getOrientation(aPolyPoly2.getB2DPolygon(0L)));

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
                for( sal_uInt32 a(0L); a < aPolyPoly1.count(); a++ )
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

                if(ImpMorphPolygons(aPolyPoly1, aPolyPoly2, pDlg->GetFadeSteps(), aPolyPolyList))
                {
                    String aString(mpView->GetDescriptionOfMarkedObjects());

                    aString.Append(sal_Unicode(' '));
                    aString.Append(SD_RESSTR(STR_UNDO_MORPHING));

                    mpView->BegUndo(aString);
                    ImpInsertPolygons(aPolyPolyList, pDlg->IsAttributeFade(), pObj1, pObj2);
                    mpView->EndUndo();
                }

                for( size_t i = 0, n = aPolyPolyList.size(); i < n; ++i ) {
                    delete aPolyPolyList[ i ];
                }
            }
        }
        delete pDlg;
        SdrObject::Free( pCloneObj1 );
        SdrObject::Free( pCloneObj2 );

        SdrObject::Free( pPolyObj1 );
        SdrObject::Free( pPolyObj2 );
    }
}

::basegfx::B2DPolygon ImpGetExpandedPolygon(
    const ::basegfx::B2DPolygon& rCandidate,
    sal_uInt32 nNum
)
{
    if(rCandidate.count() && nNum && rCandidate.count() != nNum)
    {
        // length of step in dest poly
        ::basegfx::B2DPolygon aRetval;
        const double fStep(::basegfx::tools::getLength(rCandidate) / (double)(rCandidate.isClosed() ? nNum : nNum - 1L));
        double fDestPos(0.0);
        double fSrcPos(0.0);
        sal_uInt32 nSrcPos(0L);
        sal_uInt32 nSrcPosNext((nSrcPos + 1L == rCandidate.count()) ? 0L : nSrcPos + 1L);
        double fNextSrcLen(::basegfx::B2DVector(rCandidate.getB2DPoint(nSrcPos) - rCandidate.getB2DPoint(nSrcPosNext)).getLength());

        for(sal_uInt32 b(0L); b < nNum; b++)
        {
            // calc fDestPos in source
            while(fSrcPos + fNextSrcLen < fDestPos)
            {
                fSrcPos += fNextSrcLen;
                nSrcPos++;
                nSrcPosNext = (nSrcPos + 1L == rCandidate.count()) ? 0L : nSrcPos + 1L;
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

        if(aRetval.count() >= 3L)
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
    const ::basegfx::B2DRange aSrcSize(::basegfx::tools::getRange(rBig));
    const ::basegfx::B2DPoint aSrcPos(aSrcSize.getCenter());
    const ::basegfx::B2DRange aDstSize(::basegfx::tools::getRange(rSmall));
    const ::basegfx::B2DPoint aDstPos(aDstSize.getCenter());

    basegfx::B2DHomMatrix aTrans(basegfx::tools::createTranslateB2DHomMatrix(-aSrcPos.getX(), -aSrcPos.getY()));
    aTrans.scale(aDstSize.getWidth() / aSrcSize.getWidth(), aDstSize.getHeight() / aSrcSize.getHeight());
    aTrans.translate(aDstPos.getX(), aDstPos.getY());

    // transpose points to have smooth linear blending
    ::basegfx::B2DPolygon aPoly2;
    aPoly2.append(::basegfx::B2DPoint(), nCnt);
    sal_uInt32 nInd(ImpGetNearestIndex(aPoly1, aTrans * rBig.getB2DPoint(0L)));

    for(sal_uInt32 a(0L); a < nCnt; a++)
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

    for(sal_uInt32 a(0L); a < rPoly.count(); a++)
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
        const ::basegfx::B2DPolygon aToBeCopied(rBigger.getB2DPolygon(rSmaller.count()));
        const ::basegfx::B2DRange aToBeCopiedPolySize(::basegfx::tools::getRange(aToBeCopied));
        ::basegfx::B2DPoint aNewPoint(aToBeCopiedPolySize.getCenter());
        ::basegfx::B2DPolygon aNewPoly;

        const ::basegfx::B2DRange aSrcSize(::basegfx::tools::getRange(rBigger.getB2DPolygon(0L)));
        const ::basegfx::B2DPoint aSrcPos(aSrcSize.getCenter());
        const ::basegfx::B2DRange aDstSize(::basegfx::tools::getRange(rSmaller.getB2DPolygon(0L)));
        const ::basegfx::B2DPoint aDstPos(aDstSize.getCenter());
        aNewPoint = aNewPoint - aSrcPos + aDstPos;

        for(sal_uInt32 a(0L); a < aToBeCopied.count(); a++)
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
    sal_Bool bAttributeFade,
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
    SfxItemPool*        pPool = pObj1->GetObjectItemPool();
    SfxItemSet          aSet1( *pPool,SDRATTR_START,SDRATTR_NOTPERSIST_FIRST-1,EE_ITEMS_START,EE_ITEMS_END,0 );
    SfxItemSet          aSet2( aSet1 );
    sal_Bool            bLineColor = sal_False;
    sal_Bool            bFillColor = sal_False;
    sal_Bool            bLineWidth = sal_False;
    sal_Bool            bIgnoreLine = sal_False;
    sal_Bool            bIgnoreFill = sal_False;

    aSet1.Put(pObj1->GetMergedItemSet());
    aSet2.Put(pObj2->GetMergedItemSet());

    const XLineStyle eLineStyle1 = ITEMVALUE( aSet1, XATTR_LINESTYLE, XLineStyleItem );
    const XLineStyle eLineStyle2 = ITEMVALUE( aSet2, XATTR_LINESTYLE, XLineStyleItem );
    const XFillStyle eFillStyle1 = ITEMVALUE( aSet1, XATTR_FILLSTYLE, XFillStyleItem );
    const XFillStyle eFillStyle2 = ITEMVALUE( aSet2, XATTR_FILLSTYLE, XFillStyleItem );

    if ( bAttributeFade )
    {
        if ( ( eLineStyle1 != XLINE_NONE ) && ( eLineStyle2 != XLINE_NONE ) )
        {
            bLineWidth = bLineColor = sal_True;

            aStartLineCol = static_cast< XLineColorItem const & >(
                aSet1.Get(XATTR_LINECOLOR)).GetColorValue();
            aEndLineCol = static_cast< XLineColorItem const & >(
                aSet2.Get(XATTR_LINECOLOR)).GetColorValue();

            nStartLineWidth = ITEMVALUE( aSet1, XATTR_LINEWIDTH, XLineWidthItem );
            nEndLineWidth = ITEMVALUE( aSet2, XATTR_LINEWIDTH, XLineWidthItem );
        }
        else if ( ( eLineStyle1 == XLINE_NONE ) && ( eLineStyle2 == XLINE_NONE ) )
            bIgnoreLine = sal_True;

        if ( ( eFillStyle1 == XFILL_SOLID ) && ( eFillStyle2 == XFILL_SOLID ) )
        {
            bFillColor = sal_True;
            aStartFillCol = static_cast< XFillColorItem const & >(
                aSet1.Get(XATTR_FILLCOLOR)).GetColorValue();
            aEndFillCol = static_cast< XFillColorItem const & >(
                aSet2.Get(XATTR_FILLCOLOR)).GetColorValue();
        }
        else if ( ( eFillStyle1 == XFILL_NONE ) && ( eFillStyle2 == XFILL_NONE ) )
            bIgnoreFill = sal_True;
    }

    if ( pPageView )
    {
        SfxItemSet      aSet( aSet1 );
        SdrObjGroup*    pObjGroup = new SdrObjGroup;
        SdrObjList*     pObjList = pObjGroup->GetSubList();
        const size_t    nCount = rPolyPolyList3D.size();
        const double    fStep = 1. / ( nCount + 1 );
        const double    fDelta = nEndLineWidth - nStartLineWidth;
        double          fFactor = fStep;

        aSet.Put( XLineStyleItem( XLINE_SOLID ) );
        aSet.Put( XFillStyleItem( XFILL_SOLID ) );

        for ( size_t i = 0; i < nCount; i++, fFactor += fStep )
        {
            const ::basegfx::B2DPolyPolygon& rPolyPoly3D = *rPolyPolyList3D[ i ];
            SdrPathObj* pNewObj = new SdrPathObj(OBJ_POLY, rPolyPoly3D);

            // line color
            if ( bLineColor )
            {
                const basegfx::BColor aLineColor(basegfx::interpolate(aStartLineCol.getBColor(), aEndLineCol.getBColor(), fFactor));
                aSet.Put( XLineColorItem( aEmptyStr, Color(aLineColor)));
            }
            else if ( bIgnoreLine )
                aSet.Put( XLineStyleItem( XLINE_NONE ) );

            // fill color
            if ( bFillColor )
            {
                const basegfx::BColor aFillColor(basegfx::interpolate(aStartFillCol.getBColor(), aEndFillCol.getBColor(), fFactor));
                aSet.Put( XFillColorItem( aEmptyStr, Color(aFillColor)));
            }
            else if ( bIgnoreFill )
                aSet.Put( XFillStyleItem( XFILL_NONE ) );

            // line width
            if ( bLineWidth )
                aSet.Put( XLineWidthItem( nStartLineWidth + (long) ( fFactor * fDelta + 0.5 ) ) );

            pNewObj->SetMergedItemSetAndBroadcast(aSet);

            pObjList->InsertObject( pNewObj, LIST_APPEND );
        }

        if ( nCount )
        {
            pObjList->InsertObject( pObj1->Clone(), 0 );
            pObjList->InsertObject( pObj2->Clone(), LIST_APPEND );
            mpView->DeleteMarked();
            mpView->InsertObjectAtView( pObjGroup, *pPageView, SDRINSERT_SETDEFLAYER );
        }
    }
}

/**
 * create single morphed PolyPolygon
 */
::basegfx::B2DPolyPolygon* FuMorph::ImpCreateMorphedPolygon(
    const ::basegfx::B2DPolyPolygon& rPolyPolyStart,
    const ::basegfx::B2DPolyPolygon& rPolyPolyEnd,
    double fMorphingFactor
)
{
    ::basegfx::B2DPolyPolygon* pNewPolyPolygon = new ::basegfx::B2DPolyPolygon();
    const double fFactor = 1.0 - fMorphingFactor;

    for(sal_uInt32 a(0L); a < rPolyPolyStart.count(); a++)
    {
        const ::basegfx::B2DPolygon aPolyStart(rPolyPolyStart.getB2DPolygon(a));
        const ::basegfx::B2DPolygon aPolyEnd(rPolyPolyEnd.getB2DPolygon(a));
        const sal_uInt32 nCount(aPolyStart.count());
        ::basegfx::B2DPolygon aNewPolygon;

        for(sal_uInt32 b(0L); b < nCount; b++)
        {
            const ::basegfx::B2DPoint& aPtStart(aPolyStart.getB2DPoint(b));
            const ::basegfx::B2DPoint& aPtEnd(aPolyEnd.getB2DPoint(b));
            aNewPolygon.append(aPtEnd + ((aPtStart - aPtEnd) * fFactor));
        }

        aNewPolygon.setClosed(aPolyStart.isClosed() && aPolyEnd.isClosed());
        pNewPolyPolygon->append(aNewPolygon);
    }

    return pNewPolyPolygon;
}

/**
 * create morphed PolyPolygons
 */
sal_Bool FuMorph::ImpMorphPolygons(
    const ::basegfx::B2DPolyPolygon& rPolyPoly1,
    const ::basegfx::B2DPolyPolygon& rPolyPoly2,
    const sal_uInt16 nSteps,
    B2DPolyPolygonList_impl& rPolyPolyList3D
)
{
    if(nSteps)
    {
        const ::basegfx::B2DRange aStartPolySize(::basegfx::tools::getRange(rPolyPoly1));
        const ::basegfx::B2DPoint aStartCenter(aStartPolySize.getCenter());
        const ::basegfx::B2DRange aEndPolySize(::basegfx::tools::getRange(rPolyPoly2));
        const ::basegfx::B2DPoint aEndCenter(aEndPolySize.getCenter());
        const ::basegfx::B2DPoint aDelta(aEndCenter - aStartCenter);
        const double fFactor(1.0 / (nSteps + 1));
        double fValue(0.0);

        for(sal_uInt16 i(0); i < nSteps; i++)
        {
            fValue += fFactor;
            ::basegfx::B2DPolyPolygon* pNewPolyPoly2D = ImpCreateMorphedPolygon(rPolyPoly1, rPolyPoly2, fValue);

            const ::basegfx::B2DRange aNewPolySize(::basegfx::tools::getRange(*pNewPolyPoly2D));
            const ::basegfx::B2DPoint aNewS(aNewPolySize.getCenter());
            const ::basegfx::B2DPoint aRealS(aStartCenter + (aDelta * fValue));
            const ::basegfx::B2DPoint aDiff(aRealS - aNewS);

            pNewPolyPoly2D->transform(basegfx::tools::createTranslateB2DHomMatrix(aDiff));
            rPolyPolyList3D.push_back( pNewPolyPoly2D );
        }
    }
    return sal_True;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
