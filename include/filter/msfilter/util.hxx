/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_MSFILTER_UTIL_HXX
#define INCLUDED_MSFILTER_UTIL_HXX

#include <rtl/textenc.h>
#include <tools/datetime.hxx>
#include <tools/color.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include "filter/msfilter/msfilterdllapi.h"
#include <com/sun/star/awt/Size.hpp>

namespace msfilter {
namespace util {

/// Returns the best-fit default 8bit encoding for a given locale
/// i.e. useful when dealing with legacy formats which use legacy text encodings without recording
/// what the encoding is, but you know or can guess the language
MSFILTER_DLLPUBLIC rtl_TextEncoding getBestTextEncodingFromLocale(const ::com::sun::star::lang::Locale &rLocale);

/// Convert a color in BGR format to RGB.
MSFILTER_DLLPUBLIC sal_uInt32 BGRToRGB(sal_uInt32 nColour);

/** Convert from DTTM to Writer's DateTime

  @author
  <a href="mailto:mmaher@openoffice.org">Martin Maher</a
  */
MSFILTER_DLLPUBLIC DateTime DTTM2DateTime( long lDTTM );

/** Convert DateTime to xsd::dateTime string.

I guess there must be an implementation of this somewhere in LO, but I failed
to find it, unfortunately :-(
*/
MSFILTER_DLLPUBLIC OString DateTimeToOString( const DateTime& rDateTime );

/// Given a cBullet in encoding r_ioChrSet and fontname r_ioFontName return a
/// suitable new Bullet and change r_ioChrSet and r_ioFontName to form the
/// best-fit replacement in terms of default available MSOffice symbol
/// fonts.
///
/// Set bDisableUnicodeSupport when exporting to 8bit encodings
///
/// Used to map from [Open|Star]Symbol to some Windows font or other.
MSFILTER_DLLPUBLIC sal_Unicode bestFitOpenSymbolToMSFont(sal_Unicode cBullet,
    rtl_TextEncoding& r_ioChrSet, OUString& r_ioFontName, bool bDisableUnicodeSupport = false);


enum TextCategory
{
    latin,      //Latin
    cs,         //Complex Script
    ea,         //East Asian
    sym         //Symbol
};

/** Categorize codepoints according to how MS seems to do it.

  It's been bugging me for ages as to what codepoint MS considers in
  what category. Tom Jebo has a post suggesting the criteria used here
  and indicating its been submitting to the standards working group
  as a proposed resolution.
*/
MSFILTER_DLLPUBLIC TextCategory categorizeCodePoint(sal_uInt32 codePoint, const OUString &rBcp47LanguageTag);

/// Converts tools Color to HTML color (without leading hashmark).
MSFILTER_DLLPUBLIC OString ConvertColor( const Color &rColor );


/** Paper size in 1/100 millimeters. */
struct MSFILTER_DLLPUBLIC ApiPaperSize
{
    sal_Int32           mnWidth;
    sal_Int32           mnHeight;
};

class MSFILTER_DLLPUBLIC PaperSizeConv
{
public:
    static sal_Int32 getMSPaperSizeIndex( const com::sun::star::awt::Size& rSize );
    static const ApiPaperSize& getApiSizeForMSPaperSizeIndex( sal_Int32 nMSOPaperIndex );
};

/**
 * Finds the quoted text in a field instruction text.
 *
 * Example: SEQ "Figure" \someoption -> "Figure"
 */
MSFILTER_DLLPUBLIC OUString findQuotedText( const OUString& rCommand, const sal_Char* cStartQuote, const sal_Unicode uEndQuote );
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
