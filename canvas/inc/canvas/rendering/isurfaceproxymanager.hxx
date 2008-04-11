/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: isurfaceproxymanager.hxx,v $
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

#ifndef INCLUDED_CANVAS_ISURFACEPROXYMANAGER_HXX
#define INCLUDED_CANVAS_ISURFACEPROXYMANAGER_HXX

#include <canvas/rendering/irendermodule.hxx>
#include <canvas/rendering/icolorbuffer.hxx>

#include <boost/shared_ptr.hpp>


namespace canvas
{
    struct ISurfaceProxy;

    /** Manager interface, which handles surface proxy objects.

        Typically, each canvas instantiation has one
        SurfaceProxyManager object, to handle their surfaces. Surfaces
        itself are opaque objects, which encapsulate a framebuffer to
        render upon, plus an optional (possibly accelerated) texture.
     */
    struct ISurfaceProxyManager
    {
        virtual ~ISurfaceProxyManager() {}

        /** Create a surface proxy for a color buffer.

            The whole idea is build around the concept that you create
            some arbitrary buffer which contains the image data and
            tell the texture manager about it.  From there on you can
            draw into this image using any kind of graphics api you
            want.  In the technical sense we allocate some space in
            local videomemory or AGP memory which will be filled on
            demand, which means if there exists any rendering
            operation that needs to read from this memory location.
            This method creates a logical hardware surface object
            which uses the given color buffer as the image source.
            Internally this texture may even be distributed to several
            real hardware surfaces.
        */
        virtual ::boost::shared_ptr< ISurfaceProxy > createSurfaceProxy(
            const IColorBufferSharedPtr& pBuffer ) const = 0;
    };

    typedef ::boost::shared_ptr< ISurfaceProxyManager > ISurfaceProxyManagerSharedPtr;


    /** Create a surface proxy for the given render module.
     */
    ISurfaceProxyManagerSharedPtr createSurfaceProxyManager( const IRenderModuleSharedPtr& rRenderModule );
}

#endif /* INCLUDED_CANVAS_ISURFACEPROXYMANAGER_HXX */
