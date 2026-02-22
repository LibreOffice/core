/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_validation.h>

#include <test/unoapi_test.hxx>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <o3tl/string_view.hxx>
#include <osl/file.hxx>
#include <osl/process.h>

#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <unotest/getargument.hxx>
#include <unotools/mediadescriptor.hxx>
#include <utility>

using namespace css;
using namespace css::uno;

UnoApiTest::UnoApiTest(OUString path)
    : mbSkipValidation(false)
    , m_aBaseString(std::move(path))
    , meImportFilterName(TestFilter::NONE)
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
        dispose();

    if (comphelper::LibreOfficeKit::isActive())
        comphelper::LibreOfficeKit::setActive(false);

    test::BootstrapFixture::tearDown();
}

OUString UnoApiTest::createFileURL(std::u16string_view aFileBase)
{
    return m_directories.getURLFromSrc(m_aBaseString, aFileBase);
}

OUString UnoApiTest::createFilePath(std::u16string_view aFileBase)
{
    return m_directories.getPathFromSrc(m_aBaseString, aFileBase);
}

#if HAVE_EXPORT_VALIDATION
namespace
{
OString loadFile(const OUString& rURL)
{
    osl::File aFile(rURL);
    osl::FileBase::RC eStatus = aFile.open(osl_File_OpenFlag_Read);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, eStatus);
    sal_uInt64 nSize;
    aFile.getSize(nSize);
    std::unique_ptr<char[]> aBytes(new char[nSize]);
    sal_uInt64 nBytesRead;
    aFile.read(aBytes.get(), nSize, nBytesRead);
    CPPUNIT_ASSERT_EQUAL(nSize, nBytesRead);
    OString aContent(aBytes.get(), nBytesRead);

    return aContent;
}

constexpr std::u16string_view grand_total = u"Grand total of errors in submitted package: ";
}
#endif

void UnoApiTest::validate(TestFilter eFilter)
{
    ValidationFormat eFormat = ValidationFormat::ODF;
    if (eFilter == TestFilter::XLSX)
        eFormat = ValidationFormat::OOXML;
    else if (eFilter == TestFilter::DOCX)
        eFormat = ValidationFormat::OOXML;
    else if (eFilter == TestFilter::PPTX)
        eFormat = ValidationFormat::OOXML;
    else if (eFilter == TestFilter::ODT)
        eFormat = ValidationFormat::ODF;
    else if (eFilter == TestFilter::ODS)
        eFormat = ValidationFormat::ODF;
    else if (eFilter == TestFilter::ODP)
        eFormat = ValidationFormat::ODF;
    else if (eFilter == TestFilter::ODG)
        eFormat = ValidationFormat::ODF;
    else if (eFilter == TestFilter::DOC)
        eFormat = ValidationFormat::MSBINARY;
    else if (eFilter == TestFilter::XLS)
        eFormat = ValidationFormat::MSBINARY;
    else if (eFilter == TestFilter::PPT)
        eFormat = ValidationFormat::MSBINARY;
    else if (eFilter == TestFilter::PDF_WRITER)
        eFormat = ValidationFormat::PDF;
    else
    {
        SAL_INFO("test", "UnoApiTest::validate: unknown filter");
        return;
    }

#if HAVE_EXPORT_VALIDATION
    OUString var;
    if (eFormat == ValidationFormat::OOXML)
    {
        var = "OFFICEOTRON";
    }
    else if (eFormat == ValidationFormat::ODF)
    {
        var = "ODFVALIDATOR";
    }
    else if (eFormat == ValidationFormat::PDF)
    {
        var = "VERAPDF";
    }
    else if (eFormat == ValidationFormat::MSBINARY)
    {
#if HAVE_BFFVALIDATOR
        var = "BFFVALIDATOR";
#else
        // Binary Format Validator is disabled
        return;
#endif
    }
    OUString aValidator;
    oslProcessError e = osl_getEnvironment(var.pData, &aValidator.pData);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OUString("cannot get env var " + var).toUtf8().getStr(),
                                 osl_Process_E_None, e);
    CPPUNIT_ASSERT_MESSAGE(OUString("empty get env var " + var).toUtf8().getStr(),
                           !aValidator.isEmpty());

    if (eFormat == ValidationFormat::ODF)
    {
        // invoke without -e so that we know when something new is written
        // in loext namespace that isn't yet in the custom schema
        aValidator
            += " -M "
               + m_directories.getPathFromSrc(
                     u"/schema/libreoffice/OpenDocument-v1.4+libreoffice-manifest-schema.rng")
               + " -D "
               + m_directories.getPathFromSrc(
                     u"/schema/libreoffice/OpenDocument-v1.4+libreoffice-dsig-schema.rng")
               + " -O "
               + m_directories.getPathFromSrc(
                     u"/schema/libreoffice/OpenDocument-v1.4+libreoffice-schema.rng")
               + " -m " + m_directories.getPathFromSrc(u"/schema/mathml2/mathml2.xsd");
    }

    utl::TempFileNamed aOutput;
    aOutput.EnableKillingFile();
    OUString aOutputFile = aOutput.GetFileName();
    OUString aCommand = aValidator + " " + maTempFile.GetFileName() + " > " + aOutputFile + " 2>&1";

#if !defined _WIN32
    // For now, this is only needed by some Linux ASan builds, so keep it simply and disable it on
    // Windows (which doesn't support the relevant shell syntax for (un-)setting environment
    // variables).
    OUString env;
    if (test::getArgument(u"env", &env))
    {
        auto const n = env.indexOf('=');
        if (n == -1)
        {
            aCommand = "unset -v " + env + " && " + aCommand;
        }
        else
        {
            aCommand = env + " " + aCommand;
        }
    }
#endif

    SAL_INFO("test", "UnoApiTest::validate: executing '" << aCommand << "'");
    int returnValue = system(OUStringToOString(aCommand, RTL_TEXTENCODING_UTF8).getStr());

    if (eFormat == ValidationFormat::PDF)
    {
        SvMemoryStream aStream;
        SvFileStream aFileStream(aOutput.GetURL(), StreamMode::READ);
        aStream.WriteStream(aFileStream);
        aStream.Seek(0);
        xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
        // Make sure the output is well-formed.
        CPPUNIT_ASSERT(pXmlDoc);
        int nCount = countXPathNodes(pXmlDoc, "/report/jobs/job/validationReport");
        CPPUNIT_ASSERT(nCount);
        for (int i = 1; i <= nCount; ++i)
            assertXPath(pXmlDoc, "/report/jobs/job/validationReport[" + OString::number(i) + "]",
                        "isCompliant", u"true");
    }
    else
    {
        OString aContentString = loadFile(aOutput.GetURL());
        OUString aContentOUString = OStringToOUString(aContentString, RTL_TEXTENCODING_UTF8);

        if (eFormat == ValidationFormat::OOXML && !aContentOUString.isEmpty())
        {
            // check for validation errors here
            sal_Int32 nIndex = aContentOUString.lastIndexOf(grand_total);
            if (nIndex == -1)
            {
                SAL_WARN("test", "no summary line");
            }
            else
            {
                sal_Int32 nStartOfNumber = nIndex + grand_total.size();
                std::u16string_view aNumber = aContentOUString.subView(nStartOfNumber);
                sal_Int32 nErrors = o3tl::toInt32(aNumber);
                OString aMsg
                    = "validation error in OOXML export: Errors: " + OString::number(nErrors);
                if (nErrors)
                {
                    SAL_WARN("test", aContentOUString);
                }
                CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsg.getStr(), sal_Int32(0), nErrors);
            }
        }
        else if (eFormat == ValidationFormat::ODF && !aContentOUString.isEmpty())
        {
            if (aContentOUString.indexOf("Error") != -1 || aContentOUString.indexOf("Fatal") != -1)
            {
                SAL_WARN("test", aContentOUString);
                CPPUNIT_FAIL(aContentString.getStr());
            }
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            OString("failed to execute: " + OUStringToOString(aCommand, RTL_TEXTENCODING_UTF8)
                    + "\n" + OUStringToOString(aContentOUString, RTL_TEXTENCODING_UTF8))
                .getStr(),
            0, returnValue);
    }
#else
    (void)eFormat;
#endif
}

void UnoApiTest::loadFromURL(OUString const& rURL,
                             const uno::Sequence<beans::PropertyValue>& rParams,
                             const char* pPassword)
{
    comphelper::SequenceAsHashMap aMediaDescriptor;

    if (rParams.hasElements())
        aMediaDescriptor.update(rParams);

    if (pPassword)
    {
        OUString sPassword = OUString::createFromAscii(pPassword);
        xInteractionHandler
            = rtl::Reference<TestInteractionHandler>(new TestInteractionHandler(sPassword));
        aMediaDescriptor["InteractionHandler"]
            <<= css::uno::Reference<task::XInteractionHandler2>(xInteractionHandler);
    }

    if (meImportFilterName != TestFilter::NONE)
        aMediaDescriptor[u"FilterName"_ustr] <<= TestFilterNames.at(meImportFilterName);

    CPPUNIT_ASSERT_MESSAGE("A document is already open!", !mxComponent.is());

    mxComponent = loadFromDesktop(rURL, OUString(), aMediaDescriptor.getAsConstPropertyValueList());
    CPPUNIT_ASSERT(mxComponent);

    if (pPassword)
    {
        CPPUNIT_ASSERT_MESSAGE("Password set but not requested",
                               xInteractionHandler->wasPasswordRequested());
    }
}

void UnoApiTest::dispose()
{
    mxComponent->dispose();
    mxComponent.clear();
}

OUString UnoApiTest::loadFromFile(std::u16string_view aFileBase,
                                  const css::uno::Sequence<css::beans::PropertyValue>& rParams,
                                  const char* pPassword)
{
    OUString aFileName = createFileURL(aFileBase);
    loadFromURL(aFileName, rParams, pPassword);
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

void UnoApiTest::save(TestFilter eFilter, const uno::Sequence<beans::PropertyValue>& rParams,
                      const char* pPassword)
{
    OUString aFilter(TestFilterNames.at(eFilter));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterName"_ustr] <<= aFilter;

    if (rParams.hasElements())
        aMediaDescriptor.update(rParams);

    if (pPassword)
    {
        if (eFilter == TestFilter::PDF_WRITER)
        {
            comphelper::SequenceAsHashMap aFilterData;
            if (aMediaDescriptor.contains(u"FilterData"_ustr))
                aFilterData = aMediaDescriptor[u"FilterData"_ustr];
            aFilterData[u"EncryptFile"_ustr] <<= true;
            aFilterData[u"DocumentOpenPassword"_ustr] <<= OUString::createFromAscii(pPassword);
            aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData.getAsConstPropertyValueList();
        }
        else if (eFilter != TestFilter::DOCX && eFilter != TestFilter::XLSX
                 && eFilter != TestFilter::PPTX)
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
        }

        // Password protected documents can't be validated
        skipValidation();
    }

    css::uno::Reference<frame::XStorable> xStorable(mxComponent, css::uno::UNO_QUERY_THROW);
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // FIXME: Don't validate pdf files by default for now
    if (!mbSkipValidation && eFilter != TestFilter::PDF_WRITER)
        validate(eFilter);
}

void UnoApiTest::saveAndReload(TestFilter eFilter,
                               const uno::Sequence<beans::PropertyValue>& rParams,
                               const char* pPassword)
{
    save(eFilter, rParams, pPassword);
    dispose();
    loadFromURL(maTempFile.GetURL(), rParams, pPassword);
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
    if (!pPdfDocument)
    {
        OString aError = OUStringToOString(pPDFium->getLastError(), RTL_TEXTENCODING_UTF8);
        CPPUNIT_ASSERT_MESSAGE(aError.getStr(), pPdfDocument);
    }
    return pPdfDocument;
}

void UnoApiTest::createTempCopy(std::u16string_view fileName)
{
    OUString url = createFileURL(fileName);
    auto const e = osl::File::copy(url, maTempFile.GetURL());
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, e);
}

xmlDocUniquePtr UnoApiTest::parseExport(OUString const& rStreamName)
{
    std::unique_ptr<SvStream> const pStream(parseExportStream(maTempFile.GetURL(), rStreamName));
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    return pXmlDoc;
}

xmlDocUniquePtr UnoApiTest::parseExportedFile()
{
    auto stream(SvFileStream(maTempFile.GetURL(), StreamMode::READ | StreamMode::TEMPORARY));
    return parseXmlStream(&stream);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
