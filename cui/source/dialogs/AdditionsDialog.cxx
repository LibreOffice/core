/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <AdditionsDialog.hxx>
#include <rtl/ustrbuf.hxx>
#include <unordered_set>
#include <sal/log.hxx>
#include <tools/stream.hxx>
#include <strings.hrc>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/inspection/PropertyLineElement.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <dialmgr.hxx>

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

namespace
{
struct AdditionInfo
{
    OUString sName;
    OUString sAuthorName;
    OUString sPreviewURL;
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
                OStringToOUString(OString(arrayElement.child("name").string_value().get()),
                                  RTL_TEXTENCODING_UTF8),
                OStringToOUString(OString(arrayElement.child("author").string_value().get()),
                                  RTL_TEXTENCODING_UTF8),
                OStringToOUString(OString(arrayElement.child("URL").string_value().get()),
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
                                              .child("compatibleVersion")
                                              .string_value()
                                              .get()),
                                  RTL_TEXTENCODING_UTF8),
                OStringToOUString(OString(arrayElement.child("releases")
                                              .child(0)
                                              .child("releaseNumber")
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
}

AdditionsDialog::AdditionsDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "cui/ui/additionsdialog.ui", "AdditionsDialog")
    , m_xEntrySearch(m_xBuilder->weld_entry("entrySearch"))
    , m_xMenuButtonSettings(m_xBuilder->weld_menu_button("buttonGear"))
    , m_xContentWindow(m_xBuilder->weld_scrolled_window("contentWindow"))
    , m_xContentGrid(m_xBuilder->weld_container("contentGrid"))
{
    fillGrid();
}

AdditionsDialog::~AdditionsDialog() {}

void AdditionsDialog::fillGrid()
{
    // TODO - Temporary URL
    OString rURL = "https://yusufketen.com/extensionTest.json";
    std::string sResponse = curlGet(rURL);
    std::vector<AdditionInfo> additionInfos;
    parseResponse(sResponse, additionInfos);

    sal_Int32 i = 0;
    for (const auto& additionInfo : additionInfos)
    {
        m_aAdditionsItems.emplace_back(m_xContentGrid.get());
        AdditionsItem& aCurrentItem = m_aAdditionsItems.back();

        sal_Int32 nGridPositionY = i++;
        aCurrentItem.m_xContainer->set_grid_left_attach(0);
        aCurrentItem.m_xContainer->set_grid_top_attach(nGridPositionY);

        aCurrentItem.m_xLinkButtonName->set_label(additionInfo.sName);
        aCurrentItem.m_xLinkButtonName->set_uri(additionInfo.sPreviewURL);
        aCurrentItem.m_xLabelDescription->set_label(additionInfo.sIntroduction);
        aCurrentItem.m_xLabelAuthor->set_label(additionInfo.sAuthorName);
        aCurrentItem.m_xButtonInstall->set_label(CuiResId(RID_SVXSTR_ADDITIONS_INSTALLBUTTON));
        OUString sLicenseString = CuiResId(RID_SVXSTR_ADDITIONS_LICENCE) + additionInfo.sLicense;
        aCurrentItem.m_xLabelLicense->set_label(sLicenseString);
        OUString sVersionString
            = CuiResId(RID_SVXSTR_ADDITIONS_REQUIREDVERSION) + additionInfo.sCompatibleVersion;
        aCurrentItem.m_xLabelVersion->set_label(sVersionString);
        aCurrentItem.m_xLinkButtonComments->set_label(additionInfo.sCommentNumber);
        aCurrentItem.m_xLinkButtonComments->set_uri(additionInfo.sCommentURL);
        aCurrentItem.m_xLabelDownloadNumber->set_label(additionInfo.sDownloadNumber);

        Reference<XGraphic> xGraphic;
        try
        {
            Reference<XComponentContext> xContext(::comphelper::getProcessComponentContext());
            Reference<XGraphicProvider> xGraphicProvider(GraphicProvider::create(xContext));

            Sequence<PropertyValue> aMediaProperties(1);
            aMediaProperties[0].Name = "URL";
            aMediaProperties[0].Value <<= additionInfo.sScreenshotURL;

            xGraphic = Reference<XGraphic>(xGraphicProvider->queryGraphic(aMediaProperties),
                                           css::uno::UNO_SET_THROW);
        }
        catch (const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("cui.dialogs");
        }

        aCurrentItem.m_xImageScreenshot->set_image(xGraphic);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
