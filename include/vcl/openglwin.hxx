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

#include <vcl/syschild.hxx>
#include <vcl/vclopengl_dllapi.hxx>

#include <boost/scoped_ptr.hpp>

class OpenGLContext;
class OpenGLWindowImpl;

// pImpl Pattern to avoid linking against OpenGL libs when using the class without the context
class VCLOPENGL_DLLPUBLIC OpenGLWindow : public SystemChildWindow
{
public:
    OpenGLWindow(Window* pParent);
    virtual ~OpenGLWindow();
    OpenGLContext* getContext();

private:
    boost::scoped_ptr<OpenGLWindowImpl> mpImpl;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
