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

#ifndef OOX_XLS_RICHSTRING_HXX
#define OOX_XLS_RICHSTRING_HXX

#include "oox/helper/containerhelper.hxx"
#include "oox/xls/stylesbuffer.hxx"

namespace com { namespace sun { namespace star {
    namespace text { class XText; }
} } }

namespace oox {
namespace xls {

// ============================================================================

/** Flags used to specify import/export mode of strings. */
typedef sal_Int32 BiffStringFlags;

const BiffStringFlags BIFF_STR_DEFAULT      = 0x0000;   /// Default string settings.
const BiffStringFlags BIFF_STR_FORCEUNICODE = 0x0001;   /// Always use UCS-2 characters (default: try to compress). BIFF8 export only.
const BiffStringFlags BIFF_STR_8BITLENGTH   = 0x0002;   /// 8-bit string length field (default: 16-bit).
const BiffStringFlags BIFF_STR_SMARTFLAGS   = 0x0004;   /// Omit flags on empty string (default: read/write always). BIFF8 only.
const BiffStringFlags BIFF_STR_KEEPFONTS    = 0x0008;   /// Keep old fonts when reading unformatted string (default: clear fonts). Import only.
const BiffStringFlags BIFF_STR_EXTRAFONTS   = 0x0010;   /// Read trailing rich-string font array (default: nothing). BIFF2-BIFF5 import only.

// ============================================================================

/** Contains text data and font attributes for a part of a rich formatted string. */
class RichStringPortion : public WorkbookHelper
{
public:
    explicit            RichStringPortion( const WorkbookHelper& rHelper );

    /** Sets text data for this portion. */
    void                setText( const ::rtl::OUString& rText );
    /** Creates and returns a new font formatting object. */
    FontRef             createFont();
    /** Links this portion to a font object from the global font list. */
    void                setFontId( sal_Int32 nFontId );

    /** Final processing after import of all strings. */
    void                finalizeImport();

    /** Returns the text data of this portion. */
    inline const ::rtl::OUString& getText() const { return maText; }

    /** Converts the portion and appends it to the passed XText. */
    void                convert(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >& rxText,
                            sal_Int32 nXfId );

private:
    ::rtl::OUString     maText;         /// Portion text.
    FontRef             mxFont;         /// Embedded portion font, may be empty.
    sal_Int32           mnFontId;       /// Link to global font list.
};

typedef ::boost::shared_ptr< RichStringPortion > RichStringPortionRef;

// ----------------------------------------------------------------------------

enum BiffFontPortionMode
{
    BIFF_FONTPORTION_8BIT,              /// Font portion with 8-bit values.
    BIFF_FONTPORTION_16BIT,             /// Font portion with 16-bit values.
    BIFF_FONTPORTION_OBJ                /// Font portion in OBJ or TXO record.
};

// ----------------------------------------------------------------------------

/** Represents a position in a rich-string containing current font identifier.

    This object stores the position of a formatted character in a rich-string
    and the identifier of a font from the global font list used to format this
    and the following characters. Used in binary filters only.
 */
struct FontPortionModel
{
    sal_Int32           mnPos;          /// First character in the string.
    sal_Int32           mnFontId;       /// Font identifier for the next characters.

    explicit inline     FontPortionModel() : mnPos( 0 ), mnFontId( -1 ) {}
    explicit inline     FontPortionModel( sal_Int32 nPos, sal_Int32 nFontId ) :
                            mnPos( nPos ), mnFontId( nFontId ) {}

    void                read( RecordInputStream& rStrm );
    void                read( BiffInputStream& rStrm, BiffFontPortionMode eMode );
};

// ----------------------------------------------------------------------------

/** A vector with all font portions in a rich-string. */
class FontPortionModelList : public ::std::vector< FontPortionModel >
{
public:
    inline explicit     FontPortionModelList() {}

    /** Appends a rich-string font identifier. */
    void                appendPortion( const FontPortionModel& rPortion );
    /** Reads count and font identifiers from the passed stream. */
    void                importPortions( RecordInputStream& rStrm );
    /** Reads nCount font identifiers from the passed stream. */
    void                importPortions( BiffInputStream& rStrm, sal_uInt16 nCount, BiffFontPortionMode eMode );
    /** Reads count and font identifiers from the passed stream. */
    void                importPortions( BiffInputStream& rStrm, bool b16Bit );
};

// ============================================================================

struct PhoneticDataModel
{
    sal_Int32           mnFontId;       /// Font identifier for text formatting.
    sal_Int32           mnType;         /// Phonetic text type.
    sal_Int32           mnAlignment;    /// Phonetic portion alignment.

    explicit            PhoneticDataModel();

    /** Sets the passed data from binary import. */
    void                setBinData( sal_Int32 nType, sal_Int32 nAlignment );
};

// ----------------------------------------------------------------------------

class PhoneticSettings : public WorkbookHelper
{
public:
    explicit            PhoneticSettings( const WorkbookHelper& rHelper );

    /** Imports phonetic settings from the phoneticPr element. */
    void                importPhoneticPr( const AttributeList& rAttribs );
    /** Imports phonetic settings from the PHONETICPR record. */
    void                importPhoneticPr( RecordInputStream& rStrm );
    /** Imports phonetic settings from the PHONETICPR record. */
    void                importPhoneticPr( BiffInputStream& rStrm );

    /** Imports phonetic settings from a rich string. */
    void                importStringData( RecordInputStream& rStrm );
    /** Imports phonetic settings from a rich string. */
    void                importStringData( BiffInputStream& rStrm );

private:
    PhoneticDataModel   maModel;
};

// ============================================================================

/** Contains text data and positioning information for a phonetic text portion. */
class RichStringPhonetic : public WorkbookHelper
{
public:
    explicit            RichStringPhonetic( const WorkbookHelper& rHelper );

    /** Sets text data for this phonetic portion. */
    void                setText( const ::rtl::OUString& rText );
    /** Imports attributes of a phonetic run (rPh element). */
    void                importPhoneticRun( const AttributeList& rAttribs );
    /** Sets the associated range in base text for this phonetic portion. */
    void                setBaseRange( sal_Int32 nBasePos, sal_Int32 nBaseEnd );

private:
    ::rtl::OUString     maText;         /// Portion text.
    sal_Int32           mnBasePos;      /// Start position in base text.
    sal_Int32           mnBaseEnd;      /// One-past-end position in base text.
};

typedef ::boost::shared_ptr< RichStringPhonetic > RichStringPhoneticRef;

// ----------------------------------------------------------------------------

/** Represents a phonetic text portion in a rich-string with phonetic text.
    Used in binary filters only. */
struct PhoneticPortionModel
{
    sal_Int32           mnPos;          /// First character in phonetic text.
    sal_Int32           mnBasePos;      /// First character in base text.
    sal_Int32           mnBaseLen;      /// Number of characters in base text.

    explicit inline     PhoneticPortionModel() : mnPos( -1 ), mnBasePos( -1 ), mnBaseLen( 0 ) {}
    explicit inline     PhoneticPortionModel( sal_Int32 nPos, sal_Int32 nBasePos, sal_Int32 nBaseLen ) :
                            mnPos( nPos ), mnBasePos( nBasePos ), mnBaseLen( nBaseLen ) {}

    void                read( RecordInputStream& rStrm );
    void                read( BiffInputStream& rStrm );
};

// ----------------------------------------------------------------------------

/** A vector with all phonetic portions in a rich-string. */
class PhoneticPortionModelList : public ::std::vector< PhoneticPortionModel >
{
public:
    inline explicit     PhoneticPortionModelList() {}

    /** Appends a rich-string phonetic portion. */
    void                appendPortion( const PhoneticPortionModel& rPortion );
    /** Reads all phonetic portions from the passed stream. */
    void                importPortions( RecordInputStream& rStrm );
    /** Reads phonetic portion data from the passed stream. */
    ::rtl::OUString     importPortions( BiffInputStream& rStrm, sal_Int32 nPhoneticSize );
};

// ============================================================================

/** Contains string data and a list of formatting runs for a rich formatted string. */
class RichString : public WorkbookHelper
{
public:
    explicit            RichString( const WorkbookHelper& rHelper );

    /** Appends and returns a portion object for a plain string (t element). */
    RichStringPortionRef importText( const AttributeList& rAttribs );
    /** Appends and returns a portion object for a new formatting run (r element). */
    RichStringPortionRef importRun( const AttributeList& rAttribs );
    /** Appends and returns a phonetic text object for a new phonetic run (rPh element). */
    RichStringPhoneticRef importPhoneticRun( const AttributeList& rAttribs );
    /** Imports phonetic settings from the rPhoneticPr element. */
    void                importPhoneticPr( const AttributeList& rAttribs );

    /** Imports a Unicode rich-string from the passed record stream. */
    void                importString( RecordInputStream& rStrm, bool bRich );

    /** Imports a byte string from the passed BIFF stream. */
    void                importByteString( BiffInputStream& rStrm, rtl_TextEncoding eDefaultTextEnc, BiffStringFlags nFlags = BIFF_STR_DEFAULT );
    /** Imports a Unicode rich-string from the passed BIFF stream. */
    void                importUniString( BiffInputStream& rStrm, BiffStringFlags nFlags = BIFF_STR_DEFAULT );

    /** Final processing after import of all strings. */
    void                finalizeImport();

    /** Returns the plain text concatenated from all string portions. */
    ::rtl::OUString     getPlainText() const;

    /** Converts the string and writes it into the passed XText. */
    void                convert(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >& rxText,
                            sal_Int32 nXfId ) const;

private:
    /** Creates, appends, and returns a new empty string portion. */
    RichStringPortionRef createPortion();
    /** Creates, appends, and returns a new empty phonetic text portion. */
    RichStringPhoneticRef createPhonetic();

    /** Create base text portions from the passed string and character formatting. */
    void                createFontPortions( const ::rtl::OString& rText, rtl_TextEncoding eDefaultTextEnc, FontPortionModelList& rPortions );
    /** Create base text portions from the passed string and character formatting. */
    void                createFontPortions( const ::rtl::OUString& rText, FontPortionModelList& rPortions );
    /** Create phonetic text portions from the passed string and portion data. */
    void                createPhoneticPortions( const ::rtl::OUString& rText, PhoneticPortionModelList& rPortions, sal_Int32 nBaseLen );

private:
    typedef RefVector< RichStringPortion >  PortionVec;
    typedef RefVector< RichStringPhonetic > PhoneticVec;

    PortionVec          maFontPortions; /// String portions with font data.
    PhoneticSettings    maPhonSettings; /// Phonetic settings for this string.
    PhoneticVec         maPhonPortions; /// Phonetic text portions.
};

typedef ::boost::shared_ptr< RichString > RichStringRef;

// ============================================================================

} // namespace xls
} // namespace oox

#endif

