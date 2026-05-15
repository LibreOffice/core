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

#include <jsuno/detail/dllapi.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

namespace jsuno
{
// Run `script` in a JS UNO context and return the JSON-stringified result (empty string for
// values JSON.stringify drops: `undefined`, a function, or a symbol).
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
// @throws css.script.provider.ScriptExceptionRaisedException
LO_DLLPUBLIC_JSUNO OUString execute(OUString const& script,
                                    std::function<void(OUString const&)> proxyCallHook = {});

// Deliver an iframe-side `jsonValue` to a `ProxyInvocation::invoke` blocked in Application::
// Yield on `callId`; spurious callIds are silently ignored:
LO_DLLPUBLIC_JSUNO void deliverProxyResult(OUString const& callId, OUString const& jsonValue);

// Unblock every spinning ProxyInvocation::invoke by marking each pending call as delivered
// with an empty JSON value, so a torn-down iframe cannot leave the kit waiting forever:
LO_DLLPUBLIC_JSUNO void cancelProxyCalls();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
