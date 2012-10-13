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

#ifndef _DAVRESOURCE_HXX_
#define _DAVRESOURCE_HXX_

#include <vector>

#include "rtl/ustring.hxx"
#include "com/sun/star/uno/Any.hxx"

namespace webdav_ucp
{

struct DAVPropertyValue
{
    rtl::OUString            Name;
    com::sun::star::uno::Any Value;
    bool                     IsCaseSensitive;

    DAVPropertyValue() : IsCaseSensitive( true ) {}
};

struct DAVResource
{
    ::rtl::OUString uri;
    std::vector< DAVPropertyValue > properties;

    DAVResource() {}
    DAVResource( const ::rtl::OUString & inUri ) : uri( inUri ) {}
};

struct DAVResourceInfo
{
    ::rtl::OUString uri;
    std::vector < ::rtl::OUString > properties;

    DAVResourceInfo( const ::rtl::OUString & inUri ) : uri( inUri ) {}
};

} // namespace webdav_ucp

#endif // _DAVRESOURCE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
