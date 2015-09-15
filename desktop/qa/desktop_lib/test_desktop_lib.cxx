/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>

#include <boost/property_tree/json_parser.hpp>
#include <comphelper/processfactory.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/lokhelper.hxx>
#include <test/unoapi_test.hxx>

#include "../../inc/lib/init.hxx"

using namespace com::sun::star;
using namespace desktop;

class DesktopLOKTest : public UnoApiTest
{
public:
    DesktopLOKTest() : UnoApiTest("/desktop/qa/data/")
    {
    }

    virtual ~DesktopLOKTest()
    {
    }

    virtual void setUp() SAL_OVERRIDE
    {
        UnoApiTest::setUp();
        mxDesktop.set(frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));
    };

    virtual void tearDown() SAL_OVERRIDE
    {
        closeDoc();
        UnoApiTest::tearDown();
    };

    LibLODocument_Impl* loadDoc(const char* pName);
    void closeDoc();

    void testGetStyles();
    void testGetFonts();
    void testCreateView();

    CPPUNIT_TEST_SUITE(DesktopLOKTest);
    CPPUNIT_TEST(testGetStyles);
    CPPUNIT_TEST(testGetFonts);
    CPPUNIT_TEST(testCreateView);
    CPPUNIT_TEST_SUITE_END();

    uno::Reference<lang::XComponent> mxComponent;
};

LibLODocument_Impl* DesktopLOKTest::loadDoc(const char* pName)
{
    OUString aFileURL;
    createFileURL(OUString::createFromAscii(pName), aFileURL);
    mxComponent = loadFromDesktop(aFileURL, "com.sun.star.text.TextDocument");
    if (!mxComponent.is())
    {
        CPPUNIT_ASSERT(false);
    }
    return new LibLODocument_Impl(mxComponent);
}

void DesktopLOKTest::closeDoc()
{
    if (mxComponent.is())
    {
        closeDocument(mxComponent);
        mxComponent.clear();
    }
}

void DesktopLOKTest::testGetStyles()
{
    LibLODocument_Impl* pDocument = loadDoc("blank_text.odt");
    boost::property_tree::ptree aTree;
    char* pJSON = pDocument->m_pDocumentClass->getCommandValues(pDocument, ".uno:StyleApply");
    std::stringstream aStream(pJSON);
    boost::property_tree::read_json(aStream, aTree);
    CPPUNIT_ASSERT( aTree.size() > 0 );
    CPPUNIT_ASSERT( aTree.get_child("commandName").get_value<std::string>() == ".uno:StyleApply" );

    boost::property_tree::ptree aValues = aTree.get_child("commandValues");
    CPPUNIT_ASSERT( aValues.size() > 0 );
    for (const std::pair<std::string, boost::property_tree::ptree>& rPair : aValues)
    {
        CPPUNIT_ASSERT( rPair.second.size() > 0);
        if (rPair.first != "CharacterStyles" &&
            rPair.first != "ParagraphStyles" &&
            rPair.first != "FrameStyles" &&
            rPair.first != "PageStyles" &&
            rPair.first != "NumberingStyles" &&
            rPair.first != "CellStyles" &&
            rPair.first != "ShapeStyles")
        {
            CPPUNIT_FAIL("Unknown style family: " + rPair.first);
        }
    }
    closeDoc();
}

void DesktopLOKTest::testGetFonts()
{
    LibLODocument_Impl* pDocument = loadDoc("blank_text.odt");
    boost::property_tree::ptree aTree;
    char* pJSON = pDocument->m_pDocumentClass->getCommandValues(pDocument, ".uno:CharFontName");
    std::stringstream aStream(pJSON);
    boost::property_tree::read_json(aStream, aTree);
    CPPUNIT_ASSERT( aTree.size() > 0 );
    CPPUNIT_ASSERT( aTree.get_child("commandName").get_value<std::string>() == ".uno:CharFontName" );

    boost::property_tree::ptree aValues = aTree.get_child("commandValues");
    CPPUNIT_ASSERT( aValues.size() > 0 );
    for (const std::pair<std::string, boost::property_tree::ptree>& rPair : aValues)
    {
        // check that we have font sizes available for each font
        CPPUNIT_ASSERT( rPair.second.size() > 0);
    }
    closeDoc();
}

void DesktopLOKTest::testCreateView()
{
    LibLODocument_Impl* pDocument = loadDoc("blank_text.odt");
    CPPUNIT_ASSERT_EQUAL(1, SfxLokHelper::getViews());

    int nId = pDocument->m_pDocumentClass->createView(pDocument);
    CPPUNIT_ASSERT_EQUAL(2, SfxLokHelper::getViews());

    pDocument->m_pDocumentClass->destroyView(pDocument, nId);
    CPPUNIT_ASSERT_EQUAL(1, SfxLokHelper::getViews());
    closeDoc();
}

CPPUNIT_TEST_SUITE_REGISTRATION(DesktopLOKTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
