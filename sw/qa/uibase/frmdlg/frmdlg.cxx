/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <comphelper/propertyvalue.hxx>

#include <com/sun/star/text/TextContentAnchorType.hpp>

/// Covers sw/source/uibase/frmdlg/ fixes.
class SwUibaseFrmdlgTest : public SwModelTestBase
{
public:
    SwUibaseFrmdlgTest()
        : SwModelTestBase("/sw/qa/uibase/frmdlg/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwUibaseFrmdlgTest, testWrappedMathObject)
{
    // The document includes a Math object with explicit wrapping.
    createSwDoc("wrapped-math-object.docx");
    uno::Reference<drawing::XShape> xMath = getShape(1);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 4 (AT_CHARACTER)
    // - Actual  : 1 (AS_CHARACTER)
    // i.e. the object lost its wrapping, leading to an incorrect position.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,
                         getProperty<text::TextContentAnchorType>(getShape(1), "AnchorType"));
}

CPPUNIT_TEST_FIXTURE(SwUibaseFrmdlgTest, testAnchorTypeFromStyle)
{
    // Given a document with aGraphics style with anchor type set to as-character:
    createSwDoc();
    uno::Reference<beans::XPropertySet> xGraphics(getStyles("FrameStyles")->getByName("Graphics"),
                                                  uno::UNO_QUERY);
    xGraphics->setPropertyValue("AnchorType", uno::Any(text::TextContentAnchorType_AS_CHARACTER));

    // When inserting an image:
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("FileName", createFileURL(u"image.png")),
    };
    dispatchCommand(mxComponent, ".uno:InsertGraphic", aArgs);

    // Then make sure the image's anchor type is as-char:
    auto eActual = getProperty<text::TextContentAnchorType>(getShape(1), "AnchorType");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1 (AS_CHARACTER)
    // - Actual  : 4 (AT_CHARACTER)
    // i.e. the anchor type from the style was ignored.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER, eActual);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
