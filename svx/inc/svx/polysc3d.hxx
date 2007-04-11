/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: polysc3d.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:05:54 $
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

#ifndef _E3D_POLYSC3D_HXX
#define _E3D_POLYSC3D_HXX

#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif

#ifndef _E3D_SCENE3D_HXX
#include <svx/scene3d.hxx>
#endif

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
