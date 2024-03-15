/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/dllapi.h>
#include <vcl/sysdata.hxx>
#include <vcl/vclptr.hxx>
#include <rtl/ref.hxx>

class Point;
class Size;
class SystemChildWindow;
namespace vcl { class Window; }

/// Holds the information of our new child window
struct VCL_DLLPUBLIC GLWindow
{
    unsigned int            Width;
    unsigned int            Height;
    bool bMultiSampleSupported;

    GLWindow()
        : Width(0)
        , Height(0)
        , bMultiSampleSupported(false)
    {
    }

    virtual bool Synchronize(bool bOnoff) const;

    virtual ~GLWindow();
};

class VCL_DLLPUBLIC OpenGLContext
{
    friend class OpenGLTests;
protected:
    OpenGLContext();
public:
    static rtl::Reference<OpenGLContext> Create();
    virtual ~OpenGLContext();

    // Avoid implicitly defined copy constructors/assignments for the DLLPUBLIC class (they may
    // require forward-declared classes used internally to be defined in places using OpenGLContext)
    OpenGLContext(const OpenGLContext&) = delete;
    OpenGLContext(OpenGLContext&&) = delete;
    OpenGLContext& operator=(const OpenGLContext&) = delete;
    OpenGLContext& operator=(OpenGLContext&&) = delete;

    void acquire() { mnRefCount++; }
    void release() { if ( --mnRefCount == 0 ) delete this; }
    void dispose();

    void requestLegacyContext();

    bool init(vcl::Window* pParent);

    void reset();

    /// Is this GL context the current context ?
    virtual bool isCurrent();
    /// Is any GL context the current context ?
    virtual bool isAnyCurrent();
    /// release bound resources from the current context
    static void clearCurrent();
    /// release contexts etc. before (potentially) allowing another thread run.
    SAL_DLLPRIVATE static void prepareForYield();
    /// Is there a current GL context ?
    SAL_DLLPRIVATE static bool hasCurrent();

    /// make this GL context current - so it is implicit in subsequent GL calls
    SAL_DLLPRIVATE virtual void makeCurrent();
    /// Put this GL context to the end of the context list.
    void registerAsCurrent();
    /// reset the GL context so this context is not implicit in subsequent GL calls.
    SAL_DLLPRIVATE virtual void resetCurrent();
    /// unbind the GL_FRAMEBUFFER to its default state, needed for gtk3
    virtual void restoreDefaultFramebuffer();
    SAL_DLLPRIVATE virtual void swapBuffers();
    virtual void sync();
    void show();

    void setWinPosAndSize(const Point &rPos, const Size& rSize);
    virtual const GLWindow& getOpenGLWindow() const = 0;

    SystemChildWindow* getChildWindow();
    SAL_DLLPRIVATE const SystemChildWindow* getChildWindow() const;

    bool isInitialized() const
    {
        return mbInitialized;
    }

    virtual SystemWindowData generateWinData(vcl::Window* pParent, bool bRequestLegacyContext);

private:
    SAL_DLLPRIVATE virtual void initWindow();
    SAL_DLLPRIVATE virtual void destroyCurrentContext();
    virtual void adjustToNewSize();

protected:
    bool InitGL();
    static void InitGLDebugging();
    static void InitChildWindow(SystemChildWindow *pChildWindow);
    static void BuffersSwapped();
    virtual GLWindow& getModifiableOpenGLWindow() = 0;
    SAL_DLLPRIVATE virtual bool ImplInit();

    VclPtr<vcl::Window> m_xWindow;
    VclPtr<vcl::Window> mpWindow; //points to m_pWindow or the parent window, don't delete it
    VclPtr<SystemChildWindow> m_pChildWindow;
    bool mbInitialized;
    int  mnRefCount;
    bool mbRequestLegacyContext;

public:

    // Don't hold references to ourselves:
    OpenGLContext *mpPrevContext;
    OpenGLContext *mpNextContext;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
