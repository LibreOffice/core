/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef VCL_OPENGL_CONTEXT_HXX
#define VCL_OPENGL_CONTEXT_HXX

#if defined( MACOSX )
#elif defined( UNX )
#  include <prex.h>
#  include "GL/glxew.h"
#  include <postx.h>
#elif defined( _WIN32 )
#  include "prewin.h"
#  include "windows.h"
#  include "postwin.h"
#endif

#include <GL/glew.h>

#if defined( _WIN32 )
#include <GL/glext.h>
#include <GL/wglext.h>
#elif defined( MACOSX )
#elif defined( UNX )
#include <GL/glext.h>
#define GLX_GLXEXT_PROTOTYPES 1
#include <GL/glx.h>
#include <GL/glxext.h>
#endif

#include <vcl/vclopengl_dllapi.hxx>
#include <boost/scoped_ptr.hpp>
#include <vcl/window.hxx>
#include <tools/gen.hxx>
#include <vcl/syschild.hxx>

/// Holds the information of our new child window
struct GLWindow
{
#if defined( _WIN32 )
    HWND                    hWnd;
    HDC                     hDC;
    HGLRC                   hRC;
#elif defined( MACOSX )
#elif defined( UNX )
    Display*           dpy;
    int                     screen;
    XLIB_Window             win;
#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )
    GLXFBConfig        fbc;
#endif
    XVisualInfo*       vi;
    GLXContext         ctx;

    bool HasGLXExtension( const char* name ) { return gluCheckExtension( (const GLubyte*) name, (const GLubyte*) GLXExtensions ); }
    const char*             GLXExtensions;
#endif
    unsigned int            bpp;
    unsigned int            Width;
    unsigned int            Height;
    const GLubyte*          GLExtensions;

    bool HasGLExtension( const char* name ) { return gluCheckExtension( (const GLubyte*) name, GLExtensions ); }

    GLWindow()
        :
#if defined( _WIN32 )
#elif defined( MACOSX )
#elif defined( UNX )
        dpy(NULL),
        screen(0),
        win(0),
#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )
        fbc(0),
#endif
        vi(NULL),
        ctx(0),
        GLXExtensions(NULL),
#endif
        bpp(0),
        Width(0),
        Height(0),
        GLExtensions(NULL)
    {
    }
};

class VCLOPENGL_DLLPUBLIC OpenGLContext
{
public:
    OpenGLContext();
    ~OpenGLContext();

    bool init();
    void setWinSize(const Size& rSize);
    GLWindow& getOpenGLWindow();

private:
    SAL_DLLPRIVATE bool initWindow();

    GLWindow m_aGLWin;
    boost::scoped_ptr<Window> m_pWindow;
    boost::scoped_ptr<SystemChildWindow> m_pChildWindow;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
