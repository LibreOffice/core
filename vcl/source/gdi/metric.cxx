/*************************************************************************
 *
 *  $RCSfile: metric.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 09:29:57 $
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

#include <impfont.hxx>
#include <metric.hxx>

// =======================================================================

ImplFontMetric::ImplFontMetric()
:   mnRefCount( 1 ),
    mnMiscFlags( 0 ),
    mnAscent( 0 ),
    mnDescent( 0 ),
    mnIntLeading( 0 ),
    mnExtLeading( 0 ),
    mnLineHeight( 0 ),
    mnSlant( 0 )
{}

// -----------------------------------------------------------------------

inline void ImplFontMetric::AddReference()
{
    ++mnRefCount;
}

// -----------------------------------------------------------------------

inline void ImplFontMetric::DeReference()
{
    if( --mnRefCount <= 0 )
        delete this;
}

// -----------------------------------------------------------------------

bool ImplFontMetric::operator==( const ImplFontMetric& r ) const
{
    if( mnMiscFlags  != r.mnMiscFlags )
        return false;
    if( mnAscent     != r.mnAscent )
        return false;
    if( mnDescent    != r.mnDescent )
        return false;
    if( mnIntLeading != r.mnIntLeading )
        return false;
    if( mnExtLeading != r.mnExtLeading )
        return false;
    if( mnSlant      != r.mnSlant )
        return false;

    return true;
}

// =======================================================================

FontInfo::FontInfo()
:   mpImplMetric( new ImplFontMetric )
{}

// -----------------------------------------------------------------------

FontInfo::FontInfo( const FontInfo& rInfo )
:  Font( rInfo )
{
    mpImplMetric = rInfo.mpImplMetric;
    mpImplMetric->AddReference();
}

// -----------------------------------------------------------------------

FontInfo::~FontInfo()
{
    mpImplMetric->DeReference();
}

// -----------------------------------------------------------------------

FontInfo& FontInfo::operator=( const FontInfo& rInfo )
{
    Font::operator=( rInfo );

    if( mpImplMetric != rInfo.mpImplMetric )
    {
        mpImplMetric->DeReference();
        mpImplMetric = rInfo.mpImplMetric;
        mpImplMetric->AddReference();
    }

    return *this;
}

// -----------------------------------------------------------------------

BOOL FontInfo::operator==( const FontInfo& rInfo ) const
{
    if( !Font::operator==( rInfo ) )
        return FALSE;
    if( mpImplMetric == rInfo.mpImplMetric )
        return TRUE;
    if( *mpImplMetric == *rInfo.mpImplMetric  )
        return TRUE;
    return FALSE;
}

// -----------------------------------------------------------------------

FontType FontInfo::GetType() const
{
    return (mpImplMetric->IsScalable() ? TYPE_SCALABLE : TYPE_RASTER);
}

// -----------------------------------------------------------------------

BOOL FontInfo::IsDeviceFont() const
{
    return mpImplMetric->IsDeviceFont();
}

// -----------------------------------------------------------------------

BOOL FontInfo::SupportsLatin() const
{
    return mpImplMetric->SupportsLatin();
}

// -----------------------------------------------------------------------

BOOL FontInfo::SupportsCJK() const
{
    return mpImplMetric->SupportsCJK();
}

// -----------------------------------------------------------------------

BOOL FontInfo::SupportsCTL() const
{
    return mpImplMetric->SupportsCTL();
}

// =======================================================================

FontMetric::FontMetric( const FontMetric& rMetric )
:    FontInfo( rMetric )
{}

// -----------------------------------------------------------------------

long FontMetric::GetAscent() const
{
    return mpImplMetric->GetAscent();
}

// -----------------------------------------------------------------------

long FontMetric::GetDescent() const
{
    return mpImplMetric->GetDescent();
}

// -----------------------------------------------------------------------

long FontMetric::GetIntLeading() const
{
    return mpImplMetric->GetIntLeading();
}

// -----------------------------------------------------------------------

long FontMetric::GetExtLeading() const
{
    return mpImplMetric->GetExtLeading();
}

// -----------------------------------------------------------------------

long FontMetric::GetLineHeight() const
{
    return mpImplMetric->GetLineHeight();
}

// -----------------------------------------------------------------------

long FontMetric::GetSlant() const
{
    return mpImplMetric->GetSlant();
}

// -----------------------------------------------------------------------

FontMetric& FontMetric::operator =( const FontMetric& rMetric )
{
    FontInfo::operator=( rMetric );
    return *this;
}

// -----------------------------------------------------------------------

BOOL FontMetric::operator==( const FontMetric& rMetric ) const
{
    return FontInfo::operator==( rMetric );
}

// =======================================================================

ImplFontCharMap::ImplFontCharMap( int nRangePairs, const sal_uInt32* pRangeCodes )
:   mpRangeCodes( pRangeCodes ),
    mnRangeCount( nRangePairs ),
    mnCharCount( 0 ),
    mnRefCount( 1 )
{
    while( --nRangePairs >= 0 )
    {
        sal_uInt32 cFirst = *(pRangeCodes++);
        sal_uInt32 cLast  = *(pRangeCodes++);
        mnCharCount += cLast - cFirst;
    }
}

static ImplFontCharMap* pDefaultImplFontCharMap = NULL;
static const sal_uInt32 pDefaultRangeCodes[] = {0x0020,0xD800, 0xE000,0xFFF0};

// -----------------------------------------------------------------------

ImplFontCharMap::~ImplFontCharMap()
{
    if( mpRangeCodes != pDefaultRangeCodes )
        delete[] mpRangeCodes;
}

// -----------------------------------------------------------------------

ImplFontCharMap* ImplFontCharMap::GetDefaultMap()
{
    if( !pDefaultImplFontCharMap )
        pDefaultImplFontCharMap = new ImplFontCharMap( 2, pDefaultRangeCodes );
    pDefaultImplFontCharMap->AddReference();
    return pDefaultImplFontCharMap;
}

// -----------------------------------------------------------------------

bool ImplFontCharMap::IsDefaultMap() const
{
    return (mpRangeCodes == pDefaultRangeCodes);
}

// -----------------------------------------------------------------------

void ImplFontCharMap::AddReference()
{
    ++mnRefCount;
}

// -----------------------------------------------------------------------

void ImplFontCharMap::DeReference()
{
    if( --mnRefCount <= 0 )
        if( this != pDefaultImplFontCharMap )
            delete this;
}

// -----------------------------------------------------------------------

int ImplFontCharMap::GetCharCount() const
{
    return mnCharCount;
}

// -----------------------------------------------------------------------

int ImplFontCharMap::ImplFindRangeIndex( sal_uInt32 cChar ) const
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

bool ImplFontCharMap::HasChar( sal_uInt32 cChar ) const
{
    int nRange = ImplFindRangeIndex( cChar );
    if( nRange==0 && cChar<mpRangeCodes[0] )
        return false;
    return ((nRange & 1) == 0);
}

// -----------------------------------------------------------------------

// returns the number of chars supported by the font, which
// are inside the unicode range from cMin to cMax (inclusive)
int ImplFontCharMap::CountCharsInRange( sal_uInt32 cMin, sal_uInt32 cMax ) const
{
    int nCount = 0;

    // find and adjust range and char count for cMin
    int nRangeMin = ImplFindRangeIndex( cMin );
    if( (nRangeMin & 1) != 0 )
        ++nRangeMin;
    else if( cMin > mpRangeCodes[ nRangeMin ] )
        nCount -= cMin - mpRangeCodes[ nRangeMin ];

    // find and adjust range and char count for cMax
    int nRangeMax = ImplFindRangeIndex( cMax );
    if( (nRangeMin & 1) != 0 )
        --nRangeMax;
    else
        nCount -= mpRangeCodes[ nRangeMax+1 ] - cMax - 1;

    // count chars in complete ranges between cMin and cMax
    for( int i = nRangeMin; i <= nRangeMax; i+=2 )
        nCount += mpRangeCodes[i+1] - mpRangeCodes[i];

    return nCount;
}

// -----------------------------------------------------------------------

sal_uInt32 ImplFontCharMap::GetFirstChar() const
{
    return mpRangeCodes[0];
}

// -----------------------------------------------------------------------

sal_uInt32 ImplFontCharMap::GetLastChar() const
{
    return (mpRangeCodes[ 2*mnRangeCount-1 ] - 1);
}

// -----------------------------------------------------------------------

sal_uInt32 ImplFontCharMap::GetNextChar( sal_uInt32 cChar ) const
{
    if( cChar < GetFirstChar() )
        return GetFirstChar();
    if( cChar >= GetLastChar() )
        return GetLastChar();

    int nRange = ImplFindRangeIndex( cChar );
    if( nRange & 1 )                        // outside a range?
        return mpRangeCodes[ nRange + 1 ];  // => first in next range
    return (cChar + 1);
}

// -----------------------------------------------------------------------

sal_uInt32 ImplFontCharMap::GetPrevChar( sal_uInt32 cChar ) const
{
    if( cChar <= GetFirstChar() )
        return GetFirstChar();
    if( cChar > GetLastChar() )
        return GetLastChar();

    int nRange = ImplFindRangeIndex( cChar );
    if( nRange & 1 )                            // outside a range?
        return (mpRangeCodes[ nRange ] - 1);    // => last in prev range
    else if( cChar == mpRangeCodes[ nRange ] )  // first in prev range?
        return (mpRangeCodes[ nRange-1 ] - 1);  // => last in prev range
    return (cChar - 1);
}

// -----------------------------------------------------------------------

int ImplFontCharMap::GetIndexFromChar( sal_uInt32 cChar ) const
{
    // TODO: improve linear walk?
    int nCharIndex = 0;
    const sal_uInt32* pRange = &mpRangeCodes[0];
    for( int i = 0; i < mnRangeCount; ++i )
    {
        sal_uInt32 cFirst = *(pRange++);
        sal_uInt32 cLast  = *(pRange++);
        if( cChar >= cLast )
            break;
        if( cChar >= cFirst )
            return nCharIndex + (cChar - cFirst);
        nCharIndex += cLast - cFirst;
    }

    return -1;
}

// -----------------------------------------------------------------------

sal_uInt32 ImplFontCharMap::GetCharFromIndex( int nCharIndex ) const
{
    // TODO: improve linear walk?
    const sal_uInt32* pRange = &mpRangeCodes[0];
    for( int i = 0; i < mnRangeCount; ++i )
    {
        sal_uInt32 cFirst = *(pRange++);
        sal_uInt32 cLast  = *(pRange++);
        nCharIndex -= cLast - cFirst;
        if( nCharIndex < 0 )
            return (cLast + nCharIndex);
    }

    // we can only get here with an out-of-bounds charindex
    return mpRangeCodes[0];
}

// =======================================================================

FontCharMap::FontCharMap()
:   mpImpl( ImplFontCharMap::GetDefaultMap() )
{}

// -----------------------------------------------------------------------

FontCharMap::~FontCharMap()
{
    mpImpl->DeReference();
}

// -----------------------------------------------------------------------

int FontCharMap::GetCharCount() const
{
    return mpImpl->GetCharCount();
}

// -----------------------------------------------------------------------

int FontCharMap::CountCharsInRange( sal_uInt32 cMin, sal_uInt32 cMax ) const
{
    return mpImpl->CountCharsInRange( cMin, cMax );
}

// -----------------------------------------------------------------------

void FontCharMap::Reset( ImplFontCharMap* pNewMap )
{
    if( pNewMap == NULL )
    {
        mpImpl->DeReference();
        mpImpl = ImplFontCharMap::GetDefaultMap();
    }
    else if( pNewMap != mpImpl )
    {
        mpImpl->DeReference();
        mpImpl = pNewMap;
        mpImpl->AddReference();
    }
}

// -----------------------------------------------------------------------

BOOL FontCharMap::IsDefaultMap() const
{
    return mpImpl->IsDefaultMap();
}

// -----------------------------------------------------------------------

BOOL FontCharMap::HasChar( sal_uInt32 cChar ) const
{
    return mpImpl->HasChar( cChar );
}

// -----------------------------------------------------------------------

sal_uInt32 FontCharMap::GetFirstChar() const
{
    return mpImpl->GetFirstChar();
}

// -----------------------------------------------------------------------

sal_uInt32 FontCharMap::GetLastChar() const
{
    return mpImpl->GetLastChar();
}

// -----------------------------------------------------------------------

sal_uInt32 FontCharMap::GetNextChar( sal_uInt32 cChar ) const
{
    return mpImpl->GetNextChar( cChar );
}

// -----------------------------------------------------------------------

sal_uInt32 FontCharMap::GetPrevChar( sal_uInt32 cChar ) const
{
    return mpImpl->GetPrevChar( cChar );
}

// -----------------------------------------------------------------------

int FontCharMap::GetIndexFromChar( sal_uInt32 cChar ) const
{
    return mpImpl->GetIndexFromChar( cChar );
}

// -----------------------------------------------------------------------

sal_uInt32 FontCharMap::GetCharFromIndex( int nIndex ) const
{
    return mpImpl->GetCharFromIndex( nIndex );
}

// =======================================================================
