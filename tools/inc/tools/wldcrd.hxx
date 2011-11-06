/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
