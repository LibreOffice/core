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

#include <config_emscripten.h>
#include <config_vclplug.h>

#if defined EMSCRIPTEN && ENABLE_QT6 && HAVE_EMSCRIPTEN_JSPI && !HAVE_EMSCRIPTEN_PROXY_TO_PTHREAD

#include <thread>

#include <emscripten/proxying.h>

namespace comphelper::emscriptenthreading
{
struct Data
{
    emscripten::ProxyingQueue proxyingQueue;
    std::thread::native_handle_type eventHandlerThread;
};

Data& getData();

void tearDown();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
