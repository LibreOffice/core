/*************************************************************************
 *
 *  $RCSfile: salogl.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 14:38:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

#ifndef _SV_SALOGL_HXX
#include <salogl.hxx>
#endif

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
    static oslModule        mpGLLib;
    static GLXContext       maGLXContext;
    static ULONG            mnOGLState;
    static Display*     mpDisplay;
    static XVisualInfo* mpVisualInfo;
    static BOOL         mbHaveGLVisual;

    static BOOL         ImplInit();
    static void         ImplFreeLib();

    static GLXContext (*pCreateContext)( Display *, XVisualInfo *, GLXContext, Bool );
    static void       (*pDestroyContext)( Display *, GLXContext );
    static GLXContext (*pGetCurrentContext)( );
    static Bool       (*pMakeCurrent)( Display *, GLXDrawable, GLXContext );
    static void     (*pSwapBuffers)( Display*, GLXDrawable );
    static int      (*pGetConfig)( Display*, XVisualInfo*, int, int* );
    static void     (*pFlush)();

    Drawable            maDrawable;

    static void* resolveSymbol( const char* pSym );
public:

    X11SalOpenGL( SalGraphics* pGraphics );
    ~X11SalOpenGL();

    static void     Release();
    static void     MakeVisualWeights(
                                      Display *pDisplay,
                                      XVisualInfo* pInfos,
                                      int* pWeights,
                                      int nVisuals
                                      );

    // overload all pure virtual methods
    virtual bool        IsValid();
    virtual void*       GetOGLFnc( const char * );
    virtual void        OGLEntry( SalGraphics* pGraphics );
    virtual void        OGLExit( SalGraphics* pGraphics );
    virtual void        StartScene( SalGraphics* pGraphics );
    virtual void        StopScene();
};

#endif // _SV_SALOGL_H
