/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: msashape3d.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:46:01 $
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

#ifndef _MSASHAPE3D_HXX
#define _MSASHAPE3D_HXX

#ifndef _MSDFFIMP_HXX
#include <msdffimp.hxx>
#endif
#include <vector>
#include <poly3d.hxx>
#include <goodies/point3d.hxx>

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
        Point3D     fViewPoint;
        double      fViewPointOriginX;
        double      fViewPointOriginY;

        public :

                        Transformation2D( const DffPropSet& rPropSet, const Rectangle& rSnapRect );

            void        ApplySkewSettings( Polygon3D& rPolyPoly3D );
            Point       Transform2D( const Vector3D& rPoint );
            sal_Bool    IsParallel() const { return bParallel; };
    };

    friend class Transformation2D;

    protected :

        static void Rotate( Vector3D& rPoint, const double x, const double y, const double z );
        static void Rotate( PolyPolygon3D&, const Point3D& rRotateCenter, const double x, const double y, const double z );
        static double Fix16ToAngle( sal_Int32 nFixAngle );
        static Rectangle CalculateNewSnapRect( const Rectangle& rOriginalSnapRect, const DffPropSet& );

    public :

        static SdrObject*       Create3DObject( const SdrObject* pSource, const DffPropSet&,
                                    SfxItemSet&, Rectangle& rSnapRect, sal_uInt32 nSpFlags );
};

#endif

