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
#include <vcl/sysdata.hxx>

class OpenGLWindowImpl
{
public:
    explicit OpenGLWindowImpl(vcl::Window* pWindow);
    ~OpenGLWindowImpl();
    OpenGLContext& getContext() { return *mxContext.get(); }
private:
    rtl::Reference<OpenGLContext> mxContext;
    VclPtr<SystemChildWindow> mxChildWindow;
};

OpenGLWindowImpl::OpenGLWindowImpl(vcl::Window* pWindow)
    : mxContext(OpenGLContext::Create())
{
    SystemWindowData aData = OpenGLContext::generateWinData(pWindow, false);
    mxChildWindow.reset(VclPtr<SystemChildWindow>::Create(pWindow, 0, &aData));
    mxChildWindow->Show();
    mxContext->init(mxChildWindow.get());
    pWindow->SetMouseTransparent(false);
}

OpenGLWindowImpl::~OpenGLWindowImpl()
{
    mxContext->dispose();
    mxChildWindow.disposeAndClear();
}

OpenGLWindow::OpenGLWindow(vcl::Window* pParent):
    Window(pParent, 0),
    mxImpl(new OpenGLWindowImpl(this)),
    mpRenderer(nullptr)
{
}

OpenGLWindow::~OpenGLWindow()
{
    disposeOnce();
}

void OpenGLWindow::dispose()
{
    if(mpRenderer)
        mpRenderer->contextDestroyed();
    mpRenderer = nullptr;
    mxImpl.reset();
    Window::dispose();
}

OpenGLContext& OpenGLWindow::getContext()
{
    return mxImpl->getContext();
}

void OpenGLWindow::Paint(vcl::RenderContext& /*rRenderContext*/, const Rectangle&)
{
    if(mpRenderer)
        mpRenderer->update();
}

void OpenGLWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    maStartPoint = rMEvt.GetPosPixel();
}

void OpenGLWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    if(!mpRenderer)
        return;

    Point aPoint = rMEvt.GetPosPixel();
    if(aPoint == maStartPoint)
    {
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
    if(!mpRenderer)
        return;

    if(rCEvt.GetCommand() == CommandEventId::Wheel)
    {
        const CommandWheelData* pData = rCEvt.GetWheelData();
        if(pData->GetMode() == CommandWheelMode::SCROLL)
        {
            long nDelta = pData->GetDelta();
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
