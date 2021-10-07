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
#include "webdavresponseparser.hxx"

#include <comphelper/attributelist.hxx>
#include <comphelper/scopeguard.hxx>

#include <officecfg/Inet.hxx>

#include <com/sun/star/io/Pipe.hpp>
#include <com/sun/star/io/SequenceInputStream.hpp>
#include <com/sun/star/io/SequenceOutputStream.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>

#include <osl/time.h>
#include <sal/log.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <config_version.h>

#include <map>
#include <optional>
#include <tuple>
#include <vector>

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
    DownloadTarget(uno::Reference<io::XOutputStream> const& i_xOutStream,
                   ResponseHeaders const& i_rHeaders)
        : xOutStream(i_xOutStream)
        , rHeaders(i_rHeaders)
    {
    }
};

struct UploadSource
{
    uno::Reference<io::XInputStream> xInStream;
    ResponseHeaders const& rHeaders;
    UploadSource(uno::Reference<io::XInputStream> const& i_xInStream,
                 ResponseHeaders const& i_rHeaders)
        : xInStream(i_xInStream)
        , rHeaders(i_rHeaders)
    {
    }
};

/// combined guard class to ensure things are released in correct order,
/// particularly in ProcessRequest() error handling
class Guard
{
private:
    /// mutex *first* because m_oGuard requires it
    ::std::unique_lock<::std::mutex> m_Lock;
    ::std::optional<::comphelper::ScopeGuard<::std::function<void()>>> m_oGuard;

public:
    explicit Guard(::std::mutex& rMutex)
        : m_Lock(rMutex)
    {
    }
    template <class Func>
    explicit Guard(::std::mutex& rMutex, Func&& rFunc)
        : m_Lock(rMutex)
        , m_oGuard(::std::move(rFunc))
    {
    }
#if 0
    void unlock()
    {
        m_oGuard.reset();
        m_Lock.unlock();
    }
#endif
};

} // namespace

namespace http_dav_ucp
{
static auto GetErrorString(CURLcode const rc, char const* const pErrorBuffer = nullptr) -> OString
{
    char const* const pMessage( // static fallback
        (pErrorBuffer && pErrorBuffer[0] != '\0') ? pErrorBuffer : curl_easy_strerror(rc));
    return OString::Concat("(") + OString::number(sal_Int32(rc)) + ") " + pMessage;
}

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
            pType = "CURLINFO_HEADER_IN";
            break;
        case CURLINFO_HEADER_OUT:
            pType = "CURLINFO_HEADER_OUT";
            break;
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
    assert(pTarget->xOutStream.is());
    auto const oResponseCode(GetResponseCode(pTarget->rHeaders));
    if (!oResponseCode)
    {
        return 0; // that is an error
    }
    if (200 <= *oResponseCode && *oResponseCode < 300)
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
    assert(pSource->xInStream.is());
    size_t const nBytes(size * nitems);
    size_t nRet(0);
    try
    {
        uno::Sequence<sal_Int8> data;
        data.realloc(nBytes);
        nRet = pSource->xInStream->readSomeBytes(data, nBytes);
        ::std::memcpy(buffer, data.getConstArray(), nRet);
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
    assert(2 <= nitems); // FIXME just for testing
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

CurlSession::CurlSession(uno::Reference<uno::XComponentContext> const& xContext,
                         ::rtl::Reference<DAVSessionFactory> const& rpFactory, OUString const& rURI,
                         ::ucbhelper::InternetProxyDecider const& rProxyDecider)
    : DAVSession(rpFactory)
    , m_xContext(xContext)
    , m_URI(rURI)
    , m_Proxy(rProxyDecider.getProxy(m_URI.GetScheme(), m_URI.GetHost(), m_URI.GetPort()))
{
    assert(m_URI.GetScheme() == "http" || m_URI.GetScheme() == "https");
    m_pCurl.reset(curl_easy_init());
    if (!m_pCurl)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "curl_easy_init failed");
        throw DAVException(DAVException::DAV_SESSION_CREATE,
                           ConnectionEndPointString(m_URI.GetHost(), m_URI.GetPort()));
    }
    curl_version_info_data const* const pVersion(curl_version_info(CURLVERSION_NOW));
    assert(pVersion);
    SAL_INFO("ucb.ucp.webdav.curl",
             "curl version: " << pVersion->version << " " << pVersion->host
                              << " features: " << ::std::hex << pVersion->features << " ssl: "
                              << pVersion->ssl_version << " libz: " << pVersion->libz_version);
    OString const useragent(OString::Concat("LibreOffice " LIBO_VERSION_DOTTED " curl/")
                            + ::std::string_view(pVersion->version, strlen(pVersion->version)) + " "
                            + pVersion->ssl_version);
    // looks like an explicit "User-Agent" header in CURLOPT_HTTPHEADER
    // will override CURLOPT_USERAGENT, see Curl_http_useragent(), so no need
    // to check anything here
    auto rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_USERAGENT, useragent.getStr());
    if (rc != CURLE_OK)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "CURLOPT_USERAGENT failed: " << GetErrorString(rc));
        throw DAVException(DAVException::DAV_SESSION_CREATE,
                           ConnectionEndPointString(m_URI.GetHost(), m_URI.GetPort()));
    }
    m_ErrorBuffer[0] = '\0';
    // this supposedly gives the highest quality error reporting
    rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_ERRORBUFFER, m_ErrorBuffer);
    assert(rc == CURLE_OK);
#if OSL_DEBUG_LEVEL > 0
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
    auto const connectTimeout(officecfg::Inet::Settings::ConnectTimeout::get(m_xContext));
    // default is 300s
    rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_CONNECTTIMEOUT,
                          ::std::max<long>(2L, ::std::min<long>(connectTimeout, 180L)));
    if (rc != CURLE_OK)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "CURLOPT_CONNECTTIMEOUT failed: " << GetErrorString(rc));
        throw DAVException(DAVException::DAV_SESSION_CREATE,
                           ConnectionEndPointString(m_URI.GetHost(), m_URI.GetPort()));
    }
#if 0
    auto const readTimeout(officecfg::Inet::Settings::ReadTimeout::get(m_xContext));
    // TODO: read timeout??? does not map to this value?
#endif
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
    // set this initially, may be overwritten during authentication
    assert(rc == CURLE_OK);
    rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_HTTPAUTH, CURLAUTH_ANY);
    assert(rc == CURLE_OK); // ANY is always available
    if (!m_Proxy.aName.isEmpty())
    {
        rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_PROXYPORT,
                              static_cast<decltype(0L)>(m_Proxy.nPort));
        assert(rc == CURLE_OK);
        OString const utf8Proxy(OUStringToOString(m_Proxy.aName, RTL_TEXTENCODING_UTF8));
        rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_PROXY, utf8Proxy.getStr());
        if (rc != CURLE_OK)
        {
            SAL_WARN("ucb.ucp.webdav.curl", "CURLOPT_PROXY failed: " << GetErrorString(rc));
            throw DAVException(DAVException::DAV_SESSION_CREATE,
                               ConnectionEndPointString(m_Proxy.aName, m_Proxy.nPort));
        }
        // set this initially, may be overwritten during authentication
        rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_PROXYAUTH, CURLAUTH_ANY);
        assert(rc == CURLE_OK); // ANY is always available
    }
}

CurlSession::~CurlSession() {}

auto CurlSession::CanUse(OUString const& rURI) -> bool
{
    try
    {
        CurlUri const uri(rURI);

        return m_URI.GetScheme() == uri.GetScheme() && m_URI.GetHost() == uri.GetHost()
               && m_URI.GetPort() == uri.GetPort();
    }
    catch (DAVException const&)
    {
        return false;
    }
}

auto CurlSession::UsesProxy() -> bool
{
    assert(m_URI.GetScheme() == "http" || m_URI.GetScheme() == "https");
    return !m_Proxy.aName.isEmpty();
}

auto CurlSession::abort() -> void
{
    // this is using synchronous libcurl apis - no way to abort?
    // might be possible with more complexity and CURLOPT_CONNECT_ONLY
    // or curl_multi API, but is it worth the complexity?
    // ... it looks like CURLOPT_CONNECT_ONLY would disable all HTTP handling.
    // abort() was a no-op since OOo 3.2 and before that it crashed.
}

/// this is just a bunch of static member functions called from CurlSession
struct CurlProcessor
{
    static auto ProcessRequest(
        CurlSession& rSession, ::std::u16string_view rURIReference,
        DAVRequestEnvironment const* pEnv,
        ::std::unique_ptr<curl_slist, deleter_from_fn<curl_slist_free_all>> pRequestHeaderList,
        uno::Reference<io::XOutputStream> const* pxOutStream,
        uno::Reference<io::XInputStream> const* pxInStream,
        ::std::pair<::std::vector<OUString> const&, DAVResource&> const* pRequestedHeaders) -> void;

    static auto
    PropFind(CurlSession& rSession, ::std::u16string_view rURIReference, Depth depth,
             ::std::tuple<::std::vector<OUString> const&, ::std::vector<DAVResource>* const,
                          ::std::vector<ucb::Lock>* const> const* o_pRequestedProperties,
             ::std::vector<DAVResourceInfo>* const o_pResourceInfos,
             DAVRequestEnvironment const& rEnv) -> void;

    static auto MoveOrCopy(CurlSession& rSession, ::std::u16string_view rSourceURIReference,
                           ::std::u16string_view rDestinationURI, DAVRequestEnvironment const& rEnv,
                           bool isOverwrite, char const* pMethod) -> void;

    static auto
    Lock(CurlSession& rSession, OUString const& rURIReference, DAVRequestEnvironment const* pEnv,
         ::std::unique_ptr<curl_slist, deleter_from_fn<curl_slist_free_all>> pRequestHeaderList,
         uno::Reference<io::XInputStream> const* pxInStream)
        -> ::std::vector<::std::pair<ucb::Lock, sal_Int32>>;

    static auto Unlock(CurlSession& rSession, OUString const& rURIReference,
                       DAVRequestEnvironment const* pEnv) -> void;
};

/// main function to initiate libcurl requests
auto CurlProcessor::ProcessRequest(
    CurlSession& rSession, ::std::u16string_view const rURIReference,
    DAVRequestEnvironment const* const pEnv,
    ::std::unique_ptr<curl_slist, deleter_from_fn<curl_slist_free_all>> pRequestHeaderList,
    uno::Reference<io::XOutputStream> const* const pxOutStream,
    uno::Reference<io::XInputStream> const* const pxInStream,
    ::std::pair<::std::vector<OUString> const&, DAVResource&> const* const pRequestedHeaders)
    -> void
{
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
    ::comphelper::ScopeGuard const g([&]() {
        auto rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_HEADERDATA, nullptr);
        assert(rc == CURLE_OK);
        (void)rc;
        if (pxOutStream)
        {
            rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_WRITEDATA, nullptr);
            assert(rc == CURLE_OK);
        }
        if (pxInStream)
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
        auto rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_HTTPHEADER,
                                   pRequestHeaderList.get());
        assert(rc == CURLE_OK);
        (void)rc;
    }

    ::std::unique_ptr<CURLU, deleter_from_fn<curl_url_cleanup>> const pUrl(
        rSession.m_URI.CloneCURLU());
    OString const utf8URIRef(OUStringToOString(rURIReference, RTL_TEXTENCODING_UTF8));
    auto uc = curl_url_set(pUrl.get(),
                           // very odd, but see DAVResourceAccess::getRequestURI() :-/
                           rSession.UsesProxy() ? CURLUPART_URL : CURLUPART_PATH,
                           utf8URIRef.getStr(), 0);
    if (uc != CURLUE_OK)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "curl_url_set failed: " << uc);
        throw DAVException(DAVException::DAV_INVALID_ARG);
    }

    auto rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_CURLU, pUrl.get());
    assert(rc == CURLE_OK); // can't fail since 7.63.0

    // authentication data may be in the URI, or requested via XInteractionHandler
    // also
    struct Auth
    {
        OUString UserName;
        OUString PassWord;
        decltype(CURLAUTH_ANY) AuthMask; ///< allowed auth methods
        Auth(OUString const& rUserName, OUString const& rPassword,
             decltype(CURLAUTH_ANY) const & rAuthMask)
            : UserName(rUserName)
            , PassWord(rPassword)
            , AuthMask(rAuthMask)
        {
        }
    };
    ::std::optional<Auth> oAuth;
    ::std::optional<Auth> oAuthProxy;
    if (pEnv && !rSession.m_isAuthenticatedProxy && !rSession.m_Proxy.aName.isEmpty())
    {
        // the hope is that this must be a URI
        CurlUri const uri(rSession.m_Proxy.aName);
        oAuthProxy.emplace(uri.GetUser(), uri.GetPassword(), CURLAUTH_ANY);
    }
    decltype(CURLAUTH_ANY) const authSystem(CURLAUTH_NEGOTIATE | CURLAUTH_NTLM | CURLAUTH_NTLM_WB);
    if (pRequestedHeaders || (pEnv && !rSession.m_isAuthenticated))
    {
    // m_aRequestURI *may* be a path or *may* be URI - wtf
    // TODO: why is there this m_aRequestURI and also rURIReference argument?
    // ... only caller is DAVResourceAccess - always identical except MOVE/COPY
    // which doesn't work if it's just a URI reference so let's just use
    // rURIReference via pUrl instead
#if 0
        CurlUri const uri(pEnv->m_aRequestURI);
#endif
        CurlUri const uri(*pUrl);
        // note: due to parsing bug pwd didn't work in previous webdav ucps
        if (pEnv && !rSession.m_isAuthenticated)
        {
            oAuth.emplace(uri.GetUser(), uri.GetPassword(), CURLAUTH_ANY);
        }
        if (pRequestedHeaders)
        {
            // note: Previously this would be the rURIReference directly but
            // that ends up in CurlUri anyway and curl is unhappy.
            // But it looks like all consumers of this .uri are interested
            // only in the path, so it shouldn't make a difference to give
            // the entire URI when the caller extracts the path anyway.
            pRequestedHeaders->second.uri = uri.GetURI();
            pRequestedHeaders->second.properties.clear();
        }
    }
    bool isRetry(false);

    // libcurl does not have an authentication callback so handle auth
    // related status codes and requesting credentials via this loop
    do
    {
        isRetry = false;

        if (oAuth)
        {
            assert(!rSession.m_isAuthenticated);
            OString const utf8UserName(OUStringToOString(oAuth->UserName, RTL_TEXTENCODING_UTF8));
            rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_USERNAME, utf8UserName.getStr());
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

        if (oAuthProxy)
        {
            assert(!rSession.m_isAuthenticatedProxy);
            OString const utf8UserName(
                OUStringToOString(oAuthProxy->UserName, RTL_TEXTENCODING_UTF8));
            rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_PROXYUSERNAME,
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
        rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_HEADERDATA, &headers);
        assert(rc == CURLE_OK);
        ::std::optional<DownloadTarget> oDownloadTarget;
        if (pxOutStream)
        {
            oDownloadTarget.emplace(*pxOutStream, headers);
            rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_WRITEDATA, &*oDownloadTarget);
            assert(rc == CURLE_OK);
        }
        ::std::optional<UploadSource> oUploadSource;
        if (pxInStream)
        {
            oUploadSource.emplace(*pxInStream, headers);
            rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_READDATA, &*oUploadSource);
            assert(rc == CURLE_OK);
            // libcurl won't upload without setting this
            rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_UPLOAD, 1L);
            assert(rc == CURLE_OK);
        }
        rSession.m_ErrorBuffer[0] = '\0';
        // this is where libcurl actually does something
        rc = curl_easy_perform(rSession.m_pCurl.get());
        // error handling part 1: libcurl errors
        if (rc != CURLE_OK)
        {
            // TODO: is there any value in extracting CURLINFO_OS_ERRNO
            SAL_WARN("ucb.ucp.webdav.curl",
                     "curl_easy_perform failed: " << GetErrorString(rc, rSession.m_ErrorBuffer));
            switch (rc)
            {
                case CURLE_COULDNT_RESOLVE_PROXY:
                    throw DAVException(
                        DAVException::DAV_HTTP_LOOKUP,
                        ConnectionEndPointString(rSession.m_Proxy.aName, rSession.m_Proxy.nPort));
                case CURLE_COULDNT_RESOLVE_HOST:
                    throw DAVException(DAVException::DAV_HTTP_LOOKUP,
                                       ConnectionEndPointString(rSession.m_URI.GetHost(),
                                                                rSession.m_URI.GetPort()));
                case CURLE_COULDNT_CONNECT:
                case CURLE_SSL_CONNECT_ERROR:
                case CURLE_SSL_CERTPROBLEM:
                case CURLE_SSL_CIPHER:
                case CURLE_PEER_FAILED_VERIFICATION:
                case CURLE_SSL_ISSUER_ERROR:
                case CURLE_SSL_PINNEDPUBKEYNOTMATCH:
                case CURLE_SSL_INVALIDCERTSTATUS:
                case CURLE_QUIC_CONNECT_ERROR:
                    throw DAVException(DAVException::DAV_HTTP_CONNECT,
                                       ConnectionEndPointString(rSession.m_URI.GetHost(),
                                                                rSession.m_URI.GetPort()));
                case CURLE_REMOTE_ACCESS_DENIED:
                case CURLE_LOGIN_DENIED:
                case CURLE_AUTH_ERROR:
                    throw DAVException(DAVException::DAV_HTTP_AUTH, // probably?
                                       ConnectionEndPointString(rSession.m_URI.GetHost(),
                                                                rSession.m_URI.GetPort()));
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
                    throw DAVException(DAVException::DAV_HTTP_FAILED,
                                       ConnectionEndPointString(rSession.m_URI.GetHost(),
                                                                rSession.m_URI.GetPort()));
                case CURLE_OPERATION_TIMEDOUT:
                    throw DAVException(DAVException::DAV_HTTP_TIMEOUT,
                                       ConnectionEndPointString(rSession.m_URI.GetHost(),
                                                                rSession.m_URI.GetPort()));
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
            ExtractRequestedHeaders(headers, pRequestedHeaders);
        }
        else
        {
            switch (statusCode)
            {
                case SC_NONE:
                    assert(false); // ??? should be error returned from perform?
                    break;
                case SC_UNAUTHORIZED:
                case SC_PROXY_AUTHENTICATION_REQUIRED:
                {
                    if (pEnv && pEnv->m_xAuthListener)
                    {
                        ::std::optional<OUString> const oRealm(ExtractRealm(
                            headers, statusCode == SC_UNAUTHORIZED ? "WWW-Authenticate"
                                                                   : "Proxy-Authenticate"));

                        ::std::optional<Auth>& roAuth(statusCode == SC_UNAUTHORIZED ? oAuth
                                                                                    : oAuthProxy);
                        OUString userName(roAuth ? roAuth->UserName : OUString());
                        OUString passWord(roAuth ? roAuth->PassWord : OUString());
                        long authAvail(0);
                        rc = curl_easy_getinfo(rSession.m_pCurl.get(),
                                               statusCode == SC_UNAUTHORIZED
                                                   ? CURLINFO_HTTPAUTH_AVAIL
                                                   : CURLINFO_PROXYAUTH_AVAIL,
                                               &authAvail);
                        assert(rc == CURLE_OK);
                        bool const isSystemCredSupported((authAvail & authSystem) != 0);

                        // ask user via XInteractionHandler
                        auto ret = pEnv->m_xAuthListener->authenticate(
                            oRealm ? *oRealm : "",
                            //statusCode == SC_UNAUTHORIZED ? uri.GetHost() : rSession.m_Proxy.aName,
                            statusCode == SC_UNAUTHORIZED ? rSession.m_URI.GetHost()
                                                          : rSession.m_Proxy.aName,
                            userName, passWord, isSystemCredSupported);

                        if (ret == 0)
                        {
                            roAuth.emplace(userName, passWord,
                                           authAvail
                                               & ((userName.isEmpty() && passWord.isEmpty())
                                                      ? authSystem
                                                      : ~authSystem));
                            isRetry = true;
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
                case SC_REQUEST_TIMEOUT:
                {
                    throw DAVException(DAVException::DAV_HTTP_TIMEOUT,
                                       ConnectionEndPointString(rSession.m_URI.GetHost(),
                                                                rSession.m_URI.GetPort()));
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
                        throw DAVException(DAVException::DAV_HTTP_REDIRECT,
                                           pRedirectURL
                                               ? OUString(pRedirectURL, strlen(pRedirectURL),
                                                          RTL_TEXTENCODING_UTF8)
                                               : OUString());
                    }
                    [[fallthrough]];
                }
                default:
                    throw DAVException(DAVException::DAV_HTTP_ERROR, "", statusCode);
            }
        }
    } while (isRetry);

    if (oAuth)
    {
        // assume this worked, leave auth data as stored in CURL
        rSession.m_isAuthenticated = true;
    }
    if (oAuthProxy)
    {
        // assume this worked, leave auth data as stored in CURL
        rSession.m_isAuthenticatedProxy = true;
    }

    if (pxOutStream)
    {
        (*pxOutStream)->closeOutput(); // signal EOF
    }
}

auto CurlProcessor::PropFind(
    CurlSession& rSession, ::std::u16string_view const rURIReference, Depth const nDepth,
    ::std::tuple<::std::vector<OUString> const&, ::std::vector<DAVResource>* const,
                 ::std::vector<ucb::Lock>* const> const* const o_pRequestedProperties,
    ::std::vector<DAVResourceInfo>* const o_pResourceInfos, DAVRequestEnvironment const& rEnv)
    -> void
{
    assert((o_pRequestedProperties != nullptr) != (o_pResourceInfos != nullptr));
    assert((o_pRequestedProperties == nullptr)
           || (::std::get<1>(*o_pRequestedProperties) != nullptr)
                  != (::std::get<2>(*o_pRequestedProperties) != nullptr));

    // TODO: either set CURLOPT_INFILESIZE_LARGE or chunked?
    ::std::unique_ptr<curl_slist, deleter_from_fn<curl_slist_free_all>> pList(
        curl_slist_append(nullptr, "Transfer-Encoding: chunked"));
    if (!pList)
    {
        throw uno::RuntimeException("curl_slist_append failed");
    }
    pList.reset(curl_slist_append(pList.release(), "Content-Type: application/xml"));
    if (!pList)
    {
        throw uno::RuntimeException("curl_slist_append failed");
    }
    OString depth;
    switch (nDepth)
    {
        case DAVZERO:
            depth = "Depth: 0";
            break;
        case DAVONE:
            depth = "Depth: 1";
            break;
        case DAVINFINITY:
            depth = "Depth: infinity";
            break;
        default:
            assert(false);
    }
    pList.reset(curl_slist_append(pList.release(), depth.getStr()));
    if (!pList)
    {
        throw uno::RuntimeException("curl_slist_append failed");
    }

    Guard g(rSession.m_Mutex, [&]() {
        auto rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_CUSTOMREQUEST, nullptr);
        assert(rc == CURLE_OK);
        (void)rc;
    });

    auto rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_CUSTOMREQUEST, "PROPFIND");
    if (rc != CURLE_OK)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "CURLOPT_CUSTOMREQUEST failed: " << GetErrorString(rc));
        throw DAVException(
            DAVException::DAV_SESSION_CREATE,
            ConnectionEndPointString(rSession.m_URI.GetHost(), rSession.m_URI.GetPort()));
    }

    uno::Reference<io::XInputStream> const xRequestInStream(io::Pipe::create(rSession.m_xContext));
    uno::Reference<io::XOutputStream> const xRequestOutStream(xRequestInStream, uno::UNO_QUERY);
    assert(xRequestInStream.is());
    assert(xRequestOutStream.is());
    uno::Reference<xml::sax::XWriter> const xWriter(xml::sax::Writer::create(rSession.m_xContext));
    xWriter->setOutputStream(xRequestOutStream);
    xWriter->startDocument();
    rtl::Reference<::comphelper::AttributeList> const pAttrList(new ::comphelper::AttributeList);
    pAttrList->AddAttribute("xmlns", "CDATA", "DAV:");
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
            for (OUString const& rName : ::std::get<0>(*o_pRequestedProperties))
            {
                xWriter->startElement("prop", nullptr);
                SerfPropName name;
                DAVProperties::createSerfPropName(rName, name);
                pAttrList->Clear();
                pAttrList->AddAttribute("xmlns", "CDATA", OUString::createFromAscii(name.nspace));
                xWriter->startElement(OUString::createFromAscii(name.name), pAttrList);
                xWriter->endElement(OUString::createFromAscii(name.name));
                xWriter->endElement("prop");
            }
        }
    }
    xWriter->endElement("propfind");
    xWriter->endDocument();
    xRequestOutStream->closeOutput();

    // stream for response
    uno::Reference<io::XInputStream> const xResponseInStream(io::Pipe::create(rSession.m_xContext));
    uno::Reference<io::XOutputStream> const xResponseOutStream(xResponseInStream, uno::UNO_QUERY);
    assert(xResponseInStream.is());
    assert(xResponseOutStream.is());

    CurlProcessor::ProcessRequest(rSession, rURIReference, &rEnv, ::std::move(pList),
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
    ::std::tuple<::std::vector<OUString> const&, ::std::vector<DAVResource>* const,
                 ::std::vector<ucb::Lock>* const> const args(rPropertyNames, &o_rResources,
                                                             nullptr);
    return CurlProcessor::PropFind(*this, rURIReference, depth, &args, nullptr, rEnv);
}

auto CurlSession::PROPFIND(OUString const& rURIReference, Depth const depth,
                           ::std::vector<DAVResourceInfo>& o_rResourceInfos,
                           DAVRequestEnvironment const& rEnv) -> void
{
    SAL_INFO("ucb.ucp.webdav.curl", "PROPFIND: " << rURIReference << " " << depth);
    return CurlProcessor::PropFind(*this, rURIReference, depth, nullptr, &o_rResourceInfos, rEnv);
}

auto CurlSession::PROPPATCH(OUString const& rURIReference,
                            ::std::vector<ProppatchValue> const& rValues,
                            DAVRequestEnvironment const& rEnv) -> void
{
    SAL_INFO("ucb.ucp.webdav.curl", "PROPPATCH: " << rURIReference);

    //FIXME why does toXML encode stuff which parser ignores
    //isUCBDeadProperty case not handled

    // TODO: either set CURLOPT_INFILESIZE_LARGE or chunked?
    ::std::unique_ptr<curl_slist, deleter_from_fn<curl_slist_free_all>> pList(
        curl_slist_append(nullptr, "Transfer-Encoding: chunked"));
    if (!pList)
    {
        throw uno::RuntimeException("curl_slist_append failed");
    }
    pList.reset(curl_slist_append(pList.release(), "Content-Type: application/xml"));
    if (!pList)
    {
        throw uno::RuntimeException("curl_slist_append failed");
    }

    Guard g(m_Mutex, [&]() {
        auto rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_CUSTOMREQUEST, nullptr);
        assert(rc == CURLE_OK);
        (void)rc;
    });

    auto rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_CUSTOMREQUEST, "PROPPATCH");
    if (rc != CURLE_OK)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "CURLOPT_CUSTOMREQUEST failed: " << GetErrorString(rc));
        throw DAVException(DAVException::DAV_SESSION_CREATE,
                           ConnectionEndPointString(m_URI.GetHost(), m_URI.GetPort()));
    }

    // generate XML document for PROPPATCH
    uno::Reference<io::XInputStream> const xRequestInStream(io::Pipe::create(m_xContext));
    uno::Reference<io::XOutputStream> const xRequestOutStream(xRequestInStream, uno::UNO_QUERY);
    assert(xRequestInStream.is());
    assert(xRequestOutStream.is());
    uno::Reference<xml::sax::XWriter> const xWriter(xml::sax::Writer::create(m_xContext));
    xWriter->setOutputStream(xRequestOutStream);
    xWriter->startDocument();
    rtl::Reference<::comphelper::AttributeList> const pAttrList(new ::comphelper::AttributeList);
    pAttrList->AddAttribute("xmlns", "CDATA", "DAV:");
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
        pAttrList->AddAttribute("xmlns", "CDATA", OUString::createFromAscii(name.nspace));
        xWriter->startElement(OUString::createFromAscii(name.name), pAttrList);
        if (rPropValue.operation == PROPSET)
        {
            if (DAVProperties::isUCBDeadProperty(name))
            {
                // TODO don't use UCBDeadPropertyValue::toXml, it's crazy
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
    xRequestOutStream->closeOutput();

    CurlProcessor::ProcessRequest(*this, rURIReference, &rEnv, ::std::move(pList), nullptr,
                                  &xRequestInStream, nullptr);
}

auto CurlSession::HEAD(OUString const& rURIReference, ::std::vector<OUString> const& rHeaderNames,
                       DAVResource& io_rResource, DAVRequestEnvironment const& rEnv) -> void
{
    SAL_INFO("ucb.ucp.webdav.curl", "HEAD: " << rURIReference);

    Guard g(m_Mutex, [&]() {
        auto rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_NOBODY, 0L);
        assert(rc == CURLE_OK);
        (void)rc;
    });
    auto rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_NOBODY, 1L);
    assert(rc == CURLE_OK);
    (void)rc;

    ::std::pair<::std::vector<OUString> const&, DAVResource&> const headers(rHeaderNames,
                                                                            io_rResource);

    CurlProcessor::ProcessRequest(*this, rURIReference, &rEnv, nullptr, nullptr, nullptr, &headers);
}

auto CurlSession::GET(OUString const& rURIReference, DAVRequestEnvironment const& rEnv)
    -> uno::Reference<io::XInputStream>
{
    SAL_INFO("ucb.ucp.webdav.curl", "GET: " << rURIReference);

    // could use either com.sun.star.io.Pipe or com.sun.star.io.SequenceInputStream?
    // Pipe can just write into its XOuputStream, which is simpler.
    // Both resize exponentially, so performance should be fine.
    // However, Pipe doesn't implement XSeekable, which is required by filters.

    uno::Reference<io::XSequenceOutputStream> const xSeqOutStream(
        io::SequenceOutputStream::create(m_xContext));
    uno::Reference<io::XOutputStream> const xResponseOutStream(xSeqOutStream);
    assert(xResponseOutStream.is());

    Guard g(m_Mutex, [&]() {
        auto rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_HTTPGET, 0L);
        assert(rc == CURLE_OK);
        (void)rc;
    });
    auto rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_HTTPGET, 1L);
    assert(rc == CURLE_OK);
    (void)rc;

    CurlProcessor::ProcessRequest(*this, rURIReference, &rEnv, nullptr, &xResponseOutStream,
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

    Guard g(m_Mutex, [&]() {
        auto rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_HTTPGET, 0L);
        assert(rc == CURLE_OK);
        (void)rc;
    });
    auto rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_HTTPGET, 1L);
    assert(rc == CURLE_OK);
    (void)rc;

    CurlProcessor::ProcessRequest(*this, rURIReference, &rEnv, nullptr, &rxOutStream, nullptr,
                                  nullptr);
}

auto CurlSession::GET(OUString const& rURIReference, ::std::vector<OUString> const& rHeaderNames,
                      DAVResource& io_rResource, DAVRequestEnvironment const& rEnv)
    -> uno::Reference<io::XInputStream>
{
    SAL_INFO("ucb.ucp.webdav.curl", "GET: " << rURIReference);

    Guard g(m_Mutex, [&]() {
        auto rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_HTTPGET, 0L);
        assert(rc == CURLE_OK);
        (void)rc;
    });
    auto rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_HTTPGET, 1L);
    assert(rc == CURLE_OK);
    (void)rc;

    uno::Reference<io::XSequenceOutputStream> const xSeqOutStream(
        io::SequenceOutputStream::create(m_xContext));
    uno::Reference<io::XOutputStream> const xResponseOutStream(xSeqOutStream);
    assert(xResponseOutStream.is());

    ::std::pair<::std::vector<OUString> const&, DAVResource&> const headers(rHeaderNames,
                                                                            io_rResource);

    CurlProcessor::ProcessRequest(*this, rURIReference, &rEnv, nullptr, &xResponseOutStream,
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

    Guard g(m_Mutex, [&]() {
        auto rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_HTTPGET, 0L);
        assert(rc == CURLE_OK);
        (void)rc;
    });
    auto rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_HTTPGET, 1L);
    assert(rc == CURLE_OK);
    (void)rc;

    ::std::pair<::std::vector<OUString> const&, DAVResource&> const headers(rHeaderNames,
                                                                            io_rResource);

    CurlProcessor::ProcessRequest(*this, rURIReference, &rEnv, nullptr, &rxOutStream, nullptr,
                                  &headers);
}

auto CurlSession::PUT(OUString const& rURIReference,
                      uno::Reference<io::XInputStream> const& rxInStream,
                      DAVRequestEnvironment const& rEnv) -> void
{
    SAL_INFO("ucb.ucp.webdav.curl", "PUT: " << rURIReference);

    // TODO: either set CURLOPT_INFILESIZE_LARGE or chunked?
    ::std::unique_ptr<curl_slist, deleter_from_fn<curl_slist_free_all>> pList(
        curl_slist_append(nullptr, "Transfer-Encoding: chunked"));
    if (!pList)
    {
        throw uno::RuntimeException("curl_slist_append failed");
    }
    // TODO: why is a *global* LockStore keyed by *path*?
    OUString const token(g_Init.LockStore.getLockToken(rURIReference));
    if (!token.isEmpty())
    {
        OString const utf8If("If: <" + OUStringToOString(rURIReference, RTL_TEXTENCODING_ASCII_US)
                             + "> (<" + OUStringToOString(token, RTL_TEXTENCODING_ASCII_US) + ">)");
        pList.reset(curl_slist_append(pList.release(), utf8If.getStr()));
        if (!pList)
        {
            throw uno::RuntimeException("curl_slist_append failed");
        }
    }

    // lock m_Mutex after accessing global LockStore to avoid deadlock
    Guard g(m_Mutex);

    CurlProcessor::ProcessRequest(*this, rURIReference, &rEnv, ::std::move(pList), nullptr,
                                  &rxInStream, nullptr);
}

auto CurlSession::POST(OUString const& rURIReference, OUString const& rContentType,
                       OUString const& rReferer, uno::Reference<io::XInputStream> const& rxInStream,
                       DAVRequestEnvironment const& rEnv) -> uno::Reference<io::XInputStream>
{
    SAL_INFO("ucb.ucp.webdav.curl", "POST: " << rURIReference);

    // TODO: either set CURLOPT_POSTFIELDSIZE_LARGE or chunked?
    ::std::unique_ptr<curl_slist, deleter_from_fn<curl_slist_free_all>> pList(
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

    Guard g(m_Mutex, [&]() {
        auto rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_POST, 0L);
        assert(rc == CURLE_OK);
        (void)rc;
    });

    auto rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_POST, 1L);
    assert(rc == CURLE_OK);
    (void)rc;

    uno::Reference<io::XSequenceOutputStream> const xSeqOutStream(
        io::SequenceOutputStream::create(m_xContext));
    uno::Reference<io::XOutputStream> const xResponseOutStream(xSeqOutStream);
    assert(xResponseOutStream.is());

    CurlProcessor::ProcessRequest(*this, rURIReference, &rEnv, ::std::move(pList),
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

    // TODO: either set CURLOPT_POSTFIELDSIZE_LARGE or chunked?
    ::std::unique_ptr<curl_slist, deleter_from_fn<curl_slist_free_all>> pList(
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

    Guard g(m_Mutex, [&]() {
        auto rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_POST, 0L);
        assert(rc == CURLE_OK);
        (void)rc;
    });

    auto rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_POST, 1L);
    assert(rc == CURLE_OK);
    (void)rc;

    CurlProcessor::ProcessRequest(*this, rURIReference, &rEnv, ::std::move(pList), &rxOutStream,
                                  &rxInStream, nullptr);
}

auto CurlSession::MKCOL(OUString const& rURIReference, DAVRequestEnvironment const& rEnv) -> void
{
    SAL_INFO("ucb.ucp.webdav.curl", "MKCOL: " << rURIReference);

    Guard g(m_Mutex, [&]() {
        auto rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_CUSTOMREQUEST, nullptr);
        assert(rc == CURLE_OK);
        (void)rc;
    });
    auto rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_CUSTOMREQUEST, "MKCOL");
    if (rc != CURLE_OK)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "CURLOPT_CUSTOMREQUEST failed: " << GetErrorString(rc));
        throw DAVException(DAVException::DAV_SESSION_CREATE,
                           ConnectionEndPointString(m_URI.GetHost(), m_URI.GetPort()));
    }

    CurlProcessor::ProcessRequest(*this, rURIReference, &rEnv, nullptr, nullptr, nullptr, nullptr);
}

auto CurlProcessor::MoveOrCopy(CurlSession& rSession,
                               ::std::u16string_view const rSourceURIReference,
                               ::std::u16string_view const rDestinationURI,
                               DAVRequestEnvironment const& rEnv, bool const isOverwrite,
                               char const* const pMethod) -> void
{
    OString const utf8Destination("Destination: "
                                  + OUStringToOString(rDestinationURI, RTL_TEXTENCODING_ASCII_US));
    ::std::unique_ptr<curl_slist, deleter_from_fn<curl_slist_free_all>> pList(
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

    Guard g(rSession.m_Mutex, [&]() {
        auto rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_CUSTOMREQUEST, nullptr);
        assert(rc == CURLE_OK);
        (void)rc;
    });

    auto rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_CUSTOMREQUEST, pMethod);
    assert(rc == CURLE_OK);
    (void)rc;

    CurlProcessor::ProcessRequest(rSession, rSourceURIReference, &rEnv, ::std::move(pList), nullptr,
                                  nullptr, nullptr);
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

    Guard g(m_Mutex, [&]() {
        auto rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_CUSTOMREQUEST, nullptr);
        assert(rc == CURLE_OK);
        (void)rc;
    });
    auto rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_CUSTOMREQUEST, "DELETE");
    if (rc != CURLE_OK)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "CURLOPT_CUSTOMREQUEST failed: " << GetErrorString(rc));
        throw DAVException(DAVException::DAV_SESSION_CREATE,
                           ConnectionEndPointString(m_URI.GetHost(), m_URI.GetPort()));
    }

    CurlProcessor::ProcessRequest(*this, rURIReference, &rEnv, nullptr, nullptr, nullptr, nullptr);
}

auto CurlProcessor::Lock(
    CurlSession& rSession, OUString const& rURIReference, DAVRequestEnvironment const* const pEnv,
    ::std::unique_ptr<curl_slist, deleter_from_fn<curl_slist_free_all>> pRequestHeaderList,
    uno::Reference<io::XInputStream> const* const pxRequestInStream)
    -> ::std::vector<::std::pair<ucb::Lock, sal_Int32>>
{
    Guard g(rSession.m_Mutex, [&]() {
        auto rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_CUSTOMREQUEST, nullptr);
        assert(rc == CURLE_OK);
        (void)rc;
    });
    auto rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_CUSTOMREQUEST, "LOCK");
    if (rc != CURLE_OK)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "CURLOPT_CUSTOMREQUEST failed: " << GetErrorString(rc));
        throw DAVException(
            DAVException::DAV_SESSION_CREATE,
            ConnectionEndPointString(rSession.m_URI.GetHost(), rSession.m_URI.GetPort()));
    }

    // stream for response
    uno::Reference<io::XInputStream> const xResponseInStream(io::Pipe::create(rSession.m_xContext));
    uno::Reference<io::XOutputStream> const xResponseOutStream(xResponseInStream, uno::UNO_QUERY);
    assert(xResponseInStream.is());
    assert(xResponseOutStream.is());

    TimeValue startTime;
    osl_getSystemTime(&startTime);

    CurlProcessor::ProcessRequest(rSession, rURIReference, pEnv, ::std::move(pRequestHeaderList),
                                  &xResponseOutStream, pxRequestInStream, nullptr);

    ::std::vector<ucb::Lock> const acquiredLocks(parseWebDAVLockResponse(xResponseInStream));
    SAL_WARN_IF(acquiredLocks.empty(), "ucb.ucp.webdav.curl",
                "could not get LOCK for " << rURIReference);

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
                         << rURIReference);
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

    // FIXME: why is a *global* LockStore keyed by *path*?
    if (g_Init.LockStore.getLockTokenForURI(rURIReference, rLock))
    {
        // already have a lock that covers the requirement
        // TODO: maybe use DAV:lockdiscovery to ensure it's valid
        return;
    }

    // note: no m_Mutex lock needed here, only in CurlProcessor::Lock()

    // generate XML document for acquiring new LOCK
    uno::Reference<io::XInputStream> const xRequestInStream(io::Pipe::create(m_xContext));
    uno::Reference<io::XOutputStream> const xRequestOutStream(xRequestInStream, uno::UNO_QUERY);
    assert(xRequestInStream.is());
    assert(xRequestOutStream.is());
    uno::Reference<xml::sax::XWriter> const xWriter(xml::sax::Writer::create(m_xContext));
    xWriter->setOutputStream(xRequestOutStream);
    xWriter->startDocument();
    rtl::Reference<::comphelper::AttributeList> const pAttrList(new ::comphelper::AttributeList);
    pAttrList->AddAttribute("xmlns", "CDATA", "DAV:");
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
    xRequestOutStream->closeOutput();

    // TODO: either set CURLOPT_INFILESIZE_LARGE or chunked?
    ::std::unique_ptr<curl_slist, deleter_from_fn<curl_slist_free_all>> pList(
        curl_slist_append(nullptr, "Transfer-Encoding: chunked"));
    if (!pList)
    {
        throw uno::RuntimeException("curl_slist_append failed");
    }
    pList.reset(curl_slist_append(pList.release(), "Content-Type: application/xml"));
    if (!pList)
    {
        throw uno::RuntimeException("curl_slist_append failed");
    }
    OString depth;
    switch (rLock.Depth)
    {
        case ucb::LockDepth_ZERO:
            depth = "Depth: 0";
            break;
        case ucb::LockDepth_ONE:
            depth = "Depth: 1";
            break;
        case ucb::LockDepth_INFINITY:
            depth = "Depth: infinity";
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
            timeout = "Timeout: Infinite";
            break;
        case 0:
            timeout = "Timeout: Second-180";
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
        = CurlProcessor::Lock(*this, rURIReference, &rEnv, ::std::move(pList), &xRequestInStream);

    for (auto const& rAcquiredLock : acquiredLocks)
    {
        g_Init.LockStore.addLock(rURIReference, rAcquiredLock.first,
                                 rAcquiredLock.first.LockTokens[0], this, rAcquiredLock.second);
        SAL_INFO("ucb.ucp.webdav.curl", "created LOCK for " << rURIReference);
    }
}

auto CurlProcessor::Unlock(CurlSession& rSession, OUString const& rURIReference,
                           DAVRequestEnvironment const* const pEnv) -> void
{
    // TODO: why is a *global* LockStore keyed by *path*?
    OUString const token(g_Init.LockStore.getLockToken(rURIReference));
    if (token.isEmpty())
    {
        SAL_WARN("ucb.ucp.webdav.curl", "attempt to unlock but not locked");
        throw DAVException(DAVException::DAV_NOT_LOCKED);
    }
    OString const utf8LockToken("Lock-Token: <"
                                + OUStringToOString(token, RTL_TEXTENCODING_ASCII_US) + ">");
    ::std::unique_ptr<curl_slist, deleter_from_fn<curl_slist_free_all>> pList(
        curl_slist_append(nullptr, utf8LockToken.getStr()));
    if (!pList)
    {
        throw uno::RuntimeException("curl_slist_append failed");
    }

    // lock m_Mutex after accessing global LockStore to avoid deadlock
    Guard g(rSession.m_Mutex, [&]() {
        auto rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_CUSTOMREQUEST, nullptr);
        assert(rc == CURLE_OK);
        (void)rc;
    });
    auto rc = curl_easy_setopt(rSession.m_pCurl.get(), CURLOPT_CUSTOMREQUEST, "UNLOCK");
    if (rc != CURLE_OK)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "CURLOPT_CUSTOMREQUEST failed: " << GetErrorString(rc));
        throw DAVException(
            DAVException::DAV_SESSION_CREATE,
            ConnectionEndPointString(rSession.m_URI.GetHost(), rSession.m_URI.GetPort()));
    }

    CurlProcessor::ProcessRequest(rSession, rURIReference, pEnv, ::std::move(pList), nullptr,
                                  nullptr, nullptr);
}

auto CurlSession::LOCK(OUString const& /*rURIReference*/, sal_Int64 const /*nTimeout*/,
                       DAVRequestEnvironment const & /*rEnv*/) -> sal_Int64
{
    // FIXME unused?
    assert(false);
    return 1234567890;
}

auto CurlSession::UNLOCK(OUString const& rURIReference, DAVRequestEnvironment const& rEnv) -> void
{
    SAL_INFO("ucb.ucp.webdav.curl", "UNLOCK: " << rURIReference);

    // note: no m_Mutex lock needed here, only in CurlProcessor::Unlock()

    CurlProcessor::Unlock(*this, rURIReference, &rEnv);

    g_Init.LockStore.removeLock(rURIReference);
}

auto CurlSession::NonInteractive_LOCK(OUString const& rURIReference,
                                      sal_Int32& o_rLastChanceToSendRefreshRequest) -> bool
{
    SAL_INFO("ucb.ucp.webdav.curl", "NonInteractive_LOCK: " << rURIReference);

    // note: no m_Mutex lock needed here, only in CurlProcessor::Lock()

    ::std::unique_ptr<curl_slist, deleter_from_fn<curl_slist_free_all>> pList(
        curl_slist_append(nullptr, "Timeout: Second-180"));

    // TODO: why is a *global* LockStore keyed by *path*?
    OUString const token(g_Init.LockStore.getLockToken(rURIReference));
    assert(!token.isEmpty()); // LockStore is the caller
    OString const utf8If("If: (<" + OUStringToOString(token, RTL_TEXTENCODING_ASCII_US) + ">)");
    pList.reset(curl_slist_append(pList.release(), utf8If.getStr()));
    if (!pList)
    {
        throw uno::RuntimeException("curl_slist_append failed");
    }

    try
    {
        auto const acquiredLocks
            = CurlProcessor::Lock(*this, rURIReference, nullptr, ::std::move(pList), nullptr);

        SAL_WARN_IF(1 < acquiredLocks.size(), "ucb.ucp.webdav.curl",
                    "multiple locks acquired on refresh for " << rURIReference);
        if (!acquiredLocks.empty())
        {
            o_rLastChanceToSendRefreshRequest = acquiredLocks.begin()->second;
        }
        SAL_INFO("ucb.ucp.webdav.curl", "NonInteractive_LOCK succeeded on " << rURIReference);
        return true;
    }
    catch (...)
    {
        SAL_INFO("ucb.ucp.webdav.curl", "NonInteractive_LOCK failed on " << rURIReference);
        return false;
    }
}

auto CurlSession::NonInteractive_UNLOCK(OUString const& rURIReference) -> void
{
    SAL_INFO("ucb.ucp.webdav.curl", "NonInteractive_UNLOCK: " << rURIReference);

    // note: no m_Mutex lock needed here, only in CurlProcessor::Unlock()

    try
    {
        CurlProcessor::Unlock(*this, rURIReference, nullptr);
        SAL_INFO("ucb.ucp.webdav.curl", "NonInteractive_UNLOCK succeeded on " << rURIReference);
        // TODO: why does webdav-serf not remove it from the lock store now???
    }
    catch (...)
    {
        SAL_INFO("ucb.ucp.webdav.curl", "NonInteractive_UNLOCK failed on " << rURIReference);
    }
}

} // namespace http_dav_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
