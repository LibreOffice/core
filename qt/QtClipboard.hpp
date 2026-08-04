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

struct COKit;

/// Connect the clipboard-ownership watcher. Call once on the GUI thread at
/// application startup, before the first document loads.
void initializeQtClipboard();

/// Install the process-global clipboard provider; called from the kit main
/// loop (kit/Kit.cpp). After this the engine advertises its formats on copy
/// and reads the system clipboard on paste through the provider callbacks,
/// using one shared clipboard for every document.
void install_clipboard_provider(COKit& rOffice);

/// Render the shared clipboard's lazy transferable into engine-held bytes
/// before the document `appDocId` is destroyed, so a paste after the document
/// closes still works. Call on the GUI thread from the document's BYE.
void flushClipboardOnDocClose(unsigned appDocId);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
