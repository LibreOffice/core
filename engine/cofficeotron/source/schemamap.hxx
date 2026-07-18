/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * cofficeotron - a C++ port of the Java(tm) Office-o-tron
 *
 * Copyright (c) 2009 Griffin Brown Digital Publishing Ltd.
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef OFFICEOTRON_SCHEMAMAP_HXX
#define OFFICEOTRON_SCHEMAMAP_HXX

#include <string>

// One row of the OOXML content-type table: the ISO/IEC 29500-1:2008
// clause defining the part, its content type, root namespace, the
// relationship type that must point at it, and the schema file (relative
// to the schema directory) that validates it. An empty schemaName means
// the part type is known but has no schema to validate against.
struct OOXMLSchemaMapping
{
    const char* clause;
    const char* contentType;
    const char* ns;
    const char* relType;
    const char* schemaName;
};

// Returns null when the content type is not in the table.
const OOXMLSchemaMapping* findMappingForContentType(const std::string& contentType);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
