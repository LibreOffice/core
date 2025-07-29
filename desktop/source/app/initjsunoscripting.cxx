/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#if defined EMSCRIPTEN

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

#include <pthread.h>

#include <emscripten.h>
#include <emscripten/threading.h>
#include <emscripten/val.h>

#include <bindings_uno.hxx>
#include <config_emscripten.h>
#include <rtl/uri.hxx>
#include <rtl/ustring.hxx>

namespace {

extern "C" void getUnoScriptUrls(std::vector<std::u16string> * urls) {
    assert(urls != nullptr);
    OUString const base(emscripten::val::global("document")["baseURI"].as<std::u16string>());
    auto const val = emscripten::val::module_property("uno_scripts");
    if (!val.isUndefined()) {
        auto const len = val["length"].as<std::uint32_t>();
        for (std::uint32_t i = 0; i != len; ++i) {
            urls->push_back(
                std::u16string(
                    rtl::Uri::convertRelToAbs(base, OUString(val[i].as<std::u16string>()))));
        }
    }
}

#if HAVE_EMSCRIPTEN_PROXY_TO_PTHREAD
EM_JS(void, runUnoScriptUrls, (emscripten::EM_VAL handle), {
    globalThis.Module ||= Module;
    importScripts.apply(self, Emval.toValue(handle));
});
#else
EM_JS(void, runUnoScriptUrls, (emscripten::EM_VAL handle), {
    const urls = Emval.toValue(handle);
    function step() {
        if (urls.length !== 0) {
            const url = urls.shift();
            fetch(url).then(res => {
                if (!res.ok) {
                    throw Error(
                        "Loading <" + res.url + "> failed with " + res.status + " "
                        + res.statusText);
                }
                return res.blob();
            }).then(blob => blob.text()).then(text => { eval(text); step(); });
        }
    };
    step();
});
#endif

EM_JS(void, setupMainChannel, (), {
    const orig = self.onmessage;
    self.onmessage = function(e) {
        if (e.data.cmd === "LOWA-channel") {
            self.onmessage = orig;
            Module.uno_mainPort = e.ports[0];
            Module.uno_init$resolve();
        } else if (orig) {
            orig(e);
        }
    };
});

extern "C" void resolveUnoMain(pthread_t id) {
#if HAVE_EMSCRIPTEN_PROXY_TO_PTHREAD
    EM_ASM({
        const sofficeMain = PThread.pthreads[$0];
        const channel = new MessageChannel();
        sofficeMain.postMessage({cmd:"LOWA-channel"}, [channel.port2]);
        Module.uno_main$resolve(channel.port1);
    }, id);
#else
    EM_ASM({
        const channel = new MessageChannel();
        postMessage({cmd:"LOWA-channel"}, [channel.port2]);
        Module.uno_main$resolve(channel.port1);
    }, id);
#endif
}

}

void initJsUnoScripting() {
    init_unoembind_uno();
    std::vector<std::u16string> urls;
    emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VI, getUnoScriptUrls, &urls);
    runUnoScriptUrls(emscripten::val::array(urls).as_handle());
    setupMainChannel();
    emscripten_async_run_in_main_runtime_thread(EM_FUNC_SIG_VI, resolveUnoMain, pthread_self());
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
