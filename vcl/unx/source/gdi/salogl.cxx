/*************************************************************************
 *
 *  $RCSfile: salogl.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 14:46:14 $
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

#define _SV_SALOGL_CXX

#include <salunx.h>

#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif

#ifndef _SV_SALDISP_HXX
#include <saldisp.hxx>
#endif

#ifndef _SV_SALOGL_H
#include <salogl.h>
#endif

#ifndef _SV_SALGDI_H
#include <salgdi.h>
#endif

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
#define OGL_LIBNAME "libGL.so"
#endif

// ----------
// - Macros -
// ----------

// -----------------
// - Statics init. -
// -----------------

// Members
GLXContext      X11SalOpenGL::maGLXContext = 0;
Display*        X11SalOpenGL::mpDisplay    = 0;
XVisualInfo*    X11SalOpenGL::mpVisualInfo = 0;
BOOL            X11SalOpenGL::mbHaveGLVisual = FALSE;

void *      X11SalOpenGL::mpGLLib    = 0;
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

X11SalOpenGL::X11SalOpenGL( SalGraphics* pSGraphics )
{
    X11SalGraphics* pGraphics = static_cast<X11SalGraphics*>(pSGraphics);
    mpDisplay    = pGraphics->GetXDisplay();
    mpVisualInfo = pGraphics->GetDisplay()->GetVisual();
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
            pGetConfig( mpDisplay, mpVisualInfo,
                        GLX_USE_GL, &nHaveGL );
            pGetConfig( mpDisplay, mpVisualInfo,
                        GLX_DOUBLEBUFFER, &nDoubleBuffer );
            if( nHaveGL && ! nDoubleBuffer )
            {
                SalDisplay* pSalDisplay = GetSalData()->GetDefDisp();
                BOOL bPreviousState =
                    pSalDisplay->GetXLib()->GetIgnoreXErrors();
                pSalDisplay->GetXLib()->SetIgnoreXErrors( TRUE );
                mbHaveGLVisual = TRUE;

                maGLXContext = pCreateContext( mpDisplay, mpVisualInfo, 0, True );
                if( pSalDisplay->GetXLib()->WasXError() )
                    mbHaveGLVisual = FALSE;
                else
                    pMakeCurrent( mpDisplay, maDrawable, maGLXContext );
                if( pSalDisplay->GetXLib()->WasXError() )
                    mbHaveGLVisual = FALSE;
                pSalDisplay->GetXLib()->SetIgnoreXErrors( bPreviousState );

                if( mbHaveGLVisual )
                    mnOGLState = OGL_STATE_VALID;
                else
                    maGLXContext = None;
            }
        }
        if( mnOGLState != OGL_STATE_VALID )
        {
            ImplFreeLib();
            mnOGLState = OGL_STATE_INVALID;
        }
#if OSL_DEBUG_LEVEL > 1
        if( mnOGLState == OGL_STATE_VALID )
            fprintf( stderr, "Using GLX on visual id %x.\n", mpVisualInfo->visualid );
        else
            fprintf( stderr, "Not using GLX.\n" );
#endif
    }

    return mnOGLState == OGL_STATE_VALID ? TRUE : FALSE;
}

// ------------------------------------------------------------------------

void X11SalOpenGL::Release()
{
    ImplFreeLib();
}

// ------------------------------------------------------------------------

void* X11SalOpenGL::GetOGLFnc( const char *pFncName )
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

void X11SalOpenGL::OGLExit( SalGraphics* pGraphics )
{
}

// ------------------------------------------------------------------------

void X11SalOpenGL::ImplFreeLib()
{
    if( mpGLLib )
    {
        if( maGLXContext && pDestroyContext )
            pDestroyContext( mpDisplay, maGLXContext );
        osl_unloadModule( mpGLLib );

        mpGLLib             = 0;
        pCreateContext      = 0;
        pDestroyContext     = 0;
        pGetCurrentContext  = 0;
        pMakeCurrent        = 0;
        pSwapBuffers        = 0;
        pGetConfig          = 0;
    }
}

// ------------------------------------------------------------------------

void* X11SalOpenGL::resolveSymbol( const char* pSymbol )
{
    void* pSym = NULL;
    if( mpGLLib )
    {
        OUString aSym = OUString::createFromAscii( pSymbol );
        pSym = osl_getSymbol( mpGLLib, aSym.pData );
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
        OUString aLibName( RTL_CONSTASCII_USTRINGPARAM( OGL_LIBNAME ) );
        mpGLLib = osl_loadModule( aLibName.pData, SAL_LOADMODULE_NOW );
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

void X11SalOpenGL::StartScene( SalGraphics* pGraphics )
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
    {
        ImplFreeLib();
        return;
    }

    for( i = 0; i < nVisuals; i++ )
    {
        int nDoubleBuffer = 0;
        int nHaveGL = 0;
        // a weight lesser than zero indicates an invalid visual (wrong screen)
        if( pInfos[i].c_class == TrueColor && pWeights[i] >= 0)
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

    ImplFreeLib();
}
