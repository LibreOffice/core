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


#ifndef INCLUDED_UCB_SOURCE_UCP_WEBDAV_DAVPROPERTIES_HXX
#define INCLUDED_UCB_SOURCE_UCP_WEBDAV_DAVPROPERTIES_HXX

#include <rtl/ustring.hxx>

namespace http_dav_ucp
{

typedef struct { const char *nspace, *name; } SerfPropName;

struct DAVProperties
{
    static const char CREATIONDATE[];
    static const char DISPLAYNAME[];
    static const char GETCONTENTLANGUAGE[];
    static const char GETCONTENTLENGTH[];
    static const char GETCONTENTTYPE[];
    static const char GETETAG[];
    static const char GETLASTMODIFIED[];
    static const char LOCKDISCOVERY[];
    static const char RESOURCETYPE[];
    static const char SUPPORTEDLOCK[];
    static const char EXECUTABLE[];

    static void createSerfPropName( const OUString & rFullName,
                                    SerfPropName & rName );
    static void createUCBPropName ( const char * nspace,
                                    const char * name,
                                    OUString & rFullName );

    static bool isUCBDeadProperty( const SerfPropName & rName );
    static bool isUCBSpecialProperty( const rtl::OUString & rFullName,
                                      rtl::OUString & rParsedName );
};

} // namespace http_dav_ucp

#endif // INCLUDED_UCB_SOURCE_UCP_WEBDAV_DAVPROPERTIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
