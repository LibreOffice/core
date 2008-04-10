/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: b3dcommn.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _B3D_B3DCOMMN_HXX
#define _B3D_B3DCOMMN_HXX

#include <goodies/base3d.hxx>
#include <goodies/b3dgeom.hxx>

// Defines fuer clipping flags (nFlag0,1)
#define CLIPFLAG_LEFT           0x0001
#define CLIPFLAG_RIGHT          0x0002
#define CLIPFLAG_BOTTOM         0x0004
#define CLIPFLAG_TOP            0x0008
#define CLIPFLAG_FRONT          0x0010
#define CLIPFLAG_BACK           0x0020
#define CLIPFLAG_ALL            (CLIPFLAG_LEFT|CLIPFLAG_RIGHT| \
                                CLIPFLAG_BOTTOM|CLIPFLAG_TOP| \
                                CLIPFLAG_FRONT|CLIPFLAG_BACK)

/*************************************************************************
|*
|* Bucket fuer Indices
|*
\************************************************************************/

BASE3D_DECL_BUCKET(sal_uInt32, Bucket)

/*************************************************************************
|*
|* Die Basisklasse fuer Standard 3D Ausgaben auf StarView Basis
|*
\************************************************************************/

#define BUFFER_OVERHEAD             (20)

class Base3DCommon : public Base3D
{
protected:
    // Buffers fuer temporaere geometrische Daten
    B3dEntityBucket         aBuffers;

    // Remember if last primitive was rejected
    unsigned                bLastPrimitiveRejected  : 1;

    // #93184# flag for polygon normal direction
    unsigned                bNormalPointsAway       : 1;

public:
    Base3DCommon(OutputDevice* pOutDev);
    virtual ~Base3DCommon();

    // Beleuchtung setzen/lesen
    virtual void SetLightGroup(B3dLightGroup* pSet, sal_Bool bSetGlobal=sal_True);

    // Info if last primitive was rejected
    sal_Bool WasLastPrimitiveRejected()
        { return bLastPrimitiveRejected; }

    // Szenenverwaltung
    virtual void StartScene();
    virtual void EndScene();

protected:
    // Geometrische Daten uebergeben
    virtual B3dEntity& ImplGetFreeEntity();

    virtual void ImplStartPrimitive();
    virtual void ImplEndPrimitive();
    virtual void ImplPostAddVertex(B3dEntity& rEnt);

    void Create3DPoint(sal_uInt32 nInd);
    void Create3DPointClipped(sal_uInt32 nInd);
    void Create3DLine(sal_uInt32 nInd1, sal_uInt32 nInd2);
    void Create3DLineClipped(sal_uInt32 nInd1, sal_uInt32 nInd2);
    void Create3DTriangle(sal_uInt32 nInd1, sal_uInt32 nInd2, sal_uInt32 nInd3);

    virtual void Clipped3DPoint(sal_uInt32 nInd) = 0;
    virtual void Clipped3DLine(sal_uInt32 nInd1,sal_uInt32 nInd2) = 0;
    virtual void Clipped3DTriangle(sal_uInt32 nInd1,sal_uInt32 nInd2, sal_uInt32 nInd3) = 0;

    // clipping functions
    sal_Bool AreEqual(sal_uInt32 nInd1, sal_uInt32 nInd2);
    sal_Bool Clip3DPoint(sal_uInt32 nInd);
    sal_Bool Clip3DLine(sal_uInt32& nInd1,sal_uInt32& nInd2);
    sal_Bool Clip3DPolygon(sal_uInt32Bucket& rEdgeIndex);
    sal_uInt16 GetClipFlags(sal_uInt32 nInd);
    sal_Bool IsInside(sal_uInt32 nInd, sal_uInt32 nDim, sal_Bool bLow);
    void ClipPoly(sal_uInt32Bucket& rEdgeIndex, sal_uInt16 nDim,sal_Bool bLow);
    void CalcNewPoint(sal_uInt32 nNew,sal_uInt32 nHigh,sal_uInt32 nLow,
        sal_uInt16 nDim, double fBound);

    // Beleuchtungsmodell (ColorModel) in einem Punkt loesen
    // Punkt MUSS in ClipCoordinates vorliegen !
    void SolveColorModel(B3dColor& rCol, basegfx::B3DVector& rVec, const basegfx::B3DPoint& rPnt);
    B3dColor SolveColorModel(B3dMaterial& rMat, basegfx::B3DVector& rVec, const basegfx::B3DPoint& rPnt);

    // Beleuchtungsmodell (ColorModel) fuer eine Lichtquelle loesen
    B3dColor SolveColorModel(B3dLight& rLight, B3dMaterial& rMat, basegfx::B3DVector& rVec, const basegfx::B3DPoint& rPnt);
};

#endif          // _B3D_B3DCOMMN_HXX
