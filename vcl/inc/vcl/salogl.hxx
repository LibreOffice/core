/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salogl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 18:07:55 $
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

#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif

#define _SVUNX_H

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif
#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _SV_SALOTYPE_HXX
#include <vcl/salotype.hxx>
#endif

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
