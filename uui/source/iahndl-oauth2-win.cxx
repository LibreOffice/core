/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <config_oauth2.h>

#include "iahndl-oauth2.hxx"

// Has to be before vcl/svapp.hxx
#include <prewin.h>
#include <http.h>
#include <postwin.h>

#pragma comment(lib, "httpapi.lib")

#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include <o3tl/temporary.hxx>
#include <osl/thread.h>
#include <rtl/uri.hxx>
#include <sal/log.hxx>
#include <sal/types.h>
#include <systools/curlinit.hxx>
#include <tools/link.hxx>
#include <unotools/configmgr.hxx>
#include <vcl/svapp.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/weld/MessageDialog.hxx>
#include <vcl/weld/weld.hxx>

#include <curl/curl.h>

#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>

static OUString getQueryComponent(std::u16string_view query, std::u16string_view name)
{
    auto i = query.find(name);
    for (;; i = query.find(name, i + 1))
    {
        if (i == std::u16string_view::npos || query.size() <= i + name.size())
            return {};
        if ((i == 0 || query[i - 1] == '&' || query[i - 1] == '?') && query[i + name.size()] == '=')
            break; // found
    }

    auto rest = query.substr(i + name.size() + 1);
    // up to next component or fragment or end of URL
    return rtl::Uri::decode(OUString(rest.substr(0, rest.find_first_of(u"&#"))),
                            rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8);
}

struct OAuth2Request::Impl
{
    std::unique_ptr<weld::MessageDialog> cancelBox;
    OUString authUrl;
    OUString redirectUrl;
    OUString retCode;
    int initStage = 0;

    HTTP_URL_GROUP_ID groupId{};
    HTTP_BINDING_INFO bindingInfo{ .Flags = { .Present = 1 }, .RequestQueueHandle = nullptr };

    Impl(const OUString& auth_url)
        : cancelBox(Application::CreateMessageDialog(
              nullptr, VclMessageType::Info, VclButtonsType::Cancel,
              u"Login to Google in the opened browser window."_ustr))
        , authUrl(auth_url)
    {
        cancelBox->set_title(u"Waiting for authentication"_ustr);
    }

    DECL_LINK(CloseDialog, void*, void);
    bool initHTTP();
    void deinitHTTP();
    static void SAL_CALL listenHTTP(void* pThis);
    bool openBrowser() const;
    void sendResponse(HTTP_REQUEST_ID id, std::string_view body = {}) const;
    void sendCloseResponse(HTTP_REQUEST_ID id, std::u16string_view error) const;
};

OAuth2Request::OAuth2Request(const OUString& auth_url)
    : impl(std::make_unique<Impl>(auth_url))
{
}

OAuth2Request::~OAuth2Request() = default; // for Impl destruction

IMPL_LINK_NOARG(OAuth2Request::Impl, CloseDialog, void*, void)
{
    cancelBox->response(retCode.isEmpty() ? VclResponseType::RET_CANCEL : VclResponseType::RET_OK);
}

// Create a minimal HTTP server on localhost, which will handle the redirect from OAuth2 server.
void OAuth2Request::run()
{
    if (impl->initHTTP())
    {
        if (auto thread = osl_createThread(Impl::listenHTTP, impl.get()))
        {
            if (impl->openBrowser())
                impl->cancelBox->run();

            // Send DELETE request
            CURL* curl = curl_easy_init();
            curl_easy_setopt(curl, CURLOPT_URL,
                             OUStringToOString(impl->redirectUrl, RTL_TEXTENCODING_UTF8).getStr());
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
            curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            osl_joinWithThread(thread);
        }
    }
    impl->deinitHTTP(); // Cleans up everything possibly initialized, even on initHTTP failure
}

const OUString& OAuth2Request::getRetCode() const { return impl->retCode; }

bool OAuth2Request::Impl::initHTTP()
{
    redirectUrl = getQueryComponent(authUrl, u"redirect_uri");
    if (!redirectUrl.startsWith(OAUTH2_REDIRECT_URI_PREFIX))
        return false;

    ULONG result = HttpInitialize(HTTPAPI_VERSION_2, HTTP_INITIALIZE_SERVER, 0);
    if (result != NO_ERROR)
    {
        SAL_WARN("ucb.ucp.cmis", "HttpInitialize failed with code " << result);
        return false;
    }
    initStage = 1;

    HTTP_SERVER_SESSION_ID serverSessionId;
    result = HttpCreateServerSession(HTTPAPI_VERSION_2, &serverSessionId, 0);
    if (result != NO_ERROR)
    {
        SAL_WARN("ucb.ucp.cmis", "HttpCreateServerSession failed with code " << result);
        return false;
    }

    result = HttpCreateUrlGroup(serverSessionId, &groupId, 0);
    if (result != NO_ERROR)
    {
        SAL_WARN("ucb.ucp.cmis", "HttpCreateUrlGroup failed with code " << result);
        return false;
    }

    result = HttpCreateRequestQueue(HTTPAPI_VERSION_2, nullptr, nullptr, 0,
                                    &bindingInfo.RequestQueueHandle);
    if (result != NO_ERROR)
    {
        SAL_WARN("ucb.ucp.cmis", "HttpCreateRequestQueue failed with code " << result);
        return false;
    }
    initStage = 2;

    result = HttpSetUrlGroupProperty(groupId, HttpServerBindingProperty, &bindingInfo,
                                     sizeof(bindingInfo));
    if (result != NO_ERROR)
    {
        SAL_WARN("ucb.ucp.cmis", "HttpSetUrlGroupProperty failed with code " << result);
        return false;
    }
    initStage = 3;

    result = HttpAddUrlToUrlGroup(groupId, o3tl::toW(redirectUrl.getStr()), 0, 0);
    if (result != NO_ERROR)
    {
        SAL_WARN("ucb.ucp.cmis", "HttpAddUrlToUrlGroup failed with code " << result);
        return false;
    }
    initStage = 4;
    return true;
}

void OAuth2Request::Impl::deinitHTTP()
{
    switch (initStage)
    {
        case 4:
            HttpRemoveUrlFromUrlGroup(groupId, o3tl::toW(redirectUrl.getStr()), 0);
            [[fallthrough]];
        case 3:
            HttpSetUrlGroupProperty(groupId, HttpServerBindingProperty,
                                    &o3tl::temporary(HTTP_BINDING_INFO{
                                        .Flags = { .Present = 0 }, .RequestQueueHandle = nullptr }),
                                    sizeof(HTTP_BINDING_INFO));
            [[fallthrough]];
        case 2:
            HttpShutdownRequestQueue(bindingInfo.RequestQueueHandle);
            [[fallthrough]];
        case 1:
            HttpTerminate(HTTP_INITIALIZE_SERVER, nullptr);
    }
}

// The server must handle the redirect data -> send HTML with a prompt to close the current page;
// and DELETE requests, which mean stop the operation.

// static
void SAL_CALL OAuth2Request::Impl::listenHTTP(void* pThis)
{
    Impl* impl = static_cast<Impl*>(pThis);
    assert(impl);

    for (;;)
    {
        char buf[sizeof(HTTP_REQUEST) + 0x10000]{}; // 64K for the data
        PHTTP_REQUEST request = reinterpret_cast<PHTTP_REQUEST>(buf);
        ULONG result = HttpReceiveHttpRequest(impl->bindingInfo.RequestQueueHandle, HTTP_NULL_ID,
                                              HTTP_RECEIVE_REQUEST_FLAG_COPY_BODY, request,
                                              std::size(buf), &o3tl::temporary(ULONG()), nullptr);
        if (result != NO_ERROR)
        {
            SAL_WARN("ucb.ucp.cmis", "HttpReceiveHttpRequest failed with code " << result);
            return;
        }

        if (request->Verb == HttpVerbDELETE)
        {
            SAL_INFO("ucb.ucp.cmis", "DELETE request received");
            impl->sendResponse(request->RequestId); // send 200 "OK"
            // No need to send anything to the dialog
            return;
        }

        if (request->Verb = HttpVerbGET)
        {
            std::u16string_view query(o3tl::toU(request->CookedUrl.pQueryString));
            if (auto code = getQueryComponent(query, u"code"); !code.isEmpty())
            {
                impl->retCode = code;
                impl->sendCloseResponse(request->RequestId, {});
                continue;
            }
            else if (auto error = getQueryComponent(query, u"error"); !error.isEmpty())
            {
                impl->sendCloseResponse(request->RequestId, error);
                continue;
            }
        }

        impl->sendResponse(request->RequestId); // just send 200 OK
    }
}

bool OAuth2Request::Impl::openBrowser() const
{
    assert(!authUrl.isEmpty());
    try
    {
        auto exec(
            css::system::SystemShellExecute::create(comphelper::getProcessComponentContext()));
        exec->execute(authUrl, OUString(), css::system::SystemShellExecuteFlags::URIS_ONLY);
        return true;
    }
    catch (const css::uno::Exception&)
    {
        css::uno::Any exc(::cppu::getCaughtException());
        OUString msg(::comphelper::anyToString(exc));
        const SolarMutexGuard guard;
        std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(
            nullptr, VclMessageType::Warning, VclButtonsType::Ok, msg));
        xErrorBox->set_title("title");
        xErrorBox->run();
        xErrorBox.reset();
        return false;
    }
}

void OAuth2Request::Impl::sendResponse(HTTP_REQUEST_ID id, std::string_view body) const
{
    HTTP_RESPONSE response{};
    response.StatusCode = 200;

    static constexpr std::string_view ok("OK");
    response.pReason = ok.data();
    response.ReasonLength = ok.size();

    static constexpr std::string_view text_html("text/html");
    response.Headers.KnownHeaders[HttpHeaderContentType].pRawValue = text_html.data();
    response.Headers.KnownHeaders[HttpHeaderContentType].RawValueLength = text_html.size();

    if (!body.empty())
    {
        HTTP_DATA_CHUNK chunk{ .DataChunkType = HttpDataChunkFromMemory,
                               .FromMemory = { .pBuffer = const_cast<char*>(body.data()),
                                               .BufferLength = static_cast<ULONG>(body.size()) } };
        response.EntityChunkCount = 1;
        response.pEntityChunks = &chunk;
    }

    HttpSendHttpResponse(bindingInfo.RequestQueueHandle, id, 0, &response, nullptr, nullptr,
                         nullptr, 0, nullptr, nullptr);
}

void OAuth2Request::Impl::sendCloseResponse(HTTP_REQUEST_ID id, std::u16string_view error) const
{
    static constexpr OUString html_tmpl(u"<!DOCTYPE html>"
                                        "<html>"
                                        "<head>"
                                        "<meta charset=\"UTF-8\">"
                                        "<title>Close Window</title>"
                                        "</head>"
                                        "<body>"
                                        "<p>$RESULT</p>"
                                        "<p>Close the page to return to $PRODUCTNAME</p>"
                                        "</body>"
                                        "</html>"_ustr);
    OUString result = retCode.isEmpty() || !error.empty() ? u"Authorization error: "_ustr + error
                                                          : u"Authorization successful."_ustr;
    OUString u16 = html_tmpl // replace the placeholders
                       .replaceFirst("$RESULT", result)
                       .replaceFirst("$PRODUCTNAME", utl::ConfigManager::getProductName());
    sendResponse(id, OUStringToOString(u16, RTL_TEXTENCODING_UTF8));

    // Executed in the VCL thread, inside the dialog's run()
    Application::PostUserEvent(LINK(const_cast<Impl*>(this), Impl, CloseDialog));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
