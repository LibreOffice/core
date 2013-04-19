/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test/unoapi_test.hxx"

#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <comphelper/processfactory.hxx>

UnoApiTest::UnoApiTest(const OUString& path)
      : m_aBaseString(path)
{
}

void UnoApiTest::setUp()
{
    test::BootstrapFixture::setUp();

    // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xCalcComponent =
        getMultiServiceFactory()->createInstance(OUString("com.sun.star.comp.Calc.SpreadsheetDocument"));
    CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xCalcComponent.is());
    mxDesktop = com::sun::star::frame::Desktop::create( comphelper::getComponentContext(getMultiServiceFactory()) );
}

void UnoApiTest::tearDown()
{
    uno::Reference< lang::XComponent >( m_xCalcComponent, UNO_QUERY_THROW )->dispose();
    test::BootstrapFixture::tearDown();
}

void UnoApiTest::createFileURL(const OUString& aFileBase, OUString& rFilePath)
{
    rFilePath = getSrcRootURL() + m_aBaseString + "/" + aFileBase;
}

void UnoApiTest::closeDocument( uno::Reference< lang::XComponent > xDocument )
{
    uno::Reference< util::XCloseable > xCloseable(xDocument, UNO_QUERY_THROW);
    xCloseable->close(false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
