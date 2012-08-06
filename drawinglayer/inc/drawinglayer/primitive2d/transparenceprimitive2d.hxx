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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TRANSPARENCEPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TRANSPARENCEPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/groupprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** TransparencePrimitive2D class

            This is the basic primitive for applying freely defined transparence
            to freely defined content. The basic idea is to associate a content
            which is defined as a sequence of primitives and hold as child content
            in the GroupPrimitive2D with a transparence channel also defined as a sequence
            of primitives and hold in the transparence member.

            The basic definition is to use the transparence content as transparence-Mask by
            interpreting the transparence-content not as RGB, but as Luminance transparence mask
            using the common RGB_to_luminance definition as e.g. used by VCL.

            The defining geometry is the Range of the child primitive sequence,
            this means the renderers will/shall use this geometric information for
            rendering, not the transparent one. The transparent one should/will be clipped
            accordingly.
         */
        class DRAWINGLAYER_DLLPUBLIC TransparencePrimitive2D : public GroupPrimitive2D
        {
        private:
            /// The transparence-Mask who's RGB-Values are interpreted as Luminance
            Primitive2DSequence                     maTransparence;

        public:
            /** constructor

                @param rChildren
                The content which is defined to have a transparency. The
                range of this primitive is defined by this content

                @param rTransparence
                The definition of the Transparence-channel for this primitive. It
                will be interpreted as mask by interpreting as gray values
                using the common RGB_to_luminance definitions
             */
            TransparencePrimitive2D(
                const Primitive2DSequence& rChildren,
                const Primitive2DSequence& rTransparence);

            /// data read access
            const Primitive2DSequence& getTransparence() const { return maTransparence; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TRANSPARENCEPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
