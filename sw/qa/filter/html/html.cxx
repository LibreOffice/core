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

#include <docsh.hxx>
#include <fmtfsize.hxx>
#include <frameformats.hxx>
#include <unotxdoc.hxx>

namespace
{
constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/filter/html/data/";

/**
 * Covers sw/source/filter/html/ fixes.
 *
 * Note that these tests are meant to be simple: either load a file and assert some result or build
 * a document model with code, export and assert that result.
 *
 * Keep using the various sw_<format>import/export suites for multiple filter calls inside a single
 * test.
 */
class Test : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(Test, testEmptyParagraph)
{
    // Given a document with 2 paragraphs, the second is empty:
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "empty-paragraph.xhtml";
    uno::Sequence<beans::PropertyValue> aLoadArgs = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
    };

    // When loading that file:
    mxComponent = loadFromDesktop(aURL, OUString(), aLoadArgs);

    // Then make sure that the resulting document has a 2nd empty paragraph:
    getParagraph(1, "a");
    // Without the accompanying fix in place, this test would have failed with:
    // An uncaught exception of type com.sun.star.container.NoSuchElementException
    // i.e. the 2nd paragraph was lost.
    getParagraph(2);
}

CPPUNIT_TEST_FIXTURE(Test, testRelativeKeepAspect)
{
    // Given a document with an OLE object, width set to 100%, height is not set:
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "relative-keep-aspect.xhtml";
    uno::Sequence<beans::PropertyValue> aLoadArgs = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
    };

    // When loading that file:
    mxComponent = loadFromDesktop(aURL, OUString(), aLoadArgs);

    // Then make sure that the aspect ratio of the image is kept:
    auto pTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDoc* pDoc = pTextDocument->GetDocShell()->GetDoc();
    const SwFrameFormats& rFormats = *pDoc->GetSpzFrameFormats();
    const SwFrameFormat* pFormat = rFormats[0];
    const SwFormatFrameSize& rSize = pFormat->GetFrameSize();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 255
    // - Actual  : 0
    // i.e. the height had a fixed value, not "keep aspect".
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(SwFormatFrameSize::SYNCED),
                         static_cast<int>(rSize.GetHeightPercent()));
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
