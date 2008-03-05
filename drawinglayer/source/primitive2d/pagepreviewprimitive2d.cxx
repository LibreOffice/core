/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pagepreviewprimitive2d.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2008-03-05 09:15:43 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PAGEPREVIEWPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/pagepreviewprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PRIMITIVETYPES2D_HXX
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MASKPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TRANSFORMPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence PagePreviewPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            Primitive2DSequence xRetval;
            Primitive2DSequence aContent(getChildren());

            if(aContent.hasElements())
            {
                // check if content overlaps with tageted size and needs to be embedded with a
                // clipping primitive
                const basegfx::B2DRange aRealContentRange(getB2DRangeFromPrimitive2DSequence(aContent, rViewInformation));
                const basegfx::B2DRange aAllowedContentRange(0.0, 0.0, getContentWidth(), getContentHeight());

                if(!aAllowedContentRange.isInside(aRealContentRange))
                {
                    const Primitive2DReference xReferenceA(new MaskPrimitive2D(basegfx::B2DPolyPolygon(basegfx::tools::createPolygonFromRect(aAllowedContentRange)), aContent));
                    aContent = Primitive2DSequence(&xReferenceA, 1);
                }

                // content is not scaled or rotated and occupates aContentRange. Create a mapping
                // to own local geometry. Create matrix describing the content by setting scaling
                basegfx::B2DHomMatrix aPageTrans;
                aPageTrans.set(0, 0, getContentWidth());
                aPageTrans.set(1, 1, getContentHeight());

                // decompose to access own scaling
                basegfx::B2DVector aScale, aTranslate;
                double fRotate, fShearX;
                getTransform().decompose(aScale, aTranslate, fRotate, fShearX);

                if(getKeepAspectRatio())
                {
                    // look at the aspect ratio of the content and the local geometry
                    const double fRatioOwn(aScale.getX() ? (aScale.getY() / aScale.getX()) : 1.0);
                    const double fRatioContent(getContentWidth() ? (getContentHeight() / getContentWidth()) : 1.0);

                    // the ratios are based on different coordinate systems, so look if they differ really more
                    // than 0,1 percent
                    if(fabs(fRatioOwn - fRatioContent) > 0.001)
                    {
                        if(basegfx::fTools::more(fRatioOwn, fRatioContent))
                        {
                            // vertically center the page by translating
                            const double fFullY(aScale.getX() ? (aScale.getY() * getContentWidth()) / aScale.getX() : 0.0);
                            const double fTransY((fFullY - getContentHeight()) * 0.5);
                            aPageTrans.set(1, 2, fTransY);
                        }
                        else
                        {
                            // horizontally center the page by translating
                            const double fFullX(aScale.getY() ? (aScale.getX() * getContentHeight()) / aScale.getY() : 0.0);
                            const double fTransX((fFullX - getContentWidth()) * 0.5);
                            aPageTrans.set(0, 2, fTransX);
                        }
                    }
                }

                // create composed transformation from content to local geometry. An
                // eventually needed clipping is already added, so directly go to local coordinates
                basegfx::B2DHomMatrix aPageToObject(aPageTrans);
                aPageToObject.invert();
                aPageToObject *= getTransform();

                // embed in necessary transformation to map from SdrPage to SdrPageObject
                const Primitive2DReference xReferenceB(new TransformPrimitive2D(aPageToObject, aContent));
                xRetval = Primitive2DSequence(&xReferenceB, 1);
            }

            return xRetval;
        }

        PagePreviewPrimitive2D::PagePreviewPrimitive2D(
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& rxDrawPage,
            const basegfx::B2DHomMatrix& rTransform,
            double fContentWidth,
            double fContentHeight,
            const Primitive2DSequence& rChildren,
            bool bKeepAspectRatio)
        :   GroupPrimitive2D(rChildren),
            mxDrawPage(rxDrawPage),
            maTransform(rTransform),
            mfContentWidth(fContentWidth),
            mfContentHeight(fContentHeight),
            mbKeepAspectRatio(bKeepAspectRatio)
        {
        }

        bool PagePreviewPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(GroupPrimitive2D::operator==(rPrimitive))
            {
                const PagePreviewPrimitive2D& rCompare = static_cast< const PagePreviewPrimitive2D& >(rPrimitive);

                return (getXDrawPage() == rCompare.getXDrawPage()
                    && getTransform() == rCompare.getTransform()
                    && getContentWidth() == rCompare.getContentWidth()
                    && getContentHeight() == rCompare.getContentHeight()
                    && getKeepAspectRatio() == rCompare.getKeepAspectRatio());
            }

            return false;
        }

        basegfx::B2DRange PagePreviewPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation`*/) const
        {
            // nothing is allowed to stick out of a PagePreviewPrimitive, thus we
            // can quickly deliver our range here
            basegfx::B2DRange aRetval(0.0, 0.0, 1.0, 1.0);
            aRetval.transform(getTransform());
            return aRetval;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(PagePreviewPrimitive2D, PRIMITIVE2D_ID_PAGEPREVIEWPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
