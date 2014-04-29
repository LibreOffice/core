/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <test/bootstrapfixture.hxx>
#include <test/htmltesttools.hxx>
#include <test/xmltesttools.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotest/macros_test.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>

#include "docsh.hxx"

using namespace css::uno;
using namespace css::lang;
using namespace css::frame;
using namespace utl;

class ScHTMLExportTest : public test::BootstrapFixture, public unotest::MacrosTest, public XmlTestTools, public HtmlTestTools
{
    Reference<XComponent> mxComponent;
    TempFile              maTempFile;
    OUString              maFilterOptions;

    void load(const char* pDir, const char* pName)
    {
        if (mxComponent.is())
            mxComponent->dispose();
        mxComponent = loadFromDesktop(getURLFromSrc(pDir) + OUString::createFromAscii(pName), "com.sun.star.comp.Calc.SpreadsheetDocument");
    }

    void save(const OUString& aFilterName, TempFile& rTempFile)
    {
        rTempFile.EnableKillingFile();

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

    virtual void setUp() SAL_OVERRIDE
    {
        test::BootstrapFixture::setUp();
        mxDesktop.set(css::frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));
    }

    void testHtmlSkipImage()
    {
        htmlDocPtr pDoc;

        load("/sc/qa/extras/testdocuments/", "BaseForHTMLExport.ods");
        save("HTML (StarCalc)", maTempFile);
        pDoc = parseHtml(maTempFile);
        CPPUNIT_ASSERT (pDoc);

        assertXPath(pDoc, "/html/body", 1);
        assertXPath(pDoc, "/html/body/table/tr/td/img", 1);

        load("/sc/qa/extras/testdocuments/", "BaseForHTMLExport.ods");
        maFilterOptions = OUString("SkipImages");
        save("HTML (StarCalc)", maTempFile);

        pDoc = parseHtml(maTempFile);
        CPPUNIT_ASSERT (pDoc);
        assertXPath(pDoc, "/html/body", 1);
        assertXPath(pDoc, "/html/body/table/tr/td/img", 0);
    }

    CPPUNIT_TEST_SUITE(ScHTMLExportTest);
    CPPUNIT_TEST(testHtmlSkipImage);
    CPPUNIT_TEST_SUITE_END();

};

CPPUNIT_TEST_SUITE_REGISTRATION(ScHTMLExportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
