/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_OPENGLWIN_HXX
#define INCLUDED_VCL_OPENGLWIN_HXX

#include <vcl/event.hxx>
#include <vcl/syschild.hxx>
#include <vcl/vclopengl_dllapi.hxx>

#include <boost/scoped_ptr.hpp>

class OpenGLContext;
class OpenGLWindowImpl;

class VCLOPENGL_DLLPUBLIC IRenderer
{
public:
    virtual ~IRenderer() {}
    virtual void update() = 0;
    virtual void clickedAt(const Point& rPos) = 0;
    virtual void mouseDragMove(const Point& rPosBegin, const Point& rPosEnd, sal_uInt16 nButtons) = 0;
    virtual void scroll(long nDelta) = 0;

    virtual void contextDestroyed() = 0;
};

// pImpl Pattern to avoid linking against OpenGL libs when using the class without the context
class VCLOPENGL_DLLPUBLIC OpenGLWindow : public SystemChildWindow
{
public:
    OpenGLWindow(Window* pParent);
    virtual ~OpenGLWindow();
    OpenGLContext* getContext();

    void setRenderer(IRenderer* pRenderer);

    virtual void Paint(const Rectangle&) SAL_OVERRIDE;

    virtual void MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void MouseButtonUp( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void MouseMove( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void Command( const CommandEvent& rCEvt ) SAL_OVERRIDE;

private:
    boost::scoped_ptr<OpenGLWindowImpl> mpImpl;
    IRenderer* mpRenderer;

    Point maStartPoint;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
