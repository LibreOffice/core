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

#include <drawinglayer/primitive3d/groupprimitive3d.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>
#include <utility>


namespace drawinglayer::primitive3d
{
        GroupPrimitive3D::GroupPrimitive3D( Primitive3DContainer aChildren )
        :   maChildren(std::move(aChildren))
        {
        }

        /** The compare operator uses the Sequence::==operator, so only checking if
            the references are equal. All non-equal references are interpreted as
            non-equal.
         */
        bool GroupPrimitive3D::operator==( const BasePrimitive3D& rPrimitive ) const
        {
            if(BasePrimitive3D::operator==(rPrimitive))
            {
                const GroupPrimitive3D& rCompare = static_cast< const GroupPrimitive3D& >(rPrimitive);

                return getChildren() == rCompare.getChildren();
            }

            return false;
        }

        /// default: just return children, so all renderers not supporting group will use its content
        Primitive3DContainer GroupPrimitive3D::get3DDecomposition(const geometry::ViewInformation3D& /*rViewInformation*/) const
        {
            return getChildren();
        }

        // provide unique ID
        ImplPrimitive3DIDBlock(GroupPrimitive3D, PRIMITIVE3D_ID_GROUPPRIMITIVE3D)

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
