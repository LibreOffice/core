/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: graphicprimitive2d.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:17 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GRAPHICPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GRAPHICPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <goodies/grfmgr.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** GraphicPrimitive2D class

            Primitive to hold graphics defined by GraphicObject and GraphicAttr
            combination. This includes MetaFiles and diverse pixel-oriented graphic
            formats. It even includes animated GIFs, Croppings and other changes
            defined in GraphicAttr.

            This makes the decomposition contain a wide variety of possibilites,
            too. From a simple BitmapPrimitive over AnimatedSwitchPrimitive2D,
            MetafilePrimitive2D (with and without embedding in a masking when e.g.
            the Metafile is bigger than the geometry) and embeddings in
            TransformPrimitive2D and MaskPrimitive2D for croppings.

            The primitive geometry area is defined by Transform.
         */
        class GraphicPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the geometric definition
            basegfx::B2DHomMatrix                       maTransform;

            /// the GraphicObject with all it's content possibilities
            GraphicObject                               maGraphicObject;

            /// The GraphicAttr with all it's modification possibilities
            GraphicAttr                                 maGraphicAttr;

        protected:
            /// local decomposition
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor(s)
            GraphicPrimitive2D(
                const basegfx::B2DHomMatrix& rTransform,
                const GraphicObject& rGraphicObject,
                const GraphicAttr& rGraphicAttr);
            GraphicPrimitive2D(
                const basegfx::B2DHomMatrix& rTransform,
                const GraphicObject& rGraphicObject);

            /// data read access
            const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }
            const GraphicObject& getGraphicObject() const { return maGraphicObject; }
            const GraphicAttr& getGraphicAttr() const { return maGraphicAttr; }
            bool isTransparent() const;

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GRAPHICPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
