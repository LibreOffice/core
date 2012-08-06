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

#include <drawinglayer/primitive3d/shadowprimitive3d.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        ShadowPrimitive3D::ShadowPrimitive3D(
            const basegfx::B2DHomMatrix& rShadowTransform,
            const basegfx::BColor& rShadowColor,
            double fShadowTransparence,
            bool bShadow3D,
            const Primitive3DSequence& rChildren)
        :   GroupPrimitive3D(rChildren),
            maShadowTransform(rShadowTransform),
            maShadowColor(rShadowColor),
            mfShadowTransparence(fShadowTransparence),
            mbShadow3D(bShadow3D)
        {
        }

        bool ShadowPrimitive3D::operator==(const BasePrimitive3D& rPrimitive) const
        {
            if(GroupPrimitive3D::operator==(rPrimitive))
            {
                const ShadowPrimitive3D& rCompare = (ShadowPrimitive3D&)rPrimitive;

                return (getShadowTransform() == rCompare.getShadowTransform()
                    && getShadowColor() == rCompare.getShadowColor()
                    && getShadowTransparence() == rCompare.getShadowTransparence()
                    && getShadow3D() == rCompare.getShadow3D());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive3DIDBlock(ShadowPrimitive3D, PRIMITIVE3D_ID_SHADOWPRIMITIVE3D)

    } // end of namespace primitive3d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
