/*************************************************************************
 *
 *  $RCSfile: EnhancedCustomShape3d.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-10-12 14:11:33 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _ENHANCEDCUSTOMSHAPE3D_HXX
#define _ENHANCEDCUSTOMSHAPE3D_HXX

#ifndef _MSDFFIMP_HXX
#include <msdffimp.hxx>
#endif
#include <vector>
#include <poly3d.hxx>
#include <goodies/point3d.hxx>
#ifndef _COM_SUN_STAR_DRAWING_PROJECTIONMODE_HPP_
#include <com/sun/star/drawing/ProjectionMode.hpp>
#endif

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
        Point3D     fViewPoint;
        double      fOriginX;
        double      fOriginY;

        const double* pMap;

        public :

                        Transformation2D( const SdrObject* pCustomShape, const Rectangle& rBoundRect, const double* pMap );

            void        ApplySkewSettings( Polygon3D& rPolyPoly3D ) const;
            Point       Transform2D( const Vector3D& rPoint ) const;
            sal_Bool    IsParallel() const;
    };

    friend class Transformation2D;

    protected :

        static void Rotate( Vector3D& rPoint, const double x, const double y, const double z );
        static void Rotate( PolyPolygon3D&, const Point3D& rRotateCenter, const double x, const double y, const double z );
        static Rectangle CalculateNewSnapRect( const SdrObject* pCustomShape, const Rectangle& rBoundRect, const double* pMap );

    public :

        static SdrObject* Create3DObject( const SdrObject* pShape2d, const SdrObject* pCustomShape );
};

#endif

