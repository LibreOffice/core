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

#include "oox/xls/richstring.hxx"

#include <com/sun/star/text/XText.hpp>
#include <rtl/ustrbuf.hxx>
#include "oox/helper/attributelist.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/xls/biffinputstream.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;

using ::rtl::OString;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

// ============================================================================

namespace {

const sal_uInt8 BIFF12_STRINGFLAG_FONTS         = 0x01;
const sal_uInt8 BIFF12_STRINGFLAG_PHONETICS     = 0x02;

inline bool lclNeedsRichTextFormat( const Font* pFont )
{
    return pFont && pFont->needsRichTextFormat();
}

} // namespace

// ============================================================================

RichStringPortion::RichStringPortion( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    mnFontId( -1 ),
    mbConverted( false )
{
}

void RichStringPortion::setText( const OUString& rText )
{
    maText = rText;
}

FontRef RichStringPortion::createFont()
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

void RichStringPortion::convert( const Reference< XText >& rxText, const Font* pFont, bool bReplace )
{
    if ( mbConverted )
        return;

    Reference< XTextRange > xRange;
    if( bReplace )
        xRange.set( rxText, UNO_QUERY );
    else
        xRange = rxText->getEnd();
    OSL_ENSURE( xRange.is(), "RichStringPortion::convert - cannot get text range interface" );

    if( xRange.is() )
    {
        xRange->setString( maText );
        if( mxFont.get() )
        {
            PropertySet aPropSet( xRange );
            mxFont->writeToPropertySet( aPropSet, FONT_PROPTYPE_TEXT );
        }
        /*  Some font attributes cannot be set to cell formatting in Calc but
            require to use rich formatting, e.g. font escapement. But do not
            use the passed font if this portion has its own font. */
        else if( lclNeedsRichTextFormat( pFont ) )
        {
            PropertySet aPropSet( xRange );
            pFont->writeToPropertySet( aPropSet, FONT_PROPTYPE_TEXT );
        }
    }

    mbConverted = true;
}

// ----------------------------------------------------------------------------

void FontPortionModel::read( SequenceInputStream& rStrm )
{
    mnPos = rStrm.readuInt16();
    mnFontId = rStrm.readuInt16();
}

void FontPortionModel::read( BiffInputStream& rStrm, BiffFontPortionMode eMode )
{
    switch( eMode )
    {
        case BIFF_FONTPORTION_8BIT:
            mnPos = rStrm.readuInt8();
            mnFontId = rStrm.readuInt8();
        break;
        case BIFF_FONTPORTION_16BIT:
            mnPos = rStrm.readuInt16();
            mnFontId = rStrm.readuInt16();
        break;
        case BIFF_FONTPORTION_OBJ:
            mnPos = rStrm.readuInt16();
            mnFontId = rStrm.readuInt16();
            rStrm.skip( 4 );
        break;
    }
}

// ----------------------------------------------------------------------------

void FontPortionModelList::appendPortion( const FontPortionModel& rPortion )
{
    // #i33341# real life -- same character index may occur several times
    OSL_ENSURE( empty() || (back().mnPos <= rPortion.mnPos), "FontPortionModelList::appendPortion - wrong char order" );
    if( empty() || (back().mnPos < rPortion.mnPos) )
        push_back( rPortion );
    else
        back().mnFontId = rPortion.mnFontId;
}

void FontPortionModelList::importPortions( SequenceInputStream& rStrm )
{
    sal_Int32 nCount = rStrm.readInt32();
    clear();
    if( nCount > 0 )
    {
        reserve( getLimitedValue< size_t, sal_Int64 >( nCount, 0, rStrm.getRemaining() / 4 ) );
        /*  #i33341# real life -- same character index may occur several times
            -> use appendPortion() to validate string position. */
        FontPortionModel aPortion;
        for( sal_Int32 nIndex = 0; !rStrm.isEof() && (nIndex < nCount); ++nIndex )
        {
            aPortion.read( rStrm );
            appendPortion( aPortion );
        }
    }
}

void FontPortionModelList::importPortions( BiffInputStream& rStrm, sal_uInt16 nCount, BiffFontPortionMode eMode )
{
    clear();
    reserve( nCount );
    /*  #i33341# real life -- same character index may occur several times
        -> use appendPortion() to validate string position. */
    FontPortionModel aPortion;
    for( sal_uInt16 nIndex = 0; !rStrm.isEof() && (nIndex < nCount); ++nIndex )
    {
        aPortion.read( rStrm, eMode );
        appendPortion( aPortion );
    }
}

void FontPortionModelList::importPortions( BiffInputStream& rStrm, bool b16Bit )
{
    sal_uInt16 nCount = b16Bit ? rStrm.readuInt16() : rStrm.readuInt8();
    importPortions( rStrm, nCount, b16Bit ? BIFF_FONTPORTION_16BIT : BIFF_FONTPORTION_8BIT );
}

// ============================================================================

PhoneticDataModel::PhoneticDataModel() :
    mnFontId( -1 ),
    mnType( XML_fullwidthKatakana ),
    mnAlignment( XML_left )
{
}

void PhoneticDataModel::setBiffData( sal_Int32 nType, sal_Int32 nAlignment )
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
    maModel.mnFontId    = rAttribs.getInteger( XML_fontId, -1 );
    maModel.mnType      = rAttribs.getToken( XML_type, XML_fullwidthKatakana );
    maModel.mnAlignment = rAttribs.getToken( XML_alignment, XML_left );
}

void PhoneticSettings::importPhoneticPr( SequenceInputStream& rStrm )
{
    sal_uInt16 nFontId;
    sal_Int32 nType, nAlignment;
    rStrm >> nFontId >> nType >> nAlignment;
    maModel.mnFontId = nFontId;
    maModel.setBiffData( nType, nAlignment );
}

void PhoneticSettings::importPhoneticPr( BiffInputStream& rStrm )
{
    sal_uInt16 nFontId, nFlags;
    rStrm >> nFontId >> nFlags;
    maModel.mnFontId = nFontId;
    maModel.setBiffData( extractValue< sal_Int32 >( nFlags, 0, 2 ), extractValue< sal_Int32 >( nFlags, 2, 2 ) );
    // following: range list with cells showing phonetic text
}

void PhoneticSettings::importStringData( SequenceInputStream& rStrm )
{
    sal_uInt16 nFontId, nFlags;
    rStrm >> nFontId >> nFlags;
    maModel.mnFontId = nFontId;
    maModel.setBiffData( extractValue< sal_Int32 >( nFlags, 0, 2 ), extractValue< sal_Int32 >( nFlags, 2, 2 ) );
}

void PhoneticSettings::importStringData( BiffInputStream& rStrm )
{
    sal_uInt16 nFontId, nFlags;
    rStrm >> nFontId >> nFlags;
    maModel.mnFontId = nFontId;
    maModel.setBiffData( extractValue< sal_Int32 >( nFlags, 0, 2 ), extractValue< sal_Int32 >( nFlags, 2, 2 ) );
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

void PhoneticPortionModel::read( SequenceInputStream& rStrm )
{
    mnPos = rStrm.readuInt16();
    mnBasePos = rStrm.readuInt16();
    mnBaseLen = rStrm.readuInt16();
}

void PhoneticPortionModel::read( BiffInputStream& rStrm )
{
    mnPos = rStrm.readuInt16();
    mnBasePos = rStrm.readuInt16();
    mnBaseLen = rStrm.readuInt16();
}

// ----------------------------------------------------------------------------

void PhoneticPortionModelList::appendPortion( const PhoneticPortionModel& rPortion )
{
    // same character index may occur several times
    OSL_ENSURE( empty() || ((back().mnPos <= rPortion.mnPos) &&
        (back().mnBasePos + back().mnBaseLen <= rPortion.mnBasePos)),
        "PhoneticPortionModelList::appendPortion - wrong char order" );
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

void PhoneticPortionModelList::importPortions( SequenceInputStream& rStrm )
{
    sal_Int32 nCount = rStrm.readInt32();
    clear();
    if( nCount > 0 )
    {
        reserve( getLimitedValue< size_t, sal_Int64 >( nCount, 0, rStrm.getRemaining() / 6 ) );
        PhoneticPortionModel aPortion;
        for( sal_Int32 nIndex = 0; !rStrm.isEof() && (nIndex < nCount); ++nIndex )
        {
            aPortion.read( rStrm );
            appendPortion( aPortion );
        }
    }
}

OUString PhoneticPortionModelList::importPortions( BiffInputStream& rStrm, sal_Int32 nPhoneticSize )
{
    OUString aPhoneticText;
    sal_uInt16 nPortionCount, nTextLen1, nTextLen2;
    rStrm >> nPortionCount >> nTextLen1 >> nTextLen2;
    OSL_ENSURE( nTextLen1 == nTextLen2, "PhoneticPortionModelList::importPortions - wrong phonetic text length" );
    if( (nTextLen1 == nTextLen2) && (nTextLen1 > 0) )
    {
        sal_Int32 nMinSize = 2 * nTextLen1 + 6 * nPortionCount + 14;
        OSL_ENSURE( nMinSize <= nPhoneticSize, "PhoneticPortionModelList::importPortions - wrong size of phonetic data" );
        if( nMinSize <= nPhoneticSize )
        {
            aPhoneticText = rStrm.readUnicodeArray( nTextLen1 );
            clear();
            reserve( nPortionCount );
            PhoneticPortionModel aPortion;
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

void RichString::importString( SequenceInputStream& rStrm, bool bRich )
{
    sal_uInt8 nFlags = bRich ? rStrm.readuInt8() : 0;
    OUString aBaseText = BiffHelper::readString( rStrm );

    if( !rStrm.isEof() && getFlag( nFlags, BIFF12_STRINGFLAG_FONTS ) )
    {
        FontPortionModelList aPortions;
        aPortions.importPortions( rStrm );
        createTextPortions( aBaseText, aPortions );
    }
    else
    {
        createPortion()->setText( aBaseText );
    }

    if( !rStrm.isEof() && getFlag( nFlags, BIFF12_STRINGFLAG_PHONETICS ) )
    {
        OUString aPhoneticText = BiffHelper::readString( rStrm );
        PhoneticPortionModelList aPortions;
        aPortions.importPortions( rStrm );
        maPhonSettings.importStringData( rStrm );
        createPhoneticPortions( aPhoneticText, aPortions, aBaseText.getLength() );
    }
}

void RichString::importCharArray( BiffInputStream& rStrm, sal_uInt16 nChars, rtl_TextEncoding eTextEnc )
{
    createPortion()->setText( rStrm.readCharArrayUC( nChars, eTextEnc ) );
}

void RichString::importByteString( BiffInputStream& rStrm, rtl_TextEncoding eTextEnc, BiffStringFlags nFlags )
{
    OSL_ENSURE( !getFlag( nFlags, BIFF_STR_KEEPFONTS ), "RichString::importString - keep fonts not implemented" );
    OSL_ENSURE( !getFlag( nFlags, static_cast< BiffStringFlags >( ~(BIFF_STR_8BITLENGTH | BIFF_STR_EXTRAFONTS) ) ), "RichString::importByteString - unknown flag" );
    bool b8BitLength = getFlag( nFlags, BIFF_STR_8BITLENGTH );

    OString aBaseText = rStrm.readByteString( !b8BitLength );

    if( !rStrm.isEof() && getFlag( nFlags, BIFF_STR_EXTRAFONTS ) )
    {
        FontPortionModelList aPortions;
        aPortions.importPortions( rStrm, false );
        createTextPortions( aBaseText, eTextEnc, aPortions );
    }
    else
    {
        createPortion()->setText( OStringToOUString( aBaseText, eTextEnc ) );
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
    sal_Int32 nPhoneticSize = bPhonetic ? rStrm.readInt32() : 0;

    // --- character array ---
    OUString aBaseText = rStrm.readUniStringChars( nChars, b16Bit );

    // --- formatting ---
    // #122185# bRich flag may be set, but format runs may be missing
    if( !rStrm.isEof() && (nFontCount > 0) )
    {
        FontPortionModelList aPortions;
        aPortions.importPortions( rStrm, nFontCount, BIFF_FONTPORTION_16BIT );
        createTextPortions( aBaseText, aPortions );
    }
    else
    {
        createPortion()->setText( aBaseText );
    }

    // --- Asian phonetic information ---
    // #122185# bPhonetic flag may be set, but phonetic info may be missing
    if( !rStrm.isEof() && (nPhoneticSize > 0) )
    {
        sal_Int64 nPhoneticEnd = rStrm.tell() + nPhoneticSize;
        OSL_ENSURE( nPhoneticSize > 14, "RichString::importUniString - wrong size of phonetic data" );
        if( nPhoneticSize > 14 )
        {
            sal_uInt16 nId, nSize;
            rStrm >> nId >> nSize;
            OSL_ENSURE( nId == 1, "RichString::importUniString - unknown phonetic data identifier" );
            sal_Int32 nMinSize = nSize + 4;
            OSL_ENSURE( nMinSize <= nPhoneticSize, "RichString::importUniString - wrong size of phonetic data" );
            if( (nId == 1) && (nMinSize <= nPhoneticSize) )
            {
                maPhonSettings.importStringData( rStrm );
                PhoneticPortionModelList aPortions;
                OUString aPhoneticText = aPortions.importPortions( rStrm, nPhoneticSize );
                createPhoneticPortions( aPhoneticText, aPortions, aBaseText.getLength() );
            }
        }
        rStrm.seek( nPhoneticEnd );
    }
}

void RichString::finalizeImport()
{
    maTextPortions.forEachMem( &RichStringPortion::finalizeImport );
}

bool RichString::extractPlainString( OUString& orString, const Font* pFirstPortionFont ) const
{
    if( !maPhonPortions.empty() )
        return false;
    if( maTextPortions.empty() )
    {
        orString = OUString();
        return true;
    }
    if( (maTextPortions.size() == 1) && !maTextPortions.front()->hasFont() && !lclNeedsRichTextFormat( pFirstPortionFont ) )
    {
        orString = maTextPortions.front()->getText();
        return orString.indexOf( '\x0A' ) < 0;
    }
    return false;
}

void RichString::convert( const Reference< XText >& rxText, bool bReplaceOld, const Font* pFirstPortionFont ) const
{
    for( PortionVector::const_iterator aIt = maTextPortions.begin(), aEnd = maTextPortions.end(); aIt != aEnd; ++aIt )
    {
        (*aIt)->convert( rxText, pFirstPortionFont, bReplaceOld );
        pFirstPortionFont = 0;  // use passed font for first portion only
        bReplaceOld = false;    // do not replace first portion text with following portions
    }
}

// private --------------------------------------------------------------------

RichStringPortionRef RichString::createPortion()
{
    RichStringPortionRef xPortion( new RichStringPortion( *this ) );
    maTextPortions.push_back( xPortion );
    return xPortion;
}

RichStringPhoneticRef RichString::createPhonetic()
{
    RichStringPhoneticRef xPhonetic( new RichStringPhonetic( *this ) );
    maPhonPortions.push_back( xPhonetic );
    return xPhonetic;
}

void RichString::createTextPortions( const OString& rText, rtl_TextEncoding eTextEnc, FontPortionModelList& rPortions )
{
    maTextPortions.clear();
    sal_Int32 nStrLen = rText.getLength();
    if( nStrLen > 0 )
    {
        // add leading and trailing string position to ease the following loop
        if( rPortions.empty() || (rPortions.front().mnPos > 0) )
            rPortions.insert( rPortions.begin(), FontPortionModel( 0, -1 ) );
        if( rPortions.back().mnPos < nStrLen )
            rPortions.push_back( FontPortionModel( nStrLen, -1 ) );

        // create all string portions according to the font id vector
        for( FontPortionModelList::const_iterator aIt = rPortions.begin(); aIt->mnPos < nStrLen; ++aIt )
        {
            sal_Int32 nPortionLen = (aIt + 1)->mnPos - aIt->mnPos;
            if( (0 < nPortionLen) && (aIt->mnPos + nPortionLen <= nStrLen) )
            {
                // convert byte string to unicode string, using current font encoding
                FontRef xFont = getStyles().getFont( aIt->mnFontId );
                rtl_TextEncoding eFontEnc = xFont.get() ? xFont->getFontEncoding() : eTextEnc;
                OUString aUniStr = OStringToOUString( rText.copy( aIt->mnPos, nPortionLen ), eFontEnc );
                // create string portion
                RichStringPortionRef xPortion = createPortion();
                xPortion->setText( aUniStr );
                xPortion->setFontId( aIt->mnFontId );
            }
        }
    }
}

void RichString::createTextPortions( const OUString& rText, FontPortionModelList& rPortions )
{
    maTextPortions.clear();
    sal_Int32 nStrLen = rText.getLength();
    if( nStrLen > 0 )
    {
        // add leading and trailing string position to ease the following loop
        if( rPortions.empty() || (rPortions.front().mnPos > 0) )
            rPortions.insert( rPortions.begin(), FontPortionModel( 0, -1 ) );
        if( rPortions.back().mnPos < nStrLen )
            rPortions.push_back( FontPortionModel( nStrLen, -1 ) );

        // create all string portions according to the font id vector
        for( FontPortionModelList::const_iterator aIt = rPortions.begin(); aIt->mnPos < nStrLen; ++aIt )
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

void RichString::createPhoneticPortions( const ::rtl::OUString& rText, PhoneticPortionModelList& rPortions, sal_Int32 nBaseLen )
{
    maPhonPortions.clear();
    sal_Int32 nStrLen = rText.getLength();
    if( nStrLen > 0 )
    {
        // no portions - assign phonetic text to entire base text
        if( rPortions.empty() )
            rPortions.push_back( PhoneticPortionModel( 0, 0, nBaseLen ) );
        // add trailing string position to ease the following loop
        if( rPortions.back().mnPos < nStrLen )
            rPortions.push_back( PhoneticPortionModel( nStrLen, nBaseLen, 0 ) );

        // create all phonetic portions according to the portions vector
        for( PhoneticPortionModelList::const_iterator aIt = rPortions.begin(); aIt->mnPos < nStrLen; ++aIt )
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
