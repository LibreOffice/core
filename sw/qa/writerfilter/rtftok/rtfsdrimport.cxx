/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/ColorMode.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>

using namespace ::com::sun::star;

namespace
{
/// Tests for sw/source/writerfilter/rtftok/rtfsdrimport.cxx.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest("/sw/qa/writerfilter/rtftok/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testPictureInTextframe)
{
    loadFromFile(u"picture-in-textframe.rtf");
    uno::Reference<drawing::XDrawPageSupplier> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xTextDocument->getDrawPage();
    uno::Reference<beans::XPropertySet> xInnerShape(xDrawPage->getByIndex(1), uno::UNO_QUERY);
    text::TextContentAnchorType eAnchorType = text::TextContentAnchorType_AT_PARAGRAPH;
    xInnerShape->getPropertyValue(u"AnchorType"_ustr) >>= eAnchorType;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 4
    // i.e. the properties of the inner shape (including its anchor type and bitmap fill) were lost
    // on import.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER, eAnchorType);
}

CPPUNIT_TEST_FIXTURE(Test, testWatermark)
{
    // Given a document with a picture watermark, and the "washout" checkbox is ticked on the Word
    // UI:
    loadFromFile(u"watermark.rtf");

    // Then make sure the watermark effect is not lost on import:
    uno::Reference<drawing::XDrawPageSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPagesSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    drawing::ColorMode eMode{};
    xShape->getPropertyValue(u"GraphicColorMode"_ustr) >>= eMode;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 3
    // - Actual  : 0
    // i.e. the color mode was STANDARD, not WATERMARK.
    CPPUNIT_ASSERT_EQUAL(drawing::ColorMode_WATERMARK, eMode);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
