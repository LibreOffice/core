/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <frameformats.hxx>
#include <frmatr.hxx>
#include <swtable.hxx>

namespace
{
/**
 * Covers sw/source/filter/xml/ fixes.
 *
 * Note that these tests are meant to be simple: either load a file and assert some result or build
 * a document model with code, export and assert that result.
 *
 * Keep using the various sw_<format>import/export suites for multiple filter calls inside a single
 * test.
 */
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/filter/xml/data/"_ustr, u"writer8"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testCoveredCellBackground)
{
    // Given a document with a table with vertically merged cells, with a solid background:
    createSwDoc("covered-cell-background.odt");

    // When checking the background of the last row's first covered table cell:
    SwDoc* pDoc = getSwDoc();
    const SwTableFormat* pTableFormat = (*pDoc->GetTableFrameFormats())[0];
    SwTable* pTable = SwTable::FindTable(pTableFormat);
    SwTableBox* pBox = const_cast<SwTableBox*>(pTable->GetTableBox(u"A5"_ustr));
    SwFrameFormat* pCellFormat = pBox->GetFrameFormat();
    const SvxBrushItem& rBackground = pCellFormat->GetAttrSet().GetBackground();

    // Then make sure the covered cell has a solid background, just like other cells in the first
    // column:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: rgba[e8f2a1ff]
    // - Actual  : rgba[ffffff00]
    // i.e. part of the merged cell had a bad white background.
    CPPUNIT_ASSERT_EQUAL(Color(0xe8f2a1), rBackground.GetColor());
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
