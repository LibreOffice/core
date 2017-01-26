/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include "UriReference.hxx"

#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <rtl/string.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

using stoc::uriproc::UriReference;

UriReference::UriReference(
    OUString const & scheme, bool bIsHierarchical, bool bHasAuthority,
    OUString const & authority, OUString const & path,
    bool bHasQuery, OUString const & query):
    m_scheme(scheme),
    m_authority(authority),
    m_path(path),
    m_query(query),
    m_isHierarchical(bIsHierarchical),
    m_hasAuthority(bHasAuthority),
    m_hasQuery(bHasQuery),
    m_hasFragment(false)
{
    OSL_ASSERT(!scheme.isEmpty() || bIsHierarchical);
    OSL_ASSERT(!bHasAuthority || bIsHierarchical);
    OSL_ASSERT(authority.isEmpty() || bHasAuthority);
    OSL_ASSERT(!bHasQuery || bIsHierarchical);
    OSL_ASSERT(query.isEmpty() || bHasQuery);
}

UriReference::~UriReference() {}

OUString UriReference::getUriReference()
{
    osl::MutexGuard g(m_mutex);
    OUStringBuffer buf;
    if (!m_scheme.isEmpty()) {
        buf.append(m_scheme);
        buf.append(':');
    }
    appendSchemeSpecificPart(buf);
    if (m_hasFragment) {
        buf.append('#');
        buf.append(m_fragment);
    }
    return buf.makeStringAndClear();
}

bool UriReference::isAbsolute() {
    return !m_scheme.isEmpty();
}


OUString UriReference::getSchemeSpecificPart()
{
    osl::MutexGuard g(m_mutex);
    OUStringBuffer buf;
    appendSchemeSpecificPart(buf);
    return buf.makeStringAndClear();
}

bool UriReference::isHierarchical() {
    osl::MutexGuard g(m_mutex);
    return m_isHierarchical;
}

bool UriReference::hasAuthority() {
    osl::MutexGuard g(m_mutex);
    return m_hasAuthority;
}

OUString UriReference::getAuthority() {
    osl::MutexGuard g(m_mutex);
    return m_authority;
}

OUString UriReference::getPath() {
    osl::MutexGuard g(m_mutex);
    return m_path;
}

bool UriReference::hasRelativePath() {
    osl::MutexGuard g(m_mutex);
    return m_isHierarchical && !m_hasAuthority
        && (m_path.isEmpty() || m_path[0] != '/');
}

sal_Int32 UriReference::getPathSegmentCount()
{
    osl::MutexGuard g(m_mutex);
    if (!m_isHierarchical || m_path.isEmpty()) {
        return 0;
    } else {
        sal_Int32 n = m_path[0] == '/' ? 0 : 1;
        for (sal_Int32 i = 0;; ++i) {
            i = m_path.indexOf('/', i);
            if (i < 0) {
                break;
            }
            ++n;
        }
        return n;
    }
}

OUString UriReference::getPathSegment(sal_Int32 index)
{
    osl::MutexGuard g(m_mutex);
    if (m_isHierarchical && !m_path.isEmpty() && index >= 0) {
        for (sal_Int32 i = m_path[0] == '/' ? 1 : 0;; ++i) {
            if (index-- == 0) {
                sal_Int32 j = m_path.indexOf('/', i);
                return j < 0 ? m_path.copy(i) : m_path.copy(i, j - i);
            }
            i = m_path.indexOf('/', i);
            if (i < 0) {
                break;
            }
        }
    }
    return OUString();
}

bool UriReference::hasQuery() {
    osl::MutexGuard g(m_mutex);
    return m_hasQuery;
}

OUString UriReference::getQuery() {
    osl::MutexGuard g(m_mutex);
    return m_query;
}

bool UriReference::hasFragment() {
    osl::MutexGuard g(m_mutex);
    return m_hasFragment;
}

OUString UriReference::getFragment() {
    osl::MutexGuard g(m_mutex);
    return m_fragment;
}

void UriReference::setFragment(OUString const & fragment)
{
    osl::MutexGuard g(m_mutex);
    m_hasFragment = true;
    m_fragment = fragment;
}

void UriReference::clearFragment() {
    osl::MutexGuard g(m_mutex);
    m_hasFragment = false;
    m_fragment.clear();
}

void UriReference::appendSchemeSpecificPart(OUStringBuffer & buffer) const
{
    if (m_hasAuthority) {
        buffer.append("//");
        buffer.append(m_authority);
    }
    buffer.append(m_path);
    if (m_hasQuery) {
        buffer.append('?');
        buffer.append(m_query);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
