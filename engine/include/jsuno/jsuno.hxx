/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <functional>
#include <string>
#include <string_view>
#include <vector>

#include <jsuno/detail/dllapi.hxx>

namespace jsuno
{
struct LO_DLLPUBLIC_JSUNO Exception {
    struct Frame {
        std::string source;
        std::string line;
        std::string column;
        std::string functionName;
    };

    Exception() = default;
    Exception(Exception const &) = default;
    Exception & operator =(Exception const &) = default;
    virtual ~Exception();

    std::string name;
    std::string message;
    std::vector<Frame> stack;
};

struct Result {
    std::string json;
    bool usedLegacyUnoApi;
};

// Run `prelude` and then `script` (for both of which the last element must be a terminating NUL) in
// one JS UNO
// context, and return the JSON-stringified result of `script` (empty string for values
// JSON.stringify drops: `undefined`, a function, or a symbol).
//
// All strings passed in and out are UTF-8.
//
// `source` and (1-based) `line` are used for any exception stack frames in `script` that are
// reported back; frames in `prelude` name the source "<prelude>" and count its lines from 1.
//
// `consoleHook` receives every call the script makes on the global `console` object.  jsuno
// registers the WHATWG Console API methods `log`, `info`, `warn`, `error`, `debug`, `trace`,
// and `assert` on that object; each dispatches to the hook with `level` set to the method name
// and `message` set to the stringified-and-space-joined arguments.  `assert` only fires on
// failed assertion.  `warn`, `error`, `trace`, and `assert` prepend the JS stack to the message.
//The hook is called synchronously on the JS thread; it must not itself call back into jsuno.
//
// `proxyCallHook`, if non-empty, is captured by every JS-UNO proxy listener stub created
// during this call and fires (from any thread, possibly after execute() has returned) when
// the wrapped C++ stub later receives a UNO call, with a JSON payload of the form
//
//     { "proxyId": "...", "callId": "...", "method": "...", "args": [...] }
//
// where `callId` is present iff the proxy's invoke is going to wait for a value back via
// deliverProxyResult (i.e. the listener method has a non-void return type).
//
// @throws jsuno::Exception
LO_DLLPUBLIC_JSUNO Result execute(
    std::u8string_view prelude, std::string_view script, std::string_view source, int line,
    std::function<void(std::string_view level, std::string_view message)> consoleHook,
    std::function<void(std::string_view payload)> proxyCallHook);

// Deliver an iframe-side `jsonValue` to a `ProxyInvocation::invoke` blocked in Application::
// Yield on `callId`; spurious callIds are silently ignored:
LO_DLLPUBLIC_JSUNO void deliverProxyResult(std::string_view callId, std::string_view jsonValue);

// Unblock the ProxyInvocation::invoke spinning on `callId` by marking the call as delivered with an
// empty JSON value; spurious callIds are silently ignored:
LO_DLLPUBLIC_JSUNO void cancelProxyCall(std::string_view callId);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
