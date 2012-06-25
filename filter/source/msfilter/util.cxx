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

#include <rtl/ustring.hxx>
#include <rtl/strbuf.hxx>
#include <unotools/fontcvt.hxx>
#include <unotools/fontdefs.hxx>
#include <vcl/svapp.hxx>
#include <filter/msfilter/util.hxx>

namespace msfilter {
namespace util {

rtl_TextEncoding getBestTextEncodingFromLocale(const ::com::sun::star::lang::Locale &rLocale)
{
    //Obviously not comprehensive, feel free to expand these, they're for ultimate fallbacks
    //in last-ditch broken-file-format cases to guess the right 8bit encodings
    const rtl::OUString &rLanguage = rLocale.Language;
    if (rLanguage == "cs" || rLanguage == "hu" || rLanguage == "pl")
        return RTL_TEXTENCODING_MS_1250;
    if (rLanguage == "ru" || rLanguage == "uk")
        return RTL_TEXTENCODING_MS_1251;
    if (rLanguage == "el")
        return RTL_TEXTENCODING_MS_1253;
    if (rLanguage == "tr")
        return RTL_TEXTENCODING_MS_1254;
    if (rLanguage == "lt")
        return RTL_TEXTENCODING_MS_1257;
    return RTL_TEXTENCODING_MS_1252;
}

sal_uInt32 BGRToRGB(sal_uInt32 nColor)
{
    sal_uInt8
        r(static_cast<sal_uInt8>(nColor&0xFF)),
        g(static_cast<sal_uInt8>(((nColor)>>8)&0xFF)),
        b(static_cast<sal_uInt8>((nColor>>16)&0xFF)),
        t(static_cast<sal_uInt8>((nColor>>24)&0xFF));
    nColor = (t<<24) + (r<<16) + (g<<8) + b;
    return nColor;
}

DateTime DTTM2DateTime( long lDTTM )
{
    /*
    mint    short   :6  0000003F    minutes (0-59)
    hr      short   :5  000007C0    hours (0-23)
    dom     short   :5  0000F800    days of month (1-31)
    mon     short   :4  000F0000    months (1-12)
    yr      short   :9  1FF00000    years (1900-2411)-1900
    wdy     short   :3  E0000000    weekday(Sunday=0
                                            Monday=1
    ( wdy can be ignored )                  Tuesday=2
                                            Wednesday=3
                                            Thursday=4
                                            Friday=5
                                            Saturday=6)
    */
    DateTime aDateTime(Date( 0 ), Time( 0 ));
    if( lDTTM )
    {
        sal_uInt16 lMin = (sal_uInt16)(lDTTM & 0x0000003F);
        lDTTM >>= 6;
        sal_uInt16 lHour= (sal_uInt16)(lDTTM & 0x0000001F);
        lDTTM >>= 5;
        sal_uInt16 lDay = (sal_uInt16)(lDTTM & 0x0000001F);
        lDTTM >>= 5;
        sal_uInt16 lMon = (sal_uInt16)(lDTTM & 0x0000000F);
        lDTTM >>= 4;
        sal_uInt16 lYear= (sal_uInt16)(lDTTM & 0x000001FF) + 1900;
        aDateTime = DateTime(Date(lDay, lMon, lYear), Time(lHour, lMin));
    }
    return aDateTime;
}

/// Append the number as 2-digit when less than 10.
static void lcl_AppendTwoDigits( rtl::OStringBuffer &rBuffer, sal_Int32 nNum )
{
    if ( nNum < 0 || nNum > 99 )
    {
        rBuffer.append( "00" );
        return;
    }

    if ( nNum < 10 )
        rBuffer.append( '0' );

    rBuffer.append( nNum );
}

rtl::OString DateTimeToOString( const DateTime& rDateTime )
{
    DateTime aInUTC( rDateTime );
// HACK: this is correct according to the spec, but MSOffice believes everybody lives
// in UTC+0 when reading it back
//    aInUTC.ConvertToUTC();

    rtl::OStringBuffer aBuffer( 25 );
    aBuffer.append( sal_Int32( aInUTC.GetYear() ) );
    aBuffer.append( '-' );

    lcl_AppendTwoDigits( aBuffer, aInUTC.GetMonth() );
    aBuffer.append( '-' );

    lcl_AppendTwoDigits( aBuffer, aInUTC.GetDay() );
    aBuffer.append( 'T' );

    lcl_AppendTwoDigits( aBuffer, aInUTC.GetHour() );
    aBuffer.append( ':' );

    lcl_AppendTwoDigits( aBuffer, aInUTC.GetMin() );
    aBuffer.append( ':' );

    lcl_AppendTwoDigits( aBuffer, aInUTC.GetSec() );
    aBuffer.append( 'Z' ); // we are in UTC

    return aBuffer.makeStringAndClear();
}

sal_Unicode bestFitOpenSymbolToMSFont(sal_Unicode cChar,
    rtl_TextEncoding& rChrSet, rtl::OUString& rFontName, bool bDisableUnicodeSupport)
{
    StarSymbolToMSMultiFont *pConvert = CreateStarSymbolToMSMultiFont();
    rtl::OUString sFont = pConvert->ConvertChar(cChar);
    delete pConvert;
    if (!sFont.isEmpty())
    {
        cChar = static_cast< sal_Unicode >(cChar | 0xF000);
        rFontName = sFont;
        rChrSet = RTL_TEXTENCODING_SYMBOL;
    }
    else if (!bDisableUnicodeSupport && (cChar < 0xE000 || cChar > 0xF8FF))
    {
        /*
          Ok we can't fit into a known windows unicode font, but
          we are not in the private area, so we are a
          standardized symbol, so turn off the symbol bit and
          let words own font substitution kick in
        */
        rChrSet = RTL_TEXTENCODING_UNICODE;
        xub_StrLen nIndex = 0;
        rFontName = ::GetNextFontToken(rFontName, nIndex);
    }
    else
    {
        /*
          Well we don't have an available substition, and we're
          in our private area, so give up and show a standard
          bullet symbol
        */
        rFontName = "Wingdings";
        cChar = static_cast< sal_Unicode >(0x6C);
    }
    return cChar;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
