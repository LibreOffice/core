/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _E3D_POLYGON3D_HXX
#define _E3D_POLYGON3D_HXX

#ifndef _E3D_OBJ3D_HXX
#include <bf_svx/obj3d.hxx>
#endif

#ifndef _POLY3D_HXX
#include <bf_svx/poly3d.hxx>
#endif
namespace binfilter {

class E3dPolygonObj : public E3dCompoundObject
{
private:
    // Parameter
    PolyPolygon3D	aPolyPoly3D;
    PolyPolygon3D	aPolyNormals3D;
    PolyPolygon3D	aPolyTexture3D;
    BOOL			bLineOnly;

    void CreateDefaultNormals();
    void CreateDefaultTexture();

public:
    void SetPolyPolygon3D(const PolyPolygon3D& rNewPolyPoly3D);
    void SetPolyNormals3D(const PolyPolygon3D& rNewPolyPoly3D);
    void SetPolyTexture3D(const PolyPolygon3D& rNewPolyPoly3D);

    TYPEINFO();

    E3dPolygonObj(E3dDefaultAttributes& rDefault, const PolyPolygon3D& rPoly3D,
        BOOL bLinOnly=FALSE);
    E3dPolygonObj(E3dDefaultAttributes& rDefault, const PolyPolygon3D& rPoly3D,
        const PolyPolygon3D& rVector3D, BOOL bLinOnly=FALSE);
    E3dPolygonObj(E3dDefaultAttributes& rDefault, const Vector3D& rP1,
        const Vector3D& rP2, BOOL bLinOnly=TRUE);

    E3dPolygonObj();
    virtual ~E3dPolygonObj();

    const PolyPolygon3D& GetPolyPolygon3D() const { return aPolyPoly3D; }
    const PolyPolygon3D& GetPolyNormals3D() const { return aPolyNormals3D; }
    const PolyPolygon3D& GetPolyTexture3D() const { return aPolyTexture3D; }

    virtual UINT16 GetObjIdentifier() const;

    virtual void WriteData(SvStream& rOut) const;
    virtual void ReadData(const SdrObjIOHeader& rHead, SvStream& rIn);

    // Geometrieerzeugung

    // Give out simple line geometry

    // LineOnly?
    BOOL GetLineOnly() { return bLineOnly; }
    void SetLineOnly(BOOL bNew);
};

}//end of namespace binfilter
#endif			// _E3D_POLYGON3D_HXX
