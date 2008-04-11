/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salogl.hxx,v $
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

#ifndef _SV_SALOGL_HXX
#define _SV_SALOGL_HXX

#define _OPENGL_EXT

#include <tools/gen.hxx>

#define _SVUNX_H
#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/salotype.hxx>

// -----------------
// - State defines -
// -----------------

#define OGL_STATE_UNLOADED      (0x00000000)
#define OGL_STATE_INVALID       (0x00000001)
#define OGL_STATE_VALID     (0x00000002)

#ifdef WNT
#define __OPENGL_CALL __stdcall
#else
#define __OPENGL_CALL
#endif

extern "C"
{
typedef void( __OPENGL_CALL *oglFunction )(void);
}

// -------------
// - SalOpenGL -
// -------------

class SalGraphics;

class VCL_DLLPUBLIC SalOpenGL
{
public:

    SalOpenGL() {}
    virtual ~SalOpenGL();

    virtual bool        IsValid() = 0;
    virtual oglFunction GetOGLFnc( const char * ) = 0;
    virtual void        OGLEntry( SalGraphics* pGraphics ) = 0;
    virtual void        OGLExit( SalGraphics* pGraphics ) = 0;
    virtual void        StartScene( SalGraphics* pGraphics ) = 0;
    virtual void        StopScene() = 0;
};

#endif
