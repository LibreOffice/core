/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <Poco/Path.h>

#include <config.h>

namespace Extensions {

std::vector<std::pair<std::string, std::string>> enumerateGasScripts(Poco::Path const & dir);

// The code files of the Apps Script project in dir with their sources, in the order in which the
// "scripts" of its _cool-gas.json lists them (which is the order in which GAS runs them), else in
// the order of their names, or else an empty optional together with the reason:
std::pair<std::optional<std::vector<std::pair<std::string, std::string>>>, std::string>
gasScripts(Poco::Path const & dir);

// A script that runs an extension's installed code for a call of the client, with the source and
// the start line for any stack frames that it reports:
struct CallScript {
    std::string source;
    int line;
    std::string text;
};

// The script, which evaluates to the function that takes the call's arguments, for a call of the
// given kind ("remote", "gas", "listen" or "unlisten") and name to the extension with
// the given ID (found first among the presets of configuration configId under childRoot, either
// empty for none, then among the built-in extensions), or an empty optional with the reason in
// error:
std::optional<CallScript> makeCallScript(
    std::string const & childRoot, std::string const & configId, std::string const & id,
    std::string const & kind, std::string const & name, std::string & error);

// Assemble an Apps Script <id>/_cool-gas.json sidecar body from the extension directory contents:
//  - `scripts` is a list of (server-side script file name, source text) pairs
// Besides the script names, the body carries the names of the functions that the scripts declare,
// and the add-on's display name and the document
// types it targets, as far as sniffing the source finds
// them.
std::string synthesizeGasSidecar(std::vector<std::pair<std::string, std::string>> const & scripts);

// The path of the file at the relative path within the directory of an extension, if it is one that
// pages of the extension may load, as opposed to code that only the kit runs (the scripts of an
// Apps Script add-on, and the files that the manifest of a native extension lists as "remote"), and
// is reached through a single spelling of its path without any symbolic links, else an empty
// optional:
std::optional<Poco::Path> servableFile(Poco::Path const & extension, std::string const & file);

#if ENABLE_DEBUG
// For the dev-only "drop a directory into browser/dist/extensions/" feature, emit
// <distExtensionsDir>/index.json as a JSON array of the <id>s of the extension subdirectories,
// plus a <id>/_cool-gas.json sidecar for each Apps Script directory
// (manifest.json vs. appsscript.json distinguishes the two kinds), and record distExtensionsDir as
// the directory of the built-in extensions.
void synthesizeBuiltinExtensionsIndex(std::string const & distExtensionsDir);
#endif

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
