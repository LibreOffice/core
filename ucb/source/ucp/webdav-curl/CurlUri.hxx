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

#pragma once

#include <curl/curl.h>

#include <memory>

#include <rtl/ustring.hxx>

#include "DAVException.hxx"

namespace http_dav_ucp
{
template <typename T, auto fn> struct deleter_from_fn
{
    void operator()(T* p) const { fn(p); }
};
template <typename T> using CurlUniquePtr = ::std::unique_ptr<T, deleter_from_fn<T, curl_free>>;

class CurlUri
{
private:
    /// native curl representation of parsed URI
    ::std::unique_ptr<CURLU, deleter_from_fn<CURLU, curl_url_cleanup>> m_pUrl;
    /// duplicate state for quick access to some components
    OUString m_URI;
    OUString m_Scheme;
    OUString m_User;
    OUString m_Password;
    OUString m_Host;
    sal_uInt16 m_nPort = 0;
    OUString m_Path;
    OUString m_QueryAndFragment;

    void Init();

public:
    CurlUri(CurlUri const& rUri);
    CurlUri(CURLU /*const*/& rUrl);
    void operator=(CurlUri const& rOther);

    /// @throws DAVException
    explicit CurlUri(::std::u16string_view rURI);

    bool operator==(CurlUri const& rOther) const;

    CURLU const* GetCURLU() const { return m_pUrl.get(); }
    OUString const& GetURI() const { return m_URI; }
    OUString const& GetScheme() const { return m_Scheme; }
    OUString const& GetUser() const { return m_User; }
    OUString const& GetPassword() const { return m_Password; }
    OUString const& GetHost() const { return m_Host; }
    sal_uInt16 GetPort() const { return m_nPort; }
    OUString const& GetPath() const { return m_Path; }
    OUString GetRelativeReference() const { return m_Path + m_QueryAndFragment; }

    OUString GetPathBaseName() const;

    OUString GetPathBaseNameUnescaped() const;

    /// @throws DAVException
    void SetScheme(::std::u16string_view rScheme);
    /// @throws DAVException
    void AppendPath(::std::u16string_view rPath);
    /// @param matches: relative-ref = path-absolute [ "?" query ] [ "#" fragment ]
    /// @throws DAVException
    CurlUri CloneWithRelativeRefPathAbsolute(OUString const& rRelativeRef) const;
};

OUString EncodeSegment(OUString const& rSegment);
OUString DecodeURI(OUString const& rURI);
OUString ConnectionEndPointString(OUString const& rHost, sal_uInt16 nPort);

} // namespace http_dav_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
