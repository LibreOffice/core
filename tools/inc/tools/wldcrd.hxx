/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wldcrd.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:21:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _WLDCRD_HXX
#define _WLDCRD_HXX

#ifndef INCLUDED_TOOLSDLLAPI_H
#include "tools/toolsdllapi.h"
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

// ------------
// - WildCard -
// ------------

class TOOLS_DLLPUBLIC WildCard
{
private:
    ByteString      aWildString;
    char            cSepSymbol;

    USHORT          ImpMatch( const char *pWild, const char *pStr ) const;

public:
                    WildCard();
                    WildCard( const String& rWildCards,
                              const char cSeparator = '\0' );

    const String    GetWildCard() const     { return UniString( aWildString, osl_getThreadTextEncoding()); }
    const String    operator ()() const     { return UniString( aWildString, osl_getThreadTextEncoding()); }

    BOOL            Matches( const String& rStr ) const;

    BOOL            operator ==( const String& rString ) const
                        { return Matches( rString ); }
    BOOL            operator !=( const String& rString ) const
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
