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

#include <COKit/COKit.hxx>

#include <string>

extern const char *user_name;

extern int coolwsd_server_socket_fd;

extern COKit *lo_kit;

/**
 * Get the own installation path.
 */
std::string getBundlePath();

/**
 * Get path of the Application Support Directory (app-specific data files that should be preserved between app launches and across updates).
 */
std::string getAppSupportURL();

/**
 * Get URL of a resource in the bundle.
 */
std::string getResourceURL(const char *name, const char *ext);

/**
 * Get (filelystem) path of a resource in the bundle.
 */
std::string getResourcePath(const char *name, const char *ext);

/**
 * Reveal (and select) a document, given as a file: URI, in Finder.
 */
void reveal_in_file_manager(const char *uri);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
