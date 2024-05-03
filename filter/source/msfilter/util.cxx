/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <rtl/ustring.hxx>
#include <rtl/character.hxx>
#include <comphelper/string.hxx>
#include <unotools/fontcvt.hxx>
#include <unotools/fontdefs.hxx>
#include <utility>
#include <vcl/BitmapPalette.hxx>
#include <filter/msfilter/escherex.hxx>
#include <filter/msfilter/util.hxx>
#include <o3tl/string_view.hxx>
#include <memory>
#include <unordered_map>

namespace msfilter::util {

rtl_TextEncoding getBestTextEncodingFromLocale(const css::lang::Locale &rLocale)
{
    // Obviously not comprehensive, feel free to expand these, they're for ultimate fallbacks
    // in last-ditch broken-file-format cases to guess the right 8bit encodings
    const OUString &rLanguage = rLocale.Language;
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
    if (rLanguage == "th")
        return RTL_TEXTENCODING_MS_874;
    if (rLanguage == "vi")
        return RTL_TEXTENCODING_MS_1258;
    return RTL_TEXTENCODING_MS_1252;
}

::Color BGRToRGB(sal_uInt32 nColor)
{
    sal_uInt8
        r(static_cast<sal_uInt8>(nColor&0xFF)),
        g(static_cast<sal_uInt8>((nColor>>8)&0xFF)),
        b(static_cast<sal_uInt8>((nColor>>16)&0xFF)),
        t(static_cast<sal_uInt8>((nColor>>24)&0xFF));
    return ::Color(ColorTransparency, t, r, g, b);
}

DateTime DTTM2DateTime( tools::Long lDTTM )
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
    DateTime aDateTime(Date( 0 ), ::tools::Time( 0 ));
    if( lDTTM )
    {
        sal_uInt16 lMin = static_cast<sal_uInt16>(lDTTM & 0x0000003F);
        lDTTM >>= 6;
        sal_uInt16 lHour= static_cast<sal_uInt16>(lDTTM & 0x0000001F);
        lDTTM >>= 5;
        sal_uInt16 lDay = static_cast<sal_uInt16>(lDTTM & 0x0000001F);
        lDTTM >>= 5;
        sal_uInt16 lMon = static_cast<sal_uInt16>(lDTTM & 0x0000000F);
        lDTTM >>= 4;
        sal_uInt16 lYear= static_cast<sal_uInt16>(lDTTM & 0x000001FF) + 1900;
        aDateTime = DateTime(Date(lDay, lMon, lYear), tools::Time(lHour, lMin));
    }
    return aDateTime;
}

sal_Unicode bestFitOpenSymbolToMSFont(sal_Unicode cChar,
    rtl_TextEncoding& rChrSet, OUString& rFontName)
{
    std::unique_ptr<StarSymbolToMSMultiFont> pConvert(CreateStarSymbolToMSMultiFont());
    OUString sFont = pConvert->ConvertChar(cChar);
    pConvert.reset();
    if (!sFont.isEmpty())
    {
        cChar = static_cast< sal_Unicode >(cChar | 0xF000);
        rFontName = sFont;
        rChrSet = RTL_TEXTENCODING_SYMBOL;
    }
    else if (cChar < 0xE000 || cChar > 0xF8FF)
    {
        /*
          Ok we can't fit into a known windows unicode font, but
          we are not in the private area, so we are a
          standardized symbol, so turn off the symbol bit and
          let words own font substitution kick in
        */
        rChrSet = RTL_TEXTENCODING_UNICODE;
        sal_Int32 nIndex = 0;
        rFontName = ::GetNextFontToken(rFontName, nIndex);
    }
    else
    {
        /*
          Well we don't have an available substitution, and we're
          in our private area, so give up and show a standard
          bullet symbol
        */
        rFontName = "Wingdings";
        cChar = u'\x6C';
    }
    return cChar;
}


OString ConvertColor( const Color &rColor )
{
    static constexpr OStringLiteral AUTO( "auto" );

    if ( rColor == COL_AUTO )
        return AUTO;

    const char pHexDigits[] = "0123456789ABCDEF";
    char pBuffer[] = "000000";

    pBuffer[0] = pHexDigits[ ( rColor.GetRed()   >> 4 ) & 0x0F ];
    pBuffer[1] = pHexDigits[   rColor.GetRed()          & 0x0F ];
    pBuffer[2] = pHexDigits[ ( rColor.GetGreen() >> 4 ) & 0x0F ];
    pBuffer[3] = pHexDigits[   rColor.GetGreen()        & 0x0F ];
    pBuffer[4] = pHexDigits[ ( rColor.GetBlue()  >> 4 ) & 0x0F ];
    pBuffer[5] = pHexDigits[   rColor.GetBlue()         & 0x0F ];

    return OString( pBuffer );
}

OUString ConvertColorOU( const Color &rColor )
{
    static constexpr OUStringLiteral AUTO( u"auto" );

    if ( rColor == COL_AUTO )
        return AUTO;

    const char pHexDigits[] = "0123456789ABCDEF";
    sal_Unicode pBuffer[] = u"000000";

    pBuffer[0] = pHexDigits[ ( rColor.GetRed()   >> 4 ) & 0x0F ];
    pBuffer[1] = pHexDigits[   rColor.GetRed()          & 0x0F ];
    pBuffer[2] = pHexDigits[ ( rColor.GetGreen() >> 4 ) & 0x0F ];
    pBuffer[3] = pHexDigits[   rColor.GetGreen()        & 0x0F ];
    pBuffer[4] = pHexDigits[ ( rColor.GetBlue()  >> 4 ) & 0x0F ];
    pBuffer[5] = pHexDigits[   rColor.GetBlue()         & 0x0F ];

    return OUString( pBuffer );
}

#define IN2MM100( v )    static_cast< sal_Int32 >( (v) * 2540.0 + 0.5 )
#define MM2MM100( v )    static_cast< sal_Int32 >( (v) * 100.0 + 0.5 )

// see XclPaperSize pPaperSizeTable in calc and aDinTab in i18nutil
const ApiPaperSize spPaperSizeTable[] =
{
    { 0, 0 },                                                //  0 - (undefined)
    { IN2MM100( 8.5 ),       IN2MM100( 11 )      },          //  1 - Letter paper
    { IN2MM100( 8.5 ),       IN2MM100( 11 )      },          //  2 - Letter small paper
    { IN2MM100( 11 ),        IN2MM100( 17 )      },          //  3 - Tabloid paper
    { IN2MM100( 17 ),        IN2MM100( 11 )      },          //  4 - Ledger paper
    { IN2MM100( 8.5 ),       IN2MM100( 14 )      },          //  5 - Legal paper
    { IN2MM100( 5.5 ),       IN2MM100( 8.5 )     },          //  6 - Statement paper
    { IN2MM100( 7.25 ),      IN2MM100( 10.5 )    },          //  7 - Executive paper
    { MM2MM100( 297 ),       MM2MM100( 420 )     },          //  8 - A3 paper
    { MM2MM100( 210 ),       MM2MM100( 297 )     },          //  9 - A4 paper
    { MM2MM100( 210 ),       MM2MM100( 297 )     },          // 10 - A4 small paper
    { MM2MM100( 148 ),       MM2MM100( 210 )     },          // 11 - A5 paper
    /* for JIS vs ISO B confusion see:
       https://docs.microsoft.com/en-us/windows/win32/intl/paper-sizes
       http://wiki.openoffice.org/wiki/DefaultPaperSize comments
       http://partners.adobe.com/public/developer/en/ps/5003.PPD_Spec_v4.3.pdf */
    { MM2MM100( 257 ),       MM2MM100( 364 )     },          // 12 - B4 (JIS) paper
    { MM2MM100( 182 ),       MM2MM100( 257 )     },          // 13 - B5 (JIS) paper
    { IN2MM100( 8.5 ),       IN2MM100( 13 )      },          // 14 - Folio paper
    { MM2MM100( 215 ),       MM2MM100( 275 )     },          // 15 - Quarto paper
    { IN2MM100( 10 ),        IN2MM100( 14 )      },          // 16 - Standard paper
    { IN2MM100( 11 ),        IN2MM100( 17 )      },          // 17 - Standard paper
    { IN2MM100( 8.5 ),       IN2MM100( 11 )      },          // 18 - Note paper
    { IN2MM100( 3.875 ),     IN2MM100( 8.875 )   },          // 19 - #9 envelope
    { IN2MM100( 4.125 ),     IN2MM100( 9.5 )     },          // 20 - #10 envelope
    { IN2MM100( 4.5 ),       IN2MM100( 10.375 )  },          // 21 - #11 envelope
    { IN2MM100( 4.75 ),      IN2MM100( 11 )      },          // 22 - #12 envelope
    { IN2MM100( 5 ),         IN2MM100( 11.5 )    },          // 23 - #14 envelope
    { IN2MM100( 17 ),        IN2MM100( 22 )      },          // 24 - C paper
    { IN2MM100( 22 ),        IN2MM100( 34 )      },          // 25 - D paper
    { IN2MM100( 34 ),        IN2MM100( 44 )      },          // 26 - E paper
    { MM2MM100( 110 ),       MM2MM100( 220 )     },          // 27 - DL envelope
    { MM2MM100( 162 ),       MM2MM100( 229 )     },          // 28 - C5 envelope
    { MM2MM100( 324 ),       MM2MM100( 458 )     },          // 29 - C3 envelope
    { MM2MM100( 229 ),       MM2MM100( 324 )     },          // 30 - C4 envelope
    { MM2MM100( 114 ),       MM2MM100( 162 )     },          // 31 - C6 envelope
    { MM2MM100( 114 ),       MM2MM100( 229 )     },          // 32 - C65 envelope
    { MM2MM100( 250 ),       MM2MM100( 353 )     },          // 33 - B4 envelope
    { MM2MM100( 176 ),       MM2MM100( 250 )     },          // 34 - B5 envelope
    { MM2MM100( 176 ),       MM2MM100( 125 )     },          // 35 - B6 envelope
    { MM2MM100( 110 ),       MM2MM100( 230 )     },          // 36 - Italy envelope
    { IN2MM100( 3.875 ),     IN2MM100( 7.5 )     },          // 37 - Monarch envelope
    { IN2MM100( 3.625 ),     IN2MM100( 6.5 )     },          // 38 - 6 3/4 envelope
    { IN2MM100( 14.875 ),    IN2MM100( 11 )      },          // 39 - US standard fanfold
    { IN2MM100( 8.5 ),       IN2MM100( 12 )      },          // 40 - German standard fanfold
    { IN2MM100( 8.5 ),       IN2MM100( 13 )      },          // 41 - German legal fanfold
    { MM2MM100( 250 ),       MM2MM100( 353 )     },          // 42 - ISO B4
    { MM2MM100( 200 ),       MM2MM100( 148 )     },          // 43 - Japanese double postcard
    { IN2MM100( 9 ),         IN2MM100( 11 )      },          // 44 - Standard paper
    { IN2MM100( 10 ),        IN2MM100( 11 )      },          // 45 - Standard paper
    { IN2MM100( 15 ),        IN2MM100( 11 )      },          // 46 - Standard paper
    { MM2MM100( 220 ),       MM2MM100( 220 )     },          // 47 - Invite envelope
    { 0, 0 },                                                // 48 - (undefined)
    { 0, 0 },                                                // 49 - (undefined)
    /* See: https://docs.microsoft.com/en-us/windows/win32/intl/paper-sizes */
    { IN2MM100( 9.5 ),       IN2MM100( 12 )      },          // 50 - Letter extra paper
    { IN2MM100( 9.5 ),       IN2MM100( 15 )      },          // 51 - Legal extra paper
    { IN2MM100( 11.69 ),     IN2MM100( 18 )      },          // 52 - Tabloid extra paper
    { MM2MM100( 235 ),       MM2MM100( 322 )     },          // 53 - A4 extra paper
    { IN2MM100( 8.5 ),       IN2MM100( 11 )      },          // 54 - Letter transverse paper
    { MM2MM100( 210 ),       MM2MM100( 297 )     },          // 55 - A4 transverse paper
    { IN2MM100( 9.5 ),       IN2MM100( 12 )      },          // 56 - Letter extra transverse paper
    { MM2MM100( 227 ),       MM2MM100( 356 )     },          // 57 - SuperA/SuperA/A4 paper
    { MM2MM100( 305 ),       MM2MM100( 487 )     },          // 58 - SuperB/SuperB/A3 paper
    { IN2MM100( 8.5 ),       IN2MM100( 12.69 )   },          // 59 - Letter plus paper
    { MM2MM100( 210 ),       MM2MM100( 330 )     },          // 60 - A4 plus paper
    { MM2MM100( 148 ),       MM2MM100( 210 )     },          // 61 - A5 transverse paper
    { MM2MM100( 182 ),       MM2MM100( 257 )     },          // 62 - JIS B5 transverse paper
    { MM2MM100( 322 ),       MM2MM100( 445 )     },          // 63 - A3 extra paper
    { MM2MM100( 174 ),       MM2MM100( 235 )     },          // 64 - A5 extra paper
    { MM2MM100( 201 ),       MM2MM100( 276 )     },          // 65 - ISO B5 extra paper
    { MM2MM100( 420 ),       MM2MM100( 594 )     },          // 66 - A2 paper
    { MM2MM100( 297 ),       MM2MM100( 420 )     },          // 67 - A3 transverse paper
    { MM2MM100( 322 ),       MM2MM100( 445 )     },          // 68 - A3 extra transverse paper
    { MM2MM100( 200 ),       MM2MM100( 148 )     },          // 69 - Japanese double postcard
    { MM2MM100( 105 ),       MM2MM100( 148 ),    },          // 70 - A6 paper
    { 0, 0 },                                                // 71 - Japanese Envelope Kaku #2
    { 0, 0 },                                                // 72 - Japanese Envelope Kaku #3
    { 0, 0 },                                                // 73 - Japanese Envelope Chou #3
    { 0, 0 },                                                // 74 - Japanese Envelope Chou #4
    { IN2MM100( 11 ),        IN2MM100( 8.5 )     },          // 75 - Letter Rotated
    { MM2MM100( 420 ),       MM2MM100( 297 )     },          // 76 - A3 Rotated
    { MM2MM100( 297 ),       MM2MM100( 210 )     },          // 77 - A4 Rotated
    { MM2MM100( 210 ),       MM2MM100( 148 )     },          // 78 - A5 Rotated
    { MM2MM100( 364 ),       MM2MM100( 257 )     },          // 79 - B4 (JIS) Rotated
    { MM2MM100( 257 ),       MM2MM100( 182 )     },          // 80 - B5 (JIS) Rotated
    { MM2MM100( 148 ),       MM2MM100( 100 )     },          // 81 - Japanese Postcard Rotated
    { MM2MM100( 148 ),       MM2MM100( 200 )     },          // 82 - Double Japanese Postcard Rotated
    { MM2MM100( 148 ),       MM2MM100( 105 )     },          // 83 - A6 Rotated
    { 0, 0 },                                                // 84 - Japanese Envelope Kaku #2 Rotated
    { 0, 0 },                                                // 85 - Japanese Envelope Kaku #3 Rotated
    { 0, 0 },                                                // 86 - Japanese Envelope Chou #3 Rotated
    { 0, 0 },                                                // 87 - Japanese Envelope Chou #4 Rotated
    { MM2MM100( 128 ),       MM2MM100( 182 )     },          // 88 - B6 (JIS)
    { MM2MM100( 182 ),       MM2MM100( 128 )     },          // 89 - B6 (JIS) Rotated
    { IN2MM100( 12 ),        IN2MM100( 11 )      }           // 90 - 12x11
};

sal_Int32 PaperSizeConv::getMSPaperSizeIndex( const css::awt::Size& rSize )
{
    // Need to find the best match for current size
    sal_Int32 nDeltaWidth = 0;
    sal_Int32 nDeltaHeight = 0;

    sal_Int32 nPaperSizeIndex = 0; // Undefined
    const ApiPaperSize* pItem = spPaperSizeTable;
    const ApiPaperSize* pEnd =  spPaperSizeTable + std::size( spPaperSizeTable );
    for ( ; pItem != pEnd; ++pItem )
    {
        sal_Int32 nCurDeltaHeight = std::abs( pItem->mnHeight - rSize.Height );
        sal_Int32 nCurDeltaWidth = std::abs( pItem->mnWidth - rSize.Width );
        if ( pItem == spPaperSizeTable ) // initialize delta with first item
        {
            nDeltaWidth = nCurDeltaWidth;
            nDeltaHeight = nCurDeltaHeight;
        }
        else
        {
            if ( nCurDeltaWidth < nDeltaWidth && nCurDeltaHeight < nDeltaHeight )
            {
                nDeltaWidth = nCurDeltaWidth;
                nDeltaHeight = nCurDeltaHeight;
                nPaperSizeIndex = (pItem - spPaperSizeTable);
            }
        }
    }
    sal_Int32 nTol = 10; // hmm not sure is this the best way
    if ( nDeltaWidth <= nTol && nDeltaHeight <= nTol )
        return nPaperSizeIndex;
    return 0;
}

const ApiPaperSize& PaperSizeConv::getApiSizeForMSPaperSizeIndex( sal_Int32 nMSOPaperIndex )
{
    if ( nMSOPaperIndex  < 0 || nMSOPaperIndex > std::ssize( spPaperSizeTable ) - 1 )
        return spPaperSizeTable[ 0 ];
    return spPaperSizeTable[ nMSOPaperIndex ];
}

OUString CreateDOCXStyleId(std::u16string_view const aName)
{
    OUStringBuffer aStyleIdBuf(aName.size());
    for (size_t i = 0; i < aName.size(); ++i)
    {
        sal_Unicode nChar = aName[i];
        if (rtl::isAsciiAlphanumeric(nChar) || nChar == '-')
        {
            // do not uppercase first letter
            aStyleIdBuf.append(char(nChar));
        }
    }
    return aStyleIdBuf.makeStringAndClear();
}

std::u16string_view findQuotedText( std::u16string_view rCommand,
                std::u16string_view sStartQuote, const sal_Unicode uEndQuote )
{
    std::u16string_view sRet;
    size_t nStartIndex = rCommand.find( sStartQuote );
    if( nStartIndex != std::u16string_view::npos )
    {
        sal_Int32 nStartLength = sStartQuote.size();
        size_t nEndIndex = rCommand.find( uEndQuote, nStartIndex + nStartLength);
        if( nEndIndex != std::u16string_view::npos && nEndIndex > nStartIndex )
        {
            sRet = rCommand.substr( nStartIndex + nStartLength, nEndIndex - nStartIndex - nStartLength);
        }
    }
    return sRet;

}

WW8ReadFieldParams::WW8ReadFieldParams( OUString _aData )
    : aData(std::move( _aData ))
    , nFnd( 0 )
    , nNext( 0 )
    , nSavPtr( 0 )
{

    /*
        First look for an opening bracket or a space or a question mark or a backslash, so that the field (i.e. INCLUDEPICTURE or EINFUEGENGRAFIK or...) gets read over
    */
    const sal_Int32 nLen = aData.getLength();

    while ( nNext<nLen && aData[nNext]==' ' )
        ++nNext;

    while ( nNext<nLen )
    {
        const sal_Unicode c = aData[nNext];
        if ( c==' ' || c=='"' || c=='\\' || c==132 || c==0x201c )
            break;
        ++nNext;
    }

    nFnd      = nNext;
    nSavPtr   = nNext;
}

OUString WW8ReadFieldParams::GetResult() const
{
    if (nFnd<0 && nSavPtr>nFnd)
        return OUString();
    else
    {
        return nSavPtr < nFnd ? aData.copy(nFnd) : aData.copy(nFnd, nSavPtr-nFnd);
    }
}


bool WW8ReadFieldParams::GoToTokenParam()
{
    const sal_Int32 nOld = nNext;
    if( -2 == SkipToNextToken() )
        return GetTokenSttPtr()>=0;
    nNext = nOld;
    return false;
}

// ret: -2: NOT a '\' parameter but normal text
sal_Int32 WW8ReadFieldParams::SkipToNextToken()
{
    if ( nNext<0 || nNext>=aData.getLength() )
        return -1;

    nFnd = FindNextStringPiece(nNext);
    if ( nFnd<0 )
        return -1;

    nSavPtr = nNext;

    if (nFnd+1<aData.getLength() && aData[nFnd+1]!='\\' && aData[nFnd]=='\\')
    {
        const sal_Int32 nRet = aData[++nFnd];
        nNext = ++nFnd;             // and set after
        return nRet;
    }

    if ( nSavPtr>0 && (aData[nSavPtr-1]=='"' || aData[nSavPtr-1]==0x201d ) )
    {
        --nSavPtr;
    }
    return -2;
}

// FindNextPara searches the next backslash parameter or the next string
// until the next blank or "\" or closing quotation mark
// or the end of the string of pStr.
//
// Output ppNext (if ppNext != 0) search begin of next parameter resp. 0
//
// Return value: 0 if end of string reached,
//             otherwise beginning of the parameter resp. string
//
sal_Int32 WW8ReadFieldParams::FindNextStringPiece(const sal_Int32 nStart)
{
    const sal_Int32 nLen = aData.getLength();
    sal_Int32  n = nStart<0  ? nFnd : nStart;  // start
    sal_Int32 n2;          // end

    nNext = -1;        // if not found -> Default

    while ( n<nLen && aData[n]==' ' )
        ++n;

    if ( n==nLen )
        return -1;

    if ( aData[n]==0x13 )
    {
        // Skip the nested field code since it's not supported
        while ( n<nLen && aData[n]!=0x14 )
            ++n;
        if ( n==nLen )
            return -1;
    }

    // quotation marks before paragraph?
    if ( aData[n]=='"' || aData[n]==0x201c || aData[n]==132 || aData[n]==0x14 )
    {
        n++;                        // read over quotation marks
        n2 = n;                     // search for the end from here on
        while(     (nLen > n2)
                && (aData[n2] != '"')
                && (aData[n2] != 0x201d)
                && (aData[n2] != 147)
                && (aData[n2] != 0x15) )
            n2++;                   // search for the end of the paragraph
    }
    else                        // no quotation mark
    {
        n2 = n;                     // search for the end from here on
        while ( n2<nLen && aData[n2]!=' ' ) // search for the end of the paragraph
        {
            if ( aData[n2]=='\\' )
            {
                if ( n2+1<nLen && aData[n2+1]=='\\' )
                    n2 += 2;        // double backslash -> OK
                else
                {
                    if( n2 > n )
                        n2--;
                    break;          // single backslash -> end
                }
            }
            else
                n2++;               // no backslash -> OK
        }
    }
    if( nLen > n2 )
    {
        if (aData[n2]!=' ') ++n2;
        nNext = n2;
    }
    return n;
}


// read parameters "1-3" or 1-3 with both values between 1 and nMax
bool WW8ReadFieldParams::GetTokenSttFromTo(sal_Int32* pFrom, sal_Int32* pTo, sal_Int32 nMax)
{
    sal_Int32 nStart = 0;
    sal_Int32 nEnd   = 0;
    if ( GoToTokenParam() )
    {

        const OUString sParams( GetResult() );

        sal_Int32 nIndex = 0;
        const std::u16string_view sStart = o3tl::getToken(sParams, 0, '-', nIndex);
        if (nIndex>=0)
        {
            nStart = o3tl::toInt32(sStart);
            nEnd   = o3tl::toInt32(sParams.subView(nIndex));
        }
    }
    if( pFrom ) *pFrom = nStart;
    if( pTo )   *pTo   = nEnd;

    return nStart && nEnd && (nMax >= nStart) && (nMax >= nEnd);
}

static EquationResult Read_SubF_Combined(WW8ReadFieldParams& rReadParam)
{
    EquationResult aResult;

    OUString sCombinedCharacters;
    WW8ReadFieldParams aOriFldParam = rReadParam;
    const sal_Int32 cGetChar = rReadParam.SkipToNextToken();
    switch( cGetChar )
    {
    case 'a':
    case 'A':
        if ( !rReadParam.GetResult().startsWithIgnoreAsciiCase("d") )
        {
            break;
        }
        (void)rReadParam.SkipToNextToken();
        [[fallthrough]];
    case -2:
        {
            if ( rReadParam.GetResult().startsWithIgnoreAsciiCase("(") )
            {
                for (int i=0;i<2;i++)
                {
                    if ('s' == rReadParam.SkipToNextToken())
                    {
                        const sal_Int32 cChar = rReadParam.SkipToNextToken();
                        if (-2 != rReadParam.SkipToNextToken())
                            break;
                        const OUString sF = rReadParam.GetResult();
                        if ((('u' == cChar) && sF.startsWithIgnoreAsciiCase("p"))
                            || (('d' == cChar) && sF.startsWithIgnoreAsciiCase("o")))
                        {
                            if (-2 == rReadParam.SkipToNextToken())
                            {
                                OUString sPart = rReadParam.GetResult();
                                sal_Int32 nBegin = sPart.indexOf('(');

                                // Word disallows brackets in this field, which
                                // aids figuring out the case of an end of )) vs )
                                sal_Int32 nEnd = sPart.indexOf(')');

                                if (nBegin != -1 && nEnd != -1)
                                {
                                    sCombinedCharacters +=
                                        sPart.subView(nBegin+1,nEnd-nBegin-1);
                                }
                            }
                        }
                    }
                }
                if (!sCombinedCharacters.isEmpty())
                {
                    aResult.sType = "CombinedCharacters";
                    aResult.sResult = sCombinedCharacters;
                }
                else
                {
                    const OUString sPart = aOriFldParam.GetResult();
                    sal_Int32 nBegin = sPart.indexOf('(');
                    sal_Int32 nEnd = sPart.indexOf(',');
                    if ( nEnd == -1 )
                    {
                        nEnd = sPart.indexOf(')');
                    }
                    if ( nBegin != -1 && nEnd != -1 )
                    {
                        // skip certain leading characters
                        for (int i = nBegin;i < nEnd-1;i++)
                        {
                            const sal_Unicode cC = sPart[nBegin+1];
                            if ( cC < 32 )
                            {
                                nBegin++;
                            }
                            else
                                break;
                        }
                        sCombinedCharacters = sPart.copy( nBegin+1, nEnd-nBegin-1 );
                        if ( !sCombinedCharacters.isEmpty() )
                        {
                            aResult.sType = "Input";
                            aResult.sResult = sCombinedCharacters;
                        }
                    }
                }
            }
            break;
        }
    default:
        break;
    }
    return aResult;
}

EquationResult ParseCombinedChars(const OUString& rStr)
{
    EquationResult aResult;
    WW8ReadFieldParams aReadParam( rStr );
    const sal_Int32 cChar = aReadParam.SkipToNextToken();
    if ('o' == cChar || 'O' == cChar)
        aResult = Read_SubF_Combined(aReadParam);
    return aResult;
}

OString GetOOXMLPresetGeometry( std::u16string_view rShapeType )
{
    typedef std::unordered_map<std::u16string_view, OString> CustomShapeTypeTranslationHashMap;
    static const CustomShapeTypeTranslationHashMap aCustomShapeTypeTranslationHashMap{
        // { "non-primitive", mso_sptMin },
        { u"frame", "frame" },
        { u"rectangle", "rect" },
        { u"round-rectangle", "roundRect" },
        { u"ellipse", "ellipse" },
        { u"diamond", "diamond" },
        { u"isosceles-triangle", "triangle" },
        { u"right-triangle", "rtTriangle" },
        { u"parallelogram", "parallelogram" },
        { u"trapezoid", "trapezoid" },
        { u"hexagon", "hexagon" },
        { u"octagon", "octagon" },
        { u"cross", "plus" },
        { u"star5", "star5" },
        { u"right-arrow", "rightArrow" },
        // { u"mso-spt14", mso_sptThickArrow },
        { u"pentagon-right", "homePlate" },
        { u"cube", "cube" },
        // { u"mso-spt17", mso_sptBalloon },
        // { u"mso-spt18", mso_sptSeal },
        { u"mso-spt19", "arc" },
        { u"mso-spt20", "line" },
        { u"mso-spt21", "plaque" },
        { u"can", "can" },
        { u"ring", "donut" },
        { u"mso-spt24", "textPlain" },
        { u"mso-spt25", "textStop" },
        { u"mso-spt26", "textTriangle" },
        { u"mso-spt27", "textCanDown" },
        { u"mso-spt28", "textWave1" },
        { u"mso-spt29", "textArchUpPour" },
        { u"mso-spt30", "textCanDown" },
        { u"mso-spt31", "textArchUp" },
        { u"mso-spt32", "straightConnector1" },
        { u"mso-spt33", "bentConnector2" },
        { u"mso-spt34", "bentConnector3" },
        { u"mso-spt35", "bentConnector4" },
        { u"mso-spt36", "bentConnector5" },
        { u"mso-spt37", "curvedConnector2" },
        { u"mso-spt38", "curvedConnector3" },
        { u"mso-spt39", "curvedConnector4" },
        { u"mso-spt40", "curvedConnector5" },
        { u"mso-spt41", "callout1" },
        { u"mso-spt42", "callout2" },
        { u"mso-spt43", "callout3" },
        { u"mso-spt44", "accentCallout1" },
        { u"mso-spt45", "accentCallout2" },
        { u"mso-spt46", "accentCallout3" },
        { u"line-callout-1", "borderCallout1" },
        { u"line-callout-2", "borderCallout2" },
        { u"line-callout-3", "borderCallout3" },
        { u"mso-spt49", "borderCallout3" },
        { u"mso-spt50", "accentBorderCallout1" },
        { u"mso-spt51", "accentBorderCallout2" },
        { u"mso-spt52", "accentBorderCallout3" },
        { u"mso-spt53", "ribbon" },
        { u"mso-spt54", "ribbon2" },
        { u"chevron", "chevron" },
        { u"pentagon", "pentagon" },
        { u"forbidden", "noSmoking" },
        { u"star8", "star8" },
        { u"mso-spt59", "star16" },
        { u"mso-spt60", "star32" },
        { u"rectangular-callout", "wedgeRectCallout" },
        { u"round-rectangular-callout", "wedgeRoundRectCallout" },
        { u"round-callout", "wedgeEllipseCallout" },
        { u"mso-spt64", "wave" },
        { u"paper", "foldedCorner" },
        { u"left-arrow", "leftArrow" },
        { u"down-arrow", "downArrow" },
        { u"up-arrow", "upArrow" },
        { u"left-right-arrow", "leftRightArrow" },
        { u"up-down-arrow", "upDownArrow" },
        { u"mso-spt71", "irregularSeal1" },
        { u"bang", "irregularSeal2" },
        { u"lightning", "lightningBolt" },
        { u"heart", "heart" },
        { u"quad-arrow", "quadArrow" },
        { u"left-arrow-callout", "leftArrowCallout" },
        { u"right-arrow-callout", "rightArrowCallout" },
        { u"up-arrow-callout", "upArrowCallout" },
        { u"down-arrow-callout", "downArrowCallout" },
        { u"left-right-arrow-callout", "leftRightArrowCallout" },
        { u"up-down-arrow-callout", "upDownArrowCallout" },
        { u"quad-arrow-callout", "quadArrowCallout" },
        { u"quad-bevel", "bevel" },
        { u"left-bracket", "leftBracket" },
        { u"right-bracket", "rightBracket" },
        { u"left-brace", "leftBrace" },
        { u"right-brace", "rightBrace" },
        { u"mso-spt89", "leftUpArrow" },
        { u"mso-spt90", "bentUpArrow" },
        { u"mso-spt91", "bentArrow" },
        { u"star24", "star24" },
        { u"striped-right-arrow", "stripedRightArrow" },
        { u"notched-right-arrow", "notchedRightArrow" },
        { u"block-arc", "blockArc" },
        { u"smiley", "smileyFace" },
        { u"vertical-scroll", "verticalScroll" },
        { u"horizontal-scroll", "horizontalScroll" },
        { u"circular-arrow", "circularArrow" },
        { u"mso-spt100", "pie" }, // looks like MSO_SPT is wrong here
        { u"mso-spt101", "uturnArrow" },
        { u"mso-spt102", "curvedRightArrow" },
        { u"mso-spt103", "curvedLeftArrow" },
        { u"mso-spt104", "curvedUpArrow" },
        { u"mso-spt105", "curvedDownArrow" },
        { u"cloud-callout", "cloudCallout" },
        { u"mso-spt107", "ellipseRibbon" },
        { u"mso-spt108", "ellipseRibbon2" },
        { u"flowchart-process", "flowChartProcess" },
        { u"flowchart-decision", "flowChartDecision" },
        { u"flowchart-data", "flowChartInputOutput" },
        { u"flowchart-predefined-process", "flowChartPredefinedProcess" },
        { u"flowchart-internal-storage", "flowChartInternalStorage" },
        { u"flowchart-document", "flowChartDocument" },
        { u"flowchart-multidocument", "flowChartMultidocument" },
        { u"flowchart-terminator", "flowChartTerminator" },
        { u"flowchart-preparation", "flowChartPreparation" },
        { u"flowchart-manual-input", "flowChartManualInput" },
        { u"flowchart-manual-operation", "flowChartManualOperation" },
        { u"flowchart-connector", "flowChartConnector" },
        { u"flowchart-card", "flowChartPunchedCard" },
        { u"flowchart-punched-tape", "flowChartPunchedTape" },
        { u"flowchart-summing-junction", "flowChartSummingJunction" },
        { u"flowchart-or", "flowChartOr" },
        { u"flowchart-collate", "flowChartCollate" },
        { u"flowchart-sort", "flowChartSort" },
        { u"flowchart-extract", "flowChartExtract" },
        { u"flowchart-merge", "flowChartMerge" },
        { u"mso-spt129", "flowChartOfflineStorage" },
        { u"flowchart-stored-data", "flowChartOnlineStorage" },
        { u"flowchart-sequential-access", "flowChartMagneticTape" },
        { u"flowchart-magnetic-disk", "flowChartMagneticDisk" },
        { u"flowchart-direct-access-storage", "flowChartMagneticDrum" },
        { u"flowchart-display", "flowChartDisplay" },
        { u"flowchart-delay", "flowChartDelay" },
        // { u"fontwork-plain-text", "textPlainText" },
        // { u"fontwork-stop", "textStop" },
        // { u"fontwork-triangle-up", "textTriangle" },
        // { u"fontwork-triangle-down", "textTriangleInverted" },
        // { u"fontwork-chevron-up", "textChevron" },
        // { u"fontwork-chevron-down", "textChevronInverted" },
        // { u"mso-spt142", "textRingInside" },
        // { u"mso-spt143", "textRingOutside" },
        // { u"fontwork-arch-up-curve", "textArchUpCurve" },
        // { u"fontwork-arch-down-curve", "textArchDownCurve" },
        // { u"fontwork-circle-curve", "textCircleCurve" },
        // { u"fontwork-open-circle-curve", "textButtonCurve" },
        // { u"fontwork-arch-up-pour", "textArchUpPour" },
        // { u"fontwork-arch-down-pour", "textArchDownPour" },
        // { u"fontwork-circle-pour", "textCirclePour" },
        // { u"fontwork-open-circle-pour", "textButtonPour" },
        // { u"fontwork-curve-up", "textCurveUp" },
        // { u"fontwork-curve-down", "textCurveDown" },
        // { u"fontwork-fade-up-and-right", "textCascadeUp" },
        // { u"fontwork-fade-up-and-left", "textCascadeDown" },
        // { u"fontwork-wave", "textWave1" },
        // { u"mso-spt157", "textWave2" },
        // { u"mso-spt158", "textWave3" },
        // { u"mso-spt159", "textWave4" },
        // { u"fontwork-inflate", "textInflate" },
        // { u"mso-spt161", "textDeflate" },
        // { u"mso-spt162", "textInflateBottom" },
        // { u"mso-spt163", "textDeflateBottom" },
        // { u"mso-spt164", "textInflateTop" },
        // { u"mso-spt165", "textDeflateTop" },
        // { u"mso-spt166", "textDeflateInflate" },
        // { u"mso-spt167", "textDeflateInflateDeflate" },
        // { u"fontwork-fade-right", "textFadeRight" },
        // { u"fontwork-fade-left", "textFadeLeft" },
        // { u"fontwork-fade-up", "textFadeUp" },
        // { u"fontwork-fade-down", "textFadeDown" },
        // { u"fontwork-slant-up", "textSlantUp" },
        // { u"fontwork-slant-down", "textSlantDown" },
        // { u"mso-spt174", "textCanUp" },
        // { u"mso-spt175", "textCanDown" },
        { u"flowchart-alternate-process", "flowChartAlternateProcess" },
        { u"flowchart-off-page-connector", "flowChartOffpageConnector" },
        { u"mso-spt178", "callout1" },
        { u"mso-spt179", "accentCallout1" },
        { u"mso-spt180", "borderCallout1" },
        { u"mso-spt182", "leftRightUpArrow" },
        { u"sun", "sun" },
        { u"moon", "moon" },
        { u"bracket-pair", "bracketPair" },
        { u"brace-pair", "bracePair" },
        { u"star4", "star4" },
        { u"mso-spt188", "doubleWave" },
        { u"mso-spt189", "actionButtonBlank" },
        { u"mso-spt190", "actionButtonHome" },
        { u"mso-spt191", "actionButtonHelp" },
        { u"mso-spt192", "actionButtonInformation" },
        { u"mso-spt193", "actionButtonForwardNext" },
        { u"mso-spt194", "actionButtonBackPrevious" },
        { u"mso-spt195", "actionButtonEnd" },
        { u"mso-spt196", "actionButtonBeginning" },
        { u"mso-spt197", "actionButtonReturn" },
        { u"mso-spt198", "actionButtonDocument" },
        { u"mso-spt199", "actionButtonSound" },
        { u"mso-spt200", "actionButtonMovie" },
        // { u"mso-spt201", "hostControl" },
        { u"mso-spt202", "rect" },
        { u"ooxml-actionButtonSound", "actionButtonSound" },
        { u"ooxml-borderCallout1", "borderCallout1" },
        { u"ooxml-plaqueTabs", "plaqueTabs" },
        { u"ooxml-curvedLeftArrow", "curvedLeftArrow" },
        { u"ooxml-octagon", "octagon" },
        { u"ooxml-leftRightRibbon", "leftRightRibbon" },
        { u"ooxml-actionButtonInformation", "actionButtonInformation" },
        { u"ooxml-bentConnector5", "bentConnector5" },
        { u"ooxml-circularArrow", "circularArrow" },
        { u"ooxml-downArrowCallout", "downArrowCallout" },
        { u"ooxml-mathMinus", "mathMinus" },
        { u"ooxml-gear9", "gear9" },
        { u"ooxml-round1Rect", "round1Rect" },
        { u"ooxml-sun", "sun" },
        { u"ooxml-plaque", "plaque" },
        { u"ooxml-chevron", "chevron" },
        { u"ooxml-flowChartPreparation", "flowChartPreparation" },
        { u"ooxml-diagStripe", "diagStripe" },
        { u"ooxml-pentagon", "pentagon" },
        { u"ooxml-funnel", "funnel" },
        { u"ooxml-chartStar", "chartStar" },
        { u"ooxml-accentBorderCallout1", "accentBorderCallout1" },
        { u"ooxml-notchedRightArrow", "notchedRightArrow" },
        { u"ooxml-rightBracket", "rightBracket" },
        { u"ooxml-flowChartOffpageConnector", "flowChartOffpageConnector" },
        { u"ooxml-leftRightArrow", "leftRightArrow" },
        { u"ooxml-decagon", "decagon" },
        { u"ooxml-actionButtonHelp", "actionButtonHelp" },
        { u"ooxml-star24", "star24" },
        { u"ooxml-mathDivide", "mathDivide" },
        { u"ooxml-curvedConnector4", "curvedConnector4" },
        { u"ooxml-flowChartOr", "flowChartOr" },
        { u"ooxml-borderCallout3", "borderCallout3" },
        { u"ooxml-upDownArrowCallout", "upDownArrowCallout" },
        { u"ooxml-flowChartDecision", "flowChartDecision" },
        { u"ooxml-leftRightArrowCallout", "leftRightArrowCallout" },
        { u"ooxml-flowChartManualOperation", "flowChartManualOperation" },
        { u"ooxml-snipRoundRect", "snipRoundRect" },
        { u"ooxml-mathPlus", "mathPlus" },
        { u"ooxml-actionButtonForwardNext", "actionButtonForwardNext" },
        { u"ooxml-can", "can" },
        { u"ooxml-foldedCorner", "foldedCorner" },
        { u"ooxml-star32", "star32" },
        { u"ooxml-flowChartInternalStorage", "flowChartInternalStorage" },
        { u"ooxml-upDownArrow", "upDownArrow" },
        { u"ooxml-irregularSeal2", "irregularSeal2" },
        { u"ooxml-mathEqual", "mathEqual" },
        { u"ooxml-star12", "star12" },
        { u"ooxml-uturnArrow", "uturnArrow" },
        { u"ooxml-squareTabs", "squareTabs" },
        { u"ooxml-leftRightUpArrow", "leftRightUpArrow" },
        { u"ooxml-homePlate", "homePlate" },
        { u"ooxml-dodecagon", "dodecagon" },
        { u"ooxml-leftArrowCallout", "leftArrowCallout" },
        { u"ooxml-chord", "chord" },
        { u"ooxml-quadArrowCallout", "quadArrowCallout" },
        { u"ooxml-actionButtonBeginning", "actionButtonBeginning" },
        { u"ooxml-ellipse", "ellipse" },
        { u"ooxml-actionButtonEnd", "actionButtonEnd" },
        { u"ooxml-arc", "arc" },
        { u"ooxml-star16", "star16" },
        { u"ooxml-parallelogram", "parallelogram" },
        { u"ooxml-bevel", "bevel" },
        { u"ooxml-roundRect", "roundRect" },
        { u"ooxml-accentCallout1", "accentCallout1" },
        { u"ooxml-flowChartSort", "flowChartSort" },
        { u"ooxml-star8", "star8" },
        { u"ooxml-flowChartAlternateProcess", "flowChartAlternateProcess" },
        { u"ooxml-moon", "moon" },
        { u"ooxml-star6", "star6" },
        { u"ooxml-round2SameRect", "round2SameRect" },
        { u"ooxml-nonIsoscelesTrapezoid", "nonIsoscelesTrapezoid" },
        { u"ooxml-diamond", "diamond" },
        { u"ooxml-ellipseRibbon", "ellipseRibbon" },
        { u"ooxml-callout2", "callout2" },
        { u"ooxml-pie", "pie" },
        { u"ooxml-star4", "star4" },
        { u"ooxml-flowChartPredefinedProcess", "flowChartPredefinedProcess" },
        { u"ooxml-flowChartPunchedTape", "flowChartPunchedTape" },
        { u"ooxml-curvedConnector2", "curvedConnector2" },
        { u"ooxml-bentConnector3", "bentConnector3" },
        { u"ooxml-cornerTabs", "cornerTabs" },
        { u"ooxml-hexagon", "hexagon" },
        { u"ooxml-flowChartConnector", "flowChartConnector" },
        { u"ooxml-flowChartMagneticDisk", "flowChartMagneticDisk" },
        { u"ooxml-heart", "heart" },
        { u"ooxml-ribbon2", "ribbon2" },
        { u"ooxml-bracePair", "bracePair" },
        { u"ooxml-flowChartExtract", "flowChartExtract" },
        { u"ooxml-actionButtonHome", "actionButtonHome" },
        { u"ooxml-accentBorderCallout3", "accentBorderCallout3" },
        { u"ooxml-flowChartOfflineStorage", "flowChartOfflineStorage" },
        { u"ooxml-irregularSeal1", "irregularSeal1" },
        { u"ooxml-quadArrow", "quadArrow" },
        { u"ooxml-leftBrace", "leftBrace" },
        { u"ooxml-leftBracket", "leftBracket" },
        { u"ooxml-blockArc", "blockArc" },
        { u"ooxml-curvedConnector3", "curvedConnector3" },
        { u"ooxml-wedgeRoundRectCallout", "wedgeRoundRectCallout" },
        { u"ooxml-actionButtonMovie", "actionButtonMovie" },
        { u"ooxml-flowChartOnlineStorage", "flowChartOnlineStorage" },
        { u"ooxml-gear6", "gear6" },
        { u"ooxml-halfFrame", "halfFrame" },
        { u"ooxml-snip2SameRect", "snip2SameRect" },
        { u"ooxml-triangle", "triangle" },
        { u"ooxml-teardrop", "teardrop" },
        { u"ooxml-flowChartDocument", "flowChartDocument" },
        { u"ooxml-rightArrowCallout", "rightArrowCallout" },
        { u"ooxml-rightBrace", "rightBrace" },
        { u"ooxml-chartPlus", "chartPlus" },
        { u"ooxml-flowChartManualInput", "flowChartManualInput" },
        { u"ooxml-flowChartMerge", "flowChartMerge" },
        { u"ooxml-line", "line" },
        { u"ooxml-downArrow", "downArrow" },
        { u"ooxml-upArrow", "upArrow" },
        { u"ooxml-curvedDownArrow", "curvedDownArrow" },
        { u"ooxml-actionButtonReturn", "actionButtonReturn" },
        { u"ooxml-flowChartInputOutput", "flowChartInputOutput" },
        { u"ooxml-bracketPair", "bracketPair" },
        { u"ooxml-smileyFace", "smileyFace" },
        { u"ooxml-actionButtonBlank", "actionButtonBlank" },
        { u"ooxml-wave", "wave" },
        { u"ooxml-swooshArrow", "swooshArrow" },
        { u"ooxml-flowChartSummingJunction", "flowChartSummingJunction" },
        { u"ooxml-lightningBolt", "lightningBolt" },
        { u"ooxml-flowChartDisplay", "flowChartDisplay" },
        { u"ooxml-actionButtonBackPrevious", "actionButtonBackPrevious" },
        { u"ooxml-frame", "frame" },
        { u"ooxml-rtTriangle", "rtTriangle" },
        { u"ooxml-flowChartMagneticTape", "flowChartMagneticTape" },
        { u"ooxml-curvedRightArrow", "curvedRightArrow" },
        { u"ooxml-leftUpArrow", "leftUpArrow" },
        { u"ooxml-wedgeEllipseCallout", "wedgeEllipseCallout" },
        { u"ooxml-doubleWave", "doubleWave" },
        { u"ooxml-bentArrow", "bentArrow" },
        { u"ooxml-star10", "star10" },
        { u"ooxml-leftArrow", "leftArrow" },
        { u"ooxml-curvedUpArrow", "curvedUpArrow" },
        { u"ooxml-snip1Rect", "snip1Rect" },
        { u"ooxml-ellipseRibbon2", "ellipseRibbon2" },
        { u"ooxml-plus", "plus" },
        { u"ooxml-accentCallout3", "accentCallout3" },
        { u"ooxml-leftCircularArrow", "leftCircularArrow" },
        { u"ooxml-rightArrow", "rightArrow" },
        { u"ooxml-flowChartPunchedCard", "flowChartPunchedCard" },
        { u"ooxml-snip2DiagRect", "snip2DiagRect" },
        { u"ooxml-verticalScroll", "verticalScroll" },
        { u"ooxml-star7", "star7" },
        { u"ooxml-chartX", "chartX" },
        { u"ooxml-cloud", "cloud" },
        { u"ooxml-cube", "cube" },
        { u"ooxml-round2DiagRect", "round2DiagRect" },
        { u"ooxml-flowChartMultidocument", "flowChartMultidocument" },
        { u"ooxml-actionButtonDocument", "actionButtonDocument" },
        { u"ooxml-flowChartTerminator", "flowChartTerminator" },
        { u"ooxml-flowChartDelay", "flowChartDelay" },
        { u"ooxml-curvedConnector5", "curvedConnector5" },
        { u"ooxml-horizontalScroll", "horizontalScroll" },
        { u"ooxml-bentConnector4", "bentConnector4" },
        { u"ooxml-leftRightCircularArrow", "leftRightCircularArrow" },
        { u"ooxml-wedgeRectCallout", "wedgeRectCallout" },
        { u"ooxml-accentCallout2", "accentCallout2" },
        { u"ooxml-flowChartMagneticDrum", "flowChartMagneticDrum" },
        { u"ooxml-corner", "corner" },
        { u"ooxml-borderCallout2", "borderCallout2" },
        { u"ooxml-donut", "donut" },
        { u"ooxml-flowChartCollate", "flowChartCollate" },
        { u"ooxml-mathNotEqual", "mathNotEqual" },
        { u"ooxml-bentConnector2", "bentConnector2" },
        { u"ooxml-mathMultiply", "mathMultiply" },
        { u"ooxml-heptagon", "heptagon" },
        { u"ooxml-rect", "rect" },
        { u"ooxml-accentBorderCallout2", "accentBorderCallout2" },
        { u"ooxml-pieWedge", "pieWedge" },
        { u"ooxml-upArrowCallout", "upArrowCallout" },
        { u"ooxml-flowChartProcess", "flowChartProcess" },
        { u"ooxml-star5", "star5" },
        { u"ooxml-lineInv", "lineInv" },
        { u"ooxml-straightConnector1", "straightConnector1" },
        { u"ooxml-stripedRightArrow", "stripedRightArrow" },
        { u"ooxml-callout3", "callout3" },
        { u"ooxml-bentUpArrow", "bentUpArrow" },
        { u"ooxml-noSmoking", "noSmoking" },
        { u"ooxml-trapezoid", "trapezoid" },
        { u"ooxml-cloudCallout", "cloudCallout" },
        { u"ooxml-callout1", "callout1" },
        { u"ooxml-ribbon", "ribbon" },
        { u"ooxml-rect", "rect" },
    };
    auto i(aCustomShapeTypeTranslationHashMap.find(rShapeType));
    return i == aCustomShapeTypeTranslationHashMap.end() ? "rect"_ostr : i->second;
}

MSO_SPT GETVMLShapeType(std::u16string_view aType)
{
    typedef std::unordered_map<std::string_view, MSO_SPT> DMLToVMLTranslationHashMap;
    static const DMLToVMLTranslationHashMap aDMLToVMLMap{
        {"notPrimitive", mso_sptNotPrimitive},
        {"rectangle", mso_sptRectangle},
        {"roundRectangle", mso_sptRoundRectangle},
        {"ellipse", mso_sptEllipse},
        {"diamond", mso_sptDiamond},
        {"triangle", mso_sptIsocelesTriangle},
        {"rtTriangle", mso_sptRightTriangle},
        {"parallelogram", mso_sptParallelogram},
        {"trapezoid", mso_sptTrapezoid},
        {"hexagon", mso_sptHexagon},
        {"octagon", mso_sptOctagon},
        {"plus", mso_sptPlus},
        {"star5", mso_sptStar},
        {"rightArrow", mso_sptArrow},
        {"thickArrow", mso_sptThickArrow},
        {"homePlate", mso_sptHomePlate},
        {"cube", mso_sptCube},
        {"wedgeRoundRectCallout", mso_sptBalloon},
        {"star16", mso_sptSeal},
        {"arc", mso_sptArc},
        {"line", mso_sptLine},
        {"plaque", mso_sptPlaque},
        {"can", mso_sptCan},
        {"donut", mso_sptDonut},
        {"textPlain", mso_sptTextSimple},
        {"textStop", mso_sptTextOctagon},
        {"textTriangle", mso_sptTextHexagon},
        {"textCanDown", mso_sptTextCurve},
        {"textWave1", mso_sptTextWave},
        {"textArchUpPour", mso_sptTextRing},
        {"textCanDown", mso_sptTextOnCurve},
        {"textArchUp", mso_sptTextOnRing},
        {"straightConnector1", mso_sptStraightConnector1},
        {"bentConnector2", mso_sptBentConnector2},
        {"bentConnector3", mso_sptBentConnector3},
        {"bentConnector4", mso_sptBentConnector4},
        {"bentConnector5", mso_sptBentConnector5},
        {"curvedConnector2", mso_sptCurvedConnector2},
        {"curvedConnector3", mso_sptCurvedConnector3},
        {"curvedConnector4", mso_sptCurvedConnector4},
        {"curvedConnector5", mso_sptCurvedConnector5},
        {"callout1", mso_sptCallout1},
        {"callout2", mso_sptCallout2},
        {"callout3", mso_sptCallout3},
        {"accentCallout1", mso_sptAccentCallout1},
        {"accentCallout2", mso_sptAccentCallout2},
        {"accentCallout3", mso_sptAccentCallout3},
        {"borderCallout1", mso_sptBorderCallout1},
        {"borderCallout2", mso_sptBorderCallout2},
        {"borderCallout3", mso_sptBorderCallout3},
        {"accentBorderCallout1", mso_sptAccentBorderCallout1},
        {"accentBorderCallout2", mso_sptAccentBorderCallout2},
        {"accentBorderCallout3", mso_sptAccentBorderCallout3},
        {"ribbon", mso_sptRibbon},
        {"ribbon2", mso_sptRibbon2},
        {"chevron", mso_sptChevron},
        {"pentagon", mso_sptPentagon},
        {"noSmoking", mso_sptNoSmoking},
        {"star8", mso_sptSeal8},
        {"star16", mso_sptSeal16},
        {"star32", mso_sptSeal32},
        {"wedgeRectCallout", mso_sptWedgeRectCallout},
        {"wedgeRoundRectCallout", mso_sptWedgeRRectCallout},
        {"wedgeEllipseCallout", mso_sptWedgeEllipseCallout},
        {"wave", mso_sptWave},
        {"foldedCorner", mso_sptFoldedCorner},
        {"leftArrow", mso_sptLeftArrow},
        {"downArrow", mso_sptDownArrow},
        {"upArrow", mso_sptUpArrow},
        {"leftRightArrow", mso_sptLeftRightArrow},
        {"upDownArrow", mso_sptUpDownArrow},
        {"irregularSeal1", mso_sptIrregularSeal1},
        {"irregularSeal2", mso_sptIrregularSeal2},
        {"lightningBolt", mso_sptLightningBolt},
        {"heart", mso_sptHeart},
        {"pictureFrame", mso_sptPictureFrame},
        {"quadArrow", mso_sptQuadArrow},
        {"leftArrowCallout", mso_sptLeftArrowCallout},
        {"rightArrowCallout", mso_sptRightArrowCallout},
        {"upArrowCallout", mso_sptUpArrowCallout},
        {"downArrowCallout", mso_sptDownArrowCallout},
        {"leftRightArrowCallout", mso_sptLeftRightArrowCallout},
        {"upDownArrowCallout", mso_sptUpDownArrowCallout},
        {"quadArrowCallout", mso_sptQuadArrowCallout},
        {"bevel", mso_sptBevel},
        {"leftBracket", mso_sptLeftBracket},
        {"rightBracket", mso_sptRightBracket},
        {"leftBrace", mso_sptLeftBrace},
        {"rightBrace", mso_sptRightBrace},
        {"leftUpArrow", mso_sptLeftUpArrow},
        {"bentUpArrow", mso_sptBentUpArrow},
        {"bentArrow", mso_sptBentArrow},
        {"star24", mso_sptSeal24},
        {"stripedRightArrow", mso_sptStripedRightArrow},
        {"notchedRightArrow", mso_sptNotchedRightArrow},
        {"blockArc", mso_sptBlockArc},
        {"smileyFace", mso_sptSmileyFace},
        {"verticalScroll", mso_sptVerticalScroll},
        {"horizontalScroll", mso_sptHorizontalScroll},
        {"circularArrow", mso_sptCircularArrow},
        {"notchedCircularArrow", mso_sptNotchedCircularArrow},
        {"uturnArrow", mso_sptUturnArrow},
        {"curvedRightArrow", mso_sptCurvedRightArrow},
        {"curvedLeftArrow", mso_sptCurvedLeftArrow},
        {"curvedUpArrow", mso_sptCurvedUpArrow},
        {"curvedDownArrow", mso_sptCurvedDownArrow},
        {"cloudCallout", mso_sptCloudCallout},
        {"ellipseRibbon", mso_sptEllipseRibbon},
        {"ellipseRibbon2", mso_sptEllipseRibbon2},
        {"flowChartProcess", mso_sptFlowChartProcess},
        {"flowChartDecision", mso_sptFlowChartDecision},
        {"flowChartInputOutput", mso_sptFlowChartInputOutput},
        {"flowChartPredefinedProcess", mso_sptFlowChartPredefinedProcess},
        {"flowChartInternalStorage", mso_sptFlowChartInternalStorage},
        {"flowChartDocument", mso_sptFlowChartDocument},
        {"flowChartMultidocument", mso_sptFlowChartMultidocument},
        {"flowChartTerminator", mso_sptFlowChartTerminator},
        {"flowChartPreparation", mso_sptFlowChartPreparation},
        {"flowChartManualInput", mso_sptFlowChartManualInput},
        {"flowChartManualOperation", mso_sptFlowChartManualOperation},
        {"flowChartConnector", mso_sptFlowChartConnector},
        {"flowChartPunchedCard", mso_sptFlowChartPunchedCard},
        {"flowChartPunchedTape", mso_sptFlowChartPunchedTape},
        {"flowChartSummingJunction", mso_sptFlowChartSummingJunction},
        {"flowChartOr", mso_sptFlowChartOr},
        {"flowChartCollate", mso_sptFlowChartCollate},
        {"flowChartSort", mso_sptFlowChartSort},
        {"flowChartExtract", mso_sptFlowChartExtract},
        {"flowChartMerge", mso_sptFlowChartMerge},
        {"flowChartOfflineStorage", mso_sptFlowChartOfflineStorage},
        {"flowChartOnlineStorage", mso_sptFlowChartOnlineStorage},
        {"flowChartMagneticTape", mso_sptFlowChartMagneticTape},
        {"flowChartMagneticDisk", mso_sptFlowChartMagneticDisk},
        {"flowChartMagneticDrum", mso_sptFlowChartMagneticDrum},
        {"flowChartDisplay", mso_sptFlowChartDisplay},
        {"flowChartDelay", mso_sptFlowChartDelay},
        {"textPlain", mso_sptTextPlainText},
        {"textStop", mso_sptTextStop},
        {"textTriangle", mso_sptTextTriangle},
        {"textTriangleInverted", mso_sptTextTriangleInverted},
        {"textChevron", mso_sptTextChevron},
        {"textChevronInverted", mso_sptTextChevronInverted},
        {"textRingInside", mso_sptTextRingInside},
        {"textRingOutside", mso_sptTextRingOutside},
        {"textArchUp", mso_sptTextArchUpCurve},
        {"textArchDown", mso_sptTextArchDownCurve},
        {"textCircle", mso_sptTextCircleCurve},
        {"textButton", mso_sptTextButtonCurve},
        {"textArchUpPour", mso_sptTextArchUpPour},
        {"textArchDownPour", mso_sptTextArchDownPour},
        {"textCirclePour", mso_sptTextCirclePour},
        {"textButtonPour", mso_sptTextButtonPour},
        {"textCurveUp", mso_sptTextCurveUp},
        {"textCurveDown", mso_sptTextCurveDown},
        {"textCascadeUp", mso_sptTextCascadeUp},
        {"textCascadeDown", mso_sptTextCascadeDown},
        {"textWave1", mso_sptTextWave1},
        {"textWave2", mso_sptTextWave2},
        {"textWave3", mso_sptTextWave3},
        {"textWave4", mso_sptTextWave4},
        {"textInflate", mso_sptTextInflate},
        {"textDeflate", mso_sptTextDeflate},
        {"textInflateBottom", mso_sptTextInflateBottom},
        {"textDeflateBottom", mso_sptTextDeflateBottom},
        {"textInflateTop", mso_sptTextInflateTop},
        {"textDeflateTop", mso_sptTextDeflateTop},
        {"textDeflateInflate", mso_sptTextDeflateInflate},
        {"textDeflateInflateDeflate", mso_sptTextDeflateInflateDeflate},
        {"textFadeRight", mso_sptTextFadeRight},
        {"textFadeLeft", mso_sptTextFadeLeft},
        {"textFadeUp", mso_sptTextFadeUp},
        {"textFadeDown", mso_sptTextFadeDown},
        {"textSlantUp", mso_sptTextSlantUp},
        {"textSlantDown", mso_sptTextSlantDown},
        {"textCanUp", mso_sptTextCanUp},
        {"textCanDown", mso_sptTextCanDown},
        {"flowChartAlternateProcess", mso_sptFlowChartAlternateProcess},
        {"flowChartOffpageConnector", mso_sptFlowChartOffpageConnector},
        {"callout1", mso_sptCallout90},
        {"accentCallout1", mso_sptAccentCallout90},
        {"borderCallout1", mso_sptBorderCallout90},
        {"accentBorderCallout1", mso_sptAccentBorderCallout90},
        {"leftRightUpArrow", mso_sptLeftRightUpArrow},
        {"sun", mso_sptSun},
        {"moon", mso_sptMoon},
        {"bracketPair", mso_sptBracketPair},
        {"bracePair", mso_sptBracePair},
        {"star4", mso_sptSeal4},
        {"doubleWave", mso_sptDoubleWave},
        {"actionButtonBlank", mso_sptActionButtonBlank},
        {"actionButtonHome", mso_sptActionButtonHome},
        {"actionButtonHelp", mso_sptActionButtonHelp},
        {"actionButtonInformation", mso_sptActionButtonInformation},
        {"actionButtonForwardNext", mso_sptActionButtonForwardNext},
        {"actionButtonBackPrevious", mso_sptActionButtonBackPrevious},
        {"actionButtonEnd", mso_sptActionButtonEnd},
        {"actionButtonBeginning", mso_sptActionButtonBeginning},
        {"actionButtonReturn", mso_sptActionButtonReturn},
        {"actionButtonDocument", mso_sptActionButtonDocument},
        {"actionButtonSound", mso_sptActionButtonSound},
        {"actionButtonMovie", mso_sptActionButtonMovie},
        {"hostControl", mso_sptHostControl},
        {"textBox", mso_sptTextBox},
    };

    auto i(aDMLToVMLMap.find(GetOOXMLPresetGeometry(aType)));
    return i == aDMLToVMLMap.end() ? mso_sptNil : i->second;
}

bool HasTextBoxContent(sal_uInt32 nShapeType)
{
    switch (nShapeType)
    {
    case ESCHER_ShpInst_TextPlainText:
    case ESCHER_ShpInst_TextSlantUp:
    case ESCHER_ShpInst_TextDeflateInflateDeflate:
        return false;
    default:
        return true;
    }
}

namespace
{

// Scheme means pattern of chromatic values.
// [2,2,1] -> red and green are approximately equal and blue is the dominant color (e.g. blue)
// [1,1,1] -> all chromatic values are approximately equal (e.g. white, gray, black)
void CalculateScheme(const BitmapColor& rBitmapColor, std::vector<int> &vScheme, sal_uInt16 nVariance)
{
    vScheme.resize(3,1);
    if( rBitmapColor.GetRed() < rBitmapColor.GetGreen() + nVariance )
        ++vScheme[0];
    if( rBitmapColor.GetRed() < rBitmapColor.GetBlue() + nVariance )
        ++vScheme[0];
    if( rBitmapColor.GetGreen() < rBitmapColor.GetRed() + nVariance )
        ++vScheme[1];
    if( rBitmapColor.GetGreen() < rBitmapColor.GetBlue() + nVariance )
        ++vScheme[1];
    if( rBitmapColor.GetBlue() < rBitmapColor.GetRed() + nVariance )
        ++vScheme[2];
    if( rBitmapColor.GetBlue() < rBitmapColor.GetGreen() + nVariance )
        ++vScheme[2];
}

bool HasSimilarScheme(const BitmapColor& rBitmapColor1, const BitmapColor& rBitmapColor2, sal_uInt16 nVariance)
{
    std::vector<int> vScheme1, vScheme2;
    CalculateScheme(rBitmapColor1, vScheme1, nVariance);
    CalculateScheme(rBitmapColor2, vScheme2, nVariance);
    for( int i = 0; i < 3; ++i )
    {
        if( vScheme1[i] != vScheme2[i] )
            return false;
    }
    return true;
}

// Find the best match in the color palette using scheme of the input color
sal_uInt16 GetBestIndex(const BitmapPalette& rPalette, const BitmapColor& rBitmapColor)
{
    sal_uInt16 nReturn = 0;
    sal_uInt16 nLastErr = SAL_MAX_UINT16;
    bool bFound = false;

    // Prefer those colors which have similar scheme as the input
    // Allow bigger and bigger variance of the schemes until we find
    // a color in the palette with similar scheme.
    for( sal_uInt16 nVariance = 0; nVariance <= 255; ++nVariance )
    {
        for( sal_uInt16 i = 0; i < rPalette.GetEntryCount(); ++i )
        {
            if( HasSimilarScheme(rBitmapColor, rPalette[i], nVariance) )
            {
                const sal_uInt16 nActErr = rBitmapColor.GetColorError( rPalette[i] );
                if( nActErr < nLastErr )
                {
                    nLastErr = nActErr;
                    nReturn = i;
                    bFound = true;
                }
            }
        }
        if( bFound )
            return nReturn;
    }
    return nReturn;
}
}

sal_uInt8 TransColToIco( const Color& rCol )
{
    sal_uInt8 nCol = 0;      // ->Auto
    switch( sal_uInt32(rCol) )
    {
    case sal_uInt32(COL_BLACK):         nCol = 1;   break;
    case sal_uInt32(COL_BLUE):          nCol = 9;   break;
    case sal_uInt32(COL_GREEN):         nCol = 11;  break;
    case sal_uInt32(COL_CYAN):          nCol = 10;  break;
    case sal_uInt32(COL_RED):           nCol = 13;  break;
    case sal_uInt32(COL_MAGENTA):       nCol = 12;  break;
    case sal_uInt32(COL_BROWN):         nCol = 14;  break;
    case sal_uInt32(COL_GRAY):          nCol = 15;  break;
    case sal_uInt32(COL_LIGHTGRAY):     nCol = 16;  break;
    case sal_uInt32(COL_LIGHTBLUE):     nCol = 2;   break;
    case sal_uInt32(COL_LIGHTGREEN):    nCol = 4;   break;
    case sal_uInt32(COL_LIGHTCYAN):     nCol = 3;   break;
    case sal_uInt32(COL_LIGHTRED):      nCol = 6;   break;
    case sal_uInt32(COL_LIGHTMAGENTA):  nCol = 5;   break;
    case sal_uInt32(COL_YELLOW):        nCol = 7;   break;
    case sal_uInt32(COL_WHITE):         nCol = 8;   break;
    case sal_uInt32(COL_AUTO):          nCol = 0;   break;

    default:
        static const BitmapPalette aBmpPal {
            COL_BLACK,      COL_LIGHTBLUE,  COL_LIGHTCYAN,  COL_LIGHTGREEN,
            COL_LIGHTMAGENTA,COL_LIGHTRED,  COL_YELLOW,     COL_WHITE,
            COL_BLUE,       COL_CYAN,       COL_GREEN,      COL_MAGENTA,
            COL_RED,        COL_BROWN,      COL_GRAY,       COL_LIGHTGRAY
        };

        nCol = static_cast< sal_uInt8 >(GetBestIndex(aBmpPal, rCol) + 1);
        break;
    }
    return nCol;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
