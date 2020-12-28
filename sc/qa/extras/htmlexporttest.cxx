/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <test/bootstrapfixture.hxx>
#include <test/htmltesttools.hxx>
#include <test/xmltesttools.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotest/macros_test.hxx>

using namespace css::uno;
using namespace css::lang;
using namespace css::frame;
using namespace utl;

class ScHTMLExportTest : public test::BootstrapFixture, public unotest::MacrosTest, public XmlTestTools, public HtmlTestTools
{
    Reference<XComponent> mxComponent;
    OUString              maFilterOptions;

    void load(std::u16string_view pDir, const char* pName)
    {
        if (mxComponent.is())
            mxComponent->dispose();
        mxComponent = loadFromDesktop(m_directories.getURLFromSrc(pDir) + OUString::createFromAscii(pName), "com.sun.star.comp.Calc.SpreadsheetDocument");
    }

    void save(const OUString& aFilterName, TempFile const & rTempFile)
    {
        Reference<XStorable> xStorable(mxComponent, UNO_QUERY);
        MediaDescriptor aMediaDescriptor;
        aMediaDescriptor["FilterName"] <<= aFilterName;
        if (!maFilterOptions.isEmpty())
            aMediaDescriptor["FilterOptions"] <<= maFilterOptions;
        xStorable->storeToURL(rTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    }

public:
    ScHTMLExportTest()
    {}

    virtual void setUp() override
    {
        test::BootstrapFixture::setUp();
        mxDesktop.set(css::frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));
    }

    virtual void tearDown() override
    {
        if (mxComponent.is())
            mxComponent->dispose();

        test::BootstrapFixture::tearDown();
    }

    void testHtmlSkipImage()
    {
        // need a temp dir, because there's an image exported too
        TempFile aTempDir(nullptr, true);
        OUString const url(aTempDir.GetURL());
        TempFile aTempFile(&url, false);

        htmlDocUniquePtr pDoc;

        load(u"/sc/qa/extras/testdocuments/", "BaseForHTMLExport.ods");
        save("HTML (StarCalc)", aTempFile);
        pDoc = parseHtml(aTempFile);
        CPPUNIT_ASSERT (pDoc);

        assertXPath(pDoc, "/html/body", 1);
        assertXPath(pDoc, "/html/body/table/tr/td/img", 1);

        load(u"/sc/qa/extras/testdocuments/", "BaseForHTMLExport.ods");
        maFilterOptions = "SkipImages";
        save("HTML (StarCalc)", aTempFile);

        pDoc = parseHtml(aTempFile);
        CPPUNIT_ASSERT (pDoc);
        assertXPath(pDoc, "/html/body", 1);
        assertXPath(pDoc, "/html/body/table/tr/td/img", 0);

        utl::removeTree(aTempDir.GetURL());
    }

    CPPUNIT_TEST_SUITE(ScHTMLExportTest);
    CPPUNIT_TEST(testHtmlSkipImage);
    CPPUNIT_TEST_SUITE_END();

};

CPPUNIT_TEST_SUITE_REGISTRATION(ScHTMLExportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
