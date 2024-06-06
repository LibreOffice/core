/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

namespace
{
/// Covers sw/source/core/layout/sortedobjs.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/layout/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testSortedObjsInsert)
{
    // Given a document with two anchored objects, one is a fly frame in the header and the other is
    // a group shape where one inner shape has an associated fly frame:
    // When importing that document:
    // Then make sure that we don't try to do binary search on an unsorted container that leads to a
    // crash in debug builds:
    createSwDoc("sorted-objs-insert.docx");
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
