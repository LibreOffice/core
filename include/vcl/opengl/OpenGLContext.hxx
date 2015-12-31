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
#include <GL/wglew.h>
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
#endif

#include <vcl/dllapi.h>
#include <vcl/window.hxx>
#include <tools/gen.hxx>
#include <vcl/syschild.hxx>
#include <rtl/crc.h>
#include <rtl/ref.hxx>

#include <map>
#include <memory>
#include <set>
#include <unordered_map>

class OpenGLFramebuffer;
class OpenGLProgram;
class OpenGLTexture;
class SalGraphicsImpl;
class OpenGLTests;

/// Holds the information of our new child window
struct GLWindow
{
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
#if defined( GLX_EXT_texture_from_pixmap )
    GLXFBConfig        fbc;
#endif
    XVisualInfo*       vi;
    GLXContext         ctx;

    bool HasGLXExtension( const char* name ) const;
    const char*             GLXExtensions;
#endif
    unsigned int            Width;
    unsigned int            Height;
    const GLubyte*          GLExtensions;
    bool bMultiSampleSupported;

    GLWindow()
        :
#if defined( _WIN32 )
        hWnd(NULL),
        hDC(NULL),
        hRC(NULL),
#elif defined( MACOSX )
#elif defined( IOS )
#elif defined( ANDROID )
#elif defined( LIBO_HEADLESS )
#elif defined( UNX )
        dpy(nullptr),
        screen(0),
        win(0),
#if defined( GLX_EXT_texture_from_pixmap )
        fbc(nullptr),
#endif
        vi(nullptr),
        ctx(nullptr),
        GLXExtensions(nullptr),
#endif
        Width(0),
        Height(0),
        GLExtensions(nullptr),
        bMultiSampleSupported(false)
    {
    }

    ~GLWindow();
};

class VCL_DLLPUBLIC OpenGLContext
{
    friend class OpenGLTests;
    OpenGLContext();
public:
    static rtl::Reference<OpenGLContext> Create();
    ~OpenGLContext();
    void acquire() { mnRefCount++; }
    void release() { if ( --mnRefCount == 0 ) delete this; }
    void dispose();

    void requestLegacyContext();
    void requestSingleBufferedRendering();

    bool init(vcl::Window* pParent = nullptr);
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
    void UnbindTextureFromFramebuffers( GLuint nTexture );
    static bool        IsTextureAttachedAnywhere( GLuint nTexture );

    void               ReleaseFramebuffer( const OpenGLTexture& rTexture );
    void               ReleaseFramebuffers();

    // retrieve a program from the cache or compile/link it
    OpenGLProgram*      GetProgram( const OUString& rVertexShader, const OUString& rFragmentShader, const OString& preamble = "" );
    OpenGLProgram*      UseProgram( const OUString& rVertexShader, const OUString& rFragmentShader, const OString& preamble = "" );
    void                UseNoProgram();

    /// Is this GL context the current context ?
    bool isCurrent();
    /// release bound resources from the current context
    static void clearCurrent();
    /// release contexts etc. before (potentially) allowing another thread run.
    static void prepareForYield();
    /// Is there a current GL context ?
    static bool hasCurrent();
    /// make this GL context current - so it is implicit in subsequent GL calls
    void makeCurrent();
    /// Put this GL context to the end of the context list.
    void registerAsCurrent();
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

    /// VCL promiscuously re-uses its own contexts:
    void setVCLOnly() { mbVCLOnly = true; }
    bool isVCLOnly() { return mbVCLOnly; }

    bool supportMultiSampling() const;

    static SystemWindowData generateWinData(vcl::Window* pParent, bool bRequestLegacyContext);

private:
    SAL_DLLPRIVATE bool InitGLEW();
    SAL_DLLPRIVATE void InitGLEWDebugging();
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
    bool mbVCLOnly;

    int mnFramebufferCount;
    OpenGLFramebuffer* mpCurrentFramebuffer;
    OpenGLFramebuffer* mpFirstFramebuffer;
    OpenGLFramebuffer* mpLastFramebuffer;

    struct ProgramHash
    {
        size_t operator()( const rtl::OString& aDigest ) const
        {
            return (size_t)( rtl_crc32( 0, aDigest.getStr(), aDigest.getLength() ) );
        }
    };

    typedef std::unordered_map< rtl::OString, std::shared_ptr<OpenGLProgram>, ProgramHash > ProgramCollection;
    ProgramCollection maPrograms;
    OpenGLProgram* mpCurrentProgram;

public:
    vcl::Region maClipRegion;
    int mnPainting;

    // Don't hold references to ourselves:
    OpenGLContext *mpPrevContext;
    OpenGLContext *mpNextContext;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
