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

#ifndef INCLUDED_STOC_SOURCE_URIPROC_URIREFERENCE_HXX
#define INCLUDED_STOC_SOURCE_URIPROC_URIREFERENCE_HXX

#include <com/sun/star/uno/RuntimeException.hpp>
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <rtl/ustrbuf.hxx>

namespace stoc { namespace uriproc {

class UriReference {
public:
    UriReference(
        OUString const & scheme, bool isHierarchical, bool hasAuthority,
        OUString const & authority, OUString const & path,
        bool hasQuery, OUString const & query);

    ~UriReference();

    OUString getUriReference()
        throw (css::uno::RuntimeException);

    bool isAbsolute() throw (css::uno::RuntimeException);

    OUString getScheme() throw (css::uno::RuntimeException) { return m_scheme;}

    OUString getSchemeSpecificPart()
        throw (css::uno::RuntimeException);

    bool isHierarchical() throw (css::uno::RuntimeException);

    bool hasAuthority() throw (css::uno::RuntimeException);

    OUString getAuthority() throw (css::uno::RuntimeException);

    OUString getPath() throw (css::uno::RuntimeException);

    bool hasRelativePath() throw (css::uno::RuntimeException);

    sal_Int32 getPathSegmentCount()
        throw (css::uno::RuntimeException);

    OUString getPathSegment(sal_Int32 index)
        throw (css::uno::RuntimeException);

    bool hasQuery() throw (css::uno::RuntimeException);

    OUString getQuery() throw (css::uno::RuntimeException);

    bool hasFragment() throw (css::uno::RuntimeException);

    OUString getFragment() throw (css::uno::RuntimeException);

    void setFragment(OUString const & fragment)
        throw (css::uno::RuntimeException);

    void clearFragment() throw (css::uno::RuntimeException);

    osl::Mutex m_mutex;
    OUString m_scheme;
    OUString m_authority;
    OUString m_path;
    OUString m_query;
    OUString m_fragment;
    bool m_isHierarchical;
    bool m_hasAuthority;
    bool m_hasQuery;
    bool m_hasFragment;

private:
    UriReference(UriReference &) = delete;
    void operator =(UriReference) = delete;

    void appendSchemeSpecificPart(OUStringBuffer & buffer) const;
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
