/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dx_rendermodule.hxx,v $
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
