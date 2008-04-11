/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salogl.h,v $
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

#ifndef _SV_SALOGL_H
#define _SV_SALOGL_H

#if defined EXPLICITLY_INCLUDE_GLX_H
#include <prex.h>
#define GL_H
#include <GL/glx.h>
#include <postx.h>
#else
typedef void* GLXContext;
typedef XID   GLXDrawable;
// needed GLX_ constants
#define GLX_USE_GL          1
#define GLX_DOUBLEBUFFER    5
#endif
#include <vcl/salogl.hxx>

#ifndef _OSL_MODULE_H
#include <osl/module.h>
#endif

// -------------
// - SalOpenGL -
// -------------

class SalGraphics;

class X11SalOpenGL : public SalOpenGL
{
private:
    static oslModule            mpGLLib;
    static GLXContext           maGLXContext;
    static ULONG                mnOGLState;
    static Display*             mpDisplay;
    static const XVisualInfo*   mpVisualInfo;
    static BOOL                 mbHaveGLVisual;

    static BOOL         ImplInit();

    static GLXContext (*pCreateContext)( Display *, XVisualInfo *, GLXContext, Bool );
    static void       (*pDestroyContext)( Display *, GLXContext );
    static GLXContext (*pGetCurrentContext)( );
    static Bool       (*pMakeCurrent)( Display *, GLXDrawable, GLXContext );
    static void     (*pSwapBuffers)( Display*, GLXDrawable );
    static int      (*pGetConfig)( Display*, XVisualInfo*, int, int* );
    static void     (*pFlush)();

    Drawable            maDrawable;

    static oglFunction resolveSymbol( const char* pSym );
public:

    X11SalOpenGL( SalGraphics* pGraphics );
    ~X11SalOpenGL();

    static void     Release();
    static void     ReleaseLib();
    static void     MakeVisualWeights(
                                      Display *pDisplay,
                                      XVisualInfo* pInfos,
                                      int* pWeights,
                                      int nVisuals
                                      );

    // overload all pure virtual methods
    virtual bool        IsValid();
    virtual oglFunction GetOGLFnc( const char * );
    virtual void        OGLEntry( SalGraphics* pGraphics );
    virtual void        OGLExit( SalGraphics* pGraphics );
    virtual void        StartScene( SalGraphics* pGraphics );
    virtual void        StopScene();
};

#endif // _SV_SALOGL_H
