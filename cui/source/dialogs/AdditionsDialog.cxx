/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#include <AdditionsDialog.hxx>
#include <curl/curl.h>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/deployment/ExtensionManager.hpp>
#include <com/sun/star/deployment/XExtensionManager.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <sal/log.hxx>
#include <sal/config.h>
#include <osl/file.hxx>
#include <unotools/tempfile.hxx>
#include <cppuhelper/implbase.hxx>

using namespace comphelper;
using namespace css;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
AdditionsDialog::AdditionsDialog(weld::Widget* pParent)
    : GenericDialogController(pParent, "cui/ui/additionsdialog.ui", "AdditionsDialog")
    , m_xInstallButton(m_xBuilder->weld_button("btnInstall"))
    , m_xExtensionName(m_xBuilder->weld_label("labelExtensionName"))
    , m_xAuthor(m_xBuilder->weld_label("labelAuthor"))
    , m_xIntro(m_xBuilder->weld_label("labelIntro"))
{
    m_xAuthor->set_label("Additions Dialog Test");
    m_xInstallButton->connect_clicked(LINK(this, AdditionsDialog, InstallButtonTestHdl));
}

AdditionsDialog::~AdditionsDialog() {}

// Callback to get the response data from server.
size_t WriteCallback(void* ptr, size_t size, size_t nmemb, void* userp)
{
    if (!userp)
        return 0;

    std::string* response = static_cast<std::string*>(userp);
    size_t real_size = size * nmemb;
    response->append(static_cast<char*>(ptr), real_size);
    return real_size;
}

// Callback to get the response data from server to a file.
size_t WriteCallbackFile(void* ptr, size_t size, size_t nmemb, void* userp)
{
    if (!userp)
        return 0;

    SvStream* response = static_cast<SvStream*>(userp);
    size_t real_size = size * nmemb;
    response->WriteBytes(ptr, real_size);
    return real_size;
}

// Downloads and saves the file at the given rURL to a local path (sFileURL)
void curlDownload(const OString& rURL, const OUString& sFileURL)
{
    CURL* curl = curl_easy_init();
    SvFileStream aFile(sFileURL, StreamMode::WRITE);

    if (!curl)
        return;

    curl_easy_setopt(curl, CURLOPT_URL, rURL.getStr());
    //curl_easy_setopt(curl, CURLOPT_USERAGENT, kUserAgent);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackFile);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, static_cast<void*>(&aFile));

    CURLcode cc = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    if (http_code != 200)
    {
        SAL_WARN("cui.options", "Download failed. Error code: " << http_code);
    }

    if (cc != CURLE_OK)
    {
        SAL_WARN("cui.options", "curl error: " << cc);
    }
}

IMPL_LINK_NOARG(AdditionsDialog, InstallButtonTestHdl, weld::Button&, void)
{
    m_xExtensionName->set_label("Additions Dialog Test is successfully completed!");
    // create content from current extension configuration
    uno::Sequence<uno::Sequence<uno::Reference<deployment::XPackage>>> xAllPackages;
    uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
    uno::Reference<deployment::XExtensionManager> m_xExtensionManager
        = deployment::ExtensionManager::get(xContext);

    utl::TempFile aTempInput;
    const OUString aExtensionTempLocation = aTempInput.GetTempNameBaseDirectory();
    const OString testDownloadURL
        = "https://extensions.libreoffice.org/assets/downloads/z/0ec1c2e8_access2base.oxt";
    const OUString sFileURL = aExtensionTempLocation + "0ec1c2e8_access2base.oxt";
    curlDownload(testDownloadURL, sFileURL);

    bool option_force = false;
    bool option_verbose = false;
    bool option_suppressLicense = false;
    TmpRepositoryCommandEnv* pCmdEnv = new TmpRepositoryCommandEnv();

    uno::Reference<ucb::XCommandEnvironment> xCmdEnv(static_cast<cppu::OWeakObject*>(pCmdEnv),
                                                     uno::UNO_QUERY);
    uno::Reference<task::XAbortChannel> xAbortChannel;

    m_xExtensionManager->addExtension(sFileURL, uno::Sequence<beans::NamedValue>(), "user",
                                      xAbortChannel, xCmdEnv);
}
// TmpRepositoryCommandEnv

TmpRepositoryCommandEnv::TmpRepositoryCommandEnv() {}

TmpRepositoryCommandEnv::~TmpRepositoryCommandEnv() {}
// XCommandEnvironment

uno::Reference<task::XInteractionHandler> TmpRepositoryCommandEnv::getInteractionHandler()
{
    return this;
}

uno::Reference<ucb::XProgressHandler> TmpRepositoryCommandEnv::getProgressHandler() { return this; }

// XInteractionHandler
void TmpRepositoryCommandEnv::handle(uno::Reference<task::XInteractionRequest> const& xRequest)
{
    OSL_ASSERT(xRequest->getRequest().getValueTypeClass() == uno::TypeClass_EXCEPTION);

    bool approve = true;

    // select:
    uno::Sequence<Reference<task::XInteractionContinuation>> conts(xRequest->getContinuations());
    Reference<task::XInteractionContinuation> const* pConts = conts.getConstArray();
    sal_Int32 len = conts.getLength();
    for (sal_Int32 pos = 0; pos < len; ++pos)
    {
        if (approve)
        {
            uno::Reference<task::XInteractionApprove> xInteractionApprove(pConts[pos],
                                                                          uno::UNO_QUERY);
            if (xInteractionApprove.is())
            {
                xInteractionApprove->select();
                // don't query again for ongoing continuations:
                approve = false;
            }
        }
    }
}

// XProgressHandler
void TmpRepositoryCommandEnv::push(uno::Any const& /*Status*/) {}

void TmpRepositoryCommandEnv::update(uno::Any const& /*Status */) {}

void TmpRepositoryCommandEnv::pop() {}
