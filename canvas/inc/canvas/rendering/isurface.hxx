/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: isurface.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 12:45:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_CANVAS_ISURFACE_HXX
#define INCLUDED_CANVAS_ISURFACE_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

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
