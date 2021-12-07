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

#pragma once

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

// GroupPrimitive2D class

namespace drawinglayer::primitive2d
{
        /** GroupPrimitive2D class

            Baseclass for all grouping 2D primitives

            The grouping primitive in its basic form is capable of holding
            a child primitive content and returns it on decomposition on default.
            It is used for two main purposes, but more may apply:

            - to transport extended information, e.g. for text classification,
              see e.g. TextHierarchy*Primitive2D implementations. Since they
              decompose to their child content, renderers not aware/interested
              in that extra information will just ignore these primitives

            - to encapsulate common geometry, e.g. the ShadowPrimitive2D implements
              applying a generic shadow to a child sequence by adding the needed
              offset and color stuff in the decomposition

            In most cases the decomposition is straightforward, so by default
            this primitive will not buffer the result and is not derived from
            BufferedDecompositionPrimitive2D, but from BasePrimitive2D.

            A renderer has to take GroupPrimitive2D derivations into account which
            are used to hold a state.

            Current Basic 2D StatePrimitives are:

            - TransparencePrimitive2D (objects with freely defined transparence)
            - InvertPrimitive2D (for XOR)
            - MaskPrimitive2D (for masking)
            - ModifiedColorPrimitive2D (for a stack of color modifications)
            - TransformPrimitive2D (for a transformation stack)
         */
        class DRAWINGLAYER_DLLPUBLIC GroupPrimitive2D : public BasePrimitive2D
        {
        private:
            /// the children. Declared private since this shall never be changed at all after construction
            Primitive2DContainer                             maChildren;

        public:
            /// constructor
            explicit GroupPrimitive2D(Primitive2DContainer&& aChildren);

            /// data read access
            const Primitive2DContainer& getChildren() const { return maChildren; }

            void getChildren(Primitive2DDecompositionVisitor& rVisitor) const { rVisitor.visit(maChildren); }

            /// compare operator
            virtual bool operator==( const BasePrimitive2D& rPrimitive ) const override;

            /// local decomposition. Implementation will just return children
            virtual void get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor, const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;

            // XAccounting
            virtual sal_Int64 SAL_CALL estimateUsage() override;
        };
} // end of namespace drawinglayer::primitive2d


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
