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

#include <com/sun/star/frame/XStorable.hpp>
#include <comphelper/propertyvalue.hxx>

using namespace css;
using namespace css::uno;

CalcUnoApiTest::CalcUnoApiTest(const OUString& path)
      : UnoApiTest(path)
{
}

void CalcUnoApiTest::tearDown()
{
    if (mxComponent.is())
    {
        mxComponent->dispose();
        mxComponent.clear();
    }

    test::BootstrapFixture::tearDown();
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

utl::TempFileNamed CalcUnoApiTest::save(const OUString& rFilter)
{
    utl::TempFileNamed aTempFile;
    aTempFile.EnableKillingFile();
    uno::Sequence aArgs{ comphelper::makePropertyValue("FilterName", rFilter) };
    css::uno::Reference<frame::XStorable> xStorable(mxComponent, css::uno::UNO_QUERY_THROW);
    xStorable->storeAsURL(aTempFile.GetURL(), aArgs);
    mxComponent->dispose();
    mxComponent.clear();

    return aTempFile;
}

void CalcUnoApiTest::saveAndReload(const OUString& rFilter)
{
    utl::TempFileNamed aTempFile = save(rFilter);

    mxComponent = loadFromDesktop(aTempFile.GetURL(), "com.sun.star.sheet.SpreadsheetDocument");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
