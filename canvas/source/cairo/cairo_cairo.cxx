/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cairo_cairo.cxx,v $
 * $Revision: 1.8 $
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

/****************************************************************************************
 * Platform independent part of surface backends for OpenOffice.org Cairo Canvas        *
 * For rest of the functions, see platform specific cairo_<platform>_cairo.cxx          *
 ****************************************************************************************/

#include "cairo_cairo.hxx"

namespace cairo
{

/**
   * Surface::getCairo:  Create Cairo for the Canvas surface
   *
   * @return new Cairo or NULL
   **/
    Cairo* Surface::getCairo()
    {
        Cairo *cr = NULL;
        if (mpSurface) {
            cr = cairo_create( mpSurface );
        }
        return cr;
    }


// This is needed to distinguish support for Cairo versions < 1.2.
#if !defined (USE_CAIRO10_APIS)

  /**
   * Surface::~Surface:  Destroy the Canvas surface
   *
   * Cairo itself takes care of freeing any resources, such as
   * image data and other references related to the surface.
   *
   **/
    Surface::~Surface()
    {
        if( mpSurface )
        {
            cairo_surface_destroy( mpSurface );
            mpSurface = NULL;
        }
    }


  /**
   * Surface::getSimilar:  Create new similar Canvas surface
   * @param aContent format of the new surface (cairo_content_t from cairo/src/cairo.h)
   * @param width width of the new surface
   * @param height height of the new surface
   *
   * Creates a new Canvas surface. This should create platform native surface, even though
   * generic function is used.
   *
   * Cairo surface from aContent (cairo_content_t)
   *
   * @return new surface or NULL
   **/
    Surface* Surface::getSimilar( Content aContent, int width, int height )
    {
        // This should create platform native Cairo surface on ALL platforms.
        return new Surface( cairo_surface_create_similar( mpSurface, aContent, width, height ) );

        // For example on Mac OS X, cairo_surface_create_similar() actually results in native surface,
        // equivalent to cairo_quartz_surface_create()
    }
#endif   // !defined (USE_CAIRO10_APIS)


  /**
   * Surface::createVirtualDevice:  Create new VCL virtual device
   *
   * Creates a new virtual device in VCL, with the current mpSurface contents as data.
   * This is used by e.g. cairo_canvashelper_text.cxx to make VCL draw text on the cairo surface.
   *
   * @return new virtual device
   **/
    VirtualDevice* Surface::createVirtualDevice()
    {
        // struct SystemGraphicsData in vcl/inc/sysdata.hxx
        SystemGraphicsData aSystemGraphicsData;

        aSystemGraphicsData.nSize = sizeof(SystemGraphicsData);
        fillSystemGraphicsData( aSystemGraphicsData );

        /**
         *    Contrary to the description of
         *    VirtualDevice(SystemGraphicsData*, nBitCount);  (in vcl/inc/virdev.hxx)
         *    at least X11 behaves differently, i.e. nBitCount is color depth, not just 1 or 0.
        **/
        return new VirtualDevice( &aSystemGraphicsData,
                                  sal::static_int_cast<USHORT>(getDepth()) );
    }

}  // namespace cairo


