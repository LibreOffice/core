/*************************************************************************
 *
 *  $RCSfile: b3dcommn.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:10 $
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

BASE3D_DECL_BUCKET(UINT32, Bucket)

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
    BOOL                    bLastPrimitiveRejected  : 1;

public:
    Base3DCommon(OutputDevice* pOutDev);
    virtual ~Base3DCommon();

    // Beleuchtung setzen/lesen
    virtual void SetLightGroup(B3dLightGroup* pSet, BOOL bSetGlobal=TRUE);

    // Info if last primitive was rejected
    BOOL WasLastPrimitiveRejected()
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

    void Create3DPoint(UINT32 nInd);
    void Create3DPointClipped(UINT32 nInd);
    void Create3DLine(UINT32 nInd1, UINT32 nInd2);
    void Create3DLineClipped(UINT32 nInd1, UINT32 nInd2);
    void Create3DTriangle(UINT32 nInd1, UINT32 nInd2, UINT32 nInd3);

    virtual void Clipped3DPoint(UINT32 nInd) = 0;
    virtual void Clipped3DLine(UINT32 nInd1,UINT32 nInd2) = 0;
    virtual void Clipped3DTriangle(UINT32 nInd1,UINT32 nInd2, UINT32 nInd3) = 0;

    // clipping functions
    BOOL AreEqual(UINT32 nInd1, UINT32 nInd2);
    BOOL Clip3DPoint(UINT32 nInd);
    BOOL Clip3DLine(UINT32& nInd1,UINT32& nInd2);
    BOOL Clip3DPolygon(UINT32Bucket& rEdgeIndex);
    UINT16 GetClipFlags(UINT32 nInd);
    BOOL IsInside(UINT32 nInd, UINT32 nDim, BOOL bLow);
    void ClipPoly(UINT32Bucket& rEdgeIndex, UINT16 nDim,BOOL bLow);
    void CalcNewPoint(UINT32 nNew,UINT32 nHigh,UINT32 nLow,
        UINT16 nDim, double fBound);

    // Beleuchtungsmodell (ColorModel) in einem Punkt loesen
    // Punkt MUSS in ClipCoordinates vorliegen !
    void SolveColorModel(B3dColor&, Vector3D&, const Vector3D&);
    B3dColor SolveColorModel(B3dMaterial& rMat, Vector3D& rVec,
        const Vector3D& rPnt);

    // Beleuchtungsmodell (ColorModel) fuer eine Lichtquelle loesen
    B3dColor SolveColorModel(B3dLight& rLight, B3dMaterial& rMat,
        Vector3D& rVec, const Vector3D& rPnt);
};


#endif          // _B3D_B3DCOMMN_HXX
