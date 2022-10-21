/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>

#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <utility>

using namespace css;
using namespace css::uno;

UnoApiTest::UnoApiTest(OUString path)
    : m_aBaseString(std::move(path))
{
}

void UnoApiTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop
        = css::frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory()));
    CPPUNIT_ASSERT_MESSAGE("no desktop!", mxDesktop.is());
    SfxApplication::GetOrCreate();
}

void UnoApiTest::tearDown()
{
    if (mxComponent.is())
    {
        mxComponent->dispose();
        mxComponent.clear();
    }

    test::BootstrapFixture::tearDown();
}

void UnoApiTest::createFileURL(std::u16string_view aFileBase, OUString& rFilePath)
{
    rFilePath = m_directories.getSrcRootURL() + m_aBaseString + "/" + aFileBase;
}

uno::Any UnoApiTest::executeMacro(const OUString& rScriptURL,
                                  const uno::Sequence<uno::Any>& rParams)
{
    uno::Any aRet;
    uno::Sequence<sal_Int16> aOutParamIndex;
    uno::Sequence<uno::Any> aOutParam;

    ErrCode result = SfxObjectShell::CallXScript(mxComponent, rScriptURL, rParams, aRet,
                                                 aOutParamIndex, aOutParam);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, result);

    return aRet;
}

utl::TempFileNamed UnoApiTest::save(const OUString& rFilter)
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

void UnoApiTest::saveAndReload(const OUString& rFilter)
{
    utl::TempFileNamed aTempFile = save(rFilter);

    mxComponent = loadFromDesktop(aTempFile.GetURL());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
