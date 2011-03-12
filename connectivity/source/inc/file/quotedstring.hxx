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

#ifndef CONNECTIVITY_QUOTED_STRING_HXX
#define CONNECTIVITY_QUOTED_STRING_HXX

#include <tools/string.hxx>
#include "file/filedllapi.hxx"

namespace connectivity
{
    //==================================================================
    // Ableitung von String mit ueberladenen GetToken/GetTokenCount-Methoden
    // Speziell fuer FLAT FILE-Format: Strings koennen gequotet sein
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
