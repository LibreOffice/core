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
#ifndef INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_DAVPROPERTIES_HXX
#define INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_DAVPROPERTIES_HXX

#include <config_lgpl.h>
#include <rtl/ustring.hxx>
#include "NeonTypes.hxx"

namespace webdav_ucp
{

struct DAVProperties
{
    static constexpr OUStringLiteral CREATIONDATE = u"DAV:creationdate";
    static constexpr OUStringLiteral DISPLAYNAME = u"DAV:displayname";
    static constexpr OUStringLiteral GETCONTENTLANGUAGE = u"DAV:getcontentlanguage";
    static constexpr OUStringLiteral GETCONTENTLENGTH = u"DAV:getcontentlength";
    static constexpr OUStringLiteral GETCONTENTTYPE = u"DAV:getcontenttype";
    static constexpr OUStringLiteral GETETAG = u"DAV:getetag";
    static constexpr OUStringLiteral GETLASTMODIFIED = u"DAV:getlastmodified";
    static constexpr OUStringLiteral LOCKDISCOVERY = u"DAV:lockdiscovery";
    static constexpr OUStringLiteral RESOURCETYPE = u"DAV:resourcetype";
    static constexpr OUStringLiteral SOURCE = u"DAV:source";
    static constexpr OUStringLiteral SUPPORTEDLOCK = u"DAV:supportedlock";
    static constexpr OUStringLiteral EXECUTABLE = u"http://apache.org/dav/props/executable";

    static void createNeonPropName( const OUString & rFullName,
                                    NeonPropName & rName );
    static void createUCBPropName ( const char * nspace,
                                    const char * name,
                                    OUString & rFullName );

    static bool isUCBDeadProperty( const NeonPropName & rName );
    static bool isUCBSpecialProperty( const OUString & rFullName,
                                      OUString & rParsedName );
};

} // namespace webdav_ucp

#endif // INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_DAVPROPERTIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
