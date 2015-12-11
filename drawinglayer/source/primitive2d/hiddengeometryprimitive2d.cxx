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

#include <drawinglayer/primitive2d/hiddengeometryprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>



using namespace com::sun::star;



namespace drawinglayer
{
    namespace primitive2d
    {
        HiddenGeometryPrimitive2D::HiddenGeometryPrimitive2D(
            const Primitive2DContainer& rChildren)
        :   GroupPrimitive2D(rChildren)
        {
        }

        basegfx::B2DRange HiddenGeometryPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            return getChildren().getB2DRange(rViewInformation);
        }

        Primitive2DContainer HiddenGeometryPrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            return Primitive2DContainer();
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(HiddenGeometryPrimitive2D, PRIMITIVE2D_ID_HIDDENGEOMETRYPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
