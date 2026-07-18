/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * cofficeotron - a C++ port of the Java(tm) Office-o-tron
 *
 * Copyright (c) 2009-2010 Griffin Brown Digital Publishing Ltd.
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef OFFICEOTRON_ODFSESSION_HXX
#define OFFICEOTRON_ODFSESSION_HXX

#include "report.hxx"
#include "schemacache.hxx"
#include "zipfile.hxx"

struct OdfSessionOptions
{
    // Validate against the ISO/IEC 26300 (ODF 1.0) schema no matter what
    // version the documents claim.
    bool forceIs = false;
    // Cross-check ID and IDREF attribute values within each document.
    bool checkIds = false;
};

// Validates an ODF package: walks the manifest, sniffs each XML entry,
// validates documents in the office namespace against the RELAX NG
// schema of their claimed version, and for packages claiming 1.2 or
// later also validates the manifest itself.
void runOdfValidation(const ZipArchive& zip, ValidationReport& report, SchemaCache& schemas,
                      const OdfSessionOptions& options);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
