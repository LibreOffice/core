/*************************************************************************
 *
 *  $RCSfile: opengl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:38 $
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

#define _SV_OPENGL_CXX

#define private public
#include <svsys.h>
#include <window.hxx>
#undef private
#define private public

#ifndef _SV_OUTDEV_HXX
#include <outdev.hxx>
#endif
#ifndef _SV_SALOGL_HXX
#include <salogl.hxx>
#endif
#ifndef _SV_OPENGL_HXX
#include <opengl.hxx>
#endif

#include <svapp.hxx>
#include <vos/mutex.hxx>

// -----------------------
// - Fnc-Pointer-Typedef -
// -----------------------

typedef void ( *OGLFncClearDepth )( GLclampd fDepth );
typedef void ( *OGLFncDepthFunc )( GLenum fFunc );
typedef void ( *OGLFncEnable )( GLenum eCap );
typedef void ( *OGLFncDisable )( GLenum eCap );
typedef void ( *OGLFncDepthMask )( GLboolean bFlag );
typedef void ( *OGLFncShadeModel )( GLenum eMode );
typedef void ( *OGLFncEdgeFlag )( GLboolean bFlag );
typedef void ( *OGLFncClear )( GLbitfield nMask );
typedef void ( *OGLFncFlush )( void );
typedef void ( *OGLFncFinish )( void );
typedef void ( *OGLFncViewport )( GLint nX, GLint nY, GLsizei nWidth, GLsizei nHeight );
typedef void ( *OGLFncBegin )( GLenum eMode );
typedef void ( *OGLFncEnd )( void );
typedef void ( *OGLFncVertex3dv )( const GLdouble *fV );
typedef void ( *OGLFncNormal3dv )( const GLdouble *fV );
typedef void ( *OGLFncColor4ub )( GLubyte cRed, GLubyte cGreen, GLubyte cBlue, GLubyte cAlpha );
typedef void ( *OGLFncMaterialfv )( GLenum eFace, GLenum ePNname, const GLfloat *fParams );
typedef void ( *OGLFncMaterialf )( GLenum eFace, GLenum ePName, GLfloat fParam );
typedef void ( *OGLFncLightModelfv )( GLenum ePNname, const GLfloat *fParams );
typedef void ( *OGLFncLightModelf )( GLenum ePname, GLfloat fParam );
typedef void ( *OGLFncLightfv )( GLenum eLight, GLenum ePNname, const GLfloat *fParams );
typedef void ( *OGLFncLightf )( GLenum eLight, GLenum ePname, GLfloat fParam );
typedef void ( *OGLFncPolygonMode )( GLenum eFace, GLenum eMode );
typedef void ( *OGLFncCullFace )( GLenum eMode );
typedef void ( *OGLFncPointSize )( GLfloat fSize );
typedef void ( *OGLFncLineWidth )( GLfloat fWidth );
typedef void ( *OGLFncMatrixMode )( GLenum eMode );
typedef void ( *OGLFncLoadMatrixd )( const GLdouble *fM );
typedef void ( *OGLFncTexCoord2dv )( const GLdouble *pParams );
typedef void ( *OGLFncTexCoord3dv )( const GLdouble *fV );
typedef void ( *OGLFncTexImage1D )( GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels );
typedef void ( *OGLFncTexImage2D )( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels );
typedef void ( *OGLFncCopyTexImage1D )( GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border );
typedef void ( *OGLFncCopyTexImage2D )( GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border );
typedef void ( *OGLFncCopyTexSubImage1D )( GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width );
typedef void ( *OGLFncCopyTexSubImage2D )( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height );
typedef void ( *OGLFncPixelTransferf )( GLenum pname, GLfloat param );
typedef void ( *OGLFncPixelTransferi )( GLenum pname, GLint param );
typedef void ( *OGLFncGetTexLevelParameterfv )( GLenum target, GLint level, GLenum pname, GLfloat *params );
typedef void ( *OGLFncGetTexLevelParameteriv )( GLenum target, GLint level, GLenum pname, GLint *params );
typedef void ( *OGLFncGetTexParameterfv )( GLenum target, GLenum pname, GLfloat *params );
typedef void ( *OGLFncGetTexParameteriv )( GLenum target, GLenum pname, GLint *params );
typedef void ( *OGLFncTexSubImage1D )( GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels );
typedef void ( *OGLFncTexSubImage2D )( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels );
typedef void ( *OGLFncPixelStoref )( GLenum pname, GLfloat param );
typedef void ( *OGLFncPixelStorei )( GLenum pname, GLint param );
typedef void ( *OGLFncGenTextures )( GLsizei n, GLuint *textures );
typedef GLboolean ( *OGLFncIsTexture )( GLuint texture );
typedef void ( *OGLFncBindTexture )( GLenum target, GLuint texture );
typedef void ( *OGLFncDeleteTextures )( GLsizei n, const GLuint *textures );
typedef GLboolean ( *OGLFncAreTexturesResident )( GLsizei n, const GLuint *textures, GLboolean *residences );
typedef void ( *OGLFncPrioritizeTextures )( GLsizei n, const GLuint *textures, const GLclampf *priorities );
typedef void ( *OGLFncTexEnvf )( GLenum target, GLenum pname, GLfloat param );
typedef void ( *OGLFncTexEnvfv )( GLenum target, GLenum pname, const GLfloat *params );
typedef void ( *OGLFncTexEnvi )( GLenum target, GLenum pname, GLint param );
typedef void ( *OGLFncTexEnviv )( GLenum target, GLenum pname, const GLint *params );
typedef void ( *OGLFncTexParameterf )( GLenum target, GLenum pname, GLfloat param );
typedef void ( *OGLFncTexParameterfv )( GLenum target, GLenum pname, const GLfloat *params );
typedef void ( *OGLFncTexParameteri )( GLenum target, GLenum pname, GLint param );
typedef void ( *OGLFncTexParameteriv )( GLenum target, GLenum pname, const GLint *params );
typedef void ( *OGLFncTexGend )( GLenum coord, GLenum pname, GLdouble param );
typedef void ( *OGLFncTexGendv )( GLenum coord, GLenum pname, const GLdouble *params );
typedef void ( *OGLFncTexGenf )( GLenum coord, GLenum pname, GLfloat param );
typedef void ( *OGLFncTexGenfv )( GLenum coord, GLenum pname, const GLfloat *params );
typedef void ( *OGLFncTexGeni )( GLenum coord, GLenum pname, GLint param );
typedef void ( *OGLFncTexGeniv )( GLenum coord, GLenum pname, const GLint *params );
typedef void ( *OGLFncGetIntegerv )( GLenum pname, GLint *params );
typedef void ( *OGLFncPolygonOffset ) ( GLfloat factor, GLfloat units );
typedef void ( *OGLFncScissor ) ( GLint x, GLint y, GLsizei width, GLsizei height );

typedef void ( *OGLFncEnableClientState ) ( GLenum array );
typedef void ( *OGLFncDisableClientState ) ( GLenum array );
typedef void ( *OGLFncVertexPointer ) ( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer );
typedef void ( *OGLFncColorPointer ) ( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer );
typedef void ( *OGLFncIndexPointer ) ( GLenum type, GLsizei stride, const GLvoid *pointer );
typedef void ( *OGLFncNormalPointer ) ( GLenum type, GLsizei stride, const GLvoid *pointer );
typedef void ( *OGLFncTexCoordPointer ) ( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer );
typedef void ( *OGLFncEdgeFlagPointer ) ( GLsizei stride, const GLvoid *pointer );
typedef void ( *OGLFncArrayElement ) ( GLint i );
typedef void ( *OGLFncDrawElements ) ( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices );
typedef void ( *OGLFncDrawArrays ) ( GLenum mode, GLint first, GLsizei count );
typedef void ( *OGLFncInterleavedArrays ) ( GLenum format, GLsizei stride, const GLvoid *pointer );

typedef void ( *OGLFncLoadIdentity ) ();
typedef void ( *OGLFncBlendFunc ) ( GLenum sfactor, GLenum dfactor );

// ----------
// - Macros -
// ----------

#define PGRAPHICS mpOutDev->mpGraphics

#ifndef REMOTE_APPSERVER
#define OGL_INIT() (mpOGL && (mpOutDev->mpGraphics || mpOutDev->ImplGetGraphics()))
#else
#define OGL_INIT() (mpOGL && (mpOutDev->mpGraphics || mpOutDev->ImplGetGraphics()))
#endif

#define INIT_OGLFNC( FncName ) static OGLFnc##FncName pImplOpenGLFnc##FncName = NULL;
#define GET_OGLFNC_GL( FncName )                                                    \
pImplOpenGLFnc##FncName = (OGLFnc##FncName##) mpOGL->GetOGLFnc( "gl" #FncName );    \
if( !pImplOpenGLFnc##FncName ) bRet = FALSE;

// ----------
// - OpenGL -
// ----------

static BOOL bImplOpenGLFncPtrInitialized = FALSE;

INIT_OGLFNC( ClearDepth );
INIT_OGLFNC( DepthFunc );
INIT_OGLFNC( Enable );
INIT_OGLFNC( Disable );
INIT_OGLFNC( DepthMask );
INIT_OGLFNC( ShadeModel );
INIT_OGLFNC( EdgeFlag );
INIT_OGLFNC( Clear );
INIT_OGLFNC( Flush );
INIT_OGLFNC( Finish );
INIT_OGLFNC( Viewport );
INIT_OGLFNC( Begin );
INIT_OGLFNC( End );
INIT_OGLFNC( Vertex3dv );
INIT_OGLFNC( Normal3dv );
INIT_OGLFNC( Color4ub );
INIT_OGLFNC( Materialfv );
INIT_OGLFNC( Materialf );
INIT_OGLFNC( LightModelfv );
INIT_OGLFNC( LightModelf );
INIT_OGLFNC( Lightfv );
INIT_OGLFNC( Lightf );
INIT_OGLFNC( PolygonMode );
INIT_OGLFNC( CullFace );
INIT_OGLFNC( PointSize );
INIT_OGLFNC( LineWidth );
INIT_OGLFNC( MatrixMode );
INIT_OGLFNC( LoadMatrixd );
INIT_OGLFNC( TexCoord2dv );
INIT_OGLFNC( TexCoord3dv );
INIT_OGLFNC( TexImage1D );
INIT_OGLFNC( TexImage2D );
INIT_OGLFNC( CopyTexImage1D );
INIT_OGLFNC( CopyTexImage2D );
INIT_OGLFNC( CopyTexSubImage1D );
INIT_OGLFNC( CopyTexSubImage2D );
INIT_OGLFNC( PixelTransferf );
INIT_OGLFNC( PixelTransferi );
INIT_OGLFNC( GetTexLevelParameterfv );
INIT_OGLFNC( GetTexLevelParameteriv );
INIT_OGLFNC( GetTexParameterfv );
INIT_OGLFNC( GetTexParameteriv );
INIT_OGLFNC( TexSubImage1D );
INIT_OGLFNC( TexSubImage2D );
INIT_OGLFNC( PixelStoref );
INIT_OGLFNC( PixelStorei );
INIT_OGLFNC( GenTextures );
INIT_OGLFNC( IsTexture );
INIT_OGLFNC( BindTexture );
INIT_OGLFNC( DeleteTextures );
INIT_OGLFNC( AreTexturesResident );
INIT_OGLFNC( PrioritizeTextures );
INIT_OGLFNC( TexEnvf );
INIT_OGLFNC( TexEnvfv );
INIT_OGLFNC( TexEnvi );
INIT_OGLFNC( TexEnviv );
INIT_OGLFNC( TexParameterf );
INIT_OGLFNC( TexParameterfv );
INIT_OGLFNC( TexParameteri );
INIT_OGLFNC( TexParameteriv );
INIT_OGLFNC( TexGend );
INIT_OGLFNC( TexGendv );
INIT_OGLFNC( TexGenf );
INIT_OGLFNC( TexGenfv );
INIT_OGLFNC( TexGeni );
INIT_OGLFNC( TexGeniv );
INIT_OGLFNC( GetIntegerv );
INIT_OGLFNC( PolygonOffset );
INIT_OGLFNC( Scissor );

INIT_OGLFNC( EnableClientState );
INIT_OGLFNC( DisableClientState );
INIT_OGLFNC( VertexPointer );
INIT_OGLFNC( ColorPointer );
INIT_OGLFNC( IndexPointer );
INIT_OGLFNC( NormalPointer );
INIT_OGLFNC( TexCoordPointer );
INIT_OGLFNC( EdgeFlagPointer );
INIT_OGLFNC( ArrayElement );
INIT_OGLFNC( DrawElements );
INIT_OGLFNC( DrawArrays );
INIT_OGLFNC( InterleavedArrays );

INIT_OGLFNC( LoadIdentity );
INIT_OGLFNC( BlendFunc );

// ------------------------------------------------------------------------

BOOL OpenGL::ImplInitFncPointers()
{
#ifndef REMOTE_APPSERVER
    BOOL bRet = TRUE;

    GET_OGLFNC_GL( ClearDepth );
    GET_OGLFNC_GL( DepthFunc );
    GET_OGLFNC_GL( Enable );
    GET_OGLFNC_GL( Disable );
    GET_OGLFNC_GL( DepthMask );
    GET_OGLFNC_GL( ShadeModel );
    GET_OGLFNC_GL( EdgeFlag );
    GET_OGLFNC_GL( Clear );
    GET_OGLFNC_GL( Flush );
    GET_OGLFNC_GL( Finish );
    GET_OGLFNC_GL( Viewport );
    GET_OGLFNC_GL( Begin );
    GET_OGLFNC_GL( End );
    GET_OGLFNC_GL( Vertex3dv );
    GET_OGLFNC_GL( Normal3dv );
    GET_OGLFNC_GL( Color4ub );
    GET_OGLFNC_GL( Materialfv );
    GET_OGLFNC_GL( Materialf );
    GET_OGLFNC_GL( LightModelfv );
    GET_OGLFNC_GL( LightModelf  );
    GET_OGLFNC_GL( Lightfv );
    GET_OGLFNC_GL( Lightf );
    GET_OGLFNC_GL( PolygonMode  );
    GET_OGLFNC_GL( CullFace );
    GET_OGLFNC_GL( PointSize );
    GET_OGLFNC_GL( LineWidth );
    GET_OGLFNC_GL( MatrixMode );
    GET_OGLFNC_GL( LoadMatrixd  );
    GET_OGLFNC_GL( TexCoord2dv  );
    GET_OGLFNC_GL( TexCoord3dv  );
    GET_OGLFNC_GL( TexImage1D );
    GET_OGLFNC_GL( TexImage2D );
    GET_OGLFNC_GL( CopyTexImage1D );
    GET_OGLFNC_GL( CopyTexImage2D );
    GET_OGLFNC_GL( CopyTexSubImage1D );
    GET_OGLFNC_GL( CopyTexSubImage2D );
    GET_OGLFNC_GL( PixelTransferf );
    GET_OGLFNC_GL( PixelTransferi );
    GET_OGLFNC_GL( GetTexLevelParameterfv );
    GET_OGLFNC_GL( GetTexLevelParameteriv );
    GET_OGLFNC_GL( GetTexParameterfv );
    GET_OGLFNC_GL( GetTexParameteriv );
    GET_OGLFNC_GL( TexSubImage1D );
    GET_OGLFNC_GL( TexSubImage2D );
    GET_OGLFNC_GL( PixelStoref );
    GET_OGLFNC_GL( PixelStorei );
    GET_OGLFNC_GL( GenTextures );
    GET_OGLFNC_GL( IsTexture );
    GET_OGLFNC_GL( BindTexture );
    GET_OGLFNC_GL( DeleteTextures );
    GET_OGLFNC_GL( AreTexturesResident );
    GET_OGLFNC_GL( PrioritizeTextures );
    GET_OGLFNC_GL( TexEnvf );
    GET_OGLFNC_GL( TexEnvfv );
    GET_OGLFNC_GL( TexEnvi );
    GET_OGLFNC_GL( TexEnviv );
    GET_OGLFNC_GL( TexParameterf );
    GET_OGLFNC_GL( TexParameterfv );
    GET_OGLFNC_GL( TexParameteri );
    GET_OGLFNC_GL( TexParameteriv );
    GET_OGLFNC_GL( TexGend );
    GET_OGLFNC_GL( TexGendv );
    GET_OGLFNC_GL( TexGenf );
    GET_OGLFNC_GL( TexGenfv );
    GET_OGLFNC_GL( TexGeni );
    GET_OGLFNC_GL( TexGeniv );
    GET_OGLFNC_GL( GetIntegerv );
    GET_OGLFNC_GL( PolygonOffset );
    GET_OGLFNC_GL( Scissor );

    GET_OGLFNC_GL( EnableClientState );
    GET_OGLFNC_GL( DisableClientState );
    GET_OGLFNC_GL( VertexPointer );
    GET_OGLFNC_GL( ColorPointer );
    GET_OGLFNC_GL( IndexPointer );
    GET_OGLFNC_GL( NormalPointer );
    GET_OGLFNC_GL( TexCoordPointer );
    GET_OGLFNC_GL( EdgeFlagPointer );
    GET_OGLFNC_GL( ArrayElement );
    GET_OGLFNC_GL( DrawElements );
    GET_OGLFNC_GL( DrawArrays );
    GET_OGLFNC_GL( InterleavedArrays );

    GET_OGLFNC_GL( LoadIdentity );
    GET_OGLFNC_GL( BlendFunc );

    return bRet;
#else
    return FALSE;
#endif
}

// ------------------------------------------------------------------------

OpenGL::OpenGL( OutputDevice* pOutDev ) :
            mpOutDev( pOutDev )
{
    ImplInit();
}

// ------------------------------------------------------------------------

OpenGL::~OpenGL()
{
#ifndef REMOTE_APPSERVER
    delete mpOGL;
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::ImplInit()
{
#ifndef REMOTE_APPSERVER
    if( PGRAPHICS || mpOutDev->ImplGetGraphics() )
    {
        mpOGL = new SalOpenGL( PGRAPHICS );

        if ( !mpOGL->Create() || (!bImplOpenGLFncPtrInitialized && !ImplInitFncPointers()) )
        {
            delete mpOGL;
            mpOGL = NULL;
        }
        else
            bImplOpenGLFncPtrInitialized = TRUE;
    }
    else
        mpOGL = NULL;
#else
    mpOGL = NULL;
#endif
}

// ------------------------------------------------------------------------

void OpenGL::SetConnectOutputDevice( OutputDevice* pOutDev )
{
#ifndef REMOTE_APPSERVER
    delete mpOGL;
    mpOutDev = pOutDev;
    ImplInit();
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::ClearDepth( GLclampd fDepth )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncClearDepth( fDepth );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::DepthFunc( GLenum eFunc )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncDepthFunc( eFunc );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::Enable( GLenum eCap )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncEnable( eCap );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::Disable( GLenum eCap )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncDisable( eCap );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::DepthMask( GLboolean bFlag )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncDepthMask( bFlag );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::ShadeModel( GLenum eMode )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncShadeModel( eMode );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::EdgeFlag( GLboolean bFlag )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncEdgeFlag( bFlag );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::Clear( GLbitfield nMask )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
#if defined UNX && ! defined MACOSX
        mpOGL->StartScene( PGRAPHICS );
#endif
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncClear( nMask );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::Flush()
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncFlush();
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::Finish()
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncFinish();
#if defined UNX && ! defined MACOSX
        mpOGL->StopScene();
#endif
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::Viewport( GLint nX, GLint nY, GLsizei nWidth, GLsizei nHeight )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        long nOutHeight;

        if( mpOutDev->GetOutDevType() == OUTDEV_WINDOW )
            nOutHeight = ( (Window*) mpOutDev )->mpFrameWindow->mnOutHeight;
        else
            nOutHeight = mpOutDev->mnOutHeight;

        mpOGL->OGLEntry( PGRAPHICS );

        pImplOpenGLFncViewport( nX + mpOutDev->mnOutOffX,
                      nOutHeight - nY - nHeight - mpOutDev->mnOutOffY,
                      nWidth, nHeight );

        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::Begin( GLenum eMode )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncBegin( eMode );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::End()
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncEnd();
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::Vertex3dv( const GLdouble* fVar )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncVertex3dv( fVar );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::Normal3dv( const GLdouble* fVar )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncNormal3dv( fVar );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::Color4ub( GLubyte cRed, GLubyte cGreen, GLubyte cBlue, GLubyte cAlpha )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncColor4ub( cRed, cGreen, cBlue, cAlpha );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::Materialfv( GLenum eFace, GLenum ePName, const GLfloat *fParams )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncMaterialfv( eFace, ePName, fParams );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::Materialf( GLenum eFace, GLenum ePName, GLfloat fParam )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncMaterialf( eFace, ePName, fParam );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::LightModelfv( GLenum ePName, const GLfloat *fParams )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncLightModelfv( ePName, fParams );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::LightModelf( GLenum ePName, GLfloat fParam )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncLightModelf( ePName, fParam );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::Lightfv( GLenum eLight, GLenum ePName, const GLfloat *fParams )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncLightfv( eLight, ePName, fParams );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::Lightf( GLenum eLight, GLenum ePName, GLfloat fParam )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncLightf( eLight, ePName, fParam );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::PolygonMode( GLenum eFace, GLenum eMode )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncPolygonMode( eFace, eMode );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::CullFace( GLenum eMode )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncCullFace( eMode );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::PointSize( GLfloat fSize )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncPointSize( fSize );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::LineWidth( GLfloat fWidth )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncLineWidth( fWidth );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::MatrixMode( GLenum eMode )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncMatrixMode( eMode );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::LoadMatrixd( const GLdouble *fM )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncLoadMatrixd( fM );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::TexCoord2dv( const GLdouble *pParams )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncTexCoord2dv( pParams );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::TexCoord3dv( const GLdouble *fVar )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncTexCoord3dv( fVar );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::TexImage1D( GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncTexImage1D( target, level, internalformat, width, border, format, type, pixels );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::TexImage2D( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncTexImage2D( target, level, internalformat, width, height, border, format, type, pixels );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::CopyTexImage1D( GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncCopyTexImage1D( target, level, internalFormat, x, y, width, border );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::CopyTexImage2D( GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncCopyTexImage2D( target, level, internalFormat, x, y, width, height, border );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::CopyTexSubImage1D( GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncCopyTexSubImage1D( target, level, xoffset, x, y, width );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::CopyTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncCopyTexSubImage2D( target, level, xoffset, yoffset, x, y, width, height );
        mpOGL->OGLExit( PGRAPHICS );
    }

#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::PixelTransferf( GLenum pname, GLfloat param )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncPixelTransferf( pname, param );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::PixelTransferi( GLenum pname, GLint param )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncPixelTransferi( pname, param );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::GetTexLevelParameterfv( GLenum target, GLint level, GLenum pname, GLfloat *params )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncGetTexLevelParameterfv( target, level, pname, params );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::GetTexLevelParameteriv( GLenum target, GLint level, GLenum pname, GLint *params )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncGetTexLevelParameteriv( target, level, pname, params );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::GetTexParameterfv( GLenum target, GLenum pname, GLfloat *params )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncGetTexParameterfv( target, pname, params );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::GetTexParameteriv( GLenum target, GLenum pname, GLint *params )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncGetTexParameteriv( target, pname, params );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::TexSubImage1D( GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncTexSubImage1D( target, level, xoffset, width, format, type, pixels );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::TexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncTexSubImage2D( target, level, xoffset, yoffset, width, height, format, type, pixels );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::PixelStoref( GLenum pname, GLfloat param )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncPixelStoref( pname, param );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::PixelStorei( GLenum pname, GLint param )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncPixelStorei( pname, param );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::GenTextures( GLsizei n, GLuint *textures )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncGenTextures( n, textures );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

GLboolean OpenGL::IsTexture( GLuint texture )
{
    GLboolean bRet = FALSE;

#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        bRet = pImplOpenGLFncIsTexture( texture );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif

    return bRet;
}

// ------------------------------------------------------------------------

void OpenGL::BindTexture( GLenum target, GLuint texture )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncBindTexture( target, texture );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::DeleteTextures( GLsizei n, const GLuint *textures )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncDeleteTextures( n, textures );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

GLboolean OpenGL::AreTexturesResident( GLsizei n, const GLuint *textures, GLboolean *residences )
{
    GLboolean bRet = FALSE;

#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        bRet = pImplOpenGLFncAreTexturesResident( n, textures, residences );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif

    return bRet;
}

// ------------------------------------------------------------------------

void OpenGL::PrioritizeTextures( GLsizei n, const GLuint *textures, const GLclampf *priorities )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncPrioritizeTextures( n, textures, priorities );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::TexEnvf( GLenum target, GLenum pname, GLfloat param )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncTexEnvf( target, pname, param );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::TexEnvfv( GLenum target, GLenum pname, const GLfloat *params )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncTexEnvfv( target, pname, params );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::TexEnvi( GLenum target, GLenum pname, GLint param )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncTexEnvi( target, pname, param );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::TexEnviv( GLenum target, GLenum pname, const GLint *params )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncTexEnviv( target, pname, params );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::TexParameterf( GLenum target, GLenum pname, GLfloat param )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncTexParameterf( target, pname, param );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::TexParameterfv( GLenum target, GLenum pname, const GLfloat *params )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncTexParameterfv( target, pname, params );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::TexParameteri( GLenum target, GLenum pname, GLint param )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncTexParameteri( target, pname, param );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::TexParameteriv( GLenum target, GLenum pname, const GLint *params )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncTexParameteriv( target, pname, params );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::TexGend( GLenum coord, GLenum pname, GLdouble param )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncTexGend( coord, pname, param );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::TexGendv( GLenum coord, GLenum pname, const GLdouble *params )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncTexGendv( coord, pname, params );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::TexGenf( GLenum coord, GLenum pname, GLfloat param )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncTexGenf( coord, pname, param );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::TexGenfv( GLenum coord, GLenum pname, const GLfloat *params )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncTexGenfv( coord, pname, params );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::TexGeni( GLenum coord, GLenum pname, GLint param )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncTexGeni( coord, pname, param );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::TexGeniv( GLenum coord, GLenum pname, const GLint *params )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncTexGeniv( coord, pname, params );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::GetIntegerv( GLenum pname, GLint *params )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncGetIntegerv( pname, params );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::PolygonOffset( GLfloat factor, GLfloat units )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncPolygonOffset( factor, units );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::Scissor( GLint nX, GLint nY, GLsizei nWidth, GLsizei nHeight )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        long nOutHeight;

        if( mpOutDev->GetOutDevType() == OUTDEV_WINDOW )
            nOutHeight = ( (Window*) mpOutDev )->mpFrameWindow->mnOutHeight;
        else
            nOutHeight = mpOutDev->mnOutHeight;

        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncScissor( nX + mpOutDev->mnOutOffX,
                     nOutHeight - nY - nHeight - mpOutDev->mnOutOffY,
                     nWidth, nHeight );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::EnableClientState( GLenum array )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncEnableClientState( array );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::DisableClientState( GLenum array )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncDisableClientState( array );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::VertexPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncVertexPointer( size, type, stride, pointer );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::ColorPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncColorPointer( size, type, stride, pointer );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::IndexPointer( GLenum type, GLsizei stride, const GLvoid *pointer )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncIndexPointer( type, stride, pointer );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::NormalPointer( GLenum type, GLsizei stride, const GLvoid *pointer )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncNormalPointer( type, stride, pointer );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::TexCoordPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncTexCoordPointer( size, type, stride, pointer );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::EdgeFlagPointer( GLsizei stride, const GLvoid *pointer )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncEdgeFlagPointer( stride, pointer );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::ArrayElement( GLint i )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncArrayElement( i );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::DrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncDrawElements( mode, count, type, indices );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::DrawArrays( GLenum mode, GLint first, GLsizei count )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncDrawArrays( mode, first, count );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::InterleavedArrays( GLenum format, GLsizei stride, const GLvoid *pointer )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncInterleavedArrays( format, stride, pointer );
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::LoadIdentity( )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncLoadIdentity();
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

// ------------------------------------------------------------------------

void OpenGL::BlendFunc( GLenum sfactor, GLenum dfactor )
{
#ifndef REMOTE_APPSERVER
    if( OGL_INIT() )
    {
        mpOGL->OGLEntry( PGRAPHICS );
        pImplOpenGLFncBlendFunc( sfactor, dfactor);
        mpOGL->OGLExit( PGRAPHICS );
    }
#else
#endif
}

