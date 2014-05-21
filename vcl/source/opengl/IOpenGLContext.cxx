/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/opengl/IOpenGLContext.hxx>
#include <vcl/sysdata.hxx>

#include "salinst.hxx"
#include "svdata.hxx"

namespace vcl
{

IOpenGLContext::~IOpenGLContext()
{
}

IOpenGLContext* createOpenGLContext()
{
    return ImplGetSVData()->mpDefInst->CreateOpenGLContext();
}

IOpenGLContext* createOpenGLContextFor(Window* const pParent)
{
    IOpenGLContext* const pContext(createOpenGLContext());
    return pContext->init(pParent) ? pContext : 0;
}

IOpenGLContext* createOpenGLContextFor(SystemChildWindow* const pChildWindow)
{
    IOpenGLContext* const pContext(createOpenGLContext());
    return pContext->init(pChildWindow) ? pContext : 0;
}

SystemWindowData generateSystemWindowData(Window* const pParent)
{
    return ImplGetSVData()->mpDefInst->GenerateSystemWindowData(pParent);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
