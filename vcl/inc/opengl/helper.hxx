/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_OPENGL_HELPER_HXX
#define INCLUDED_VCL_INC_OPENGL_HELPER_HXX

#include <sstream>

#include <sal/log.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <vcl/opengl/OpenGLHelper.hxx>

/// Helper to do a SAL_INFO as well as a GL log.
#define VCL_GL_INFO(stream) \
    do { \
        if (SAL_DETAIL_ENABLE_LOG_INFO && OpenGLHelper::isVCLOpenGLEnabled()) \
        { \
            std::ostringstream detail_stream; \
            detail_stream << stream;            \
            OpenGLHelper::debugMsgStream(detail_stream); \
        } \
    } while (false)

namespace LocalOpenGLHelper
{
    OString GetDigest(const OUString& rVertexShaderName, const OUString& rFragmentShaderName, const OString& preamble = "" );
}

#endif // INCLUDED_VCL_INC_OPENGL_HELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
