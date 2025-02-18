/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <comphelper/emscriptenthreading.hxx>
#include <config_emscripten.h>
#include <config_vclplug.h>

#if defined EMSCRIPTEN && ENABLE_QT6 && HAVE_EMSCRIPTEN_JSPI && !HAVE_EMSCRIPTEN_PROXY_TO_PTHREAD

#include <cassert>
#include <mutex>
#include <thread>

namespace
{
std::mutex mutex;
comphelper::emscriptenthreading::Data* data = nullptr;
}

void comphelper::emscriptenthreading::setUp()
{
    std::scoped_lock g(mutex);
    assert(data == nullptr);
    data = new Data;
    std::thread t([] { emscripten_exit_with_live_runtime(); });
    data->eventHandlerThread = t.native_handle();
    t.detach();
}

void comphelper::emscriptenthreading::tearDown()
{
    std::scoped_lock g(mutex);
    assert(data != nullptr);
    delete data;
    data = nullptr;
}

comphelper::emscriptenthreading::Data& comphelper::emscriptenthreading::getData()
{
    std::scoped_lock g(mutex);
    assert(data != nullptr);
    return *data;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
