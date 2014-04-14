/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef VCL_OPENGLHELPERS_HXX
#define VCL_OPENGLHELPERS_HXX

#include <GL/glew.h>
#include <vcl/vclopengl_dllapi.hxx>

#include <rtl/ustring.hxx>

class VCLOPENGL_DLLPUBLIC OpenGLHelper
{
public:
    static GLint LoadShaders(const OUString& rVertexShaderName, const OUString& rFragmentShaderName);

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
