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

namespace http_dav_ucp
{

typedef struct { const char *nspace, *name; } SerfPropName;

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
    static constexpr OUStringLiteral SUPPORTEDLOCK = u"DAV:supportedlock";
    static constexpr OUStringLiteral EXECUTABLE = u"http://apache.org/dav/props/executable";

    static void createSerfPropName( ::std::u16string_view rFullName,
                                    SerfPropName & rName );
    static void createUCBPropName ( const char * nspace,
                                    const char * name,
                                    OUString & rFullName );

    static bool isUCBDeadProperty( const SerfPropName & rName );
    static bool isUCBSpecialProperty( const OUString & rFullName,
                                      OUString & rParsedName );
};

} // namespace http_dav_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
