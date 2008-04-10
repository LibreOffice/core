/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: polysc3d.hxx,v $
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

#ifndef _E3D_POLYSC3D_HXX
#define _E3D_POLYSC3D_HXX

#include <svx/svdpage.hxx>
#include <svx/scene3d.hxx>

/*************************************************************************
|*
|* 3D-Szene mit Darstellung durch 2D-Polygone
|*
\************************************************************************/

class SVX_DLLPUBLIC E3dPolyScene : public E3dScene
{
public:
    TYPEINFO();
    E3dPolyScene();
    E3dPolyScene(E3dDefaultAttributes& rDefault);

    virtual UINT16 GetObjIdentifier() const;

    // Zeichenmethode
    virtual sal_Bool DoPaintObject(XOutputDevice&, const SdrPaintInfoRec&) const;

    // Die Kontur fuer TextToContour
    virtual basegfx::B2DPolyPolygon TakeContour() const;
    virtual basegfx::B2DPolyPolygon ImpTakeContour3D() const;

    virtual void Paint3D(XOutputDevice& rOut, Base3D* pBase3D,
        const SdrPaintInfoRec& rInfoRec, UINT16 nDrawFlags=0);

protected:
    void DrawAllShadows(Base3D *pBase3D, XOutputDevice& rXOut,
        const Rectangle& rBound, const Volume3D& rVolume,
        const SdrPaintInfoRec& rInfoRec);

    BOOL LocalPaint3D(XOutputDevice& rOut, const SdrPaintInfoRec& rInfoRec);

    void DrawPolySceneClip(XOutputDevice& rOut, const E3dObject *p3DObj,
        Base3D *pBase3D, const SdrPaintInfoRec& rInfoRec);

    void DrawWireframe(Base3D *pBase3D, XOutputDevice& rXOut);
};

#endif          // _E3D_POLYSC3D_HXX
