/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sdmodeltestbase.hxx"

#include <test/xmltesttools.hxx>
#include <unotools/mediadescriptor.hxx>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>

#include <boost/preprocessor/stringize.hpp>

#define MAKE_PATH_STRING( path ) BOOST_PP_STRINGIZE( path )
#define SVG_SVG  *[name()='svg']
#define SVG_G *[name()='g']
#define SVG_TEXT *[name()='text']
#define SVG_TSPAN *[name()='tspan']

using namespace css;

class SdSVGFilterTest : public test::BootstrapFixture, public unotest::MacrosTest, public XmlTestTools
{
    uno::Reference<lang::XComponent> mxComponent;
    utl::TempFile maTempFile;

protected:
    virtual void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx) override
    {
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("svg"), BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"));
    }

    void load(const OUString& pDir, const char* pName)
    {
        return loadURL(m_directories.getURLFromSrc(pDir) + OUString::createFromAscii(pName), pName);
    }

    void loadURL(OUString const& rURL, const char* pName)
    {
        if (mxComponent.is())
            mxComponent->dispose();
        // Output name early, so in the case of a hang, the name of the hanging input file is visible.
        if (pName)
            std::cout << pName << ",";
        mxComponent = loadFromDesktop(rURL);
        CPPUNIT_ASSERT(mxComponent.is());
    }

    void save()
    {
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
        utl::MediaDescriptor aMediaDescriptor;
        aMediaDescriptor["FilterName"] <<= OUString("impress_svg_Export");
        xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    }

public:
    SdSVGFilterTest()
    {
        maTempFile.EnableKillingFile();
    }

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

    void executeExport(const char* pName)
    {
        load( "/sd/qa/unit/data/odp/", pName );
        save();
    }

    void testSVGExportTextDecorations()
    {
        executeExport( "svg-export-text-decorations.odp" );

        xmlDocPtr svgDoc = parseXml(maTempFile);
        CPPUNIT_ASSERT(svgDoc);

        svgDoc->name = reinterpret_cast<char *>(xmlStrdup(reinterpret_cast<xmlChar const *>(OUStringToOString(maTempFile.GetURL(), RTL_TEXTENCODING_UTF8).getStr())));

        assertXPath(svgDoc, MAKE_PATH_STRING( /SVG_SVG ), 1);
        assertXPath(svgDoc, MAKE_PATH_STRING( /SVG_SVG/SVG_G[2] ), "class", "SlideGroup");
        assertXPath(svgDoc, MAKE_PATH_STRING( /SVG_SVG/SVG_G[2]/SVG_G/SVG_G/SVG_G ), "class", "Slide");
        assertXPath(svgDoc, MAKE_PATH_STRING( /SVG_SVG/SVG_G[2]/SVG_G/SVG_G/SVG_G/SVG_G/SVG_G[1] ), "class", "TitleText");
        assertXPath(svgDoc, MAKE_PATH_STRING( /SVG_SVG/SVG_G[2]/SVG_G/SVG_G/SVG_G/SVG_G/SVG_G[1]/SVG_G/SVG_TEXT ), "class", "TextShape");
        assertXPath(svgDoc, MAKE_PATH_STRING( /SVG_SVG/SVG_G[2]/SVG_G/SVG_G/SVG_G/SVG_G/SVG_G[1]/SVG_G/SVG_TEXT/SVG_TSPAN ), "class", "TextParagraph");
        assertXPath(svgDoc, MAKE_PATH_STRING( /SVG_SVG/SVG_G[2]/SVG_G/SVG_G/SVG_G/SVG_G/SVG_G[1]/SVG_G/SVG_TEXT/SVG_TSPAN ), "text-decoration", "underline");

        assertXPath(svgDoc, MAKE_PATH_STRING( /SVG_SVG/SVG_G[2]/SVG_G/SVG_G/SVG_G/SVG_G/SVG_G[2]/SVG_G/SVG_TEXT ), "class", "TextShape");
        assertXPath(svgDoc, MAKE_PATH_STRING( /SVG_SVG/SVG_G[2]/SVG_G/SVG_G/SVG_G/SVG_G/SVG_G[2]/SVG_G/SVG_TEXT/SVG_TSPAN ), "class", "TextParagraph");
        assertXPath(svgDoc, MAKE_PATH_STRING( /SVG_SVG/SVG_G[2]/SVG_G/SVG_G/SVG_G/SVG_G/SVG_G[2]/SVG_G/SVG_TEXT/SVG_TSPAN ), "text-decoration", "line-through");
    }

    void testSVGExportJavascriptURL()
    {
        executeExport("textbox-link-javascript.odp");

        xmlDocPtr svgDoc = parseXml(maTempFile);
        CPPUNIT_ASSERT(svgDoc);

        // There should be only one child (no link to javascript url)
        assertXPathChildren(svgDoc,
                            MAKE_PATH_STRING(/ SVG_SVG / SVG_G[2] / SVG_G / SVG_G / SVG_G / SVG_G
                                             / SVG_G[4] / SVG_G),
                            1);
    }

    CPPUNIT_TEST_SUITE(SdSVGFilterTest);
    CPPUNIT_TEST(testSVGExportTextDecorations);
    CPPUNIT_TEST(testSVGExportJavascriptURL);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SdSVGFilterTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
