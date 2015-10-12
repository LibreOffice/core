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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_TRANSFORMPRIMITIVE3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_TRANSFORMPRIMITIVE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive3d/groupprimitive3d.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>



namespace drawinglayer
{
    namespace primitive3d
    {
        /** TransformPrimitive3D class

            This is one of the basic grouping 3D primitives and it provides
            embedding a sequence of primitives (a geometry) into a
            3D transformation.

            Please see the description for TransformPrimitive2D since these
            primitives are pretty similar.
         */
        class DRAWINGLAYER_DLLPUBLIC TransformPrimitive3D : public GroupPrimitive3D
        {
        private:
            // the 3D transformation to apply
            basegfx::B3DHomMatrix                   maTransformation;

        public:
            /// constructor
            TransformPrimitive3D(
                const basegfx::B3DHomMatrix& rTransformation,
                const Primitive3DSequence& rChildren);

            /// data read access
            const basegfx::B3DHomMatrix& getTransformation() const { return maTransformation; }

            /// compare operator
            virtual bool operator==(const BasePrimitive3D& rPrimitive) const override;

            /// get range
            virtual basegfx::B3DRange getB3DRange(const geometry::ViewInformation3D& rViewInformation) const override;

            /// provide unique ID
            DeclPrimitive3DIDBlock()
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer



#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_TRANSFORMPRIMITIVE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
