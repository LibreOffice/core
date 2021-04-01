/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <Qt5OpenGLContext.hxx>

#include <epoxy/gl.h>

#include <vcl/sysdata.hxx>
#include <opengl/zone.hxx>
#include <sal/log.hxx>

#include <window.h>

#include <Qt5Object.hxx>

#include <QtGui/QOpenGLContext>
#include <QtGui/QWindow>

bool Qt5OpenGLContext::g_bAnyCurrent = false;

void Qt5OpenGLContext::swapBuffers()
{
    OpenGLZone aZone;

    if (m_pContext && m_pWindow && m_pWindow->isExposed())
    {
        m_pContext->swapBuffers(m_pWindow);
    }

    BuffersSwapped();
}

void Qt5OpenGLContext::resetCurrent()
{
    clearCurrent();

    OpenGLZone aZone;

    if (m_pContext)
    {
        m_pContext->doneCurrent();
        g_bAnyCurrent = false;
    }
}

bool Qt5OpenGLContext::isCurrent()
{
    OpenGLZone aZone;
    return g_bAnyCurrent && (QOpenGLContext::currentContext() == m_pContext);
}

bool Qt5OpenGLContext::isAnyCurrent()
{
    OpenGLZone aZone;
    return g_bAnyCurrent && (QOpenGLContext::currentContext() != nullptr);
}

bool Qt5OpenGLContext::ImplInit()
{
    if (!m_pWindow)
    {
        SAL_WARN("vcl.opengl.qt5", "failed to create window");
        return false;
    }

    m_pWindow->setSurfaceType(QSurface::OpenGLSurface);
    m_pWindow->create();

    m_pContext = new QOpenGLContext(m_pWindow);
    if (!m_pContext->create())
    {
        SAL_WARN("vcl.opengl.qt5", "failed to create context");
        return false;
    }

    m_pContext->makeCurrent(m_pWindow);
    g_bAnyCurrent = true;

    bool bRet = InitGL();
    InitGLDebugging();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    registerAsCurrent();

    return bRet;
}

void Qt5OpenGLContext::makeCurrent()
{
    if (isCurrent())
        return;

    OpenGLZone aZone;

    clearCurrent();

    if (m_pContext && m_pWindow)
    {
        m_pContext->makeCurrent(m_pWindow);
        g_bAnyCurrent = true;
    }

    registerAsCurrent();
}

void Qt5OpenGLContext::destroyCurrentContext()
{
    OpenGLZone aZone;

    if (m_pContext)
    {
        m_pContext->doneCurrent();
        g_bAnyCurrent = false;
    }

    if (glGetError() != GL_NO_ERROR)
    {
        SAL_WARN("vcl.opengl.qt5", "glError: " << glGetError());
    }
}

void Qt5OpenGLContext::initWindow()
{
    if (!m_pChildWindow)
    {
        SystemWindowData winData = generateWinData(mpWindow, mbRequestLegacyContext);
        m_pChildWindow = VclPtr<SystemChildWindow>::Create(mpWindow, 0, &winData, false);
    }

    if (m_pChildWindow)
    {
        InitChildWindow(m_pChildWindow.get());
    }

    m_pWindow
        = static_cast<Qt5Object*>(m_pChildWindow->ImplGetWindowImpl()->mpSysObj)->windowHandle();
}
