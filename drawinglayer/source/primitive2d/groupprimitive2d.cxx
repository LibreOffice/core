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


using namespace com::sun::star;


namespace drawinglayer::primitive2d
{
        GroupPrimitive2D::GroupPrimitive2D( Primitive2DContainer&& aChildren )
        :   maChildren(std::move(aChildren))
        {
        }

        /** The compare opertator uses the Sequence::==operator, so only checking if
            the references are equal. All non-equal references are interpreted as
            non-equal.
         */
        bool GroupPrimitive2D::operator==( const BasePrimitive2D& rPrimitive ) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const GroupPrimitive2D& rCompare = static_cast< const GroupPrimitive2D& >(rPrimitive);

                return getChildren() == rCompare.getChildren();
            }

            return false;
        }

        /// default: just return children, so all renderers not supporting group will use its content
        void GroupPrimitive2D::get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor, const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            getChildren(rVisitor);
        }

        sal_Int64 SAL_CALL GroupPrimitive2D::estimateUsage()
        {
            size_t nRet(0);
            for (auto& it : getChildren())
            {
                nRet += it->estimateUsage();
            }
            return nRet;
        }

        // provide unique ID
        sal_uInt32 GroupPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_GROUPPRIMITIVE2D;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
