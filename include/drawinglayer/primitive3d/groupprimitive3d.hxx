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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_GROUPPRIMITIVE3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_GROUPPRIMITIVE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive3d/baseprimitive3d.hxx>



namespace drawinglayer
{
    namespace primitive3d
    {
        /** GroupPrimitive3D class

            Baseclass for all grouping 3D primitives

            The description/functionality is identical with the 2D case in groupprimitive2d.hxx,
            please see there for detailed information.

            Current Basic 3D StatePrimitives are:

            - ModifiedColorPrimitive3D (for a stack of color modifications)
            - ShadowPrimitive3D (for 3D objects with shadow; this is a special case
              since the shadow of a 3D primitive is a 2D primitive set)
            - TexturePrimitive3D (with the following variations)
                - GradientTexturePrimitive3D (for 3D gradient fill)
                - BitmapTexturePrimitive3D (for 3D Bitmap fill)
                - TransparenceTexturePrimitive3D (for 3D transparence)
                - HatchTexturePrimitive3D (for 3D hatch fill)
            - TransformPrimitive3D (for a transformation stack)
         */
        class DRAWINGLAYER_DLLPUBLIC GroupPrimitive3D : public BasePrimitive3D
        {
        private:
            /// the children. Declared private since this shall never be changed at all after construction
            Primitive3DSequence                             maChildren;

        public:
            /// constructor
            explicit GroupPrimitive3D(const Primitive3DSequence& rChildren);

            /// data read access
            Primitive3DSequence getChildren() const { return maChildren; }

            /// compare operator
            virtual bool operator==( const BasePrimitive3D& rPrimitive ) const override;

            /// local decomposition. Implementation will just return children
            virtual Primitive3DSequence get3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const override;

            /// provide unique ID
            DeclPrimitive3DIDBlock()
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer



#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_GROUPPRIMITIVE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
