/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TRANSPARENCEPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TRANSPARENCEPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/groupprimitive2d.hxx>



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
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer



#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TRANSPARENCEPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
