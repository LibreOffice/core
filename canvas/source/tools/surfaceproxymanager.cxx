/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: surfaceproxymanager.cxx,v $
 * $Revision: 1.4 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_canvas.hxx"

#include <canvas/rendering/isurfaceproxymanager.hxx>
#include <canvas/rendering/isurfaceproxy.hxx>
#include "surfaceproxy.hxx"

namespace canvas
{

    //////////////////////////////////////////////////////////////////////////////////
    // SurfaceProxyManager
    //////////////////////////////////////////////////////////////////////////////////

    class SurfaceProxyManager : public ISurfaceProxyManager
    {
    public:

        SurfaceProxyManager( const IRenderModuleSharedPtr pRenderModule ) :
            mpPageManager( new PageManager(pRenderModule) )
        {
        }

        /** the whole idea is build around the concept that you create
            some arbitrary buffer which contains the image data and
            tell the texture manager about it.  from there on you can
            draw this image using any kind of graphics api you want.
            in the technical sense we allocate some space in local
            videomemory or AGP memory which will be filled on demand,
            which means if there exists any rendering operation that
            needs to read from this memory location.  this method
            creates a logical hardware surface object which uses the
            given color buffer as the image source.  internally this
            texture may be distributed to several real hardware
            surfaces.
        */
        virtual ISurfaceProxySharedPtr createSurfaceProxy( const IColorBufferSharedPtr& pBuffer ) const
        {
            // not much to do for now, simply allocate a new surface
            // proxy from our internal pool and initialize this thing
            // properly. we *don't* create a hardware surface for now.
            return SurfaceProxySharedPtr(new SurfaceProxy(pBuffer,mpPageManager));
        }

    private:
        PageManagerSharedPtr mpPageManager;
    };

    //////////////////////////////////////////////////////////////////////////////////
    // createSurfaceProxyManager
    //////////////////////////////////////////////////////////////////////////////////

    ISurfaceProxyManagerSharedPtr createSurfaceProxyManager( const IRenderModuleSharedPtr& rRenderModule )
    {
        return ISurfaceProxyManagerSharedPtr(
            new SurfaceProxyManager(
                rRenderModule));
    }
}
