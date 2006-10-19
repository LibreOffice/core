/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrpolypolygonprimitive3d.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2006-10-19 10:38:34 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRPOLYPOLYGONPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/sdrpolypolygonprimitive3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRDECOMPOSITIONTOOLS3D_HXX
#include <drawinglayer/primitive3d/sdrdecompositiontools3d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        Primitive3DSequence SdrPolyPolygonPrimitive3D::createLocalDecomposition(double /*fTime*/) const
        {
            Primitive3DSequence aRetval;

            if(getPolyPolygon3D().count())
            {
                // add fill
                if(getSdrLFSAttribute().getFill())
                {
                    // create single PolyPolygonFill primitives
                    ::std::vector< basegfx::B3DPolyPolygon > aFill;
                    aFill.push_back(getPolyPolygon3D());

                    aRetval = create3DPolyPolygonFillPrimitives(
                        aFill, getTransform(), getTextureSize(),
                        getSdr3DObjectAttribute(), *getSdrLFSAttribute().getFill(),
                        getSdrLFSAttribute().getFillFloatTransGradient());
                }

                // add line
                if(getSdrLFSAttribute().getLine())
                {
                    basegfx::B3DPolyPolygon aLine(getPolyPolygon3D());
                    aLine.clearNormals();
                    aLine.clearTextureCoordinates();
                    const Primitive3DSequence aLines(create3DPolyPolygonLinePrimitives(aLine, getTransform(), *getSdrLFSAttribute().getLine()));
                    appendPrimitive3DSequenceToPrimitive3DSequence(aRetval, aLines);
                }

                // add shadow
                if(getSdrLFSAttribute().getShadow() && aRetval.hasElements())
                {
                    const Primitive3DSequence aShadow(createShadowPrimitive3D(aRetval, *getSdrLFSAttribute().getShadow(), getSdr3DObjectAttribute().getShadow3D()));
                    appendPrimitive3DSequenceToPrimitive3DSequence(aRetval, aShadow);
                }
            }

            return aRetval;
        }

        SdrPolyPolygonPrimitive3D::SdrPolyPolygonPrimitive3D(
            const basegfx::B3DPolyPolygon& rPolyPolygon3D,
            const basegfx::B3DHomMatrix& rTransform,
            const basegfx::B2DVector& rTextureSize,
            const attribute::SdrLineFillShadowAttribute& rSdrLFSAttribute,
            const attribute::Sdr3DObjectAttribute& rSdr3DObjectAttribute)
        :   SdrPrimitive3D(rTransform, rTextureSize, rSdrLFSAttribute, rSdr3DObjectAttribute),
            maPolyPolygon3D(rPolyPolygon3D)
        {
        }

        bool SdrPolyPolygonPrimitive3D::operator==(const BasePrimitive3D& rPrimitive) const
        {
            if(SdrPrimitive3D::operator==(rPrimitive))
            {
                const SdrPolyPolygonPrimitive3D& rCompare = static_cast< const SdrPolyPolygonPrimitive3D& >(rPrimitive);

                return (getPolyPolygon3D() == rCompare.getPolyPolygon3D());
            }

            return false;
        }

        sal_uInt32 SdrPolyPolygonPrimitive3D::getPrimitiveID() const
        {
            return Create3DPrimitiveID('3','P','P','o');
        }
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
