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
#include <com/sun/star/xml/sax/FastToken.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>

#include <sax/fastparser.hxx>
#include <sax/fastattribs.hxx>
#include <test/bootstrapfixture.hxx>
#include <rtl/ref.hxx>

using namespace css;
using namespace css::xml::sax;

namespace {

class DummyTokenHandler : public sax_fastparser::FastTokenHandlerBase
{
public:
             DummyTokenHandler() {}

    virtual sal_Int32 SAL_CALL getTokenFromUTF8( const uno::Sequence<sal_Int8>& ) override
    {
        return FastToken::DONTKNOW;
    }
    virtual uno::Sequence< sal_Int8 > SAL_CALL getUTF8Identifier( sal_Int32 ) override
    {
        CPPUNIT_ASSERT_MESSAGE( "getUTF8Identifier: unexpected call", false );
        return uno::Sequence<sal_Int8>();
    }
    virtual sal_Int32 getTokenDirect( const char * /* pToken */, sal_Int32 /* nLength */ ) const override
    {
        return -1;
    }
};

class ParserTest: public test::BootstrapFixture
{
    InputSource maInput;
    rtl::Reference< sax_fastparser::FastSaxParser > mxParser;
    rtl::Reference< DummyTokenHandler > mxTokenHandler;

public:
    virtual void setUp() override;

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
    mxTokenHandler.set( new DummyTokenHandler() );
    mxParser.set( new sax_fastparser::FastSaxParser() );
    mxParser->setTokenHandler( mxTokenHandler );
}

uno::Reference< io::XInputStream > ParserTest::createStream(const OString& sInput)
{
    uno::Reference< io::XOutputStream > xPipe( io::Pipe::create(m_xContext) );
    uno::Reference< io::XInputStream > xInStream( xPipe, uno::UNO_QUERY );
    uno::Sequence< sal_Int8 > aSeq( reinterpret_cast<sal_Int8 const *>(sInput.getStr()), sInput.getLength() );
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
    CPPUNIT_ASSERT_THROW( mxParser->parseStream( maInput ), css::xml::sax::SAXParseException );
}

CPPUNIT_TEST_SUITE_REGISTRATION(ParserTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
