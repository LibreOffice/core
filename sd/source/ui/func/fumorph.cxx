/*************************************************************************
 *
 *  $RCSfile: fumorph.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2000-10-30 11:46:16 $
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

#ifndef _SVX_FILLITEM_HXX //autogen
#include <svx/xfillit.hxx>
#endif
#ifndef _SVX_XLINIIT_HXX //autogen
#include <svx/xlineit.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVDPOOL_HXX //autogen
#include <svx/svdpool.hxx>
#endif
#ifndef _SV_POLY_HXX //autogen
#include <vcl/poly.hxx>
#endif
#ifndef _SVDOPATH_HXX //autogen
#include <svx/svdopath.hxx>
#endif
#ifndef _SVDOGRP_HXX //autogen
#include <svx/svdogrp.hxx>
#endif
#include <svx/eeitem.hxx>

#pragma hdrstop

#include "sdview.hxx"
#include "viewshel.hxx"
#include "morphdlg.hxx"
#include "strings.hrc"
#include "sdresid.hxx"

#define _FUMORPH_PRIVATE
#include "fumorph.hxx"

#define  ITEMVALUE( ItemSet, Id, Cast ) ( ( (const Cast&) (ItemSet).Get( (Id) ) ).GetValue() )
TYPEINIT1( FuMorph, FuPoor );

//////////////////////////////////////////////////////////////////////////////
// constructor
//
FuMorph::FuMorph(SdViewShell* pViewSh, SdWindow* pWin, SdView* pView,
    SdDrawDocument* pDoc, SfxRequest& rReq )
:   FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
    const SdrMarkList&  rMarkList = pView->GetMarkList();

    if(rMarkList.GetMarkCount() == 2)
    {
        // Clones erzeugen
        SdrObject*  pObj1 = rMarkList.GetMark(0)->GetObj();
        SdrObject*  pObj2 = rMarkList.GetMark(1)->GetObj();
        SdrObject*  pCloneObj1 = pObj1->Clone();
        SdrObject*  pCloneObj2 = pObj2->Clone();

        // Text am Clone loeschen, da wir sonst kein richtiges PathObj bekommen
        pCloneObj1->SetOutlinerParaObject(NULL);
        pCloneObj2->SetOutlinerParaObject(NULL);

        // Path-Objekte erzeugen
        SdrPathObj* pPolyObj1 = (SdrPathObj*)pCloneObj1->ConvertToPolyObj(FALSE, FALSE);
        SdrPathObj* pPolyObj2 = (SdrPathObj*)pCloneObj2->ConvertToPolyObj(FALSE, FALSE);
        SdMorphDlg aDlg((Window*)pWindow, pObj1, pObj2);

        if(pPolyObj1 && pPolyObj2 && (aDlg.Execute() == RET_OK))
        {
            List aPolyPolyList3D;
            PolyPolygon3D aPolyPoly1(pPolyObj1->GetPathPoly());
            PolyPolygon3D aPolyPoly2(pPolyObj2->GetPathPoly());
            PolyPolygon3D* pPolyPoly;

            aDlg.SaveSettings();

            // Morphing durchfuehren
            if(aPolyPoly1.Count() && aPolyPoly2.Count())
            {
                aPolyPoly1.SetDirections();
                aPolyPoly1.RemoveDoublePoints();
                BOOL bIsClockwise1 = aPolyPoly1.IsClockwise();

                aPolyPoly2.SetDirections();
                aPolyPoly2.RemoveDoublePoints();
                BOOL bIsClockwise2 = aPolyPoly2.IsClockwise();

                // set same orientation
                if(bIsClockwise1 != bIsClockwise2)
                    aPolyPoly2.FlipDirections();

                // force same poly count
                if(aPolyPoly1.Count() < aPolyPoly2.Count())
                    ImpAddPolys(aPolyPoly1, aPolyPoly2);
                else if(aPolyPoly2.Count() < aPolyPoly1.Count())
                    ImpAddPolys(aPolyPoly2, aPolyPoly1);

                // use orientation flag from dialog
                if(!aDlg.IsOrientationFade())
                    aPolyPoly2.FlipDirections();

                // force same point counts
                for(sal_uInt32 a(0L); a < aPolyPoly1.Count(); a++)
                {
                    if(aPolyPoly1[a].GetPointCount() < aPolyPoly2[a].GetPointCount())
                        ImpEqualizePolyPointCount(aPolyPoly1[a], aPolyPoly2[a]);
                    else if(aPolyPoly2[a].GetPointCount() < aPolyPoly1[a].GetPointCount())
                        ImpEqualizePolyPointCount(aPolyPoly2[a], aPolyPoly1[a]);
                }

                if(ImpMorphPolygons(aPolyPoly1, aPolyPoly2, aDlg.GetFadeSteps(), aPolyPolyList3D))
                {
                    String aString(pView->GetMarkDescription());

                    aString.Append(sal_Unicode(' '));
                    aString.Append(String(SdResId(STR_UNDO_MORPHING)));

                    pView->BegUndo(aString);
                    ImpInsertPolygons(aPolyPolyList3D, aDlg.IsAttributeFade(), pObj1, pObj2);
                    pView->EndUndo();
                }

                // erzeugte Polygone wieder loeschen
                for(pPolyPoly = (PolyPolygon3D*)aPolyPolyList3D.First(); pPolyPoly; pPolyPoly = (PolyPolygon3D*)aPolyPolyList3D.Next())
                    delete pPolyPoly;
            }
        }

        delete pCloneObj1;
        delete pCloneObj2;

        delete pPolyObj1;
        delete pPolyObj2;
    }
}

//////////////////////////////////////////////////////////////////////////////
// make the point count of the polygons equal in adding points
//
void FuMorph::ImpEqualizePolyPointCount(Polygon3D& rSmall, const Polygon3D& rBig)
{
    // create poly with equal point count
    const sal_uInt16 nCnt = rBig.GetPointCount();
    Polygon3D aPoly1 = rSmall.GetExpandedPolygon(nCnt);

    // create transformation for rBig to do the compare
    Volume3D aSrcSize = rBig.GetPolySize();
    Vector3D aSrcPos = (aSrcSize.MinVec() + aSrcSize.MaxVec()) / 2.0;
    Volume3D aDstSize = rSmall.GetPolySize();
    Vector3D aDstPos = (aDstSize.MinVec() + aDstSize.MaxVec()) / 2.0;
    Matrix4D aTrans;
    aTrans.Translate(-aSrcPos.X(), -aSrcPos.Y(), 0.0);
    aTrans.Scale(
        aDstSize.GetWidth()/aSrcSize.GetWidth(),
        aDstSize.GetHeight()/aSrcSize.GetHeight(), 1.0);
    aTrans.Translate(aDstPos.X(), aDstPos.Y(), 0.0);

    // transpose points to have smooth linear blending
    Polygon3D aPoly2(nCnt);
    sal_uInt16 nInd = ImpGetNearestIndex(aPoly1, aTrans * rBig[0]);
    for(sal_uInt16 a(0); a < nCnt; a++)
        aPoly2[(a + nCnt - nInd) % nCnt] = aPoly1[a];

    aPoly2.SetClosed(rBig.IsClosed());
    rSmall = aPoly2;
}

//////////////////////////////////////////////////////////////////////////////
//
sal_uInt16 FuMorph::ImpGetNearestIndex(const Polygon3D& rPoly, const Vector3D& rPos)
{
    double fMinDist;
    sal_uInt16 nActInd;

    for(sal_uInt16 a(0); a < rPoly.GetPointCount(); a++)
    {
        double fNewDist = (rPoly[a] - rPos).GetLength();
        if(!a || fNewDist < fMinDist)
        {
            fMinDist = fNewDist;
            nActInd = a;
        }
    }

    return nActInd;
}

//////////////////////////////////////////////////////////////////////////////
// add to a point reduced polys until count is same
//
void FuMorph::ImpAddPolys(PolyPolygon3D& rSmaller, const PolyPolygon3D& rBigger)
{
    while(rSmaller.Count() < rBigger.Count())
    {
        const Polygon3D& rToBeCopied = rBigger[rSmaller.Count()];
        Polygon3D aNewPoly(rToBeCopied.GetPointCount());
        Volume3D aToBeCopiedPolySize = rToBeCopied.GetPolySize();
        Vector3D aNewPoint = (aToBeCopiedPolySize.MinVec() + aToBeCopiedPolySize.MaxVec()) / 2.0;

        Volume3D aSrcSize = rBigger[0].GetPolySize();
        Vector3D aSrcPos = (aSrcSize.MinVec() + aSrcSize.MaxVec()) / 2.0;
        Volume3D aDstSize = rSmaller[0].GetPolySize();
        Vector3D aDstPos = (aDstSize.MinVec() + aDstSize.MaxVec()) / 2.0;

        aNewPoint = aNewPoint - aSrcPos + aDstPos;

        for(sal_uInt16 a(0); a < rToBeCopied.GetPointCount(); a++)
            aNewPoly[a] = aNewPoint;

        rSmaller.Insert(aNewPoly, POLYPOLY3D_APPEND);
    }
}

//////////////////////////////////////////////////////////////////////////////
// create group object with morphed polygons
//
void FuMorph::ImpInsertPolygons(List& rPolyPolyList3D, BOOL bAttributeFade,
    const SdrObject* pObj1, const SdrObject* pObj2)
{
    Color               aStartFillCol;
    Color               aEndFillCol;
    Color               aStartLineCol;
    Color               aEndLineCol;
    long                nStartLineWidth;
    long                nEndLineWidth;
    SdrPageView*        pPageView = pView->GetPageViewPvNum( 0 );
    SfxItemPool*        pPool = pObj1->GetItemPool();
    SfxItemSet          aSet1( *pPool,SDRATTR_START,SDRATTR_NOTPERSIST_FIRST-1,EE_ITEMS_START,EE_ITEMS_END,0 );
    SfxItemSet          aSet2( aSet1 );
    BOOL                bLineColor = FALSE;
    BOOL                bFillColor = FALSE;
    BOOL                bLineWidth = FALSE;
    BOOL                bIgnoreLine = FALSE;
    BOOL                bIgnoreFill = FALSE;

//-/    pObj1->TakeAttributes( aSet1, TRUE, FALSE );
    aSet1.Put(pObj1->GetItemSet());

//-/    pObj2->TakeAttributes( aSet2, TRUE, FALSE );
    aSet2.Put(pObj2->GetItemSet());

    const XLineStyle eLineStyle1 = ITEMVALUE( aSet1, XATTR_LINESTYLE, XLineStyleItem );
    const XLineStyle eLineStyle2 = ITEMVALUE( aSet2, XATTR_LINESTYLE, XLineStyleItem );
    const XFillStyle eFillStyle1 = ITEMVALUE( aSet1, XATTR_FILLSTYLE, XFillStyleItem );
    const XFillStyle eFillStyle2 = ITEMVALUE( aSet2, XATTR_FILLSTYLE, XFillStyleItem );

    if ( bAttributeFade )
    {
        if ( ( eLineStyle1 != XLINE_NONE ) && ( eLineStyle2 != XLINE_NONE ) )
        {
            bLineWidth = bLineColor = TRUE;

            aStartLineCol = ITEMVALUE( aSet1, XATTR_LINECOLOR, XLineColorItem );
            aEndLineCol = ITEMVALUE( aSet2, XATTR_LINECOLOR, XLineColorItem );

            nStartLineWidth = ITEMVALUE( aSet1, XATTR_LINEWIDTH, XLineWidthItem );
            nEndLineWidth = ITEMVALUE( aSet2, XATTR_LINEWIDTH, XLineWidthItem );
        }
        else if ( ( eLineStyle1 == XLINE_NONE ) && ( eLineStyle2 == XLINE_NONE ) )
            bIgnoreLine = TRUE;

        if ( ( eFillStyle1 == XFILL_SOLID ) && ( eFillStyle2 == XFILL_SOLID ) )
        {
            bFillColor = TRUE;
            aStartFillCol = ITEMVALUE( aSet1, XATTR_FILLCOLOR, XFillColorItem );
            aEndFillCol = ITEMVALUE( aSet2, XATTR_FILLCOLOR, XFillColorItem );
        }
        else if ( ( eFillStyle1 == XFILL_NONE ) && ( eFillStyle2 == XFILL_NONE ) )
            bIgnoreFill = TRUE;
    }

    if ( pPageView )
    {
        SfxItemSet      aSet( aSet1 );
        SdrObjGroup*    pObjGroup = new SdrObjGroup;
        SdrObjList*     pObjList = pObjGroup->GetSubList();
        const String    aEmptyStr;
        const ULONG     nCount = rPolyPolyList3D.Count();
        const double    fStep = 1. / ( nCount + 1 );
        const double    fDelta = nEndLineWidth - nStartLineWidth;
        double          fFactor = fStep;

        aSet.Put( XLineStyleItem( XLINE_SOLID ) );
        aSet.Put( XFillStyleItem( XFILL_SOLID ) );

        for ( ULONG i = 0; i < nCount; i++, fFactor += fStep )
        {
            const PolyPolygon3D& rPolyPoly3D = *(PolyPolygon3D*)rPolyPolyList3D.GetObject(i);
            SdrPathObj* pNewObj = new SdrPathObj(OBJ_POLY, rPolyPoly3D.GetXPolyPolygon());
            B3dColor aLineCol, aFillCol;
            aLineCol.CalcInBetween(aStartLineCol, aEndLineCol, fFactor);
            aFillCol.CalcInBetween(aStartFillCol, aEndFillCol, fFactor);

            // Linienfarbe
            if ( bLineColor )
                aSet.Put( XLineColorItem( aEmptyStr, aLineCol));
            else if ( bIgnoreLine )
                aSet.Put( XLineStyleItem( XLINE_NONE ) );

            // Fuellfarbe
            if ( bFillColor )
                aSet.Put( XFillColorItem( aEmptyStr, aFillCol));
            else if ( bIgnoreFill )
                aSet.Put( XFillStyleItem( XFILL_NONE ) );

            // Linienstaerke
            if ( bLineWidth )
                aSet.Put( XLineWidthItem( nStartLineWidth + (long) ( fFactor * fDelta + 0.5 ) ) );

//-/            pNewObj->SetAttributes( aSet, FALSE );
//-/            SdrBroadcastItemChange aItemChange(*pNewObj);
            pNewObj->SetItemSetAndBroadcast(aSet);
//-/            pNewObj->BroadcastItemChange(aItemChange);

            pObjList->InsertObject( pNewObj, LIST_APPEND );
        }

        if ( nCount )
        {
            pObjList->InsertObject( pObj1->Clone(), 0 );
            pObjList->InsertObject( pObj2->Clone(), LIST_APPEND );
            pView->DeleteMarked();
            pView->InsertObject ( pObjGroup, *pPageView, SDRINSERT_SETDEFLAYER );
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// create single morphed PolyPolygon
//
PolyPolygon3D* FuMorph::ImpCreateMorphedPolygon(
    const PolyPolygon3D& rPolyPolyStart,
    const PolyPolygon3D& rPolyPolyEnd,
    const double fMorphingFactor)
{
    PolyPolygon3D* pNewPolyPolygon = new PolyPolygon3D();
    const double fFactor = 1.0 - fMorphingFactor;

    for(sal_uInt16 a(0); a < rPolyPolyStart.Count(); a++)
    {
        const Polygon3D& rPolyStart = rPolyPolyStart[a];
        const Polygon3D& rPolyEnd = rPolyPolyEnd[a];
        const sal_uInt16 nCount = rPolyStart.GetPointCount();
        Polygon3D aNewPolygon(nCount);

        for(sal_uInt16 b(0); b < nCount; b++)
        {
            const Vector3D& rPtStart = rPolyStart[b];
            const Vector3D& rPtEnd = rPolyEnd[b];

            aNewPolygon[b] = rPtEnd + ((rPtStart - rPtEnd) * fFactor);
        }

        aNewPolygon.SetClosed(rPolyStart.IsClosed() && rPolyEnd.IsClosed());

        pNewPolyPolygon->Insert(aNewPolygon, POLYPOLY3D_APPEND);
    }

    return pNewPolyPolygon;
}

//////////////////////////////////////////////////////////////////////////////
// create morphed PolyPolygons
//
BOOL FuMorph::ImpMorphPolygons(
    const PolyPolygon3D& rPolyPoly1, const PolyPolygon3D& rPolyPoly2,
    const USHORT nSteps, List& rPolyPolyList3D)
{
    if(nSteps)
    {
        Volume3D aStartPolySize = rPolyPoly1.GetPolySize();
        Vector3D aStartCenter = (aStartPolySize.MinVec() + aStartPolySize.MaxVec()) / 2.0;
        Volume3D aEndPolySize = rPolyPoly2.GetPolySize();
        Vector3D aEndCenter = (aEndPolySize.MinVec() + aEndPolySize.MaxVec()) / 2.0;
        Vector3D aDelta = aEndCenter - aStartCenter;
        const double fFactor = 1.0/(nSteps+1);
        double fValue = 0.0;

        for(sal_uInt16 i(0); i < nSteps; i++)
        {
            fValue += fFactor;
            PolyPolygon3D* pNewPolyPoly3D = ImpCreateMorphedPolygon(rPolyPoly1, rPolyPoly2, fValue);

            Volume3D aNewPolySize = pNewPolyPoly3D->GetPolySize();
            Vector3D aNewS = (aNewPolySize.MinVec() + aNewPolySize.MaxVec()) / 2.0;
            Vector3D aRealS = aStartCenter + (aDelta * fValue);
            Matrix4D aTrans;
            Vector3D aDiff = aRealS - aNewS;
            aTrans.Translate(aDiff.X(), aDiff.Y(), aDiff.Z());
            pNewPolyPoly3D->Transform(aTrans);
            rPolyPolyList3D.Insert(pNewPolyPoly3D, LIST_APPEND);
        }
    }
    return TRUE;
}


