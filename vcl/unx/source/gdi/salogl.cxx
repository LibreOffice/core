/*************************************************************************
 *
 *  $RCSfile: salogl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:43 $
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

#ifndef _SV_SALOGL_HXX
#include <salogl.hxx>
#endif

#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif

#include <stdlib.h>
#include <stdio.h>

// ------------
// - Lib-Name -
// ------------

#define OGL_LIBNAME "libGL.so"
// ----------
// - Macros -
// ----------

// NETBSD has neither RTLD_GLOBAL nor RTLD_NOW
#ifdef NETBSD
#define DLOPEN_MODE 0
#else
#define DLOPEN_MODE (RTLD_NOW | RTLD_GLOBAL)
#endif


// -----------------
// - Statics init. -
// -----------------

// Members
GLXContext      SalOpenGL::maGLXContext = 0;
Display*        SalOpenGL::mpDisplay    = 0;
XVisualInfo*    SalOpenGL::mpVisualInfo = 0;
BOOL            SalOpenGL::mbHaveGLVisual = FALSE;

void *      SalOpenGL::mpGLLib    = 0;
ULONG       SalOpenGL::mnOGLState = OGL_STATE_UNLOADED;

GLXContext (*SalOpenGL::pCreateContext)( Display *, XVisualInfo *, GLXContext, Bool ) = 0;
void       (*SalOpenGL::pDestroyContext)( Display *, GLXContext ) = 0;
GLXContext (*SalOpenGL::pGetCurrentContext)( ) = 0;
Bool       (*SalOpenGL::pMakeCurrent)( Display *, GLXDrawable, GLXContext ) = 0;
void        (*SalOpenGL::pSwapBuffers)( Display*, GLXDrawable ) = 0;
int         (*SalOpenGL::pGetConfig)( Display*, XVisualInfo*, int, int* ) = 0;
void       (*SalOpenGL::pFlush)() = 0;

// -------------
// - SalOpenGL -
// -------------

SalOpenGL::SalOpenGL( SalGraphics* pGraphics )
{
    mpDisplay    = pGraphics->maGraphicsData.GetXDisplay();
    mpVisualInfo = pGraphics->maGraphicsData.GetDisplay()->GetVisual();
    maDrawable   = pGraphics->maGraphicsData.GetDrawable();
}

// ------------------------------------------------------------------------

SalOpenGL::~SalOpenGL()
{
}

// ------------------------------------------------------------------------

BOOL SalOpenGL::Create()
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
#ifdef DEBUG
            if( ! bHasGLX )
                fprintf( stderr, "XServer does not support GLX extension\n" );
#endif
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
#if defined DEBUG
        if( mnOGLState == OGL_STATE_VALID )
            fprintf( stderr, "Using GLX on visual id %x.\n", mpVisualInfo->visualid );
        else
            fprintf( stderr, "Not using GLX.\n" );
#endif
    }

    return mnOGLState == OGL_STATE_VALID ? TRUE : FALSE;
}

// ------------------------------------------------------------------------

void SalOpenGL::Release()
{
    ImplFreeLib();
}

// ------------------------------------------------------------------------

void* SalOpenGL::GetOGLFnc( const String& rFncName )
{
    if( mpGLLib )
    {
        return dlsym( mpGLLib, ByteString( rFncName, RTL_TEXTENCODING_ASCII_US ).GetBuffer() );
    }
    return NULL;
}

void* SalOpenGL::GetOGLFnc( char *pFncName )
{
    if( mpGLLib )
    {
        return dlsym( mpGLLib, pFncName );
    }
    return NULL;
}

// ------------------------------------------------------------------------

void SalOpenGL::OGLEntry( SalGraphics* pGraphics )
{
    GLXDrawable aDrawable = pGraphics->maGraphicsData.GetDrawable();
    if( aDrawable != maDrawable )
    {
        maDrawable = aDrawable;
        pMakeCurrent( mpDisplay, maDrawable, maGLXContext );
    }
}

// ------------------------------------------------------------------------

void SalOpenGL::OGLExit( SalGraphics* pGraphics )
{
}

// ------------------------------------------------------------------------

void SalOpenGL::ImplFreeLib()
{
    if( mpGLLib )
    {
        if( maGLXContext && pDestroyContext )
            pDestroyContext( mpDisplay, maGLXContext );
        dlclose( mpGLLib );

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

BOOL SalOpenGL::ImplInit()
{
    if( ! mpGLLib )
    {
        ByteString sNoGL( getenv( "SAL_NOOPENGL" ) );
        if( sNoGL.ToLowerAscii() == "true"  ) return FALSE;
        mpGLLib = dlopen( OGL_LIBNAME, DLOPEN_MODE );
    }
    if( ! mpGLLib )
    {
#ifdef DEBUG
        fprintf( stderr, OGL_LIBNAME "could not be opened: %s\n", dlerror() );
#endif
        return FALSE;
    }

    // Internal use
    pCreateContext     = (GLXContext(*)(Display*,XVisualInfo*,GLXContext,Bool ))
        GetOGLFnc( "glXCreateContext" );
    pDestroyContext    = (void(*)(Display*,GLXContext))
        GetOGLFnc( "glXDestroyContext" );
    pGetCurrentContext = (GLXContext(*)())
        GetOGLFnc( "glXGetCurrentContext" );
    pMakeCurrent       = (Bool(*)(Display*,GLXDrawable,GLXContext))
        GetOGLFnc( "glXMakeCurrent" );
    pSwapBuffers=(void(*)(Display*, GLXDrawable))
        GetOGLFnc( "glXSwapBuffers" );
    pGetConfig = (int(*)(Display*, XVisualInfo*, int, int* ))
        GetOGLFnc( "glXGetConfig" );
    pFlush = (void(*)())
        GetOGLFnc( "glFlush" );

    BOOL bRet = pCreateContext && pDestroyContext && pGetCurrentContext && pMakeCurrent && pSwapBuffers && pGetConfig ? TRUE : FALSE;

#ifdef DEBUG
    if( ! bRet )
        fprintf( stderr, "could not find all needed symbols in " OGL_LIBNAME "\n" );
#endif

    return bRet;
}

void SalOpenGL::StartScene( SalGraphics* pGraphics )
{
    // flush pending operations which otherwise might be drawn
    // at the wrong time
    XSync( mpDisplay, False );
}

void SalOpenGL::StopScene()
{
    if( maDrawable )
    {
        pSwapBuffers( mpDisplay, maDrawable );
        pFlush();
    }
}

void SalOpenGL::MakeVisualWeights( Display* pDisplay,
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
        if( pInfos[i].c_class == TrueColor )
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
