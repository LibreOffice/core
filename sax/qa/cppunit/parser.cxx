/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <com/sun/star/io/Pipe.hpp>
#include <com/sun/star/xml/sax/FastParser.hpp>
#include <com/sun/star/xml/sax/FastTokenHandler.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/XFastParser.hpp>

#include <test/bootstrapfixture.hxx>

using namespace css;
using namespace css::xml::sax;

namespace {

class ParserTest: public test::BootstrapFixture
{
    InputSource maInput;
    uno::Reference< XFastParser > mxParser;
    uno::Reference< XFastDocumentHandler > mxDocumentHandler;

public:
    virtual void setUp();
    virtual void tearDown();

    void parse();

    CPPUNIT_TEST_SUITE(ParserTest);
    CPPUNIT_TEST(parse);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference< io::XInputStream > createStream(const OString& sInput);
};

void ParserTest::setUp()
{
    test::BootstrapFixture::setUp();
    mxParser = css::xml::sax::FastParser::create(m_xContext);
    mxParser->setTokenHandler(
        css::xml::sax::FastTokenHandler::create(m_xContext));
}

void ParserTest::tearDown()
{
    test::BootstrapFixture::tearDown();
}

uno::Reference< io::XInputStream > ParserTest::createStream(const OString& sInput)
{
    uno::Reference< io::XOutputStream > xPipe( io::Pipe::create(m_xContext) );
    uno::Reference< io::XInputStream > xInStream( xPipe, uno::UNO_QUERY );
    uno::Sequence< sal_Int8 > aSeq( (sal_Int8*)sInput.getStr(), sInput.getLength() );
    xPipe->writeBytes( aSeq );
    xPipe->flush();
    xPipe->closeOutput();
    return xInStream;
}

void ParserTest::parse()
{
    maInput.aInputStream = createStream("<a>...<b />..</a>");
    mxParser->parseStream( maInput );

    maInput.aInputStream = createStream("<b></a>");
    bool bException = false;
    try
    {
        mxParser->parseStream( maInput );
    }
    catch (const SAXParseException &)
    {
        bException = true;
    }
    CPPUNIT_ASSERT_MESSAGE("No Exception!", bException);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ParserTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
