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
#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>

#include <cppuhelper/implbase.hxx>
#include <sax/fastparser.hxx>
#include <test/bootstrapfixture.hxx>

using namespace css;
using namespace css::xml::sax;

namespace {

class DummyTokenHandler : public cppu::WeakImplHelper< xml::sax::XFastTokenHandler >
{
public:
             DummyTokenHandler() {}
    virtual ~DummyTokenHandler() {}

    virtual sal_Int32 SAL_CALL getTokenFromUTF8( const uno::Sequence<sal_Int8>& )
        throw (uno::RuntimeException, std::exception) override
    {
        return FastToken::DONTKNOW;
    }
    virtual uno::Sequence< sal_Int8 > SAL_CALL getUTF8Identifier( sal_Int32 )
        throw (uno::RuntimeException, std::exception) override
    {
        CPPUNIT_ASSERT_MESSAGE( "getUTF8Identifier: unexpected call", false );
        return uno::Sequence<sal_Int8>();
    }
};

class ParserTest: public test::BootstrapFixture
{
    InputSource maInput;
    uno::Reference< sax_fastparser::FastSaxParser > mxParser;
    uno::Reference< XFastDocumentHandler > mxDocumentHandler;
    uno::Reference< DummyTokenHandler > mxTokenHandler;

public:
    virtual void setUp() override;
    virtual void tearDown() override;

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
    mxParser->setTokenHandler( mxTokenHandler.get() );
}

void ParserTest::tearDown()
{
    test::BootstrapFixture::tearDown();
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
