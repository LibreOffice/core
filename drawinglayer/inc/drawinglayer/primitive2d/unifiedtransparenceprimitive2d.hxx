/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_UNIFIEDTRANSPARENCEPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_UNIFIEDTRANSPARENCEPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/groupprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** UnifiedTransparencePrimitive2D class

            This primitive encapsualtes a child hierarchy and defines
            that it shall be visualized with the given transparency. That
            transparency is unique for all contained geometry, so that
            e.g. overlapping polygons in the child geometry will not show
            regions of combined transparency, but be all rendered with the
            defined, single transparency.
         */
        class DRAWINGLAYER_DLLPUBLIC UnifiedTransparencePrimitive2D : public GroupPrimitive2D
        {
        private:
            /// the unified transparence
            double                              mfTransparence;

        public:
            /// constructor
            UnifiedTransparencePrimitive2D(
                const Primitive2DSequence& rChildren,
                double fTransparence);

            /// data read access
            double getTransparence() const { return mfTransparence; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// own getB2DRange implementation to include transparent geometries to BoundRect calculations
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /// create decomposition
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_UNIFIEDTRANSPARENCEPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
