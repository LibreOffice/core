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


#include "gradtrns.hxx"
#include <svx/svdobj.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <tools/helpers.hxx>

//////////////////////////////////////////////////////////////////////////////

void GradTransformer::GradToVec(GradTransGradient& rG, GradTransVector& rV, const SdrObject* pObj)
{
    // handle start color
    rV.aCol1 = rG.aGradient.GetStartColor();
    if(100 != rG.aGradient.GetStartIntens())
    {
        const double fFact((double)rG.aGradient.GetStartIntens() / 100.0);
        rV.aCol1 = Color(rV.aCol1.getBColor() * fFact);
    }

    // handle end color
    rV.aCol2 = rG.aGradient.GetEndColor();
    if(100 != rG.aGradient.GetEndIntens())
    {
        const double fFact((double)rG.aGradient.GetEndIntens() / 100.0);
        rV.aCol2 = Color(rV.aCol2.getBColor() * fFact);
    }

    // calc the basic positions
    const Rectangle aObjectSnapRectangle(pObj->GetSnapRect());
    const basegfx::B2DRange aRange(aObjectSnapRectangle.Left(), aObjectSnapRectangle.Top(), aObjectSnapRectangle.Right(), aObjectSnapRectangle.Bottom());
    const basegfx::B2DPoint aCenter(aRange.getCenter());
    basegfx::B2DPoint aStartPos, aEndPos;

    switch(rG.aGradient.GetGradientStyle())
    {
        case XGRAD_LINEAR :
        {
            aStartPos = basegfx::B2DPoint(aCenter.getX(), aRange.getMinY());
            aEndPos = basegfx::B2DPoint(aCenter.getX(), aRange.getMaximum().getY());

            if(rG.aGradient.GetBorder())
            {
                basegfx::B2DVector aFullVec(aStartPos - aEndPos);
                const double fLen = (aFullVec.getLength() * (100.0 - (double)rG.aGradient.GetBorder())) / 100.0;
                aFullVec.normalize();
                aStartPos = aEndPos + (aFullVec * fLen);
            }

            if(rG.aGradient.GetAngle())
            {
                const double fAngle = (double)rG.aGradient.GetAngle() * (F_PI180 / 10.0);
                const basegfx::B2DHomMatrix aTransformation(basegfx::tools::createRotateAroundPoint(aCenter, -fAngle));

                aStartPos *= aTransformation;
                aEndPos *= aTransformation;
            }
            break;
        }
        case XGRAD_AXIAL :
        {
            aStartPos = aCenter;
            aEndPos = basegfx::B2DPoint(aCenter.getX(), aRange.getMaximum().getY());

            if(rG.aGradient.GetBorder())
            {
                basegfx::B2DVector aFullVec(aEndPos - aStartPos);
                const double fLen = (aFullVec.getLength() * (100.0 - (double)rG.aGradient.GetBorder())) / 100.0;
                aFullVec.normalize();
                aEndPos = aStartPos + (aFullVec * fLen);
            }

            if(rG.aGradient.GetAngle())
            {
                const double fAngle = (double)rG.aGradient.GetAngle() * (F_PI180 / 10.0);
                const basegfx::B2DHomMatrix aTransformation(basegfx::tools::createRotateAroundPoint(aCenter, -fAngle));

                aStartPos *= aTransformation;
                aEndPos *= aTransformation;
            }
            break;
        }
        case XGRAD_RADIAL :
        case XGRAD_SQUARE :
        {
            aStartPos = basegfx::B2DPoint(aRange.getMinX(), aRange.getMaximum().getY());
            aEndPos = basegfx::B2DPoint(aRange.getMinX(), aRange.getMinY());

            if(rG.aGradient.GetBorder())
            {
                basegfx::B2DVector aFullVec(aStartPos - aEndPos);
                const double fLen = (aFullVec.getLength() * (100.0 - (double)rG.aGradient.GetBorder())) / 100.0;
                aFullVec.normalize();
                aStartPos = aEndPos + (aFullVec * fLen);
            }

            if(rG.aGradient.GetAngle())
            {
                const double fAngle = (double)rG.aGradient.GetAngle() * (F_PI180 / 10.0);
                const basegfx::B2DHomMatrix aTransformation(basegfx::tools::createRotateAroundPoint(aEndPos, -fAngle));

                aStartPos *= aTransformation;
                aEndPos *= aTransformation;
            }

            if(rG.aGradient.GetXOffset() || rG.aGradient.GetYOffset())
            {
                basegfx::B2DPoint aOffset(
                    (aRange.getWidth() * rG.aGradient.GetXOffset()) / 100.0,
                    (aRange.getHeight() * rG.aGradient.GetYOffset()) / 100.0);

                aStartPos += aOffset;
                aEndPos += aOffset;
            }

            break;
        }
        case XGRAD_ELLIPTICAL :
        case XGRAD_RECT :
        {
            aStartPos = basegfx::B2DPoint(aRange.getMinX(), aCenter.getY());
            aEndPos = basegfx::B2DPoint(aRange.getMinX(), aRange.getMinY());

            if(rG.aGradient.GetBorder())
            {
                basegfx::B2DVector aFullVec(aStartPos - aEndPos);
                const double fLen = (aFullVec.getLength() * (100.0 - (double)rG.aGradient.GetBorder())) / 100.0;
                aFullVec.normalize();
                aStartPos = aEndPos + (aFullVec * fLen);
            }

            if(rG.aGradient.GetAngle())
            {
                const double fAngle = (double)rG.aGradient.GetAngle() * (F_PI180 / 10.0);
                const basegfx::B2DHomMatrix aTransformation(basegfx::tools::createRotateAroundPoint(aEndPos, -fAngle));

                aStartPos *= aTransformation;
                aEndPos *= aTransformation;
            }

            if(rG.aGradient.GetXOffset() || rG.aGradient.GetYOffset())
            {
                basegfx::B2DPoint aOffset(
                    (aRange.getWidth() * rG.aGradient.GetXOffset()) / 100.0,
                    (aRange.getHeight() * rG.aGradient.GetYOffset()) / 100.0);

                aStartPos += aOffset;
                aEndPos += aOffset;
            }

            break;
        }
    }

    // set values for vector positions now
    rV.maPositionA = aStartPos;
    rV.maPositionB = aEndPos;
}

//////////////////////////////////////////////////////////////////////////////

void GradTransformer::VecToGrad(GradTransVector& rV, GradTransGradient& rG, GradTransGradient& rGOld, const SdrObject* pObj,
    sal_Bool bMoveSingle, sal_Bool bMoveFirst)
{
    // fill old gradient to new gradient to have a base
    rG = rGOld;

    // handle color changes
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

    // calc the basic positions
    const Rectangle aObjectSnapRectangle(pObj->GetSnapRect());
    const basegfx::B2DRange aRange(aObjectSnapRectangle.Left(), aObjectSnapRectangle.Top(), aObjectSnapRectangle.Right(), aObjectSnapRectangle.Bottom());
    const basegfx::B2DPoint aCenter(aRange.getCenter());
    basegfx::B2DPoint aStartPos(rV.maPositionA);
    basegfx::B2DPoint aEndPos(rV.maPositionB);

    switch(rG.aGradient.GetGradientStyle())
    {
        case XGRAD_LINEAR :
        {
            if(!bMoveSingle || (bMoveSingle && !bMoveFirst))
            {
                basegfx::B2DVector aFullVec(aEndPos - aStartPos);

                if(bMoveSingle)
                {
                    aFullVec = aEndPos - aCenter;
                }

                aFullVec.normalize();

                double fNewFullAngle(atan2(aFullVec.getY(), aFullVec.getX()));
                fNewFullAngle /= F_PI180;
                fNewFullAngle *= -10.0;
                fNewFullAngle += 900.0;

                // clip
                while(fNewFullAngle < 0.0)
                {
                    fNewFullAngle += 3600.0;
                }

                while(fNewFullAngle >= 3600.0)
                {
                    fNewFullAngle -= 3600.0;
                }

                // to int and set
                sal_Int32 nNewAngle = FRound(fNewFullAngle);

                if(nNewAngle != rGOld.aGradient.GetAngle())
                {
                    rG.aGradient.SetAngle(nNewAngle);
                }
            }

            if(!bMoveSingle || (bMoveSingle && bMoveFirst))
            {
                const basegfx::B2DVector aFullVec(aEndPos - aStartPos);
                const basegfx::B2DPoint aBottomLeft(aRange.getMinX(), aRange.getMaximum().getY());
                const basegfx::B2DPoint aTopLeft(aRange.getMinX(), aRange.getMinY());
                const basegfx::B2DVector aOldVec(aBottomLeft - aTopLeft);
                const double fFullLen(aFullVec.getLength());
                const double fOldLen(aOldVec.getLength());
                const double fNewBorder((fFullLen * 100.0) / fOldLen);
                sal_Int32 nNewBorder(100L - FRound(fNewBorder));

                // clip
                if(nNewBorder < 0L)
                {
                    nNewBorder = 0L;
                }

                if(nNewBorder > 100L)
                {
                    nNewBorder = 100L;
                }

                // set
                if(nNewBorder != rG.aGradient.GetBorder())
                {
                    rG.aGradient.SetBorder((sal_uInt16)nNewBorder);
                }
            }

            break;
        }
        case XGRAD_AXIAL :
        {
            if(!bMoveSingle || (bMoveSingle && !bMoveFirst))
            {
                basegfx::B2DVector aFullVec(aEndPos - aCenter);
                const basegfx::B2DVector aOldVec(basegfx::B2DPoint(aCenter.getX(), aRange.getMaximum().getY()) - aCenter);
                const double fFullLen(aFullVec.getLength());
                const double fOldLen(aOldVec.getLength());
                const double fNewBorder((fFullLen * 100.0) / fOldLen);
                sal_Int32 nNewBorder = 100 - FRound(fNewBorder);

                // clip
                if(nNewBorder < 0L)
                {
                    nNewBorder = 0L;
                }

                if(nNewBorder > 100L)
                {
                    nNewBorder = 100L;
                }

                // set
                if(nNewBorder != rG.aGradient.GetBorder())
                {
                    rG.aGradient.SetBorder((sal_uInt16)nNewBorder);
                }

                aFullVec.normalize();
                double fNewFullAngle(atan2(aFullVec.getY(), aFullVec.getX()));
                fNewFullAngle /= F_PI180;
                fNewFullAngle *= -10.0;
                fNewFullAngle += 900.0;

                // clip
                while(fNewFullAngle < 0.0)
                {
                    fNewFullAngle += 3600.0;
                }

                while(fNewFullAngle >= 3600.0)
                {
                    fNewFullAngle -= 3600.0;
                }

                // to int and set
                const sal_Int32 nNewAngle(FRound(fNewFullAngle));

                if(nNewAngle != rGOld.aGradient.GetAngle())
                {
                    rG.aGradient.SetAngle(nNewAngle);
                }
            }

            break;
        }
        case XGRAD_RADIAL :
        case XGRAD_SQUARE :
        {
            if(!bMoveSingle || (bMoveSingle && !bMoveFirst))
            {
                const basegfx::B2DPoint aTopLeft(aRange.getMinX(), aRange.getMinY());
                const basegfx::B2DPoint aOffset(aEndPos - aTopLeft);
                sal_Int32 nNewXOffset(FRound((aOffset.getX() * 100.0) / aRange.getWidth()));
                sal_Int32 nNewYOffset(FRound((aOffset.getY() * 100.0) / aRange.getHeight()));

                // clip
                if(nNewXOffset < 0L)
                {
                    nNewXOffset = 0L;
                }

                if(nNewXOffset > 100L)
                {
                    nNewXOffset = 100L;
                }

                if(nNewYOffset < 0L)
                {
                    nNewYOffset = 0L;
                }

                if(nNewYOffset > 100L)
                {
                    nNewYOffset = 100L;
                }

                rG.aGradient.SetXOffset((sal_uInt16)nNewXOffset);
                rG.aGradient.SetYOffset((sal_uInt16)nNewYOffset);

                aStartPos -= aOffset;
                aEndPos -= aOffset;
            }

            if(!bMoveSingle || (bMoveSingle && bMoveFirst))
            {
                basegfx::B2DVector aFullVec(aStartPos - aEndPos);
                const basegfx::B2DPoint aBottomLeft(aRange.getMinX(), aRange.getMaximum().getY());
                const basegfx::B2DPoint aTopLeft(aRange.getMinX(), aRange.getMinY());
                const basegfx::B2DVector aOldVec(aBottomLeft - aTopLeft);
                const double fFullLen(aFullVec.getLength());
                const double fOldLen(aOldVec.getLength());
                const double fNewBorder((fFullLen * 100.0) / fOldLen);
                sal_Int32 nNewBorder(100L - FRound(fNewBorder));

                // clip
                if(nNewBorder < 0L)
                {
                    nNewBorder = 0L;
                }

                if(nNewBorder > 100L)
                {
                    nNewBorder = 100L;
                }

                // set
                if(nNewBorder != rG.aGradient.GetBorder())
                {
                    rG.aGradient.SetBorder((sal_uInt16)nNewBorder);
                }

                // angle is not definitely necessary for these modes, but it makes
                // controlling more fun for the user
                aFullVec.normalize();
                double fNewFullAngle(atan2(aFullVec.getY(), aFullVec.getX()));
                fNewFullAngle /= F_PI180;
                fNewFullAngle *= -10.0;
                fNewFullAngle += 900.0;

                // clip
                while(fNewFullAngle < 0.0)
                {
                    fNewFullAngle += 3600.0;
                }

                while(fNewFullAngle >= 3600.0)
                {
                    fNewFullAngle -= 3600.0;
                }

                // to int and set
                const sal_Int32 nNewAngle(FRound(fNewFullAngle));

                if(nNewAngle != rGOld.aGradient.GetAngle())
                {
                    rG.aGradient.SetAngle(nNewAngle);
                }
            }

            break;
        }
        case XGRAD_ELLIPTICAL :
        case XGRAD_RECT :
        {
            if(!bMoveSingle || (bMoveSingle && !bMoveFirst))
            {
                const basegfx::B2DPoint aTopLeft(aRange.getMinX(), aRange.getMinY());
                const basegfx::B2DPoint aOffset(aEndPos - aTopLeft);
                sal_Int32 nNewXOffset(FRound((aOffset.getX() * 100.0) / aRange.getWidth()));
                sal_Int32 nNewYOffset(FRound((aOffset.getY() * 100.0) / aRange.getHeight()));

                // clip
                if(nNewXOffset < 0L)
                {
                    nNewXOffset = 0L;
                }

                if(nNewXOffset > 100L)
                {
                    nNewXOffset = 100L;
                }

                if(nNewYOffset < 0L)
                {
                    nNewYOffset = 0L;
                }

                if(nNewYOffset > 100L)
                {
                    nNewYOffset = 100L;
                }

                rG.aGradient.SetXOffset((sal_uInt16)nNewXOffset);
                rG.aGradient.SetYOffset((sal_uInt16)nNewYOffset);

                aStartPos -= aOffset;
                aEndPos -= aOffset;
            }

            if(!bMoveSingle || (bMoveSingle && bMoveFirst))
            {
                basegfx::B2DVector aFullVec(aStartPos - aEndPos);
                const basegfx::B2DPoint aTopLeft(aRange.getMinX(), aRange.getMinY());
                const basegfx::B2DPoint aCenterLeft(aRange.getMinX(), aRange.getHeight());
                const basegfx::B2DVector aOldVec(aCenterLeft - aTopLeft);
                const double fFullLen(aFullVec.getLength());
                const double fOldLen(aOldVec.getLength());
                const double fNewBorder((fFullLen * 100.0) / fOldLen);
                sal_Int32 nNewBorder(100L - FRound(fNewBorder));

                // clip
                if(nNewBorder < 0L)
                {
                    nNewBorder = 0L;
                }

                if(nNewBorder > 100L)
                {
                    nNewBorder = 100L;
                }

                // set
                if(nNewBorder != rG.aGradient.GetBorder())
                {
                    rG.aGradient.SetBorder((sal_uInt16)nNewBorder);
                }

                // angle is not definitely necessary for these modes, but it makes
                // controlling more fun for the user
                aFullVec.normalize();
                double fNewFullAngle(atan2(aFullVec.getY(), aFullVec.getX()));
                fNewFullAngle /= F_PI180;
                fNewFullAngle *= -10.0;
                fNewFullAngle += 900.0;

                // clip
                while(fNewFullAngle < 0.0)
                {
                    fNewFullAngle += 3600.0;
                }

                while(fNewFullAngle >= 3600.0)
                {
                    fNewFullAngle -= 3600.0;
                }

                // to int and set
                const sal_Int32 nNewAngle(FRound(fNewFullAngle));

                if(nNewAngle != rGOld.aGradient.GetAngle())
                {
                    rG.aGradient.SetAngle(nNewAngle);
                }
            }

            break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
