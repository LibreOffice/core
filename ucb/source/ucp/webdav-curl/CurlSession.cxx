/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "CurlSession.hxx"

#include <sal/log.hxx>
#include <rtl/strbuf.hxx>

#include <map>
#include <vector>

namespace http_dav_ucp
{
class Init
{
    Init()
    {
        if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK)
        {
            assert(!"curl_global_init failed");
        }
    }
    // do not call curl_global_cleanup() - this is not the only client of curl
};
static Init g_Init;

OString GetErrorString(CURLcode const rc, char const* const pErrorBuffer = nullptr)
{
    OStringBuffer buf;
    buf.append('(');
    buf.append(sal_Int32(rc));
    buf.append(')');
    buf.append(' ');
    char const* const pMessage( // static fallback
        (pErrorBuffer && pErrorBuffer[0] != '\0') ? pErrorBuffer : curl_easy_strerror(rc));
    buf.append(pMessage, strlen(pMessage));
    return buf.makeStringAndClear();
}

int debug_callback(CURL* handle, curl_infotype type, char* data, size_t size, void* userdata)
{
    char const pType(nullptr);
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

size_t write_callback(char* const ptr, size_t const size, size_t const nmemb, void* const userdata)
{
    //CurlSession *const pSession(static_cast<CurlSession*>(userdata));
    auto* const pTarget(static_cast<CurlSession::DownloadTarget*>(userdata));
    assert(pTarget);
    assert(size == 1); // says the man page
    assert(pTarget->xOutStream.is());
    try
    {
        uno::Sequence<sal_Int8> const data(ptr, nmemb);
        pTarget->xOutStream->writeBytes(data);
    }
    catch (...)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "exception in write_callback");
        return 0; // error
    }
    return nmemb;
}

size_t read_callback(char* const buffer, size_t const size, size_t const nitems,
                     void* const userdata)
{
    auto* const pSource(static_cast<CurlSession::UploadSource*>(userdata));
    assert(pSource);
    assert(pSource->xInStream.is());
    size_t const nBytes(size * nitems);
    size_t nRet(0);
    try
    {
        uno::Sequence<sal_Int8> data;
        data.realloc(nBytes);
        nRet = pSource->readSomeBytes(data, nBytes);
        ::std::memcpy(buffer, data.getConstArray(), nRet);
    }
    catch (...)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "exception in read_callback");
        return CURL_READFUNC_ABORT; // error
    }
    return nRet;
}

size_t header_callback(char* const buffer, size_t const size, size_t const nitems,
                       void* const userdata)
{
    auto* const pHeaders(static_cast<CurlSession::Headers*>(userdata));
    //assert(pHeaders);
    if (!pHeaders) // TODO maybe not needed in every request? not sure
    {
        return nitems;
    }
    assert(size == 1); // says the man page
    try
    {
#if 0
        if ("HTTP")
        {
        }
        else
            // start
#endif
        if (nitems == 0)
        {
            // end of header, body follows...
            if (pHeaders.empty())
            {
                SAL_WARN("ucb.ucp.webdav.curl", "header_callback: empty header?");
                return 0; // error
            }
            pHeaders->back().second = true;
        }
        else if ([0] == ' ' || [0] == '\t') // folded header field?
        {
            do
            {
                ++idx;
            } while (idx == ' ' || idx == '\t');
            if (pHeaders.empty() || pHeaders.back().second || pHeaders.back().first.empty())
            {
                SAL_WARN("ucb.ucp.webdav.curl",
                         "header_callback: folded header field without start");
                return 0; // error
            }
            pHeaders.back().first.back().append(' ');
            pHeaders.back().first.back().append(idx...);
        }
        else
        {
            if (pHeaders.empty() || pHeaders.back().second)
            {
                pHeaders.emplace_back({}, false);
            }
            pHeaders.back().first.emplace_back(OString(buffer, nitems));
        }
    }
    catch (...)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "exception in header_callback");
        return 0; // error
    }
    return nitems;
}

::std::map<OString, OString> ProcessHeaders(::std::vector<OString> const& rHeaders)
{
    ::std::map<OString, OString> ret;
    for (OString const& rLine : rHeaders)
    {
        auto const nColon(rLine.indexOf(':'));
        if (nColon == -1)
        {
            SAL_WARN("ucb.ucp.webdav.curl", "invalid header field (no :)");
            continue;
        }
        if (nColon == 0)
        {
            SAL_WARN("ucb.ucp.webdav.curl", "invalid header field (empty name)");
            continue;
        }
        auto const name(rLine.copy(0, nColon).toAsciiLowerCase()); // case insensitive
        sal_Int32 nStart(nColon + 1);
        while (nStart < rLine.getLength() && rLine[nStart] == ' ' && rLine[nStart] == '\t')
        {
            ++nStart;
        }
        sal_Int32 nEnd(rLine.getLength() - 1);
        while (nStart < nEnd && rLine[nEnd] == ' ' && rLine[nEnd] == '\t')
        {
            --nEnd;
        }
        auto const value(rLine.copy(nStart, nEnd - nStart));
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
        SAL_WARN("ucb.ucp.webdav.curl", "curl_easy_init failed: " << GetErrorString(rc).getStr());
        throw DAVException(DAVException::DAV_SESSION_CREATE,
                           ConnectionEndPointString(m_URI.GetHost(), m_URI.GetPort()));
    }
    curl_version_info_data const* const pVersion(curl_version_info(CURLVERSION_NOW));
    assert(pVersion);
    SAL_INFO("ucb.ucp.webdav.curl",
             "curl version: " << pVersion->version << " " << pVersion->host << " features: "
                              << std::hex << pVersion->features << " ssl: " << pVersion->ssl_version
                              << " libz: " << pVersion->libz_version);
    OString const useragent("LibreOffice " LIBO_VERSION_DOTTED " curl/" + pVersion->version + " "
                            + pVersion->ssl_version) auto rc
        = curl_easy_setopt(m_pCurl.get(), CURLOPT_USERAGENT, useragent.getStr());
    if (rc != CURLE_OK)
    {
        SAL_WARN("ucb.ucp.webdav.curl",
                 "CURLOPT_USERAGENT failed: " << GetErrorString(rc).getStr());
        throw DAVException(DAVException::DAV_SESSION_CREATE,
                           ConnectionEndPointString(m_URI.GetHost(), m_URI.GetPort()));
    }
    m_ErrorBuffer[0] = '\0';
    // this supposedly gives the highest quality error reporting
    auto rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_ERRORBUFFER, m_ErrorBuffer);
    assert(rc == CURLE_OK);
    // just for debugging...
    rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_DEBUGFUNCTION, debug_callback);
    assert(rc == CURLE_OK);
    rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_VERBOSE, 1L);
    assert(rc == CURLE_OK);
    // accept any encoding supported by libcurl
    rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_ACCEPT_ENCODING, "");
    if (rc != CURLE_OK)
    {
        SAL_WARN("ucb.ucp.webdav.curl",
                 "CURLOPT_ACCEPT_ENCODING failed: " << GetErrorString(rc).getStr());
        throw DAVException(DAVException::DAV_SESSION_CREATE,
                           ConnectionEndPointString(m_URI.GetHost(), m_URI.GetPort()));
    }
    // default is infinite
    rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_TIMEOUT, 60L);
    if (rc != CURLE_OK)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "CURLOPT_TIMEOUT failed: " << GetErrorString(rc).getStr());
        throw DAVException(DAVException::DAV_SESSION_CREATE,
                           ConnectionEndPointString(m_URI.GetHost(), m_URI.GetPort()));
    }
    rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_WRITEFUNCTION, &write_callback);
    assert(rc == CURLE_OK);
#if 0
    rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_WRITEDATA, &m_DownloadTarget);
    assert(rc == CURLE_OK);
#endif
    rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_READFUNCTION, &read_callback);
    assert(rc == CURLE_OK);
#if 0
    rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_READDATA, &m_UploadSource);
    assert(rc == CURLE_OK);
#endif
    rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_HEADERFUNCTION, &header_callback);
#if 1
    // set this initially, may be overwritten during authentication
    assert(rc == CURLE_OK);
    rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_HTTPAUTH, CURLAUTH_ANY);
#endif
    assert(rc == CURLE_OK); // ANY is always available
    if (!m_Proxy.aName.isEmpty())
    {
        rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_PROXYPORT, static_cast<long>(m_Proxy.nPort));
        assert(rc == CURLE_OK);
        OString const utf8Proxy(::rtl::OUStringToOString(m_Proxy.aName, RTL_TEXTENCODING_UTF8));
        rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_PROXY, );
        if (rc != CURLE_OK)
        {
            SAL_WARN("ucb.ucp.webdav.curl",
                     "CURLOPT_PROXY failed: " << GetErrorString(rc).getStr());
            throw DAVException(DAVException::DAV_SESSION_CREATE,
                               ConnectionEndPointString(m_Proxy.aName, m_Proxy.nPort));
        }
#if 1
        // set this initially, may be overwritten during authentication
        rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_PROXYAUTH, CURLAUTH_ANY);
        assert(rc == CURLE_OK); // ANY is always available
#endif
    }

//TODO:
//curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
#if 0
struct curl_slist *list = NULL;
list = curl_slist_append(list, "Name: Mr Smith");
curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
curl_easy_perform(curl);
curl_slist_free_all(list); /* free the list again */
#endif
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
    // ??? could use a pipe and select
    // Win32 code uses SOCKET ...
    // could use curl_multi_wakeup?
}

// DAV methods
auto CurlSession::PROPFIND(OUString const& rInPath, Depth depth,
                           ::std::vector<OUString> const& rPropertyNames,
                           ::std::vector<DAVResource>& o_rResources,
                           DAVRequestEnvironment const& rEnv) -> void
{
}

auto CurlSession::PROPFIND(OUString const& rInPath, Depth depth,
                           ::std::vector<DAVResourceInfo>& o_rResourceInfos,
                           DAVRequestEnvironment const& rEnv) -> void
{
}

auto CurlSession::PROPPATCH(OUString const& rInPath, ::std::vector<ProppatchValue> const& rValues,
                            DAVRequestEnvironment const& rEnv) -> void
{
}

auto CurlSession::HEAD(OUString const& rInPath, ::std::vector<OUString> const& rHeaderNames,
                       DAVResource& io_rResource, DAVRequestEnvironment const& rEnv) -> void
{
}

auto CurlSession::GET(OUString const& rInPath, DAVRequestEnvironment const& rEnv)
    -> uno::Reference<io::XInputStream>
{
    ::std::scoped_lock const g(m_Mutex);

    m_aEnv ? ? ?

    // could use either com.sun.star.io.Pipe or com.sun.star.io.SequenceInputStream?
    // Pipe can just write into its XOuputStream, which is simpler
    // also it resizes exponentially, so performance should be fine
    // only drawback is no XSeekable, not sure if anybody needs it

#if 0
    assert(m_DownloadTarget.buffer.isEmpty());
#endif
#if 0
    assert(!m_DownloadTarget.xOutStream.is());
    m_DownloadTarget.xOutStream.set(io::Pipe::create(m_xContext));
    ::comphelper::ScopeGuard const gg([this](){ m_DownloadTarget.xOutStream.clear(); });
#endif
               DownloadTarget downloadTarget(io::Pipe::create(m_xContext));
    ::comphelper::ScopeGuard const gg([this]() {
        auto rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_WRITEDATA, nullptr);
        assert(rc == CURLE_OK);
    });
    ::comphelper::ScopeGuard const gg([this]() {
        auto rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_HEADERDATA, nullptr);
        assert(rc == CURLE_OK);
    });

//CURLOPT_HEADERDATA
#if 0
    CurlURi uri(m_URI);
    if (uri.GetPath() != rInPath)
    {
        uri.SetPath(rInPath);
    }
#endif
    ::std::unique_ptr<CURLU, deleter_from_fn<curl_url_cleanup>> const pUrl(
        curl_url_dup(m_URI.GetCURLU()));
    OString const utf8Path(::rtl::OUStringToOString(rInPath, RTL_TEXTENCODING_UTF8));
    auto uc = curl_url_set(m_pUrl.get(), CURLUPART_PATH, utf8Path.getStr(), 0);
    if (uc != CURLUE_OK)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "curl_url_set failed: " << uc);
        throw DAVException(DAVException::DAV_INVALID_ARG);
    }

    auto rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_CURLU, pUrl.get());
    assert(rc == CURLE_OK); // can't fail since 7.63.0
#if 0
    ::std::vector<::std::pair<::std::vector<OString>, bool>> headers;
    rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_HEADERDATA, &headers);
    assert(rc == CURLE_OK);
#endif
    rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_HTTPGET, 1L);
    assert(rc == CURLE_OK);

    struct Auth
    {
        OUString UserName;
        OUString PassWord;
        long Auth;
    };
    ::std::optional<Auth> oAuth;
    ::std::optional<Auth> oAuthProxy;
#if 0
    std::optional<OUString> oUserNameProxy;
    std::optional<OUString> oPassWordProxy;
#endif
    if (!m_isAuthenticatedProxy && !m_Proxy.aName.isEmpty())
    {
        CurlUri const uri(m_Proxy.aName);
#if 0
        oUserNameProxy = uri.GetUser();
        oPassWordProxy = uri.GetPassword();
#endif
        oAuthProxy.emplace(uri.GetUser(), uri.GetPassword(), CURLAUTH_ANY);
    }
    // TODO: why is there this m_aRequestURI and also rInPath argument?
    CurlUri const uri(rEnv.m_aRequestURI);
#if 0
    std::optional<OUString> oUserName;
    std::optional<OUString> oPassWord;
    long auth(CURLAUTH_ANY);
    long authProxy(CURLAUTH_ANY);
#endif
    long const authSystem(CURLAUTH_NEGOTIATE | CURLAUTH_NTLM | CURLAUTH_NTLM_WB);
    if (!m_isAuthenticated)
    {
#if 0
        oUserName = uri.GetUser();
        // note: due to parsing bug pwd didn't work in previous webdav ucps
        oPassWord = uri.GetPassword();
#endif
        oAuth.emplace(uri.GetUser(), uri.GetPassword(), CURLAUTH_ANY);
    }
    bool isRetry(false);

    do
    {
        isRetry = false;

        //if (oUserName && oPassWord)
        if (oAuth)
        {
            assert(!m_isAuthenticated);
            //OString const utf8UserName(::rtl::OUStringToOString(*oUserName, RTL_TEXTENCODING_UTF8));
            OString const utf8UserName(
                ::rtl::OUStringToOString(*oAuth.UserName, RTL_TEXTENCODING_UTF8));
            rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_USERNAME, utf8UserName.getStr());
            if (rc != CURLE_OK)
            {
                SAL_WARN("ucb.ucp.webdav.curl",
                         "CURLOPT_USERNAME failed: " << GetErrorString(rc).getStr());
                throw DAVException(DAVException::DAV_INVALID_ARG);
            }
            //OString const utf8UserName(::rtl::OUStringToOString(*oPassWord, RTL_TEXTENCODING_UTF8));
            OString const utf8UserName(
                ::rtl::OUStringToOString(*oAuth.PassWord, RTL_TEXTENCODING_UTF8));
            rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_PASSWORD, utf8PassWord.getStr());
            if (rc != CURLE_OK)
            {
                SAL_WARN("ucb.ucp.webdav.curl",
                         "CURLOPT_PASSWORD failed: " << GetErrorString(rc).getStr());
                throw DAVException(DAVException::DAV_INVALID_ARG);
            }
            //rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_HTTPAUTH, auth);
            rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_HTTPAUTH, oAuth.Auth);
            assert(
                rc
                == CURLE_OK); // it shouldn't be possible to reduce auth to 0 via the authSystem masks
        }

        //if (oUserNameProxy && oPassWordProxy)
        if (oAuthProxy)
        {
            assert(!m_isAuthenticatedProxy);
            //OString const utf8UserName(::rtl::OUStringToOString(*oUserNameProxy, RTL_TEXTENCODING_UTF8));
            OString const utf8UserName(
                ::rtl::OUStringToOString(*oAuthProxy.UserName, RTL_TEXTENCODING_UTF8));
            rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_PROXYUSERNAME, utf8UserName.getStr());
            if (rc != CURLE_OK)
            {
                SAL_WARN("ucb.ucp.webdav.curl",
                         "CURLOPT_PROXYUSERNAME failed: " << GetErrorString(rc).getStr());
                throw DAVException(DAVException::DAV_INVALID_ARG);
            }
            //OString const utf8UserName(::rtl::OUStringToOString(*oPassWordProxy, RTL_TEXTENCODING_UTF8));
            OString const utf8UserName(
                ::rtl::OUStringToOString(*oAuthProxy.PassWord, RTL_TEXTENCODING_UTF8));
            rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_PROXYPASSWORD, utf8PassWord.getStr());
            if (rc != CURLE_OK)
            {
                SAL_WARN("ucb.ucp.webdav.curl",
                         "CURLOPT_PROXYPASSWORD failed: " << GetErrorString(rc).getStr());
                throw DAVException(DAVException::DAV_INVALID_ARG);
            }
            rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_PROXYAUTH, oAuthProxy.Auth);
            assert(
                rc
                == CURLE_OK); // it shouldn't be possible to reduce auth to 0 via the authSystem masks
        }

        rc = curl_easy_setopt(m_pCurl.get(), CURLOPT_WRITEDATA, &downloadTarget);
        assert(rc == CURLE_OK);
        m_ErrorBuffer[0] = '\0';
        auto rc = curl_easy_perform(m_pCurl.get());
        if (rc != CURLE_OK)
        {
            // TODO: is there any value in extracting CURLINFO_OS_ERRNO
            SAL_WARN("ucb.ucp.webdav.curl",
                     "curl_easy_perform failed: " << GetErrorString(rc, m_ErrorBuffer).getStr());
            switch (rc)
            {
                case CURLE_COULDNT_RESOLVE_PROXY:
                throw DAVException(DAVException::DAV_HTTP_LOOKUP,
                    ConnectionEndPointString(m_Proxy.aName, m_Proxy.nPort);
            case CURLE_COULDNT_RESOLVE_HOST:
                throw DAVException(DAVException::DAV_HTTP_LOOKUP,
                    ConnectionEndPointString(m_URI.GetHost(), m_URI.GetPort());
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
                    ConnectionEndPointString(m_URI.GetHost(), m_URI.GetPort());
            case CURLE_REMOTE_ACCESS_DENIED:
            case CURLE_LOGIN_DENIED:
            case CURLE_AUTH_ERROR:
                throw DAVException(DAVException::DAV_AUTH, // probably?
                    ConnectionEndPointString(m_URI.GetHost(), m_URI.GetPort());
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
                    ConnectionEndPointString(m_URI.GetHost(), m_URI.GetPort());
            case CURLE_OPERATION_TIMEDOUT:
                throw DAVException(DAVException::DAV_HTTP_TIMEOUT,
                    ConnectionEndPointString(m_URI.GetHost(), m_URI.GetPort());
            default: // lots of generic errors
                throw DAVException(DAVException::DAV_HTTP_ERROR, "", 0);
            }
        }
        long statusCode(SC_NONE);
        rc = curl_easy_getinfo(m_pCurl.get(), CURLINFO_RESPONSE_CODE, &statusCode);
        assert(rc == CURLE_OK);
        assert(statusCode != SC_NONE); // ??? should be error returned from perform?
        if (300 <= statusCode)
        {
            switch (statusCode)
            {
                case SC_NONE:
                    assert(false); // ??? should be error returned from perform?
                    break;
                case SC_UNAUTHORIZED:
                case SC_PROXY_AUTHENTICATION_REQUIRED:
                    if (rEnv.m_xAuthListener)
                    {
#if 0
                CurlUri const uri(rEnv.m_aRequestURI);
                OUString userName(uri.GetUser());
                OUString passWord(uri.GetPassword());
#endif
#if 0
                OUString userName(statusCode == SC_UNAUTHORIZED ? oUserName ? *oUserName : OUString() : oUserNameProxy ? *oUserNameProxy : OUString() );
                OUString passWord(statusCode == SC_UNAUTHORIZED ? oPassWord ? *oPassWord : OUString() : oPassWordProxy ? *oPassWordProxy : OUString() );
#endif
                        ::std::optional<Auth>& rAuth(statusCode == SC_UNAUTHORIZED ? oAuth
                                                                                   : oAuthProxy);
                        OUString userName(rAuth ? rAuth->UserName : OUString());
                        OUString passWord(rAuth ? rAuth->PassWord : OUString());
                        long authAvail(0);
                        rc = curl_easy_getinfo(m_pCurl.get(),
                                               statusCode == SC_UNAUTHORIZED
                                                   ? CURLINFO_HTTPAUTH_AVAIL
                                                   : CURLINFO_PROXYAUTH_AVAIL,
                                               &authAvail);
                        assert(rc == CURLE_OK);
                        bool const isSystemCredSupported((authAvail & authSystem) != 0);

                        auto ret = rEnv.m_xAuthListener->authenticate(
                            "", // TODO Realm ???
                            statuscode == SC_UNAUTHORIZED ? uri.GetHost() : m_Proxy.aName, userName,
                            passWord, isSystemCredSupported);

                        if (ret == 0)
                        {
                            rAuth.emplace(userName, passWord,
                                          authAvail
                                              & ((userName.isEmpty() && passWord.isEmpty())
                                                     ? authSystem
                                                     : ~authSystem));
                            isRetry = true;
#if 0
                    if (statusCode == SC_UNAUTHORIZED)
                    {
                        oUserName = userName;
                        oPassWord = passWord;
                        // why does it set these to empty
                        auth = authAvail &
                            ((userName.isEmpty() && passWord.isEmpty())
                                ? authSystem : ~authSystem);
                        isRetry = true;
                    }
                    else
                    {
                        oUserNameProxy = userName;
                        oPassWordProxy = passWord;
                        authProxy = authAvail &
                            ((userName.isEmpty() && passWord.isEmpty())
                                ? authSystem : ~authSystem);
                        isRetry = true;
                    }
#endif
                        }
                        else
                        {
                    throw DAVException(DAVException::DAV_NOAUTH,
                        ConnectionEndPointString(m_URI.GetHost(), m_URI.GetPort());
                        }

#if 0
                // not m_aRequestURI?
                CurlUri const uri(m_Proxy.aName);
                rc = curl_easy_getinfo(m_pCurl.get(), CURLINFO_PROXYAUTH_AVAIL, &);
                assert(rc == CURLE_OK);
#endif
                    }
                    break;
                case SC_MOVED_PERMANENTLY:
                case SC_MOVED_TEMPORARILY:
                case SC_SEE_OTHER:
                case SC_TEMPORARY_REDIRECT:
                {
                    // could also use CURLOPT_FOLLOWLOCATION but apparently the
                    // upper layer wants to know about redirects?
                    char* pRedirectURL(nullptr);
                    rc = curl_easy_getinfo(m_pCurl.get(), CURLINFO_REDIRECT_URL, &pRedirectURL);
                    assert(rc == CURLE_OK);
                    if (pRedirectURL)
                    {
                        throw DAVException(DAVException::DAV_HTTP_REDIRECT,
                                           pRedirectURL
                                               ? OUString(pRedirectURL, strlen(pRedirectURL))
                                               : OUString());
                    }
                }
                default:
                    throw DAVException(DAVException::DAV_HTTP_ERROR, "", statusCode);
            }
        }
    } while (isRetry);

    if (oUserName && oPassWord)
    {
        // assume this worked, leave auth data as stored in CURL
        m_isAuthenticated = true;
    }
    if (oUserNameProxy && oPassWordProxy)
    {
        // assume this worked, leave auth data as stored in CURL
        m_isAuthenticatedProxy = true;
    }

#if 0
    auto const xStream(io::SequenceInputStream::createStreamFromSequence(m_DownloadTarget.buffer));
    assert(xStream.is());
    m_DownloadTarget.buffer.clear();
    return xStream;
#endif
    return downloadTarget.xOutStream;
}

auto CurlSession::GET(OUString const& rInPath, uno::Reference<io::XOutputStream>& rOutStream,
                      DAVRequestEnvironment const& rEnv) -> void
{
}

auto CurlSession::GET(OUString const& rInPath, ::std::vector<OUString> const& rHeaderNames,
                      DAVResource& io_rResource, DAVRequestEnvironment const& rEnv)
    -> uno::Reference<io::XInputStream>
{
}

auto CurlSession::GET(OUString const& rInPath, uno::Reference<io::XOutputStream>& rOutStream,
                      ::std::vector<OUString> const& rHeaderNames, DAVResource& io_rResource,
                      DAVRequestEnvironment const& rEnv) -> void
{
}

auto CurlSession::PUT(OUString const& rInPath, uno::Reference<io::XInputStream> const& rInStream,
                      DAVRequestEnvironment const& rEnv) -> void
{
}

auto CurlSession::POST(OUString const& rInPath, OUString const& rContentType,
                       OUString const& rReferer, uno::Reference<io::XInputStream> const& rInStream,
                       DAVRequestEnvironment const& rEnv) -> uno::Reference<io::XInputStream>
{
}

auto CurlSession::POST(OUString const& rInPath, OUString const& rContentType,
                       OUString const& rReferer, uno::Reference<io::XInputStream> const& rInStream,
                       uno::Reference<io::XOutputStream>& rOutStream,
                       DAVRequestEnvironment const& rEnv) -> void
{
}

auto CurlSession::MKCOL(OUString const& rInPath, DAVRequestEnvironment const& rEnv) -> void {}

auto CurlSession::COPY(OUString const& rInPathSource, OUString const& rInPathDestination,
                       DAVRequestEnvironment const& rEnv, bool const isOverwrite) -> void
{
}

auto CurlSession::MOVE(OUString const& rInPathSource, OUString const& rInPathDestination,
                       DAVRequestEnvironment const& rEnv, bool const isOverwrite) -> void
{
}

auto CurlSession::DESTROY(OUString const& rInPath, DAVRequestEnvironment const& rEnv) -> void {}

auto CurlSession::LOCK(OUString const& rInPath, ucb::Lock /*const*/& rLock,
                       DAVRequestEnvironment const& rEnv) -> void
{
}

auto CurlSession::LOCK(OUString const& rInPath, sal_Int64 const nTimeout,
                       DAVRequestEnvironment const& rEnv) -> sal_Int64
{
}

auto CurlSession::UNLOCK(OUString const& rInPath, DAVRequestEnvironment const& rEnv) -> void {}

} // namespace http_dav_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
