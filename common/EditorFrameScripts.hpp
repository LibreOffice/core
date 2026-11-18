/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string>

// The scripts that CODA runs in the editor frame, the iframe into which the integrator posts its
// form for coolwsd's editor page:
namespace EditorFrameScripts {

// The fragment of an editor frame that shows the server's own editor, and the reasons why a server
// does not allow client document loading (its client_document_loading.enable is off, or it is too
// old to say):
inline std::string const serverEditorFragment = "coda-server";
inline std::string const disabledReason = "disabled";
inline std::string const unsupportedReason = "unsupported";

// Replaces coolwsd's page in the editor frame with an empty document before it is parsed, unless
// the frame shows the server's own editor:
inline std::string const emptyCoolPageScript = R"JS(
if (window !== window.top && location.pathname.endsWith('/cool.html')
    && location.hash !== '#)JS" + serverEditorFragment + R"JS(') {
    document.open();
    document.close();
}
)JS";

// A JS function of the address of coolwsd's editor page, whose promise gives the reason why the
// server does not allow client document loading, or else null (also when the server cannot be
// asked, which leaves the outcome to the connection to its /co/collab):
inline std::string const clientDocumentLoadingRefusalScript = R"JS(
(editorUrl) => {
    // A coolwsd that does not say whether it allows client document loading is older than that
    // setting (so its administrator cannot switch client document loading off, even where it has
    // /co/collab):
    const editorPath = new URL(editorUrl).pathname;
    const root = editorPath.slice(0, Math.max(editorPath.indexOf('/browser/'), 0));
    return fetch(root + '/hosting/capabilities', { signal: AbortSignal.timeout(10000) })
        .then((response) => {
            if (!response.ok) {
                throw new Error('/hosting/capabilities returned ' + response.status);
            }
            return response.json();
        })
        .then((capabilities) => {
            if (capabilities?.hasClientDocumentLoadingSupport === true) {
                return null;
            } else if (capabilities?.hasClientDocumentLoadingSupport === false) {
                return ')JS" + disabledReason + R"JS(';
            } else {
                return ')JS" + unsupportedReason + R"JS(';
            }
        }, () => null);
}
)JS";

// A JS function of the address of coolwsd's editor page and the integrator's URL-encoded form body,
// which posts that form into the frame again for the server's own editor:
inline std::string const serverEditorFormScript = R"JS(
(editorUrl, formBody) => {
    const url = new URL(editorUrl);
    url.hash = ')JS" + serverEditorFragment + R"JS(';
    const form = document.createElement('form');
    form.method = 'POST';
    form.action = url.href;
    for (const [name, value] of new URLSearchParams(formBody)) {
        const input = document.createElement('input');
        input.type = 'hidden';
        input.name = name;
        input.value = value;
        form.append(input);
    }
    document.body.append(form);
    form.submit();
}
)JS";

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
