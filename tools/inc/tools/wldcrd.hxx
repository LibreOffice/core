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
#ifndef _WLDCRD_HXX
#define _WLDCRD_HXX

#include "tools/toolsdllapi.h"
#include <tools/solar.h>
#include <tools/string.hxx>
#include <osl/thread.h>

// ------------
// - WildCard -
// ------------

class TOOLS_DLLPUBLIC WildCard
{
private:
    ByteString      aWildString;
    char            cSepSymbol;

    sal_uInt16          ImpMatch( const char *pWild, const char *pStr ) const;

public:
                    WildCard();
                    WildCard( const String& rWildCards,
                              const char cSeparator = '\0' );

    const String    GetWildCard() const     { return UniString( aWildString, osl_getThreadTextEncoding()); }
    const String    operator ()() const     { return UniString( aWildString, osl_getThreadTextEncoding()); }

    sal_Bool            Matches( const String& rStr ) const;

    sal_Bool            operator ==( const String& rString ) const
                        { return Matches( rString ); }
    sal_Bool            operator !=( const String& rString ) const
                        { return !( Matches( rString ) ); }

    WildCard&       operator =( const String& rString );
    WildCard&       operator =( const WildCard& rWildCard );
};

inline WildCard::WildCard() :
                    aWildString( '*' )
{
    cSepSymbol  = '\0';
}

inline WildCard::WildCard( const String& rWildCard, const char cSeparator ) :
                    aWildString( rWildCard, osl_getThreadTextEncoding())
{
    cSepSymbol  = cSeparator;
}

inline WildCard& WildCard::operator=( const String& rString )
{
    aWildString = ByteString(rString, osl_getThreadTextEncoding());
    return *this;
}

inline WildCard& WildCard::operator=( const WildCard& rWildCard )
{
    aWildString = rWildCard.aWildString;
    cSepSymbol = rWildCard.cSepSymbol;
    return *this;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
