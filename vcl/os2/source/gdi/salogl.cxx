/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salogl.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-25 10:05:35 $
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

#include <malloc.h>
#define INCL_DOSMODULEMGR
#include <svpm.h>
#define _SV_SALOGL_CXX

#ifndef _SV_SALOGL_H
#include <salogl.h>
#endif
#ifndef _SV_SALGDI_H
#include <salgdi.h>
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
    nResult = DosQueryProcAddr( mhOGLLib, 0, (PSZ) "pgl" #FncName, (PFN*)&pFnc##FncName );  \
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
HMODULE     Os2SalOpenGL::mhOGLLib = 0;
HGC         Os2SalOpenGL::mhOGLContext = 0;
HDC         Os2SalOpenGL::mhOGLLastDC = 0;
ULONG       Os2SalOpenGL::mnOGLState = OGL_STATE_UNLOADED;

// Internal use
#define INIT_OGLFNC( a, FncName ) static OGLFnc##FncName pFnc##FncName = NULL;

INIT_OGLFNC( Os2SalOpenGL, ChooseConfig );
INIT_OGLFNC( Os2SalOpenGL, QueryConfigs );
INIT_OGLFNC( Os2SalOpenGL, CreateContext );
INIT_OGLFNC( Os2SalOpenGL, DestroyContext );
INIT_OGLFNC( Os2SalOpenGL, GetCurrentContext );
INIT_OGLFNC( Os2SalOpenGL, MakeCurrent );
INIT_OGLFNC( Os2SalOpenGL, QueryCapability );

// -------------
// - Os2SalOpenGL -
// -------------

Os2SalOpenGL::Os2SalOpenGL( SalGraphics* pGraphics )
{
    // Set mhOGLLastDC only the first time a
    // Os2SalOpenGL object is created; we need
    // this DC in Os2SalOpenGL::Create();
    if( OGL_STATE_UNLOADED == mnOGLState )
        mhOGLLastDC = static_cast<Os2SalGraphics*>(pGraphics)->mhDC;
}

// ------------------------------------------------------------------------

Os2SalOpenGL::~Os2SalOpenGL()
{
}

// ------------------------------------------------------------------------

bool Os2SalOpenGL::IsValid()
{

    if( OGL_STATE_UNLOADED == mnOGLState )
    {
        if( ImplInitLib() )
        {
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
                    }
                }
            }
            else
            {
                ImplFreeLib();
                mnOGLState = OGL_STATE_INVALID;
            }
        }
        else
            mnOGLState = OGL_STATE_INVALID;
    }

    return mnOGLState == OGL_STATE_VALID ? TRUE : FALSE;
}

// ------------------------------------------------------------------------

void Os2SalOpenGL::Release()
{
    ImplFreeLib();
}

// ------------------------------------------------------------------------

oglFunction Os2SalOpenGL::GetOGLFnc( const char* rFncName )
{
    void* pRet;

    if( mhOGLLib )
    {
        APIRET rc;
        PFN    pFunction;

        rc = DosQueryProcAddr( mhOGLLib, 0, (PSZ)rFncName, &pFunction );
        pRet = rc == NULL ? (void*)pFunction : NULL;
    }
    else
        pRet = NULL;

    return (oglFunction)pRet;
}

// ------------------------------------------------------------------------

void Os2SalOpenGL::OGLEntry( SalGraphics* pSGraphics )
{
    Os2SalGraphics* pGraphics = static_cast<Os2SalGraphics*>(pSGraphics);
    if( pGraphics->mhDC != mhOGLLastDC )
    {
        mhOGLLastDC = pGraphics->mhDC;
        pFncMakeCurrent( GetSalData()->mhAB, mhOGLContext, WinWindowFromDC( mhOGLLastDC ) );
    }
}

// ------------------------------------------------------------------------

void Os2SalOpenGL::OGLExit( SalGraphics* pGraphics )
{
}

// ------------------------------------------------------------------------

void Os2SalOpenGL::StartScene( SalGraphics* pGraphics )
{
}

// ------------------------------------------------------------------------

void Os2SalOpenGL::StopScene()
{
}

// ------------------------------------------------------------------------

BOOL Os2SalOpenGL::ImplInitLib()
{
    DosLoadModule ((PSZ)0, 0, (PSZ)OGL_LIBNAME, &mhOGLLib );
    return( mhOGLLib != NULL );
}

// ------------------------------------------------------------------------

void Os2SalOpenGL::ImplFreeLib()
{
    if( mhOGLLib )
        DosFreeModule( mhOGLLib );
}

// ------------------------------------------------------------------------

BOOL Os2SalOpenGL::ImplInit()
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
