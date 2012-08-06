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

#include <drawinglayer/primitive3d/hiddengeometryprimitive3d.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        HiddenGeometryPrimitive3D::HiddenGeometryPrimitive3D(
            const Primitive3DSequence& rChildren)
        :   GroupPrimitive3D(rChildren)
        {
        }

        basegfx::B3DRange HiddenGeometryPrimitive3D::getB3DRange(const geometry::ViewInformation3D& rViewInformation) const
        {
            return getB3DRangeFromPrimitive3DSequence(getChildren(), rViewInformation);
        }

        Primitive3DSequence HiddenGeometryPrimitive3D::get3DDecomposition(const geometry::ViewInformation3D& /*rViewInformation*/) const
        {
            // return empty sequence
            return Primitive3DSequence();
        }

        // provide unique ID
        ImplPrimitrive3DIDBlock(HiddenGeometryPrimitive3D, PRIMITIVE3D_ID_HIDDENGEOMETRYPRIMITIVE3D)

    } // end of namespace primitive3d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
