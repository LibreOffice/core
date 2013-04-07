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

#ifndef INCLUDED_stoc_source_uriproc_UriReference_hxx
#define INCLUDED_stoc_source_uriproc_UriReference_hxx

#include "com/sun/star/uno/RuntimeException.hpp"
#include "osl/mutex.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "rtl/ustrbuf.hxx"

namespace stoc { namespace uriproc {

class UriReference {
public:
    UriReference(
        OUString const & scheme, bool isHierarchical, bool hasAuthority,
        OUString const & authority, OUString const & path,
        bool hasQuery, OUString const & query);

    ~UriReference();

    OUString getUriReference()
        throw (com::sun::star::uno::RuntimeException);

    sal_Bool isAbsolute() throw (com::sun::star::uno::RuntimeException);

    OUString getScheme() throw (com::sun::star::uno::RuntimeException);

    OUString getSchemeSpecificPart()
        throw (com::sun::star::uno::RuntimeException);

    sal_Bool isHierarchical() throw (com::sun::star::uno::RuntimeException);

    sal_Bool hasAuthority() throw (com::sun::star::uno::RuntimeException);

    OUString getAuthority() throw (com::sun::star::uno::RuntimeException);

    OUString getPath() throw (com::sun::star::uno::RuntimeException);

    sal_Bool hasRelativePath() throw (com::sun::star::uno::RuntimeException);

    sal_Int32 getPathSegmentCount()
        throw (com::sun::star::uno::RuntimeException);

    OUString getPathSegment(sal_Int32 index)
        throw (com::sun::star::uno::RuntimeException);

    sal_Bool hasQuery() throw (com::sun::star::uno::RuntimeException);

    OUString getQuery() throw (com::sun::star::uno::RuntimeException);

    sal_Bool hasFragment() throw (com::sun::star::uno::RuntimeException);

    OUString getFragment() throw (com::sun::star::uno::RuntimeException);

    void setFragment(OUString const & fragment)
        throw (com::sun::star::uno::RuntimeException);

    void clearFragment() throw (com::sun::star::uno::RuntimeException);

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
    UriReference(UriReference &); // not implemented
    void operator =(UriReference); // not implemented

    void appendSchemeSpecificPart(OUStringBuffer & buffer) const;
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
