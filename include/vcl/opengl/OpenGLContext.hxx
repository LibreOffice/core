/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_OPENGL_OPENGLCONTEXT_HXX
#define INCLUDED_VCL_OPENGL_OPENGLCONTEXT_HXX

#include <string.h>

#include <GL/glew.h>

#if defined( MACOSX )
#elif defined( IOS )
#elif defined( ANDROID )
#elif defined( UNX )
#  include <prex.h>
#  include "GL/glxew.h"
#  include <postx.h>
#elif defined( _WIN32 )
#  include "prewin.h"
#  include "windows.h"
#  include "postwin.h"
#endif

#if defined( _WIN32 )
#include <GL/glext.h>
#include <GL/wglext.h>
#elif defined( MACOSX )
#elif defined( IOS )
#elif defined( ANDROID )
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
    // Copy of gluCheckExtension(), from the Apache-licensed
    // https://code.google.com/p/glues/source/browse/trunk/glues/source/glues_registry.c
    static GLboolean checkExtension(const GLubyte* extName, const GLubyte* extString)
    {
      GLboolean flag=GL_FALSE;
      char* word;
      char* lookHere;
      char* deleteThis;

      if (extString==NULL)
      {
         return GL_FALSE;
      }

      deleteThis=lookHere=(char*)malloc(strlen((const char*)extString)+1);
      if (lookHere==NULL)
      {
         return GL_FALSE;
      }

      /* strtok() will modify string, so copy it somewhere */
      strcpy(lookHere,(const char*)extString);

      while ((word=strtok(lookHere, " "))!=NULL)
      {
         if (strcmp(word,(const char*)extName)==0)
         {
            flag=GL_TRUE;
            break;
         }
         lookHere=NULL; /* get next token */
      }
      free((void*)deleteThis);

      return flag;
    }

#if defined( _WIN32 )
    HWND                    hWnd;
    HDC                     hDC;
    HGLRC                   hRC;
#elif defined( MACOSX )
#elif defined( IOS )
#elif defined( ANDROID )
#elif defined( UNX )
    Display*           dpy;
    int                     screen;
    XLIB_Window             win;
#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )
    GLXFBConfig        fbc;
#endif
    XVisualInfo*       vi;
    GLXContext         ctx;

    bool HasGLXExtension( const char* name ) { return checkExtension( (const GLubyte*) name, (const GLubyte*) GLXExtensions ); }
    const char*             GLXExtensions;
#endif
    unsigned int            bpp;
    unsigned int            Width;
    unsigned int            Height;
    const GLubyte*          GLExtensions;
    bool bMultiSampleSupported;

    bool HasGLExtension( const char* name ) { return checkExtension( (const GLubyte*) name, GLExtensions ); }

    GLWindow()
        :
#if defined( _WIN32 )
#elif defined( MACOSX )
#elif defined( IOS )
#elif defined( ANDROID )
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
        GLExtensions(NULL),
        bMultiSampleSupported(false)
    {
    }
};

class VCLOPENGL_DLLPUBLIC OpenGLContext
{
public:
    OpenGLContext();
    ~OpenGLContext();

    bool init(Window* pParent = 0);
    void setWinSize(const Size& rSize);
    GLWindow& getOpenGLWindow();

    void renderToFile();

    bool isInitialized()
    {
        return mbInitialized;
    }

    static SystemWindowData generateWinData(Window* pParent);

private:
    SAL_DLLPRIVATE bool initWindow();

    GLWindow m_aGLWin;
    boost::scoped_ptr<Window> m_pWindow;
    Window* mpWindow; //points to m_pWindow or the parent window, don't delete it
    boost::scoped_ptr<SystemChildWindow> m_pChildWindow;
    bool mbInitialized;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
