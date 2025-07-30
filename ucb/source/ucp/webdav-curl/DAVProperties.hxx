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


#pragma once

#include <rtl/ustring.hxx>
#include <rtl/string.hxx>

namespace http_dav_ucp
{

typedef struct { OString nspace; OString name; } SerfPropName;

struct DAVProperties
{
    static constexpr OUString CREATIONDATE = u"DAV:creationdate"_ustr;
    static constexpr OUString DISPLAYNAME = u"DAV:displayname"_ustr;
    static constexpr OUString GETCONTENTLANGUAGE = u"DAV:getcontentlanguage"_ustr;
    static constexpr OUString GETCONTENTLENGTH = u"DAV:getcontentlength"_ustr;
    static constexpr OUString GETCONTENTTYPE = u"DAV:getcontenttype"_ustr;
    static constexpr OUString GETETAG = u"DAV:getetag"_ustr;
    static constexpr OUString GETLASTMODIFIED = u"DAV:getlastmodified"_ustr;
    static constexpr OUString LOCKDISCOVERY = u"DAV:lockdiscovery"_ustr;
    static constexpr OUString RESOURCETYPE = u"DAV:resourcetype"_ustr;
    static constexpr OUString SUPPORTEDLOCK = u"DAV:supportedlock"_ustr;
    static constexpr OUString EXECUTABLE = u"http://apache.org/dav/props/executable"_ustr;

    static void createSerfPropName( ::std::u16string_view rFullName,
                                    SerfPropName & rName );
    static void createUCBPropName ( const char * nspace,
                                    const char * name,
                                    OUString & rFullName );

    static bool isUCBDeadProperty( const SerfPropName & rName );
    static bool isUCBSpecialProperty( std::u16string_view rFullName,
                                      OUString & rParsedName );
};

} // namespace http_dav_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
