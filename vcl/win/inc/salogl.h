/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salogl.h,v $
 * $Revision: 1.6 $
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

#ifndef _SV_SALOGL_H
#define _SV_SALOGL_H

#define _OPENGL_EXT

#include <wincomp.hxx>
#include <tools/gen.hxx>
#include <vcl/sv.h>
#include <vcl/salogl.hxx>

// -----------------
// - State defines -
// -----------------

#define OGL_STATE_UNLOADED      (0x00000000)
#define OGL_STATE_INVALID       (0x00000001)
#define OGL_STATE_VALID         (0x00000002)

// -------------
// - SalOpenGL -
// -------------

class SalGraphics;
class String;

class WinSalOpenGL : public SalOpenGL
{
private:
    static HGLRC        mhOGLContext;
    static HDC          mhOGLLastDC;
    static ULONG        mnOGLState;

private:
    static BOOL         ImplInitLib();
    static BOOL         ImplInit();
    static void         ImplFreeLib();

public:
    WinSalOpenGL( SalGraphics* );
    virtual ~WinSalOpenGL();

    virtual bool        IsValid();
    virtual oglFunction GetOGLFnc( const char * );
    virtual void        OGLEntry( SalGraphics* pGraphics );
    virtual void        OGLExit( SalGraphics* pGraphics );
    virtual void        StartScene( SalGraphics* pGraphics );
    virtual void        StopScene();

    static void         Release();
};

#endif // _SV_SALOGL_H
