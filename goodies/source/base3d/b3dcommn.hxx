/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b3dcommn.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 16:06:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _B3D_B3DCOMMN_HXX
#define _B3D_B3DCOMMN_HXX

#ifndef _B3D_BASE3D_HXX
#include "base3d.hxx"
#endif

#ifndef _B3D_B3DGEOM_HXX
#include "b3dgeom.hxx"
#endif

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
