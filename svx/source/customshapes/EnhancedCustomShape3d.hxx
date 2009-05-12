/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: EnhancedCustomShape3d.hxx,v $
 * $Revision: 1.7.226.1 $
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

#ifndef _ENHANCEDCUSTOMSHAPE3D_HXX
#define _ENHANCEDCUSTOMSHAPE3D_HXX

#include <svx/msdffimp.hxx>
#include <vector>
//#include <poly3d.hxx>
//#include <goodies/point3d.hxx>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>

class EnhancedCustomShape3d
{
    class Transformation2D
    {
        Point                                   aCenter;
        com::sun::star::drawing::ProjectionMode eProjectionMode;

        // parallel projection
        double      fSkewAngle;
        double      fSkew;          // in percent

        // perspective projection
        double      fZScreen;
        basegfx::B3DPoint       fViewPoint;
        double      fOriginX;
        double      fOriginY;

        const double* pMap;

        public :

                        Transformation2D( const SdrObject* pCustomShape, const Rectangle& rBoundRect, const double* pMap );

            basegfx::B3DPolygon ApplySkewSettings( const basegfx::B3DPolygon& rPolygon3D ) const;
            Point       Transform2D( const basegfx::B3DPoint& rPoint ) const;
            sal_Bool    IsParallel() const;
    };

    friend class Transformation2D;

    protected :
        static Rectangle CalculateNewSnapRect( const SdrObject* pCustomShape, const Rectangle& rSnapRect, const Rectangle& rBoundRect, const double* pMap );

    public :
        static SdrObject* Create3DObject( const SdrObject* pShape2d, const SdrObject* pCustomShape );
};

#endif

