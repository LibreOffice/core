/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <address.hxx>

#include <cppunit/plugin/TestPlugIn.h>

// Compile time tests for ScAddress

static_assert(ScAddress() == ScAddress(0, 0, 0));
static_assert(ScAddress().IsValid() == true);
static_assert(ScAddress(ScAddress::INITIALIZE_INVALID).IsValid() == false);

// Equal, not Equal

// Format is Col, Row, Tab and Col has priority over Row
static_assert(ScAddress(0, 0, 0) != ScAddress(1, 0, 0)); // A1 != B1
static_assert(ScAddress(0, 0, 0) != ScAddress(0, 1, 0)); // A1 != A2
static_assert(ScAddress(0, 0, 0) != ScAddress(0, 0, 1)); // Sheet1:A1 != Sheet2:A1
static_assert(ScAddress(0, 0, 0) == ScAddress(0, 0, 0)); // A1 == A1

// Less

static_assert(!(ScAddress(1, 0, 0) < ScAddress(1, 0, 0))); // B1 < B1
static_assert(!(ScAddress(0, 1, 0) < ScAddress(0, 1, 0))); // A2 < A2
static_assert(!(ScAddress(0, 0, 1) < ScAddress(0, 0, 1))); // Sheet2:A1 < Sheet2:A1

static_assert(ScAddress(0, 0, 0) < ScAddress(1, 0, 0)); // A1 < B1
static_assert(ScAddress(0, 1, 0) < ScAddress(1, 0, 0)); // A2 < B1
static_assert(ScAddress(0, 10000, 0) < ScAddress(1, 0, 0)); // A10000 < B1
static_assert(ScAddress(0, 10000, 0) < ScAddress(1, 10000, 0)); // A10000 < B10000
static_assert(ScAddress(1, 0, 0) < ScAddress(1, 0, 1)); // Sheet1:A1 < Sheet2:A1

// Less or Equal

static_assert(ScAddress(1, 0, 0) <= ScAddress(1, 0, 0)); // B1 <= B1
static_assert(ScAddress(0, 1, 0) <= ScAddress(0, 1, 0)); // A2 <= A2
static_assert(ScAddress(0, 0, 1) <= ScAddress(0, 0, 1)); // Sheet2:A1 <= Sheet2:A1

static_assert(ScAddress(0, 0, 0) <= ScAddress(1, 0, 0)); // A1 <= B1
static_assert(ScAddress(0, 1, 0) <= ScAddress(1, 0, 0)); // A2 <= B1
static_assert(ScAddress(0, 10000, 0) <= ScAddress(1, 0, 0)); // A10000 <= B1
static_assert(ScAddress(0, 10000, 0) <= ScAddress(1, 10000, 0)); // A10000 <= B10000
static_assert(ScAddress(1, 0, 0) <= ScAddress(1, 0, 1)); // Sheet1:A1 <= Sheet2:A1

// More

static_assert(!(ScAddress(1, 0, 0) > ScAddress(1, 0, 0))); // B1 > B1
static_assert(!(ScAddress(0, 1, 0) > ScAddress(0, 1, 0))); // A2 > A2
static_assert(!(ScAddress(0, 0, 1) > ScAddress(0, 0, 1))); // Sheet1:A1 > Sheet2:A1

static_assert(ScAddress(1, 0, 0) > ScAddress(0, 0, 0)); // B1 > A1
static_assert(ScAddress(1, 0, 0) > ScAddress(0, 1, 0)); // B1 > A2
static_assert(ScAddress(1, 0, 0) > ScAddress(0, 10000, 0)); // B1 > A10000
static_assert(ScAddress(1, 10000, 0) > ScAddress(0, 10000, 0)); // B10000 > A10000
static_assert(ScAddress(1, 0, 1) > ScAddress(1, 0, 0)); // Sheet2:A1 > Sheet1:A1

// More or Equal

static_assert(ScAddress(1, 0, 0) >= ScAddress(1, 0, 0)); // B1 >= B1
static_assert(ScAddress(0, 1, 0) >= ScAddress(0, 1, 0)); // A2 >= A2
static_assert(ScAddress(0, 0, 1) >= ScAddress(0, 0, 1)); // Sheet1:A1 >= Sheet2:A1

static_assert(ScAddress(1, 0, 0) >= ScAddress(0, 0, 0)); // B1 >= A1
static_assert(ScAddress(1, 0, 0) >= ScAddress(0, 1, 0)); // B1 >= A2
static_assert(ScAddress(1, 0, 0) >= ScAddress(0, 10000, 0)); // B1 >= A10000
static_assert(ScAddress(1, 10000, 0) >= ScAddress(0, 10000, 0)); // B10000 >= A10000
static_assert(ScAddress(1, 0, 1) >= ScAddress(1, 0, 0)); // Sheet2:A1 >= Sheet1:A1

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
