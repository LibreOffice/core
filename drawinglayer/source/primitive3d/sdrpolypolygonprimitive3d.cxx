/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrpolypolygonprimitive3d.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: aw $ $Date: 2008-06-10 09:29:33 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/primitive3d/sdrpolypolygonprimitive3d.hxx>
#include <drawinglayer/primitive3d/sdrdecompositiontools3d.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#include <drawinglayer/attribute/sdrattribute.hxx>
#include <drawinglayer/primitive3d/hittestprimitive3d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        Primitive3DSequence SdrPolyPolygonPrimitive3D::createLocalDecomposition(const geometry::ViewInformation3D& /*rViewInformation*/) const
        {
            Primitive3DSequence aRetval;

            if(getPolyPolygon3D().count())
            {
                ::std::vector< basegfx::B3DPolyPolygon > aFill;
                aFill.push_back(getPolyPolygon3D());

                // get full range
                const basegfx::B3DRange aRange(getRangeFrom3DGeometry(aFill));

                // #i98295# normal creation
                if(getSdrLFSAttribute().getFill())
                {
                    if(::com::sun::star::drawing::NormalsKind_SPHERE == getSdr3DObjectAttribute().getNormalsKind())
                    {
                        applyNormalsKindSphereTo3DGeometry(aFill, aRange);
                    }
                    else if(::com::sun::star::drawing::NormalsKind_FLAT == getSdr3DObjectAttribute().getNormalsKind())
                    {
                        applyNormalsKindFlatTo3DGeometry(aFill);
                    }

                    if(getSdr3DObjectAttribute().getNormalsInvert())
                    {
                        applyNormalsInvertTo3DGeometry(aFill);
                    }
                }

                // #i98314# texture coordinates
                if(getSdrLFSAttribute().getFill())
                {
                    applyTextureTo3DGeometry(
                        getSdr3DObjectAttribute().getTextureProjectionX(),
                        getSdr3DObjectAttribute().getTextureProjectionY(),
                        aFill,
                        aRange,
                        getTextureSize());
                }

                if(getSdrLFSAttribute().getFill())
                {
                    // add fill
                    aRetval = create3DPolyPolygonFillPrimitives(
                        aFill,
                        getTransform(),
                        getTextureSize(),
                        getSdr3DObjectAttribute(),
                        *getSdrLFSAttribute().getFill(),
                        getSdrLFSAttribute().getFillFloatTransGradient());
                }
                else
                {
                    // create simplified 3d hit test geometry
                    const attribute::SdrFillAttribute aSimplifiedFillAttribute(0.0, basegfx::BColor(), 0, 0, 0);

                    aRetval = create3DPolyPolygonFillPrimitives(
                        aFill,
                        getTransform(),
                        getTextureSize(),
                        getSdr3DObjectAttribute(),
                        aSimplifiedFillAttribute,
                        0);

                    // encapsulate in HitTestPrimitive3D and add
                    const Primitive3DReference xRef(new HitTestPrimitive3D(aRetval));
                    aRetval = Primitive3DSequence(&xRef, 1L);
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

        basegfx::B3DRange SdrPolyPolygonPrimitive3D::getB3DRange(const geometry::ViewInformation3D& /*rViewInformation*/) const
        {
            // added this implementation to make sure that non-visible objects of this
            // kind will deliver their expansion. If not implemented, it would never deliver
            // the used space for non-visible objects since the decomposition for that
            // case will be empty (what is correct). To support chart ATM which relies on
            // non-visible objects occupying space in 3D, this method was added
            basegfx::B3DRange aRetval;

            if(getPolyPolygon3D().count())
            {
                aRetval = basegfx::tools::getRange(getPolyPolygon3D());
                aRetval.transform(getTransform());

                if(getSdrLFSAttribute().getLine())
                {
                    const attribute::SdrLineAttribute& rLine = *getSdrLFSAttribute().getLine();

                    if(rLine.isVisible() && !basegfx::fTools::equalZero(rLine.getWidth()))
                    {
                        // expand by half LineWidth as tube radius
                        aRetval.grow(rLine.getWidth() / 2.0);
                    }
                }
            }

            return aRetval;
        }

        // provide unique ID
        ImplPrimitrive3DIDBlock(SdrPolyPolygonPrimitive3D, PRIMITIVE3D_ID_SDRPOLYPOLYGONPRIMITIVE3D)

    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
