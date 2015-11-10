/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <test/bootstrapfixture.hxx>

#include <xmloff/xmlexp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlaustp.hxx>
#include "SchXMLExport.hxx"
#include "XMLChartPropertySetMapper.hxx"
#include "impastpl.hxx"
#include <comphelper/processfactory.hxx>
#include <com/sun/star/util/MeasureUnit.hpp>

using namespace ::xmloff::token;
using namespace ::com::sun::star;

class Test : public test::BootstrapFixture {
public:
    Test();

    virtual void setUp() override;
    virtual void tearDown() override;

    void testAutoStylePool();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testAutoStylePool);
    CPPUNIT_TEST_SUITE_END();
private:
    SvXMLExport *pExport;
};

Test::Test()
    : pExport( nullptr )
{
}

void Test::setUp()
{
    BootstrapFixture::setUp();

    pExport = new SchXMLExport(
        comphelper::getProcessComponentContext(), "SchXMLExport.Compact",
        SvXMLExportFlags::ALL);
}

void Test::tearDown()
{
    delete pExport;
    BootstrapFixture::tearDown();
}

void Test::testAutoStylePool()
{
    rtl::Reference< SvXMLAutoStylePoolP > xPool(
        new SvXMLAutoStylePoolP( *pExport ) );
    rtl::Reference< XMLPropertySetMapper > xSetMapper(
        new XMLChartPropertySetMapper( true) );
    rtl::Reference< XMLChartExportPropertyMapper > xExportPropMapper(
        new XMLChartExportPropertyMapper( xSetMapper, *pExport ) );

    xPool->AddFamily( XML_STYLE_FAMILY_TEXT_PARAGRAPH,
                      GetXMLToken( XML_PARAGRAPH ),
                      xExportPropMapper.get(),
                      OUString( "Bob" ) );

    std::vector< XMLPropertyState > aProperties;
    OUString aName = xPool->Add( XML_STYLE_FAMILY_TEXT_PARAGRAPH, "", aProperties );

    // not that interesting but worth checking
    bool bHack = (getenv("LIBO_ONEWAY_STABLE_ODF_EXPORT") != nullptr);
    if (bHack)
        CPPUNIT_ASSERT_MESSAGE( "style / naming changed", aName == "Bob" );
    else
        CPPUNIT_ASSERT_MESSAGE( "style / naming changed", aName == "Bob1" );

    // find ourselves again:
    OUString aSameName = xPool->Find( XML_STYLE_FAMILY_TEXT_PARAGRAPH, "", aProperties );
    CPPUNIT_ASSERT_MESSAGE( "same style not found", aSameName == aName );
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
