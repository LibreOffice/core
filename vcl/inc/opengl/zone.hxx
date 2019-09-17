/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_OPENGL_ZONE_H
#define INCLUDED_VCL_INC_OPENGL_ZONE_H

#include <sal/config.h>
#include <sal/types.h>
#include <vcl/dllapi.h>

class OpenGLWatchdogThread;

/**
 * We want to be able to detect if a given crash came
 * from the OpenGL code, so use this helper to track that.
 */
class VCL_DLLPUBLIC OpenGLZone {
    friend class OpenGLWatchdogThread;
    friend class OpenGLSalGraphicsImpl;

    /// how many times have we entered a GL zone
    static volatile sal_uInt64 gnEnterCount;
    /// how many times have we left a new GL zone
    static volatile sal_uInt64 gnLeaveCount;

    static void enter() { gnEnterCount++; }
    static void leave() { gnLeaveCount++; }
public:
     OpenGLZone() { enter(); }
    ~OpenGLZone() { leave(); }
    static bool isInZone() { return gnEnterCount != gnLeaveCount; }
    static void hardDisable();
    static void relaxWatchdogTimings();
};

/// Create this to not only enter the zone, but set VCL context.
class OpenGLVCLContextZone {
    OpenGLZone const aZone;
public:
    OpenGLVCLContextZone();
};

class VCL_DLLPUBLIC PreDefaultWinNoOpenGLZone {
public:
    PreDefaultWinNoOpenGLZone();
    ~PreDefaultWinNoOpenGLZone();
};

#endif // INCLUDED_VCL_INC_OPENGL_ZONE_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
