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
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/mnemonic.hxx>

#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/util/SearchAlgorithms2.hpp>
#include <unotools/textsearch.hxx>

#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <ucbhelper/content.hxx>
#include <comphelper/simplefileaccessinteraction.hxx>

#include <com/sun/star/deployment/DeploymentException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/frame/TerminationVetoException.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ucb/CommandFailedException.hpp>

//cURL
#include <curl/curl.h>
#include <orcus/json_document_tree.hpp>
#include <orcus/config.hpp>
#include <orcus/pstring.hpp>

// FIXME
// This preprocessor command is written to prevent the error caused by "css::util::SearchAlgorithms2::ABSOLUTE" in the AdditionsDialog().
#ifdef ABSOLUTE
#undef ABSOLUTE
#endif

#define PAGE_SIZE 30

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

void LoadImage(const OUString& rPreviewFile, std::shared_ptr<AdditionsItem> pCurrentItem)
{
    SolarMutexGuard aGuard;

    GraphicFilter aFilter;
    Graphic aGraphic;

    INetURLObject aURLObj(rPreviewFile);

    // for VCL to be able to create bitmaps / do visual changes in the thread
    aFilter.ImportGraphic(aGraphic, aURLObj);
    BitmapEx aBmp = aGraphic.GetBitmapEx();

    ScopedVclPtr<VirtualDevice> xVirDev = pCurrentItem->m_xImageScreenshot->create_virtual_device();
    xVirDev->SetOutputSizePixel(aBmp.GetSizePixel());
    xVirDev->DrawBitmapEx(Point(0, 0), aBmp);

    pCurrentItem->m_xImageScreenshot->set_image(xVirDev.get());
    xVirDev.disposeAndClear();
}

} // End of the anonymous namespace

SearchAndParseThread::SearchAndParseThread(AdditionsDialog* pDialog, const bool& isFirstLoading)
    : Thread("cuiAdditionsSearchThread")
    , m_pAdditionsDialog(pDialog)
    , m_bExecute(true)
    , m_bIsFirstLoading(isFirstLoading)
{
}

SearchAndParseThread::~SearchAndParseThread() {}

void SearchAndParseThread::Append(AdditionInfo& additionInfo)
{
    if (!m_bExecute)
        return;
    OUString aPreviewFile;
    bool bResult = getPreviewFile(additionInfo, aPreviewFile); // info vector json data

    if (!bResult)
    {
        SAL_INFO("cui.dialogs", "Couldn't get the preview file. Skipping: " << aPreviewFile);
        return;
    }

    SolarMutexGuard aGuard;

    auto newItem = std::make_shared<AdditionsItem>(m_pAdditionsDialog->m_xContentGrid.get(),
                                                   m_pAdditionsDialog, additionInfo);
    m_pAdditionsDialog->m_aAdditionsItems.push_back(newItem);
    std::shared_ptr<AdditionsItem> aCurrentItem = m_pAdditionsDialog->m_aAdditionsItems.back();

    LoadImage(aPreviewFile, aCurrentItem);
    m_pAdditionsDialog->m_nCurrentListItemCount++;

    if (m_pAdditionsDialog->m_nCurrentListItemCount == m_pAdditionsDialog->m_nMaxItemCount)
    {
        if (m_pAdditionsDialog->m_nCurrentListItemCount
            != m_pAdditionsDialog->m_aAllExtensionsVector.size())
            aCurrentItem->m_xButtonShowMore->set_visible(true);
    }
}

void SearchAndParseThread::Search()
{
    m_pAdditionsDialog->m_searchOptions.searchString
        = m_pAdditionsDialog->m_xEntrySearch->get_text();
    utl::TextSearch textSearch(m_pAdditionsDialog->m_searchOptions);

    size_t nIteration = 0;
    for (auto& rInfo : m_pAdditionsDialog->m_aAllExtensionsVector)
    {
        if (m_pAdditionsDialog->m_nCurrentListItemCount == m_pAdditionsDialog->m_nMaxItemCount)
            break;

        OUString sExtensionName = rInfo.sName;
        OUString sExtensionDescription = rInfo.sDescription;

        if (!m_pAdditionsDialog->m_xEntrySearch->get_text().isEmpty()
            && !textSearch.searchForward(sExtensionName)
            && !textSearch.searchForward(sExtensionDescription))
        {
            continue;
        }
        else
        {
            if (nIteration >= m_pAdditionsDialog->m_nCurrentListItemCount)
                Append(rInfo);
            nIteration++;
        }
    }
    CheckInstalledExtensions();
}

void SearchAndParseThread::CheckInstalledExtensions()
{
    uno::Sequence<uno::Sequence<uno::Reference<deployment::XPackage>>> xAllPackages
        = m_pAdditionsDialog->getInstalledExtensions();

    if (!xAllPackages.hasElements())
        return;

    OUString currentExtensionName;

    for (auto& package : xAllPackages)
    {
        for (auto& extensionVersion : package)
        {
            if (extensionVersion.is())
            {
                currentExtensionName = extensionVersion->getName();
                if (currentExtensionName.isEmpty())
                    continue;

                m_pAdditionsDialog->m_searchOptions.searchString = currentExtensionName;
                utl::TextSearch textSearch(m_pAdditionsDialog->m_searchOptions);

                for (auto& rInfo : m_pAdditionsDialog->m_aAdditionsItems)
                {
                    OUString sExtensionDownloadURL = rInfo->m_sDownloadURL;

                    if (!textSearch.searchForward(sExtensionDownloadURL))
                    {
                        continue;
                    }
                    else
                    {
                        rInfo->m_xButtonInstall->set_sensitive(false);
                    }
                }
            }
        }
    }
}

void SearchAndParseThread::execute()
{
    OUString sProgress;
    if (m_bIsFirstLoading)
        sProgress = CuiResId(RID_SVXSTR_ADDITIONS_LOADING);
    else
        sProgress = CuiResId(RID_SVXSTR_ADDITIONS_SEARCHING);

    m_pAdditionsDialog->SetProgress(
        sProgress); // Loading or searching according to being first call or not

    if (m_bIsFirstLoading)
    {
        std::string sResponse = curlGet(m_pAdditionsDialog->m_sURL);
        parseResponse(sResponse, m_pAdditionsDialog->m_aAllExtensionsVector);
        Search();
    }
    else // Searching
    {
        Search();
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
    , m_xGearBtn(m_xBuilder->weld_menu_button("buttonGear"))
{
    m_aSearchDataTimer.SetInvokeHandler(LINK(this, AdditionsDialog, ImplUpdateDataHdl));
    m_aSearchDataTimer.SetDebugName("AdditionsDialog SearchDataTimer");
    m_aSearchDataTimer.SetTimeout(EDIT_UPDATEDATA_TIMEOUT);

    m_xEntrySearch->connect_changed(LINK(this, AdditionsDialog, SearchUpdateHdl));
    m_xEntrySearch->connect_focus_out(LINK(this, AdditionsDialog, FocusOut_Impl));
    m_xButtonClose->connect_clicked(LINK(this, AdditionsDialog, CloseButtonHdl));

    m_sTag = OUStringToOString(sAdditionsTag, RTL_TEXTENCODING_UTF8);
    m_nMaxItemCount = PAGE_SIZE; // Dialog initialization item count
    m_nCurrentListItemCount = 0; // First, there is no item on the list.

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
    m_sURL = rURL;

    m_xExtensionManager
        = deployment::ExtensionManager::get(::comphelper::getProcessComponentContext());

    //Initialize search util
    m_searchOptions.AlgorithmType2 = css::util::SearchAlgorithms2::ABSOLUTE;
    m_searchOptions.transliterateFlags |= TransliterationFlags::IGNORE_CASE;
    m_searchOptions.searchFlag |= (css::util::SearchFlags::REG_NOT_BEGINOFLINE
                                   | css::util::SearchFlags::REG_NOT_ENDOFLINE);
    m_pSearchThread = new SearchAndParseThread(this, true);
    m_pSearchThread->launch();
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

uno::Sequence<uno::Sequence<uno::Reference<deployment::XPackage>>>
AdditionsDialog::getInstalledExtensions()
{
    uno::Sequence<uno::Sequence<uno::Reference<deployment::XPackage>>> xAllPackages;

    try
    {
        xAllPackages = m_xExtensionManager->getAllExtensions(
            uno::Reference<task::XAbortChannel>(), uno::Reference<ucb::XCommandEnvironment>());
    }
    catch (const deployment::DeploymentException&)
    {
        SAL_WARN("cui.dialogs", "Deployment Exception");
    }
    catch (const ucb::CommandFailedException&)
    {
        SAL_WARN("cui.dialogs", "Command Failed Exception");
    }
    catch (const ucb::CommandAbortedException&)
    {
        SAL_WARN("cui.dialogs", "Command Aborted Exception");
    }
    catch (const lang::IllegalArgumentException& e)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw css::lang::WrappedTargetRuntimeException(e.Message, e.Context, anyEx);
    }
    return xAllPackages;
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
        item->m_xContainer->hide();
    }
    this->m_aAdditionsItems.clear();
}

AdditionsItem::AdditionsItem(weld::Widget* pParent, AdditionsDialog* pParentDialog,
                             AdditionInfo& additionInfo)
    : m_xBuilder(Application::CreateBuilder(pParent, "cui/ui/additionsfragment.ui"))
    , m_xContainer(m_xBuilder->weld_widget("additionsEntry"))
    , m_xImageScreenshot(m_xBuilder->weld_image("imageScreenshot"))
    , m_xButtonInstall(m_xBuilder->weld_button("buttonInstall"))
    , m_xLinkButtonName(m_xBuilder->weld_link_button("linkButtonName"))
    , m_xLabelAuthor(m_xBuilder->weld_label("labelAuthor"))
    , m_xLabelDesc(m_xBuilder->weld_label("labelDesc")) // no change (print description)
    , m_xLabelDescription(m_xBuilder->weld_label("labelDescription"))
    , m_xLabelLicense(m_xBuilder->weld_label("labelLicense"))
    , m_xLabelVersion(m_xBuilder->weld_label("labelVersion"))
    , m_xLabelComments(m_xBuilder->weld_label("labelComments")) // no change
    , m_xLinkButtonComments(m_xBuilder->weld_link_button("linkButtonComments"))
    , m_xImageVoting(m_xBuilder->weld_image("imageVoting"))
    , m_xImageDownloadNumber(m_xBuilder->weld_image("imageDownloadNumber"))
    , m_xLabelDownloadNumber(m_xBuilder->weld_label("labelDownloadNumber"))
    , m_xButtonShowMore(m_xBuilder->weld_button("buttonShowMore"))
    , m_pParentDialog(pParentDialog)
    , m_sDownloadURL("")
{
    SolarMutexGuard aGuard;

    m_xContainer->set_grid_left_attach(0);
    m_xContainer->set_grid_top_attach(pParentDialog->m_aAdditionsItems.size() - 1);

    m_xLinkButtonName->set_label(additionInfo.sName);
    m_xLinkButtonName->set_uri(additionInfo.sExtensionURL);
    m_xLabelDescription->set_label(additionInfo.sIntroduction);
    m_xLabelAuthor->set_label(additionInfo.sAuthorName);
    m_xButtonInstall->set_label(CuiResId(RID_SVXSTR_ADDITIONS_INSTALLBUTTON));
    OUString sLicenseString = CuiResId(RID_SVXSTR_ADDITIONS_LICENCE) + additionInfo.sLicense;
    m_xLabelLicense->set_label(sLicenseString);
    OUString sVersionString
        = CuiResId(RID_SVXSTR_ADDITIONS_REQUIREDVERSION) + additionInfo.sCompatibleVersion;
    m_xLabelVersion->set_label(sVersionString);
    m_xLinkButtonComments->set_label(additionInfo.sCommentNumber);
    m_xLinkButtonComments->set_uri(additionInfo.sCommentURL);
    m_xLabelDownloadNumber->set_label(additionInfo.sDownloadNumber);
    m_pParentDialog = pParentDialog;
    m_sDownloadURL = additionInfo.sDownloadURL;

    m_xButtonShowMore->connect_clicked(LINK(this, AdditionsItem, ShowMoreHdl));
    m_xButtonInstall->connect_clicked(LINK(this, AdditionsItem, InstallHdl));
}

IMPL_LINK_NOARG(AdditionsDialog, ImplUpdateDataHdl, Timer*, void)
{
    if (m_pSearchThread.is())
        m_pSearchThread->StopExecution();
    ClearList();
    m_nCurrentListItemCount = 0;
    m_nMaxItemCount = PAGE_SIZE;
    m_pSearchThread = new SearchAndParseThread(this, false);
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

IMPL_LINK_NOARG(AdditionsItem, ShowMoreHdl, weld::Button&, void)
{
    this->m_xButtonShowMore->set_visible(false);
    m_pParentDialog->m_nMaxItemCount += PAGE_SIZE;
    if (m_pParentDialog->m_pSearchThread.is())
        m_pParentDialog->m_pSearchThread->StopExecution();
    m_pParentDialog->m_pSearchThread = new SearchAndParseThread(m_pParentDialog, false);
    m_pParentDialog->m_pSearchThread->launch();
}

IMPL_LINK_NOARG(AdditionsItem, InstallHdl, weld::Button&, void)
{
    m_xButtonInstall->set_label("Success");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
