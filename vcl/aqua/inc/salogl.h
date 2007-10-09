/*************************************************************************
*
*  OpenOffice.org - a multi-platform office productivity suite
*
*  $RCSfile: salogl.h,v $
*
*  $Revision: 1.4 $
*
*  last change: $Author: kz $ $Date: 2007-10-09 15:10:29 $
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
