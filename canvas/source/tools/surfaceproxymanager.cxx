/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: surfaceproxymanager.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 12:56:15 $
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
