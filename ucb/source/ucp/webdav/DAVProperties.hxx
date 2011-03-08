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
#ifndef _DAVPROPERTIES_HXX_
#define _DAVPROPERTIES_HXX_

#include <rtl/ustring.hxx>
#include "NeonTypes.hxx"

namespace webdav_ucp
{

struct DAVProperties
{
    static const ::rtl::OUString CREATIONDATE;
    static const ::rtl::OUString DISPLAYNAME;
    static const ::rtl::OUString GETCONTENTLANGUAGE;
    static const ::rtl::OUString GETCONTENTLENGTH;
    static const ::rtl::OUString GETCONTENTTYPE;
    static const ::rtl::OUString GETETAG;
    static const ::rtl::OUString GETLASTMODIFIED;
    static const ::rtl::OUString LOCKDISCOVERY;
    static const ::rtl::OUString RESOURCETYPE;
    static const ::rtl::OUString SOURCE;
    static const ::rtl::OUString SUPPORTEDLOCK;
    static const ::rtl::OUString EXECUTABLE;

    static void createNeonPropName( const rtl::OUString & rFullName,
                                    NeonPropName & rName );
    static void createUCBPropName ( const char * nspace,
                                    const char * name,
                                    rtl::OUString & rFullName );

    static bool isUCBDeadProperty( const NeonPropName & rName );
};

} // namespace webdav_ucp

#endif // _DAVPROPERTIES_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
