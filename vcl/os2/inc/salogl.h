/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salogl.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 12:45:59 $
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

#define _OPENGL_EXT

#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif
//#ifndef _SV_SALOTYPE_HXX
//#include <salotype.hxx>
//#endif

#ifndef _SV_SALOGL_HXX
#include <vcl/salogl.hxx>
#endif

// ------------------
// - OpenGL defines -
// ------------------

#define  PGL_USE_GL             1
#define  PGL_BUFFER_SIZE        2
#define  PGL_LEVEL              3
#define  PGL_RGBA               4
#define  PGL_DOUBLEBUFFER       5
#define  PGL_STEREO             6
#define  PGL_AUX_BUFFERS        7
#define  PGL_RED_SIZE           8
#define  PGL_GREEN_SIZE         9
#define  PGL_BLUE_SIZE          10
#define  PGL_ALPHA_SIZE         11
#define  PGL_DEPTH_SIZE         12
#define  PGL_STENCIL_SIZE       13
#define  PGL_ACCUM_RED_SIZE     14
#define  PGL_ACCUM_GREEN_SIZE   15
#define  PGL_ACCUM_BLUE_SIZE    16
#define  PGL_ACCUM_ALPHA_SIZE   17
#define  PGL_SINGLEBUFFER       18

// -----------------
// - State defines -
// -----------------

#define OGL_STATE_UNLOADED      (0x00000000)
#define OGL_STATE_INVALID       (0x00000001)
#define OGL_STATE_VALID         (0x00000002)

// ----------
// - Macros -
// ----------

#define DECL_OGLFNC( FncName ) static OGLFnc##FncName pFnc##FncName

// -------------------------------
// - Additional typedefs for init.
// -------------------------------

typedef struct visualconfig
{
    unsigned long  vid;  /*Visual ID*/
    BOOL   rgba;
    int redSize, greenSize, blueSize, alphaSize;
    ULONG redMask, greenMask, blueMask;
    int accumRedSize, accumGreenSize, accumBlueSize, accumAlphaSize;
    BOOL doubleBuffer;
    BOOL  stereo;
    int bufferSize;
    int depthSize;
    int stencilSize;
    int auxBuffers;
    int level;
    PVOID reserved;
    struct visualconfig *next;
}
VISUALCONFIG, *PVISUALCONFIG;

extern "C"
{
    typedef LHANDLE         HGC;
    typedef PVISUALCONFIG*  ( *APIENTRY OGLFncQueryConfigs )( HAB hab, ... );
    typedef PVISUALCONFIG   ( *APIENTRY OGLFncChooseConfig )( HAB hab, int *attriblist, ... );
    typedef HGC             ( *APIENTRY OGLFncCreateContext )( HAB hab, PVISUALCONFIG pVisualConfig, HGC Sharelist, BOOL IsDirect, ... );
    typedef BOOL            ( *APIENTRY OGLFncDestroyContext )( HAB hab, HGC hgc, ... );
    typedef HGC             ( *APIENTRY OGLFncGetCurrentContext )( HAB hab, ... );
    typedef BOOL            ( *APIENTRY OGLFncMakeCurrent )( HAB hab, HGC hgc, HWND hwnd, ... );
    typedef LONG            ( *APIENTRY OGLFncQueryCapability)( HAB hab, ... );
}

// -------------
// - SalOpenGL -
// -------------

class SalGraphics;
class String;

//class SalOpenGL
class Os2SalOpenGL : public SalOpenGL
{
private:

    static HMODULE      mhOGLLib;
    static HGC          mhOGLContext;
    static HDC          mhOGLLastDC;
    static ULONG        mnOGLState;

#if 0
private:

    DECL_OGLFNC( QueryConfigs );
    DECL_OGLFNC( ChooseConfig );
    DECL_OGLFNC( CreateContext );
    DECL_OGLFNC( DestroyContext );
    DECL_OGLFNC( GetCurrentContext );
    DECL_OGLFNC( MakeCurrent );
    DECL_OGLFNC( QueryCapability );
#endif

private:

    static BOOL         ImplInitLib();
    static BOOL         ImplInit();
    static void         ImplFreeLib();

public:

    Os2SalOpenGL( SalGraphics* );
    virtual ~Os2SalOpenGL();

    virtual bool        IsValid();
    virtual oglFunction GetOGLFnc( const char * );
    virtual void        OGLEntry( SalGraphics* pGraphics );
    virtual void        OGLExit( SalGraphics* pGraphics );
    virtual void        StartScene( SalGraphics* pGraphics );
    virtual void        StopScene();

    static void         Release();

};

#endif // _SV_SALOGL_H
