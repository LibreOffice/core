/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "CurlSession.hxx"

#include "SerfLockStore.hxx"
#include "DAVProperties.hxx"
#include "UCBDeadPropertyValue.hxx"
#include "webdavresponseparser.hxx"

#include <comphelper/attributelist.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/string.hxx>

#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>

#include <officecfg/Inet.hxx>
#include <officecfg/Office/Security.hxx>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/io/Pipe.hpp>
#include <com/sun/star/io/SequenceInputStream.hpp>
#include <com/sun/star/io/SequenceOutputStream.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>

#include <osl/time.h>
#include <sal/log.hxx>
#include <rtl/uri.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <systools/curlinit.hxx>
#include <tools/hostfilter.hxx>
#include <config_version.h>

#include <map>
#include <optional>
#include <tuple>
#include <utility>

using namespace ::com::sun::star;

namespace
{
/// globals container
struct Init
{
    /// note: LockStore has its own mutex and calls CurlSession from its thread
    ///       so don't call LockStore with m_Mutex held to prevent deadlock.
    ::http_dav_ucp::SerfLockStore LockStore;

    Init()
    {
        if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK)
        {
            assert(!"curl_global_init failed");
        }
    }
    // do not call curl_global_cleanup() - this is not the only client of curl
};
Init g_Init;

struct ResponseHeaders
{
    ::std::vector<::std::pair<::std::vector<OString>, ::std::optional<long>>> HeaderFields;
    CURL* pCurl;
    ResponseHeaders(CURL* const i_pCurl)
        : pCurl(i_pCurl)
    {
    }
};

auto GetResponseCode(ResponseHeaders const& rHeaders) -> ::std::optional<long>
{
    return (rHeaders.HeaderFields.empty()) ? ::std::optional<long>{}
                                           : rHeaders.HeaderFields.back().second;
}

struct DownloadTarget
{
    uno::Reference<io::XOutputStream> xOutStream;
    ResponseHeaders const& rHeaders;
    DownloadTarget(uno::Reference<io::XOutputStream> i_xOutStream,
                   ResponseHeaders const& i_rHeaders)
        : xOutStream(std::move(i_xOutStream))
        , rHeaders(i_rHeaders)
    {
    }
};

struct UploadSource
{
    uno::Sequence<sal_Int8> const& rInData;
    size_t nPosition;
    UploadSource(uno::Sequence<sal_Int8> const& i_rInData)
        : rInData(i_rInData)
        , nPosition(0)
    {
    }
};

auto GetErrorString(CURLcode const rc, char const* const pErrorBuffer = nullptr) -> OString
{
    char const* const pMessage( // static fallback
        (pErrorBuffer && pErrorBuffer[0] != '\0') ? pErrorBuffer : curl_easy_strerror(rc));
    return OString::Concat("(") + OString::number(sal_Int32(rc)) + ") " + pMessage;
}

auto GetErrorStringMulti(CURLMcode const mc) -> OString
{
    return OString::Concat("(") + OString::number(sal_Int32(mc)) + ") " + curl_multi_strerror(mc);
}

/// represent an option to be passed to curl_easy_setopt()
struct CurlOption
{
    CURLoption const Option;
    enum class Type
    {
        Pointer,
        Long,
        CurlOffT
    };
    Type const Tag;
    union {
        void const* const pValue;
        long /*const*/ lValue;
        curl_off_t /*const*/ cValue;
    };
    char const* const pExceptionString;

    CurlOption(CURLoption const i_Option, void const* const i_Value,
               char const* const i_pExceptionString)
        : Option(i_Option)
        , Tag(Type::Pointer)
        , pValue(i_Value)
        , pExceptionString(i_pExceptionString)
    {
    }
    // Depending on platform, curl_off_t may be "long" or a larger type
    // so cannot use overloading to distinguish these cases.
    CurlOption(CURLoption const i_Option, curl_off_t const i_Value,
               char const* const i_pExceptionString, Type const type = Type::Long)
        : Option(i_Option)
        , Tag(type)
        , pExceptionString(i_pExceptionString)
    {
        static_assert(sizeof(long) <= sizeof(curl_off_t));
        switch (type)
        {
            case Type::Long:
                lValue = i_Value;
                break;
            case Type::CurlOffT:
                cValue = i_Value;
                break;
            default:
                assert(false);
        }
    }
};

/// combined guard class to ensure things are released in correct order,
/// particularly in ProcessRequest() error handling
class Guard
{
private:
    /// mutex *first* because m_oGuard requires it
    ::std::unique_lock<::std::mutex> m_Lock;
    ::std::vector<CurlOption> const m_Options;
    ::http_dav_ucp::CurlUri const& m_rURI;
    CURL* const m_pCurl;

public:
    explicit Guard(::std::mutex& rMutex, ::std::vector<CurlOption> aOptions,
                   ::http_dav_ucp::CurlUri const& rURI, CURL* const pCurl)
        : m_Lock(rMutex, ::std::defer_lock)
        , m_Options(std::move(aOptions))
        , m_rURI(rURI)
        , m_pCurl(pCurl)
    {
        Acquire();
    }
    ~Guard()
    {
        if (m_Lock.owns_lock())
        {
            Release();
        }
    }

    void Acquire()
    {
        assert(!m_Lock.owns_lock());
        m_Lock.lock();
        for (auto const& it : m_Options)
        {
            CURLcode rc(CURL_LAST); // warning C4701
            if (it.Tag == CurlOption::Type::Pointer)
            {
                rc = curl_easy_setopt(m_pCurl, it.Option, it.pValue);
            }
            else if (it.Tag == CurlOption::Type::Long)
            {
                rc = curl_easy_setopt(m_pCurl, it.Option, it.lValue);
            }
            else if (it.Tag == CurlOption::Type::CurlOffT)
            {
                rc = curl_easy_setopt(m_pCurl, it.Option, it.cValue);
            }
            else
            {
                assert(false);
            }
            if (it.pExceptionString != nullptr)
            {
                if (rc != CURLE_OK)
                {
                    SAL_WARN("ucb.ucp.webdav.curl",
                             "set " << it.pExceptionString << " failed: " << GetErrorString(rc));
                    throw ::http_dav_ucp::DAVException(
                        ::http_dav_ucp::DAVException::DAV_SESSION_CREATE,
                        ::http_dav_ucp::ConnectionEndPointString(m_rURI.GetHost(),
                                                                 m_rURI.GetPort()));
                }
            }
            else // many of the options cannot fail
            {
                assert(rc == CURLE_OK);
            }
        }
    }
    void Release()
    {
        assert(m_Lock.owns_lock());
        for (auto const& it : m_Options)
        {
            CURLcode rc(CURL_LAST); // warning C4701
            if (it.Tag == CurlOption::Type::Pointer)
            {
                rc = curl_easy_setopt(m_pCurl, it.Option, nullptr);
            }
            else if (it.Tag == CurlOption::Type::Long)
            {
                rc = curl_easy_setopt(m_pCurl, it.Option, 0L);
            }
            else if (it.Tag == CurlOption::Type::CurlOffT)
            {
                rc = curl_easy_setopt(m_pCurl, it.Option, curl_off_t(-1));
            }
            else
            {
                assert(false);
            }
            assert(rc == CURLE_OK);
            (void)rc;
        }
        m_Lock.unlock();
    }
};

} // namespace

namespace http_dav_ucp
{
// libcurl callbacks:

static int debug_callback(CURL* handle, curl_infotype type, char* data, size_t size,
                          void* /*userdata*/)
{
    char const* pType(nullptr);
    switch (type)
    {
        case CURLINFO_TEXT:
            SAL_INFO("ucb.ucp.webdav.curl", "debug log: " << handle << ": " << data);
            return 0;
        case CURLINFO_HEADER_IN:
            SAL_INFO("ucb.ucp.webdav.curl",
                     "CURLINFO_HEADER_IN: " << handle << ": " << OString(data, size));
            return 0;
        case CURLINFO_HEADER_OUT:
        {
            // unlike IN, this is all headers in one call
            OString tmp(data, size);
            sal_Int32 const start(tmp.indexOf("Authorization: "));
            if (start != -1)
            {
                sal_Int32 const end(tmp.indexOf("\r\n", start));
                assert(end != -1);
                sal_Int32 const len(SAL_N_ELEMENTS("Authorization: ") - 1);
                tmp = tmp.replaceAt(
                    start + len, end - start - len,
                    Concat2View(OString::number(end - start - len) + " bytes redacted"));
            }
            SAL_INFO("ucb.ucp.webdav.curl", "CURLINFO_HEADER_OUT: " << handle << ": " << tmp);
            return 0;
        }
        case CURLINFO_DATA_IN:
            pType = "CURLINFO_DATA_IN";
            break;
        case CURLINFO_DATA_OUT:
            pType = "CURLINFO_DATA_OUT";
            break;
        case CURLINFO_SSL_DATA_IN:
            pType = "CURLINFO_SSL_DATA_IN";
            break;
        case CURLINFO_SSL_DATA_OUT:
            pType = "CURLINFO_SSL_DATA_OUT";
            break;
        default:
            SAL_WARN("ucb.ucp.webdav.curl", "unexpected debug log type");
            return 0;
    }
    SAL_INFO("ucb.ucp.webdav.curl", "debug log: " << handle << ": " << pType << " " << size);
    return 0;
}

static size_t write_callback(char* const ptr, size_t const size, size_t const nmemb,
                             void* const userdata)
{
    auto* const pTarget(static_cast<DownloadTarget*>(userdata));
    if (!pTarget) // looks like ~every request may have a response body
    {
        return nmemb;
    }
    assert(size == 1); // says the man page
    (void)size;
    assert(pTarget->xOutStream.is());
    auto const oResponseCode(GetResponseCode(pTarget->rHeaders));
    if (!oResponseCode)
    {
        return 0; // that is an error
    }
    // always write, for exception handler in ProcessRequest()
    //    if (200 <= *oResponseCode && *oResponseCode < 300)
    {
        try
        {
            uno::Sequence<sal_Int8> const data(reinterpret_cast<sal_Int8*>(ptr), nmemb);
            pTarget->xOutStream->writeBytes(data);
        }
        catch (...)
        {
            SAL_WARN("ucb.ucp.webdav.curl", "exception in write_callback");
            return 0; // error
        }
    }
    // else: ignore the body? CurlSession will check the status eventually
    return nmemb;
}

static size_t read_callback(char* const buffer, size_t const size, size_t const nitems,
                            void* const userdata)
{
    auto* const pSource(static_cast<UploadSource*>(userdata));
    assert(pSource);
    size_t const nBytes(size * nitems);
    size_t nRet(0);
    try
    {
        assert(pSource->nPosition <= o3tl::make_unsigned(pSource->rInData.getLength()));
        nRet = ::std::min<size_t>(pSource->rInData.getLength() - pSource->nPosition, nBytes);
        ::std::memcpy(buffer, pSource->rInData.getConstArray() + pSource->nPosition, nRet);
        pSource->nPosition += nRet;
    }
    catch (...)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "exception in read_callback");
        return CURL_READFUNC_ABORT; // error
    }
    return nRet;
}

static size_t header_callback(char* const buffer, size_t const size, size_t const nitems,
                              void* const userdata)
{
    auto* const pHeaders(static_cast<ResponseHeaders*>(userdata));
    assert(pHeaders);
#if 0
    if (!pHeaders) // TODO maybe not needed in every request? not sure
    {
        return nitems;
    }
#endif
    assert(size == 1); // says the man page
    (void)size;
    try
    {
        if (nitems <= 2)
        {
            // end of header, body follows...
            if (pHeaders->HeaderFields.empty())
            {
                SAL_WARN("ucb.ucp.webdav.curl", "header_callback: empty header?");
                return 0; // error
            }
            // unfortunately there's no separate callback when the body begins,
            // so have to manually retrieve the status code here
            long statusCode(SC_NONE);
            auto rc = curl_easy_getinfo(pHeaders->pCurl, CURLINFO_RESPONSE_CODE, &statusCode);
            assert(rc == CURLE_OK);
            (void)rc;
            // always put the current response code here - wasn't necessarily in this header
            pHeaders->HeaderFields.back().second.emplace(statusCode);
        }
        else if (buffer[0] == ' ' || buffer[0] == '\t') // folded header field?
        {
            size_t i(0);
            do
            {
                ++i;
            } while (i == ' ' || i == '\t');
            if (pHeaders->HeaderFields.empty() || pHeaders->HeaderFields.back().second
                || pHeaders->HeaderFields.back().first.empty())
            {
                SAL_WARN("ucb.ucp.webdav.curl",
                         "header_callback: folded header field without start");
                return 0; // error
            }
            pHeaders->HeaderFields.back().first.back()
                += OString::Concat(" ") + ::std::string_view(&buffer[i], nitems - i);
        }
        else
        {
            if (pHeaders->HeaderFields.empty() || pHeaders->HeaderFields.back().second)
            {
                pHeaders->HeaderFields.emplace_back();
            }
            pHeaders->HeaderFields.back().first.emplace_back(OString(buffer, nitems));
        }
    }
    catch (...)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "exception in header_callback");
        return 0; // error
    }
    return nitems;
}

static auto ProcessHeaders(::std::vector<OString> const& rHeaders) -> ::std::map<OUString, OUString>
{
    ::std::map<OUString, OUString> ret;
    for (OString const& rLine : rHeaders)
    {
        OString line;
        if (!rLine.endsWith("\r\n", &line))
        {
            SAL_WARN("ucb.ucp.webdav.curl", "invalid header field (no CRLF)");
            continue;
        }
        if (line.startsWith("HTTP/") // first line
            || line.isEmpty()) // last line
        {
            continue;
        }
        auto const nColon(line.indexOf(':'));
        if (nColon == -1)
        {
            {
                SAL_WARN("ucb.ucp.webdav.curl", "invalid header field (no :)");
            }
            continue;
        }
        if (nColon == 0)
        {
            SAL_WARN("ucb.ucp.webdav.curl", "invalid header field (empty name)");
            continue;
        }
        // case insensitive; must be ASCII
        auto const name(::rtl::OStringToOUString(line.copy(0, nColon).toAsciiLowerCase(),
                                                 RTL_TEXTENCODING_ASCII_US));
        sal_Int32 nStart(nColon + 1);
        while (nStart < line.getLength() && (line[nStart] == ' ' || line[nStart] == '\t'))
        {
            ++nStart;
        }
        sal_Int32 nEnd(line.getLength());
        while (nStart < nEnd && (line[nEnd - 1] == ' ' || line[nEnd - 1] == '\t'))
        {
            --nEnd;
        }
        // RFC 7230 says that only ASCII works reliably anyway (neon also did this)
        auto const value(::rtl::OStringToOUString(line.subView(nStart, nEnd - nStart),
                                                  RTL_TEXTENCODING_ASCII_US));
        auto const it(ret.find(name));
        if (it != ret.end())
        {
            it->second = it->second + "," + value;
        }
        else
        {
            ret[name] = value;
        }
    }
    return ret;
}

static auto ExtractRequestedHeaders(
    ResponseHeaders const& rHeaders,
    ::std::pair<::std::vector<OUString> const&, DAVResource&> const* const pRequestedHeaders)
    -> void
{
    ::std::map<OUString, OUString> const headerMap(
        ProcessHeaders(rHeaders.HeaderFields.back().first));
    if (pRequestedHeaders)
    {
        for (OUString const& rHeader : pRequestedHeaders->first)
        {
            auto const it(headerMap.find(rHeader.toAsciiLowerCase()));
            if (it != headerMap.end())
            {
                DAVPropertyValue value;
                value.IsCaseSensitive = false;
                value.Name = it->first;
                value.Value <<= it->second;
                pRequestedHeaders->second.properties.push_back(value);
            }
        }
    }
}

// this appears to be the only way to get the "realm" from libcurl
static auto ExtractRealm(ResponseHeaders const& rHeaders, char const* const pAuthHeaderName)
    -> ::std::optional<OUString>
{
    ::std::map<OUString, OUString> const headerMap(
        ProcessHeaders(rHeaders.HeaderFields.back().first));
    auto const it(headerMap.find(OUString::createFromAscii(pAuthHeaderName).toAsciiLowerCase()));
    if (it == headerMap.end())
    {
        SAL_WARN("ucb.ucp.webdav.curl", "cannot find auth header");
        return {};
    }
    // It may be possible that the header contains multiple methods each with
    // a different realm - extract only the first one bc the downstream API
    // only supports one anyway.
    // case insensitive!
    auto i(it->second.toAsciiLowerCase().indexOf("realm="));
    // is optional
    if (i == -1)
    {
        SAL_INFO("ucb.ucp.webdav.curl", "auth header has no realm");
        return {};
    }
    // no whitespace allowed before or after =
    i += ::std::strlen("realm=");
    if (it->second.getLength() < i + 2 || it->second[i] != '\"')
    {
        SAL_WARN("ucb.ucp.webdav.curl", "no realm value");
        return {};
    }
    ++i;
    OUStringBuffer buf;
    while (i < it->second.getLength() && it->second[i] != '\"')
    {
        if (it->second[i] == '\\') // quoted-pair escape
        {
            ++i;
            if (it->second.getLength() <= i)
            {
                SAL_WARN("ucb.ucp.webdav.curl", "unterminated quoted-pair");
                return {};
            }
        }
        buf.append(it->second[i]);
        ++i;
    }
    if (it->second.getLength() <= i)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "unterminated realm");
        return {};
    }
    return buf.makeStringAndClear();
}

CurlSession::CurlSession(uno::Reference<uno::XComponentContext> xContext,
                         ::rtl::Reference<DAVSessionFactory> const& rpFactory, OUString const& rURI,
                         uno::Sequence<beans::NamedValue> const& rFlags,
                         ::ucbhelper::InternetProxyDecider const& rProxyDecider)
    : DAVSession(rpFactory)
    , m_xContext(std::move(xContext))
    , m_Flags(rFlags)
    , m_URI(rURI)
    , m_Proxy(rProxyDecider.getProxy(m_URI.GetScheme(), m_URI.GetHost(), m_URI.GetPort()))
{
    assert(m_URI.GetScheme() == "http" || m_URI.GetScheme() == "https");
    m_pCurlMulti.reset(curl_multi_init());
    if (!m_pCurlMulti)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "curl_multi_init failed");
        throw DAVException(DAVException::DAV_SESSION_CREATE,
                           ConnectionEndPointString(m_URI.GetHost(), m_URI.GetPort()));
    }
    m_pCurl.reset(curl_easy_init());
    if (!m_pCurl)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "curl_easy_init failed");
        throw DAVException(DAVException::DAV_SESSION_CREATE,
                           ConnectionEndPointString(m_URI.GetHost(), m_URI.GetPort()));
    }
    m_ErrorBuffer[0] = '\0';
    // this supposedly gives the highest quality error reporting
    auto rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_ERRORBUFFER, m_ErrorBuffer);
    assert(rc == CURLE_OK);
#if 1
    // just for debugging...
    rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_DEBUGFUNCTION, debug_callback);
    assert(rc == CURLE_OK);
#endif
    rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_VERBOSE, 1L);
    assert(rc == CURLE_OK);
    // accept any encoding supported by libcurl
    rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_ACCEPT_ENCODING, "");
    if (rc != CURLE_OK)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "CURLOPT_ACCEPT_ENCODING failed: " << GetErrorString(rc));
        throw DAVException(DAVException::DAV_SESSION_CREATE,
                           ConnectionEndPointString(m_URI.GetHost(), m_URI.GetPort()));
    }
    auto const connectTimeout(officecfg::Inet::Settings::ConnectTimeout::get());
    // default is 300s
    rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_CONNECTTIMEOUT,
                          ::std::max<long>(2L, ::std::min<long>(connectTimeout, 180L)));
    if (rc != CURLE_OK)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "CURLOPT_CONNECTTIMEOUT failed: " << GetErrorString(rc));
        throw DAVException(DAVException::DAV_SESSION_CREATE,
                           ConnectionEndPointString(m_URI.GetHost(), m_URI.GetPort()));
    }
    auto const readTimeout(officecfg::Inet::Settings::ReadTimeout::get());
    m_nReadTimeout = ::std::max<int>(20, ::std::min<long>(readTimeout, 180)) * 1000;
    // default is infinite
    rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_TIMEOUT, 300L);
    if (rc != CURLE_OK)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "CURLOPT_TIMEOUT failed: " << GetErrorString(rc));
        throw DAVException(DAVException::DAV_SESSION_CREATE,
                           ConnectionEndPointString(m_URI.GetHost(), m_URI.GetPort()));
    }
    rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_WRITEFUNCTION, &write_callback);
    assert(rc == CURLE_OK);
    rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_READFUNCTION, &read_callback);
    assert(rc == CURLE_OK);
    rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_HEADERFUNCTION, &header_callback);
    assert(rc == CURLE_OK);
    ::InitCurl_easy(m_pCurl.get());
    if (officecfg::Office::Security::Net::AllowInsecureProtocols::get())
    {
    // tdf#149921 by default, with schannel (WNT) connection fails if revocation
    // lists cannot be checked; try to limit the checking to when revocation
    // lists can actually be retrieved (usually not the case for self-signed CA)
#if CURL_AT_LEAST_VERSION(7, 70, 0)
        rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_SSL_OPTIONS, CURLSSLOPT_REVOKE_BEST_EFFORT);
        assert(rc == CURLE_OK);
        rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_PROXY_SSL_OPTIONS,
                              CURLSSLOPT_REVOKE_BEST_EFFORT);
        assert(rc == CURLE_OK);
#endif
    }
    // set this initially, may be overwritten during authentication
    rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_HTTPAUTH, CURLAUTH_ANY);
    assert(rc == CURLE_OK); // ANY is always available
    // always set CURLOPT_PROXY to suppress proxy detection in libcurl
    OString const utf8Proxy(OUStringToOString(m_Proxy, RTL_TEXTENCODING_UTF8));
    rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_PROXY, utf8Proxy.getStr());
    if (rc != CURLE_OK)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "CURLOPT_PROXY failed: " << GetErrorString(rc));
        throw DAVException(DAVException::DAV_SESSION_CREATE, m_Proxy);
    }
    if (!m_Proxy.isEmpty())
    {
        // set this initially, may be overwritten during authentication
        rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_PROXYAUTH, CURLAUTH_ANY);
        assert(rc == CURLE_OK); // ANY is always available
    }
    auto const it(::std::find_if(m_Flags.begin(), m_Flags.end(),
                                 [](auto const& rFlag) { return rFlag.Name == "KeepAlive"; }));
    if (it != m_Flags.end() && it->Value.get<bool>())
    {
        // neon would close the connection from ne_end_request(), this seems
        // to be the equivalent and not CURLOPT_TCP_KEEPALIVE
        rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_FORBID_REUSE, 1L);
        assert(rc == CURLE_OK);
    }
    // If WOPI-like host has self-signed certificate, it's not possible to insert images
    // to the document, so here is a compromise. The user has already accepted the self
    // signed certificate in the browser, when we get here.
    if (comphelper::LibreOfficeKit::isActive())
    {
        rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_SSL_VERIFYPEER, 0L);
        assert(rc == CURLE_OK);
    }
}

CurlSession::~CurlSession() {}

auto CurlSession::CanUse(OUString const& rURI, uno::Sequence<beans::NamedValue> const& rFlags)
    -> bool
{
    try
    {
        CurlUri const uri(rURI);

        return m_URI.GetScheme() == uri.GetScheme() && m_URI.GetHost() == uri.GetHost()
               && m_URI.GetPort() == uri.GetPort() && m_Flags == rFlags;
    }
    catch (DAVException const&)
    {
        return false;
    }
}

auto CurlSession::UsesProxy() -> bool
{
    assert(m_URI.GetScheme() == "http" || m_URI.GetScheme() == "https");
    return !m_Proxy.isEmpty();
}

auto CurlSession::abort() -> void
{
    // note: abort() was a no-op since OOo 3.2 and before that it crashed.
    bool expected(false);
    // it would be pointless to lock m_Mutex here as the other thread holds it
    if (m_AbortFlag.compare_exchange_strong(expected, true))
    {
        // This function looks safe to call without m_Mutex as long as the
        // m_pCurlMulti handle is not destroyed, and the caller must own a ref
        // to this object which keeps it alive; it should cause poll to return.
        curl_multi_wakeup(m_pCurlMulti.get());
    }
}

/// this is just a bunch of static member functions called from CurlSession
struct CurlProcessor
{
    static auto URIReferenceToURI(CurlSession& rSession, std::u16string_view rURIReference)
        -> CurlUri;

    static auto ProcessRequestImpl(
        CurlSession& rSession, CurlUri const& rURI, OUString const& rMethod,
        curl_slist* pRequestHeaderList, uno::Reference<io::XOutputStream> const* pxOutStream,
        uno::Sequence<sal_Int8> const* pInData,
        ::std::pair<::std::vector<OUString> const&, DAVResource&> const* pRequestedHeaders,
        ResponseHeaders& rHeaders) -> void;

    static auto ProcessRequest(
        CurlSession& rSession, CurlUri const& rURI, OUString const& rMethod,
        ::std::vector<CurlOption> const& rOptions, DAVRequestEnvironment const* pEnv,
        ::std::unique_ptr<curl_slist, deleter_from_fn<curl_slist, curl_slist_free_all>>
            pRequestHeaderList,
        uno::Reference<io::XOutputStream> const* pxOutStream,
        uno::Reference<io::XInputStream> const* pxInStream,
        ::std::pair<::std::vector<OUString> const&, DAVResource&> const* pRequestedHeaders) -> void;

    static auto
    PropFind(CurlSession& rSession, CurlUri const& rURI, Depth depth,
             ::std::tuple<::std::vector<OUString> const&, ::std::vector<DAVResource>* const,
                          ::std::vector<ucb::Lock>* const> const* o_pRequestedProperties,
             ::std::vector<DAVResourceInfo>* const o_pResourceInfos,
             DAVRequestEnvironment const& rEnv) -> void;

    static auto MoveOrCopy(CurlSession& rSession, std::u16string_view rSourceURIReference,
                           ::std::u16string_view rDestinationURI, DAVRequestEnvironment const& rEnv,
                           bool isOverwrite, char const* pMethod) -> void;

    static auto Lock(CurlSession& rSession, CurlUri const& rURI, DAVRequestEnvironment const* pEnv,
                     ::std::unique_ptr<curl_slist, deleter_from_fn<curl_slist, curl_slist_free_all>>
                         pRequestHeaderList,
                     uno::Reference<io::XInputStream> const* pxInStream)
        -> ::std::vector<::std::pair<ucb::Lock, sal_Int32>>;

    static auto Unlock(CurlSession& rSession, CurlUri const& rURI,
                       DAVRequestEnvironment const* pEnv) -> void;
};

auto CurlProcessor::URIReferenceToURI(CurlSession& rSession, std::u16string_view rURIReference)
    -> CurlUri
{
    // No need to acquire rSession.m_Mutex because accessed members are const.
    if (rSession.UsesProxy())
    // very odd, but see DAVResourceAccess::getRequestURI() :-/
    {
        assert(o3tl::starts_with(rURIReference, u"http://")
               || o3tl::starts_with(rURIReference, u"https://"));
        return CurlUri(rURIReference);
    }
    else
    {
        assert(o3tl::starts_with(rURIReference, u"/"));
        return rSession.m_URI.CloneWithRelativeRefPathAbsolute(rURIReference);
    }
}

/// main function to initiate libcurl requests
auto CurlProcessor::ProcessRequestImpl(
    CurlSession& rSession, CurlUri const& rURI, OUString const& rMethod,
    curl_slist* const pRequestHeaderList,
    uno::Reference<io::XOutputStream> const* const pxOutStream,
    uno::Sequence<sal_Int8> const* const pInData,
    ::std::pair<::std::vector<OUString> const&, DAVResource&> const* const pRequestedHeaders,
    ResponseHeaders& rHeaders) -> void
{
    ::comphelper::ScopeGuard const g([&]() {
        auto rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_HEADERDATA, nullptr);
        assert(rc == CURLE_OK);
        (void)rc;
        if (pxOutStream)
        {
            rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_WRITEDATA, nullptr);
            assert(rc == CURLE_OK);
        }
        if (pInData)
        {
            rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_READDATA, nullptr);
            assert(rc == CURLE_OK);
            rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_UPLOAD, 0L);
            assert(rc == CURLE_OK);
        }
        if (pRequestHeaderList)
        {
            rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_HTTPHEADER, nullptr);
            assert(rc == CURLE_OK);
        }
    });

    if (pRequestHeaderList)
    {
        auto rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_HTTPHEADER, pRequestHeaderList);
        assert(rc == CURLE_OK);
        (void)rc;
    }

    auto rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_CURLU, rURI.GetCURLU());
    assert(rc == CURLE_OK); // can't fail since 7.63.0

    rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_HEADERDATA, &rHeaders);
    assert(rc == CURLE_OK);
    ::std::optional<DownloadTarget> oDownloadTarget;
    if (pxOutStream)
    {
        oDownloadTarget.emplace(*pxOutStream, rHeaders);
        rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_WRITEDATA, &*oDownloadTarget);
        assert(rc == CURLE_OK);
    }
    ::std::optional<UploadSource> oUploadSource;
    if (pInData)
    {
        oUploadSource.emplace(*pInData);
        rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_READDATA, &*oUploadSource);
        assert(rc == CURLE_OK);
    }
    rSession.m_ErrorBuffer[0] = '\0';

    // note: easy handle must be added for *every* transfer!
    // otherwise it gets stuck in MSTATE_MSGSENT forever after 1st transfer
    auto mc = curl_multi_add_handle(rSession.m_pCurlMulti.get(), rSession.m_pCurl.get());
    if (mc != CURLM_OK)
    {
        SAL_WARN("ucb.ucp.webdav.curl",
                 "curl_multi_add_handle failed: " << GetErrorStringMulti(mc));
        throw DAVException(
            DAVException::DAV_SESSION_CREATE,
            ConnectionEndPointString(rSession.m_URI.GetHost(), rSession.m_URI.GetPort()));
    }
    ::comphelper::ScopeGuard const gg([&]() {
        mc = curl_multi_remove_handle(rSession.m_pCurlMulti.get(), rSession.m_pCurl.get());
        if (mc != CURLM_OK)
        {
            SAL_WARN("ucb.ucp.webdav.curl",
                     "curl_multi_remove_handle failed: " << GetErrorStringMulti(mc));
        }
    });

    // this is where libcurl actually does something
    rc = CURL_LAST; // clear current value
    int nRunning;
    do
    {
        mc = curl_multi_perform(rSession.m_pCurlMulti.get(), &nRunning);
        if (mc != CURLM_OK)
        {
            SAL_WARN("ucb.ucp.webdav.curl",
                     "curl_multi_perform failed: " << GetErrorStringMulti(mc));
            throw DAVException(
                DAVException::DAV_HTTP_CONNECT,
                ConnectionEndPointString(rSession.m_URI.GetHost(), rSession.m_URI.GetPort()));
        }
        if (nRunning == 0)
        { // short request like HEAD on loopback could be done in first call
            break;
        }
        int nFDs;
        mc = curl_multi_poll(rSession.m_pCurlMulti.get(), nullptr, 0, rSession.m_nReadTimeout,
                             &nFDs);
        if (mc != CURLM_OK)
        {
            SAL_WARN("ucb.ucp.webdav.curl", "curl_multi_poll failed: " << GetErrorStringMulti(mc));
            throw DAVException(
                DAVException::DAV_HTTP_CONNECT,
                ConnectionEndPointString(rSession.m_URI.GetHost(), rSession.m_URI.GetPort()));
        }
        if (rSession.m_AbortFlag.load())
        { // flag was set by abort() -> not sure what exception to throw?
            throw DAVException(DAVException::DAV_HTTP_ERROR, "abort() was called", 0);
        }
    } while (nRunning != 0);
    // there should be exactly 1 CURLMsg now, but the interface is
    // extensible so future libcurl versions could yield additional things
    do
    {
        CURLMsg const* const pMsg = curl_multi_info_read(rSession.m_pCurlMulti.get(), &nRunning);
        if (pMsg && pMsg->msg == CURLMSG_DONE)
        {
            assert(pMsg->easy_handle == rSession.m_pCurl.get());
            rc = pMsg->data.result;
        }
        else
        {
            SAL_WARN("ucb.ucp.webdav.curl", "curl_multi_info_read unexpected result");
        }
    } while (nRunning != 0);

    // error handling part 1: libcurl errors
    if (rc != CURLE_OK)
    {
        // TODO: is there any value in extracting CURLINFO_OS_ERRNO
        SAL_WARN("ucb.ucp.webdav.curl",
                 "curl_easy_perform failed: " << GetErrorString(rc, rSession.m_ErrorBuffer));
        switch (rc)
        {
            case CURLE_UNSUPPORTED_PROTOCOL:
                throw DAVException(DAVException::DAV_UNSUPPORTED);
            case CURLE_COULDNT_RESOLVE_PROXY:
                throw DAVException(DAVException::DAV_HTTP_LOOKUP, rSession.m_Proxy);
            case CURLE_COULDNT_RESOLVE_HOST:
                throw DAVException(
                    DAVException::DAV_HTTP_LOOKUP,
                    ConnectionEndPointString(rSession.m_URI.GetHost(), rSession.m_URI.GetPort()));
            case CURLE_COULDNT_CONNECT:
            case CURLE_SSL_CONNECT_ERROR:
            case CURLE_SSL_CERTPROBLEM:
            case CURLE_SSL_CIPHER:
            case CURLE_PEER_FAILED_VERIFICATION:
            case CURLE_SSL_ISSUER_ERROR:
            case CURLE_SSL_PINNEDPUBKEYNOTMATCH:
            case CURLE_SSL_INVALIDCERTSTATUS:
            case CURLE_FAILED_INIT:
#if CURL_AT_LEAST_VERSION(7, 69, 0)
            case CURLE_QUIC_CONNECT_ERROR:
#endif
                throw DAVException(
                    DAVException::DAV_HTTP_CONNECT,
                    ConnectionEndPointString(rSession.m_URI.GetHost(), rSession.m_URI.GetPort()));
            case CURLE_REMOTE_ACCESS_DENIED:
            case CURLE_LOGIN_DENIED:
            case CURLE_AUTH_ERROR:
                throw DAVException(
                    DAVException::DAV_HTTP_AUTH, // probably?
                    ConnectionEndPointString(rSession.m_URI.GetHost(), rSession.m_URI.GetPort()));
            case CURLE_WRITE_ERROR:
            case CURLE_READ_ERROR: // error returned from our callbacks
            case CURLE_OUT_OF_MEMORY:
            case CURLE_ABORTED_BY_CALLBACK:
            case CURLE_BAD_FUNCTION_ARGUMENT:
            case CURLE_SEND_ERROR:
            case CURLE_RECV_ERROR:
            case CURLE_SSL_CACERT_BADFILE:
            case CURLE_SSL_CRL_BADFILE:
            case CURLE_RECURSIVE_API_CALL:
                throw DAVException(
                    DAVException::DAV_HTTP_FAILED,
                    ConnectionEndPointString(rSession.m_URI.GetHost(), rSession.m_URI.GetPort()));
            case CURLE_OPERATION_TIMEDOUT:
                throw DAVException(
                    DAVException::DAV_HTTP_TIMEOUT,
                    ConnectionEndPointString(rSession.m_URI.GetHost(), rSession.m_URI.GetPort()));
            default: // lots of generic errors
                throw DAVException(DAVException::DAV_HTTP_ERROR, "", 0);
        }
    }
    // error handling part 2: HTTP status codes
    long statusCode(SC_NONE);
    rc = curl_easy_getinfo(rSession.m_pCurl.get(), CURLINFO_RESPONSE_CODE, &statusCode);
    assert(rc == CURLE_OK);
    assert(statusCode != SC_NONE); // ??? should be error returned from perform?
    SAL_INFO("ucb.ucp.webdav.curl", "HTTP status code: " << statusCode);
    if (statusCode < 300)
    {
        // neon did this regardless of status or even error, which seems odd
        ExtractRequestedHeaders(rHeaders, pRequestedHeaders);
    }
    else
    {
        // create message containing the HTTP method and response status line
        OUString statusLine("\n" + rMethod + "\n=>\n");
        if (!rHeaders.HeaderFields.empty() && !rHeaders.HeaderFields.back().first.empty()
            && rHeaders.HeaderFields.back().first.front().startsWith("HTTP"))
        {
            statusLine += ::rtl::OStringToOUString(
                ::o3tl::trim(rHeaders.HeaderFields.back().first.front()),
                RTL_TEXTENCODING_ASCII_US);
        }
        switch (statusCode)
        {
            case SC_REQUEST_TIMEOUT:
            {
                throw DAVException(
                    DAVException::DAV_HTTP_TIMEOUT,
                    ConnectionEndPointString(rSession.m_URI.GetHost(), rSession.m_URI.GetPort()));
                break;
            }
            case SC_MOVED_PERMANENTLY:
            case SC_MOVED_TEMPORARILY:
            case SC_SEE_OTHER:
            case SC_TEMPORARY_REDIRECT:
            {
                // could also use CURLOPT_FOLLOWLOCATION but apparently the
                // upper layer wants to know about redirects?
                char* pRedirectURL(nullptr);
                rc = curl_easy_getinfo(rSession.m_pCurl.get(), CURLINFO_REDIRECT_URL,
                                       &pRedirectURL);
                assert(rc == CURLE_OK);
                if (pRedirectURL)
                {
                    // Sharepoint 2016 workaround: contains unencoded U+0020
                    OUString const redirectURL(::rtl::Uri::encode(
                        pRedirectURL
                            ? OUString(pRedirectURL, strlen(pRedirectURL), RTL_TEXTENCODING_UTF8)
                            : OUString(),
                        rtl_UriCharClassUric, rtl_UriEncodeKeepEscapes, RTL_TEXTENCODING_UTF8));

                    throw DAVException(DAVException::DAV_HTTP_REDIRECT, redirectURL);
                }
                [[fallthrough]];
            }
            default:
                throw DAVException(DAVException::DAV_HTTP_ERROR, statusLine, statusCode);
        }
    }

    if (pxOutStream)
    {
        (*pxOutStream)->closeOutput(); // signal EOF
    }
}

static auto TryRemoveExpiredLockToken(CurlSession& rSession, CurlUri const& rURI,
                                      DAVRequestEnvironment const* const pEnv) -> bool
{
    if (!pEnv)
    {
        // caller was a NonInteractive_*LOCK function anyway, its caller is LockStore
        return false;
    }
    OUString const* const pToken(g_Init.LockStore.getLockTokenForURI(rURI.GetURI(), nullptr));
    if (!pToken)
    {
        return false;
    }
    try
    {
        // determine validity of existing lock via lockdiscovery request
        ::std::vector<OUString> const propertyNames{ DAVProperties::LOCKDISCOVERY };
        ::std::vector<ucb::Lock> locks;
        ::std::tuple<::std::vector<OUString> const&, ::std::vector<DAVResource>* const,
                     ::std::vector<ucb::Lock>* const> const args(propertyNames, nullptr, &locks);

        CurlProcessor::PropFind(rSession, rURI, DAVZERO, &args, nullptr, *pEnv);

        // https://datatracker.ietf.org/doc/html/rfc4918#section-15.8
        // The response MAY not contain tokens, but hopefully it
        // will if client is properly authenticated.
        if (::std::any_of(locks.begin(), locks.end(), [pToken](ucb::Lock const& rLock) {
                return ::std::any_of(
                    rLock.LockTokens.begin(), rLock.LockTokens.end(),
                    [pToken](OUString const& rToken) { return *pToken == rToken; });
            }))
        {
            return false; // still have the lock
        }

        SAL_INFO("ucb.ucp.webdav.curl",
                 "lock token expired, removing: " << rURI.GetURI() << " " << *pToken);
        g_Init.LockStore.removeLock(rURI.GetURI());
        return true;
    }
    catch (DAVException const&)
    {
        return false; // ignore, the caller already has a better exception
    }
}

auto CurlProcessor::ProcessRequest(
    CurlSession& rSession, CurlUri const& rURI, OUString const& rMethod,
    ::std::vector<CurlOption> const& rOptions, DAVRequestEnvironment const* const pEnv,
    ::std::unique_ptr<curl_slist, deleter_from_fn<curl_slist, curl_slist_free_all>>
        pRequestHeaderList,
    uno::Reference<io::XOutputStream> const* const pxOutStream,
    uno::Reference<io::XInputStream> const* const pxInStream,
    ::std::pair<::std::vector<OUString> const&, DAVResource&> const* const pRequestedHeaders)
    -> void
{
    if (HostFilter::isForbidden(rURI.GetHost()))
    {
        SAL_WARN("ucb.ucp.webdav.curl", "Access denied to host: " << rURI.GetHost());
        throw uno::RuntimeException("access to host denied");
    }

    if (pEnv)
    { // add custom request headers passed by caller
        for (auto const& rHeader : pEnv->m_aRequestHeaders)
        {
            OString const utf8Header(
                OUStringToOString(rHeader.first, RTL_TEXTENCODING_ASCII_US) + ": "
                + OUStringToOString(rHeader.second, RTL_TEXTENCODING_ASCII_US));
            pRequestHeaderList.reset(
                curl_slist_append(pRequestHeaderList.release(), utf8Header.getStr()));
            if (!pRequestHeaderList)
            {
                throw uno::RuntimeException("curl_slist_append failed");
            }
        }
    }

    uno::Sequence<sal_Int8> data;
    if (pxInStream)
    {
        uno::Reference<io::XSeekable> const xSeekable(*pxInStream, uno::UNO_QUERY);
        if (xSeekable.is())
        {
            auto const len(xSeekable->getLength() - xSeekable->getPosition());
            if ((**pxInStream).readBytes(data, len) != len)
            {
                throw uno::RuntimeException("short readBytes");
            }
        }
        else
        {
            ::std::vector<uno::Sequence<sal_Int8>> bufs;
            bool isDone(false);
            do
            {
                bufs.emplace_back();
                isDone = (**pxInStream).readSomeBytes(bufs.back(), 65536) == 0;
            } while (!isDone);
            sal_Int32 nSize(0);
            for (auto const& rBuf : bufs)
            {
                if (o3tl::checked_add(nSize, rBuf.getLength(), nSize))
                {
                    throw std::bad_alloc(); // too large for Sequence
                }
            }
            data.realloc(nSize);
            size_t nCopied(0);
            for (auto const& rBuf : bufs)
            {
                ::std::memcpy(data.getArray() + nCopied, rBuf.getConstArray(), rBuf.getLength());
                nCopied += rBuf.getLength(); // can't overflow
            }
        }
    }

    // Clear flag before transfer starts; only a transfer started before
    // calling abort() will be aborted, not one started later.
    rSession.m_AbortFlag.store(false);

    Guard guard(rSession.m_Mutex, rOptions, rURI, rSession.m_pCurl.get());

    // authentication data may be in the URI, or requested via XInteractionHandler
    struct Auth
    {
        OUString UserName;
        OUString PassWord;
        decltype(CURLAUTH_ANY) AuthMask; ///< allowed auth methods
        Auth(OUString aUserName, OUString aPassword, decltype(CURLAUTH_ANY) const & rAuthMask)
            : UserName(std::move(aUserName))
            , PassWord(std::move(aPassword))
            , AuthMask(rAuthMask)
        {
        }
    };
    ::std::optional<Auth> oAuth;
    ::std::optional<Auth> oAuthProxy;
    if (pEnv && !rSession.m_isAuthenticatedProxy && !rSession.m_Proxy.isEmpty())
    {
        try
        {
            // the hope is that this must be a URI
            CurlUri const uri(rSession.m_Proxy);
            if (!uri.GetUser().isEmpty() || !uri.GetPassword().isEmpty())
            {
                oAuthProxy.emplace(uri.GetUser(), uri.GetPassword(), CURLAUTH_ANY);
            }
        }
        catch (DAVException&)
        {
            // ignore any parsing failure here
        }
    }
    decltype(CURLAUTH_ANY) const authSystem(CURLAUTH_NEGOTIATE | CURLAUTH_NTLM | CURLAUTH_NTLM_WB);
    if (pRequestedHeaders || (pEnv && !rSession.m_isAuthenticated))
    {
    // m_aRequestURI *may* be a path or *may* be URI - wtf
    // TODO: why is there this m_aRequestURI and also rURIReference argument?
    // ... only caller is DAVResourceAccess - always identical except MOVE/COPY
    // which doesn't work if it's just a URI reference so let's just use
    // rURIReference via rURI instead
#if 0
        CurlUri const uri(pEnv->m_aRequestURI);
#endif
        // note: due to parsing bug pwd didn't work in previous webdav ucps
        if (pEnv && !rSession.m_isAuthenticated
            && (!rURI.GetUser().isEmpty() || !rURI.GetPassword().isEmpty()))
        {
            oAuth.emplace(rURI.GetUser(), rURI.GetPassword(), CURLAUTH_ANY);
        }
        if (pRequestedHeaders)
        {
            // note: Previously this would be the rURIReference directly but
            // that ends up in CurlUri anyway and curl is unhappy.
            // But it looks like all consumers of this .uri are interested
            // only in the path, so it shouldn't make a difference to give
            // the entire URI when the caller extracts the path anyway.
            pRequestedHeaders->second.uri = rURI.GetURI();
            pRequestedHeaders->second.properties.clear();
        }
    }
    bool isRetry(false);
    bool isFallbackHTTP10(false);
    int nAuthRequests(0);
    int nAuthRequestsProxy(0);

    // libcurl does not have an authentication callback so handle auth
    // related status codes and requesting credentials via this loop
    do
    {
        isRetry = false;

        // re-check m_isAuthenticated flags every time, could have been set
        // by re-entrant call
        if (oAuth && !rSession.m_isAuthenticated)
        {
            OString const utf8UserName(OUStringToOString(oAuth->UserName, RTL_TEXTENCODING_UTF8));
            auto rc
                = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_USERNAME, utf8UserName.getStr());
            if (rc != CURLE_OK)
            {
                SAL_WARN("ucb.ucp.webdav.curl", "CURLOPT_USERNAME failed: " << GetErrorString(rc));
                throw DAVException(DAVException::DAV_INVALID_ARG);
            }
            OString const utf8PassWord(OUStringToOString(oAuth->PassWord, RTL_TEXTENCODING_UTF8));
            rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_PASSWORD, utf8PassWord.getStr());
            if (rc != CURLE_OK)
            {
                SAL_WARN("ucb.ucp.webdav.curl", "CURLOPT_PASSWORD failed: " << GetErrorString(rc));
                throw DAVException(DAVException::DAV_INVALID_ARG);
            }
            rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_HTTPAUTH, oAuth->AuthMask);
            assert(
                rc
                == CURLE_OK); // it shouldn't be possible to reduce auth to 0 via the authSystem masks
        }

        if (oAuthProxy && !rSession.m_isAuthenticatedProxy)
        {
            OString const utf8UserName(
                OUStringToOString(oAuthProxy->UserName, RTL_TEXTENCODING_UTF8));
            auto rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_PROXYUSERNAME,
                                       utf8UserName.getStr());
            if (rc != CURLE_OK)
            {
                SAL_WARN("ucb.ucp.webdav.curl",
                         "CURLOPT_PROXYUSERNAME failed: " << GetErrorString(rc));
                throw DAVException(DAVException::DAV_INVALID_ARG);
            }
            OString const utf8PassWord(
                OUStringToOString(oAuthProxy->PassWord, RTL_TEXTENCODING_UTF8));
            rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_PROXYPASSWORD,
                                  utf8PassWord.getStr());
            if (rc != CURLE_OK)
            {
                SAL_WARN("ucb.ucp.webdav.curl",
                         "CURLOPT_PROXYPASSWORD failed: " << GetErrorString(rc));
                throw DAVException(DAVException::DAV_INVALID_ARG);
            }
            rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_PROXYAUTH, oAuthProxy->AuthMask);
            assert(
                rc
                == CURLE_OK); // it shouldn't be possible to reduce auth to 0 via the authSystem masks
        }

        ResponseHeaders headers(rSession.m_pCurl.get());
        // always pass a stream for debug logging, buffer the result body
        uno::Reference<io::XSequenceOutputStream> const xSeqOutStream(
            io::SequenceOutputStream::create(rSession.m_xContext));
        uno::Reference<io::XOutputStream> const xTempOutStream(xSeqOutStream);
        assert(xTempOutStream.is());

        try
        {
            ProcessRequestImpl(rSession, rURI, rMethod, pRequestHeaderList.get(), &xTempOutStream,
                               pxInStream ? &data : nullptr, pRequestedHeaders, headers);
            if (pxOutStream)
            { // only copy to result stream if transfer was successful
                (*pxOutStream)->writeBytes(xSeqOutStream->getWrittenBytes());
                (*pxOutStream)->closeOutput(); // signal EOF
            }
        }
        catch (DAVException const& rException)
        {
            // log start of request body if there was any
            auto const bytes(xSeqOutStream->getWrittenBytes());
            auto const len(::std::min<sal_Int32>(bytes.getLength(), 10000));
            SAL_INFO("ucb.ucp.webdav.curl",
                     "DAVException; (first) " << len << " bytes of data received:");
            if (0 < len)
            {
                OStringBuffer buf(len);
                for (sal_Int32 i = 0; i < len; ++i)
                {
                    if (bytes[i] < 0x20) // also if negative
                    {
                        static char const hexDigit[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
                                                           '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
                        buf.append(OString::Concat("\\x")
                                   + OStringChar(hexDigit[static_cast<sal_uInt8>(bytes[i]) >> 4])
                                   + OStringChar(hexDigit[bytes[i] & 0x0F]));
                    }
                    else
                    {
                        buf.append(static_cast<char>(bytes[i]));
                    }
                }
                SAL_INFO("ucb.ucp.webdav.curl", buf.makeStringAndClear());
            }

            // error handling part 3: special HTTP status codes
            // that require unlocking m_Mutex to handle
            if (rException.getError() == DAVException::DAV_HTTP_ERROR)
            {
                auto const statusCode(rException.getStatus());
                switch (statusCode)
                {
                    case SC_LOCKED:
                    {
                        guard.Release(); // release m_Mutex before accessing LockStore
                        if (g_Init.LockStore.getLockTokenForURI(rURI.GetURI(), nullptr))
                        {
                            throw DAVException(DAVException::DAV_LOCKED_SELF);
                        }
                        else // locked by third party
                        {
                            throw DAVException(DAVException::DAV_LOCKED);
                        }
                        break;
                    }
                    case SC_PRECONDITION_FAILED:
                    case SC_BAD_REQUEST:
                    {
                        guard.Release(); // release m_Mutex before accessing LockStore
                        // Not obvious but apparently these codes may indicate
                        // the expiration of a lock.
                        // Initiate a new request *outside* ProcessRequestImpl
                        // *after* rGuard.unlock() to avoid messing up m_pCurl state.
                        if (TryRemoveExpiredLockToken(rSession, rURI, pEnv))
                        {
                            throw DAVException(DAVException::DAV_LOCK_EXPIRED);
                        }
                        break;
                    }
                    case SC_UNAUTHORIZED:
                    case SC_PROXY_AUTHENTICATION_REQUIRED:
                    {
                        (statusCode != SC_PROXY_AUTHENTICATION_REQUIRED
                             ? rSession.m_isAuthenticated
                             : rSession.m_isAuthenticatedProxy)
                            = false; // any auth data in m_pCurl is invalid
                        auto& rnAuthRequests(statusCode == SC_UNAUTHORIZED ? nAuthRequests
                                                                           : nAuthRequestsProxy);
                        if (rnAuthRequests == 10)
                        {
                            SAL_INFO("ucb.ucp.webdav.curl", "aborting authentication after "
                                                                << rnAuthRequests << " attempts");
                        }
                        else if (pEnv && pEnv->m_xAuthListener)
                        {
                            ::std::optional<OUString> const oRealm(ExtractRealm(
                                headers, statusCode == SC_UNAUTHORIZED ? "WWW-Authenticate"
                                                                       : "Proxy-Authenticate"));

                            ::std::optional<Auth>& roAuth(
                                statusCode == SC_UNAUTHORIZED ? oAuth : oAuthProxy);
                            OUString userName(roAuth ? roAuth->UserName : OUString());
                            OUString passWord(roAuth ? roAuth->PassWord : OUString());
                            long authAvail(0);
                            auto const rc = curl_easy_getinfo(rSession.m_pCurl.get(),
                                                              statusCode == SC_UNAUTHORIZED
                                                                  ? CURLINFO_HTTPAUTH_AVAIL
                                                                  : CURLINFO_PROXYAUTH_AVAIL,
                                                              &authAvail);
                            assert(rc == CURLE_OK);
                            (void)rc;
                            // only allow SystemCredentials once - the
                            // PasswordContainer may have stored it in the
                            // Config (TrySystemCredentialsFirst or
                            // AuthenticateUsingSystemCredentials) and then it
                            // will always force its use no matter how hopeless
                            bool const isSystemCredSupported((authAvail & authSystem) != 0
                                                             && rnAuthRequests == 0);
                            ++rnAuthRequests;

                            // Ask user via XInteractionHandler.
                            // Warning: This likely runs an event loop which may
                            // end up calling back into this instance, so all
                            // changes to m_pCurl must be undone now and
                            // restored after return.
                            guard.Release();

                            auto const ret = pEnv->m_xAuthListener->authenticate(
                                oRealm ? *oRealm : "",
                                statusCode == SC_UNAUTHORIZED ? rSession.m_URI.GetHost()
                                                              : rSession.m_Proxy,
                                userName, passWord, isSystemCredSupported);

                            if (ret == 0)
                            {
                                // NTLM may either use a password requested
                                // from the user, or from the system via SSPI
                                // so i guess it should not be disabled here
                                // regardless of the state of the system auth
                                // checkbox, particularly since SSPI is only
                                // available on WNT.
                                // Additionally, "Negotiate" has a "legacy"
                                // mode that is actually just NTLM according to
                                // https://curl.se/rfc/ntlm.html#ntlmHttpAuthentication
                                // so there's nothing in authSystem that can be
                                // disabled here.
                                roAuth.emplace(userName, passWord,
                                               ((userName.isEmpty() && passWord.isEmpty())
                                                    ? (authAvail & authSystem)
                                                    : authAvail));
                                isRetry = true;
                                // Acquire is only necessary in case of success.
                                guard.Acquire();
                                break; // break out of switch
                            }
                            // else: throw
                        }
                        SAL_INFO("ucb.ucp.webdav.curl", "no auth credentials provided");
                        throw DAVException(DAVException::DAV_HTTP_NOAUTH,
                                           ConnectionEndPointString(rSession.m_URI.GetHost(),
                                                                    rSession.m_URI.GetPort()));
                        break;
                    }
                }
            }
            else if (rException.getError() == DAVException::DAV_UNSUPPORTED)
            {
                // tdf#152493 libcurl can't handle "Transfer-Encoding: chunked"
                // in HTTP/1.1 100 Continue response.
                // workaround: if HTTP/1.1 didn't work, try HTTP/1.0
                // (but fallback only once - to prevent infinite loop)
                if (isFallbackHTTP10)
                {
                    throw DAVException(DAVException::DAV_HTTP_ERROR);
                }
                isFallbackHTTP10 = true;
                // note: this is not reset - future requests to this URI use it!
                auto rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_HTTP_VERSION,
                                           CURL_HTTP_VERSION_1_0);
                if (rc != CURLE_OK)
                {
                    throw DAVException(DAVException::DAV_HTTP_ERROR);
                }
                SAL_INFO("ucb.ucp.webdav.curl", "attempting fallback to HTTP/1.0");
                isRetry = true;
            }
            if (!isRetry)
            {
                throw; // everything else: re-throw
            }
        }
    } while (isRetry);

    if (oAuth)
    {
        // assume this worked, leave auth data as stored in m_pCurl
        rSession.m_isAuthenticated = true;
    }
    if (oAuthProxy)
    {
        // assume this worked, leave auth data as stored in m_pCurl
        rSession.m_isAuthenticatedProxy = true;
    }
}

auto CurlSession::OPTIONS(OUString const& rURIReference,

                          DAVOptions& rOptions, DAVRequestEnvironment const& rEnv) -> void
{
    SAL_INFO("ucb.ucp.webdav.curl", "OPTIONS: " << rURIReference);

    rOptions.init();

    CurlUri const uri(CurlProcessor::URIReferenceToURI(*this, rURIReference));

    ::std::vector<OUString> const headerNames{ "allow", "dav" };
    DAVResource result;
    ::std::pair<::std::vector<OUString> const&, DAVResource&> const headers(headerNames, result);

    ::std::vector<CurlOption> const options{ { CURLOPT_CUSTOMREQUEST, "OPTIONS",
                                               "CURLOPT_CUSTOMREQUEST" } };

    CurlProcessor::ProcessRequest(*this, uri, "OPTIONS", options, &rEnv, nullptr, nullptr, nullptr,
                                  &headers);

    for (auto const& it : result.properties)
    {
        OUString value;
        it.Value >>= value;
        SAL_INFO("ucb.ucp.webdav.curl", "OPTIONS: header: " << it.Name << ": " << value);
        if (it.Name.equalsIgnoreAsciiCase("allow"))
        {
            rOptions.setAllowedMethods(value);
        }
        else if (it.Name.equalsIgnoreAsciiCase("dav"))
        {
            // see <http://tools.ietf.org/html/rfc4918#section-10.1>,
            // <http://tools.ietf.org/html/rfc4918#section-18>,
            // and <http://tools.ietf.org/html/rfc7230#section-3.2>
            // we detect the class (1, 2 and 3), other elements (token, URL)
            // are not used for now
            auto const list(::comphelper::string::convertCommaSeparated(value));
            for (OUString const& v : list)
            {
                if (v == "1")
                {
                    rOptions.setClass1();
                }
                else if (v == "2")
                {
                    rOptions.setClass2();
                }
                else if (v == "3")
                {
                    rOptions.setClass3();
                }
            }
        }
    }
    if (rOptions.isClass2() || rOptions.isClass3())
    {
        if (g_Init.LockStore.getLockTokenForURI(uri.GetURI(), nullptr))
        {
            rOptions.setLocked();
        }
    }
}

auto CurlProcessor::PropFind(
    CurlSession& rSession, CurlUri const& rURI, Depth const nDepth,
    ::std::tuple<::std::vector<OUString> const&, ::std::vector<DAVResource>* const,
                 ::std::vector<ucb::Lock>* const> const* const o_pRequestedProperties,
    ::std::vector<DAVResourceInfo>* const o_pResourceInfos, DAVRequestEnvironment const& rEnv)
    -> void
{
    assert((o_pRequestedProperties != nullptr) != (o_pResourceInfos != nullptr));
    assert((o_pRequestedProperties == nullptr)
           || (::std::get<1>(*o_pRequestedProperties) != nullptr)
                  != (::std::get<2>(*o_pRequestedProperties) != nullptr));

    ::std::unique_ptr<curl_slist, deleter_from_fn<curl_slist, curl_slist_free_all>> pList;
    pList.reset(curl_slist_append(pList.release(), "Content-Type: application/xml"));
    if (!pList)
    {
        throw uno::RuntimeException("curl_slist_append failed");
    }
    OString depth;
    switch (nDepth)
    {
        case DAVZERO:
            depth = "Depth: 0"_ostr;
            break;
        case DAVONE:
            depth = "Depth: 1"_ostr;
            break;
        case DAVINFINITY:
            depth = "Depth: infinity"_ostr;
            break;
        default:
            assert(false);
    }
    pList.reset(curl_slist_append(pList.release(), depth.getStr()));
    if (!pList)
    {
        throw uno::RuntimeException("curl_slist_append failed");
    }

    uno::Reference<io::XSequenceOutputStream> const xSeqOutStream(
        io::SequenceOutputStream::create(rSession.m_xContext));
    uno::Reference<io::XOutputStream> const xRequestOutStream(xSeqOutStream);
    assert(xRequestOutStream.is());

    uno::Reference<xml::sax::XWriter> const xWriter(xml::sax::Writer::create(rSession.m_xContext));
    xWriter->setOutputStream(xRequestOutStream);
    xWriter->startDocument();
    rtl::Reference<::comphelper::AttributeList> const pAttrList(new ::comphelper::AttributeList);
    pAttrList->AddAttribute("xmlns", "DAV:");
    xWriter->startElement("propfind", pAttrList);
    if (o_pResourceInfos)
    {
        xWriter->startElement("propname", nullptr);
        xWriter->endElement("propname");
    }
    else
    {
        if (::std::get<0>(*o_pRequestedProperties).empty())
        {
            xWriter->startElement("allprop", nullptr);
            xWriter->endElement("allprop");
        }
        else
        {
            xWriter->startElement("prop", nullptr);
            for (OUString const& rName : ::std::get<0>(*o_pRequestedProperties))
            {
                SerfPropName name;
                DAVProperties::createSerfPropName(rName, name);
                pAttrList->Clear();
                pAttrList->AddAttribute("xmlns", OUString::createFromAscii(name.nspace));
                xWriter->startElement(OUString::createFromAscii(name.name), pAttrList);
                xWriter->endElement(OUString::createFromAscii(name.name));
            }
            xWriter->endElement("prop");
        }
    }
    xWriter->endElement("propfind");
    xWriter->endDocument();

    uno::Reference<io::XInputStream> const xRequestInStream(
        io::SequenceInputStream::createStreamFromSequence(rSession.m_xContext,
                                                          xSeqOutStream->getWrittenBytes()));
    assert(xRequestInStream.is());

    curl_off_t const len(xSeqOutStream->getWrittenBytes().getLength());

    ::std::vector<CurlOption> const options{
        { CURLOPT_UPLOAD, 1L, nullptr },
        { CURLOPT_CUSTOMREQUEST, "PROPFIND", "CURLOPT_CUSTOMREQUEST" },
        // note: Sharepoint cannot handle "Transfer-Encoding: chunked"
        { CURLOPT_INFILESIZE_LARGE, len, nullptr, CurlOption::Type::CurlOffT }
    };

    // stream for response
    uno::Reference<io::XInputStream> const xResponseInStream(io::Pipe::create(rSession.m_xContext));
    uno::Reference<io::XOutputStream> const xResponseOutStream(xResponseInStream, uno::UNO_QUERY);
    assert(xResponseInStream.is());
    assert(xResponseOutStream.is());

    CurlProcessor::ProcessRequest(rSession, rURI, "PROPFIND", options, &rEnv, ::std::move(pList),
                                  &xResponseOutStream, &xRequestInStream, nullptr);

    if (o_pResourceInfos)
    {
        *o_pResourceInfos = parseWebDAVPropNameResponse(xResponseInStream);
    }
    else
    {
        if (::std::get<1>(*o_pRequestedProperties) != nullptr)
        {
            *::std::get<1>(*o_pRequestedProperties)
                = parseWebDAVPropFindResponse(xResponseInStream);
            for (DAVResource& it : *::std::get<1>(*o_pRequestedProperties))
            {
                // caller will give these uris to CurlUri so can't be relative
                if (it.uri.startsWith("/"))
                {
                    try
                    {
                        it.uri = rSession.m_URI.CloneWithRelativeRefPathAbsolute(it.uri).GetURI();
                    }
                    catch (DAVException const&)
                    {
                        SAL_INFO("ucb.ucp.webdav.curl",
                                 "PROPFIND: exception parsing uri " << it.uri);
                    }
                }
            }
        }
        else
        {
            *::std::get<2>(*o_pRequestedProperties) = parseWebDAVLockResponse(xResponseInStream);
        }
    }
}

// DAV methods
auto CurlSession::PROPFIND(OUString const& rURIReference, Depth const depth,
                           ::std::vector<OUString> const& rPropertyNames,
                           ::std::vector<DAVResource>& o_rResources,
                           DAVRequestEnvironment const& rEnv) -> void
{
    SAL_INFO("ucb.ucp.webdav.curl", "PROPFIND: " << rURIReference << " " << depth);

    CurlUri const uri(CurlProcessor::URIReferenceToURI(*this, rURIReference));

    ::std::tuple<::std::vector<OUString> const&, ::std::vector<DAVResource>* const,
                 ::std::vector<ucb::Lock>* const> const args(rPropertyNames, &o_rResources,
                                                             nullptr);
    return CurlProcessor::PropFind(*this, uri, depth, &args, nullptr, rEnv);
}

auto CurlSession::PROPFIND(OUString const& rURIReference, Depth const depth,
                           ::std::vector<DAVResourceInfo>& o_rResourceInfos,
                           DAVRequestEnvironment const& rEnv) -> void
{
    SAL_INFO("ucb.ucp.webdav.curl", "PROPFIND: " << rURIReference << " " << depth);

    CurlUri const uri(CurlProcessor::URIReferenceToURI(*this, rURIReference));

    return CurlProcessor::PropFind(*this, uri, depth, nullptr, &o_rResourceInfos, rEnv);
}

auto CurlSession::PROPPATCH(OUString const& rURIReference,
                            ::std::vector<ProppatchValue> const& rValues,
                            DAVRequestEnvironment const& rEnv) -> void
{
    SAL_INFO("ucb.ucp.webdav.curl", "PROPPATCH: " << rURIReference);

    CurlUri const uri(CurlProcessor::URIReferenceToURI(*this, rURIReference));

    // TODO: either set CURLOPT_INFILESIZE_LARGE or chunked?
    ::std::unique_ptr<curl_slist, deleter_from_fn<curl_slist, curl_slist_free_all>> pList;
    pList.reset(curl_slist_append(pList.release(), "Content-Type: application/xml"));
    if (!pList)
    {
        throw uno::RuntimeException("curl_slist_append failed");
    }

    // generate XML document for PROPPATCH
    uno::Reference<io::XSequenceOutputStream> const xSeqOutStream(
        io::SequenceOutputStream::create(m_xContext));
    uno::Reference<io::XOutputStream> const xRequestOutStream(xSeqOutStream);
    assert(xRequestOutStream.is());
    uno::Reference<xml::sax::XWriter> const xWriter(xml::sax::Writer::create(m_xContext));
    xWriter->setOutputStream(xRequestOutStream);
    xWriter->startDocument();
    rtl::Reference<::comphelper::AttributeList> const pAttrList(new ::comphelper::AttributeList);
    pAttrList->AddAttribute("xmlns", "DAV:");
    xWriter->startElement("propertyupdate", pAttrList);
    for (ProppatchValue const& rPropValue : rValues)
    {
        assert(rPropValue.operation == PROPSET || rPropValue.operation == PROPREMOVE);
        OUString const operation((rPropValue.operation == PROPSET) ? OUString("set")
                                                                   : OUString("remove"));
        xWriter->startElement(operation, nullptr);
        xWriter->startElement("prop", nullptr);
        SerfPropName name;
        DAVProperties::createSerfPropName(rPropValue.name, name);
        pAttrList->Clear();
        pAttrList->AddAttribute("xmlns", OUString::createFromAscii(name.nspace));
        xWriter->startElement(OUString::createFromAscii(name.name), pAttrList);
        if (rPropValue.operation == PROPSET)
        {
            if (DAVProperties::isUCBDeadProperty(name))
            {
                ::std::optional<::std::pair<OUString, OUString>> const oProp(
                    UCBDeadPropertyValue::toXML(rPropValue.value));
                if (oProp)
                {
                    xWriter->startElement("ucbprop", nullptr);
                    xWriter->startElement("type", nullptr);
                    xWriter->characters(oProp->first);
                    xWriter->endElement("type");
                    xWriter->startElement("value", nullptr);
                    xWriter->characters(oProp->second);
                    xWriter->endElement("value");
                    xWriter->endElement("ucbprop");
                }
            }
            else
            {
                OUString value;
                rPropValue.value >>= value;
                xWriter->characters(value);
            }
        }
        xWriter->endElement(OUString::createFromAscii(name.name));
        xWriter->endElement("prop");
        xWriter->endElement(operation);
    }
    xWriter->endElement("propertyupdate");
    xWriter->endDocument();

    uno::Reference<io::XInputStream> const xRequestInStream(
        io::SequenceInputStream::createStreamFromSequence(m_xContext,
                                                          xSeqOutStream->getWrittenBytes()));
    assert(xRequestInStream.is());

    curl_off_t const len(xSeqOutStream->getWrittenBytes().getLength());

    ::std::vector<CurlOption> const options{
        { CURLOPT_UPLOAD, 1L, nullptr },
        { CURLOPT_CUSTOMREQUEST, "PROPPATCH", "CURLOPT_CUSTOMREQUEST" },
        // note: Sharepoint cannot handle "Transfer-Encoding: chunked"
        { CURLOPT_INFILESIZE_LARGE, len, nullptr, CurlOption::Type::CurlOffT }
    };

    CurlProcessor::ProcessRequest(*this, uri, "PROPPATCH", options, &rEnv, ::std::move(pList),
                                  nullptr, &xRequestInStream, nullptr);
}

auto CurlSession::HEAD(OUString const& rURIReference, ::std::vector<OUString> const& rHeaderNames,
                       DAVResource& io_rResource, DAVRequestEnvironment const& rEnv) -> void
{
    SAL_INFO("ucb.ucp.webdav.curl", "HEAD: " << rURIReference);

    CurlUri const uri(CurlProcessor::URIReferenceToURI(*this, rURIReference));

    ::std::vector<CurlOption> const options{
        // NOBODY will prevent logging the response body in ProcessRequest()
        // exception, but omitting it here results in a long timeout until the
        // server closes the connection, which is worse
        { CURLOPT_NOBODY, 1L, nullptr },
        { CURLOPT_CUSTOMREQUEST, "HEAD", "CURLOPT_CUSTOMREQUEST" }
    };

    ::std::pair<::std::vector<OUString> const&, DAVResource&> const headers(rHeaderNames,
                                                                            io_rResource);

    CurlProcessor::ProcessRequest(*this, uri, "HEAD", options, &rEnv, nullptr, nullptr, nullptr,
                                  &headers);
}

auto CurlSession::GET(OUString const& rURIReference, DAVRequestEnvironment const& rEnv)
    -> uno::Reference<io::XInputStream>
{
    SAL_INFO("ucb.ucp.webdav.curl", "GET: " << rURIReference);

    CurlUri const uri(CurlProcessor::URIReferenceToURI(*this, rURIReference));

    // could use either com.sun.star.io.Pipe or com.sun.star.io.SequenceInputStream?
    // Pipe can just write into its XOuputStream, which is simpler.
    // Both resize exponentially, so performance should be fine.
    // However, Pipe doesn't implement XSeekable, which is required by filters.

    uno::Reference<io::XSequenceOutputStream> const xSeqOutStream(
        io::SequenceOutputStream::create(m_xContext));
    uno::Reference<io::XOutputStream> const xResponseOutStream(xSeqOutStream);
    assert(xResponseOutStream.is());

    ::std::vector<CurlOption> const options{ { CURLOPT_HTTPGET, 1L, nullptr } };

    CurlProcessor::ProcessRequest(*this, uri, "GET", options, &rEnv, nullptr, &xResponseOutStream,
                                  nullptr, nullptr);

    uno::Reference<io::XInputStream> const xResponseInStream(
        io::SequenceInputStream::createStreamFromSequence(m_xContext,
                                                          xSeqOutStream->getWrittenBytes()));
    assert(xResponseInStream.is());

    return xResponseInStream;
}

auto CurlSession::GET(OUString const& rURIReference, uno::Reference<io::XOutputStream>& rxOutStream,
                      DAVRequestEnvironment const& rEnv) -> void
{
    SAL_INFO("ucb.ucp.webdav.curl", "GET: " << rURIReference);

    CurlUri const uri(CurlProcessor::URIReferenceToURI(*this, rURIReference));

    ::std::vector<CurlOption> const options{ { CURLOPT_HTTPGET, 1L, nullptr } };

    CurlProcessor::ProcessRequest(*this, uri, "GET", options, &rEnv, nullptr, &rxOutStream, nullptr,
                                  nullptr);
}

auto CurlSession::GET(OUString const& rURIReference, ::std::vector<OUString> const& rHeaderNames,
                      DAVResource& io_rResource, DAVRequestEnvironment const& rEnv)
    -> uno::Reference<io::XInputStream>
{
    SAL_INFO("ucb.ucp.webdav.curl", "GET: " << rURIReference);

    CurlUri const uri(CurlProcessor::URIReferenceToURI(*this, rURIReference));

    ::std::vector<CurlOption> const options{ { CURLOPT_HTTPGET, 1L, nullptr } };

    uno::Reference<io::XSequenceOutputStream> const xSeqOutStream(
        io::SequenceOutputStream::create(m_xContext));
    uno::Reference<io::XOutputStream> const xResponseOutStream(xSeqOutStream);
    assert(xResponseOutStream.is());

    ::std::pair<::std::vector<OUString> const&, DAVResource&> const headers(rHeaderNames,
                                                                            io_rResource);

    CurlProcessor::ProcessRequest(*this, uri, "GET", options, &rEnv, nullptr, &xResponseOutStream,
                                  nullptr, &headers);

    uno::Reference<io::XInputStream> const xResponseInStream(
        io::SequenceInputStream::createStreamFromSequence(m_xContext,
                                                          xSeqOutStream->getWrittenBytes()));
    assert(xResponseInStream.is());

    return xResponseInStream;
}

auto CurlSession::GET(OUString const& rURIReference, uno::Reference<io::XOutputStream>& rxOutStream,
                      ::std::vector<OUString> const& rHeaderNames, DAVResource& io_rResource,
                      DAVRequestEnvironment const& rEnv) -> void
{
    SAL_INFO("ucb.ucp.webdav.curl", "GET: " << rURIReference);

    CurlUri const uri(CurlProcessor::URIReferenceToURI(*this, rURIReference));

    ::std::vector<CurlOption> const options{ { CURLOPT_HTTPGET, 1L, nullptr } };

    ::std::pair<::std::vector<OUString> const&, DAVResource&> const headers(rHeaderNames,
                                                                            io_rResource);

    CurlProcessor::ProcessRequest(*this, uri, "GET", options, &rEnv, nullptr, &rxOutStream, nullptr,
                                  &headers);
}

auto CurlSession::PUT(OUString const& rURIReference,
                      uno::Reference<io::XInputStream> const& rxInStream,
                      DAVRequestEnvironment const& rEnv) -> void
{
    SAL_INFO("ucb.ucp.webdav.curl", "PUT: " << rURIReference);

    CurlUri const uri(CurlProcessor::URIReferenceToURI(*this, rURIReference));

    // NextCloud silently fails with chunked encoding
    uno::Reference<io::XSeekable> const xSeekable(rxInStream, uno::UNO_QUERY);
    if (!xSeekable.is())
    {
        throw uno::RuntimeException("TODO: not seekable");
    }
    curl_off_t const len(xSeekable->getLength() - xSeekable->getPosition());

    ::std::unique_ptr<curl_slist, deleter_from_fn<curl_slist, curl_slist_free_all>> pList;
    OUString const* const pToken(g_Init.LockStore.getLockTokenForURI(uri.GetURI(), nullptr));
    if (pToken)
    {
        OString const utf8If("If: "
        // disabled as Sharepoint 2013 workaround, it accepts only
        // "No-Tag-List", see fed2984281a85a5a2f308841ec810f218c75f2ab
#if 0
                "<" + OUStringToOString(rURIReference, RTL_TEXTENCODING_ASCII_US)
                             + "> "
#endif
                             "(<"
                             + OUStringToOString(*pToken, RTL_TEXTENCODING_ASCII_US) + ">)");
        pList.reset(curl_slist_append(pList.release(), utf8If.getStr()));
        if (!pList)
        {
            throw uno::RuntimeException("curl_slist_append failed");
        }
    }

    // lock m_Mutex after accessing global LockStore to avoid deadlock

    // note: Nextcloud 20 cannot handle "Transfer-Encoding: chunked"
    ::std::vector<CurlOption> const options{
        { CURLOPT_UPLOAD, 1L, nullptr }, // libcurl won't upload without setting this
        { CURLOPT_INFILESIZE_LARGE, len, nullptr, CurlOption::Type::CurlOffT }
    };

    CurlProcessor::ProcessRequest(*this, uri, "PUT", options, &rEnv, ::std::move(pList), nullptr,
                                  &rxInStream, nullptr);
}

auto CurlSession::POST(OUString const& rURIReference, OUString const& rContentType,
                       OUString const& rReferer, uno::Reference<io::XInputStream> const& rxInStream,
                       DAVRequestEnvironment const& rEnv) -> uno::Reference<io::XInputStream>
{
    SAL_INFO("ucb.ucp.webdav.curl", "POST: " << rURIReference);

    CurlUri const uri(CurlProcessor::URIReferenceToURI(*this, rURIReference));

    // TODO: either set CURLOPT_POSTFIELDSIZE_LARGE or chunked?
    ::std::unique_ptr<curl_slist, deleter_from_fn<curl_slist, curl_slist_free_all>> pList(
        curl_slist_append(nullptr, "Transfer-Encoding: chunked"));
    if (!pList)
    {
        throw uno::RuntimeException("curl_slist_append failed");
    }
    OString const utf8ContentType("Content-Type: "
                                  + OUStringToOString(rContentType, RTL_TEXTENCODING_ASCII_US));
    pList.reset(curl_slist_append(pList.release(), utf8ContentType.getStr()));
    if (!pList)
    {
        throw uno::RuntimeException("curl_slist_append failed");
    }
    OString const utf8Referer("Referer: " + OUStringToOString(rReferer, RTL_TEXTENCODING_ASCII_US));
    pList.reset(curl_slist_append(pList.release(), utf8Referer.getStr()));
    if (!pList)
    {
        throw uno::RuntimeException("curl_slist_append failed");
    }

    ::std::vector<CurlOption> const options{ { CURLOPT_POST, 1L, nullptr } };

    uno::Reference<io::XSequenceOutputStream> const xSeqOutStream(
        io::SequenceOutputStream::create(m_xContext));
    uno::Reference<io::XOutputStream> const xResponseOutStream(xSeqOutStream);
    assert(xResponseOutStream.is());

    CurlProcessor::ProcessRequest(*this, uri, "POST", options, &rEnv, ::std::move(pList),
                                  &xResponseOutStream, &rxInStream, nullptr);

    uno::Reference<io::XInputStream> const xResponseInStream(
        io::SequenceInputStream::createStreamFromSequence(m_xContext,
                                                          xSeqOutStream->getWrittenBytes()));
    assert(xResponseInStream.is());

    return xResponseInStream;
}

auto CurlSession::POST(OUString const& rURIReference, OUString const& rContentType,
                       OUString const& rReferer, uno::Reference<io::XInputStream> const& rxInStream,
                       uno::Reference<io::XOutputStream>& rxOutStream,
                       DAVRequestEnvironment const& rEnv) -> void
{
    SAL_INFO("ucb.ucp.webdav.curl", "POST: " << rURIReference);

    CurlUri const uri(CurlProcessor::URIReferenceToURI(*this, rURIReference));

    // TODO: either set CURLOPT_POSTFIELDSIZE_LARGE or chunked?
    ::std::unique_ptr<curl_slist, deleter_from_fn<curl_slist, curl_slist_free_all>> pList(
        curl_slist_append(nullptr, "Transfer-Encoding: chunked"));
    if (!pList)
    {
        throw uno::RuntimeException("curl_slist_append failed");
    }
    OString const utf8ContentType("Content-Type: "
                                  + OUStringToOString(rContentType, RTL_TEXTENCODING_ASCII_US));
    pList.reset(curl_slist_append(pList.release(), utf8ContentType.getStr()));
    if (!pList)
    {
        throw uno::RuntimeException("curl_slist_append failed");
    }
    OString const utf8Referer("Referer: " + OUStringToOString(rReferer, RTL_TEXTENCODING_ASCII_US));
    pList.reset(curl_slist_append(pList.release(), utf8Referer.getStr()));
    if (!pList)
    {
        throw uno::RuntimeException("curl_slist_append failed");
    }

    ::std::vector<CurlOption> const options{ { CURLOPT_POST, 1L, nullptr } };

    CurlProcessor::ProcessRequest(*this, uri, "POST", options, &rEnv, ::std::move(pList),
                                  &rxOutStream, &rxInStream, nullptr);
}

auto CurlSession::MKCOL(OUString const& rURIReference, DAVRequestEnvironment const& rEnv) -> void
{
    SAL_INFO("ucb.ucp.webdav.curl", "MKCOL: " << rURIReference);

    CurlUri const uri(CurlProcessor::URIReferenceToURI(*this, rURIReference));

    ::std::vector<CurlOption> const options{ { CURLOPT_CUSTOMREQUEST, "MKCOL",
                                               "CURLOPT_CUSTOMREQUEST" } };

    CurlProcessor::ProcessRequest(*this, uri, "MKCOL", options, &rEnv, nullptr, nullptr, nullptr,
                                  nullptr);
}

auto CurlProcessor::MoveOrCopy(CurlSession& rSession, std::u16string_view rSourceURIReference,
                               ::std::u16string_view const rDestinationURI,
                               DAVRequestEnvironment const& rEnv, bool const isOverwrite,
                               char const* const pMethod) -> void
{
    CurlUri const uriSource(CurlProcessor::URIReferenceToURI(rSession, rSourceURIReference));

    OString const utf8Destination("Destination: "
                                  + OUStringToOString(rDestinationURI, RTL_TEXTENCODING_ASCII_US));
    ::std::unique_ptr<curl_slist, deleter_from_fn<curl_slist, curl_slist_free_all>> pList(
        curl_slist_append(nullptr, utf8Destination.getStr()));
    if (!pList)
    {
        throw uno::RuntimeException("curl_slist_append failed");
    }
    OString const utf8Overwrite(OString::Concat("Overwrite: ") + (isOverwrite ? "T" : "F"));
    pList.reset(curl_slist_append(pList.release(), utf8Overwrite.getStr()));
    if (!pList)
    {
        throw uno::RuntimeException("curl_slist_append failed");
    }

    ::std::vector<CurlOption> const options{ { CURLOPT_CUSTOMREQUEST, pMethod,
                                               "CURLOPT_CUSTOMREQUEST" } };

    CurlProcessor::ProcessRequest(rSession, uriSource, OUString::createFromAscii(pMethod), options,
                                  &rEnv, ::std::move(pList), nullptr, nullptr, nullptr);
}

auto CurlSession::COPY(OUString const& rSourceURIReference, OUString const& rDestinationURI,
                       DAVRequestEnvironment const& rEnv, bool const isOverwrite) -> void
{
    SAL_INFO("ucb.ucp.webdav.curl", "COPY: " << rSourceURIReference);

    return CurlProcessor::MoveOrCopy(*this, rSourceURIReference, rDestinationURI, rEnv, isOverwrite,
                                     "COPY");
}

auto CurlSession::MOVE(OUString const& rSourceURIReference, OUString const& rDestinationURI,
                       DAVRequestEnvironment const& rEnv, bool const isOverwrite) -> void
{
    SAL_INFO("ucb.ucp.webdav.curl", "MOVE: " << rSourceURIReference);

    return CurlProcessor::MoveOrCopy(*this, rSourceURIReference, rDestinationURI, rEnv, isOverwrite,
                                     "MOVE");
}

auto CurlSession::DESTROY(OUString const& rURIReference, DAVRequestEnvironment const& rEnv) -> void
{
    SAL_INFO("ucb.ucp.webdav.curl", "DESTROY: " << rURIReference);

    CurlUri const uri(CurlProcessor::URIReferenceToURI(*this, rURIReference));

    ::std::vector<CurlOption> const options{ { CURLOPT_CUSTOMREQUEST, "DELETE",
                                               "CURLOPT_CUSTOMREQUEST" } };

    CurlProcessor::ProcessRequest(*this, uri, "DESTROY", options, &rEnv, nullptr, nullptr, nullptr,
                                  nullptr);
}

auto CurlProcessor::Lock(
    CurlSession& rSession, CurlUri const& rURI, DAVRequestEnvironment const* const pEnv,
    ::std::unique_ptr<curl_slist, deleter_from_fn<curl_slist, curl_slist_free_all>>
        pRequestHeaderList,
    uno::Reference<io::XInputStream> const* const pxRequestInStream)
    -> ::std::vector<::std::pair<ucb::Lock, sal_Int32>>
{
    curl_off_t len(0);
    if (pxRequestInStream)
    {
        uno::Reference<io::XSeekable> const xSeekable(*pxRequestInStream, uno::UNO_QUERY);
        assert(xSeekable.is());
        len = xSeekable->getLength();
    }

    ::std::vector<CurlOption> const options{
        { CURLOPT_UPLOAD, 1L, nullptr },
        { CURLOPT_CUSTOMREQUEST, "LOCK", "CURLOPT_CUSTOMREQUEST" },
        // note: Sharepoint cannot handle "Transfer-Encoding: chunked"
        { CURLOPT_INFILESIZE_LARGE, len, nullptr, CurlOption::Type::CurlOffT }
    };

    // stream for response
    uno::Reference<io::XInputStream> const xResponseInStream(io::Pipe::create(rSession.m_xContext));
    uno::Reference<io::XOutputStream> const xResponseOutStream(xResponseInStream, uno::UNO_QUERY);
    assert(xResponseInStream.is());
    assert(xResponseOutStream.is());

    TimeValue startTime;
    osl_getSystemTime(&startTime);

    CurlProcessor::ProcessRequest(rSession, rURI, "LOCK", options, pEnv,
                                  ::std::move(pRequestHeaderList), &xResponseOutStream,
                                  pxRequestInStream, nullptr);

    ::std::vector<ucb::Lock> const acquiredLocks(parseWebDAVLockResponse(xResponseInStream));
    SAL_WARN_IF(acquiredLocks.empty(), "ucb.ucp.webdav.curl",
                "could not get LOCK for " << rURI.GetURI());

    TimeValue endTime;
    osl_getSystemTime(&endTime);
    auto const elapsedSeconds(endTime.Seconds - startTime.Seconds);

    // determine expiration time (seconds from endTime) for each acquired lock
    ::std::vector<::std::pair<ucb::Lock, sal_Int32>> ret;
    ret.reserve(acquiredLocks.size());
    for (auto const& rLock : acquiredLocks)
    {
        sal_Int32 lockExpirationTimeSeconds;
        if (rLock.Timeout == -1)
        {
            lockExpirationTimeSeconds = -1;
        }
        else if (rLock.Timeout <= elapsedSeconds)
        {
            SAL_WARN("ucb.ucp.webdav.curl",
                     "LOCK timeout already expired when receiving LOCK response for "
                         << rURI.GetURI());
            lockExpirationTimeSeconds = 0;
        }
        else
        {
            lockExpirationTimeSeconds = startTime.Seconds + rLock.Timeout;
        }
        ret.emplace_back(rLock, lockExpirationTimeSeconds);
    }

    return ret;
}

auto CurlSession::LOCK(OUString const& rURIReference, ucb::Lock /*const*/& rLock,
                       DAVRequestEnvironment const& rEnv) -> void
{
    SAL_INFO("ucb.ucp.webdav.curl", "LOCK: " << rURIReference);

    CurlUri const uri(CurlProcessor::URIReferenceToURI(*this, rURIReference));

    if (g_Init.LockStore.getLockTokenForURI(uri.GetURI(), &rLock))
    {
        // already have a lock that covers the requirement
        // TODO: maybe use DAV:lockdiscovery to ensure it's valid
        return;
    }

    // note: no m_Mutex lock needed here, only in CurlProcessor::Lock()

    // generate XML document for acquiring new LOCK
    uno::Reference<io::XSequenceOutputStream> const xSeqOutStream(
        io::SequenceOutputStream::create(m_xContext));
    uno::Reference<io::XOutputStream> const xRequestOutStream(xSeqOutStream);
    assert(xRequestOutStream.is());
    uno::Reference<xml::sax::XWriter> const xWriter(xml::sax::Writer::create(m_xContext));
    xWriter->setOutputStream(xRequestOutStream);
    xWriter->startDocument();
    rtl::Reference<::comphelper::AttributeList> const pAttrList(new ::comphelper::AttributeList);
    pAttrList->AddAttribute("xmlns", "DAV:");
    xWriter->startElement("lockinfo", pAttrList);
    xWriter->startElement("lockscope", nullptr);
    switch (rLock.Scope)
    {
        case ucb::LockScope_EXCLUSIVE:
            xWriter->startElement("exclusive", nullptr);
            xWriter->endElement("exclusive");
            break;
        case ucb::LockScope_SHARED:
            xWriter->startElement("shared", nullptr);
            xWriter->endElement("shared");
            break;
        default:
            assert(false);
    }
    xWriter->endElement("lockscope");
    xWriter->startElement("locktype", nullptr);
    xWriter->startElement("write", nullptr);
    xWriter->endElement("write");
    xWriter->endElement("locktype");
    OUString owner;
    if ((rLock.Owner >>= owner) && !owner.isEmpty())
    {
        xWriter->startElement("owner", nullptr);
        xWriter->characters(owner);
        xWriter->endElement("owner");
    }
    xWriter->endElement("lockinfo");
    xWriter->endDocument();

    uno::Reference<io::XInputStream> const xRequestInStream(
        io::SequenceInputStream::createStreamFromSequence(m_xContext,
                                                          xSeqOutStream->getWrittenBytes()));
    assert(xRequestInStream.is());

    ::std::unique_ptr<curl_slist, deleter_from_fn<curl_slist, curl_slist_free_all>> pList;
    pList.reset(curl_slist_append(pList.release(), "Content-Type: application/xml"));
    if (!pList)
    {
        throw uno::RuntimeException("curl_slist_append failed");
    }
    OString depth;
    switch (rLock.Depth)
    {
        case ucb::LockDepth_ZERO:
            depth = "Depth: 0"_ostr;
            break;
        case ucb::LockDepth_ONE:
            depth = "Depth: 1"_ostr;
            break;
        case ucb::LockDepth_INFINITY:
            depth = "Depth: infinity"_ostr;
            break;
        default:
            assert(false);
    }
    pList.reset(curl_slist_append(pList.release(), depth.getStr()));
    if (!pList)
    {
        throw uno::RuntimeException("curl_slist_append failed");
    }
    OString timeout;
    switch (rLock.Timeout)
    {
        case -1:
            timeout = "Timeout: Infinite"_ostr;
            break;
        case 0:
            timeout = "Timeout: Second-180"_ostr;
            break;
        default:
            timeout = "Timeout: Second-" + OString::number(rLock.Timeout);
            assert(0 < rLock.Timeout);
            break;
    }
    pList.reset(curl_slist_append(pList.release(), timeout.getStr()));
    if (!pList)
    {
        throw uno::RuntimeException("curl_slist_append failed");
    }

    auto const acquiredLocks
        = CurlProcessor::Lock(*this, uri, &rEnv, ::std::move(pList), &xRequestInStream);

    for (auto const& rAcquiredLock : acquiredLocks)
    {
        g_Init.LockStore.addLock(uri.GetURI(), rAcquiredLock.first,
                                 rAcquiredLock.first.LockTokens[0], this, rAcquiredLock.second);
        SAL_INFO("ucb.ucp.webdav.curl", "created LOCK for " << rURIReference);
    }
}

auto CurlProcessor::Unlock(CurlSession& rSession, CurlUri const& rURI,
                           DAVRequestEnvironment const* const pEnv) -> void
{
    OUString const* const pToken(g_Init.LockStore.getLockTokenForURI(rURI.GetURI(), nullptr));
    if (!pToken)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "attempt to unlock but not locked");
        throw DAVException(DAVException::DAV_NOT_LOCKED);
    }
    OString const utf8LockToken("Lock-Token: <"
                                + OUStringToOString(*pToken, RTL_TEXTENCODING_ASCII_US) + ">");
    ::std::unique_ptr<curl_slist, deleter_from_fn<curl_slist, curl_slist_free_all>> pList(
        curl_slist_append(nullptr, utf8LockToken.getStr()));
    if (!pList)
    {
        throw uno::RuntimeException("curl_slist_append failed");
    }

    ::std::vector<CurlOption> const options{ { CURLOPT_CUSTOMREQUEST, "UNLOCK",
                                               "CURLOPT_CUSTOMREQUEST" } };

    CurlProcessor::ProcessRequest(rSession, rURI, "UNLOCK", options, pEnv, ::std::move(pList),
                                  nullptr, nullptr, nullptr);
}

auto CurlSession::UNLOCK(OUString const& rURIReference, DAVRequestEnvironment const& rEnv) -> void
{
    SAL_INFO("ucb.ucp.webdav.curl", "UNLOCK: " << rURIReference);

    // note: no m_Mutex lock needed here, only in CurlProcessor::Unlock()

    CurlUri const uri(CurlProcessor::URIReferenceToURI(*this, rURIReference));

    CurlProcessor::Unlock(*this, uri, &rEnv);

    g_Init.LockStore.removeLock(uri.GetURI());
}

auto CurlSession::NonInteractive_LOCK(OUString const& rURI, ::std::u16string_view const rLockToken,
                                      sal_Int32& o_rLastChanceToSendRefreshRequest,
                                      bool& o_rIsAuthFailed) -> bool
{
    SAL_INFO("ucb.ucp.webdav.curl", "NonInteractive_LOCK: " << rURI);

    // note: no m_Mutex lock needed here, only in CurlProcessor::Lock()

    try
    {
        CurlUri const uri(rURI);
        ::std::unique_ptr<curl_slist, deleter_from_fn<curl_slist, curl_slist_free_all>> pList(
            curl_slist_append(nullptr, "Timeout: Second-180"));

        assert(!rLockToken.empty()); // LockStore is the caller
        OString const utf8If("If: (<" + OUStringToOString(rLockToken, RTL_TEXTENCODING_ASCII_US)
                             + ">)");
        pList.reset(curl_slist_append(pList.release(), utf8If.getStr()));
        if (!pList)
        {
            throw uno::RuntimeException("curl_slist_append failed");
        }

        auto const acquiredLocks
            = CurlProcessor::Lock(*this, uri, nullptr, ::std::move(pList), nullptr);

        SAL_WARN_IF(1 < acquiredLocks.size(), "ucb.ucp.webdav.curl",
                    "multiple locks acquired on refresh for " << rURI);
        if (!acquiredLocks.empty())
        {
            o_rLastChanceToSendRefreshRequest = acquiredLocks.begin()->second;
        }
        SAL_INFO("ucb.ucp.webdav.curl", "NonInteractive_LOCK succeeded on " << rURI);
        return true;
    }
    catch (DAVException const& rException)
    {
        SAL_INFO("ucb.ucp.webdav.curl", "NonInteractive_LOCK failed on " << rURI);
        switch (rException.getError())
        {
            case DAVException::DAV_HTTP_AUTH:
            case DAVException::DAV_HTTP_NOAUTH:
                o_rIsAuthFailed = true;
                break;
            default:
                break;
        }
        return false;
    }
    catch (...)
    {
        SAL_INFO("ucb.ucp.webdav.curl", "NonInteractive_LOCK failed on " << rURI);
        return false;
    }
}

auto CurlSession::NonInteractive_UNLOCK(OUString const& rURI) -> void
{
    SAL_INFO("ucb.ucp.webdav.curl", "NonInteractive_UNLOCK: " << rURI);

    // note: no m_Mutex lock needed here, only in CurlProcessor::Unlock()

    try
    {
        CurlUri const uri(rURI);

        CurlProcessor::Unlock(*this, uri, nullptr);

        // the only caller is the dtor of the LockStore, don't call remove!
        SAL_INFO("ucb.ucp.webdav.curl", "NonInteractive_UNLOCK succeeded on " << rURI);
    }
    catch (...)
    {
        SAL_INFO("ucb.ucp.webdav.curl", "NonInteractive_UNLOCK failed on " << rURI);
    }
}

} // namespace http_dav_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
