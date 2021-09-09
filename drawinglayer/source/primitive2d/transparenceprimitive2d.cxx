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

#include <drawinglayer/primitive2d/transparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>


using namespace com::sun::star;


namespace drawinglayer::primitive2d
{
        TransparencePrimitive2D::TransparencePrimitive2D(
            Primitive2DContainer&& aChildren,
            Primitive2DContainer&& aTransparence)
        :   GroupPrimitive2D(std::move(aChildren)),
            maTransparence(std::move(aTransparence))
        {
        }

        bool TransparencePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(GroupPrimitive2D::operator==(rPrimitive))
            {
                const TransparencePrimitive2D& rCompare = static_cast<const TransparencePrimitive2D&>(rPrimitive);

                return (getTransparence() == rCompare.getTransparence());
            }

            return false;
        }

        // provide unique ID
        sal_uInt32 TransparencePrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_TRANSPARENCEPRIMITIVE2D;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
