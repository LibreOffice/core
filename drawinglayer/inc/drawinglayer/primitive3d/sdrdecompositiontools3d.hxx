/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrdecompositiontools3d.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2006-10-19 10:32:40 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRDECOMPOSITIONTOOLS3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRDECOMPOSITIONTOOLS3D_HXX

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_BASEPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/baseprimitive3d.hxx>
#endif

#include <vector>

//////////////////////////////////////////////////////////////////////////////
// predefines
namespace basegfx {
    class B3DPolygon;
    class B3DPolyPolygon;
    class B3DHomMatrix;
    class B2DVector;
}

namespace drawinglayer { namespace attribute {
    class SdrLineAttribute;
    class SdrFillAttribute;
    class Sdr3DObjectAttribute;
    class FillGradientAttribute;
    class SdrShadowAttribute;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        Primitive3DSequence create3DPolyPolygonLinePrimitives(
            const basegfx::B3DPolyPolygon& rUnitPolyPolygon,
            const basegfx::B3DHomMatrix& rObjectTransform,
            const attribute::SdrLineAttribute& rLine);

        Primitive3DSequence create3DPolyPolygonFillPrimitives(
            const ::std::vector< basegfx::B3DPolyPolygon >& r3DPolyPolygonVector,
            const basegfx::B3DHomMatrix& rObjectTransform,
            const basegfx::B2DVector& rTextureSize,
            const attribute::Sdr3DObjectAttribute& aSdr3DObjectAttribute,
            const attribute::SdrFillAttribute& rFill,
            const attribute::FillGradientAttribute* pFillGradient = 0L);

        Primitive3DSequence createShadowPrimitive3D(
            const Primitive3DSequence& rSource,
            const attribute::SdrShadowAttribute& rShadow,
            bool bShadow3D);
    } // end of namespace overlay
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //_DRAWINGLAYER_PRIMITIVE3D_SDRDECOMPOSITIONTOOLS3D_HXX

// eof
