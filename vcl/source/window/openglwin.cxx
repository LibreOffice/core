/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/openglwin.hxx>
#include <vcl/opengl/IOpenGLContext.hxx>
#include <vcl/event.hxx>

class OpenGLWindowImpl
{
public:
    OpenGLWindowImpl(SystemChildWindow* pWindow);
    vcl::IOpenGLContext* getContext();
private:
    boost::scoped_ptr<vcl::IOpenGLContext> mpContext;
};

OpenGLWindowImpl::OpenGLWindowImpl(SystemChildWindow* pWindow)
    : mpContext(vcl::createOpenGLContext())
{
    mpContext->init(pWindow);
    pWindow->SetMouseTransparent(false);
}

vcl::IOpenGLContext* OpenGLWindowImpl::getContext()
{
    return mpContext.get();
}

OpenGLWindow::OpenGLWindow(Window* pParent):
    SystemChildWindow(pParent, 0),
    mpImpl(new OpenGLWindowImpl(this)),
    mpRenderer(NULL)
{
}

OpenGLWindow::~OpenGLWindow()
{
}

vcl::IOpenGLContext* OpenGLWindow::getContext()
{
    return mpImpl->getContext();
}

void OpenGLWindow::Paint(const Rectangle&)
{
    if(mpRenderer)
        mpRenderer->update();
}

void OpenGLWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    Point aPoint = rMEvt.GetPosPixel();

    Color aColor = GetPixel(aPoint);
    SAL_WARN("vcl.opengl", aColor.GetColor());
    if(mpRenderer)
        mpRenderer->clickedAt(aPoint);
}

void OpenGLWindow::setRenderer(IRenderer* pRenderer)
{
    mpRenderer = pRenderer;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
