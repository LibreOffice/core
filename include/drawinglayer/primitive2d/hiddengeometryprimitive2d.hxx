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

#include <drawinglayer/primitive2d/groupprimitive2d.hxx>


namespace drawinglayer::primitive2d
{
        // This primitive is used to represent geometry for non-visible objects,
        // e.g. a PresObj's outline. To still be able to use primitives for HitTest
        // functionality, the 2d decompositions will produce an as much as possible
        // simplified line geometry encapsulated in this primitive when there is no
        // line geometry. In a further enhanced version this may change to 'if neither
        // filled nor lines' creation criteria. The whole primitive decomposes to nothing,
        // so no one not knowing it will be influenced. Only helper processors for hit test
        // (and maybe BoundRect extractors) will use it and its children subcontent.
        class DRAWINGLAYER_DLLPUBLIC HiddenGeometryPrimitive2D final : public GroupPrimitive2D
        {
        public:
            explicit HiddenGeometryPrimitive2D(const Primitive2DContainer& rChildren);

            // despite returning an empty decomposition since it's no visualisation data,
            // range calculation is intended to use hidden geometry, so
            // the local implementation will return the children's range
            virtual basegfx::B2DRange getB2DRange(VisitingParameters const & rParameters) const override;

            /// local decomposition. Implementation will return empty Primitive2DSequence
            virtual void get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor, VisitingParameters const & rParameters) const override;

            // provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };
} // end of namespace drawinglayer::primitive2d


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
