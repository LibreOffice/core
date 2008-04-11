/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salogl.cxx,v $
 * $Revision: 1.22 $
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

#include <salunx.h>
#include <saldata.hxx>
#include <saldisp.hxx>
#include <salogl.h>
#include <salgdi.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

using namespace rtl;

// ------------
// - Lib-Name -
// ------------

#ifdef MACOSX
#define OGL_LIBNAME "libGL.dylib"
#else
#define OGL_LIBNAME "libGL.so.1"
#endif

// ----------
// - Macros -
// ----------

// -----------------
// - Statics init. -
// -----------------

// Members
GLXContext          X11SalOpenGL::maGLXContext = 0;
Display*            X11SalOpenGL::mpDisplay    = 0;
const XVisualInfo*  X11SalOpenGL::mpVisualInfo = 0;
BOOL                X11SalOpenGL::mbHaveGLVisual = FALSE;

oslModule           X11SalOpenGL::mpGLLib    = 0;
#ifdef SOLARIS
oslModule      aMotifLib;
#endif

ULONG       X11SalOpenGL::mnOGLState = OGL_STATE_UNLOADED;

GLXContext (*X11SalOpenGL::pCreateContext)( Display *, XVisualInfo *, GLXContext, Bool ) = 0;
void       (*X11SalOpenGL::pDestroyContext)( Display *, GLXContext ) = 0;
GLXContext (*X11SalOpenGL::pGetCurrentContext)( ) = 0;
Bool       (*X11SalOpenGL::pMakeCurrent)( Display *, GLXDrawable, GLXContext ) = 0;
void        (*X11SalOpenGL::pSwapBuffers)( Display*, GLXDrawable ) = 0;
int         (*X11SalOpenGL::pGetConfig)( Display*, XVisualInfo*, int, int* ) = 0;
void       (*X11SalOpenGL::pFlush)() = 0;

// -------------
// - X11SalOpenGL -
// -------------
// FIXME: Multiscreen
X11SalOpenGL::X11SalOpenGL( SalGraphics* pSGraphics )
{
    X11SalGraphics* pGraphics = static_cast<X11SalGraphics*>(pSGraphics);
    mpDisplay    = pGraphics->GetXDisplay();
    mpVisualInfo = &pGraphics->GetDisplay()->GetVisual(pGraphics->GetScreenNumber());
    maDrawable   = pGraphics->GetDrawable();
}

// ------------------------------------------------------------------------

X11SalOpenGL::~X11SalOpenGL()
{
}

// ------------------------------------------------------------------------

bool X11SalOpenGL::IsValid()
{
    if( OGL_STATE_UNLOADED == mnOGLState )
    {
        BOOL bHasGLX = FALSE;
        char **ppExtensions;
        int nExtensions;

        if( *DisplayString( mpDisplay ) == ':'                          ||
            ! strncmp( DisplayString( mpDisplay ), "localhost:", 10 )
            )
        {
            // GLX only on local displays due to strange problems
            // with remote GLX
            ppExtensions = XListExtensions( mpDisplay, &nExtensions );
            for( int i=0; i < nExtensions; i++ )
            {
                if( ! strncmp( "GLX", ppExtensions[ i ], 3 ) )
                {
                    bHasGLX = TRUE;
                    break;
                }
            }
            XFreeExtensionList( ppExtensions );
#if OSL_DEBUG_LEVEL > 1
            if( ! bHasGLX )
                fprintf( stderr, "XServer does not support GLX extension\n" );
#endif
            if( bHasGLX )
            {
                /*
                 *  #82406# the XFree4.0 GLX module does not seem
                 *  to work that great, at least not the one that comes
                 *  with the default installation and Matrox cards.
                 *  Since these are common we disable usage of
                 *  OpenGL per default.
                 */
                static const char* pOverrideGLX = getenv( "SAL_ENABLE_GLX_XFREE4" );
                if( ! strncmp( ServerVendor( mpDisplay ), "The XFree86 Project, Inc", 24 ) &&
                    VendorRelease( mpDisplay ) >= 4000 &&
                    ! pOverrideGLX
                    )
                {
#if OSL_DEBUG_LEVEL > 1
                    fprintf( stderr, "disabling GLX usage on XFree >= 4.0\n" );
#endif
                    bHasGLX = FALSE;
                }
            }
        }
        if( bHasGLX && mpVisualInfo->c_class == TrueColor && ImplInit() )
        {
            int nDoubleBuffer = 0;
            int nHaveGL = 0;
            pGetConfig( mpDisplay, const_cast<XVisualInfo*>(mpVisualInfo),
                        GLX_USE_GL, &nHaveGL );
            pGetConfig( mpDisplay, const_cast<XVisualInfo*>(mpVisualInfo),
                        GLX_DOUBLEBUFFER, &nDoubleBuffer );
            if( nHaveGL && ! nDoubleBuffer )
            {
                SalDisplay* pSalDisplay = GetX11SalData()->GetDisplay();
                pSalDisplay->GetXLib()->PushXErrorLevel( true );
                mbHaveGLVisual = TRUE;

                maGLXContext = pCreateContext( mpDisplay, const_cast<XVisualInfo*>(mpVisualInfo), 0, True );
                if( ! pSalDisplay->GetXLib()->HasXErrorOccured() )
                    pMakeCurrent( mpDisplay, maDrawable, maGLXContext );
                if( pSalDisplay->GetXLib()->HasXErrorOccured() )
                    mbHaveGLVisual = FALSE;
                pSalDisplay->GetXLib()->PopXErrorLevel();

                if( mbHaveGLVisual )
                    mnOGLState = OGL_STATE_VALID;
                else
                    maGLXContext = None;
            }
        }
        if( mnOGLState != OGL_STATE_VALID )
            mnOGLState = OGL_STATE_INVALID;
#if OSL_DEBUG_LEVEL > 1
        if( mnOGLState == OGL_STATE_VALID )
            fprintf( stderr, "Using GLX on visual id %lx.\n", mpVisualInfo->visualid );
        else
            fprintf( stderr, "Not using GLX.\n" );
#endif
    }

    return mnOGLState == OGL_STATE_VALID ? TRUE : FALSE;
}

void X11SalOpenGL::Release()
{
    if( maGLXContext && pDestroyContext )
        pDestroyContext( mpDisplay, maGLXContext );
}

// ------------------------------------------------------------------------

void X11SalOpenGL::ReleaseLib()
{
    if( mpGLLib )
    {
        osl_unloadModule( mpGLLib );
        #ifdef SOLARIS
        if( aMotifLib )
            osl_unloadModule( aMotifLib );
        #endif

        mpGLLib             = 0;
        pCreateContext      = 0;
        pDestroyContext     = 0;
        pGetCurrentContext  = 0;
        pMakeCurrent        = 0;
        pSwapBuffers        = 0;
        pGetConfig          = 0;

        mnOGLState          = OGL_STATE_UNLOADED;
    }
}

// ------------------------------------------------------------------------

oglFunction X11SalOpenGL::GetOGLFnc( const char *pFncName )
{
    return resolveSymbol( pFncName );
}

// ------------------------------------------------------------------------

void X11SalOpenGL::OGLEntry( SalGraphics* pGraphics )
{
    GLXDrawable aDrawable = static_cast<X11SalGraphics*>(pGraphics)->GetDrawable();
    if( aDrawable != maDrawable )
    {
        maDrawable = aDrawable;
        pMakeCurrent( mpDisplay, maDrawable, maGLXContext );
    }
}

// ------------------------------------------------------------------------

void X11SalOpenGL::OGLExit( SalGraphics* )
{
}

// ------------------------------------------------------------------------

oglFunction X11SalOpenGL::resolveSymbol( const char* pSymbol )
{
    oglFunction pSym = NULL;
    if( mpGLLib )
    {
        OUString aSym = OUString::createFromAscii( pSymbol );
        pSym = osl_getFunctionSymbol( mpGLLib, aSym.pData );
    }
    return pSym;
}


BOOL X11SalOpenGL::ImplInit()
{
    if( ! mpGLLib )
    {
        ByteString sNoGL( getenv( "SAL_NOOPENGL" ) );
        if( sNoGL.ToLowerAscii() == "true"  )
            return FALSE;

        sal_Int32 nRtldMode = SAL_LOADMODULE_NOW;
        #ifdef SOLARIS
        /* #i36866# an obscure interaction with jvm can let java crash
        *  if we do not use SAL_LOADMODULE_GLOBAL here
        */
        nRtldMode |= SAL_LOADMODULE_GLOBAL;

        /* #i36899# and we need Xm, too, else jvm will not work properly.
        */
        OUString aMotifName( RTL_CONSTASCII_USTRINGPARAM( "libXm.so" ) );
        aMotifLib = osl_loadModule( aMotifName.pData, nRtldMode );
        #endif
        OUString aLibName( RTL_CONSTASCII_USTRINGPARAM( OGL_LIBNAME ) );
        mpGLLib = osl_loadModule( aLibName.pData, nRtldMode );
    }
    if( ! mpGLLib )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, OGL_LIBNAME "could not be opened\n" );
#endif
        return FALSE;
    }

    // Internal use
    pCreateContext     = (GLXContext(*)(Display*,XVisualInfo*,GLXContext,Bool ))
        resolveSymbol( "glXCreateContext" );
    pDestroyContext    = (void(*)(Display*,GLXContext))
        resolveSymbol( "glXDestroyContext" );
    pGetCurrentContext = (GLXContext(*)())
        resolveSymbol( "glXGetCurrentContext" );
    pMakeCurrent       = (Bool(*)(Display*,GLXDrawable,GLXContext))
        resolveSymbol( "glXMakeCurrent" );
    pSwapBuffers=(void(*)(Display*, GLXDrawable))
        resolveSymbol( "glXSwapBuffers" );
    pGetConfig = (int(*)(Display*, XVisualInfo*, int, int* ))
        resolveSymbol( "glXGetConfig" );
    pFlush = (void(*)())
        resolveSymbol( "glFlush" );

    BOOL bRet = pCreateContext && pDestroyContext && pGetCurrentContext && pMakeCurrent && pSwapBuffers && pGetConfig ? TRUE : FALSE;

#if OSL_DEBUG_LEVEL > 1
    if( ! bRet )
        fprintf( stderr, "could not find all needed symbols in " OGL_LIBNAME "\n" );
#endif

    return bRet;
}

void X11SalOpenGL::StartScene( SalGraphics* )
{
    // flush pending operations which otherwise might be drawn
    // at the wrong time
    XSync( mpDisplay, False );
}

void X11SalOpenGL::StopScene()
{
    if( maDrawable )
    {
        pSwapBuffers( mpDisplay, maDrawable );
        pFlush();
    }
}

void X11SalOpenGL::MakeVisualWeights( Display* pDisplay,
                                   XVisualInfo* pInfos,
                                   int *pWeights,
                                   int nVisuals )
{
    BOOL bHasGLX = FALSE;
    char **ppExtensions;
    int nExtensions,i ;

    // GLX only on local displays due to strange problems
    // with remote GLX
    if( ! ( *DisplayString( pDisplay ) == ':'                           ||
            !strncmp( DisplayString( pDisplay ), "localhost:", 10 )
            ) )
        return;

    ppExtensions = XListExtensions( pDisplay, &nExtensions );
    for( i=0; i < nExtensions; i++ )
    {
        if( ! strncmp( "GLX", ppExtensions[ i ], 3 ) )
        {
            bHasGLX = TRUE;
            break;
        }
    }
    XFreeExtensionList( ppExtensions );
    if( ! bHasGLX )
        return;

    if( ! ImplInit() )
        return;

    for( i = 0; i < nVisuals; i++ )
    {
        int nDoubleBuffer = 0;
        int nHaveGL = 0;
        // a weight lesser than zero indicates an invalid visual (wrong screen)
        if( pInfos[i].c_class == TrueColor && pInfos[i].depth > 14 && pWeights[i] >= 0)
        {
            pGetConfig( pDisplay, &pInfos[ i ], GLX_USE_GL, &nHaveGL );
            pGetConfig( pDisplay, &pInfos[ i ], GLX_DOUBLEBUFFER, &nDoubleBuffer );
            if( nHaveGL && ! nDoubleBuffer )
            {
                mbHaveGLVisual = TRUE;
                pWeights[ i ] += 65536;
            }
        }
    }
}
