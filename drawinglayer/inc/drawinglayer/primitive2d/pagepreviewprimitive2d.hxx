/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pagepreviewprimitive2d.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2008-02-07 13:42:32 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PAGEPREVIEWPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PAGEPREVIEWPRIMITIVE2D_HXX

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GROUPPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        // This primitive is needed to have the correct XDrawPage as ViewInformation for decomposing
        // the page contents (given as childs of the GroupPrimitive2D here) if these contain e.g.
        // view-dependent (in this case XDrawPage-dependent) text fields like PageNumber. If You want
        // those primitives to be visualized correctly, Your renderer needs to locally correct it's
        // ViewInformation2D to reference the new XDrawPage.
        class PagePreviewPrimitive2D : public GroupPrimitive2D
        {
        private:
            // the XDrawPage visualized by this primitive. When we go forward with primitives
            // this will not only be used by the renderers to provide the correct decompose
            // graphic attribute context, but also to completely create the page's sub-content.
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > mxDrawPage;

            // the own geometry
            basegfx::B2DHomMatrix                       maTransform;

            // content width and height
            double                                      mfContentWidth;
            double                                      mfContentHeight;

            // bitfield
            // flag to allow keeping the aspect ratio
            unsigned                                    mbKeepAspectRatio : 1;

        protected:
            // local decomposition. Implementation will just return children
            virtual Primitive2DSequence createLocalDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            PagePreviewPrimitive2D(
                const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& rxDrawPage,
                const basegfx::B2DHomMatrix& rTransform,
                double fContentWidth,
                double fContentHeight,
                const Primitive2DSequence& rChildren,
                bool bKeepAspectRatio);

            // get data
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& getXDrawPage() const { return mxDrawPage; }
            const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }
            double getContentWidth() const { return mfContentWidth; }
            double getContentHeight() const { return mfContentHeight; }
            bool getKeepAspectRatio() const { return mbKeepAspectRatio; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            // own getB2DRange
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PAGEPREVIEWPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
