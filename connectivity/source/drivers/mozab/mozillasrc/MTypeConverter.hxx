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


#ifndef _CONNECTIVITY_MAB_TYPECONVERTER_HXX_
#define _CONNECTIVITY_MAB_TYPECONVERTER_HXX_

#include <rtl/ustring.hxx>

#include<string>

namespace connectivity
{
    namespace mozab
    {
        class MTypeConverter
        {
        public:
            static void  ouStringToNsString(const OUString&, nsString&);
            static void  nsStringToOUString(const nsString&, OUString&);
            static void  prUnicharToOUString(const PRUnichar*, OUString&);
            // Use free() for the following 3 calls.
            static char *ouStringToCCharStringAscii(const OUString&);
            static char *nsStringToCCharStringAscii(const nsString&);
            static char *ouStringToCCharStringUtf8(const OUString&);
            // Convert to stl-string.
            static ::std::string ouStringToStlString(const OUString&);
            static ::std::string nsStringToStlString(const nsString&);

            static OString nsACStringToOString( const nsACString& _source );
            static void asciiToNsACString( const sal_Char* _asciiString, nsACString& _dest );

        private:
            MTypeConverter() {};
        };
    }
}

#endif // _CONNECTIVITY_MAB_TYPECONVERTER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
