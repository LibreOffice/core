/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_UNOAPI_TEST_HXX
#define INCLUDED_TEST_UNOAPI_TEST_HXX

#include <sal/config.h>

#include <string_view>

#include <rtl/ref.hxx>
#include <test/bootstrapfixture.hxx>
#include <test/testinteractionhandler.hxx>
#include <unotest/macros_test.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <osl/file.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>

// basic uno api test class

class OOO_DLLPUBLIC_TEST UnoApiTest : public test::BootstrapFixture, public unotest::MacrosTest
{
public:
    UnoApiTest(OUString path);

    virtual void setUp() override;
    virtual void tearDown() override;

    OUString createFileURL(std::u16string_view aFileBase);
    OUString createFilePath(std::u16string_view aFileBase);
    void load(const OUString& rURL, const char* pPassword = nullptr);
    OUString loadFromURL(std::u16string_view aFileBase, const char* pPassword = nullptr);

    css::uno::Any executeMacro(const OUString& rScriptURL,
                               const css::uno::Sequence<css::uno::Any>& rParams = {});

    void save(const OUString& rFilter, const char* pPassword = nullptr);
    void saveAndReload(const OUString& rFilter, const char* pPassword = nullptr);

    std::unique_ptr<vcl::pdf::PDFiumDocument> parsePDFExport(const OString& rPassword = OString());

    void createTempCopy(std::u16string_view fileName);

    void skipValidation() { mbSkipValidation = true; }
    void setFilterOptions(const OUString& rFilterOptions) { maFilterOptions = rFilterOptions; }

    void setImportFilterOptions(const OUString& rFilterOptions)
    {
        maImportFilterOptions = rFilterOptions;
    }

    void setImportFilterName(const OUString& rFilterName) { maImportFilterName = rFilterName; }

protected:
    // reference to document component that we are testing
    css::uno::Reference<css::lang::XComponent> mxComponent;

    // In case the test needs to work with two documents at the same time
    css::uno::Reference<css::lang::XComponent> mxComponent2;

    utl::TempFileNamed maTempFile;

    SvMemoryStream maMemory; // Underlying memory for parsed PDF files.

    rtl::Reference<TestInteractionHandler> xInteractionHandler;

private:
    void
    setTestInteractionHandler(const char* pPassword,
                              std::vector<com::sun::star::beans::PropertyValue>& rFilterOptions);

    bool mbSkipValidation;
    OUString m_aBaseString;
    OUString maFilterOptions;

    OUString maImportFilterOptions;
    OUString maImportFilterName;
};

#endif // INCLUDED_TEST_UNOAPI_TEST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
