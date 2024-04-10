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

#include <drawinglayer/primitive3d/modifiedcolorprimitive3d.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>
#include <utility>


namespace drawinglayer::primitive3d
{
        ModifiedColorPrimitive3D::ModifiedColorPrimitive3D(
            const Primitive3DContainer& rChildren,
            basegfx::BColorModifierSharedPtr xColorModifier)
        :   GroupPrimitive3D(rChildren),
            maColorModifier(std::move(xColorModifier))
        {
        }

        bool ModifiedColorPrimitive3D::operator==(const BasePrimitive3D& rPrimitive) const
        {
            if(GroupPrimitive3D::operator==(rPrimitive))
            {
                const ModifiedColorPrimitive3D& rCompare = static_cast<const ModifiedColorPrimitive3D&>(rPrimitive);

                if(getColorModifier().get() == rCompare.getColorModifier().get())
                {
                    return true;
                }

                if(!getColorModifier() || !rCompare.getColorModifier())
                {
                    return false;
                }

                return *getColorModifier() == *rCompare.getColorModifier();
            }

            return false;
        }

        // provide unique ID
        ImplPrimitive3DIDBlock(ModifiedColorPrimitive3D, PRIMITIVE3D_ID_MODIFIEDCOLORPRIMITIVE3D)

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
