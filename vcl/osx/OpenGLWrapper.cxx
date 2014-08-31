/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "OpenGLWrapper.hxx"

void OpenGLWrapper::swapBuffers(NSOpenGLView* pView)
{
    [[pView openGLContext] flushBuffer];
}

void OpenGLWrapper::makeCurrent(NSOpenGLView* pView)
{
    [[pView openGLContext] makeCurrentContext];
}

void OpenGLWrapper::resetCurrent()
{
    [NSOpenGLContext clearCurrentContext];
}

void OpenGLWrapper::init(NSOpenGLView* pView)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
