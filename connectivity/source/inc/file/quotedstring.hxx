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

#include <rtl/ustring.hxx>
#include "file/filedllapi.hxx"

namespace connectivity
{
    //==================================================================
    // Derived from String, overriding GetToken/GetTokenCount methods
    // Especially true for the flat file format: Strings can be quoted
    //==================================================================
    class OOO_DLLPUBLIC_FILE QuotedTokenizedString
    {
        OUString m_sString;
    public:
        QuotedTokenizedString() {}
        QuotedTokenizedString(const OUString& _sString) : m_sString(_sString){}

        sal_Int32  GetTokenCount( sal_Unicode cTok , sal_Unicode cStrDel ) const;
        OUString   GetTokenSpecial(sal_Int32& nStartPos, sal_Unicode cTok = ';', sal_Unicode cStrDel = '\0') const;
        inline OUString& GetString() { return m_sString; }
        inline void SetString(OUString aStr) { m_sString = aStr;}
        inline sal_Int32 Len() const { return m_sString.getLength(); }
        inline operator OUString&() { return m_sString; }
    };
}

#endif // CONNECTIVITY_QUOTED_STRING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
