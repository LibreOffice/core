/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: msashape3d.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:29:34 $
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

#ifndef _BGFX_POINT_B3DPOINT_HXX
#include <basegfx/point/b3dpoint.hxx>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYGON_HXX
#include <basegfx/polygon/b3dpolygon.hxx>
#endif

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

