/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: geometry2dextractor.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2008-01-30 12:26:48 $
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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR3D_GEOMETRY2DEXTRACTOR_HXX
#include <drawinglayer/processor3d/geometry2dextractor.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_PRIMITIVETYPES3D_HXX
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_TRANSFORMPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/transformprimitive3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_MODIFIEDCOLORPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/modifiedcolorprimitive3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYGONPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/polygonprimitive3d.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYPOLYGONPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/polypolygonprimitive3d.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYPOLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_TEXTUREPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/textureprimitive3d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor3d
    {
        Geometry2DExtractingProcessor::Geometry2DExtractingProcessor(
            double fTime,
            const basegfx::B2DHomMatrix& rObjectTransformation,
            const basegfx::B3DHomMatrix& rWorldToView)
        :   BaseProcessor3D(fTime),
            maPrimitive2DSequence(),
            maObjectTransformation(rObjectTransformation),
            maWorldToView(rWorldToView),
            maBColorModifierStack()
        {
        }

        void Geometry2DExtractingProcessor::process(const primitive3d::Primitive3DSequence& rSource)
        {
            if(rSource.hasElements())
            {
                const sal_Int32 nCount(rSource.getLength());

                for(sal_Int32 a(0L); a < nCount; a++)
                {
                    // get reference
                    const primitive3d::Primitive3DReference xReference(rSource[a]);

                    if(xReference.is())
                    {
                        // try to cast to BasePrimitive3D implementation
                        const primitive3d::BasePrimitive3D* pBasePrimitive = dynamic_cast< const primitive3d::BasePrimitive3D* >(xReference.get());

                        if(pBasePrimitive)
                        {
                            // it is a BasePrimitive3D implementation, use getPrimitiveID() call for switch
                            switch(pBasePrimitive->getPrimitiveID())
                            {
                                case PRIMITIVE3D_ID_TRANSFORMPRIMITIVE3D :
                                {
                                    // transform group. Remember current transformations
                                    const primitive3d::TransformPrimitive3D& rPrimitive = static_cast< const primitive3d::TransformPrimitive3D& >(*(xReference.get()));
                                    basegfx::B3DHomMatrix aLastWorldToView(getWorldToView());

                                    // create new transformations
                                    maWorldToView = getWorldToView() * rPrimitive.getTransformation();

                                    // let break down recursively
                                    process(rPrimitive.getChildren());

                                    // restore transformations
                                    maWorldToView = aLastWorldToView;
                                    break;
                                }
                                case PRIMITIVE3D_ID_MODIFIEDCOLORPRIMITIVE3D :
                                {
                                    // ModifiedColorPrimitive3D; push, process and pop
                                    const primitive3d::ModifiedColorPrimitive3D& rModifiedCandidate = static_cast< const primitive3d::ModifiedColorPrimitive3D& >(*pBasePrimitive);
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
                                    const primitive3d::PolygonHairlinePrimitive3D& rPrimitive = static_cast< const primitive3d::PolygonHairlinePrimitive3D& >(*pBasePrimitive);
                                    basegfx::B2DPolygon a2DHairline(basegfx::tools::createB2DPolygonFromB3DPolygon(rPrimitive.getB3DPolygon(), getWorldToView()));

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
                                    const primitive3d::PolyPolygonMaterialPrimitive3D& rPrimitive = static_cast< const primitive3d::PolyPolygonMaterialPrimitive3D& >(*pBasePrimitive);
                                    basegfx::B2DPolyPolygon a2DFill(basegfx::tools::createB2DPolyPolygonFromB3DPolyPolygon(rPrimitive.getB3DPolyPolygon(), getWorldToView()));

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
                                    const primitive3d::TexturePrimitive3D& rTexturePrimitive = static_cast< const primitive3d::TexturePrimitive3D& >(*pBasePrimitive);
                                    const primitive3d::Primitive3DSequence aChildren(rTexturePrimitive.getChildren());

                                    if(aChildren.hasElements())
                                    {
                                        process(aChildren);
                                    }
                                    break;
                                }
                                case PRIMITIVE3D_ID_SDRLABELPRIMITIVE3D :
                                case PRIMITIVE3D_ID_SHADOWPRIMITIVE3D :
                                {
                                    // accept but ignore labels and shadow; these should be extracted seperately
                                    break;
                                }
                                default :
                                {
                                    // process recursively
                                    process(pBasePrimitive->get3DDecomposition(getTime()));
                                    break;
                                }
                            }
                        }
                        else
                        {
                            // unknown implementation, use UNO API call instead and process recursively
                            com::sun::star::graphic::Primitive3DParameters aPrimitive3DParameters;
                            aPrimitive3DParameters.Time = getTime();
                            process(xReference->getDecomposition(aPrimitive3DParameters));
                        }
                    }
                }
            }
        }
    } // end of namespace processor3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
