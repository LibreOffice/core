/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/awt/FontFamily.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

#include <vcl/embeddedfontshelper.hxx>

using namespace com::sun::star;

namespace
{
/// Tests for sw/source/writerfilter/dmapper/FontTable.cxx.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest(u"/writerfilter/qa/cppunittests/dmapper/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testSubsettedEmbeddedFont)
{
    // Given a document with a subsetted embedded font, loaded for editing:
    loadFromFile(u"subsetted-embedded-font.docx");

    // When checking if the font is available:
    OUString aUrl = EmbeddedFontsHelper::fontFileUrl(
        u"IBM Plex Serif Light", FAMILY_ROMAN, ITALIC_NONE, WEIGHT_NORMAL, PITCH_VARIABLE,
        EmbeddedFontsHelper::FontRights::ViewingAllowed);

    // Then make sure the subsetted font is not available, given that the newly inserted characters
    // during editing may be missing from the subsetted font:
    CPPUNIT_ASSERT(aUrl.isEmpty());
}

CPPUNIT_TEST_FIXTURE(Test, testSubsettedFullEmbeddedFont)
{
#if !defined(MACOSX) // FIXME fails on macOS
    // Given a document with an embedded font (marked as subsetted, but otherwise full in practice),
    // loaded for editing:
    loadFromFile(u"subsetted-full-embedded-font.docx");

    // When checking if the font is available:
    OUString aUrl = EmbeddedFontsHelper::fontFileUrl(
        u"IBM Plex Serif Light", FAMILY_ROMAN, ITALIC_NONE, WEIGHT_NORMAL, PITCH_VARIABLE,
        EmbeddedFontsHelper::FontRights::ViewingAllowed);

    // Then make sure the subsetted font is available, given that it has the reasonable amount of
    // glyphs:
    CPPUNIT_ASSERT(!aUrl.isEmpty());
#endif
}

CPPUNIT_TEST_FIXTURE(Test, testFontFamily)
{
    // Given a document with 2 paragraphs, first is sans, second is serif:
    // When loading that document:
    loadFromFile(u"font-family.docx");

    // Then make sure we import <w:family w:val="...">:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    // First paragraph: sans.
    uno::Reference<container::XEnumerationAccess> xPortionEnumAccess(xParaEnum->nextElement(),
                                                                     uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortionEnum = xPortionEnumAccess->createEnumeration();
    uno::Reference<beans::XPropertySet> xPortion(xPortionEnum->nextElement(), uno::UNO_QUERY);
    sal_Int16 nFontFamily = awt::FontFamily::DONTKNOW;
    xPortion->getPropertyValue(u"CharFontFamily"_ustr) >>= nFontFamily;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 5 (SWISS)
    // - Actual  : 3 (ROMAN)
    // i.e. the font family was not imported, all font family was roman.
    CPPUNIT_ASSERT_EQUAL(awt::FontFamily::SWISS, nFontFamily);
    // Second paragraph: serif.
    xPortionEnumAccess.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPortionEnum = xPortionEnumAccess->createEnumeration();
    xPortion.set(xPortionEnum->nextElement(), uno::UNO_QUERY);
    nFontFamily = awt::FontFamily::DONTKNOW;
    xPortion->getPropertyValue(u"CharFontFamily"_ustr) >>= nFontFamily;
    CPPUNIT_ASSERT_EQUAL(awt::FontFamily::ROMAN, nFontFamily);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
