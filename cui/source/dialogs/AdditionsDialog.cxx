/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sal/config.h>

#include <string_view>

#include <config_folders.h>

#include <AdditionsDialog.hxx>
#include <dialmgr.hxx>
#include <strings.hrc>

#include <sal/log.hxx>

#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/processfactory.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/graphicfilter.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/util/SearchAlgorithms2.hpp>
#include <unotools/textsearch.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <ucbhelper/content.hxx>

#include <com/sun/star/deployment/DeploymentException.hpp>
#include <com/sun/star/deployment/ExtensionManager.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ucb/CommandFailedException.hpp>

#include <com/sun/star/task/XInteractionApprove.hpp>

#include <orcus/json_document_tree.hpp>
#include <orcus/config.hpp>
#include <orcus/pstring.hpp>

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

namespace
{
// Gets the content of the given URL and returns as a standard string
std::string ucbGet(const OUString& rURL)
{
    try
    {
        auto const s = utl::UcbStreamHelper::CreateStream(rURL, StreamMode::STD_READ);
        if (!s)
        {
            SAL_WARN("cui.dialogs", "CreateStream <" << rURL << "> failed");
            return {};
        }
        std::string response_body;
        do
        {
            char buf[4096];
            auto const n = s->ReadBytes(buf, sizeof buf);
            response_body.append(buf, n);
        } while (s->good());
        if (s->bad())
        {
            SAL_WARN("cui.dialogs", "Reading <" << rURL << "> failed with " << s->GetError());
            return {};
        }
        return response_body;
    }
    catch (css::uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("cui.dialogs", "Download failed");
        return {};
    }
}

// Downloads and saves the file at the given rURL to a local path (sFolderURL/fileName)
void ucbDownload(const OUString& rURL, const OUString& sFolderURL, const OUString& fileName)
{
    try
    {
        ucbhelper::Content(sFolderURL, {}, comphelper::getProcessComponentContext())
            .transferContent(ucbhelper::Content(rURL, {}, comphelper::getProcessComponentContext()),
                             ucbhelper::InsertOperation::Copy, fileName,
                             css::ucb::NameClash::OVERWRITE);
    }
    catch (css::uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("cui.dialogs", "Download failed");
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
                OStringToOUString(std::string_view(arrayElement.child("id").string_value().get()),
                                  RTL_TEXTENCODING_UTF8),
                OStringToOUString(std::string_view(arrayElement.child("name").string_value().get()),
                                  RTL_TEXTENCODING_UTF8),
                OStringToOUString(
                    std::string_view(arrayElement.child("author").string_value().get()),
                    RTL_TEXTENCODING_UTF8),
                OStringToOUString(std::string_view(arrayElement.child("url").string_value().get()),
                                  RTL_TEXTENCODING_UTF8),
                OStringToOUString(
                    std::string_view(arrayElement.child("screenshotURL").string_value().get()),
                    RTL_TEXTENCODING_UTF8),
                OStringToOUString(
                    std::string_view(
                        arrayElement.child("extensionIntroduction").string_value().get()),
                    RTL_TEXTENCODING_UTF8),
                OStringToOUString(
                    std::string_view(
                        arrayElement.child("extensionDescription").string_value().get()),
                    RTL_TEXTENCODING_UTF8),
                OStringToOUString(std::string_view(arrayElement.child("releases")
                                                       .child(0)
                                                       .child("compatibility")
                                                       .string_value()
                                                       .get()),
                                  RTL_TEXTENCODING_UTF8),
                OStringToOUString(std::string_view(arrayElement.child("releases")
                                                       .child(0)
                                                       .child("releaseName")
                                                       .string_value()
                                                       .get()),
                                  RTL_TEXTENCODING_UTF8),
                OStringToOUString(std::string_view(arrayElement.child("releases")
                                                       .child(0)
                                                       .child("license")
                                                       .string_value()
                                                       .get()),
                                  RTL_TEXTENCODING_UTF8),
                OStringToOUString(
                    std::string_view(arrayElement.child("commentNumber").string_value().get()),
                    RTL_TEXTENCODING_UTF8),
                OStringToOUString(
                    std::string_view(arrayElement.child("commentURL").string_value().get()),
                    RTL_TEXTENCODING_UTF8),
                OStringToOUString(
                    std::string_view(arrayElement.child("rating").string_value().get()),
                    RTL_TEXTENCODING_UTF8),
                OStringToOUString(
                    std::string_view(arrayElement.child("downloadNumber").string_value().get()),
                    RTL_TEXTENCODING_UTF8),
                OStringToOUString(std::string_view(arrayElement.child("releases")
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

    // copy the images to the user's additions folder
    OUString userFolder = "${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER
                          "/" SAL_CONFIGFILE("bootstrap") "::UserInstallation}";
    rtl::Bootstrap::expandMacros(userFolder);
    userFolder += "/user/additions/" + aAdditionInfo.sExtensionID + "/";

    OUString aPreviewFile(INetURLObject(aAdditionInfo.sScreenshotURL).getName());
    OUString aPreviewURL = aAdditionInfo.sScreenshotURL;

    try
    {
        osl::Directory::createPath(userFolder);

        if (!xFileAccess->exists(userFolder + aPreviewFile))
            ucbDownload(aPreviewURL, userFolder, aPreviewFile);
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
    const sal_Int8 Margin = 6;

    SolarMutexGuard aGuard;

    GraphicFilter aFilter;
    Graphic aGraphic;

    INetURLObject aURLObj(rPreviewFile);

    // for VCL to be able to create bitmaps / do visual changes in the thread
    aFilter.ImportGraphic(aGraphic, aURLObj);
    BitmapEx aBmp = aGraphic.GetBitmapEx();
    Size aBmpSize = aBmp.GetSizePixel();
    Size aThumbSize(pCurrentItem->m_xImageScreenshot->get_size_request());
    if (!aBmp.IsEmpty())
    {
        double aScale;
        if (aBmpSize.Width() > aThumbSize.Width() - 2 * Margin)
        {
            aScale = static_cast<double>(aBmpSize.Width()) / (aThumbSize.Width() - 2 * Margin);
            aBmp.Scale(Size(aBmpSize.Width() / aScale, aBmpSize.Height() / aScale));
        }
        else if (aBmpSize.Height() > aThumbSize.Height() - 2 * Margin)
        {
            aScale = static_cast<double>(aBmpSize.Height()) / (aThumbSize.Height() - 2 * Margin);
            aBmp.Scale(Size(aBmpSize.Width() / aScale, aBmpSize.Height() / aScale));
        };
        aBmpSize = aBmp.GetSizePixel();
    }

    ScopedVclPtr<VirtualDevice> xVirDev = pCurrentItem->m_xImageScreenshot->create_virtual_device();
    xVirDev->SetOutputSizePixel(aThumbSize);
    //white background since images come with a white border
    xVirDev->SetBackground(Wallpaper(COL_WHITE));
    xVirDev->Erase();
    xVirDev->DrawBitmapEx(Point(aThumbSize.Width() / 2 - aBmpSize.Width() / 2, Margin), aBmp);
    pCurrentItem->m_xImageScreenshot->set_image(xVirDev.get());
    xVirDev.disposeAndClear();
}

} // End of the anonymous namespace

SearchAndParseThread::SearchAndParseThread(AdditionsDialog* pDialog, const bool isFirstLoading)
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
                        SolarMutexGuard aGuard;
                        rInfo->m_xButtonInstall->set_sensitive(false);
                        rInfo->m_xButtonInstall->set_label(
                            CuiResId(RID_SVXSTR_ADDITIONS_INSTALLEDBUTTON));
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
        std::string sResponse = ucbGet(m_pAdditionsDialog->m_sURL);
        parseResponse(sResponse, m_pAdditionsDialog->m_aAllExtensionsVector);
        std::sort(m_pAdditionsDialog->m_aAllExtensionsVector.begin(),
                  m_pAdditionsDialog->m_aAllExtensionsVector.end(),
                  AdditionsDialog::sortByDownload);
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
    m_xGearBtn->connect_selected(LINK(this, AdditionsDialog, GearHdl));
    m_xGearBtn->set_item_active("gear_sort_voting", true);

    m_aSearchDataTimer.SetInvokeHandler(LINK(this, AdditionsDialog, ImplUpdateDataHdl));
    m_aSearchDataTimer.SetDebugName("AdditionsDialog SearchDataTimer");
    m_aSearchDataTimer.SetTimeout(EDIT_UPDATEDATA_TIMEOUT);

    m_xEntrySearch->connect_changed(LINK(this, AdditionsDialog, SearchUpdateHdl));
    m_xEntrySearch->connect_focus_out(LINK(this, AdditionsDialog, FocusOut_Impl));
    m_xButtonClose->connect_clicked(LINK(this, AdditionsDialog, CloseButtonHdl));

    m_sTag = sAdditionsTag;
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
    OUString rURL = "https://libreoffice.yusufketen.com/api/" + m_sTag + ".json";
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
        TOOLS_WARN_EXCEPTION("cui.dialogs", "");
    }
    catch (const ucb::CommandFailedException&)
    {
        TOOLS_WARN_EXCEPTION("cui.dialogs", "");
    }
    catch (const ucb::CommandAbortedException&)
    {
        TOOLS_WARN_EXCEPTION("cui.dialogs", "");
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
    {
        m_xLabelProgress->hide();
        m_xButtonClose->set_sensitive(true);
    }
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

void AdditionsDialog::RefreshUI()
{
    if (m_pSearchThread.is())
        m_pSearchThread->StopExecution();
    ClearList();
    m_nCurrentListItemCount = 0;
    m_nMaxItemCount = PAGE_SIZE;
    m_pSearchThread = new SearchAndParseThread(this, false);
    m_pSearchThread->launch();
}

bool AdditionsDialog::sortByRating(const AdditionInfo& a, const AdditionInfo& b)
{
    return a.sRating.toDouble() > b.sRating.toDouble();
}

bool AdditionsDialog::sortByComment(const AdditionInfo& a, const AdditionInfo& b)
{
    return a.sCommentNumber.toUInt32() > b.sCommentNumber.toUInt32();
}

bool AdditionsDialog::sortByDownload(const AdditionInfo& a, const AdditionInfo& b)
{
    return a.sDownloadNumber.toUInt32() > b.sDownloadNumber.toUInt32();
}

AdditionsItem::AdditionsItem(weld::Widget* pParent, AdditionsDialog* pParentDialog,
                             const AdditionInfo& additionInfo)
    : m_xBuilder(Application::CreateBuilder(pParent, "cui/ui/additionsfragment.ui"))
    , m_xContainer(m_xBuilder->weld_widget("additionsEntry"))
    , m_xImageScreenshot(m_xBuilder->weld_image("imageScreenshot"))
    , m_xButtonInstall(m_xBuilder->weld_button("buttonInstall"))
    , m_xLinkButtonWebsite(m_xBuilder->weld_link_button("btnWebsite"))
    , m_xLabelName(m_xBuilder->weld_label("lbName"))
    , m_xLabelAuthor(m_xBuilder->weld_label("labelAuthor"))
    , m_xLabelDesc(m_xBuilder->weld_label("labelDesc")) // no change (print description)
    , m_xLabelDescription(m_xBuilder->weld_label("labelDescription"))
    , m_xLabelLicense(m_xBuilder->weld_label("lbLicenseText"))
    , m_xLabelVersion(m_xBuilder->weld_label("lbVersionText"))
    , m_xLabelComments(m_xBuilder->weld_label("labelComments")) // no change
    , m_xLinkButtonComments(m_xBuilder->weld_link_button("linkButtonComments"))
    , m_xImageVoting1(m_xBuilder->weld_image("imageVoting1"))
    , m_xImageVoting2(m_xBuilder->weld_image("imageVoting2"))
    , m_xImageVoting3(m_xBuilder->weld_image("imageVoting3"))
    , m_xImageVoting4(m_xBuilder->weld_image("imageVoting4"))
    , m_xImageVoting5(m_xBuilder->weld_image("imageVoting5"))
    , m_xLabelNoVoting(m_xBuilder->weld_label("votingLabel"))
    , m_xImageDownloadNumber(m_xBuilder->weld_image("imageDownloadNumber"))
    , m_xLabelDownloadNumber(m_xBuilder->weld_label("labelDownloadNumber"))
    , m_xButtonShowMore(m_xBuilder->weld_button("buttonShowMore"))
    , m_pParentDialog(pParentDialog)
    , m_sDownloadURL("")
    , m_sExtensionID("")
{
    SolarMutexGuard aGuard;

    // AdditionsItem set location
    m_xContainer->set_grid_left_attach(0);
    m_xContainer->set_grid_top_attach(pParentDialog->m_aAdditionsItems.size());

    // Set maximum length of the extension title
    OUString sExtensionName;
    const sal_Int32 maxExtensionNameLength = 30;

    if (additionInfo.sName.getLength() > maxExtensionNameLength)
    {
        OUString sShortName = additionInfo.sName.copy(0, maxExtensionNameLength - 3);
        sExtensionName = sShortName + "...";
    }
    else
    {
        sExtensionName = additionInfo.sName;
    }

    m_xLabelName->set_label(sExtensionName);

    double aExtensionRating = additionInfo.sRating.toDouble();
    switch (int(aExtensionRating))
    {
        case 5:
            m_xImageVoting5->set_from_icon_name("cmd/sc_stars-full.png");
            [[fallthrough]];
        case 4:
            m_xImageVoting4->set_from_icon_name("cmd/sc_stars-full.png");
            [[fallthrough]];
        case 3:
            m_xImageVoting3->set_from_icon_name("cmd/sc_stars-full.png");
            [[fallthrough]];
        case 2:
            m_xImageVoting2->set_from_icon_name("cmd/sc_stars-full.png");
            [[fallthrough]];
        case 1:
            m_xImageVoting1->set_from_icon_name("cmd/sc_stars-full.png");
            break;
    }

    m_xLinkButtonWebsite->set_uri(additionInfo.sExtensionURL);
    m_xLabelDescription->set_label(additionInfo.sIntroduction);

    if (!additionInfo.sAuthorName.equalsIgnoreAsciiCase("null"))
        m_xLabelAuthor->set_label(additionInfo.sAuthorName);

    m_xButtonInstall->set_label(CuiResId(RID_SVXSTR_ADDITIONS_INSTALLBUTTON));
    m_xLabelLicense->set_label(additionInfo.sLicense);
    m_xLabelVersion->set_label(">=" + additionInfo.sCompatibleVersion);
    m_xLinkButtonComments->set_label(additionInfo.sCommentNumber);
    m_xLinkButtonComments->set_uri(additionInfo.sCommentURL);
    m_xLabelDownloadNumber->set_label(additionInfo.sDownloadNumber);
    m_pParentDialog = pParentDialog;
    m_sDownloadURL = additionInfo.sDownloadURL;
    m_sExtensionID = additionInfo.sExtensionID;

    m_xButtonShowMore->connect_clicked(LINK(this, AdditionsItem, ShowMoreHdl));
    m_xButtonInstall->connect_clicked(LINK(this, AdditionsItem, InstallHdl));
}

bool AdditionsItem::getExtensionFile(OUString& sExtensionFile)
{
    uno::Reference<ucb::XSimpleFileAccess3> xFileAccess
        = ucb::SimpleFileAccess::create(comphelper::getProcessComponentContext());

    // copy the extensions' files to the user's additions folder
    OUString userFolder = "${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER
                          "/" SAL_CONFIGFILE("bootstrap") "::UserInstallation}";
    rtl::Bootstrap::expandMacros(userFolder);
    userFolder += "/user/additions/" + m_sExtensionID + "/";

    OUString aExtesionsFile(INetURLObject(m_sDownloadURL).getName());
    OUString aExtesionsURL = m_sDownloadURL;

    try
    {
        osl::Directory::createPath(userFolder);

        if (!xFileAccess->exists(userFolder + aExtesionsFile))
            ucbDownload(aExtesionsURL, userFolder, aExtesionsFile);
    }
    catch (const uno::Exception&)
    {
        return false;
    }
    sExtensionFile = userFolder + aExtesionsFile;
    return true;
}

IMPL_LINK_NOARG(AdditionsDialog, ImplUpdateDataHdl, Timer*, void) { RefreshUI(); }

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
    m_xButtonInstall->set_label(CuiResId(RID_SVXSTR_ADDITIONS_INSTALLING));
    m_xButtonInstall->set_sensitive(false);
    OUString aExtensionFile;
    bool bResult = getExtensionFile(aExtensionFile); // info vector json data

    if (!bResult)
    {
        m_xButtonInstall->set_label(CuiResId(RID_SVXSTR_ADDITIONS_INSTALLBUTTON));
        m_xButtonInstall->set_sensitive(true);

        SAL_INFO("cui.dialogs", "Couldn't get the extension file.");
        return;
    }

    rtl::Reference<TmpRepositoryCommandEnv> pCmdEnv = new TmpRepositoryCommandEnv();
    uno::Reference<task::XAbortChannel> xAbortChannel;
    try
    {
        m_pParentDialog->m_xExtensionManager->addExtension(
            aExtensionFile, uno::Sequence<beans::NamedValue>(), "user", xAbortChannel, pCmdEnv);
        m_xButtonInstall->set_label(CuiResId(RID_SVXSTR_ADDITIONS_INSTALLEDBUTTON));
    }
    catch (const ucb::CommandFailedException)
    {
        TOOLS_WARN_EXCEPTION("cui.dialogs", "");
        m_xButtonInstall->set_label(CuiResId(RID_SVXSTR_ADDITIONS_INSTALLBUTTON));
        m_xButtonInstall->set_sensitive(true);
    }
    catch (const ucb::CommandAbortedException)
    {
        TOOLS_WARN_EXCEPTION("cui.dialogs", "");
        m_xButtonInstall->set_label(CuiResId(RID_SVXSTR_ADDITIONS_INSTALLBUTTON));
        m_xButtonInstall->set_sensitive(true);
    }
    catch (const deployment::DeploymentException)
    {
        TOOLS_WARN_EXCEPTION("cui.dialogs", "");
        m_xButtonInstall->set_label(CuiResId(RID_SVXSTR_ADDITIONS_INSTALLBUTTON));
        m_xButtonInstall->set_sensitive(true);
    }
    catch (const lang::IllegalArgumentException)
    {
        TOOLS_WARN_EXCEPTION("cui.dialogs", "");
        m_xButtonInstall->set_label(CuiResId(RID_SVXSTR_ADDITIONS_INSTALLBUTTON));
        m_xButtonInstall->set_sensitive(true);
    }
    catch (const css::uno::Exception)
    {
        TOOLS_WARN_EXCEPTION("cui.dialogs", "");
        m_xButtonInstall->set_label(CuiResId(RID_SVXSTR_ADDITIONS_INSTALLBUTTON));
        m_xButtonInstall->set_sensitive(true);
    }
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

IMPL_LINK(AdditionsDialog, GearHdl, const OString&, rIdent, void)
{
    if (rIdent == "gear_sort_voting")
    {
        std::sort(m_aAllExtensionsVector.begin(), m_aAllExtensionsVector.end(), sortByRating);
    }
    else if (rIdent == "gear_sort_comments")
    {
        std::sort(m_aAllExtensionsVector.begin(), m_aAllExtensionsVector.end(), sortByComment);
    }
    else if (rIdent == "gear_sort_downloads")
    {
        std::sort(m_aAllExtensionsVector.begin(), m_aAllExtensionsVector.end(), sortByDownload);
    }
    // After the sorting, UI will be refreshed to update extension list.
    RefreshUI();
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
