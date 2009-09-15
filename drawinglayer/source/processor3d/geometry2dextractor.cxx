/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: geometry2dextractor.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: aw $ $Date: 2008-06-24 15:31:09 $
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

#include <drawinglayer/processor3d/geometry2dextractor.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>
#include <drawinglayer/primitive3d/transformprimitive3d.hxx>
#include <drawinglayer/primitive3d/modifiedcolorprimitive3d.hxx>
#include <drawinglayer/primitive3d/polygonprimitive3d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive3d/polypolygonprimitive3d.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive3d/textureprimitive3d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor3d
    {
        // as tooling, the process() implementation takes over API handling and calls this
        // virtual render method when the primitive implementation is BasePrimitive3D-based.
        void Geometry2DExtractingProcessor::processBasePrimitive3D(const primitive3d::BasePrimitive3D& rCandidate)
        {
            // it is a BasePrimitive3D implementation, use getPrimitive3DID() call for switch
            switch(rCandidate.getPrimitive3DID())
            {
                case PRIMITIVE3D_ID_TRANSFORMPRIMITIVE3D :
                {
                    // transform group. Remember current transformations
                    const primitive3d::TransformPrimitive3D& rPrimitive = static_cast< const primitive3d::TransformPrimitive3D& >(rCandidate);
                    const geometry::ViewInformation3D aLastViewInformation3D(getViewInformation3D());

                    // create new transformation; add new object transform from right side
                    const geometry::ViewInformation3D aNewViewInformation3D(
                        aLastViewInformation3D.getObjectTransformation() * rPrimitive.getTransformation(),
                        aLastViewInformation3D.getOrientation(),
                        aLastViewInformation3D.getProjection(),
                        aLastViewInformation3D.getDeviceToView(),
                        aLastViewInformation3D.getViewTime(),
                        aLastViewInformation3D.getExtendedInformationSequence());
                    updateViewInformation(aNewViewInformation3D);

                    // let break down recursively
                    process(rPrimitive.getChildren());

                    // restore transformations
                    updateViewInformation(aLastViewInformation3D);
                    break;
                }
                case PRIMITIVE3D_ID_MODIFIEDCOLORPRIMITIVE3D :
                {
                    // ModifiedColorPrimitive3D; push, process and pop
                    const primitive3d::ModifiedColorPrimitive3D& rModifiedCandidate = static_cast< const primitive3d::ModifiedColorPrimitive3D& >(rCandidate);
                    const primitive3d::Primitive3DSequence& rSubSequence = rModifiedCandidate.getChildren();

                    if(rSubSequence.hasElements())
                    {
                        maBColorModifierStack.push(rModifiedCandidate.getColorModifier());
                        process(rModifiedCandidate.getChildren());
                        maBColorModifierStack.pop();
                    }
                    break;
                }
                case PRIMITIVE3D_ID_POLYGONHAIRLINEPRIMITIVE3D :
                {
                    // PolygonHairlinePrimitive3D
                    const primitive3d::PolygonHairlinePrimitive3D& rPrimitive = static_cast< const primitive3d::PolygonHairlinePrimitive3D& >(rCandidate);
                    basegfx::B2DPolygon a2DHairline(basegfx::tools::createB2DPolygonFromB3DPolygon(rPrimitive.getB3DPolygon(), getViewInformation3D().getObjectToView()));

                    if(a2DHairline.count())
                    {
                        a2DHairline.transform(getObjectTransformation());
                        const basegfx::BColor aModifiedColor(maBColorModifierStack.getModifiedColor(rPrimitive.getBColor()));
                        const primitive2d::Primitive2DReference xRef(new primitive2d::PolygonHairlinePrimitive2D(a2DHairline, aModifiedColor));
                        primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(maPrimitive2DSequence, xRef);
                    }
                    break;
                }
                case PRIMITIVE3D_ID_POLYPOLYGONMATERIALPRIMITIVE3D :
                {
                    // PolyPolygonMaterialPrimitive3D
                    const primitive3d::PolyPolygonMaterialPrimitive3D& rPrimitive = static_cast< const primitive3d::PolyPolygonMaterialPrimitive3D& >(rCandidate);
                    basegfx::B2DPolyPolygon a2DFill(basegfx::tools::createB2DPolyPolygonFromB3DPolyPolygon(rPrimitive.getB3DPolyPolygon(), getViewInformation3D().getObjectToView()));

                    if(a2DFill.count())
                    {
                        a2DFill.transform(getObjectTransformation());
                        const basegfx::BColor aModifiedColor(maBColorModifierStack.getModifiedColor(rPrimitive.getMaterial().getColor()));
                        const primitive2d::Primitive2DReference xRef(new primitive2d::PolyPolygonColorPrimitive2D(a2DFill, aModifiedColor));
                        primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(maPrimitive2DSequence, xRef);
                    }
                    break;
                }
                case PRIMITIVE3D_ID_GRADIENTTEXTUREPRIMITIVE3D :
                case PRIMITIVE3D_ID_HATCHTEXTUREPRIMITIVE3D :
                case PRIMITIVE3D_ID_BITMAPTEXTUREPRIMITIVE3D :
                case PRIMITIVE3D_ID_ALPHATEXTUREPRIMITIVE3D :
                case PRIMITIVE3D_ID_UNIFIEDALPHATEXTUREPRIMITIVE3D :
                {
                    // TexturePrimitive3D: Process children, do not try to decompose
                    const primitive3d::TexturePrimitive3D& rTexturePrimitive = static_cast< const primitive3d::TexturePrimitive3D& >(rCandidate);
                    const primitive3d::Primitive3DSequence aChildren(rTexturePrimitive.getChildren());

                    if(aChildren.hasElements())
                    {
                        process(aChildren);
                    }
                    break;
                }
                case PRIMITIVE3D_ID_SHADOWPRIMITIVE3D :
                {
                    // accept but ignore labels and shadow; these should be extracted seperately
                    break;
                }
                default :
                {
                    // process recursively
                    process(rCandidate.get3DDecomposition(getViewInformation3D()));
                    break;
                }
            }
        }

        Geometry2DExtractingProcessor::Geometry2DExtractingProcessor(
            const geometry::ViewInformation3D& rViewInformation,
            const basegfx::B2DHomMatrix& rObjectTransformation)
        :   BaseProcessor3D(rViewInformation),
            maPrimitive2DSequence(),
            maObjectTransformation(rObjectTransformation),
            maBColorModifierStack()
        {
        }
    } // end of namespace processor3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
