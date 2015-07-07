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
#elif defined( LIBO_HEADLESS )
#elif defined( UNX )
#  include <prex.h>
#  include "GL/glxew.h"
#  include <postx.h>
#elif defined( _WIN32 )
#ifndef INCLUDED_PRE_POST_WIN_H
#define INCLUDED_PRE_POST_WIN_H
#  include "prewin.h"
#  include "postwin.h"
#endif
#endif

#if defined( _WIN32 )
#include <GL/glext.h>
#include <GL/wglew.h>
#include <GL/wglext.h>
#elif defined( MACOSX )
#include <OpenGL/OpenGL.h>
#ifdef __OBJC__
@class NSOpenGLView;
#else
class NSOpenGLView;
#endif
#elif defined( IOS )
#elif defined( ANDROID )
#elif defined( LIBO_HEADLESS )
#elif defined( UNX )
#include <GL/glext.h>
#define GLX_GLXEXT_PROTOTYPES 1
#include <GL/glx.h>
#include <GL/glxext.h>
#endif

#include <vcl/dllapi.h>
#include <boost/ptr_container/ptr_map.hpp>
#include <vcl/window.hxx>
#include <tools/gen.hxx>
#include <vcl/syschild.hxx>

#include <set>

class OpenGLFramebuffer;
class OpenGLProgram;
class OpenGLTexture;
class SalGraphicsImpl;

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

      deleteThis=lookHere=static_cast<char*>(malloc(strlen(reinterpret_cast<const char*>(extString))+1));
      if (lookHere==NULL)
      {
         return GL_FALSE;
      }

      /* strtok() will modify string, so copy it somewhere */
      strcpy(lookHere, reinterpret_cast<const char*>(extString));

      while ((word=strtok(lookHere, " "))!=NULL)
      {
         if (strcmp(word, reinterpret_cast<const char*>(extName))==0)
         {
            flag=GL_TRUE;
            break;
         }
         lookHere=NULL; /* get next token */
      }
      free(static_cast<void*>(deleteThis));

      return flag;
    }

#if defined( _WIN32 )
    HWND                    hWnd;
    HDC                     hDC;
    HGLRC                   hRC;
#elif defined( MACOSX )
#elif defined( IOS )
#elif defined( ANDROID )
#elif defined( LIBO_HEADLESS )
#elif defined( UNX )
    Display*            dpy;
    int                 screen;
    Window              win;
    Pixmap              pix;
#if defined( GLX_EXT_texture_from_pixmap )
    GLXFBConfig        fbc;
#endif
    XVisualInfo*       vi;
    GLXContext         ctx;
    GLXPixmap           glPix;

    bool HasGLXExtension( const char* name ) { return checkExtension( reinterpret_cast<const GLubyte*>(name), reinterpret_cast<const GLubyte*>(GLXExtensions) ); }
    const char*             GLXExtensions;
#endif
    unsigned int            bpp;
    unsigned int            Width;
    unsigned int            Height;
    const GLubyte*          GLExtensions;
    bool bMultiSampleSupported;

    GLWindow()
        :
#if defined( _WIN32 )
#elif defined( MACOSX )
#elif defined( IOS )
#elif defined( ANDROID )
#elif defined( LIBO_HEADLESS )
#elif defined( UNX )
        dpy(NULL),
        screen(0),
        win(0),
        pix(0),
#if defined( GLX_EXT_texture_from_pixmap )
        fbc(0),
#endif
        vi(NULL),
        ctx(0),
        glPix(0),
        GLXExtensions(NULL),
#endif
        bpp(0),
        Width(0),
        Height(0),
        GLExtensions(NULL),
        bMultiSampleSupported(false)
    {
    }

    ~GLWindow();
};

class VCL_DLLPUBLIC OpenGLContext
{
public:
    OpenGLContext();
    ~OpenGLContext();

    void requestLegacyContext();
    void requestSingleBufferedRendering();

    bool init(vcl::Window* pParent = 0);
    bool init(SystemChildWindow* pChildWindow);

// these methods are for the deep platform layer, don't use them in normal code
// only in vcl's platform code
#if defined( UNX ) && !defined MACOSX && !defined IOS && !defined ANDROID && !defined(LIBO_HEADLESS)
    bool init(Display* dpy, Window win, int screen);
#elif defined( _WIN32 )
    bool init( HDC hDC, HWND hWnd );
#endif
    void reset();

    // use these methods right after setting a context to make sure drawing happens
    // in the right FBO (default one is for onscreen painting)
    bool               BindFramebuffer( OpenGLFramebuffer* pFramebuffer );
    bool               AcquireDefaultFramebuffer();
    OpenGLFramebuffer* AcquireFramebuffer( const OpenGLTexture& rTexture );
    static void        ReleaseFramebuffer( OpenGLFramebuffer* pFramebuffer );
#ifdef DBG_UTIL
    void AddRef(SalGraphicsImpl*);
    void DeRef(SalGraphicsImpl*);
#else
    void AddRef();
    void DeRef();
#endif
    void               ReleaseFramebuffer( const OpenGLTexture& rTexture );
    void               ReleaseFramebuffers();

    // retrieve a program from the cache or compile/link it
    OpenGLProgram*      GetProgram( const OUString& rVertexShader, const OUString& rFragmentShader, const OString& preamble = "" );
    OpenGLProgram*      UseProgram( const OUString& rVertexShader, const OUString& rFragmentShader, const OString& preamble = "" );

    bool isCurrent();
    static void clearCurrent();

    /// make this GL context current - so it is implicit in subsequent GL calls
    void makeCurrent();
    /// reset the GL context so this context is not implicit in subsequent GL calls.
    void resetCurrent();
    void swapBuffers();
    void sync();
    void show();

    void setWinPosAndSize(const Point &rPos, const Size& rSize);
    void setWinSize(const Size& rSize);
    const GLWindow& getOpenGLWindow() const { return m_aGLWin;}

    SystemChildWindow* getChildWindow();
    const SystemChildWindow* getChildWindow() const;

    bool isInitialized()
    {
        return mbInitialized;
    }

    bool supportMultiSampling() const;

    static SystemWindowData generateWinData(vcl::Window* pParent, bool bRequestLegacyContext);

private:
    SAL_DLLPRIVATE bool InitGLEW();
    SAL_DLLPRIVATE bool initWindow();
    SAL_DLLPRIVATE bool ImplInit();
#if defined( UNX ) && !defined MACOSX && !defined IOS && !defined ANDROID && !defined(LIBO_HEADLESS)
    SAL_DLLPRIVATE void initGLWindow(Visual* pVisual);
#endif

#if defined(MACOSX)
    NSOpenGLView* getOpenGLView();
#endif

    GLWindow m_aGLWin;
    VclPtr<vcl::Window> m_xWindow;
    VclPtr<vcl::Window> mpWindow; //points to m_pWindow or the parent window, don't delete it
    VclPtr<SystemChildWindow> m_pChildWindow;
    bool mbInitialized;
    int  mnRefCount;
    bool mbRequestLegacyContext;
    bool mbUseDoubleBufferedRendering;
#if defined( UNX ) && !defined MACOSX && !defined IOS && !defined ANDROID && !defined(LIBO_HEADLESS)
    bool mbPixmap; // is a pixmap instead of a window
#endif

    int mnFramebufferCount;
    OpenGLFramebuffer* mpCurrentFramebuffer;
    OpenGLFramebuffer* mpFirstFramebuffer;
    OpenGLFramebuffer* mpLastFramebuffer;

    struct ProgramKey
    {
        ProgramKey( const OUString& vertexShader, const OUString& fragmentShader, const OString& preamble );
        bool operator< ( const ProgramKey& other ) const;
        OUString vertexShader;
        OUString fragmentShader;
        OString preamble;
    };
    std::map<ProgramKey, boost::shared_ptr<OpenGLProgram> > maPrograms;
    OpenGLProgram* mpCurrentProgram;
#ifdef DBG_UTIL
    std::set<SalGraphicsImpl*> maParents;
#endif

public:
    vcl::Region maClipRegion;
    int mnPainting;

    OpenGLContext* mpPrevContext;
    OpenGLContext* mpNextContext;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
