/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2011 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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
