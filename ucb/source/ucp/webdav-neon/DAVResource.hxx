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

#ifndef INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_DAVRESOURCE_HXX
#define INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_DAVRESOURCE_HXX

#include <config_lgpl.h>
#include <vector>

#include "rtl/ustring.hxx"
#include "com/sun/star/uno/Any.hxx"

namespace webdav_ucp
{

struct DAVPropertyValue
{
    OUString         Name;
    css::uno::Any    Value;
    bool             IsCaseSensitive;

    DAVPropertyValue() : IsCaseSensitive( true ) {}
};

struct DAVResource
{
    OUString uri;
    std::vector< DAVPropertyValue > properties;

    DAVResource() {}
    explicit DAVResource( const OUString & inUri ) : uri( inUri ) {}
};

struct DAVResourceInfo
{
    std::vector < OUString > properties;

    bool operator==( const struct DAVResourceInfo& a ) const
    {
        return (properties == a.properties );
    }
};

} // namespace webdav_ucp

#endif // INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_DAVRESOURCE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
