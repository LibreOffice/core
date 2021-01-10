/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/text/TextContentAnchorType.hpp>

constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/uibase/frmdlg/data/";

/// Covers sw/source/uibase/frmdlg/ fixes.
class SwUibaseFrmdlgTest : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(SwUibaseFrmdlgTest, testWrappedMathObject)
{
    // The document includes a Math object with explicit wrapping.
    load(DATA_DIRECTORY, "wrapped-math-object.docx");
    uno::Reference<drawing::XShape> xMath = getShape(1);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 4 (AT_CHARACTER)
    // - Actual  : 1 (AS_CHARACTER)
    // i.e. the object lost its wrapping, leading to an incorrect position.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,
                         getProperty<text::TextContentAnchorType>(getShape(1), "AnchorType"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
