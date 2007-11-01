/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dx_rendermodule.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 17:56:28 $
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

#ifndef _DXCANVAS_RENDERMODULE_HXX
#define _DXCANVAS_RENDERMODULE_HXX

#include <basegfx/vector/b2ivector.hxx>
#include <basegfx/range/b2irectangle.hxx>
#include <canvas/rendering/irendermodule.hxx>
#include <boost/shared_ptr.hpp>
#include "dx_winstuff.hxx"

class Window;
namespace basegfx
{
    class B2IRange;
    class B2DVector;
}

namespace dxcanvas
{
    /// Specialization of IRenderModule for DirectX
    struct IDXRenderModule : public canvas::IRenderModule
    {
        /** Flip front- and backbuffer, update only given area

            Note: Both update area and offset are ignored for
            fullscreen canvas, that uses page flipping (cannot, by
            definition, do anything else there except displaying the
            full backbuffer instead of the front buffer)

            @param rUpdateArea
            Area to copy from backbuffer to front

            @param rCurrWindowArea
            Current area of VCL window (coordinates relative to VCL
            HWND)
         */
        virtual bool flip( const ::basegfx::B2IRectangle& rUpdateArea,
                           const ::basegfx::B2IRectangle& rCurrWindowArea ) = 0;

        /** Resize backbuffer area for this render module
         */
        virtual void resize( const ::basegfx::B2IRange& rect ) = 0;

        /// Write a snapshot of the screen to disk
        virtual void screenShot() = 0;

        virtual COMReference<surface_type>
            createSystemMemorySurface(
                const ::basegfx::B2IVector& rSize ) = 0;

        virtual void disposing() = 0;
        virtual HWND getHWND() const = 0;
    };

    typedef ::boost::shared_ptr< IDXRenderModule > IDXRenderModuleSharedPtr;


    /** Factory method, to create an IRenderModule instance for the
        given VCL window instance
     */
    IDXRenderModuleSharedPtr createRenderModule( const ::Window& rParent );
}

#endif
