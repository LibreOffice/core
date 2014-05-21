/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_OPENGL_IOPENGLCONTEXT_HXX
#define INCLUDED_VCL_OPENGL_IOPENGLCONTEXT_HXX

#include <vcl/vclopengl_dllapi.hxx>

class Point;
class Size;
class SystemChildWindow;
class SystemWindowData;
class Window;

namespace vcl
{

class VCLOPENGL_DLLPUBLIC IOpenGLContext
{
public:
    virtual ~IOpenGLContext() = 0;

    virtual bool init(Window* pParent = 0) = 0;
    virtual bool init(SystemChildWindow* pChildWindow) = 0;

    virtual void makeCurrent() = 0;
    virtual void swapBuffers() = 0;
    virtual void sync() = 0;
    virtual void show() = 0;

    virtual void setWinPosAndSize(const Point &rPos, const Size& rSize) = 0;
    virtual void setWinSize(const Size& rSize) = 0;

    virtual void renderToFile() = 0;

    virtual bool isInitialized() const = 0;

    virtual bool hasGLExtension(const char *pName) const = 0;

    virtual unsigned getWidth() const = 0;
    virtual unsigned getHeight() const = 0;
};

/** Create a platform-specific IOpenGLContext.
  */
VCLOPENGL_DLLPUBLIC IOpenGLContext* createOpenGLContext();

/** Create and initialize a platform-specific IOpenGLContext.
  */
VCLOPENGL_DLLPUBLIC IOpenGLContext* createOpenGLContextFor(Window* pParent);

/** Create and initialize a platform-specific IOpenGLContext.
  */
VCLOPENGL_DLLPUBLIC IOpenGLContext* createOpenGLContextFor(SystemChildWindow* pChildWindow);

VCLOPENGL_DLLPUBLIC SystemWindowData generateSystemWindowData(Window* pParent);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
