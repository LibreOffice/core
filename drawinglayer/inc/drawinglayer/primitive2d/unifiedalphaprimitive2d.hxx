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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_UNIFIEDALPHAPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_UNIFIEDALPHAPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/groupprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** UnifiedAlphaPrimitive2D class

            This primitive encapsualtes a child hierarchy and defines
            that it shall be visualized with the given transparency. That
            transparency is unique for all contained geometry, so that
            e.g. overlapping polygons in the child geometry will not show
            regions of combined transparency, but be all rendered with the
            defined, single transparency.
         */
        class UnifiedAlphaPrimitive2D : public GroupPrimitive2D
        {
        private:
            /// the unified alpha transparence
            double                              mfAlpha;

        public:
            /// constructor
            UnifiedAlphaPrimitive2D(
                const Primitive2DSequence& rChildren,
                double fAlpha);

            /// data read access
            double getAlpha() const { return mfAlpha; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// create decomposition
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_UNIFIEDALPHAPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
