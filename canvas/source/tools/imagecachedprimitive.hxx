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

#ifndef INCLUDED_CANVAS_IMAGECACHEDPRIMITIVE_HXX
#define INCLUDED_CANVAS_IMAGECACHEDPRIMITIVE_HXX

#include <canvas/rendering/icachedprimitive.hxx>

#include <boost/shared_ptr.hpp>


namespace canvas
{
    /** Objects with this interface are returned from every Image
        render operation.

        These objects can be used to implement the
        rendering::XCachedPrimitive interface, which in turn caches
        render state and objects to facilitate quick redraws.

        Derived from ICachedPrimitive, to add the setImage() method
        (which, strictly speaking, is a technicality, because Image
        cannot create objects with a shared_ptr to itself).
     */
    struct ImageCachedPrimitive : public ICachedPrimitive
    {
        virtual void setImage( const ::boost::shared_ptr< class Image >& rTargetImage ) = 0;
    };

    typedef ::boost::shared_ptr< ImageCachedPrimitive > ImageCachedPrimitiveSharedPtr;
}

#endif /* INCLUDED_CANVAS_IMAGECACHEDPRIMITIVE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
