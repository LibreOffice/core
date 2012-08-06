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

#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        GroupPrimitive2D::GroupPrimitive2D( const Primitive2DSequence& rChildren )
        :   BasePrimitive2D(),
            maChildren(rChildren)
        {
        }

        /** The compare opertator uses the Sequence::==operator, so only checking if
            the rererences are equal. All non-equal references are interpreted as
            non-equal.
         */
        bool GroupPrimitive2D::operator==( const BasePrimitive2D& rPrimitive ) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const GroupPrimitive2D& rCompare = static_cast< const GroupPrimitive2D& >(rPrimitive);

                return (arePrimitive2DSequencesEqual(getChildren(), rCompare.getChildren()));
            }

            return false;
        }

        /// default: just return children, so all renderers not supporting group will use it's content
        Primitive2DSequence GroupPrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            return getChildren();
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(GroupPrimitive2D, PRIMITIVE2D_ID_GROUPPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
