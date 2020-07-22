/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */
#include <config_folders.h>

#include <AdditionsDialog.hxx>

#include <rtl/ustrbuf.hxx>
#include <unordered_set>
#include <sal/log.hxx>
#include <strings.hrc>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/inspection/PropertyLineElement.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/strbuf.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <dialmgr.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/mnemonic.hxx>

#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <ucbhelper/content.hxx>
#include <comphelper/simplefileaccessinteraction.hxx>

#include <curl/curl.h>
#include <orcus/json_document_tree.hpp>
#include <orcus/config.hpp>
#include <orcus/pstring.hpp>

using namespace css;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::XComponentContext;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::graphic::GraphicProvider;
using ::com::sun::star::graphic::XGraphicProvider;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::graphic::XGraphic;
using namespace com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::beans;

#ifdef UNX
const char kUserAgent[] = "LibreOffice AdditionsDownloader/1.0 (Linux)";
#else
const char kUserAgent[] = "LibreOffice AdditionsDownloader/1.0 (unknown platform)";
#endif

namespace
{
struct AdditionInfo
{
    OUString sExtensionID;
    OUString sName;
    OUString sAuthorName;
    OUString sExtensionURL;
    OUString sScreenshotURL;
    OUString sIntroduction;
    OUString sDescription;
    OUString sCompatibleVersion;
    OUString sReleaseVersion;
    OUString sLicense;
    OUString sCommentNumber;
    OUString sCommentURL;
    OUString sRating;
    OUString sDownloadNumber;
    OUString sDownloadURL;
};

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

// Gets the content of the given URL and returns as a standard string
std::string curlGet(const OString& rURL)
{
    CURL* curl = curl_easy_init();

    if (!curl)
        return std::string();

    curl_easy_setopt(curl, CURLOPT_URL, rURL.getStr());

    std::string response_body;

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, static_cast<void*>(&response_body));

    CURLcode cc = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    if (http_code != 200)
    {
        SAL_WARN("cui.dialogs", "Download failed. Error code: " << http_code);
    }

    if (cc != CURLE_OK)
    {
        SAL_WARN("cui.dialogs", "curl error: " << cc);
    }

    return response_body;
}

// Downloads and saves the file at the given rURL to a local path (sFileURL)
void curlDownload(const OString& rURL, const OUString& sFileURL)
{
    CURL* curl = curl_easy_init();
    SvFileStream aFile(sFileURL, StreamMode::WRITE);

    if (!curl)
        return;

    curl_easy_setopt(curl, CURLOPT_URL, rURL.getStr());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, kUserAgent);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackFile);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, static_cast<void*>(&aFile));

    CURLcode cc = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    if (http_code != 200)
    {
        SAL_WARN("cui.dialogs", "Download failed. Error code: " << http_code);
    }

    if (cc != CURLE_OK)
    {
        SAL_WARN("cui.dialogs", "curl error: " << cc);
    }
}

void parseResponse(const std::string& rResponse, std::vector<AdditionInfo>& aAdditions)
{
    orcus::json::document_tree aJsonDoc;
    orcus::json_config aConfig;

    if (rResponse.empty())
        return;

    aJsonDoc.load(rResponse, aConfig);

    auto aDocumentRoot = aJsonDoc.get_document_root();
    if (aDocumentRoot.type() != orcus::json::node_t::object)
    {
        SAL_WARN("cui.dialogs", "invalid root entries: " << rResponse);
        return;
    }

    auto resultsArray = aDocumentRoot.child("extension");

    for (size_t i = 0; i < resultsArray.child_count(); ++i)
    {
        auto arrayElement = resultsArray.child(i);

        try
        {
            AdditionInfo aNewAddition = {
                OStringToOUString(OString(arrayElement.child("id").string_value().get()),
                                  RTL_TEXTENCODING_UTF8),
                OStringToOUString(OString(arrayElement.child("name").string_value().get()),
                                  RTL_TEXTENCODING_UTF8),
                OStringToOUString(OString(arrayElement.child("author").string_value().get()),
                                  RTL_TEXTENCODING_UTF8),
                OStringToOUString(OString(arrayElement.child("url").string_value().get()),
                                  RTL_TEXTENCODING_UTF8),
                OStringToOUString(OString(arrayElement.child("screenshotURL").string_value().get()),
                                  RTL_TEXTENCODING_UTF8),
                OStringToOUString(
                    OString(arrayElement.child("extensionIntroduction").string_value().get()),
                    RTL_TEXTENCODING_UTF8),
                OStringToOUString(
                    OString(arrayElement.child("extensionDescription").string_value().get()),
                    RTL_TEXTENCODING_UTF8),
                OStringToOUString(OString(arrayElement.child("releases")
                                              .child(0)
                                              .child("compatibility")
                                              .string_value()
                                              .get()),
                                  RTL_TEXTENCODING_UTF8),
                OStringToOUString(OString(arrayElement.child("releases")
                                              .child(0)
                                              .child("releaseName")
                                              .string_value()
                                              .get()),
                                  RTL_TEXTENCODING_UTF8),
                OStringToOUString(OString(arrayElement.child("releases")
                                              .child(0)
                                              .child("license")
                                              .string_value()
                                              .get()),
                                  RTL_TEXTENCODING_UTF8),
                OStringToOUString(OString(arrayElement.child("commentNumber").string_value().get()),
                                  RTL_TEXTENCODING_UTF8),
                OStringToOUString(OString(arrayElement.child("commentURL").string_value().get()),
                                  RTL_TEXTENCODING_UTF8),
                OStringToOUString(OString(arrayElement.child("rating").string_value().get()),
                                  RTL_TEXTENCODING_UTF8),
                OStringToOUString(
                    OString(arrayElement.child("downloadNumber").string_value().get()),
                    RTL_TEXTENCODING_UTF8),
                OStringToOUString(OString(arrayElement.child("releases")
                                              .child(0)
                                              .child("downloadURL")
                                              .string_value()
                                              .get()),
                                  RTL_TEXTENCODING_UTF8)
            };

            aAdditions.push_back(aNewAddition);
        }
        catch (orcus::json::document_error& e)
        {
            // This usually happens when one of the values is null (type() == orcus::json::node_t::null)
            // TODO: Allow null values in additions.
            SAL_WARN("cui.dialogs", "Additions JSON parse error: " << e.what());
        }
    }
}

bool getPreviewFile(const AdditionInfo& aAdditionInfo, OUString& sPreviewFile)
{
    uno::Reference<ucb::XSimpleFileAccess3> xFileAccess
        = ucb::SimpleFileAccess::create(comphelper::getProcessComponentContext());
    Reference<XComponentContext> xContext(::comphelper::getProcessComponentContext());

    // copy the images to the user's additions folder
    OUString userFolder = "${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER
                          "/" SAL_CONFIGFILE("bootstrap") "::UserInstallation}";
    rtl::Bootstrap::expandMacros(userFolder);
    userFolder += "/user/additions/" + aAdditionInfo.sExtensionID + "/";

    OUString aPreviewFile(INetURLObject(aAdditionInfo.sScreenshotURL).getName());
    OString aPreviewURL = OUStringToOString(aAdditionInfo.sScreenshotURL, RTL_TEXTENCODING_UTF8);

    try
    {
        osl::Directory::createPath(userFolder);

        if (!xFileAccess->exists(userFolder + aPreviewFile))
            curlDownload(aPreviewURL, userFolder + aPreviewFile);
    }
    catch (const uno::Exception&)
    {
        return false;
    }
    sPreviewFile = userFolder + aPreviewFile;
    return true;
}

} // End of the anonymous namespace

SearchAndParseThread::SearchAndParseThread(AdditionsDialog* pDialog, const OUString& rURL,
                                           const bool& isFirstLoading)
    : Thread("cuiAdditionsSearchThread")
    , m_pAdditionsDialog(pDialog)
    , m_aURL(rURL)
    , m_bExecute(true)
    , m_bIsFirstLoading(isFirstLoading)
{
}

SearchAndParseThread::~SearchAndParseThread() {}

void SearchAndParseThread::execute()
{
    //m_pAdditionsDialog->ClearSearchResults();
    OUString sProgress;
    if (m_bIsFirstLoading)
        sProgress = CuiResId(RID_SVXSTR_ADDITIONS_LOADING);
    else
        sProgress = CuiResId(RID_SVXSTR_ADDITIONS_SEARCHING);

    m_pAdditionsDialog->SetProgress(sProgress);

    uno::Reference<ucb::XSimpleFileAccess3> xFileAccess
        = ucb::SimpleFileAccess::create(comphelper::getProcessComponentContext());
    OString rURL = OUStringToOString(m_aURL, RTL_TEXTENCODING_UTF8);
    std::string sResponse = curlGet(rURL);

    std::vector<AdditionInfo> additionInfos;

    parseResponse(sResponse, additionInfos);

    if (additionInfos.empty())
    {
        sProgress = CuiResId(RID_SVXSTR_ADDITIONS_NORESULTS);
        m_pAdditionsDialog->SetProgress(sProgress);
        return;
    }
    else
    {
        //Get Preview Files
        sal_Int32 i = 0;
        for (const auto& additionInfo : additionInfos)
        {
            if (!m_bExecute)
                return;

            OUString aPreviewFile;
            bool bResult = getPreviewFile(additionInfo, aPreviewFile);

            if (!bResult)
            {
                SAL_INFO("cui.dialogs",
                         "Couldn't get the preview file. Skipping: " << aPreviewFile);
                continue;
            }

            SolarMutexGuard aGuard;
            m_pAdditionsDialog->m_aAdditionsItems.emplace_back(
                m_pAdditionsDialog->m_xContentGrid.get());
            AdditionsItem& aCurrentItem = m_pAdditionsDialog->m_aAdditionsItems.back();

            sal_Int32 nGridPositionY = i++;
            aCurrentItem.m_xContainer->set_grid_left_attach(0);
            aCurrentItem.m_xContainer->set_grid_top_attach(nGridPositionY);

            aCurrentItem.m_xLinkButtonName->set_label(additionInfo.sName);
            aCurrentItem.m_xLinkButtonName->set_uri(additionInfo.sExtensionURL);
            aCurrentItem.m_xLabelDescription->set_label(additionInfo.sIntroduction);
            aCurrentItem.m_xLabelAuthor->set_label(additionInfo.sAuthorName);
            aCurrentItem.m_xButtonInstall->set_label(CuiResId(RID_SVXSTR_ADDITIONS_INSTALLBUTTON));
            OUString sLicenseString
                = CuiResId(RID_SVXSTR_ADDITIONS_LICENCE) + additionInfo.sLicense;
            aCurrentItem.m_xLabelLicense->set_label(sLicenseString);
            OUString sVersionString
                = CuiResId(RID_SVXSTR_ADDITIONS_REQUIREDVERSION) + additionInfo.sCompatibleVersion;
            aCurrentItem.m_xLabelVersion->set_label(sVersionString);
            aCurrentItem.m_xLinkButtonComments->set_label(additionInfo.sCommentNumber);
            aCurrentItem.m_xLinkButtonComments->set_uri(additionInfo.sCommentURL);
            aCurrentItem.m_xLabelDownloadNumber->set_label(additionInfo.sDownloadNumber);

            GraphicFilter aFilter;
            Graphic aGraphic;

            INetURLObject aURLObj(aPreviewFile);

            // This block may be added according to need
            /*OUString aAdditionsSetting = additionInfo.sSlug
                    + ";" + additionInfo.sName
                    + ";" + additionInfo.sPreviewURL
                    + ";" + additionInfo.sHeaderURL
                    + ";" + additionInfo.sFooterURL
                    + ";" + additionInfo.sTextColor;

            m_pAdditionsDialog->AddPersonaSetting( aPersonaSetting );
            */

            // for VCL to be able to create bitmaps / do visual changes in the thread
            aFilter.ImportGraphic(aGraphic, aURLObj);
            BitmapEx aBmp = aGraphic.GetBitmapEx();

            ScopedVclPtr<VirtualDevice> xVirDev
                = aCurrentItem.m_xImageScreenshot->create_virtual_device();
            xVirDev->SetOutputSizePixel(aBmp.GetSizePixel());
            xVirDev->DrawBitmapEx(Point(0, 0), aBmp);

            aCurrentItem.m_xImageScreenshot->set_image(xVirDev.get());
            xVirDev.disposeAndClear();
        }
    }

    if (!m_bExecute)
        return;

    SolarMutexGuard aGuard;
    sProgress.clear();
    m_pAdditionsDialog->SetProgress(sProgress);
}

AdditionsDialog::AdditionsDialog(weld::Window* pParent, const OUString& sAdditionsTag)
    : GenericDialogController(pParent, "cui/ui/additionsdialog.ui", "AdditionsDialog")
    , m_aSearchDataTimer("SearchDataTimer")
    , m_xEntrySearch(m_xBuilder->weld_entry("entrySearch"))
    , m_xButtonClose(m_xBuilder->weld_button("buttonClose"))
    , m_xMenuButtonSettings(m_xBuilder->weld_menu_button("buttonGear"))
    , m_xContentWindow(m_xBuilder->weld_scrolled_window("contentWindow"))
    , m_xContentGrid(m_xBuilder->weld_container("contentGrid"))
    , m_xLabelProgress(m_xBuilder->weld_label("labelProgress"))
{
    m_aSearchDataTimer.SetInvokeHandler(LINK(this, AdditionsDialog, ImplUpdateDataHdl));
    m_aSearchDataTimer.SetDebugName("AdditionsDialog SearchDataTimer");
    m_aSearchDataTimer.SetTimeout(EDIT_UPDATEDATA_TIMEOUT);

    m_xEntrySearch->connect_changed(LINK(this, AdditionsDialog, SearchUpdateHdl));
    m_xEntrySearch->connect_focus_out(LINK(this, AdditionsDialog, FocusOut_Impl));

    m_xButtonClose->connect_clicked(LINK(this, AdditionsDialog, CloseButtonHdl));

    m_sTag = OUStringToOString(sAdditionsTag, RTL_TEXTENCODING_UTF8);

    OUString titlePrefix = CuiResId(RID_SVXSTR_ADDITIONS_DIALOG_TITLE_PREFIX);
    if (!m_sTag.isEmpty())
    {
        this->set_title(titlePrefix + ": " + sAdditionsTag);
    }
    else
    {
        this->set_title(titlePrefix);
        m_sTag = "allextensions"; // Means empty parameter
    }
    //FIXME: Temporary URL
    OString sPrefixURL = "https://yusufketen.com/api/";
    OString sSuffixURL = ".json";
    OString rURL = sPrefixURL + m_sTag + sSuffixURL;

    m_pSearchThread
        = new SearchAndParseThread(this, OStringToOUString(rURL, RTL_TEXTENCODING_UTF8), true);
    m_pSearchThread->launch();
    // fillGrid();
}

AdditionsDialog::~AdditionsDialog()
{
    if (m_pSearchThread.is())
    {
        m_pSearchThread->StopExecution();
        // Release the solar mutex, so the thread is not affected by the race
        // when it's after the m_bExecute check but before taking the solar
        // mutex.
        SolarMutexReleaser aReleaser;
        m_pSearchThread->join();
    }
}

void AdditionsDialog::SetProgress(const OUString& rProgress)
{
    if (rProgress.isEmpty())
        m_xLabelProgress->hide();
    else
    {
        SolarMutexGuard aGuard;
        m_xLabelProgress->show();
        m_xLabelProgress->set_label(rProgress);
        m_xDialog->resize_to_request(); //TODO
    }
}

void AdditionsDialog::ClearList()
{
    // for VCL to be able to destroy bitmaps
    SolarMutexGuard aGuard;

    for (auto& item : this->m_aAdditionsItems)
    {
        item.m_xContainer->hide();
    }
    this->m_aAdditionsItems.clear();
}

IMPL_LINK_NOARG(AdditionsDialog, ImplUpdateDataHdl, Timer*, void)
{
    this->ClearList();
    OUString aSearchTerm(m_xEntrySearch->get_text());
    /* OPTIONAL
    if (aSearchTerm.isEmpty())
        return;
    */
    if (m_pSearchThread.is())
        m_pSearchThread->StopExecution();

    OString rURL = "https://yusufketen.com/extensionTest.json"; // + q=aSearchTerm
    OUString finalURL = OStringToOUString(rURL + "?q=", RTL_TEXTENCODING_UTF8) + aSearchTerm;

    // Search Test
    if (aSearchTerm == "2")
    {
        rURL = "https://yusufketen.com/extensionTest2.json";
    }

    this->SetProgress(finalURL);
    m_pSearchThread
        = new SearchAndParseThread(this, OStringToOUString(rURL, RTL_TEXTENCODING_UTF8), false);
    m_pSearchThread->launch();
}

IMPL_LINK_NOARG(AdditionsDialog, SearchUpdateHdl, weld::Entry&, void)
{
    m_aSearchDataTimer.Start();
}

IMPL_LINK_NOARG(AdditionsDialog, FocusOut_Impl, weld::Widget&, void)
{
    if (m_aSearchDataTimer.IsActive())
    {
        m_aSearchDataTimer.Stop();
        m_aSearchDataTimer.Invoke();
    }
}

IMPL_LINK_NOARG(AdditionsDialog, CloseButtonHdl, weld::Button&, void)
{
    if (m_pSearchThread.is())
        m_pSearchThread->StopExecution();
    this->response(RET_CLOSE);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
