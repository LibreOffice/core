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

    // TODO: why put this in *path* ? because before 2007, ne_uri path contained query/fragment as well :-/
    auto const oQuery(GetURLComponent(*m_pUrl, CURLUPART_QUERY, CURLUE_NO_QUERY));
    if (oQuery)
    {
        m_Path += *oQuery;
    }
    auto const oFragment(GetURLComponent(*m_pUrl, CURLUPART_FRAGMENT, CURLUE_NO_FRAGMENT));
    if (oFragment)
    {
        m_Path += *oFragment;
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
        return "/";
    }
    OUString aTemp(m_Path.copy(nPos + 1, m_Path.getLength() - nPos - 1 - nTrail));

    // query, fragment present?
    nPos = aTemp.indexOf('?');
    if (nPos == -1)
        nPos = aTemp.indexOf('#');

    if (nPos != -1)
        aTemp = aTemp.copy(0, nPos);

    return aTemp;
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
    // FIXME: it is utter nonsense that m_Path contains query/fragment
    if (path.lastIndexOf('/') != path.getLength() - 1)
    {
        path.append("/");
    }
    path.append(rPath);
    OString const utf8Path(OUStringToOString(path.makeStringAndClear(), RTL_TEXTENCODING_UTF8));
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

OUString EncodeSegment(OUString const& rSegment)
{
    return rtl::Uri::encode(rSegment, rtl_UriCharClassPchar, rtl_UriEncodeIgnoreEscapes,
                            RTL_TEXTENCODING_UTF8);
}

OUString DecodeURI(OUString const& rURI)
{
    return rtl::Uri::decode(rURI, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8);
}

OUString ConnectionEndPointString(OUString const& rHostName, sal_uInt16 const nPort)
{
    OUStringBuffer aBuf;

    // Is host a numeric IPv6 address?
    if ((rHostName.indexOf(':') != -1) && (rHostName[0] != '['))
    {
        aBuf.append("[");
        aBuf.append(rHostName);
        aBuf.append("]");
    }
    else
    {
        aBuf.append(rHostName);
    }

    if ((nPort != DEFAULT_HTTP_PORT) && (nPort != DEFAULT_HTTPS_PORT))
    {
        aBuf.append(":");
        aBuf.append(sal_Int32(nPort));
    }
    return aBuf.makeStringAndClear();
}

} // namespace http_dav_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
