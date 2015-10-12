/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Fully wrapped methods that have no exotic GL / GLEW header deps.

#ifndef INCLUDED_VCL_OPENGL_OPENGLWRAPPER_HXX
#define INCLUDED_VCL_OPENGL_OPENGLWRAPPER_HXX

#include <vcl/dllapi.h>

// All member functions static and VCL_DLLPUBLIC. Basically a glorified namespace.
struct VCL_DLLPUBLIC OpenGLWrapper
{
    OpenGLWrapper() = delete; // Should not be instantiated

    /**
     * Returns true if VCL has OpenGL rendering enabled
     */
    static bool isVCLOpenGLEnabled();
};

#endif // INCLUDED_VCL_OPENGL_OPENGLWRAPPER_HXX
