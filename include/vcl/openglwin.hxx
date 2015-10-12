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
#include <vcl/dllapi.h>

class OpenGLContext;
class OpenGLWindowImpl;

class VCL_DLLPUBLIC IRenderer
{
public:
    virtual ~IRenderer() {}
    virtual void update() = 0;
    virtual void clickedAt(const Point& rPos, sal_uInt16 nButtons) = 0;
    virtual void mouseDragMove(const Point& rPosBegin, const Point& rPosEnd, sal_uInt16 nButtons) = 0;
    virtual void scroll(long nDelta) = 0;

    virtual void contextDestroyed() = 0;
};

// pImpl Pattern to avoid linking against OpenGL libs when using the class without the context
class VCL_DLLPUBLIC OpenGLWindow : public vcl::Window
{
public:
                   OpenGLWindow(vcl::Window* pParent);
    virtual        ~OpenGLWindow();
    virtual void   dispose() override;

    OpenGLContext& getContext();

    void setRenderer(IRenderer* pRenderer);

    virtual void Paint(vcl::RenderContext& rRenderContext, const Rectangle&) override;

    virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void MouseMove( const MouseEvent& rMEvt ) override;
    virtual void Command( const CommandEvent& rCEvt ) override;

private:
    std::unique_ptr<OpenGLWindowImpl> mxImpl;
    IRenderer* mpRenderer;

    Point maStartPoint;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
