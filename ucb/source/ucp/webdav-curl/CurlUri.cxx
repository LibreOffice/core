/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include "CurlUri.hxx"

#include <sal/log.hxx>
#include <rtl/uri.hxx>
#include <rtl/ustrbuf.hxx>

#include <optional>

namespace http_dav_ucp
{
const auto DEFAULT_HTTP_PORT = 80;
const auto DEFAULT_HTTPS_PORT = 443;

static ::std::optional<OUString> GetURLComponent(CURLU& rURI, CURLUPart const what,
                                                 CURLUcode const expected,
                                                 unsigned int const flags = 0)
{
    char* pPart(nullptr);
    auto uc = curl_url_get(&rURI, what, &pPart, flags);
    if (expected != CURLUE_OK && uc == expected)
    {
        return ::std::optional<OUString>();
    }
    if (uc != CURLUE_OK)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "curl_url_get failed: " << what << " " << uc);
        throw DAVException(DAVException::DAV_INVALID_ARG);
    }
    assert(pPart);
    CurlUniquePtr<char> pPart2(pPart);
    return ::rtl::OStringToOUString(pPart, RTL_TEXTENCODING_UTF8);
}

void CurlUri::Init()
{
    // looks like the result should be the same as the old calculateURI()
    auto const oURI(GetURLComponent(*m_pUrl, CURLUPART_URL, CURLUE_OK, CURLU_NO_DEFAULT_PORT));
    assert(oURI);
    m_URI = *oURI;

    auto const oScheme(GetURLComponent(*m_pUrl, CURLUPART_SCHEME, CURLUE_NO_SCHEME));
    if (oScheme)
    {
        m_Scheme = *oScheme;
    }
    auto const oUser(GetURLComponent(*m_pUrl, CURLUPART_USER, CURLUE_NO_USER));
    if (oUser)
    {
        m_User = *oUser;
    }
    auto const oPassWord(GetURLComponent(*m_pUrl, CURLUPART_PASSWORD, CURLUE_NO_PASSWORD));
    if (oPassWord)
    {
        m_Password = *oPassWord;
    }
    auto const oHost(GetURLComponent(*m_pUrl, CURLUPART_HOST, CURLUE_NO_HOST));
    if (oHost)
    {
        m_Host = *oHost;
    }
    // DAV schemes always have port but Content::transfer() is called with
    // arbitrary URLs so use CURLUE_NO_PORT
    auto const oPort(GetURLComponent(*m_pUrl, CURLUPART_PORT, CURLUE_NO_PORT, CURLU_DEFAULT_PORT));
    if (oPort)
    {
        m_nPort = oPort->toInt32();
    }

    auto const oPath(GetURLComponent(*m_pUrl, CURLUPART_PATH, CURLUE_OK));
    assert(oPath);
    m_Path = *oPath;

    // note: this used to be added to m_Path because before 2007, ne_uri path contained query/fragment as well :-/
    auto const oQuery(GetURLComponent(*m_pUrl, CURLUPART_QUERY, CURLUE_NO_QUERY));
    if (oQuery)
    {
        m_QueryAndFragment += "?" + *oQuery;
    }
    auto const oFragment(GetURLComponent(*m_pUrl, CURLUPART_FRAGMENT, CURLUE_NO_FRAGMENT));
    if (oFragment)
    {
        m_QueryAndFragment += "#" + *oFragment;
    }
}

CurlUri::CurlUri(::std::u16string_view const rURI)
{
    // note: in the old implementation, the rURI would be URI-encoded again
    // here, apparently because it could actually be an IRI (RFC 3987) and
    // neon didn't support that - not clear if this is a good idea

    m_pUrl.reset(curl_url());
    if (!m_pUrl)
    {
        throw ::std::bad_alloc();
    }

    // use curl to parse the URI, to get a consistent interpretation
    if (rURI.find(u'\0') != std::u16string_view::npos)
    {
        throw DAVException(DAVException::DAV_INVALID_ARG);
    }
    OString const utf8URI(OUStringToOString(rURI, RTL_TEXTENCODING_UTF8));
    auto uc = curl_url_set(m_pUrl.get(), CURLUPART_URL, utf8URI.getStr(), 0);
    if (uc != CURLUE_OK)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "curl_url_set failed: " << uc);
        throw DAVException(DAVException::DAV_INVALID_ARG);
    }

    Init();
}

CurlUri::CurlUri(CURLU /*const*/& rUrl)
    : m_pUrl(curl_url_dup(&rUrl))
{
    if (!m_pUrl)
    {
        throw ::std::bad_alloc();
    }

    Init();
}

CurlUri::CurlUri(CurlUri const& rOther)
    : m_pUrl(curl_url_dup(rOther.m_pUrl.get()))
    , m_URI(rOther.m_URI)
    , m_Scheme(rOther.m_Scheme)
    , m_User(rOther.m_User)
    , m_Password(rOther.m_Password)
    , m_Host(rOther.m_Host)
    , m_nPort(rOther.m_nPort)
    , m_Path(rOther.m_Path)
    , m_QueryAndFragment(rOther.m_QueryAndFragment)
{
    assert(rOther.m_pUrl);
    if (!m_pUrl)
    {
        throw ::std::bad_alloc();
    }
}

void CurlUri::operator=(CurlUri const& rOther)
{
    assert(rOther.m_pUrl);
    m_pUrl.reset(curl_url_dup(rOther.m_pUrl.get()));
    if (!m_pUrl)
    {
        throw ::std::bad_alloc();
    }
    m_URI = rOther.m_URI;
    m_Scheme = rOther.m_Scheme;
    m_User = rOther.m_User;
    m_Password = rOther.m_Password;
    m_Host = rOther.m_Host;
    m_nPort = rOther.m_nPort;
    m_Path = rOther.m_Path;
    m_QueryAndFragment = rOther.m_QueryAndFragment;
}

bool CurlUri::operator==(CurlUri const& rOther) const { return m_URI == rOther.m_URI; }

OUString CurlUri::GetPathBaseName() const
{
    sal_Int32 nPos = m_Path.lastIndexOf('/');
    sal_Int32 nTrail = 0;
    if (nPos == m_Path.getLength() - 1)
    {
        // Trailing slash found. Skip.
        nTrail = 1;
        nPos = m_Path.lastIndexOf('/', nPos);
    }
    if (nPos == -1)
    {
        return u"/"_ustr;
    }
    return m_Path.copy(nPos + 1, m_Path.getLength() - nPos - 1 - nTrail);
}

OUString CurlUri::GetPathBaseNameUnescaped() const { return DecodeURI(GetPathBaseName()); }

void CurlUri::SetScheme(::std::u16string_view const rScheme)
{
    OString const utf8URI(OUStringToOString(rScheme, RTL_TEXTENCODING_UTF8));
    auto uc = curl_url_set(m_pUrl.get(), CURLUPART_SCHEME, utf8URI.getStr(), 0);
    if (uc != CURLUE_OK)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "curl_url_set failed: " << uc);
        throw DAVException(DAVException::DAV_INVALID_ARG);
    }
    auto const oURI(GetURLComponent(*m_pUrl, CURLUPART_URL, CURLUE_OK, CURLU_NO_DEFAULT_PORT));
    assert(oURI);
    m_URI = *oURI;
    auto const oScheme(GetURLComponent(*m_pUrl, CURLUPART_SCHEME, CURLUE_NO_SCHEME));
    if (oScheme)
    {
        m_Scheme = *oScheme;
    }
}

void CurlUri::AppendPath(::std::u16string_view const rPath)
{
    OUStringBuffer path(m_Path);
    if (path.lastIndexOf('/') != path.getLength() - 1)
    {
        path.append("/");
    }
    path.append(rPath);
    OString const utf8Path(OUStringToOString(path, RTL_TEXTENCODING_UTF8));
    auto uc = curl_url_set(m_pUrl.get(), CURLUPART_PATH, utf8Path.getStr(), 0);
    if (uc != CURLUE_OK)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "curl_url_set failed: " << uc);
        throw DAVException(DAVException::DAV_INVALID_ARG);
    }
    auto const oURI(GetURLComponent(*m_pUrl, CURLUPART_URL, CURLUE_OK, CURLU_NO_DEFAULT_PORT));
    assert(oURI);
    m_URI = *oURI;
    auto const oPath(GetURLComponent(*m_pUrl, CURLUPART_PATH, CURLUE_OK));
    assert(oPath);
    m_Path = *oPath;
}

CurlUri CurlUri::CloneWithRelativeRefPathAbsolute(std::u16string_view rRelativeRef) const
{
    ::std::unique_ptr<CURLU, deleter_from_fn<CURLU, curl_url_cleanup>> pUrl(
        curl_url_dup(m_pUrl.get()));
    size_t indexEnd(rRelativeRef.size());
    auto const indexQuery(rRelativeRef.find('?'));
    auto const indexFragment(rRelativeRef.find('#'));
    CURLUcode uc;
    if (indexFragment != std::u16string_view::npos)
    {
        std::u16string_view const fragment(rRelativeRef.substr(indexFragment + 1));
        indexEnd = indexFragment;
        OString const utf8Fragment(OUStringToOString(fragment, RTL_TEXTENCODING_UTF8));
        uc = curl_url_set(pUrl.get(), CURLUPART_FRAGMENT, utf8Fragment.getStr(), 0);
    }
    else
    {
        uc = curl_url_set(pUrl.get(), CURLUPART_FRAGMENT, nullptr, 0);
    }
    if (uc != CURLUE_OK)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "curl_url_set failed: " << uc);
        throw DAVException(DAVException::DAV_INVALID_ARG);
    }
    if (indexQuery != std::u16string_view::npos
        && (indexFragment == std::u16string_view::npos || indexQuery < indexFragment))
    {
        std::u16string_view const query(
            rRelativeRef.substr(indexQuery + 1, indexEnd - indexQuery - 1));
        indexEnd = indexQuery;
        OString const utf8Query(OUStringToOString(query, RTL_TEXTENCODING_UTF8));
        uc = curl_url_set(pUrl.get(), CURLUPART_QUERY, utf8Query.getStr(), 0);
    }
    else
    {
        uc = curl_url_set(pUrl.get(), CURLUPART_QUERY, nullptr, 0);
    }
    if (uc != CURLUE_OK)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "curl_url_set failed: " << uc);
        throw DAVException(DAVException::DAV_INVALID_ARG);
    }
    std::u16string_view const path(rRelativeRef.substr(0, indexEnd));
    OString const utf8Path(OUStringToOString(path, RTL_TEXTENCODING_UTF8));
    uc = curl_url_set(pUrl.get(), CURLUPART_PATH, utf8Path.getStr(), 0);
    if (uc != CURLUE_OK)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "curl_url_set failed: " << uc);
        throw DAVException(DAVException::DAV_INVALID_ARG);
    }
    return CurlUri(*pUrl);
}

OUString EncodeSegment(OUString const& rSegment)
{
    return rtl::Uri::encode(rSegment, rtl_UriCharClassPchar, rtl_UriEncodeIgnoreEscapes,
                            RTL_TEXTENCODING_UTF8);
}

OUString DecodeURI(OUString const& rURI)
{
    return rtl::Uri::decode(rURI, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8);
}

OUString ConnectionEndPointString(std::u16string_view rHostName, sal_uInt16 const nPort)
{
    OUStringBuffer aBuf;

    // Is host a numeric IPv6 address?
    if ((rHostName.find(':') != std::u16string_view::npos) && (rHostName[0] != '['))
    {
        aBuf.append(OUString::Concat("[") + rHostName + "]");
    }
    else
    {
        aBuf.append(rHostName);
    }

    if ((nPort != DEFAULT_HTTP_PORT) && (nPort != DEFAULT_HTTPS_PORT))
    {
        aBuf.append(":" + OUString::number(sal_Int32(nPort)));
    }
    return aBuf.makeStringAndClear();
}

} // namespace http_dav_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
