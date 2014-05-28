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
    OpenGLWindowImpl(Window* pWindow);
    OpenGLContext* getContext();
private:
    OpenGLContext maContext;
    boost::scoped_ptr<SystemChildWindow> mpChildWindow;
};

OpenGLWindowImpl::OpenGLWindowImpl(Window* pWindow):
    mpChildWindow(new SystemChildWindow(pWindow))
{
    maContext.init(mpChildWindow.get());
    pWindow->SetMouseTransparent(false);
    maContext.show();
}

OpenGLContext* OpenGLWindowImpl::getContext()
{
    return &maContext;
}

OpenGLWindow::OpenGLWindow(Window* pParent):
    Window(pParent, 0),
    mpImpl(new OpenGLWindowImpl(this)),
    mpRenderer(NULL)
{
}

OpenGLWindow::~OpenGLWindow()
{
    if(mpRenderer)
        mpRenderer->contextDestroyed();
}

OpenGLContext* OpenGLWindow::getContext()
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
    getContext()->show();
    maStartPoint = rMEvt.GetPosPixel();
}

void OpenGLWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    Point aPoint = rMEvt.GetPosPixel();
    if(aPoint == maStartPoint)
    {
        Color aColor = GetPixel(aPoint);
        SAL_WARN("vcl.opengl", aColor.GetColor());
        if(mpRenderer)
            mpRenderer->clickedAt(aPoint, rMEvt.GetButtons());
    }
    else
    {
        mpRenderer->mouseDragMove(maStartPoint, aPoint,
                                  rMEvt.GetButtons());
    }
}

void OpenGLWindow::Command( const CommandEvent& rCEvt )
{
    if(rCEvt.GetCommand() == COMMAND_WHEEL)
    {
        const CommandWheelData* pData = rCEvt.GetWheelData();
        if(pData->GetMode() == COMMAND_WHEEL_SCROLL)
        {
            long nDelta = pData->GetDelta();
            if(mpRenderer)
                mpRenderer->scroll(nDelta);
        }
    }
}

void OpenGLWindow::MouseMove( const MouseEvent& /*rMEvt*/ )
{
}

void OpenGLWindow::setRenderer(IRenderer* pRenderer)
{
    mpRenderer = pRenderer;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
