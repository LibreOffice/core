/*************************************************************************
 *
 *  $RCSfile: salogl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:49 $
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
