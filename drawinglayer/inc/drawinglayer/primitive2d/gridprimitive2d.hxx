/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gridprimitive2d.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2008-06-24 15:30:17 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GRIDPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GRIDPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/color/bcolor.hxx>
#include <vcl/bitmapex.hxx>

//////////////////////////////////////////////////////////////////////////////
// GridPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        class GridPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            basegfx::B2DHomMatrix                           maTransform;
            double                                          mfWidth;
            double                                          mfHeight;
            double                                          mfSmallestViewDistance;
            double                                          mfSmallestSubdivisionViewDistance;
            sal_uInt32                                      mnSubdivisionsX;
            sal_uInt32                                      mnSubdivisionsY;
            basegfx::BColor                                 maBColor;
            BitmapEx                                        maCrossMarker;

            // the last used object to view transformtion and the last Viewport,
            // used from getDecomposition for decide buffering
            basegfx::B2DHomMatrix                           maLastObjectToViewTransformation;
            basegfx::B2DRange                               maLastViewport;

        protected:
            // create local decomposition
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            GridPrimitive2D(
                const basegfx::B2DHomMatrix& rTransform,
                double fWidth,
                double fHeight,
                double fSmallestViewDistance,
                double fSmallestSubdivisionViewDistance,
                sal_uInt32 nSubdivisionsX,
                sal_uInt32 nSubdivisionsY,
                const basegfx::BColor& rBColor,
                const BitmapEx& rCrossMarker);

            // get data
            const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }
            double getWidth() const { return mfWidth; }
            double getHeight() const { return mfHeight; }
            double getSmallestViewDistance() const { return mfSmallestViewDistance; }
            double getSmallestSubdivisionViewDistance() const { return mfSmallestSubdivisionViewDistance; }
            sal_uInt32 getSubdivisionsX() const { return mnSubdivisionsX; }
            sal_uInt32 getSubdivisionsY() const { return mnSubdivisionsY; }
            const basegfx::BColor& getBColor() const { return maBColor; }
            const BitmapEx& getCrossMarker() const { return maCrossMarker; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            // get 2d range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()

            // Overload standard getDecomposition call to be view-dependent here
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GRIDPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
