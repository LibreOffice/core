/*************************************************************************
 *
 *  $RCSfile: salogl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pluby $ $Date: 2000-11-01 03:12:45 $
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

#ifndef _SVWIN_H
#include <tools/svwin.h>
#endif

#define _SV_SALOGL_CXX

#ifndef _SV_SALOGL_HXX
#include <salogl.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif

// -------------------------------
// - Additional typedefs for init.
// -------------------------------

typedef HGLRC   ( *OGLFncCreateContext )( VCLVIEW hDC );
typedef BOOL    ( *OGLFncDeleteContext )( HGLRC hContext );
typedef HGLRC   ( *OGLFncGetCurrentContext )( void );
typedef void    ( *OGLFncMakeCurrent )( VCLVIEW hDC, HGLRC hContext  );

// ------------
// - Lib-Name -
// ------------

#ifdef WIN
#define OGL_LIBNAME "OPENGL32.DLL"
#endif

// ----------
// - Macros -
// ----------

#ifdef WIN
#define INIT_OGLFNC_WGL( FncName ) static OGLFnc##FncName pImplOpenWGLFnc##FncName = NULL;
#define GET_OGLFNC_WGL( FncName ) \
pImplOpenWGLFnc##FncName = (OGLFnc##FncName##) GetProcAddress( hImplOGLLib, "wgl" #FncName ); \
if( !pImplOpenWGLFnc##FncName ) bRet = FALSE;
#endif

// -----------------
// - Statics init. -
// -----------------

// Members
HGLRC               SalOpenGL::mhOGLContext = 0;
VCLVIEW             SalOpenGL::mhOGLLastDC = 0;
ULONG               SalOpenGL::mnOGLState = OGL_STATE_UNLOADED;

#ifdef WIN
INIT_OGLFNC_WGL( CreateContext );
INIT_OGLFNC_WGL( DeleteContext );
INIT_OGLFNC_WGL( GetCurrentContext );
INIT_OGLFNC_WGL( MakeCurrent );
#endif

// -----------
// - WndProc -
// -----------

#ifdef WIN
LRESULT CALLBACK OpenGLWndProc( VCLWINDOW hWnd,UINT nMsg, WPARAM nPar1, LPARAM nPar2 )
{
    return DefWindowProc( hWnd, nMsg, nPar1, nPar2 );
}
#endif

// -------------
// - SalOpenGL -
// -------------

SalOpenGL::SalOpenGL( SalGraphics* pGraphics )
{
    // Set mhOGLLastDC only the first time a
    // SalOpenGL object is created; we need
    // this DC in SalOpenGL::Create();
    if ( OGL_STATE_UNLOADED == mnOGLState )
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

    if ( OGL_STATE_UNLOADED == mnOGLState )
    {
        if( ImplInitLib() )
        {
#ifdef WIN
            USHORT nBitCount = GetDeviceCaps( mhOGLLastDC, BITSPIXEL );
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
                    VCLWINDOW       hDummyWnd;

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
                    hDummyWnd = CreateWindow( aWc.lpszClassName, NULL, WS_OVERLAPPED, 0, -50, 1, 1, VCLWINDOW_DESKTOP, NULL, aWc.hInstance, 0 );
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
#endif
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

void* SalOpenGL::GetOGLFnc( const char* pFncName )
{
#ifdef WIN
    if ( hImplOGLLib )
        return (void*)GetProcAddress( hImplOGLLib, pFncName );
    else
#endif
        return NULL;
}

// ------------------------------------------------------------------------

#ifdef WIN
typedef BOOL (WINAPI *MyFuncType)(VCLVIEW, HGLRC);
#endif

void SalOpenGL::OGLEntry( SalGraphics* pGraphics )
{
    if ( pGraphics->maGraphicsData.mhDC != mhOGLLastDC )
    {
#ifdef WIN
        PIXELFORMATDESCRIPTOR pfd =
        {
            sizeof( PIXELFORMATDESCRIPTOR ),
            1,
            PFD_DRAW_TO_WINDOW | PFD_SUPPORT_GDI | PFD_SUPPORT_OPENGL,
            PFD_TYPE_RGBA,
            GetDeviceCaps( pGraphics->maGraphicsData.mhDC, BITSPIXEL ),
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

        const int nIndex = ChoosePixelFormat( pGraphics->maGraphicsData.mhDC, &pfd );
        if ( nIndex && SetPixelFormat( pGraphics->maGraphicsData.mhDC, nIndex, &pfd ) )
        {
            WNDCLASS    aWc;
            VCLWINDOW       hDummyWnd;

            pImplOpenWGLFncDeleteContext( mhOGLContext );
            mhOGLLastDC = pGraphics->maGraphicsData.mhDC;
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
            hDummyWnd = CreateWindow( aWc.lpszClassName, NULL, WS_OVERLAPPED, 0, -50, 1, 1, VCLWINDOW_DESKTOP, NULL, aWc.hInstance, 0 );
            ShowWindow( hDummyWnd, SW_SHOW );
            DestroyWindow( hDummyWnd );
            UnregisterClass( aWc.lpszClassName, aWc.hInstance );
        }
#endif
    }
}

// ------------------------------------------------------------------------

void SalOpenGL::OGLExit( SalGraphics* pGraphics )
{
}

// ------------------------------------------------------------------------

BOOL SalOpenGL::ImplInitLib()
{
#ifdef WIN
    return ((hImplOGLLib = LoadLibrary( OGL_LIBNAME )) != NULL);
#endif
}

// ------------------------------------------------------------------------

void SalOpenGL::ImplFreeLib()
{
#ifdef WIN
    if ( hImplOGLLib )
    {
        FreeLibrary( hImplOGLLib );
        hImplOGLLib = NULL;
        mnOGLState = OGL_STATE_UNLOADED;
    }
#endif
}

// ------------------------------------------------------------------------

BOOL SalOpenGL::ImplInit()
{
    BOOL bRet = TRUE;

#ifdef WIN
    // Internal use
    GET_OGLFNC_WGL( CreateContext );
    GET_OGLFNC_WGL( DeleteContext );
    GET_OGLFNC_WGL( GetCurrentContext );
    GET_OGLFNC_WGL( MakeCurrent );
#endif

    return bRet;
}
