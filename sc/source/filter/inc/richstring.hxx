/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_RICHSTRING_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_RICHSTRING_HXX

#include <oox/helper/refvector.hxx>
#include "stylesbuffer.hxx"

class EditTextObject;
struct ESelection;
class ScEditEngineDefaulter;

namespace com { namespace sun { namespace star {
    namespace text { class XText; }
} } }

namespace oox {
namespace xls {

/** Contains text data and font attributes for a part of a rich formatted string. */
class RichStringPortion : public WorkbookHelper
{
public:
    explicit            RichStringPortion( const WorkbookHelper& rHelper );

    /** Sets text data for this portion. */
    void                setText( const OUString& rText );
    /** Creates and returns a new font formatting object. */
    FontRef const &     createFont();
    /** Links this portion to a font object from the global font list. */
    void                setFontId( sal_Int32 nFontId );

    /** Final processing after import of all strings. */
    void                finalizeImport();

    /** Returns the text data of this portion. */
    inline const OUString& getText() const { return maText; }
    /** Returns true, if the portion contains font formatting. */
    inline bool         hasFont() const { return mxFont.get() != nullptr; }

    /** Converts the portion and replaces or appends to the passed XText. */
    void                convert(
                            const css::uno::Reference< css::text::XText >& rxText,
                            const oox::xls::Font* pFont, bool bReplace );
    void                convert( ScEditEngineDefaulter& rEE, ESelection& rSelection, const oox::xls::Font* pFont );

    void                writeFontProperties(
        const css::uno::Reference< css::text::XText >& rxText,
        const oox::xls::Font* pFont ) const;

private:
    OUString            maText;         /// Portion text.
    FontRef             mxFont;         /// Embedded portion font, may be empty.
    sal_Int32           mnFontId;       /// Link to global font list.
    bool                mbConverted;    /// Without repeatly convert
};

typedef std::shared_ptr< RichStringPortion > RichStringPortionRef;

enum BiffFontPortionMode
{
    BIFF_FONTPORTION_8BIT,              /// Font portion with 8-bit values.
    BIFF_FONTPORTION_16BIT,             /// Font portion with 16-bit values.
    BIFF_FONTPORTION_OBJ                /// Font portion in OBJ or TXO record.
};

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

    void                read( SequenceInputStream& rStrm );
};

/** A vector with all font portions in a rich-string. */
class FontPortionModelList {
    ::std::vector< FontPortionModel > mvModels;

public:
    inline explicit     FontPortionModelList() : mvModels() {}

    bool empty() const { return mvModels.empty(); }

    const FontPortionModel& back() const { return mvModels.back(); }
    const FontPortionModel& front() const { return mvModels.front(); }

    void push_back(const FontPortionModel& rModel) { mvModels.push_back(rModel); }

    void insert(::std::vector< FontPortionModel >::iterator it,
                const FontPortionModel& rModel)
    { mvModels.insert(it, rModel); }

    ::std::vector< FontPortionModel >::iterator begin() { return mvModels.begin(); }

    /** Appends a rich-string font identifier. */
    void                appendPortion( const FontPortionModel& rPortion );
    /** Reads count and font identifiers from the passed stream. */
    void                importPortions( SequenceInputStream& rStrm );
};

struct PhoneticDataModel
{
    sal_Int32           mnFontId;       /// Font identifier for text formatting.
    sal_Int32           mnType;         /// Phonetic text type.
    sal_Int32           mnAlignment;    /// Phonetic portion alignment.

    explicit            PhoneticDataModel();

    /** Sets the passed data from binary import. */
    void                setBiffData( sal_Int32 nType, sal_Int32 nAlignment );
};

class PhoneticSettings : public WorkbookHelper
{
public:
    explicit            PhoneticSettings( const WorkbookHelper& rHelper );

    /** Imports phonetic settings from the phoneticPr element. */
    void                importPhoneticPr( const AttributeList& rAttribs );
    /** Imports phonetic settings from the PHONETICPR record. */
    void                importPhoneticPr( SequenceInputStream& rStrm );

    /** Imports phonetic settings from a rich string. */
    void                importStringData( SequenceInputStream& rStrm );

private:
    PhoneticDataModel   maModel;
};

/** Contains text data and positioning information for a phonetic text portion. */
class RichStringPhonetic : public WorkbookHelper
{
public:
    explicit            RichStringPhonetic( const WorkbookHelper& rHelper );

    /** Sets text data for this phonetic portion. */
    void                setText( const OUString& rText );
    /** Imports attributes of a phonetic run (rPh element). */
    void                importPhoneticRun( const AttributeList& rAttribs );
    /** Sets the associated range in base text for this phonetic portion. */
    void                setBaseRange( sal_Int32 nBasePos, sal_Int32 nBaseEnd );

private:
    OUString     maText;         /// Portion text.
    sal_Int32           mnBasePos;      /// Start position in base text.
    sal_Int32           mnBaseEnd;      /// One-past-end position in base text.
};

typedef std::shared_ptr< RichStringPhonetic > RichStringPhoneticRef;

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

    void                read( SequenceInputStream& rStrm );
};

/** A vector with all phonetic portions in a rich-string. */
class PhoneticPortionModelList
{
public:
    inline explicit     PhoneticPortionModelList() : mvModels() {}

    bool empty() const { return mvModels.empty(); }

    const PhoneticPortionModel& back() const { return mvModels.back(); }

    void push_back(const PhoneticPortionModel& rModel) { mvModels.push_back(rModel); }

    ::std::vector< PhoneticPortionModel >::const_iterator begin() const { return mvModels.begin(); }

    /** Appends a rich-string phonetic portion. */
    void                appendPortion( const PhoneticPortionModel& rPortion );
    /** Reads all phonetic portions from the passed stream. */
    void                importPortions( SequenceInputStream& rStrm );

private:
    ::std::vector< PhoneticPortionModel > mvModels;
};

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
    void                importString( SequenceInputStream& rStrm, bool bRich );

    /** Final processing after import of all strings. */
    void                finalizeImport();

    /** Tries to extract a plain string from this object. Returns the string,
        if there is only one unformatted portion. */
    bool                extractPlainString(
                            OUString& orString,
                            const oox::xls::Font* pFirstPortionFont = nullptr ) const;

    /** Converts the string and writes it into the passed XText.
        @param rxText  The XText interface of the target object.
        @param bReplaceOld  True = replace old contents of the text object. */
    void                convert(
                            const css::uno::Reference< css::text::XText >& rxText,
                            bool bReplaceOld ) const;
    ::EditTextObject*   convert( ScEditEngineDefaulter& rEE, const oox::xls::Font* pFont ) const;

private:
    /** Creates, appends, and returns a new empty string portion. */
    RichStringPortionRef createPortion();
    /** Creates, appends, and returns a new empty phonetic text portion. */
    RichStringPhoneticRef createPhonetic();

    /** Create base text portions from the passed string and character formatting. */
    void                createTextPortions( const OUString& rText, FontPortionModelList& rPortions );
    /** Create phonetic text portions from the passed string and portion data. */
    void                createPhoneticPortions( const OUString& rText, PhoneticPortionModelList& rPortions, sal_Int32 nBaseLen );

private:
    typedef RefVector< RichStringPortion >  PortionVector;
    typedef RefVector< RichStringPhonetic > PhoneticVector;

    PortionVector       maTextPortions; /// String portions with font data.
    PhoneticSettings    maPhonSettings; /// Phonetic settings for this string.
    PhoneticVector      maPhonPortions; /// Phonetic text portions.
};

typedef std::shared_ptr< RichString > RichStringRef;

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
