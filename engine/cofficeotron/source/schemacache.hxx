/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * cofficeotron - a C++ port of the Java(tm) Office-o-tron
 *
 * Copyright (c) 2010-2011 Novell Inc.
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef OFFICEOTRON_SCHEMACACHE_HXX
#define OFFICEOTRON_SCHEMACACHE_HXX

#include <map>
#include <string>

#include "report.hxx"
#include "xmlutil.hxx"

// Initializes the RNV validator library. Must run once before any
// schema is loaded or any document validated.
void initValidation();

// Loads RELAX NG compact syntax schemas from below a schema directory
// and keeps the compiled start pattern for the life of the process, so
// several documents on one command line share one compilation per
// schema.
class SchemaCache
{
public:
    explicit SchemaCache(const std::string& schemaDirectory);

    SchemaCache(const SchemaCache&) = delete;
    SchemaCache& operator=(const SchemaCache&) = delete;

    const std::string& directory() const { return schemaDir; }

    // Returns the schema's start pattern, or 0 on compilation failure
    // with a message in error; a failed compilation is remembered and
    // not retried.
    int getSchema(const std::string& relativePath, std::string& error);

private:
    std::string schemaDir;
    std::map<std::string, int> startByPath;
    std::map<std::string, std::string> failureByPath;
};

// Validates one element tree against a loaded schema's start pattern,
// reporting each validity error through the capper with the line number
// of the nearest enclosing element. Returns true when the tree is
// valid.
bool validateElement(const XmlNode* element, int startPattern, ErrorCapper& capper);

// Finds the schema directory: an explicit override, the COFFICEOTRON_SCHEMA_DIR
// environment variable, etc/schema next to the executable or one level above
// it, or etc/schema under the current directory. Returns the empty string
// when none of those exists.
std::string locateSchemaDirectory(const std::string& override_);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
