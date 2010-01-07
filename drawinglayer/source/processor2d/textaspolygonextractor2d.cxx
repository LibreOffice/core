/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: contourextractor2d.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: aw $ $Date: 2008-06-24 15:31:08 $
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

#include <drawinglayer/processor2d/textaspolygonextractor2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        void TextAsPolygonExtractor2D::processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate)
        {
            switch(rCandidate.getPrimitive2DID())
            {
                case PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D :
                {
                    // TextDecoratedPortionPrimitive2D can produce the following primitives
                    // when being decomposed:
                    //
                    // - TextSimplePortionPrimitive2D
                    // - PolygonWavePrimitive2D
                    //      - PolygonStrokePrimitive2D
                    // - PolygonStrokePrimitive2D
                    //      - PolyPolygonColorPrimitive2D
                    //      - PolyPolygonHairlinePrimitive2D
                    //          - PolygonHairlinePrimitive2D
                    // - ShadowPrimitive2D
                    //      - ModifiedColorPrimitive2D
                    //      - TransformPrimitive2D
                    // - TextEffectPrimitive2D
                    //      - ModifiedColorPrimitive2D
                    //      - TransformPrimitive2D
                    //      - GroupPrimitive2D

                    // encapsulate with flag and use decomposition
                    mnInText++;
                    process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    mnInText--;

                    break;
                }
                case PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D :
                {
                    // TextSimplePortionPrimitive2D can produce the following primitives
                    // when being decomposed:
                    //
                    // - PolyPolygonColorPrimitive2D
                    // - TextEffectPrimitive2D
                    //      - ModifiedColorPrimitive2D
                    //      - TransformPrimitive2D
                    //      - GroupPrimitive2D

                    // encapsulate with flag and use decomposition
                    mnInText++;
                    process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    mnInText--;

                    break;
                }

                // as can be seen from the TextSimplePortionPrimitive2D and the
                // TextDecoratedPortionPrimitive2D, inside of the mnInText marks
                // the following primitives can occurr containing geometry data
                // from text decomposition:
                //
                // - PolyPolygonColorPrimitive2D
                // - PolygonHairlinePrimitive2D
                // - PolyPolygonHairlinePrimitive2D (for convenience)
                //
                case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D :
                {
                    if(mnInText)
                    {
                        const primitive2d::PolyPolygonColorPrimitive2D& rPoPoCoCandidate(static_cast< const primitive2d::PolyPolygonColorPrimitive2D& >(rCandidate));
                        basegfx::B2DPolyPolygon aPolyPolygon(rPoPoCoCandidate.getB2DPolyPolygon());

                        if(aPolyPolygon.count())
                        {
                            // transform the PolyPolygon
                            aPolyPolygon.transform(getViewInformation2D().getObjectToViewTransformation());

                            // get evtl. corrected color
                            const basegfx::BColor aColor(maBColorModifierStack.getModifiedColor(rPoPoCoCandidate.getBColor()));

                            // add to result vector
                            maTarget.push_back(TextAsPolygonDataNode(aPolyPolygon, aColor, true));
                        }
                    }

                    break;
                }
                case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D :
                {
                    if(mnInText)
                    {
                        const primitive2d::PolygonHairlinePrimitive2D& rPoHaCandidate(static_cast< const primitive2d::PolygonHairlinePrimitive2D& >(rCandidate));
                        basegfx::B2DPolygon aPolygon(rPoHaCandidate.getB2DPolygon());

                        if(aPolygon.count())
                        {
                            // transform the Polygon
                            aPolygon.transform(getViewInformation2D().getObjectToViewTransformation());

                            // get evtl. corrected color
                            const basegfx::BColor aColor(maBColorModifierStack.getModifiedColor(rPoHaCandidate.getBColor()));

                            // add to result vector
                            maTarget.push_back(TextAsPolygonDataNode(basegfx::B2DPolyPolygon(aPolygon), aColor, false));
                        }
                    }

                    break;
                }
                case PRIMITIVE2D_ID_POLYPOLYGONHAIRLINEPRIMITIVE2D :
                {
                    if(mnInText)
                    {
                        const primitive2d::PolyPolygonHairlinePrimitive2D& rPoPoHaCandidate(static_cast< const primitive2d::PolyPolygonHairlinePrimitive2D& >(rCandidate));
                        basegfx::B2DPolyPolygon aPolyPolygon(rPoPoHaCandidate.getB2DPolyPolygon());

                        if(aPolyPolygon.count())
                        {
                            // transform the Polygon
                            aPolyPolygon.transform(getViewInformation2D().getObjectToViewTransformation());

                            // get evtl. corrected color
                            const basegfx::BColor aColor(maBColorModifierStack.getModifiedColor(rPoPoHaCandidate.getBColor()));

                            // add to result vector
                            maTarget.push_back(TextAsPolygonDataNode(aPolyPolygon, aColor, false));
                        }
                    }

                    break;
                }

                // usage of color modification stack is needed
                case PRIMITIVE2D_ID_MODIFIEDCOLORPRIMITIVE2D :
                {
                    const primitive2d::ModifiedColorPrimitive2D& rModifiedColorCandidate(static_cast< const primitive2d::ModifiedColorPrimitive2D& >(rCandidate));

                    if(rModifiedColorCandidate.getChildren().hasElements())
                    {
                        maBColorModifierStack.push(rModifiedColorCandidate.getColorModifier());
                        process(rModifiedColorCandidate.getChildren());
                        maBColorModifierStack.pop();
                    }

                    break;
                }

                // usage of transformation stack is needed
                case PRIMITIVE2D_ID_TRANSFORMPRIMITIVE2D :
                {
                    // remember current transformation and ViewInformation
                    const primitive2d::TransformPrimitive2D& rTransformCandidate(static_cast< const primitive2d::TransformPrimitive2D& >(rCandidate));
                    const geometry::ViewInformation2D aLastViewInformation2D(getViewInformation2D());

                    // create new transformations for CurrentTransformation and for local ViewInformation2D
                    const geometry::ViewInformation2D aViewInformation2D(
                        getViewInformation2D().getObjectTransformation() * rTransformCandidate.getTransformation(),
                        getViewInformation2D().getViewTransformation(),
                        getViewInformation2D().getViewport(),
                        getViewInformation2D().getVisualizedPage(),
                        getViewInformation2D().getViewTime(),
                        getViewInformation2D().getExtendedInformationSequence());
                    updateViewInformation(aViewInformation2D);

                    // proccess content
                    process(rTransformCandidate.getChildren());

                    // restore transformations
                    updateViewInformation(aLastViewInformation2D);

                    break;
                }

                // ignorable primitives
                case PRIMITIVE2D_ID_SCENEPRIMITIVE2D :
                case PRIMITIVE2D_ID_WRONGSPELLPRIMITIVE2D :
                case PRIMITIVE2D_ID_MARKERARRAYPRIMITIVE2D :
                case PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D :
                case PRIMITIVE2D_ID_BITMAPPRIMITIVE2D :
                case PRIMITIVE2D_ID_METAFILEPRIMITIVE2D :
                case PRIMITIVE2D_ID_MASKPRIMITIVE2D :
                {
                    break;
                }

                default :
                {
                    // process recursively
                    process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    break;
                }
            }
        }

        TextAsPolygonExtractor2D::TextAsPolygonExtractor2D(const geometry::ViewInformation2D& rViewInformation)
        :   BaseProcessor2D(rViewInformation),
            maTarget(),
            maBColorModifierStack(),
            mnInText(0)
        {
        }

        TextAsPolygonExtractor2D::~TextAsPolygonExtractor2D()
        {
        }
    } // end of namespace processor2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
