/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: b3dgeometry.hxx,v $
 *
 * $Revision: 1.2 $
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

#ifndef _BGFX_POLYGON_B3DGEOMETRY_HXX
#define _BGFX_POLYGON_B3DGEOMETRY_HXX

//////////////////////////////////////////////////////////////////////////////
// predeclarations

namespace basegfx
{
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    class B3DGeometry
    {
    private:
        B2DPolyPolygon                                  maPolyPolygon;      // the PolyPolygon geometry data, defines point number
        B3DHomMatrix                                    maPolygonTo3D;      // transformation to create 3D PolyPolygon
        B3DPolyPolygon                                  maPolyNormal;       // normal for each point or empty -> unified normal
        B2DPolyPolygon                                  maPolyTexture;      // texture coordinate for each point or empty -> unified coordinate
        B3DVector                                       maUnifiedVector;    // used when maNormal is empty

        // bitfield
        unsigned                                        mbUnifiedVectorValid : 1;   // flag to know if uvec is calculated yet

    public:
        B3DGeometry();
        ~B3DGeometry();

        // compare operators
        bool operator==(const B3DGeometry& rGeometry) const;
        bool operator!=(const B3DGeometry& rGeometry) const { return (!operator==(rGeometry)); }

        // member count
        sal_uInt32 count() const { return maPolyPolygon.count(); }
    };
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////


#endif /* _BGFX_POLYGON_B3DPOLYGON_HXX */
