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


#ifndef _DAVPROPERTIES_HXX_
#define _DAVPROPERTIES_HXX_

#include <rtl/ustring.hxx>
#include "SerfTypes.hxx"

namespace http_dav_ucp
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
    static const ::rtl::OUString SUPPORTEDLOCK;
    static const ::rtl::OUString EXECUTABLE;

    static void createSerfPropName( const rtl::OUString & rFullName,
                                    SerfPropName & rName );
    static void createUCBPropName ( const char * nspace,
                                    const char * name,
                                    rtl::OUString & rFullName );

    static bool isUCBDeadProperty( const SerfPropName & rName );
};

} // namespace http_dav_ucp

#endif // _DAVPROPERTIES_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
