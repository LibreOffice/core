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
    : mbSkipValidation(false)
    , m_aBaseString(std::move(path))
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

OUString UnoApiTest::createFileURL(std::u16string_view aFileBase)
{
    return m_directories.getSrcRootURL() + m_aBaseString + "/" + aFileBase;
}

OUString UnoApiTest::loadFromURL(std::u16string_view aFileBase)
{
    if (mxComponent.is())
    {
        mxComponent->dispose();
        mxComponent.clear();
    }

    OUString aFileName = createFileURL(aFileBase);
    mxComponent = loadFromDesktop(aFileName);
    return aFileName;
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
    xStorable->storeToURL(aTempFile.GetURL(), aArgs);

    if (!mbSkipValidation)
    {
        if (rFilter == "Office Open XML Text")
            validate(aTempFile.GetFileName(), test::OOXML);
        else if (rFilter == "Calc Office Open XML")
            validate(aTempFile.GetFileName(), test::OOXML);
        else if (rFilter == "Impress Office Open XML")
            validate(aTempFile.GetFileName(), test::OOXML);
        else if (rFilter == "writer8")
            validate(aTempFile.GetFileName(), test::ODF);
        else if (rFilter == "calc8")
            validate(aTempFile.GetFileName(), test::ODF);
        else if (rFilter == "impress8")
            validate(aTempFile.GetFileName(), test::ODF);
        else if (rFilter == "draw8")
            validate(aTempFile.GetFileName(), test::ODF);
        else if (rFilter == "OpenDocument Text Flat XML")
            validate(aTempFile.GetFileName(), test::ODF);
        else if (rFilter == "MS Word 97")
            validate(aTempFile.GetFileName(), test::MSBINARY);
        else if (rFilter == "MS Excel 97")
            validate(aTempFile.GetFileName(), test::MSBINARY);
        else if (rFilter == "MS PowerPoint 97")
            validate(aTempFile.GetFileName(), test::MSBINARY);
    }

    return aTempFile;
}

utl::TempFileNamed UnoApiTest::saveAndClose(const OUString& rFilter)
{
    utl::TempFileNamed aTempFile = save(rFilter);

    mxComponent->dispose();
    mxComponent.clear();

    return aTempFile;
}

utl::TempFileNamed UnoApiTest::saveAndReload(const OUString& rFilter)
{
    utl::TempFileNamed aTempFile = saveAndClose(rFilter);

    mxComponent = loadFromDesktop(aTempFile.GetURL());

    return aTempFile;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
