/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <QString>

#include <string>

// The wildcards from COKit::getInsertableFileWildcards(), set once on the kit thread at
// start-up.
void setInsertableFileWildcards(const std::string& wildcards);

// Whether the engine inserts a file of this name, by extension, ignoring case. Safe on the
// GUI thread; false until the wildcards are set.
bool isInsertableFileName(const QString& path);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
