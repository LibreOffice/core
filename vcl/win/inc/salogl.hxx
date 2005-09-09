/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salogl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 13:59:01 $
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

#ifndef _SV_SALOGL_HXX
#define _SV_SALOGL_HXX

#define _OPENGL_EXT

#ifndef _SV_WINCOMP_HXX
#include <wincomp.hxx>
#endif
#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _SV_SV_H
#include <sv.h>
#endif
#ifndef _SV_SALOTYPE_HXX
#include <salotype.hxx>
#endif

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

class SalOpenGL
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
                        SalOpenGL( SalGraphics* pGraphics );
                        ~SalOpenGL();

    static BOOL         Create();
    static void         Release();
    static ULONG        GetState() { return SalOpenGL::mnOGLState; }
    static BOOL         IsValid()  { return( OGL_STATE_VALID == SalOpenGL::mnOGLState ); }

    static void*        GetOGLFnc( const char* pFncName );

    static void         OGLEntry( SalGraphics* pGraphics );
    static void         OGLExit( SalGraphics* pGraphics );
};

#endif // _SV_SALOGL_HXX
