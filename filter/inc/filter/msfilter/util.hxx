/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Miklos Vajna <vmiklos@suse.cz> (SUSE, Inc.)
 * Portions created by the Initial Developer are Copyright (C) 2012 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef INCLUDED_MSFILTER_UTIL_HXX
#define INCLUDED_MSFILTER_UTIL_HXX

#include <rtl/textenc.h>
#include <tools/datetime.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include "filter/msfilter/msfilterdllapi.h"

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
MSFILTER_DLLPUBLIC rtl::OString DateTimeToOString( const DateTime& rDateTime );

/// Given a cBullet in encoding r_ioChrSet and fontname r_ioFontName return a
/// suitable new Bullet and change r_ioChrSet and r_ioFontName to form the
/// best-fit replacement in terms of default available MSOffice symbol
/// fonts.
///
/// Set bDisableUnicodeSupport when exporting to 8bit encodings
///
/// Used to map from [Open|Star]Symbol to some Windows font or other.
MSFILTER_DLLPUBLIC sal_Unicode bestFitOpenSymbolToMSFont(sal_Unicode cBullet,
    rtl_TextEncoding& r_ioChrSet, rtl::OUString& r_ioFontName, bool bDisableUnicodeSupport = false);


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
MSFILTER_DLLPUBLIC TextCategory categorizeCodePoint(sal_uInt32 codePoint, const rtl::OUString &rBcp47LanguageTag);

}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
