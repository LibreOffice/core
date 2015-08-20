/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_OPENGL_GUARD_H
#define INCLUDED_VCL_INC_OPENGL_GUARD_H

#include <sal/config.h>

/**
 * We want to be able to detect if a given crash came
 * from the OpenGL code, so use this helper to track that.
 */
class OpenGLSalGraphicsImpl;
class OpenGLZone {
    static int gnInOpenGLZone;
    friend class OpenGLSalGraphicsImpl;
    static void enter() { gnInOpenGLZone++; }
    static void leave() { gnInOpenGLZone--; }
public:
     OpenGLZone() { enter(); }
    ~OpenGLZone() { leave(); }
    static bool isInZone() { return gnInOpenGLZone > 0; }
    static void hardDisable();
};

#endif // INCLUDED_VCL_INC_OPENGL_PROGRAM_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
