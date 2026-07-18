/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * cofficeotron - a C++ port of the Java(tm) Office-o-tron
 *
 * Copyright (c) 2011 Novell Inc.
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef OFFICEOTRON_MCEFILTER_HXX
#define OFFICEOTRON_MCEFILTER_HXX

#include <string>

#include "report.hxx"
#include "xmlutil.hxx"

// Rewrites a parsed OOXML part in place, applying the Markup
// Compatibility and Extensibility rules of ISO/IEC 29500 Part 3:
//
// - Elements whose prefix an ancestor's mc:Ignorable declares are
//   removed with their content, except that content matching an
//   mc:ProcessContent declaration is kept (the ignorable tag itself
//   still goes away).
// - Attributes with an ignorable prefix and all attributes in the MCE
//   namespace are removed.
// - mc:AlternateContent, mc:Choice and mc:Fallback tags are removed;
//   the content of mc:Choice is dropped and the content of mc:Fallback
//   is kept. Structural misuse of the MCE elements is reported through
//   the ErrorCapper.
// - In a VML stream the root element must be <xml>, and every element
//   named plain "xml" is treated as ignorable.
void MCEFilter(XmlNode* document, bool vmlStream, ErrorCapper& errors);

// Collapses runs of whitespace to single spaces and trims the ends, as
// ISO/IEC 29500 Part 3, Chapter 10 requires for MCE attribute values.
std::string MCENormalizeWhitespaces(const std::string& value);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
