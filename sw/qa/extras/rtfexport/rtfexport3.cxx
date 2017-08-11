/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XTextColumns.hpp>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/rtfexport/data/", "Rich Text Format") {}
};

DECLARE_RTFEXPORT_TEST(testTdf108949, "tdf108949_footnoteCharFormat.odt")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Paragraph Numbering style", OUString(), getProperty<OUString>(getParagraph(2), "NumberingStyleName"));

    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);

    uno::Reference<text::XText> xFootnoteText;
    xFootnotes->getByIndex(0) >>= xFootnoteText;
    // This was green (0x00A800), the character property of the footnote character, not the footnote text
    //CPPUNIT_ASSERT_EQUAL(sal_Int32(0x000000), getProperty<sal_Int32>(getRun(getParagraphOfText(1, xFootnoteText),1), "CharColor"));
}

DECLARE_RTFIMPORT_TEST(testTdf108949_footnote, "tdf108949_footnote.rtf")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Paragraph Numbering style", OUString(), getProperty<OUString>(getParagraph(2), "NumberingStyleName"));

    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);

    uno::Reference<text::XFootnote> xFootnote;
    xFootnotes->getByIndex(0) >>= xFootnote;
    // The color of the footnote anchor was black (0x000000)
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Footnote Character color", sal_Int32(0xFF0000), getProperty< sal_Int32 >(xFootnote->getAnchor(), "CharColor") );
}

DECLARE_RTFIMPORT_TEST(testTdf109382_customFootnotes, "tdf109382_customFootnotes.rtf")
{
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);
    uno::Reference<text::XFootnote> xFootnote;

    xFootnotes->getByIndex(0) >>= xFootnote;
    CPPUNIT_ASSERT_EQUAL( OUString("CUSTOM"), xFootnote->getAnchor()->getString() );
    xFootnotes->getByIndex(1) >>= xFootnote;
    CPPUNIT_ASSERT_EQUAL( OUString("X"), xFootnote->getAnchor()->getString() );

    uno::Reference<text::XText> xFootnoteText;
    xFootnotes->getByIndex(1) >>= xFootnoteText;
    CPPUNIT_ASSERT_EQUAL( OUString("X - single character anchor"), xFootnoteText->getString() );

    uno::Reference<text::XEndnotesSupplier> xEndnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEndnotes(xEndnotesSupplier->getEndnotes(), uno::UNO_QUERY);
    uno::Reference<text::XFootnote> xEndnote;

    xEndnotes->getByIndex(0) >>= xEndnote;
    CPPUNIT_ASSERT_EQUAL( OUString("FANCY"), xEndnote->getAnchor()->getString() );
    xEndnotes->getByIndex(1) >>= xEndnote;
    CPPUNIT_ASSERT_EQUAL( OUString("Y"), xEndnote->getAnchor()->getString() );

    uno::Reference<text::XText> xEndnoteText;
    xEndnotes->getByIndex(1) >>= xEndnoteText;
    CPPUNIT_ASSERT_EQUAL( OUString("Y - simple custom endnote"), xEndnoteText->getString() );
}
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
