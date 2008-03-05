/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: richstring.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 19:04:57 $
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

#include "oox/xls/richstring.hxx"
#include <com/sun/star/text/XText.hpp>
#include "oox/helper/attributelist.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/helper/recordinputstream.hxx"
#include "oox/xls/biffinputstream.hxx"

using ::rtl::OString;
using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::text::XText;
using ::com::sun::star::text::XTextRange;

namespace oox {
namespace xls {

// ============================================================================

namespace {

const sal_uInt8 OOBIN_STRINGFLAG_FONTS          = 0x01;
const sal_uInt8 OOBIN_STRINGFLAG_PHONETICS      = 0x02;

} // namespace

// ============================================================================

RichStringPortion::RichStringPortion( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    mnFontId( -1 )
{
}

void RichStringPortion::setText( const OUString& rText )
{
    maText = rText;
}

FontRef RichStringPortion::importFont( const AttributeList& )
{
    mxFont.reset( new Font( *this, false ) );
    return mxFont;
}

void RichStringPortion::setFontId( sal_Int32 nFontId )
{
    mnFontId = nFontId;
}

void RichStringPortion::finalizeImport()
{
    if( mxFont.get() )
        mxFont->finalizeImport();
    else if( mnFontId >= 0 )
        mxFont = getStyles().getFont( mnFontId );
}

void RichStringPortion::convert( const Reference< XText >& rxText, sal_Int32 nXfId )
{
    Reference< XTextRange > xRange = rxText->getEnd();
    xRange->setString( maText );
    if( mxFont.get() )
    {
        PropertySet aPropSet( xRange );
        mxFont->writeToPropertySet( aPropSet, FONT_PROPTYPE_RICHTEXT );
    }
    if( const Font* pFont = getStyles().getFontFromCellXf( nXfId ).get() )
    {
        if( pFont->needsRichTextFormat() )
        {
            PropertySet aPropSet( xRange );
            pFont->writeToPropertySet( aPropSet, FONT_PROPTYPE_RICHTEXT );
        }
    }
}

// ----------------------------------------------------------------------------

void BinFontPortionData::read( RecordInputStream& rStrm )
{
    mnPos = rStrm.readuInt16();
    mnFontId = rStrm.readuInt16();
}

void BinFontPortionData::read( BiffInputStream& rStrm, bool b16Bit )
{
    if( b16Bit )
    {
        mnPos = rStrm.readuInt16();
        mnFontId = rStrm.readuInt16();
    }
    else
    {
        mnPos = rStrm.readuInt8();
        mnFontId = rStrm.readuInt8();
    }
}

// ----------------------------------------------------------------------------

void BinFontPortionList::appendPortion( const BinFontPortionData& rPortion )
{
    // #i33341# real life -- same character index may occur several times
    OSL_ENSURE( empty() || (back().mnPos <= rPortion.mnPos), "BinFontPortionList::appendPortion - wrong char order" );
    if( empty() || (back().mnPos < rPortion.mnPos) )
        push_back( rPortion );
    else
        back().mnFontId = rPortion.mnFontId;
}

void BinFontPortionList::importPortions( RecordInputStream& rStrm )
{
    sal_Int32 nCount = rStrm.readInt32();
    clear();
    if( nCount > 0 )
    {
        reserve( getLimitedValue< size_t, sal_Int32 >( nCount, 0, rStrm.getRecLeft() / 4 ) );
        /*  #i33341# real life -- same character index may occur several times
            -> use appendPortion() to validate string position. */
        BinFontPortionData aPortion;
        for( sal_Int32 nIndex = 0; rStrm.isValid() && (nIndex < nCount); ++nIndex )
        {
            aPortion.read( rStrm );
            appendPortion( aPortion );
        }
    }
}

void BinFontPortionList::importPortions( BiffInputStream& rStrm, sal_uInt16 nCount, bool b16Bit )
{
    clear();
    reserve( nCount );
    /*  #i33341# real life -- same character index may occur several times
        -> use appendPortion() to validate string position. */
    BinFontPortionData aPortion;
    for( sal_uInt16 nIndex = 0; rStrm.isValid() && (nIndex < nCount); ++nIndex )
    {
        aPortion.read( rStrm, b16Bit );
        appendPortion( aPortion );
    }
}

void BinFontPortionList::importPortions( BiffInputStream& rStrm, bool b16Bit )
{
    sal_uInt16 nCount = b16Bit ? rStrm.readuInt16() : rStrm.readuInt8();
    importPortions( rStrm, nCount, b16Bit );
}

// ============================================================================

OoxPhoneticData::OoxPhoneticData() :
    mnFontId( -1 ),
    mnType( XML_fullwidthKatakana ),
    mnAlignment( XML_left )
{
}

void OoxPhoneticData::setBinData( sal_Int32 nType, sal_Int32 nAlignment )
{
    static const sal_Int32 spnTypeIds[] = { XML_halfwidthKatakana, XML_fullwidthKatakana, XML_hiragana, XML_noConversion };
    mnType = STATIC_ARRAY_SELECT( spnTypeIds, nType, XML_fullwidthKatakana );

    static const sal_Int32 spnAlignments[] = { XML_noControl, XML_left, XML_center, XML_distributed };
    mnAlignment = STATIC_ARRAY_SELECT( spnAlignments, nAlignment, XML_left );
}

// ----------------------------------------------------------------------------

PhoneticSettings::PhoneticSettings( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

void PhoneticSettings::importPhoneticPr( const AttributeList& rAttribs )
{
    maOoxData.mnFontId    = rAttribs.getInteger( XML_fontId, -1 );
    maOoxData.mnType      = rAttribs.getToken( XML_type, XML_fullwidthKatakana );
    maOoxData.mnAlignment = rAttribs.getToken( XML_alignment, XML_left );
}

void PhoneticSettings::importPhoneticPr( RecordInputStream& rStrm )
{
    sal_uInt16 nFontId;
    sal_Int32 nType, nAlignment;
    rStrm >> nFontId >> nType >> nAlignment;
    maOoxData.mnFontId = nFontId;
    maOoxData.setBinData( nType, nAlignment );
}

void PhoneticSettings::importPhoneticPr( BiffInputStream& rStrm )
{
    sal_uInt16 nFontId, nFlags;
    rStrm >> nFontId >> nFlags;
    maOoxData.mnFontId = nFontId;
    maOoxData.setBinData( extractValue< sal_Int32 >( nFlags, 0, 2 ), extractValue< sal_Int32 >( nFlags, 2, 2 ) );
    // following: range list with cells showing phonetic text
}

void PhoneticSettings::importStringData( RecordInputStream& rStrm )
{
    sal_uInt16 nFontId, nFlags;
    rStrm >> nFontId >> nFlags;
    maOoxData.mnFontId = nFontId;
    maOoxData.setBinData( extractValue< sal_Int32 >( nFlags, 0, 2 ), extractValue< sal_Int32 >( nFlags, 2, 2 ) );
}

void PhoneticSettings::importStringData( BiffInputStream& rStrm )
{
    sal_uInt16 nFontId, nFlags;
    rStrm >> nFontId >> nFlags;
    maOoxData.mnFontId = nFontId;
    maOoxData.setBinData( extractValue< sal_Int32 >( nFlags, 0, 2 ), extractValue< sal_Int32 >( nFlags, 2, 2 ) );
}

// ============================================================================

RichStringPhonetic::RichStringPhonetic( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    mnBasePos( -1 ),
    mnBaseEnd( -1 )
{
}

void RichStringPhonetic::setText( const OUString& rText )
{
    maText = rText;
}

void RichStringPhonetic::importPhoneticRun( const AttributeList& rAttribs )
{
    mnBasePos = rAttribs.getInteger( XML_sb, -1 );
    mnBaseEnd = rAttribs.getInteger( XML_eb, -1 );
}

void RichStringPhonetic::setBaseRange( sal_Int32 nBasePos, sal_Int32 nBaseEnd )
{
    mnBasePos = nBasePos;
    mnBaseEnd = nBaseEnd;
}

// ----------------------------------------------------------------------------

void BinPhoneticPortionData::read( RecordInputStream& rStrm )
{
    mnPos = rStrm.readuInt16();
    mnBasePos = rStrm.readuInt16();
    mnBaseLen = rStrm.readuInt16();
}

void BinPhoneticPortionData::read( BiffInputStream& rStrm )
{
    mnPos = rStrm.readuInt16();
    mnBasePos = rStrm.readuInt16();
    mnBaseLen = rStrm.readuInt16();
}

// ----------------------------------------------------------------------------

void BinPhoneticPortionList::appendPortion( const BinPhoneticPortionData& rPortion )
{
    // same character index may occur several times
    OSL_ENSURE( empty() || ((back().mnPos <= rPortion.mnPos) &&
        (back().mnBasePos + back().mnBaseLen <= rPortion.mnBasePos)),
        "BinPhoneticPortionList::appendPortion - wrong char order" );
    if( empty() || (back().mnPos < rPortion.mnPos) )
    {
        push_back( rPortion );
    }
    else if( back().mnPos == rPortion.mnPos )
    {
        back().mnBasePos = rPortion.mnBasePos;
        back().mnBaseLen = rPortion.mnBaseLen;
    }
}

void BinPhoneticPortionList::importPortions( RecordInputStream& rStrm )
{
    sal_Int32 nCount = rStrm.readInt32();
    clear();
    if( nCount > 0 )
    {
        reserve( getLimitedValue< size_t, sal_Int32 >( nCount, 0, rStrm.getRecLeft() / 6 ) );
        BinPhoneticPortionData aPortion;
        for( sal_Int32 nIndex = 0; rStrm.isValid() && (nIndex < nCount); ++nIndex )
        {
            aPortion.read( rStrm );
            appendPortion( aPortion );
        }
    }
}

OUString BinPhoneticPortionList::importPortions( BiffInputStream& rStrm, sal_uInt32 nPhoneticSize )
{
    OUString aPhoneticText;
    sal_uInt16 nPortionCount, nTextLen1, nTextLen2;
    rStrm >> nPortionCount >> nTextLen1 >> nTextLen2;
    OSL_ENSURE( nTextLen1 == nTextLen2, "BinPhoneticPortionList::importPortions - wrong phonetic text length" );
    if( (nTextLen1 == nTextLen2) && (nTextLen1 > 0) )
    {
        sal_uInt32 nMinSize = static_cast< sal_uInt32 >( 2 * nTextLen1 + 6 * nPortionCount + 14 );
        OSL_ENSURE( nMinSize <= nPhoneticSize, "BinPhoneticPortionList::importPortions - wrong size of phonetic data" );
        if( nMinSize <= nPhoneticSize )
        {
            aPhoneticText = rStrm.readUnicodeArray( nTextLen1 );
            clear();
            reserve( nPortionCount );
            BinPhoneticPortionData aPortion;
            for( sal_uInt16 nPortion = 0; nPortion < nPortionCount; ++nPortion )
            {
                aPortion.read( rStrm );
                appendPortion( aPortion );
            }
        }
    }
    return aPhoneticText;
}

// ============================================================================

RichString::RichString( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    maPhonSettings( rHelper )
{
}

RichStringPortionRef RichString::importText( const AttributeList& )
{
    return createPortion();
}

RichStringPortionRef RichString::importRun( const AttributeList& )
{
    return createPortion();
}

RichStringPhoneticRef RichString::importPhoneticRun( const AttributeList& rAttribs )
{
    RichStringPhoneticRef xPhonetic = createPhonetic();
    xPhonetic->importPhoneticRun( rAttribs );
    return xPhonetic;
}

void RichString::importPhoneticPr( const AttributeList& rAttribs )
{
    maPhonSettings.importPhoneticPr( rAttribs );
}

void RichString::importString( RecordInputStream& rStrm, bool bRich )
{
    sal_uInt8 nFlags = bRich ? rStrm.readuInt8() : 0;
    OUString aBaseText = rStrm.readString();

    if( rStrm.isValid() && getFlag( nFlags, OOBIN_STRINGFLAG_FONTS ) )
    {
        BinFontPortionList aPortions;
        aPortions.importPortions( rStrm );
        createFontPortions( aBaseText, aPortions );
    }
    else
    {
        createPortion()->setText( aBaseText );
    }

    if( rStrm.isValid() && getFlag( nFlags, OOBIN_STRINGFLAG_PHONETICS ) )
    {
        OUString aPhoneticText = rStrm.readString();
        BinPhoneticPortionList aPortions;
        aPortions.importPortions( rStrm );
        maPhonSettings.importStringData( rStrm );
        createPhoneticPortions( aPhoneticText, aPortions, aBaseText.getLength() );
    }
}

void RichString::importByteString( BiffInputStream& rStrm, rtl_TextEncoding eDefaultTextEnc, BiffStringFlags nFlags )
{
    OSL_ENSURE( !getFlag( nFlags, BIFF_STR_KEEPFONTS ), "RichString::importString - keep fonts not implemented" );
    OSL_ENSURE( !getFlag( nFlags, static_cast< BiffStringFlags >( ~(BIFF_STR_8BITLENGTH | BIFF_STR_EXTRAFONTS) ) ), "RichString::importByteString - unknown flag" );
    bool b8BitLength = getFlag( nFlags, BIFF_STR_8BITLENGTH );

    OString aBaseText = rStrm.readByteString( !b8BitLength );

    if( rStrm.isValid() && getFlag( nFlags, BIFF_STR_EXTRAFONTS ) )
    {
        BinFontPortionList aPortions;
        aPortions.importPortions( rStrm, false );
        createFontPortions( aBaseText, eDefaultTextEnc, aPortions );
    }
    else
    {
        createPortion()->setText( OStringToOUString( aBaseText, eDefaultTextEnc ) );
    }
}

void RichString::importUniString( BiffInputStream& rStrm, BiffStringFlags nFlags )
{
    OSL_ENSURE( !getFlag( nFlags, BIFF_STR_KEEPFONTS ), "RichString::importUniString - keep fonts not implemented" );
    OSL_ENSURE( !getFlag( nFlags, static_cast< BiffStringFlags >( ~(BIFF_STR_8BITLENGTH | BIFF_STR_SMARTFLAGS) ) ), "RichString::importUniString - unknown flag" );
    bool b8BitLength = getFlag( nFlags, BIFF_STR_8BITLENGTH );

    // --- string header ---
    sal_uInt16 nChars = b8BitLength ? rStrm.readuInt8() : rStrm.readuInt16();
    sal_uInt8 nFlagField = 0;
    if( (nChars > 0) || !getFlag( nFlags, BIFF_STR_SMARTFLAGS ) )
        rStrm >> nFlagField;
    bool b16Bit    = getFlag( nFlagField, BIFF_STRF_16BIT );
    bool bFonts    = getFlag( nFlagField, BIFF_STRF_RICH );
    bool bPhonetic = getFlag( nFlagField, BIFF_STRF_PHONETIC );
    sal_uInt16 nFontCount = bFonts ? rStrm.readuInt16() : 0;
    sal_uInt32 nPhoneticSize = bPhonetic ? rStrm.readuInt32() : 0;

    // --- character array ---
    OUString aBaseText = rStrm.readUniStringChars( nChars, b16Bit );

    // --- formatting ---
    // #122185# bRich flag may be set, but format runs may be missing
    if( rStrm.isValid() && (nFontCount > 0) )
    {
        BinFontPortionList aPortions;
        aPortions.importPortions( rStrm, nFontCount, true );
        createFontPortions( aBaseText, aPortions );
    }
    else
    {
        createPortion()->setText( aBaseText );
    }

    // --- Asian phonetic information ---
    // #122185# bPhonetic flag may be set, but phonetic info may be missing
    if( rStrm.isValid() && (nPhoneticSize > 0) )
    {
        sal_uInt32 nPhoneticEnd = rStrm.getRecPos() + nPhoneticSize;
        OSL_ENSURE( nPhoneticSize > 14, "RichString::importUniString - wrong size of phonetic data" );
        if( nPhoneticSize > 14 )
        {
            sal_uInt16 nId, nSize;
            rStrm >> nId >> nSize;
            OSL_ENSURE( nId == 1, "RichString::importUniString - unknown phonetic data identifier" );
            sal_uInt32 nMinSize = static_cast< sal_uInt32 >( nSize + 4 );
            OSL_ENSURE( nMinSize <= nPhoneticSize, "RichString::importUniString - wrong size of phonetic data" );
            if( (nId == 1) && (nMinSize <= nPhoneticSize) )
            {
                maPhonSettings.importStringData( rStrm );
                BinPhoneticPortionList aPortions;
                OUString aPhoneticText = aPortions.importPortions( rStrm, nPhoneticSize );
                createPhoneticPortions( aPhoneticText, aPortions, aBaseText.getLength() );
            }
        }
        rStrm.seek( nPhoneticEnd );
    }
}

void RichString::finalizeImport()
{
    maFontPortions.forEachMem( &RichStringPortion::finalizeImport );
}

void RichString::convert( const Reference< XText >& rxText, sal_Int32 nXfId ) const
{
    for( PortionVec::const_iterator aIt = maFontPortions.begin(), aEnd = maFontPortions.end(); aIt != aEnd; ++aIt )
    {
        (*aIt)->convert( rxText, nXfId );
        nXfId = -1;
    }
}

// private --------------------------------------------------------------------

RichStringPortionRef RichString::createPortion()
{
    RichStringPortionRef xPortion( new RichStringPortion( *this ) );
    maFontPortions.push_back( xPortion );
    return xPortion;
}

RichStringPhoneticRef RichString::createPhonetic()
{
    RichStringPhoneticRef xPhonetic( new RichStringPhonetic( *this ) );
    maPhonPortions.push_back( xPhonetic );
    return xPhonetic;
}

void RichString::createFontPortions( const OString& rText, rtl_TextEncoding eDefaultTextEnc, BinFontPortionList& rPortions )
{
    maFontPortions.clear();
    sal_Int32 nStrLen = rText.getLength();
    if( nStrLen > 0 )
    {
        // add leading and trailing string position to ease the following loop
        if( rPortions.empty() || (rPortions.front().mnPos > 0) )
            rPortions.insert( rPortions.begin(), BinFontPortionData( 0, -1 ) );
        if( rPortions.back().mnPos < nStrLen )
            rPortions.push_back( BinFontPortionData( nStrLen, -1 ) );

        // create all string portions according to the font id vector
        for( BinFontPortionList::const_iterator aIt = rPortions.begin(); aIt->mnPos < nStrLen; ++aIt )
        {
            sal_Int32 nPortionLen = (aIt + 1)->mnPos - aIt->mnPos;
            if( (0 < nPortionLen) && (aIt->mnPos + nPortionLen <= nStrLen) )
            {
                // convert byte string to unicode string, using current font encoding
                FontRef xFont = getStyles().getFont( aIt->mnFontId );
                rtl_TextEncoding eTextEnc = xFont.get() ? xFont->getFontEncoding() : eDefaultTextEnc;
                OUString aUniStr = OStringToOUString( rText.copy( aIt->mnPos, nPortionLen ), eTextEnc );
                // create string portion
                RichStringPortionRef xPortion = createPortion();
                xPortion->setText( aUniStr );
                xPortion->setFontId( aIt->mnFontId );
            }
        }
    }
}

void RichString::createFontPortions( const OUString& rText, BinFontPortionList& rPortions )
{
    maFontPortions.clear();
    sal_Int32 nStrLen = rText.getLength();
    if( nStrLen > 0 )
    {
        // add leading and trailing string position to ease the following loop
        if( rPortions.empty() || (rPortions.front().mnPos > 0) )
            rPortions.insert( rPortions.begin(), BinFontPortionData( 0, -1 ) );
        if( rPortions.back().mnPos < nStrLen )
            rPortions.push_back( BinFontPortionData( nStrLen, -1 ) );

        // create all string portions according to the font id vector
        for( BinFontPortionList::const_iterator aIt = rPortions.begin(); aIt->mnPos < nStrLen; ++aIt )
        {
            sal_Int32 nPortionLen = (aIt + 1)->mnPos - aIt->mnPos;
            if( (0 < nPortionLen) && (aIt->mnPos + nPortionLen <= nStrLen) )
            {
                RichStringPortionRef xPortion = createPortion();
                xPortion->setText( rText.copy( aIt->mnPos, nPortionLen ) );
                xPortion->setFontId( aIt->mnFontId );
            }
        }
    }
}

void RichString::createPhoneticPortions( const ::rtl::OUString& rText, BinPhoneticPortionList& rPortions, sal_Int32 nBaseLen )
{
    maPhonPortions.clear();
    sal_Int32 nStrLen = rText.getLength();
    if( nStrLen > 0 )
    {
        // no portions - assign phonetic text to entire base text
        if( rPortions.empty() )
            rPortions.push_back( BinPhoneticPortionData( 0, 0, nBaseLen ) );
        // add trailing string position to ease the following loop
        if( rPortions.back().mnPos < nStrLen )
            rPortions.push_back( BinPhoneticPortionData( nStrLen, nBaseLen, 0 ) );

        // create all phonetic portions according to the portions vector
        for( BinPhoneticPortionList::const_iterator aIt = rPortions.begin(); aIt->mnPos < nStrLen; ++aIt )
        {
            sal_Int32 nPortionLen = (aIt + 1)->mnPos - aIt->mnPos;
            if( (0 < nPortionLen) && (aIt->mnPos + nPortionLen <= nStrLen) )
            {
                RichStringPhoneticRef xPhonetic = createPhonetic();
                xPhonetic->setText( rText.copy( aIt->mnPos, nPortionLen ) );
                xPhonetic->setBaseRange( aIt->mnBasePos, aIt->mnBasePos + aIt->mnBaseLen );
            }
        }
    }
}

// ============================================================================

} // namespace xls
} // namespace oox

