/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

struct COKit;

// Install the process-global file picker provider, so a command that needs the user to
// pick a file opens a QFileDialog instead of the engine's file dialog. Called once,
// while the engine's main loop starts.
void install_filepicker_provider(COKit& rOffice);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
