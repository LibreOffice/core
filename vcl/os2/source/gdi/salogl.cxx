/*************************************************************************
 *
 *  $RCSfile: salogl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:34 $
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

#include <malloc.h>
#define INCL_DOSMODULEMGR
#include <tools/svpm.h>
#define _SV_SALOGL_CXX
#ifndef _SV_SALOGL_HXX
#include <salogl.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif

// ------------
// - Typedefs -
// ------------

typedef VISUALCONFIG* PCFG;

// ------------
// - Lib-Name -
// ------------

#define OGL_LIBNAME "OPENGL"

// ----------
// - Macros -
// ----------

#define GET_OGLFNC_PGL( FncName )                                                       \
    nResult = DosQueryProcAddr( mhOGLLib, 0, "pgl" #FncName, (PFN*)&pFnc##FncName );    \
if( nResult != 0 )                                                                      \
    bRet = FALSE;

// ------------------------------------------------------------------------

#define GET_OGLFNC_GL( FncName )                                                        \
    nResult = DosQueryProcAddr( mhOGLLib, 0, "gl" #FncName, (PFN*)&pFnc##FncName );     \
if( nResult != 0 )                                                                      \
    bRet = FALSE;

// -----------------
// - Statics init. -
// -----------------

// Members
HMODULE     SalOpenGL::mhOGLLib = 0;
HGC         SalOpenGL::mhOGLContext = 0;
HDC         SalOpenGL::mhOGLLastDC = 0;
ULONG       SalOpenGL::mnOGLState = OGL_STATE_UNLOADED;

// Internal use
INIT_OGLFNC( SalOpenGL, ChooseConfig );
INIT_OGLFNC( SalOpenGL, QueryConfigs );
INIT_OGLFNC( SalOpenGL, CreateContext );
INIT_OGLFNC( SalOpenGL, DestroyContext );
INIT_OGLFNC( SalOpenGL, GetCurrentContext );
INIT_OGLFNC( SalOpenGL, MakeCurrent );
INIT_OGLFNC( SalOpenGL, QueryCapability );

// -------------
// - SalOpenGL -
// -------------

SalOpenGL::SalOpenGL( SalGraphics* pGraphics )
{
    // Set mhOGLLastDC only the first time a
    // SalOpenGL object is created; we need
    // this DC in SalOpenGL::Create();
    if( OGL_STATE_UNLOADED == mnOGLState )
        mhOGLLastDC = pGraphics->maGraphicsData.mhDC;
}

// ------------------------------------------------------------------------

SalOpenGL::~SalOpenGL()
{
}

// ------------------------------------------------------------------------

BOOL SalOpenGL::Create()
{
    BOOL bRet = FALSE;

    if( OGL_STATE_UNLOADED == mnOGLState )
    {
        if( ImplInitLib() )
        {
/*
            if( ImplInit() )
            {
                HAB             hAB = GetSalData()->mhAB;
                VISUALCONFIG    aCfg = { PGL_RGBA, PGL_RED_SIZE, 4, PGL_GREEN_SIZE, 4,PGL_BLUE_SIZE, 4, PGL_DOUBLEBUFFER, 0 };
                PCFG*           ppCfgs = pFncQueryConfigs( hAB );

                if( *ppCfgs )
                {
                    if( ( mhOGLContext = pFncCreateContext( hAB, *ppCfgs, NULL, FALSE ) ) != 0 )
                    {
                        pFncMakeCurrent( hAB, mhOGLContext, WinWindowFromDC( mhOGLLastDC ) );
                        mnOGLState = OGL_STATE_VALID;
                        bRet = TRUE;
                    }
                }
            }
*/

            if( !bRet )
            {
                ImplFreeLib();
                mnOGLState = OGL_STATE_INVALID;
            }
        }
        else
            mnOGLState = OGL_STATE_INVALID;
    }
    else if( OGL_STATE_VALID == mnOGLState )
        bRet = TRUE;

    return bRet;
}

// ------------------------------------------------------------------------

void SalOpenGL::Release()
{
    ImplFreeLib();
}

// ------------------------------------------------------------------------

void* SalOpenGL::GetOGLFnc( const String& rFncName )
{
    void* pRet;

    if( mhOGLLib )
    {
        APIRET rc;
        PFN    pFunction;

        rc = DosQueryProcAddr( mhOGLLib, 0, rFncName, &pFunction );
        pRet = rc == NULL ? pFunction : NULL;
    }
    else
        pRet = NULL;

    return pRet;
}

// ------------------------------------------------------------------------

void SalOpenGL::OGLEntry( SalGraphics* pGraphics )
{
    if( pGraphics->maGraphicsData.mhDC != mhOGLLastDC )
    {
        mhOGLLastDC = pGraphics->maGraphicsData.mhDC;
        pFncMakeCurrent( GetSalData()->mhAB, mhOGLContext, WinWindowFromDC( mhOGLLastDC ) );
    }
}

// ------------------------------------------------------------------------

void SalOpenGL::OGLExit( SalGraphics* pGraphics )
{
}

// ------------------------------------------------------------------------

BOOL SalOpenGL::ImplInitLib()
{
    DosLoadModule ((PSZ)0, 0, OGL_LIBNAME, &mhOGLLib );
    return( mhOGLLib != NULL );
}

// ------------------------------------------------------------------------

void SalOpenGL::ImplFreeLib()
{
    if( mhOGLLib )
        DosFreeModule( mhOGLLib );
}

// ------------------------------------------------------------------------

BOOL SalOpenGL::ImplInit()
{
    ULONG   nResult;
    BOOL    bRet = TRUE;

    // Internal use
    GET_OGLFNC_PGL( ChooseConfig );
    GET_OGLFNC_PGL( QueryConfigs );
    GET_OGLFNC_PGL( CreateContext );
    GET_OGLFNC_PGL( DestroyContext );
    GET_OGLFNC_PGL( GetCurrentContext );
    GET_OGLFNC_PGL( MakeCurrent );
    GET_OGLFNC_PGL( QueryCapability );

    return bRet;
}
