/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>
#include <config_features.h>

#if defined(UNX) && !defined MACOSX && !defined IOS && !defined ANDROID && HAVE_FEATURE_UI         \
    && HAVE_FEATURE_OPENGL
/* Run test for OpenGL support in own process to avoid crash with broken
 * OpenGL drivers. Start process as early as possible.
 * In non-headless mode, the process will be reaped in X11OpenGLDeviceInfo::GetData
 * (vcl/opengl/x11/X11DeviceInfo.cxx).  In headless mode, the process will be reaped late in
 * Desktop::Main (desktop/source/app/app.cxx).
 */

bool fire_glxtest_process();

void reap_glxtest_process();

#else

inline bool fire_glxtest_process() { return true; }

inline void reap_glxtest_process() {}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
