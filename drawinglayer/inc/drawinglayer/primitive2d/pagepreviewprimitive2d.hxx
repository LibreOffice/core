/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PAGEPREVIEWPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PAGEPREVIEWPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <basegfx/matrix/b2dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** PagePreviewPrimitive2D class

            This primitive is needed to have the correct XDrawPage as ViewInformation for decomposing
            the page contents (given as PageContent here) if these contain e.g.
            view-dependent (in this case XDrawPage-dependent) text fields like PageNumber. If You want
            those primitives to be visualized correctly, Your renderer needs to locally correct it's
            ViewInformation2D to reference the new XDrawPage.
         */
        class DRAWINGLAYER_DLLPUBLIC PagePreviewPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /** the XDrawPage visualized by this primitive. When we go forward with primitives
                this will not only be used by the renderers to provide the correct decompose
                graphic attribute context, but also to completely create the page's sub-content.
             */
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > mxDrawPage;

            /// the PageContent
            Primitive2DSequence                         maPageContent;

            /// the own geometry
            basegfx::B2DHomMatrix                       maTransform;

            /// content width and height
            double                                      mfContentWidth;
            double                                      mfContentHeight;

            /// bitfield
            /// flag to allow keeping the aspect ratio
            unsigned                                    mbKeepAspectRatio : 1;

        protected:
            /// local decomposition. Implementation will just return children
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            PagePreviewPrimitive2D(
                const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& rxDrawPage,
                const basegfx::B2DHomMatrix& rTransform,
                double fContentWidth,
                double fContentHeight,
                const Primitive2DSequence& rPageContent,
                bool bKeepAspectRatio);

            /// data read access
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& getXDrawPage() const { return mxDrawPage; }
            const Primitive2DSequence& getPageContent() const { return maPageContent; }
            const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }
            double getContentWidth() const { return mfContentWidth; }
            double getContentHeight() const { return mfContentHeight; }
            bool getKeepAspectRatio() const { return mbKeepAspectRatio; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// own getB2DRange
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PAGEPREVIEWPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
