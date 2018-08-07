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
#include <com/sun/star/text/TextContentAnchorType.hpp>

namespace
{
char const DATA_DIRECTORY[] = "/sw/qa/extras/unowriter/data/";
}

/// Test to assert UNO API call results of Writer.
class SwUnoWriter : public SwModelTestBase
{
public:
    void testDefaultCharStyle();
    void testGraphicDesciptorURL();
    void testGraphicDesciptorURLBitmap();
    void testSetPagePrintSettings();

    CPPUNIT_TEST_SUITE(SwUnoWriter);
    CPPUNIT_TEST(testDefaultCharStyle);
    CPPUNIT_TEST(testGraphicDesciptorURL);
    CPPUNIT_TEST(testGraphicDesciptorURLBitmap);
    CPPUNIT_TEST(testSetPagePrintSettings);
    CPPUNIT_TEST_SUITE_END();
};

void SwUnoWriter::testDefaultCharStyle()
{
    // Create a new document, type a character, set its char style to Emphasis
    // and assert the style was set.
    loadURL("private:factory/swriter", nullptr);

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XSimpleText> xBodyText(xTextDocument->getText(), uno::UNO_QUERY);
    xBodyText->insertString(xBodyText->getStart(), "x", false);

    uno::Reference<text::XTextCursor> xCursor(xBodyText->createTextCursor());
    xCursor->goLeft(1, true);

    uno::Reference<beans::XPropertySet> xCursorProps(xCursor, uno::UNO_QUERY);
    xCursorProps->setPropertyValue("CharStyleName", uno::makeAny(OUString("Emphasis")));
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                         getProperty<awt::FontSlant>(xCursorProps, "CharPosture"));

    // Now reset the char style and assert that the font slant is back to none.
    // This resulted in a lang.IllegalArgumentException, Standard was not
    // mapped to 'Default Style'.
    xCursorProps->setPropertyValue("CharStyleName", uno::makeAny(OUString("Standard")));
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                         getProperty<awt::FontSlant>(xCursorProps, "CharPosture"));
}

void SwUnoWriter::testGraphicDesciptorURL()
{
    loadURL("private:factory/swriter", nullptr);

    // Create a graphic object, but don't insert it yet.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextGraphic(
        xFactory->createInstance("com.sun.star.text.TextGraphicObject"), uno::UNO_QUERY);

    // Set an URL on it.
    OUString aGraphicURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "test.jpg";
    xTextGraphic->setPropertyValue("GraphicURL", uno::makeAny(aGraphicURL));
    xTextGraphic->setPropertyValue("AnchorType",
                                   uno::makeAny(text::TextContentAnchorType_AT_CHARACTER));

    // Insert it.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xBodyText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextCursor> xCursor(xBodyText->createTextCursor());
    uno::Reference<text::XTextContent> xTextContent(xTextGraphic, uno::UNO_QUERY);
    xBodyText->insertTextContent(xCursor, xTextContent, false);

    // This failed, the graphic object had no graphic.
    auto xGraphic = getProperty<uno::Reference<graphic::XGraphic>>(getShape(1), "Graphic");
    CPPUNIT_ASSERT(xGraphic.is());
}

void SwUnoWriter::testGraphicDesciptorURLBitmap()
{
    loadURL("private:factory/swriter", nullptr);

    // Load a bitmap into the bitmap table.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameContainer> xBitmaps(
        xFactory->createInstance("com.sun.star.drawing.BitmapTable"), uno::UNO_QUERY);
    OUString aGraphicURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "test.jpg";
    xBitmaps->insertByName("test", uno::makeAny(aGraphicURL));

    // Create a graphic.
    uno::Reference<beans::XPropertySet> xTextGraphic(
        xFactory->createInstance("com.sun.star.text.TextGraphicObject"), uno::UNO_QUERY);
    xTextGraphic->setPropertyValue("GraphicURL", xBitmaps->getByName("test"));
    xTextGraphic->setPropertyValue("AnchorType",
                                   uno::makeAny(text::TextContentAnchorType_AT_CHARACTER));

    // Insert it.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xBodyText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextCursor> xCursor(xBodyText->createTextCursor());
    uno::Reference<text::XTextContent> xTextContent(xTextGraphic, uno::UNO_QUERY);
    xBodyText->insertTextContent(xCursor, xTextContent, false);

    // This failed: setting GraphicURL to the result of getByName() did not
    // work anymore.
    auto xGraphic = getProperty<uno::Reference<graphic::XGraphic>>(getShape(1), "Graphic");
    CPPUNIT_ASSERT(xGraphic.is());
}

void SwUnoWriter::testSetPagePrintSettings()
{
    // Create an empty new document with a single char
    loadURL("private:factory/swriter", nullptr);

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XSimpleText> xBodyText(xTextDocument->getText(), uno::UNO_QUERY);
    xBodyText->insertString(xBodyText->getStart(), "x", false);

    uno::Reference<text::XPagePrintable> xPagePrintable(mxComponent, uno::UNO_QUERY);

    // set some stuff, try to get it back
    uno::Sequence<beans::PropertyValue> aProps(2);
    aProps[0].Name = "PageColumns";
    aProps[0].Value = css::uno::Any(sal_Int16(2));
    aProps[1].Name = "IsLandscape";
    aProps[1].Value = css::uno::Any(true);

    xPagePrintable->setPagePrintSettings(aProps);
    aProps = xPagePrintable->getPagePrintSettings();

    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), aProps[0].Value.get<short>());
    CPPUNIT_ASSERT_EQUAL(true, aProps[1].Value.get<bool>());
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwUnoWriter);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
