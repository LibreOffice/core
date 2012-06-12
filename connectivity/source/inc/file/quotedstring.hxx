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

#ifndef CONNECTIVITY_QUOTED_STRING_HXX
#define CONNECTIVITY_QUOTED_STRING_HXX

#include <tools/string.hxx>
#include "file/filedllapi.hxx"

namespace connectivity
{
    //==================================================================
    // Derived from String, overriding GetToken/GetTokenCount methods
    // Especially true for the flat file format: Strings can be quoted
    //==================================================================
    class OOO_DLLPUBLIC_FILE QuotedTokenizedString
    {
        String m_sString;
    public:
        QuotedTokenizedString() {}
        QuotedTokenizedString(const String& _sString) : m_sString(_sString){}

        xub_StrLen  GetTokenCount( sal_Unicode cTok , sal_Unicode cStrDel ) const;
        void        GetTokenSpecial( String& _rStr,xub_StrLen& nStartPos, sal_Unicode cTok = ';', sal_Unicode cStrDel = '\0' ) const;
        inline String& GetString() { return m_sString; }
        inline xub_StrLen Len() const { return m_sString.Len(); }
        inline operator String&() { return m_sString; }
    };
}

#endif // CONNECTIVITY_QUOTED_STRING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
