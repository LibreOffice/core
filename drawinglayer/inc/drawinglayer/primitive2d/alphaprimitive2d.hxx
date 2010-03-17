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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_ALPHAPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_ALPHAPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/groupprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** AlphaPrimitive2D class

            This is the basic primitive for applying freely defined transparence
            to freely defined content. The basic idea is to associate a content
            which is defined as a sequence of primitives and hold as child content
            in the GroupPrimitive2D with an alpha channel also defined as a sequence
            of primitives and hold in the Alpha member.

            The basic definition is to use the Alpha content as Alpha-Mask by
            interpreting the Alpha-content not as RGB, but as Luminance alpha mask
            using the common RGB_to_luminance definition as e.g. used by VCL.

            The defining geometry is the Range of the child primitive sequence,
            this means the renderers will/shall use this geometric information for
            rendering, not the alpha one. The alpha one should/will be clipped
            accordingly.
         */
        class AlphaPrimitive2D : public GroupPrimitive2D
        {
        private:
            /// The Alpha-Mask who's RGB-Values are interpreted as Luminance
            Primitive2DSequence                     maAlpha;

        public:
            /** constructor

                @param rChildren
                The content which is defined to have a transparency. The
                range of this primitive is defined by this content

                @param rAlpha
                The definition of the Alpha-channel for this primitive. It
                will be interpreted as mask by interpreting as gray values
                using the common RGB_to_luminance definitions
             */
            AlphaPrimitive2D(
                const Primitive2DSequence& rChildren,
                const Primitive2DSequence& rAlpha);

            /// data read access
            const Primitive2DSequence& getAlpha() const { return maAlpha; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_ALPHAPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
