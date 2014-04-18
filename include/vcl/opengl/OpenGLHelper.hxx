/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_OPENGL_OPENGLHELPER_HXX
#define INCLUDED_VCL_OPENGL_OPENGLHELPER_HXX

#include <GL/glew.h>
#include <vcl/vclopengl_dllapi.hxx>
#include <vcl/bitmapex.hxx>

#include <rtl/ustring.hxx>

class VCLOPENGL_DLLPUBLIC OpenGLHelper
{
public:
    static GLint LoadShaders(const OUString& rVertexShaderName, const OUString& rFragmentShaderName);

    static sal_uInt8* ConvertBitmapExToRGBABuffer(const BitmapEx& rBitmapEx);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
