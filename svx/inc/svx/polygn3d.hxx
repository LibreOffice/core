/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: polygn3d.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:05:42 $
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

#ifndef _E3D_POLYGON3D_HXX
#define _E3D_POLYGON3D_HXX

#ifndef _E3D_OBJ3D_HXX
#include <svx/obj3d.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class SVX_DLLPUBLIC E3dPolygonObj : public E3dCompoundObject
{
private:
    // Parameter
    basegfx::B3DPolyPolygon aPolyPoly3D;
    basegfx::B3DPolyPolygon aPolyNormals3D;
    basegfx::B2DPolyPolygon aPolyTexture2D;
    BOOL            bLineOnly;

    SVX_DLLPRIVATE void CreateDefaultNormals();
    SVX_DLLPRIVATE void CreateDefaultTexture();

public:
    void SetPolyPolygon3D(const basegfx::B3DPolyPolygon& rNewPolyPoly3D);
    void SetPolyNormals3D(const basegfx::B3DPolyPolygon& rNewPolyPoly3D);
    void SetPolyTexture2D(const basegfx::B2DPolyPolygon& rNewPolyPoly2D);

    TYPEINFO();

    E3dPolygonObj(
        E3dDefaultAttributes& rDefault,
        const basegfx::B3DPolyPolygon& rPolyPoly3D,
        BOOL bLinOnly=FALSE);
    E3dPolygonObj(
        E3dDefaultAttributes& rDefault,
        const basegfx::B3DPolyPolygon& rPolyPoly3D,
        const basegfx::B3DPolyPolygon& rPolyNormals3D,
        BOOL bLinOnly=FALSE);
    E3dPolygonObj(
        E3dDefaultAttributes& rDefault,
        const basegfx::B3DPolyPolygon& rPolyPoly3D,
        const basegfx::B3DPolyPolygon& rPolyNormals3D,
        const basegfx::B2DPolyPolygon& rPolyTexture2D,
        BOOL bLinOnly=FALSE);

    E3dPolygonObj();
    virtual ~E3dPolygonObj();

    const basegfx::B3DPolyPolygon& GetPolyPolygon3D() const { return aPolyPoly3D; }
    const basegfx::B3DPolyPolygon& GetPolyNormals3D() const { return aPolyNormals3D; }
    const basegfx::B2DPolyPolygon& GetPolyTexture2D() const { return aPolyTexture2D; }

    virtual UINT16 GetObjIdentifier() const;
    virtual SdrObject* DoConvertToPolyObj(BOOL bBezier) const;

    virtual void operator=(const SdrObject&);

    // Geometrieerzeugung
    virtual void CreateGeometry();

    // Give out simple line geometry
    virtual basegfx::B3DPolyPolygon Get3DLineGeometry() const;

    // LineOnly?
    BOOL GetLineOnly() { return bLineOnly; }
    void SetLineOnly(BOOL bNew);
};

#endif          // _E3D_POLYGON3D_HXX
