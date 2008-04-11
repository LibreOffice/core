/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: msashape3d.hxx,v $
 * $Revision: 1.7 $
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

#ifndef _MSASHAPE3D_HXX
#define _MSASHAPE3D_HXX

#include <svx/msdffimp.hxx>
#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>

#include <vector>
//#include <poly3d.hxx>
//#include <goodies/point3d.hxx>

class SvxMSDffCustomShape3D
{
    class Transformation2D
    {
        Point       aCenter;
        sal_Bool    bParallel;

        // parallel projection
        double      fSkewAngle;
        sal_Int32   nSkewAmount;    // in percent

        // perspective projection
        double      fZScreen;
        basegfx::B3DPoint       fViewPoint;
        double      fViewPointOriginX;
        double      fViewPointOriginY;

        public :

            Transformation2D( const DffPropSet& rPropSet, const Rectangle& rSnapRect );
            basegfx::B3DPolygon ApplySkewSettings( const basegfx::B3DPolygon& rPoly3D );
            Point       Transform2D( const basegfx::B3DPoint& rPoint );
            sal_Bool    IsParallel() const { return bParallel; };
    };

    friend class Transformation2D;

    protected :

        static basegfx::B3DPoint Rotate( const basegfx::B3DPoint& rPoint, const double x, const double y, const double z );
//      static void Rotate( basegfx::B3DPolyPolygon&, const basegfx::B3DPoint& rRotateCenter, const double x, const double y, const double z );
        static double Fix16ToAngle( sal_Int32 nFixAngle );
        static Rectangle CalculateNewSnapRect( const Rectangle& rOriginalSnapRect, const DffPropSet& );

    public :

        static SdrObject*       Create3DObject( const SdrObject* pSource, const DffPropSet&,
                                    SfxItemSet&, Rectangle& rSnapRect, sal_uInt32 nSpFlags );
};

#endif

