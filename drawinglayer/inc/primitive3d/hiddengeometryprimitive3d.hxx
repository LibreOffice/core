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

#include <drawinglayer/primitive3d/groupprimitive3d.hxx>


namespace drawinglayer::primitive3d
    {
        // This primitive is used to represent geometry for non-visible objects,
        // e.g. a 3D cube without fill attributes. To still be able to use
        // primitives for HitTest functionality, the 3d decompositions produce
        // an as much as possible simplified fill geometry encapsulated in this
        // primitive when there is no fill geometry. Currently, the 3d hit test
        // uses only areas, so maybe in a further enhanced version this will change
        // to 'if neither filled nor lines' creation criteria. The whole primitive
        // decomposes to nothing, so no one not knowing it will be influenced. Only
        // helper processors for hit test (and maybe BoundRect extractors) will
        // use it and its children subcontent.
        class HiddenGeometryPrimitive3D final : public GroupPrimitive3D
        {
        public:
            explicit HiddenGeometryPrimitive3D(const Primitive3DContainer& rChildren);

            // despite returning an empty decomposition since it's no visualisation data,
            // range calculation is intended to use hidden geometry, so
            // the local implementation will return the children's range
            virtual basegfx::B3DRange getB3DRange(const geometry::ViewInformation3D& rViewInformation) const override;

            /// The default implementation returns an empty sequence
            virtual Primitive3DContainer get3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const override;

            // provide unique ID
            DeclPrimitive3DIDBlock()
        };

} // end of namespace drawinglayer::primitive3d


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
