/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/text/FontEmphasis.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <oox/drawingml/drawingmltypes.hxx>
#include <config_features.h>
#include <string>
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text")
    {
    }

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override
    {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx");
    }

    /**
     * Validation handling
     */
    bool mustValidate(const char* filename) const override
    {
        const char* aWhitelist[] = { "fdo38244.docx", "comments-nested.odt" };
        std::vector<const char*> vWhitelist(aWhitelist, aWhitelist + SAL_N_ELEMENTS(aWhitelist));

        return std::find(vWhitelist.begin(), vWhitelist.end(), filename) != vWhitelist.end();
    }

protected:
};

DECLARE_OOXMLEXPORT_TEST(testEmptyAnnotationMark, "empty-annotation-mark.docx")
{
    if (mbExported)
    {
        // Delete the word that is commented, and save again.
        uno::Reference<text::XTextRange> xRun = getRun(getParagraph(1), 3);
        CPPUNIT_ASSERT_EQUAL(OUString("with"), xRun->getString());
        xRun->setString("");
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
        xStorable->store();

        // Then inspect the OOXML markup of the modified document model.
        xmlDocPtr pXmlDoc = parseExport("word/document.xml");
        // There were two commentReference nodes.
        assertXPath(pXmlDoc, "//w:commentReference", "id", "0");
        // Empty comment range was not ignored on export, this was 1.
        assertXPath(pXmlDoc, "//w:commentRangeStart", 0);
        // Ditto.
        assertXPath(pXmlDoc, "//w:commentRangeEnd", 0);
    }
}

#if !defined(_WIN32)

DECLARE_OOXMLEXPORT_TEST(testFdo38244, "fdo38244.docx")
{
    /*
    * Comments attached to a range was imported without the range, check for the annotation mark start/end positions.
    *
    * oParas = ThisComponent.Text.createEnumeration
    * oPara = oParas.nextElement
    * oRuns = oPara.createEnumeration
    * oRun = oRuns.nextElement
    * oRun = oRuns.nextElement 'Annotation
    * oRun = oRuns.nextElement
    * oRun = oRuns.nextElement 'AnnotationEnd
    * xray oRun.TextPortionType
    */
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParaEnum->nextElement(),
                                                                 uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    xRunEnum->nextElement();
    uno::Reference<beans::XPropertySet> xPropertySet(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Annotation"),
                         getProperty<OUString>(xPropertySet, "TextPortionType"));
    xRunEnum->nextElement();
    xPropertySet.set(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("AnnotationEnd"),
                         getProperty<OUString>(xPropertySet, "TextPortionType"));

    /*
    * Initials were not imported.
    *
    * oFields = ThisComponent.TextFields.createEnumeration
    * oField = oFields.nextElement
    * xray oField.Initials
    */
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(
        xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    xPropertySet.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("M"), getProperty<OUString>(xPropertySet, "Initials"));

    /*
    * There was a fake empty paragraph at the end of the comment text.
    *
    * oFields = ThisComponent.TextFields.createEnumeration
    * oField = oFields.nextElement
    * oParas = oField.TextRange.createEnumeration
    * oPara = oParas.nextElement
    * oPara = oParas.nextElement
    */

    xParaEnumAccess.set(
        getProperty<uno::Reference<container::XEnumerationAccess>>(xPropertySet, "TextRange"),
        uno::UNO_QUERY);
    xParaEnum = xParaEnumAccess->createEnumeration();
    xParaEnum->nextElement();
    bool bCaught = false;
    try
    {
        xParaEnum->nextElement();
    }
    catch (container::NoSuchElementException&)
    {
        bCaught = true;
    }
    CPPUNIT_ASSERT_EQUAL(true, bCaught);
}

DECLARE_OOXMLEXPORT_TEST(testCommentsNested, "comments-nested.odt")
{
    uno::Reference<beans::XPropertySet> xOuter(
        getProperty<uno::Reference<beans::XPropertySet>>(getRun(getParagraph(1), 2), "TextField"),
        uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Outer"), getProperty<OUString>(xOuter, "Content"));

    uno::Reference<beans::XPropertySet> xInner(
        getProperty<uno::Reference<beans::XPropertySet>>(getRun(getParagraph(1), 4), "TextField"),
        uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Inner"), getProperty<OUString>(xInner, "Content"));
}

#endif

DECLARE_OOXMLEXPORT_TEST(testParagraphWithComments, "paragraphWithComments.docx")
{
    /* Comment id's were getting overwritten for annotation mark(s),
    which was causing a mismatch in the relationship for comment id's
    in document.xml and comment.xml
    */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    xmlDocPtr pXmlComm = parseExport("word/comments.xml");
    if (!pXmlDoc)
        return;

    sal_Int32 idInDocXml
        = getXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:commentRangeEnd[1]", "id").toInt32();
    sal_Int32 idInCommentXml = getXPath(pXmlComm, "/w:comments/w:comment[1]", "id").toInt32();
    CPPUNIT_ASSERT_EQUAL(idInDocXml, idInCommentXml);
}

DECLARE_OOXMLEXPORT_TEST(testTdf104707_urlComment, "tdf104707_urlComment.odt")
{
    xmlDocPtr pXmlComm = parseExport("word/comments.xml");
    CPPUNIT_ASSERT(pXmlComm);
    CPPUNIT_ASSERT_EQUAL(
        OUString("https://bugs.documentfoundation.org/show_bug.cgi?id=104707"),
        getXPathContent(pXmlComm, "/w:comments/w:comment/w:p/w:hyperlink/w:r/w:t"));
}

DECLARE_OOXMLEXPORT_TEST(testCommentInitials, "comment_initials.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/comments.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/w:comments/w:comment[1]", "initials", "initials");
}

DECLARE_OOXMLEXPORT_TEST(testTdf103651, "tdf103651.docx")
{
    uno::Reference<beans::XPropertySet> xTextField
        = getProperty<uno::Reference<beans::XPropertySet>>(getRun(getParagraph(1), 1), "TextField");
    OUString sContent;
    xTextField->getPropertyValue("Content") >>= sContent;
    // Comment in the first paragraph should not have smiley ( 0xf04a ).
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), sContent.indexOf(u'\xf04a'));

    // this document has a w:kern setting in the DocDefault character properties.  Ensure it applies.
    CPPUNIT_ASSERT(getProperty<bool>(getRun(getParagraph(1), 1), "CharAutoKerning"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf104162, "tdf104162.docx")
{
    // This crashed: the comment field contained a table with a <w:hideMark/>.
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XElementAccess> xTextFields(xTextFieldsSupplier->getTextFields());
    CPPUNIT_ASSERT(xTextFields->hasElements());
}

DECLARE_OOXMLEXPORT_TEST(testAnnotationFormatting, "annotation-formatting.docx")
{
    uno::Reference<beans::XPropertySet> xTextField
        = getProperty<uno::Reference<beans::XPropertySet>>(getRun(getParagraph(2), 2), "TextField");
    uno::Reference<text::XText> xText
        = getProperty<uno::Reference<text::XText>>(xTextField, "TextRange");
    // Make sure we test the right annotation.
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xText, "days");
    // Formatting was lost: the second text portion was NONE, not SINGLE.
    CPPUNIT_ASSERT_EQUAL(awt::FontUnderline::SINGLE,
                         getProperty<sal_Int16>(getRun(xParagraph, 1), "CharUnderline"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo85542, "fdo85542.docx")
{
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(),
                                                            uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), xBookmarksByIdx->getCount());
    uno::Reference<container::XNameAccess> xBookmarksByName(xBookmarksSupplier->getBookmarks(),
                                                            uno::UNO_QUERY);
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("B1"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("B2"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("B3"));
    // B1
    uno::Reference<text::XTextContent> xContent1(xBookmarksByName->getByName("B1"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xRange1(xContent1->getAnchor(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("ABB"), xRange1->getString());
    // B2
    uno::Reference<text::XTextContent> xContent2(xBookmarksByName->getByName("B2"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xRange2(xContent2->getAnchor(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("BBC"), xRange2->getString());
    // B3 -- testing a collapsed bookmark
    uno::Reference<text::XTextContent> xContent3(xBookmarksByName->getByName("B3"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xRange3(xContent3->getAnchor(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(xRange3->getString(), OUString());
    uno::Reference<text::XText> xText(xRange3->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextCursor> xNeighborhoodCursor(xText->createTextCursor(),
                                                          uno::UNO_QUERY);
    xNeighborhoodCursor->gotoRange(xRange3, false);
    xNeighborhoodCursor->goLeft(1, false);
    xNeighborhoodCursor->goRight(2, true);
    uno::Reference<text::XTextRange> xTextNeighborhood(xNeighborhoodCursor, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("AB"), xTextNeighborhood->getString());
}

DECLARE_OOXMLEXPORT_TEST(testTdf65955, "tdf65955.odt")
{
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(),
                                                            uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xBookmarksByIdx->getCount());
    uno::Reference<container::XNameAccess> xBookmarksByName(xBookmarksSupplier->getBookmarks(),
                                                            uno::UNO_QUERY);
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("a"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("b"));
    // a
    uno::Reference<text::XTextContent> xContent3(xBookmarksByName->getByName("a"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xRange3(xContent3->getAnchor(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(xRange3->getString(), OUString());
    // b
    uno::Reference<text::XTextContent> xContent2(xBookmarksByName->getByName("b"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xRange2(xContent2->getAnchor(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("r"), xRange2->getString());
}

DECLARE_OOXMLEXPORT_TEST(testTdf65955_2, "tdf65955_2.odt")
{
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(),
                                                            uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xBookmarksByIdx->getCount());
    uno::Reference<container::XNameAccess> xBookmarksByName(xBookmarksSupplier->getBookmarks(),
                                                            uno::UNO_QUERY);
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("test"));

    uno::Reference<text::XTextContent> xContent3(xBookmarksByName->getByName("test"),
                                                 uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xRange3(xContent3->getAnchor(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("foo bar"), xRange3->getString());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
