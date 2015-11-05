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

using namespace css;

class SdSVGFilterTest : public SdModelTestBase, public XmlTestTools
{
    uno::Reference<lang::XComponent> mxComponent;
    utl::TempFile maTempFile;

public:
    SdSVGFilterTest()
    {
        maTempFile.EnableKillingFile();
    }

    virtual void setUp() override
    {
        SdModelTestBase::setUp();

        mxDesktop.set(css::frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));
    }

    virtual void tearDown() override
    {
        if (mxComponent.is())
            mxComponent->dispose();

        SdModelTestBase::tearDown();
    }

    void testSVGExportTextDecorations()
    {
        mxComponent = loadFromDesktop(getURLFromSrc("/sd/qa/unit/data/odp/svg-export-text-decorations.odp"));
        CPPUNIT_ASSERT(mxComponent.is());

        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
        utl::MediaDescriptor aMediaDescriptor;
        OUString aFilterName("impress_svg_Export");
        aMediaDescriptor["FilterName"] <<= aFilterName;
        xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
        xmlDocPtr pXmlDoc = parseXml(maTempFile);
        CPPUNIT_ASSERT(pXmlDoc);

        // TODO use assertXPath() here.
    }

    CPPUNIT_TEST_SUITE(SdSVGFilterTest);
    CPPUNIT_TEST(testSVGExportTextDecorations);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SdSVGFilterTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
