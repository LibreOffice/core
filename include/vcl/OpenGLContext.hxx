/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef VCL_OPENGL_CONTEXT_HXX
#define VCL_OPENGL_CONTEXT_HXX

#include <vcl/OpenGLRender.hxx>

class VCL_DLLPUBLIC OpenGLContext
{
public:
    bool init(OpenGLRender& rGLRender);
    void setWinSize(const Size& rSize);

private:
    SAL_DLLPRIVATE bool initWindow();

    GLWindow m_aGLWin;
    boost::scoped_ptr<Window> m_pWindow;
    boost::scoped_ptr<SystemChildWindow> m_pChildWindow;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
