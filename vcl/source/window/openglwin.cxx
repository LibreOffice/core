/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/openglwin.hxx>
#include <vcl/opengl/OpenGLContext.hxx>
#include <vcl/event.hxx>

class OpenGLWindowImpl
{
public:
    OpenGLWindowImpl(SystemChildWindow* pWindow);
    OpenGLContext* getContext();
private:
    OpenGLContext maContext;
};

OpenGLWindowImpl::OpenGLWindowImpl(SystemChildWindow* pWindow)
{
    maContext.init(pWindow);
}

OpenGLContext* OpenGLWindowImpl::getContext()
{
    return &maContext;
}

OpenGLWindow::OpenGLWindow(Window* pParent):
    SystemChildWindow(pParent, 0),
    mpImpl(new OpenGLWindowImpl(this))
{
}

OpenGLWindow::~OpenGLWindow()
{
}

OpenGLContext* OpenGLWindow::getContext()
{
    return mpImpl->getContext();
}

void OpenGLWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    Point aPoint = rMEvt.GetPosPixel();

    Color aColor = GetPixel(aPoint);
    SAL_WARN("vcl.opengl", aColor.GetColor());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
