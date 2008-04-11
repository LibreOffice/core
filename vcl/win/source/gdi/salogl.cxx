/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salogl.cxx,v $
 * $Revision: 1.10 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include <tools/svwin.h>
#include <rtl/ustring.h>
#include <osl/module.h>
#include <salogl.h>
#include <salgdi.h>

using namespace rtl;

#ifdef WNT
#define __OPENGL_CALL __stdcall
#else
#define __OPENGL_CALL
#endif

// -------------------------------
// - Additional typedefs for init.
// -------------------------------

typedef HGLRC   ( __OPENGL_CALL *OGLFncCreateContext )( HDC hDC );
typedef BOOL    ( __OPENGL_CALL *OGLFncDeleteContext )( HGLRC hContext );
typedef HGLRC   ( __OPENGL_CALL *OGLFncGetCurrentContext )( VOID );
typedef void    ( __OPENGL_CALL *OGLFncMakeCurrent )( HDC hDC, HGLRC hContext  );

// ------------
// - Lib-Name -
// ------------

#define OGL_LIBNAME "OPENGL32.DLL"

// ----------
// - Macros -
// ----------

#define INIT_OGLFNC_WGL( FncName ) static OGLFnc##FncName pImplOpenWGLFnc##FncName = NULL;
#define GET_OGLFNC_WGL( FncName )                                                                 \
OUString queryFuncName##FncName( RTL_CONSTASCII_USTRINGPARAM( "wgl" #FncName ) );                 \
pImplOpenWGLFnc##FncName = (OGLFnc##FncName) osl_getSymbol( hImplOGLLib, queryFuncName##FncName.pData ); \
if( !pImplOpenWGLFnc##FncName ) bRet = FALSE;

// -----------------
// - Statics init. -
// -----------------

// Members
static oslModule    hImplOGLLib = NULL;
HGLRC               WinSalOpenGL::mhOGLContext = 0;
HDC                 WinSalOpenGL::mhOGLLastDC = 0;
ULONG               WinSalOpenGL::mnOGLState = OGL_STATE_UNLOADED;

INIT_OGLFNC_WGL( CreateContext );
INIT_OGLFNC_WGL( DeleteContext );
INIT_OGLFNC_WGL( GetCurrentContext );
INIT_OGLFNC_WGL( MakeCurrent );

// -----------
// - WndProc -
// -----------

LRESULT CALLBACK OpenGLWndProc( HWND hWnd,UINT nMsg, WPARAM nPar1, LPARAM nPar2 )
{
    return DefWindowProc( hWnd, nMsg, nPar1, nPar2 );
}

// -------------
// - SalOpenGL -
// -------------

WinSalOpenGL::WinSalOpenGL( SalGraphics* pGraphics )
{
    // Set mhOGLLastDC only the first time a
    // SalOpenGL object is created; we need
    // this DC in SalOpenGL::Create();
    if ( OGL_STATE_UNLOADED == mnOGLState )
        mhOGLLastDC = static_cast<WinSalGraphics*>(pGraphics)->mhDC;
}

// ------------------------------------------------------------------------

WinSalOpenGL::~WinSalOpenGL()
{
}

// ------------------------------------------------------------------------

bool WinSalOpenGL::IsValid()
{
    bool bRet = false;

    if ( OGL_STATE_UNLOADED == mnOGLState )
    {
        if( ImplInitLib() )
        {
            int nBitCount = GetDeviceCaps( mhOGLLastDC, BITSPIXEL );
            PIXELFORMATDESCRIPTOR pfd =
            {
                sizeof( PIXELFORMATDESCRIPTOR ),
                1,
                PFD_DRAW_TO_WINDOW | PFD_SUPPORT_GDI | PFD_SUPPORT_OPENGL,
                PFD_TYPE_RGBA,
                (BYTE) nBitCount,
                0, 0, 0, 0, 0, 0,
                0,
                0,
                0,
                0, 0, 0, 0,
                16,
                0,
                0,
                PFD_MAIN_PLANE,
                0,
                0, 0, 0
            };

            const int nIndex = ChoosePixelFormat( mhOGLLastDC, &pfd );

            if( nIndex && SetPixelFormat( mhOGLLastDC, nIndex, &pfd ) )
            {
                if ( (nBitCount > 8) && ImplInit() &&
                     (mhOGLContext = pImplOpenWGLFncCreateContext( mhOGLLastDC )) != 0 )
                {
                    WNDCLASS    aWc;
                    HWND        hDummyWnd;

                    SaveDC( mhOGLLastDC );
                    SelectClipRgn( mhOGLLastDC, NULL );
                    pImplOpenWGLFncMakeCurrent( mhOGLLastDC, mhOGLContext );
                    RestoreDC( mhOGLLastDC, -1 );
                    mnOGLState = OGL_STATE_VALID;
                    bRet = TRUE;

                    memset( &aWc, 0, sizeof( aWc ) );
                    aWc.hInstance = GetModuleHandle( NULL );
                    aWc.lpfnWndProc = OpenGLWndProc;
                    aWc.lpszClassName = "OpenGLWnd";
                    RegisterClass( &aWc );
                    hDummyWnd = CreateWindow( aWc.lpszClassName, NULL, WS_OVERLAPPED, 0, -50, 1, 1, HWND_DESKTOP, NULL, aWc.hInstance, 0 );
                    ShowWindow( hDummyWnd, SW_SHOW );
                    DestroyWindow( hDummyWnd );
                    UnregisterClass( aWc.lpszClassName, aWc.hInstance );
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
        else
            mnOGLState = OGL_STATE_INVALID;
    }
    else if( OGL_STATE_VALID == mnOGLState )
        bRet = true;

    return bRet;
}

// ------------------------------------------------------------------------

void WinSalOpenGL::Release()
{
    ImplFreeLib();
}

// ------------------------------------------------------------------------

oglFunction WinSalOpenGL::GetOGLFnc( const char* pFncName )
{
    if ( hImplOGLLib )
    {
        OUString queryFuncName = OUString::createFromAscii( pFncName );
        return (oglFunction)osl_getSymbol( hImplOGLLib, queryFuncName.pData );
    }
    else
        return NULL;
}

// ------------------------------------------------------------------------

typedef BOOL (WINAPI *MyFuncType)(HDC, HGLRC);

void WinSalOpenGL::OGLEntry( SalGraphics* pSGraphics )
{
    WinSalGraphics* pGraphics = static_cast<WinSalGraphics*>(pSGraphics);

    if ( pGraphics->mhDC != mhOGLLastDC )
    {
        PIXELFORMATDESCRIPTOR pfd =
        {
            sizeof( PIXELFORMATDESCRIPTOR ),
            1,
            PFD_DRAW_TO_WINDOW | PFD_SUPPORT_GDI | PFD_SUPPORT_OPENGL,
            PFD_TYPE_RGBA,
            (BYTE)GetDeviceCaps( pGraphics->mhDC, BITSPIXEL ),
            0, 0, 0, 0, 0, 0,
            0,
            0,
            0,
            0, 0, 0, 0,
            16,
            0,
            0,
            PFD_MAIN_PLANE,
            0,
            0, 0, 0
        };

        const int nIndex = ChoosePixelFormat( pGraphics->mhDC, &pfd );
        if ( nIndex && SetPixelFormat( pGraphics->mhDC, nIndex, &pfd ) )
        {
            WNDCLASS    aWc;
            HWND        hDummyWnd;

            pImplOpenWGLFncDeleteContext( mhOGLContext );
            mhOGLLastDC = pGraphics->mhDC;
            mhOGLContext = pImplOpenWGLFncCreateContext( mhOGLLastDC );

            SaveDC( mhOGLLastDC );
            SelectClipRgn( mhOGLLastDC, NULL );
            pImplOpenWGLFncMakeCurrent( mhOGLLastDC, mhOGLContext );
            RestoreDC( mhOGLLastDC, -1 );

            memset( &aWc, 0, sizeof( aWc ) );
            aWc.hInstance = GetModuleHandle( NULL );
            aWc.lpfnWndProc = OpenGLWndProc;
            aWc.lpszClassName = "OpenGLWnd";
            RegisterClass( &aWc );
            hDummyWnd = CreateWindow( aWc.lpszClassName, NULL, WS_OVERLAPPED, 0, -50, 1, 1, HWND_DESKTOP, NULL, aWc.hInstance, 0 );
            ShowWindow( hDummyWnd, SW_SHOW );
            DestroyWindow( hDummyWnd );
            UnregisterClass( aWc.lpszClassName, aWc.hInstance );
        }
    }
}

// ------------------------------------------------------------------------

void WinSalOpenGL::OGLExit( SalGraphics* )
{
}

// ------------------------------------------------------------------------

void WinSalOpenGL::StartScene( SalGraphics* )
{
}

// ------------------------------------------------------------------------

void WinSalOpenGL::StopScene()
{
}

// ------------------------------------------------------------------------

BOOL WinSalOpenGL::ImplInitLib()
{
    OUString aLibraryName( RTL_CONSTASCII_USTRINGPARAM( OGL_LIBNAME ) );
    hImplOGLLib = osl_loadModule( aLibraryName.pData, SAL_LOADMODULE_DEFAULT );
    return ( hImplOGLLib != NULL );
}

// ------------------------------------------------------------------------

void WinSalOpenGL::ImplFreeLib()
{
    if ( hImplOGLLib )
    {
        osl_unloadModule( hImplOGLLib );
        hImplOGLLib = NULL;
        mnOGLState = OGL_STATE_UNLOADED;
    }
}

// ------------------------------------------------------------------------

BOOL WinSalOpenGL::ImplInit()
{
    BOOL bRet = TRUE;

    // Internal use
    if ( hImplOGLLib )
    {
        GET_OGLFNC_WGL( CreateContext );
        GET_OGLFNC_WGL( DeleteContext );
        GET_OGLFNC_WGL( GetCurrentContext );
        GET_OGLFNC_WGL( MakeCurrent );
    }

    return bRet;
}
