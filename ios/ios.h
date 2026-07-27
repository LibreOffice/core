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

#define LIBO_INTERNAL_ONLY
#include <COKit/COKit.hxx>

extern const char *user_name;

extern int coolwsd_server_socket_fd;

extern COKit *lo_kit;

/* Install the engine-driven clipboard provider: the engine advertises formats on copy and reads
   the system pasteboard on paste through it, one shared clipboard for every open document. */
void install_clipboard_provider(COKit &rOffice);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
