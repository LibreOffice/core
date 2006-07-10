/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xistring.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 13:42:59 $
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

#ifndef SC_XISTRING_HXX
#include "xistring.hxx"
#endif

#ifndef SC_XLSTYLE_HXX
#include "xlstyle.hxx"
#endif
#ifndef SC_XISTREAM_HXX
#include "xistream.hxx"
#endif
#ifndef SC_XIROOT_HXX
#include "xiroot.hxx"
#endif

// Byte/Unicode strings =======================================================

/** All allowed flags for import. */
const XclStrFlags nAllowedFlags = EXC_STR_8BITLENGTH | EXC_STR_SMARTFLAGS | EXC_STR_KEEPFORMATS;

// ----------------------------------------------------------------------------

XclImpString::XclImpString()
{
}

XclImpString::XclImpString( const String& rString ) :
    maString( rString )
{
}

XclImpString::~XclImpString()
{
}

void XclImpString::Read( XclImpStream& rStrm, XclStrFlags nFlags )
{
    if( !::get_flag( nFlags, EXC_STR_KEEPFORMATS ) )
        maFormats.clear();

    DBG_ASSERT( (nFlags & ~nAllowedFlags) == 0, "XclImpString::Read - unknown flag" );
    bool b16BitLen = !::get_flag( nFlags, EXC_STR_8BITLENGTH );

    switch( rStrm.GetRoot().GetBiff() )
    {
        case EXC_BIFF2:
        case EXC_BIFF3:
        case EXC_BIFF4:
        case EXC_BIFF5:
            // no integrated formatting in BIFF2-BIFF7
            maString = rStrm.ReadByteString( b16BitLen );
        break;

        case EXC_BIFF8:
        {
            // --- string header ---
            sal_uInt16 nChars = b16BitLen ? rStrm.ReaduInt16() : rStrm.ReaduInt8();
            sal_uInt8 nFlagField = 0;
            if( nChars || !::get_flag( nFlags, EXC_STR_SMARTFLAGS ) )
                rStrm >> nFlagField;

            bool b16Bit, bRich, bFarEast;
            sal_uInt16 nRunCount;
            sal_uInt32 nExtInf;
            rStrm.ReadUniStringExtHeader( b16Bit, bRich, bFarEast, nRunCount, nExtInf, nFlagField );
            // #122185# ignore the flags, they may be wrong

            // --- character array ---
            maString = rStrm.ReadRawUniString( nChars, b16Bit );

            // --- formatting ---
            if( nRunCount > 0 )
                ReadFormats( rStrm, nRunCount );

            // --- extended (FarEast) information ---
            rStrm.Ignore( nExtInf );
        }
        break;

        default:
            DBG_ERROR_BIFF();
    }
}

void XclImpString::AppendFormat( sal_uInt16 nChar, sal_uInt16 nFontIdx )
{
    // #i33341# real life -- same character index may occur several times
    DBG_ASSERT( maFormats.empty() || (maFormats.back().mnChar <= nChar), "XclImpString::AppendFormat - wrong char order" );
    if( maFormats.empty() || (maFormats.back().mnChar < nChar) )
        maFormats.push_back( XclFormatRun( nChar, nFontIdx ) );
    else
        maFormats.back().mnFontIdx = nFontIdx;
}

void XclImpString::ReadFormats( XclImpStream& rStrm )
{
    bool bBiff8 = rStrm.GetRoot().GetBiff() == EXC_BIFF8;
    sal_uInt16 nCount = bBiff8 ? rStrm.ReaduInt16() : rStrm.ReaduInt8();
    ReadFormats( rStrm, nCount );
}

void XclImpString::ReadFormats( XclImpStream& rStrm, sal_uInt16 nRunCount )
{
    maFormats.clear();
    maFormats.reserve( nRunCount );
    /*  #i33341# real life -- same character index may occur several times
        -> use AppendFormat() to validate formats */
    switch( rStrm.GetRoot().GetBiff() )
    {
        case EXC_BIFF2:
        case EXC_BIFF3:
        case EXC_BIFF4:
        case EXC_BIFF5:
            for( sal_uInt16 nIdx = 0; nIdx < nRunCount; ++nIdx )
            {
                sal_uInt8 nChar, nFontIdx;
                rStrm >> nChar >> nFontIdx;
                AppendFormat( nChar, nFontIdx );
            }
        break;
        case EXC_BIFF8:
            for( sal_uInt16 nIdx = 0; nIdx < nRunCount; ++nIdx )
            {
                sal_uInt16 nChar, nFontIdx;
                rStrm >> nChar >> nFontIdx;
                AppendFormat( nChar, nFontIdx );
            }
        break;
        default:
            DBG_ERROR_BIFF();
    }
}

// String iterator ============================================================

XclImpStringIterator::XclImpStringIterator( const XclImpString& rString ) :
    mrText( rString.GetText() ),
    mrFormats( rString.GetFormats() ),
    mnPortion( 0 ),
    mnTextBeg( 0 ),
    mnTextEnd( 0 ),
    mnFormatsBeg( 0 ),
    mnFormatsEnd( 0 )
{
    // first portion is formatted, adjust vector index to next portion
    if( !mrFormats.empty() && (mrFormats.front().mnChar == 0) )
        ++mnFormatsEnd;
    // find end position of the first portion
    mnTextEnd = static_cast< xub_StrLen >( (mnFormatsEnd < mrFormats.size()) ?
        mrFormats[ mnFormatsEnd ].mnChar : mrText.Len() );
}

String XclImpStringIterator::GetPortionText() const
{
    return String( mrText, mnTextBeg, mnTextEnd - mnTextBeg );
}

sal_uInt16 XclImpStringIterator::GetPortionFont() const
{
    return (mnFormatsBeg < mnFormatsEnd) ? mrFormats[ mnFormatsBeg ].mnFontIdx : EXC_FONT_NOTFOUND;
}

XclImpStringIterator& XclImpStringIterator::operator++()
{
    if( Is() )
    {
        ++mnPortion;
        do
        {
            // indexes into vector of formatting runs
            if( mnFormatsBeg < mnFormatsEnd )
                ++mnFormatsBeg;
            if( mnFormatsEnd < mrFormats.size() )
                ++mnFormatsEnd;
            // character positions of next portion
            mnTextBeg = mnTextEnd;
            mnTextEnd = static_cast< xub_StrLen >( (mnFormatsEnd < mrFormats.size()) ?
                mrFormats[ mnFormatsEnd ].mnChar : mrText.Len() );
        }
        while( Is() && (mnTextBeg == mnTextEnd) );
    }
    return *this;
}

// ============================================================================

