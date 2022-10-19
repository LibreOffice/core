/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <sfx2/objsh.hxx>

using namespace css;
using namespace css::uno;

CalcUnoApiTest::CalcUnoApiTest(const OUString& path)
      : UnoApiTest(path)
{
}

void CalcUnoApiTest::setUp()
{
    UnoApiTest::setUp();

    // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xCalcComponent =
        getMultiServiceFactory()->createInstance("com.sun.star.comp.Calc.SpreadsheetDocument");
    CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xCalcComponent.is());
}

void CalcUnoApiTest::tearDown()
{
    closeDocument(mxComponent);
    uno::Reference< lang::XComponent >( m_xCalcComponent, UNO_QUERY_THROW )->dispose();
    UnoApiTest::tearDown();
}

uno::Any CalcUnoApiTest::executeMacro(const OUString& rScriptURL, const uno::Sequence<uno::Any>& rParams)
{
    uno::Any aRet;
    uno::Sequence<sal_Int16> aOutParamIndex;
    uno::Sequence<uno::Any> aOutParam;

    ErrCode result = SfxObjectShell::CallXScript(
        mxComponent, rScriptURL,
        rParams, aRet, aOutParamIndex, aOutParam);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, result);

    return aRet;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
