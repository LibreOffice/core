/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#define KIT_USE_UNSTABLE_API
#include <COKit/COKit.hxx>

#include <string>

extern int coolwsd_server_socket_fd;
extern const char *user_name;

// The location of the app .exe file. Note that currently that is the "program" subfolder of the top
// directory of the app.
extern std::string app_installation_path;

// The file: URI for the above.
extern std::string app_installation_uri;

extern void load_next_document();
void output_file_dialog_from_core(const char* suggestedURI, char* result, size_t resultLen);

// Open Explorer with the given file: URI's document selected.
void reveal_in_file_manager(const char* uri);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
