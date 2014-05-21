/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_UNX_X11_X11OPENGLCONTEXT_HXX
#define INCLUDED_VCL_INC_UNX_X11_X11OPENGLCONTEXT_HXX

#include <string.h>

#include <boost/noncopyable.hpp>

#include <GL/glew.h>

#include <prex.h>
#include "GL/glxew.h"
#include <postx.h>

#include <GL/glext.h>
#define GLX_GLXEXT_PROTOTYPES 1
#include <GL/glx.h>
#include <GL/glxext.h>

#include <boost/scoped_ptr.hpp>
#include <vcl/opengl/IOpenGLContext.hxx>
#include <vcl/window.hxx>
#include <tools/gen.hxx>
#include <vcl/syschild.hxx>

class X11SalInstance;

class X11OpenGLContext : public vcl::IOpenGLContext, private boost::noncopyable
{
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

        Display*           dpy;
        int                     screen;
        XLIB_Window             win;
#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )
        GLXFBConfig        fbc;
#endif
        XVisualInfo*       vi;
        GLXContext         ctx;

        bool HasGLXExtension( const char* name ) const { return checkExtension( (const GLubyte*) name, (const GLubyte*) GLXExtensions ); }
        const char*             GLXExtensions;
        unsigned int            bpp;
        unsigned int            Width;
        unsigned int            Height;
        const GLubyte*          GLExtensions;
        bool bMultiSampleSupported;

        bool HasGLExtension( const char* name ) const { return checkExtension( (const GLubyte*) name, GLExtensions ); }

        GLWindow()
            :
            dpy(NULL),
            screen(0),
            win(0),
#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )
            fbc(0),
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

public:
    X11OpenGLContext();
    virtual ~X11OpenGLContext() SAL_OVERRIDE;

    virtual bool init(Window* pParent = 0) SAL_OVERRIDE;
    virtual bool init(SystemChildWindow* pChildWindow) SAL_OVERRIDE;

    virtual void makeCurrent() SAL_OVERRIDE;
    virtual void swapBuffers() SAL_OVERRIDE;
    virtual void sync() SAL_OVERRIDE;
    virtual void show() SAL_OVERRIDE;

    virtual void setWinPosAndSize(const Point &rPos, const Size& rSize) SAL_OVERRIDE;
    virtual void setWinSize(const Size& rSize) SAL_OVERRIDE;

    virtual void renderToFile() SAL_OVERRIDE;

    virtual bool isInitialized() const SAL_OVERRIDE;

    virtual bool hasGLExtension(const char *pName) const SAL_OVERRIDE;

    virtual unsigned getWidth() const SAL_OVERRIDE;
    virtual unsigned getHeight() const SAL_OVERRIDE;

private:
    bool initWindow();
    bool ImplInit();

    GLWindow m_aGLWin;
    boost::scoped_ptr<Window> m_pWindow;
    Window* mpWindow; //points to m_pWindow or the parent window, don't delete it
    SystemChildWindow* m_pChildWindow;
    boost::scoped_ptr<SystemChildWindow> m_pChildWindowGC;
    bool mbInitialized;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
