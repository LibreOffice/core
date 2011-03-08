/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_stoc_source_uriproc_UriReference_hxx
#define INCLUDED_stoc_source_uriproc_UriReference_hxx

#include "com/sun/star/uno/RuntimeException.hpp"
#include "osl/mutex.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

namespace rtl { class OUStringBuffer; }

namespace stoc { namespace uriproc {

class UriReference {
public:
    UriReference(
        rtl::OUString const & scheme, bool isHierarchical, bool hasAuthority,
        rtl::OUString const & authority, rtl::OUString const & path,
        bool hasQuery, rtl::OUString const & query);

    ~UriReference();

    rtl::OUString getUriReference()
        throw (com::sun::star::uno::RuntimeException);

    sal_Bool isAbsolute() throw (com::sun::star::uno::RuntimeException);

    rtl::OUString getScheme() throw (com::sun::star::uno::RuntimeException);

    rtl::OUString getSchemeSpecificPart()
        throw (com::sun::star::uno::RuntimeException);

    sal_Bool isHierarchical() throw (com::sun::star::uno::RuntimeException);

    sal_Bool hasAuthority() throw (com::sun::star::uno::RuntimeException);

    rtl::OUString getAuthority() throw (com::sun::star::uno::RuntimeException);

    rtl::OUString getPath() throw (com::sun::star::uno::RuntimeException);

    sal_Bool hasRelativePath() throw (com::sun::star::uno::RuntimeException);

    sal_Int32 getPathSegmentCount()
        throw (com::sun::star::uno::RuntimeException);

    rtl::OUString getPathSegment(sal_Int32 index)
        throw (com::sun::star::uno::RuntimeException);

    sal_Bool hasQuery() throw (com::sun::star::uno::RuntimeException);

    rtl::OUString getQuery() throw (com::sun::star::uno::RuntimeException);

    sal_Bool hasFragment() throw (com::sun::star::uno::RuntimeException);

    rtl::OUString getFragment() throw (com::sun::star::uno::RuntimeException);

    void setFragment(rtl::OUString const & fragment)
        throw (com::sun::star::uno::RuntimeException);

    void clearFragment() throw (com::sun::star::uno::RuntimeException);

    osl::Mutex m_mutex;
    rtl::OUString m_scheme;
    rtl::OUString m_authority;
    rtl::OUString m_path;
    rtl::OUString m_query;
    rtl::OUString m_fragment;
    bool m_isHierarchical;
    bool m_hasAuthority;
    bool m_hasQuery;
    bool m_hasFragment;

private:
    UriReference(UriReference &); // not implemented
    void operator =(UriReference); // not implemented

    void appendSchemeSpecificPart(rtl::OUStringBuffer & buffer) const;
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
