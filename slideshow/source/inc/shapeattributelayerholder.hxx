/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_SLIDESHOW_SHAPEATTRIBUTELAYERHOLDER_HXX
#define INCLUDED_SLIDESHOW_SHAPEATTRIBUTELAYERHOLDER_HXX

#include "attributableshape.hxx"
#include "shapeattributelayer.hxx"

#include <boost/noncopyable.hpp>

namespace slideshow
{
    namespace internal
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
        class ShapeAttributeLayerHolder : private boost::noncopyable
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

                return mpAttributeLayer;
            }

            ShapeAttributeLayerSharedPtr get() const
            {
                return mpAttributeLayer;
            }

        private:
            AttributableShapeSharedPtr      mpShape;
            ShapeAttributeLayerSharedPtr    mpAttributeLayer;
        };

    }
}

#endif /* INCLUDED_SLIDESHOW_SHAPEATTRIBUTELAYERHOLDER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
