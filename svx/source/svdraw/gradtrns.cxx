/*************************************************************************
 *
 *  $RCSfile: gradtrns.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:24 $
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

#include "gradtrns.hxx"
#include "svdobj.hxx"

#ifndef _B3D_B3DCOLOR_HXX
#include <goodies/b3dcolor.hxx>
#endif

#ifndef _B2D_MATRIX3D_HXX
#include <goodies/matrix3d.hxx>
#endif

void GradTransformer::CreateRotatedBoundRect(double fDegree)
{
    // create back-rotated BoundRect to get the vector length
    Vector2D aMid(aCenter);
    Matrix3D aTrans;
    Rectangle aNewRect;

    aTrans.Translate(-aMid);
    aTrans.Rotate(fDegree);
    aTrans.Translate(aMid);

    Vector2D aNew(aBoundRect.TopLeft());
    aNew *= aTrans;
    Point aNewTrans((INT32)(aNew.X() + 0.5), (INT32)(aNew.Y() + 0.5));
    aNewRect.Union(Rectangle(aNewTrans, aNewTrans));

    aNew = Vector2D(aBoundRect.TopRight());
    aNew *= aTrans;
    aNewTrans = Point((INT32)(aNew.X() + 0.5), (INT32)(aNew.Y() + 0.5));
    aNewRect.Union(Rectangle(aNewTrans, aNewTrans));

    aNew = Vector2D(aBoundRect.BottomLeft());
    aNew *= aTrans;
    aNewTrans = Point((INT32)(aNew.X() + 0.5), (INT32)(aNew.Y() + 0.5));
    aNewRect.Union(Rectangle(aNewTrans, aNewTrans));

    aNew = Vector2D(aBoundRect.BottomRight());
    aNew *= aTrans;
    aNewTrans = Point((INT32)(aNew.X() + 0.5), (INT32)(aNew.Y() + 0.5));
    aNewRect.Union(Rectangle(aNewTrans, aNewTrans));

    aBoundRect = aNewRect;
    aCenter = aBoundRect.Center();
}

//
// gradient to vector representation
//

void GradTransformer::GradToVec(GradTransGradient& rG, GradTransVector& rV, const SdrObject* pObj)
{
    // calc colors and set them
    rV.aCol1 = rG.aGradient.GetStartColor();
    rV.aCol2 = rG.aGradient.GetEndColor();

    if(rG.aGradient.GetStartIntens() != 100)
    {
        double fFact = (double)rG.aGradient.GetStartIntens() / 100.0;
        rV.aCol1 = (B3dColor)rV.aCol1 * fFact;
    }

    if(rG.aGradient.GetEndIntens() != 100)
    {
        double fFact = (double)rG.aGradient.GetEndIntens() / 100.0;
        rV.aCol2 = (B3dColor)rV.aCol2 * fFact;
    }

    // now calc the positions for the color handles
    aBoundRect = pObj->GetBoundRect();
    aCenter = aBoundRect.Center();
    fObjectAngle = 0.0;
    fGradientAngle = 0.0;

    // gradient rotates with the object (!!!)
    if(pObj->GetRotateAngle())
        fObjectAngle = (double)(pObj->GetRotateAngle() / 100) * F_PI180;

    // get gradient angle
    if(rG.aGradient.GetAngle())
        fGradientAngle = (double)rG.aGradient.GetAngle() * (F_PI180 / 10.0);

    // correct object and gradient rotation for object boundrect
    if(fGradientAngle || fObjectAngle)
        CreateRotatedBoundRect(fGradientAngle + fObjectAngle);

    // build the in-between vector (top-down)
    bInnerOuterChanged = FALSE;
    bDoCorrectOffset = FALSE;

    switch(rG.aGradient.GetGradientStyle())
    {
        case XGRAD_LINEAR :
        {
            // vector from top to bottom
            aStartPos = Vector2D(aCenter.X(), aBoundRect.Top());
            aEndPos = Vector2D(aCenter.X(), aBoundRect.Bottom());
            aCenterPos = Vector2D(aCenter);
            break;
        }
        case XGRAD_AXIAL :
        {
            // mirrored in center; set starpos, endpos from center to bottom
            aStartPos = Vector2D(aCenter);
            aEndPos = Vector2D(aCenter.X(), aBoundRect.Bottom());
            aCenterPos = Vector2D(aCenter);

            // here the border correction is vice-versa to the others
            bInnerOuterChanged = TRUE;

            break;
        }
        case XGRAD_RADIAL :
        {
            // from bottom to center
            aStartPos = Vector2D(aCenter.X(), aBoundRect.Bottom());
            aEndPos = Vector2D(aCenter);
            aCenterPos = Vector2D(aCenter);
            bDoCorrectOffset = TRUE;
            break;
        }
        case XGRAD_ELLIPTICAL :
        case XGRAD_SQUARE :
        case XGRAD_RECT :
        {
            // from bottom to center
            aStartPos = Vector2D(aCenter.X(), aBoundRect.Bottom());
            aEndPos = Vector2D(aCenter);
            aCenterPos = Vector2D(aCenter);
            bDoCorrectOffset = TRUE;
            break;
        }
    }

    // take in account border settings, this moves the start point
    if(rG.aGradient.GetBorder())
    {
        Vector2D aFullVec;

        if(bInnerOuterChanged)
            aFullVec = aEndPos - aStartPos;
        else
            aFullVec = aStartPos - aEndPos;

        double fLen = (aFullVec.GetLength() * (100.0 - (double)rG.aGradient.GetBorder())) / 100.0;
        aFullVec.Normalize();
        aFullVec *= fLen;

        if(bInnerOuterChanged)
            aEndPos = aStartPos + aFullVec;
        else
            aStartPos = aEndPos + aFullVec;
    }

    // take offset in account
    if(bDoCorrectOffset)
    {
        if(rG.aGradient.GetXOffset() != 50 || rG.aGradient.GetYOffset() != 50)
        {
            // get scaling factors (with and without gradient rotation),
            // center is the same
            Rectangle aObjectBoundRect = pObj->GetBoundRect();
            Vector2D aScale;
            aScale.X() = ((double)aBoundRect.GetWidth() + 1.0) / ((double)aObjectBoundRect.GetWidth() + 1.0);
            aScale.Y() = ((double)aBoundRect.GetHeight() + 1.0) / ((double)aObjectBoundRect.GetHeight() + 1.0);

            // calc transformation from gradient system to object system
            Matrix3D aTrans;
            Vector2D aObjectCenter = Vector2D(aBoundRect.Center());
            aTrans.Translate(-aObjectCenter);
            aTrans.Scale(aScale);
            aTrans.Rotate(-fObjectAngle);
            aTrans.Translate(aObjectCenter);

            // define coordinate system on top of ObjectBoundRect
            Vector2D aAxisCenter(aObjectBoundRect.TopLeft());
            Vector2D aXAxis(aObjectBoundRect.TopRight());
            Vector2D aYAxis(aObjectBoundRect.BottomLeft());

            // transform all to object system
            aAxisCenter *= aTrans;
            aXAxis *= aTrans;
            aYAxis *= aTrans;
            aXAxis -= aAxisCenter;
            aYAxis -= aAxisCenter;

            Vector2D aCorrectionVector(0,0);

            if(rG.aGradient.GetXOffset() != 50)
                aCorrectionVector += aXAxis * (((double)rG.aGradient.GetXOffset() - 50.0) / 100.0);

            if(rG.aGradient.GetYOffset() != 50)
                aCorrectionVector += aYAxis * (((double)rG.aGradient.GetYOffset() - 50.0) / 100.0);

            aStartPos += aCorrectionVector;
            aEndPos += aCorrectionVector;
            aCenterPos += aCorrectionVector;
        }
    }

    // rotate around fObjectAngle, go to rotated object coordinates
    if(fObjectAngle || fGradientAngle)
    {
        Matrix3D aTrans;

        aTrans.Translate(-aCenterPos);
        aTrans.Rotate(-(fObjectAngle + fGradientAngle));
        aTrans.Translate(aCenterPos);

        aStartPos *= aTrans;
        aEndPos *= aTrans;
    }

    // set values for vector positions now
    rV.aPos1 = Point((INT32)(aStartPos.X() + 0.5), (INT32)(aStartPos.Y() + 0.5));
    rV.aPos2 = Point((INT32)(aEndPos.X() + 0.5), (INT32)(aEndPos.Y() + 0.5));
}

//
// vector to gradient representation
//

void GradTransformer::VecToGrad(GradTransVector& rV, GradTransGradient& rG, GradTransGradient& rGOld, const SdrObject* pObj, BOOL bMoveSingle, BOOL bMoveFirst)
{
    // copy old item to new item to have a valid base
    rG = rGOld;

    // on this base, create aVOld to have the previous values
    // use full vector here (but rescue OldBorder)
    GradTransVector aVOld;
    UINT16 nOldBorder = rGOld.aGradient.GetBorder();
    rGOld.aGradient.SetBorder(0);
    GradToVec(rGOld, aVOld, pObj);
    rGOld.aGradient.SetBorder(nOldBorder);

    // color changes?
    if(rV.aCol1 != rGOld.aGradient.GetStartColor())
    {
        rG.aGradient.SetStartColor(rV.aCol1);
        rG.aGradient.SetStartIntens(100);
    }

    if(rV.aCol2 != rGOld.aGradient.GetEndColor())
    {
        rG.aGradient.SetEndColor(rV.aCol2);
        rG.aGradient.SetEndIntens(100);
    }

    // back-rotate around aCenterPos with fGradientAngle and fObjectAngle
    // to have the same conditions as above
    aStartPos = Vector2D(rV.aPos1);
    aEndPos = Vector2D(rV.aPos2);

    // which points were moved? What needs to be done?
    BOOL bRotAngleCheck(FALSE);
    BOOL bCenterMoveCheck(FALSE);
    BOOL bBorderMoveCheck(FALSE);
    BOOL bRotAngleInvert(FALSE);

    switch(rGOld.aGradient.GetGradientStyle())
    {
        case XGRAD_LINEAR :
        {
            bRotAngleCheck = bMoveSingle && !bMoveFirst;
            bBorderMoveCheck = bMoveSingle && bMoveFirst;
            break;
        }
        case XGRAD_AXIAL :
        {
            bRotAngleCheck = bMoveSingle && !bMoveFirst;
            bBorderMoveCheck = bMoveSingle && !bMoveFirst;
            break;
        }
        case XGRAD_RADIAL :
        {
            bBorderMoveCheck = bMoveSingle && bMoveFirst;
            bCenterMoveCheck = (bMoveSingle && !bMoveFirst) || !bMoveSingle;
            break;
        }
        case XGRAD_ELLIPTICAL :
        case XGRAD_SQUARE :
        case XGRAD_RECT :
        {
            bRotAngleCheck = bMoveSingle && bMoveFirst;
            bBorderMoveCheck = bMoveSingle && bMoveFirst;
            bCenterMoveCheck = (bMoveSingle && !bMoveFirst) || !bMoveSingle;
            bRotAngleInvert = TRUE;
            break;
        }
    }

    // check for rotation angle change
    if(bRotAngleCheck)
    {
        Vector2D aNewFullVec(aEndPos - aStartPos);

        if(bRotAngleInvert)
            aNewFullVec = -aNewFullVec;

        double fNewFullAngle = atan2(aNewFullVec.Y(), aNewFullVec.X());
        fNewFullAngle /= F_PI180;
        fNewFullAngle *= -10.0;
        fNewFullAngle += 900.0;

        // correct object rotation
        INT32 nObjRotateAngle = pObj->GetRotateAngle();
        if(nObjRotateAngle)
            fNewFullAngle -= (double)(nObjRotateAngle / 10);

        while(fNewFullAngle < 0.0)
            fNewFullAngle += 3600.0;

        while(fNewFullAngle >= 3600.0)
            fNewFullAngle -= 3600.0;

        INT32 nNewAngle = (INT32)(fNewFullAngle + 0.5);

        if(nNewAngle != rGOld.aGradient.GetAngle())
            rG.aGradient.SetAngle(nNewAngle);
    }

    // check for border, move of the outer pixel
    if(bBorderMoveCheck)
    {
        Vector2D aNewFullVec(aEndPos - aStartPos);
        Vector2D aOldFullVec(Vector2D(aVOld.aPos2) - Vector2D(aVOld.aPos1));
        double fNewFullLen = aNewFullVec.GetLength();
        double fOldFullLen = aOldFullVec.GetLength();

        double fNewBorder = (fNewFullLen * 100.0) / fOldFullLen;
        INT32 nNewBorder = 100 - (INT32)(fNewBorder + 0.5);

        if(nNewBorder < 0)
            nNewBorder = 0;

        if(nNewBorder > 100)
            nNewBorder = 100;

        if(nNewBorder != rG.aGradient.GetBorder())
        {
            rG.aGradient.SetBorder((UINT16)nNewBorder);
        }
    }

    // move center if allowed, move of the inner pixel
    if(bCenterMoveCheck)
    {
        // get scaling factors (with and without gradient rotation),
        // center is the same
        Rectangle aObjectBoundRect = pObj->GetBoundRect();
        Vector2D aScale;
        aScale.X() = ((double)aBoundRect.GetWidth() + 1.0) / ((double)aObjectBoundRect.GetWidth() + 1.0);
        aScale.Y() = ((double)aBoundRect.GetHeight() + 1.0) / ((double)aObjectBoundRect.GetHeight() + 1.0);

        // calc transformation from this system to gradient system
        Matrix3D aTrans;
        Vector2D aObjectCenter = Vector2D(aBoundRect.Center());
        aTrans.Translate(-aObjectCenter);
        aTrans.Scale(aScale);
        aTrans.Rotate(-fObjectAngle);
        aTrans.Translate(aObjectCenter);

        // define coordinate system on top of ObjectBoundRect
        Vector2D aAxisCenter(aObjectBoundRect.TopLeft());
        Vector2D aXAxis(aObjectBoundRect.TopRight());
        Vector2D aYAxis(aObjectBoundRect.BottomLeft());

        // transform all to gradient system
        aAxisCenter *= aTrans;
        aXAxis *= aTrans;
        aYAxis *= aTrans;
        aXAxis -= aAxisCenter;
        aYAxis -= aAxisCenter;

        // build the normalized coordinate system
        double fXAxisLen = aXAxis.GetLength();
        double fYAxisLen = aYAxis.GetLength();
        aXAxis.Normalize();
        aYAxis.Normalize();

        // calc part of new position in coordinate system
        Vector2D aNewPos(aEndPos - aAxisCenter);
        double fNewXAxisPart = aXAxis.Scalar(aNewPos);
        double fNewYAxisPart = aYAxis.Scalar(aNewPos);
        fNewXAxisPart = (fNewXAxisPart * 100.0) / fXAxisLen;
        fNewYAxisPart = (fNewYAxisPart * 100.0) / fYAxisLen;
        INT32 nNewX = (INT32)(fNewXAxisPart + 0.5);
        INT32 nNewY = (INT32)(fNewYAxisPart + 0.5);

        if(nNewX < 0)
            nNewX = 0;

        if(nNewX > 100)
            nNewX = 100;

        if(nNewY < 0)
            nNewY = 0;

        if(nNewY > 100)
            nNewY = 100;

        if(rGOld.aGradient.GetXOffset() != (UINT16)nNewX
            || rGOld.aGradient.GetYOffset() != (UINT16)nNewY)
        {
            rG.aGradient.SetXOffset((UINT16)nNewX);
            rG.aGradient.SetYOffset((UINT16)nNewY);
        }
    }
}

