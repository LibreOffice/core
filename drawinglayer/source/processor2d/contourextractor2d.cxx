/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: contourextractor2d.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2008-02-07 13:41:59 $
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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_CONTOUREXTRACTOR2D_HXX
#include <drawinglayer/processor2d/contourextractor2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PRIMITIVETYPES2D_HXX
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYPOLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BITMAPPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_METAFILEPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_ALPHAPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/alphaprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MASKPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TRANSFORMPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_SCENEPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/sceneprimitive2d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        ContourExtractor2D::ContourExtractor2D(const geometry::ViewInformation2D& rViewInformation)
        :   BaseProcessor2D(rViewInformation),
            maExtractedContour()
        {
        }

        ContourExtractor2D::~ContourExtractor2D()
        {
        }

        void ContourExtractor2D::processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate)
        {
            switch(rCandidate.getPrimitiveID())
            {
                case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D :
                {
                    // extract hairline
                    const primitive2d::PolygonHairlinePrimitive2D& rPolygonCandidate(static_cast< const primitive2d::PolygonHairlinePrimitive2D& >(rCandidate));
                    basegfx::B2DPolygon aLocalPolygon(rPolygonCandidate.getB2DPolygon());
                    aLocalPolygon.transform(maCurrentTransformation);

                    if(aLocalPolygon.isClosed())
                    {
                        // line polygons need to be represented as open polygons to differentiate them
                        // from filled polygons
                        basegfx::tools::openWithGeometryChange(aLocalPolygon);
                    }

                    maExtractedContour.push_back(basegfx::B2DPolyPolygon(aLocalPolygon));
                    break;
                }
                case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D :
                {
                    // extract fill
                    const primitive2d::PolyPolygonColorPrimitive2D& rPolygonCandidate(static_cast< const primitive2d::PolyPolygonColorPrimitive2D& >(rCandidate));
                    basegfx::B2DPolyPolygon aLocalPolyPolygon(rPolygonCandidate.getB2DPolyPolygon());
                    aLocalPolyPolygon.transform(maCurrentTransformation);
                    maExtractedContour.push_back(aLocalPolyPolygon);
                    break;
                }
                case PRIMITIVE2D_ID_BITMAPPRIMITIVE2D :
                {
                    // extract BoundRect from bitmaps
                    const primitive2d::BitmapPrimitive2D& rBitmapCandidate(static_cast< const primitive2d::BitmapPrimitive2D& >(rCandidate));
                    basegfx::B2DHomMatrix aLocalTransform(maCurrentTransformation * rBitmapCandidate.getTransform());
                    basegfx::B2DPolygon aPolygon(basegfx::tools::createPolygonFromRect(basegfx::B2DRange(0.0, 0.0, 1.0, 1.0)));
                    aPolygon.transform(aLocalTransform);
                    maExtractedContour.push_back(basegfx::B2DPolyPolygon(aPolygon));
                    break;
                }
                case PRIMITIVE2D_ID_METAFILEPRIMITIVE2D :
                {
                    // extract BoundRect from MetaFiles
                    const primitive2d::MetafilePrimitive2D& rMetaCandidate(static_cast< const primitive2d::MetafilePrimitive2D& >(rCandidate));
                    basegfx::B2DHomMatrix aLocalTransform(maCurrentTransformation * rMetaCandidate.getTransform());
                    basegfx::B2DPolygon aPolygon(basegfx::tools::createPolygonFromRect(basegfx::B2DRange(0.0, 0.0, 1.0, 1.0)));
                    aPolygon.transform(aLocalTransform);
                    maExtractedContour.push_back(basegfx::B2DPolyPolygon(aPolygon));
                    break;
                }
                case PRIMITIVE2D_ID_ALPHAPRIMITIVE2D :
                {
                    // sub-transparence group. Look at children
                    const primitive2d::AlphaPrimitive2D& rTransCandidate(static_cast< const primitive2d::AlphaPrimitive2D& >(rCandidate));
                    process(rTransCandidate.getChildren());
                    break;
                }
                case PRIMITIVE2D_ID_MASKPRIMITIVE2D :
                {
                    // extract mask, ignore content
                    const primitive2d::MaskPrimitive2D& rMaskCandidate(static_cast< const primitive2d::MaskPrimitive2D& >(rCandidate));
                    basegfx::B2DPolyPolygon aMask(rMaskCandidate.getMask());
                    aMask.transform(maCurrentTransformation);
                    maExtractedContour.push_back(basegfx::B2DPolyPolygon(aMask));
                    break;
                }
                case PRIMITIVE2D_ID_TRANSFORMPRIMITIVE2D :
                {
                    // remember current transformation and ViewInformation
                    const primitive2d::TransformPrimitive2D& rTransformCandidate(static_cast< const primitive2d::TransformPrimitive2D& >(rCandidate));
                    const basegfx::B2DHomMatrix aLastCurrentTransformation(maCurrentTransformation);
                    const geometry::ViewInformation2D aLastViewInformation2D(getViewInformation2D());

                    // create new transformations for CurrentTransformation and for local ViewInformation2D
                    maCurrentTransformation = maCurrentTransformation * rTransformCandidate.getTransformation();
                    maViewInformation2D = geometry::ViewInformation2D(
                        getViewInformation2D().getViewTransformation() * rTransformCandidate.getTransformation(),
                        getViewInformation2D().getViewport(),
                        getViewInformation2D().getVisualizedPage(),
                        getViewInformation2D().getViewTime(),
                        getViewInformation2D().getExtendedInformationSequence());

                    // proccess content
                    process(rTransformCandidate.getChildren());

                    // restore transformations
                    maCurrentTransformation = aLastCurrentTransformation;
                    maViewInformation2D = aLastViewInformation2D;
                    break;
                }
                case PRIMITIVE2D_ID_SCENEPRIMITIVE2D :
                {
                    // 2D Scene primitive containing 3D stuff; extract 2D contour
                    const primitive2d::ScenePrimitive2D& rScenePrimitive2DCandidate(static_cast< const primitive2d::ScenePrimitive2D& >(rCandidate));
                    const primitive2d::Primitive2DSequence xExtracted2DSceneGeometry(rScenePrimitive2DCandidate.getGeometry2D(getViewInformation2D()));

                    // proccess content
                    if(xExtracted2DSceneGeometry.hasElements())
                    {
                        process(xExtracted2DSceneGeometry);
                    }

                    break;
                }
                case PRIMITIVE2D_ID_WRONGSPELLPRIMITIVE2D :
                case PRIMITIVE2D_ID_MARKERARRAYPRIMITIVE2D :
                case PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D :
                {
                    // ignorable primitives
                    break;
                }
                case PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D :
                case PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D :
                {
                    // primitives who's BoundRect will be added
                    basegfx::B2DRange aRange(rCandidate.getB2DRange(getViewInformation2D()));
                    aRange.transform(maCurrentTransformation);
                    maExtractedContour.push_back(basegfx::B2DPolyPolygon(basegfx::tools::createPolygonFromRect(aRange)));
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

        void ContourExtractor2D::process(const primitive2d::Primitive2DSequence& rSource)
        {
            if(rSource.hasElements())
            {
                const sal_Int32 nCount(rSource.getLength());

                for(sal_Int32 a(0L); a < nCount; a++)
                {
                    // get reference
                    const primitive2d::Primitive2DReference xReference(rSource[a]);

                    if(xReference.is())
                    {
                        // try to cast to BasePrimitive2D implementation
                        const primitive2d::BasePrimitive2D* pBasePrimitive = dynamic_cast< const primitive2d::BasePrimitive2D* >(xReference.get());

                        if(pBasePrimitive)
                        {
                            // it is a BasePrimitive2D implementation, use local processor
                            processBasePrimitive2D(*pBasePrimitive);
                        }
                        else
                        {
                            // unknown implementation, use UNO API call instead and process recursively
                            const uno::Sequence< beans::PropertyValue >& rViewParameters(getViewInformation2D().getViewInformationSequence());
                            process(xReference->getDecomposition(rViewParameters));
                        }
                    }
                }
            }
        }
    } // end of namespace processor2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
