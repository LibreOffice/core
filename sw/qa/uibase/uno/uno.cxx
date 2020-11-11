/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/util/XCloseable.hpp>

#include <vcl/scheduler.hxx>

char const DATA_DIRECTORY[] = "/sw/qa/uibase/uno/data/";

/// Covers sw/source/uibase/uno/ fixes.
class SwUibaseUnoTest : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(SwUibaseUnoTest, testCondFieldCachedValue)
{
    load(DATA_DIRECTORY, "cond-field-cached-value.docx");
    Scheduler::ProcessEventsToIdle();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  :
    // i.e. the conditional field lost its cached content.
    getParagraph(2, "1");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
