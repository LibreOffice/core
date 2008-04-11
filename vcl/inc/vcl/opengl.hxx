/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: opengl.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SV_OPENGL_HXX
#define _SV_OPENGL_HXX

#include <tools/gen.hxx>
#include <vcl/salotype.hxx>
#include <vcl/dllapi.h>

// ----------
// - OpenGL -
// ----------

class OutputDevice;
class SalOpenGL;
class RmOpenGL;

class VCL_DLLPUBLIC OpenGL
{
    static BOOL     mbNoOGL;

private:
    OutputDevice*   mpOutDev;
    SalOpenGL*      mpOGL;

    SAL_DLLPRIVATE void ImplInit();
    SAL_DLLPRIVATE BOOL ImplInitFncPointers();

public:
                    OpenGL( OutputDevice* pOutDev );
                    ~OpenGL();

    BOOL            IsValid() const { return( mpOGL != NULL ); }
    static void     Invalidate( BOOL bInvalid = TRUE ) { mbNoOGL = bInvalid; }

    void            SetConnectOutputDevice( OutputDevice* pOutDev );
    OutputDevice*   GetConnectOutputDevice() const { return mpOutDev; }

    void            ClearDepth( GLclampd fDepth );
    void            DepthFunc( GLenum eFunc );
    void            Enable( GLenum eCap );
    void            Disable( GLenum eCap );
    void            DepthMask( GLboolean bFlag );
    void            ShadeModel( GLenum eMode );
    void            EdgeFlag( GLboolean bFlag );
    void            Clear( GLbitfield nMask );
    void            Flush();
    void            Finish();
    void            Viewport( GLint nX, GLint nY, GLsizei nWidth, GLsizei nHeight );
    void            Begin( GLenum eMode );
    void            End();
    void            Vertex3dv( const GLdouble *fVar );
    void            Normal3dv( const GLdouble *fVar );
    void            TexCoord3dv( const GLdouble *fVar );
    void            Color4ub( GLubyte cRed, GLubyte cGreen, GLubyte cBlue, GLubyte cAlpha );
    void            Materialfv( GLenum eFace, GLenum ePName, const GLfloat *fParams );
    void            Materialf( GLenum eFace, GLenum ePName, GLfloat fParam );
    void            LightModelfv( GLenum ePName, const GLfloat *fParams );
    void            LightModelf( GLenum ePName, GLfloat fParam );
    void            Lightfv( GLenum eLight, GLenum ePName, const GLfloat *fParams );
    void            Lightf( GLenum eLight, GLenum ePName, GLfloat fParam );
    void            PolygonMode( GLenum eFace, GLenum eMode );
    void            CullFace( GLenum eMode );
    void            PointSize( GLfloat fSize );
    void            LineWidth( GLfloat fWidth );
    void            MatrixMode( GLenum eMode );
    void            LoadMatrixd( const GLdouble *fM );
    void            TexImage1D( GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels );
    void            TexImage2D( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels );
    void            CopyTexImage1D( GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border );
    void            CopyTexImage2D( GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border );
    void            CopyTexSubImage1D( GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width );
    void            CopyTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height );
    void            PixelTransferf( GLenum pname, GLfloat param );
    void            PixelTransferi( GLenum pname, GLint param );
    void            GetTexLevelParameterfv( GLenum target, GLint level, GLenum pname, GLfloat *params );
    void            GetTexLevelParameteriv( GLenum target, GLint level, GLenum pname, GLint *params );
    void            GetTexParameterfv( GLenum target, GLenum pname, GLfloat *params );
    void            GetTexParameteriv( GLenum target, GLenum pname, GLint *params );
    void            TexSubImage1D( GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels );
    void            TexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels );
    void            PixelStoref( GLenum pname, GLfloat param );
    void            PixelStorei( GLenum pname, GLint param );
    void            GenTextures( GLsizei n, GLuint *textures );
    GLboolean       IsTexture( GLuint texture );
    void            BindTexture( GLenum target, GLuint texture );
    void            DeleteTextures( GLsizei n, const GLuint *textures );
    GLboolean       AreTexturesResident( GLsizei n, const GLuint *textures, GLboolean *residences );
    void            PrioritizeTextures( GLsizei n, const GLuint *textures, const GLclampf *priorities );
    void            TexEnvf( GLenum target, GLenum pname, GLfloat param );
    void            TexEnvfv( GLenum target, GLenum pname, const GLfloat *params );
    void            TexEnvi( GLenum target, GLenum pname, GLint param );
    void            TexEnviv( GLenum target, GLenum pname, const GLint *params );
    void            TexParameterf( GLenum target, GLenum pname, GLfloat param );
    void            TexParameterfv( GLenum target, GLenum pname, const GLfloat *params );
    void            TexParameteri( GLenum target, GLenum pname, GLint param );
    void            TexParameteriv( GLenum target, GLenum pname, const GLint *params );
    void            TexGend( GLenum coord, GLenum pname, GLdouble param );
    void            TexGendv( GLenum coord, GLenum pname, const GLdouble *params );
    void            TexGenf( GLenum coord, GLenum pname, GLfloat param );
    void            TexGenfv( GLenum coord, GLenum pname, const GLfloat *params );
    void            TexGeni( GLenum coord, GLenum pname, GLint param );
    void            TexGeniv( GLenum coord, GLenum pname, const GLint *params );
    void            TexCoord2dv( const GLdouble *pParams );
    void            GetIntegerv( GLenum pname, GLint *params );
    void            PolygonOffset( GLfloat factor, GLfloat units );
    void            Scissor( GLint x, GLint y, GLsizei width, GLsizei height );

    void            EnableClientState( GLenum array );
    void            DisableClientState( GLenum array );
    void            VertexPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer );
    void            ColorPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer );
    void            IndexPointer( GLenum type, GLsizei stride, const GLvoid *pointer );
    void            NormalPointer( GLenum type, GLsizei stride, const GLvoid *pointer );
    void            TexCoordPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer );
    void            EdgeFlagPointer( GLsizei stride, const GLvoid *pointer );
    void            ArrayElement( GLint i );
    void            DrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices );
    void            DrawArrays( GLenum mode, GLint first, GLsizei count );
    void            InterleavedArrays( GLenum format, GLsizei stride, const GLvoid *pointer );

    void            LoadIdentity();
    void            BlendFunc( GLenum sfactor,  GLenum dfactor );

private:
                    OpenGL();
                    OpenGL( const OpenGL& );
    OpenGL&         operator=( const OpenGL& );
};

#endif // _SV_OPENGL_HXX
