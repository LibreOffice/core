/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: isurface.hxx,v $
 * $Revision: 1.3 $
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

#ifndef INCLUDED_CANVAS_ISURFACE_HXX
#define INCLUDED_CANVAS_ISURFACE_HXX

#include <sal/types.h>

#include <boost/shared_ptr.hpp>


namespace basegfx
{
    class B2IRange;
    class B2IPoint;
    class B2IVector;
}

namespace canvas
{
    struct IColorBuffer;

    struct ISurface
    {
        virtual ~ISurface() {}

        /** Select texture behind this interface to be the current one
            for primitive output.
         */
        virtual bool selectTexture() = 0;

        /** Tells wether the surface is valid or not
        */
        virtual bool isValid() = 0;

        /** Update surface content from given IColorBuffer

            This method updates the given subarea of the surface from
            the given color buffer bits.

            @param rDestPos
            Position in the surface, where the subset update should
            have its left, top edge

            @param rSourceRect
            Size and position of the rectangular subset update in the
            source color buffer

            @param rSource
            Source bits to use for the update

            @return true, if the update was successful
         */
        virtual bool update( const ::basegfx::B2IPoint& rDestPos,
                             const ::basegfx::B2IRange& rSourceRect,
                             IColorBuffer&              rSource ) = 0;

        virtual ::basegfx::B2IVector getSize() = 0;
    };

    typedef ::boost::shared_ptr< ISurface > ISurfaceSharedPtr;
}

#endif /* INCLUDED_CANVAS_ISURFACE_HXX */
