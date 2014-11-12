/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_OPENGL_CONTEXTPROVIDER_HXX
#define INCLUDED_VCL_INC_OPENGL_CONTEXTPROVIDER_HXX

#include "vclpluginapi.h"

#include <vcl/opengl/OpenGLContext.hxx>

class VCLPLUG_GEN_PUBLIC OpenGLContextProvider
{
public:
    virtual ~OpenGLContextProvider() {};

    /* Get the OpenGL context provided by this instance */
    virtual OpenGLContext* GetOpenGLContext() const = 0;
};

#endif // INCLUDED_VCL_INC_OPENGL_CONTEXTPROVIDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
