/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CANVAS_SOURCE_OPENGL_OGL_TOOLS_HXX
#define INCLUDED_CANVAS_SOURCE_OPENGL_OGL_TOOLS_HXX

#include <sal/config.h>
#include <GL/glew.h>


namespace oglcanvas
{
    struct TransformationPreserver
    {
        TransformationPreserver()
        { glPushMatrix(); }

        ~TransformationPreserver()
        { glPopMatrix(); }
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
