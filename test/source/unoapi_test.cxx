/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>

#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <unotools/mediadescriptor.hxx>
#include <utility>

using namespace css;
using namespace css::uno;

UnoApiTest::UnoApiTest(OUString path)
    : mbSkipValidation(false)
    , m_aBaseString(std::move(path))
{
    maTempFile.EnableKillingFile();
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
    if (mxComponent2.is())
    {
        mxComponent2->dispose();
        mxComponent2.clear();
    }

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

OUString UnoApiTest::createFilePath(std::u16string_view aFileBase)
{
    return m_directories.getSrcRootPath() + "/" + m_aBaseString + "/" + aFileBase;
}

void UnoApiTest::setTestInteractionHandler(const char* pPassword,
                                           std::vector<beans::PropertyValue>& rFilterOptions)
{
    OUString sPassword = OUString::createFromAscii(pPassword);
    auto& rPropertyValue = rFilterOptions.emplace_back();
    xInteractionHandler
        = rtl::Reference<TestInteractionHandler>(new TestInteractionHandler(sPassword));
    css::uno::Reference<task::XInteractionHandler2> const xInteraction(xInteractionHandler);
    rPropertyValue.Name = "InteractionHandler";
    rPropertyValue.Value <<= xInteraction;
}

void UnoApiTest::load(OUString const& rURL, const char* pPassword)
{
    std::vector<beans::PropertyValue> aFilterOptions;

    if (pPassword)
    {
        setTestInteractionHandler(pPassword, aFilterOptions);
    }

    if (!maImportFilterOptions.isEmpty())
    {
        beans::PropertyValue aValue;
        aValue.Name = "FilterOptions";
        aValue.Value <<= maImportFilterOptions;
        aFilterOptions.push_back(aValue);
    }

    if (!maImportFilterName.isEmpty())
    {
        beans::PropertyValue aValue;
        aValue.Name = "FilterName";
        aValue.Value <<= maImportFilterName;
        aFilterOptions.push_back(aValue);
    }

    loadWithParams(rURL, comphelper::containerToSequence(aFilterOptions));

    if (pPassword)
    {
        CPPUNIT_ASSERT_MESSAGE("Password set but not requested",
                               xInteractionHandler->wasPasswordRequested());
    }
}

void UnoApiTest::loadWithParams(OUString const& rURL,
                                const uno::Sequence<beans::PropertyValue>& rParams)
{
    if (mxComponent.is())
    {
        mxComponent->dispose();
        mxComponent.clear();
    }

    mxComponent = loadFromDesktop(rURL, OUString(), rParams);
}

OUString UnoApiTest::loadFromFile(std::u16string_view aFileBase, const char* pPassword)
{
    OUString aFileName = createFileURL(aFileBase);
    load(aFileName, pPassword);
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

void UnoApiTest::save(const OUString& rFilter, const char* pPassword)
{
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor[u"FilterName"_ustr] <<= rFilter;
    if (!maFilterOptions.isEmpty())
        aMediaDescriptor[u"FilterOptions"_ustr] <<= maFilterOptions;

    if (pPassword)
    {
        if (rFilter != "Office Open XML Text" && rFilter != "Calc Office Open XML"
            && rFilter != "Impress Office Open XML")
        {
            aMediaDescriptor[u"Password"_ustr] <<= OUString::createFromAscii(pPassword);
        }
        else
        {
            OUString sPassword = OUString::createFromAscii(pPassword);
            uno::Sequence<beans::NamedValue> aEncryptionData{
                { u"CryptoType"_ustr, uno::Any(u"Standard"_ustr) },
                { u"OOXPassword"_ustr, uno::Any(sPassword) }
            };
            aMediaDescriptor[utl::MediaDescriptor::PROP_ENCRYPTIONDATA] <<= aEncryptionData;

            // validation fails with "zip END header not found"
            skipValidation();
        }
    }

    saveWithParams(aMediaDescriptor.getAsConstPropertyValueList());

    if (!mbSkipValidation)
    {
        if (rFilter == "Calc Office Open XML")
            validate(maTempFile.GetFileName(), test::OOXML);
        /*
        // too many validation errors right now
        else if (rFilter == "Office Open XML Text")
            validate(maTempFile.GetFileName(), test::OOXML);
        */
        else if (rFilter == "Impress Office Open XML")
            validate(maTempFile.GetFileName(), test::OOXML);
        else if (rFilter == "writer8")
            validate(maTempFile.GetFileName(), test::ODF);
        else if (rFilter == "calc8")
            validate(maTempFile.GetFileName(), test::ODF);
        else if (rFilter == "impress8")
            validate(maTempFile.GetFileName(), test::ODF);
        else if (rFilter == "draw8")
            validate(maTempFile.GetFileName(), test::ODF);
        else if (rFilter == "OpenDocument Text Flat XML")
            validate(maTempFile.GetFileName(), test::ODF);
        else if (rFilter == "MS Word 97")
            validate(maTempFile.GetFileName(), test::MSBINARY);
        else if (rFilter == "MS Excel 97")
            validate(maTempFile.GetFileName(), test::MSBINARY);
        else if (rFilter == "MS PowerPoint 97")
            validate(maTempFile.GetFileName(), test::MSBINARY);
    }
}

void UnoApiTest::saveWithParams(const uno::Sequence<beans::PropertyValue>& rParams)
{
    css::uno::Reference<frame::XStorable> xStorable(mxComponent, css::uno::UNO_QUERY_THROW);
    xStorable->storeToURL(maTempFile.GetURL(), rParams);
}

void UnoApiTest::saveAndReload(const OUString& rFilter, const char* pPassword)
{
    save(rFilter, pPassword);

    load(maTempFile.GetURL(), pPassword);
}

std::unique_ptr<vcl::pdf::PDFiumDocument> UnoApiTest::parsePDFExport(const OString& rPassword)
{
    SvFileStream aFile(maTempFile.GetURL(), StreamMode::READ);
    maMemory.WriteStream(aFile);
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
    {
        return nullptr;
    }
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(maMemory.GetData(), maMemory.GetSize(), rPassword);
    CPPUNIT_ASSERT(pPdfDocument);
    return pPdfDocument;
}

void UnoApiTest::createTempCopy(std::u16string_view fileName)
{
    OUString url = createFileURL(fileName);
    auto const e = osl::File::copy(url, maTempFile.GetURL());
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, e);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
