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

#ifndef OFFICEOTRON_OOXMLSESSION_HXX
#define OFFICEOTRON_OOXMLSESSION_HXX

#include "report.hxx"
#include "schemacache.hxx"
#include "zipfile.hxx"

// Validates an OOXML package: checks the relationship types of the OPC
// package for consistency with ISO/IEC 29500-1:2008, then validates each
// part with a known content type against its schema, after filtering the
// MCE markup out. Returns the total error count.
int runOOXMLValidation(const ZipArchive& zip, ValidationReport& report, SchemaCache& schemas);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
