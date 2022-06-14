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
#include <drawinglayer/primitive2d/objectinfoprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <utility>

using namespace com::sun::star;

namespace drawinglayer::primitive2d
{
        ObjectInfoPrimitive2D::ObjectInfoPrimitive2D(
            Primitive2DContainer&& aChildren,
            OUString aName,
            OUString aTitle,
            OUString aDesc)
        :   GroupPrimitive2D(std::move(aChildren)),
            maName(std::move(aName)),
            maTitle(std::move(aTitle)),
            maDesc(std::move(aDesc))
        {
        }

        bool ObjectInfoPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(GroupPrimitive2D::operator==(rPrimitive))
            {
                const ObjectInfoPrimitive2D& rCompare = static_cast<const ObjectInfoPrimitive2D&>(rPrimitive);

                return (getName() == rCompare.getName()
                    && getTitle() == rCompare.getTitle()
                    && getDesc() == rCompare.getDesc());
            }

            return false;
        }

        // provide unique ID
        sal_uInt32 ObjectInfoPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_OBJECTINFOPRIMITIVE2D;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
