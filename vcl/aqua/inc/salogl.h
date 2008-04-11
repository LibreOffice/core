/*************************************************************************
*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salogl.h,v $
 * $Revision: 1.5 $
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

#include "vcl/salogl.hxx"
#include "osl/module.h"

// -------------
// - SalOpenGL -
// -------------

class SalGraphics;

class AquaSalOpenGL : public SalOpenGL
{
private:
    static ULONG        mnOGLState;

    //  static oslModule        mpGLLib;
//  static GLXContext       maGLXContext;
//  static ULONG            mnOGLState;
//  static Display*     mpDisplay;
//  static XVisualInfo* mpVisualInfo;
//  static BOOL         mbHaveGLVisual;
//
//  static BOOL         ImplInit();
//
//  static GLXContext (*pCreateContext)( Display *, XVisualInfo *, GLXContext, Bool );
//  static void       (*pDestroyContext)( Display *, GLXContext );
//  static GLXContext (*pGetCurrentContext)( );
//  static Bool       (*pMakeCurrent)( Display *, GLXDrawable, GLXContext );
//  static void     (*pSwapBuffers)( Display*, GLXDrawable );
//  static int      (*pGetConfig)( Display*, XVisualInfo*, int, int* );
//  static void     (*pFlush)();
//
//  Drawable            maDrawable;
//
//  static void* resolveSymbol( const char* pSym );
public:

        AquaSalOpenGL( SalGraphics* pGraphics );
    ~AquaSalOpenGL();

//  static void     Release();
//    static void     ReleaseLib();
//  static void     MakeVisualWeights(
//                                      Display *pDisplay,
//                                      XVisualInfo* pInfos,
//                                      int* pWeights,
//                                      int nVisuals
//                                      );

    // overload all pure virtual methods
    virtual bool        IsValid();
    virtual oglFunction GetOGLFnc( const char * );
    virtual void        OGLEntry( SalGraphics* pGraphics );
    virtual void        OGLExit( SalGraphics* pGraphics );
    virtual void        StartScene( SalGraphics* pGraphics );
    virtual void        StopScene();
    virtual BOOL Create();
    virtual void Release();

};

#endif // _SV_SALOGL_H
