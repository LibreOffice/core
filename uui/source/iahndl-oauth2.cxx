/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <config_oauth2.h>

#include "iahndl-oauth2.hxx"

#ifdef _WIN32
// On Windows, Boost.Asio pulls in windows.h
#include <prewin.h>
#endif

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#ifdef _WIN32
#include <postwin.h>
#endif

#include <comphelper/anytostring.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <o3tl/string_view.hxx>
#include <osl/thread.h>
#include <rtl/uri.hxx>
#include <sal/log.hxx>
#include <sal/types.h>
#include <tools/link.hxx>
#include <unotools/configmgr.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

#include <curl/curl.h>

#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

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
    OString redirectPath; // path portion of redirectUrl, for matching incoming requests
    OUString retCode;

    net::io_context ioc{ 1 };
    tcp::acceptor acceptor{ ioc };

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
    static void SAL_CALL listenHTTP(void* pThis);
    bool openBrowser() const;
    static void sendResponse(tcp::socket& socket, std::string_view body = {});
    void sendCloseResponse(tcp::socket& socket, std::u16string_view error) const;
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

            // Send DELETE request to stop the server thread
            CURL* curl = curl_easy_init();
            curl_easy_setopt(curl, CURLOPT_URL,
                             OUStringToOString(impl->redirectUrl, RTL_TEXTENCODING_UTF8).getStr());
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
            curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            osl_joinWithThread(thread);
        }
    }
}

const OUString& OAuth2Request::getRetCode() const { return impl->retCode; }

bool OAuth2Request::Impl::initHTTP()
{
    redirectUrl = getQueryComponent(authUrl, u"redirect_uri");
    if (!redirectUrl.startsWith(OAUTH2_REDIRECT_URI_PREFIX))
        return false;

    // Parse port from redirect URL: "http://localhost:PORT/..."
    static constexpr std::u16string_view sPrefix(u"http://localhost:");
    auto nPortEnd = redirectUrl.indexOf('/', sPrefix.size());
    if (nPortEnd < 0)
        return false;
    sal_uInt16 nPort = o3tl::toUInt32(redirectUrl.subView(sPrefix.size(), nPortEnd));
    if (nPort == 0)
        return false;

    // Store path for request matching
    redirectPath = OUStringToOString(redirectUrl.subView(nPortEnd), RTL_TEXTENCODING_UTF8);

    beast::error_code ec;
    auto endpoint = tcp::endpoint(net::ip::make_address("127.0.0.1"), nPort);

    acceptor.open(endpoint.protocol(), ec);
    if (ec)
    {
        SAL_WARN("ucb.ucp.cmis", "Failed to open acceptor: " << ec.message());
        return false;
    }

    // SO_REUSEADDR: allow rebind if the port is still in TIME_WAIT from a previous run
    acceptor.set_option(net::socket_base::reuse_address(true), ec);

    acceptor.bind(endpoint, ec);
    if (ec)
    {
        SAL_WARN("ucb.ucp.cmis", "Failed to bind to port " << nPort << ": " << ec.message());
        return false;
    }

    acceptor.listen(net::socket_base::max_listen_connections, ec);
    if (ec)
    {
        SAL_WARN("ucb.ucp.cmis", "Failed to listen: " << ec.message());
        return false;
    }

    return true;
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
        beast::error_code ec;
        tcp::socket socket(impl->ioc);
        impl->acceptor.accept(socket, ec);
        if (ec)
        {
            SAL_WARN("ucb.ucp.cmis", "accept failed: " << ec.message());
            return;
        }

        beast::flat_buffer buffer;
        http::request<http::string_body> req;
        http::read(socket, buffer, req, ec);
        if (ec)
        {
            SAL_WARN("ucb.ucp.cmis", "HTTP read failed: " << ec.message());
            continue;
        }

        if (req.method() == http::verb::delete_)
        {
            SAL_INFO("ucb.ucp.cmis", "DELETE request received");
            sendResponse(socket); // send 200 "OK"
            // No need to send anything to the dialog
            return;
        }

        if (req.method() == http::verb::get)
        {
            auto target = req.target();
            if (!target.starts_with(boost::core::string_view(impl->redirectPath)))
            {
                sendResponse(socket);
                continue;
            }
            auto qpos = target.find('?', impl->redirectPath.getLength());
            if (qpos != boost::core::string_view::npos)
            {
                OUString query = OStringToOUString(target.substr(qpos), RTL_TEXTENCODING_UTF8);
                if (auto code = getQueryComponent(query, u"code"); !code.isEmpty())
                {
                    impl->retCode = code;
                    impl->sendCloseResponse(socket, {});
                    continue;
                }
                else if (auto error = getQueryComponent(query, u"error"); !error.isEmpty())
                {
                    impl->sendCloseResponse(socket, error);
                    continue;
                }
            }
        }

        sendResponse(socket); // just send 200 OK
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

void OAuth2Request::Impl::sendResponse(tcp::socket& socket, std::string_view body)
{
    http::response<http::string_body> res(http::status::ok, 11);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(false);
    if (!body.empty())
        res.body() = std::string(body);
    res.prepare_payload();

    beast::error_code ec;
    http::write(socket, res, ec);
    // tdf#171253: graceful half-close: send FIN so the peer reads the complete response;
    // without this, the destructor's close() could send RST instead
    socket.shutdown(tcp::socket::shutdown_send, ec);
    // Wait for peer's FIN to avoid RST on close; timeout guards against a hung peer
    socket.set_option(net::detail::socket_option::integer<SOL_SOCKET, SO_RCVTIMEO>(5000), ec);
    char buf[1];
    socket.read_some(net::buffer(buf), ec);
}

void OAuth2Request::Impl::sendCloseResponse(tcp::socket& socket, std::u16string_view error) const
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
    sendResponse(socket, OUStringToOString(u16, RTL_TEXTENCODING_UTF8));

    // Executed in the VCL thread, inside the dialog's run()
    Application::PostUserEvent(LINK(const_cast<Impl*>(this), Impl, CloseDialog));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
