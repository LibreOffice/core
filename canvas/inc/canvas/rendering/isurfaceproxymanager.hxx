/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: isurfaceproxymanager.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 12:45:56 $
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

#ifndef INCLUDED_CANVAS_ISURFACEPROXYMANAGER_HXX
#define INCLUDED_CANVAS_ISURFACEPROXYMANAGER_HXX

#ifndef INCLUDED_CANVAS_IRENDERMODULE_HXX
#include <canvas/rendering/irendermodule.hxx>
#endif
#ifndef INCLUDED_CANVAS_ICOLORBUFFER_HXX
#include <canvas/rendering/icolorbuffer.hxx>
#endif

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
