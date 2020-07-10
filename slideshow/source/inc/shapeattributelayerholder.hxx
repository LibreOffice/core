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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_SHAPEATTRIBUTELAYERHOLDER_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_SHAPEATTRIBUTELAYERHOLDER_HXX

#include "attributableshape.hxx"
#include "shapeattributelayer.hxx"

namespace slideshow::internal
    {
        /** Holds a ShapeAttributeLayer, together with the associated
            Shape

            Use this class to hold ShapeAttributeLayer objects the
            RAII way. When this object gets deleted, it will
            automatically revoke the attribute layer for the given
            shape (this encapsulates the somewhat clumsy notification
            process that is required for shape and attribute layer
            interaction).
         */
        class ShapeAttributeLayerHolder
        {
        public:
            /** Create a ShapeAttributeLayerHolder instance.

                This constructor creates an empty attribute holder, to
                generate an attribute layer, you have to manually call
                createAttributeLayer().
             */
            ShapeAttributeLayerHolder() :
                mpShape(),
                mpAttributeLayer()
            {
            }

            ~ShapeAttributeLayerHolder()
            {
                reset(); // ensures that the last attribute layer is
                         // correctly deregistered from the shape.
            }

            ShapeAttributeLayerHolder(const ShapeAttributeLayerHolder&) = delete;
            ShapeAttributeLayerHolder& operator=(const ShapeAttributeLayerHolder&) = delete;

            void reset()
            {
                if( mpShape && mpAttributeLayer )
                    mpShape->revokeAttributeLayer( mpAttributeLayer );
            }

            /** This constructor receives a pointer to the Shape, from
                which attribute layers should be generated. Initially,
                this object does not create an attribute layer, you
                have to manually call createAttributeLayer().

                @param rShape
                Shape for which attribute layers should be generated.
            */
            bool createAttributeLayer( const AttributableShapeSharedPtr& rShape )
            {
                reset();

                mpShape = rShape;

                if( mpShape )
                    mpAttributeLayer = mpShape->createAttributeLayer();

                return static_cast< bool >(mpAttributeLayer);
            }

            const ShapeAttributeLayerSharedPtr& get() const
            {
                return mpAttributeLayer;
            }

        private:
            AttributableShapeSharedPtr      mpShape;
            ShapeAttributeLayerSharedPtr    mpAttributeLayer;
        };

}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_SHAPEATTRIBUTELAYERHOLDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
