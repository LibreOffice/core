/*************************************************************************
 *
 *  $RCSfile: metric.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2003-10-15 10:02:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <metric.hxx>

// =======================================================================

FontInfo::FontInfo()
{
    mpImplMetric                = new ImplFontMetric;
    mpImplMetric->mnRefCount    = 1;
    mpImplMetric->meType        = TYPE_DONTKNOW;
    mpImplMetric->mbDevice      = FALSE;
    mpImplMetric->mnAscent      = 0;
    mpImplMetric->mnDescent     = 0;
    mpImplMetric->mnIntLeading  = 0;
    mpImplMetric->mnExtLeading  = 0;
    mpImplMetric->mnLineHeight  = 0;
    mpImplMetric->mnSlant       = 0;
    mpImplMetric->mnFirstChar   = 0;
    mpImplMetric->mnLastChar    = 0;
}

// -----------------------------------------------------------------------

FontInfo::FontInfo( const FontInfo& rInfo ) :
    Font( rInfo )
{
    mpImplMetric = rInfo.mpImplMetric;
    mpImplMetric->mnRefCount++;
}

// -----------------------------------------------------------------------

FontInfo::~FontInfo()
{
    // Eventuell Metric loeschen
    if ( mpImplMetric->mnRefCount > 1 )
        mpImplMetric->mnRefCount--;
    else
        delete mpImplMetric;
}

// -----------------------------------------------------------------------

FontInfo& FontInfo::operator=( const FontInfo& rInfo )
{
    Font::operator=( rInfo );

    // Zuerst Referenzcounter erhoehen, damit man sich selbst zuweisen kann
    rInfo.mpImplMetric->mnRefCount++;

    // Sind wir nicht die letzten ?
    if ( mpImplMetric->mnRefCount > 1 )
        mpImplMetric->mnRefCount--;
    else
        delete mpImplMetric;

    mpImplMetric = rInfo.mpImplMetric;

    return *this;

}

// -----------------------------------------------------------------------

BOOL FontInfo::operator==( const FontInfo& rInfo ) const
{
    if ( !Font::operator==( rInfo ) )
        return FALSE;

    if ( mpImplMetric == rInfo.mpImplMetric )
        return TRUE;

    if ( (mpImplMetric->meType      == rInfo.mpImplMetric->meType       ) &&
         (mpImplMetric->mbDevice    == rInfo.mpImplMetric->mbDevice     ) &&
         (mpImplMetric->mnAscent    == rInfo.mpImplMetric->mnAscent     ) &&
         (mpImplMetric->mnDescent   == rInfo.mpImplMetric->mnDescent    ) &&
         (mpImplMetric->mnIntLeading== rInfo.mpImplMetric->mnIntLeading ) &&
         (mpImplMetric->mnExtLeading== rInfo.mpImplMetric->mnExtLeading ) &&
         (mpImplMetric->mnSlant     == rInfo.mpImplMetric->mnSlant      ) &&
         (mpImplMetric->mnFirstChar == rInfo.mpImplMetric->mnFirstChar  ) &&
         (mpImplMetric->mnLastChar  == rInfo.mpImplMetric->mnLastChar   ) )
        return TRUE;
    else
        return FALSE;
}

// =======================================================================

static const sal_UCS4 pDefaultRangeCodes[] = {0x0020,0xD800, 0xE000,0xFFF0};

FontCharMap::FontCharMap()
:   mpRangeCodes( NULL )
{
    ImplSetDefaultRanges();
}

// -----------------------------------------------------------------------

FontCharMap::~FontCharMap()
{
    ImplSetRanges( 0, NULL );
}

// -----------------------------------------------------------------------

FontCharMap& FontCharMap::operator=( const FontCharMap& rMap )
{
    if( rMap.mpRangeCodes == pDefaultRangeCodes )
    {
        ImplSetDefaultRanges();
    }
    else
    {
        ULONG nPairs = rMap.mnRangeCount;
        sal_UCS4* pCodes = new sal_UCS4[ 2 * nPairs ];
        for( ULONG i = 0; i < 2*nPairs; ++i )
            pCodes[ i ] = rMap.mpRangeCodes[ i ];
        ImplSetRanges( nPairs, pCodes );
    }

    return *this;
}

// -----------------------------------------------------------------------

void FontCharMap::ImplSetRanges( ULONG nPairs, const sal_UCS4* pCodes )
{
    if( mpRangeCodes && mpRangeCodes != pDefaultRangeCodes )
        delete[] const_cast<ULONG*>( mpRangeCodes );

    mnRangeCount = nPairs;
    mpRangeCodes = pCodes;

    mnCharCount = 0;
    for( int i = 0; i < nPairs; ++i )
        mnCharCount += mpRangeCodes[ 2*i+1 ] - mpRangeCodes[ 2*i ];
}

// -----------------------------------------------------------------------

void FontCharMap::ImplSetDefaultRanges()
{
    int nCodes = sizeof(pDefaultRangeCodes) / sizeof(*pDefaultRangeCodes);
    ImplSetRanges( nCodes/2, pDefaultRangeCodes );
}

// -----------------------------------------------------------------------

BOOL FontCharMap::IsDefaultMap() const
{
    return (mpRangeCodes == pDefaultRangeCodes);
}

// -----------------------------------------------------------------------

void FontCharMap::GetRange( ULONG i, sal_UCS4& cBegin, sal_UCS4& cEnd ) const
{
    if( i < 0 || i >= mnRangeCount )
    {
        cBegin = pDefaultRangeCodes[ 0 ];
        cEnd   = pDefaultRangeCodes[ 1 ];
    }
    else
    {
        cBegin = mpRangeCodes[ 2*i ];
        cEnd   = mpRangeCodes[ 2*i+1 ];
    }
}

// -----------------------------------------------------------------------

int FontCharMap::ImplFindRange( sal_UCS4 cChar ) const
{
    int nLower = 0;
    int nMid   = mnRangeCount;
    int nUpper = 2 * mnRangeCount - 1;
    while( nLower < nUpper )
    {
        if( cChar >= mpRangeCodes[ nMid ] )
            nLower = nMid;
        else
            nUpper = nMid - 1;
        nMid = (nLower + nUpper + 1) / 2;
    }
    return nMid;
}

// -----------------------------------------------------------------------

BOOL FontCharMap::HasChar( sal_UCS4 cChar ) const
{
    int nRange = ImplFindRange( cChar );
    if( nRange==0 && cChar<mpRangeCodes[0] )
        return FALSE;
    return (nRange & 1) ? FALSE: TRUE;
}

// -----------------------------------------------------------------------

sal_UCS4 FontCharMap::GetFirstChar() const
{
    return mpRangeCodes[0];
}

// -----------------------------------------------------------------------

sal_UCS4 FontCharMap::GetLastChar() const
{
    return (mpRangeCodes[ 2*mnRangeCount-1 ] - 1);
}

// -----------------------------------------------------------------------

sal_UCS4 FontCharMap::GetNextChar( sal_UCS4 cChar ) const
{
    if( cChar < GetFirstChar() )
        return GetFirstChar();
    if( cChar >= GetLastChar() )
        return GetLastChar();

    int nRange = ImplFindRange( cChar );
    if( nRange & 1 )                        // inbetween ranges?
        return mpRangeCodes[ nRange + 1 ];  // first in next range
    return (cChar + 1);
}

// -----------------------------------------------------------------------

sal_UCS4 FontCharMap::GetPrevChar( sal_UCS4 cChar ) const
{
    if( cChar <= GetFirstChar() )
        return GetFirstChar();
    if( cChar > GetLastChar() )
        return GetLastChar();

    int nRange = ImplFindRange( cChar );
    if( nRange & 1 )                            // inbetween ranges?
        return (mpRangeCodes[ nRange ] - 1);
    else if( cChar == mpRangeCodes[ nRange ] )  // first in prev range?
        return (mpRangeCodes[ nRange-1 ] - 1);
    return (cChar - 1);
}

// =======================================================================
