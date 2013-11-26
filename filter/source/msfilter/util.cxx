/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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

sal_Unicode bestFitOpenSymbolToMSFont(sal_Unicode cChar,
    rtl_TextEncoding& rChrSet, OUString& rFontName, bool bDisableUnicodeSupport)
{
    StarSymbolToMSMultiFont *pConvert = CreateStarSymbolToMSMultiFont();
    OUString sFont = pConvert->ConvertChar(cChar);
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
        sal_Int32 nIndex = 0;
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

/*
  http://social.msdn.microsoft.com/Forums/hu-HU/os_openXML-ecma/thread/1bf1f185-ee49-4314-94e7-f4e1563b5c00

  The following information is being submitted to the standards working group as
  a proposed resolution to a defect report and is not yet part of ISO 29500-1.
  ...
  For each Unicode character in DrawingML text, the font face can be any of four
  font "slots": latin ($21.1.2.3.7), cs ($21.1.2.3.1), ea ($21.1.2.3.3), or sym
  ($21.1.2.3.10), as specified in the following table. For all ranges not
  explicitly called out below, the ea font shall be used.

  U+0000-U+007F Use latin font
  U+0080-U+00A6 Use latin font
  U+00A9-U+00AF Use latin font
  U+00B2-U+00B3 Use latin font
  U+00B5-U+00D6 Use latin font
  U+00D8-U+00F6 Use latin font
  U+00F8-U+058F Use latin font
  U+0590-U+074F Use cs font
  U+0780-U+07BF Use cs font
  U+0900-U+109F Use cs font
  U+10A0-U+10FF Use latin font
  U+1200-U+137F Use latin font
  U+13A0-U+177F Use latin font
  U+1D00-U+1D7F Use latin font
  U+1E00-U+1FFF Use latin font
  U+1780-U+18AF Use cs font
  U+2000-U+200B Use latin font
  U+200C-U+200F Use cs font
  U+2010-U+2029 Use latin font Except, for the quote characters in the range
    2018 - 201E, use ea font if the text has one of the following language
    identifiers: ii-CN, ja-JP, ko-KR, zh-CN, zh-HK, zh-MO, zh-SG, zh-TW.
  U+202A-U+202F Use cs font
  U+2030-U+2046 Use latin font
  U+204A-U+245F Use latin font
  U+2670-U+2671 Use cs font
  U+27C0-U+2BFF Use latin font
  U+3099-U+309A Use ea font
  U+D835 Use latin font
  U+F000-U+F0FF Symbol, use sym font
  U+FB00-U+FB17 Use latin font
  U+FB1D-U+FB4F Use cs font
  U+FE50-U+FE6F Use latin font
  Otherwise Use ea font
*/
TextCategory categorizeCodePoint(sal_uInt32 codePoint, const OUString &rBcp47LanguageTag)
{
    TextCategory eRet = ea;
    if (codePoint <= 0x007F)
        eRet = latin;
    else if (0x0080 <= codePoint && codePoint <= 0x00A6)
        eRet = latin;
    else if (0x00A9 <= codePoint && codePoint <= 0x00AF)
        eRet = latin;
    else if (0x00B2 <= codePoint && codePoint <= 0x00B3)
        eRet = latin;
    else if (0x00B5 <= codePoint && codePoint <= 0x00D6)
        eRet = latin;
    else if (0x00D8 <= codePoint && codePoint <= 0x00F6)
        eRet = latin;
    else if (0x00F8 <= codePoint && codePoint <= 0x058F)
        eRet = latin;
    else if (0x0590 <= codePoint && codePoint <= 0x074F)
        eRet = cs;
    else if (0x0780 <= codePoint && codePoint <= 0x07BF)
        eRet = cs;
    else if (0x0900 <= codePoint && codePoint <= 0x109F)
        eRet = cs;
    else if (0x10A0 <= codePoint && codePoint <= 0x10FF)
        eRet = latin;
    else if (0x1200 <= codePoint && codePoint <= 0x137F)
        eRet = latin;
    else if (0x13A0 <= codePoint && codePoint <= 0x177F)
        eRet = latin;
    else if (0x1D00 <= codePoint && codePoint <= 0x1D7F)
        eRet = latin;
    else if (0x1E00 <= codePoint && codePoint <= 0x1FFF)
        eRet = latin;
    else if (0x1780 <= codePoint && codePoint <= 0x18AF)
        eRet = cs;
    else if (0x2000 <= codePoint && codePoint <= 0x200B)
        eRet = latin;
    else if (0x200C <= codePoint && codePoint <= 0x200F)
        eRet = cs;
    else if (0x2010 <= codePoint && codePoint <= 0x2029)
    {
        eRet = latin;
        if (0x2018 <= codePoint && codePoint <= 0x201E)
        {
            if (rBcp47LanguageTag == "ii-CN" ||
                rBcp47LanguageTag == "ja-JP" ||
                rBcp47LanguageTag == "ko-KR" ||
                rBcp47LanguageTag == "zh-CN" ||
                rBcp47LanguageTag == "zh-HK" ||
                rBcp47LanguageTag == "zh-MO" ||
                rBcp47LanguageTag == "zh-SG" ||
                rBcp47LanguageTag == "zh-TW")
            {
                eRet = ea;
            }
        }
    }
    else if (0x202A <= codePoint && codePoint <= 0x202F)
        eRet = cs;
    else if (0x2030 <= codePoint && codePoint <= 0x2046)
        eRet = latin;
    else if (0x204A <= codePoint && codePoint <= 0x245F)
        eRet = latin;
    else if (0x2670 <= codePoint && codePoint <= 0x2671)
        eRet = latin;
    else if (0x27C0 <= codePoint && codePoint <= 0x2BFF)
        eRet = latin;
    else if (0x3099 <= codePoint && codePoint <= 0x309A)
        eRet = ea;
    else if (0xD835 == codePoint)
        eRet = latin;
    else if (0xF000 <= codePoint && codePoint <= 0xF0FF)
        eRet = sym;
    else if (0xFB00 <= codePoint && codePoint <= 0xFB17)
        eRet = latin;
    else if (0xFB1D <= codePoint && codePoint <= 0xFB4F)
        eRet = cs;
    else if (0xFE50 <= codePoint && codePoint <= 0xFE6F)
        eRet = latin;
    return eRet;
}

OString ConvertColor( const Color &rColor, bool bAutoColor )
{
    OString color( "auto" );

    if (bAutoColor && rColor.GetColor() == OOXML_COLOR_AUTO)
        return color;

    if ( rColor.GetColor() != COL_AUTO )
    {
        const char pHexDigits[] = "0123456789ABCDEF";
        char pBuffer[] = "000000";

        pBuffer[0] = pHexDigits[ ( rColor.GetRed()   >> 4 ) & 0x0F ];
        pBuffer[1] = pHexDigits[   rColor.GetRed()          & 0x0F ];
        pBuffer[2] = pHexDigits[ ( rColor.GetGreen() >> 4 ) & 0x0F ];
        pBuffer[3] = pHexDigits[   rColor.GetGreen()        & 0x0F ];
        pBuffer[4] = pHexDigits[ ( rColor.GetBlue()  >> 4 ) & 0x0F ];
        pBuffer[5] = pHexDigits[   rColor.GetBlue()         & 0x0F ];

        color = OString( pBuffer );
    }
    return color;
}

#define IN2MM100( v )    static_cast< sal_Int32 >( (v) * 2540.0 + 0.5 )
#define MM2MM100( v )    static_cast< sal_Int32 >( (v) * 100.0 + 0.5 )

static const ApiPaperSize spPaperSizeTable[] =
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
    { MM2MM100( 250 ),       MM2MM100( 353 )     },          // 12 - B4 paper
    { MM2MM100( 176 ),       MM2MM100( 250 )     },          // 13 - B5 paper
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
    { IN2MM100( 9.275 ),     IN2MM100( 12 )      },          // 50 - Letter extra paper
    { IN2MM100( 9.275 ),     IN2MM100( 15 )      },          // 51 - Legal extra paper
    { IN2MM100( 11.69 ),     IN2MM100( 18 )      },          // 52 - Tabloid extra paper
    { MM2MM100( 236 ),       MM2MM100( 322 )     },          // 53 - A4 extra paper
    { IN2MM100( 8.275 ),     IN2MM100( 11 )      },          // 54 - Letter transverse paper
    { MM2MM100( 210 ),       MM2MM100( 297 )     },          // 55 - A4 transverse paper
    { IN2MM100( 9.275 ),     IN2MM100( 12 )      },          // 56 - Letter extra transverse paper
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
    { MM2MM100( 322 ),       MM2MM100( 445 )     }           // 68 - A3 extra transverse paper
};

sal_Int32 PaperSizeConv::getMSPaperSizeIndex( const com::sun::star::awt::Size& rSize )
{
    sal_Int32 nElems = SAL_N_ELEMENTS( spPaperSizeTable );
    // Need to find the best match for current size
    sal_Int32 nDeltaWidth = 0;
    sal_Int32 nDeltaHeight = 0;

    sal_Int32 nPaperSizeIndex = 0; // Undefined
    const ApiPaperSize* pItem = spPaperSizeTable;
    const ApiPaperSize* pEnd =  spPaperSizeTable + nElems;
    for ( ; pItem != pEnd; ++pItem )
    {
        sal_Int32 nCurDeltaHeight = std::abs( pItem->mnHeight - rSize.Height );
        sal_Int32 nCurDeltaWidth = std::abs( pItem->mnWidth - rSize.Width );
        if ( pItem == spPaperSizeTable ) // initialise delta with first item
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
    sal_Int32 nElems = SAL_N_ELEMENTS( spPaperSizeTable );
    if ( nMSOPaperIndex  < 0 || nMSOPaperIndex > nElems - 1 )
        return spPaperSizeTable[ 0 ];
    return spPaperSizeTable[ nMSOPaperIndex ];
}

OUString findQuotedText( const OUString& rCommand,
                const sal_Char* cStartQuote, const sal_Unicode uEndQuote )
{
    OUString sRet;
    OUString sStartQuote( OUString::createFromAscii(cStartQuote) );
    sal_Int32 nStartIndex = rCommand.indexOf( sStartQuote );
    if( nStartIndex >= 0 )
    {
        sal_Int32 nStartLength = sStartQuote.getLength();
        sal_Int32 nEndIndex = rCommand.indexOf( uEndQuote, nStartIndex + nStartLength);
        if( nEndIndex > nStartIndex )
        {
            sRet = rCommand.copy( nStartIndex + nStartLength, nEndIndex - nStartIndex - nStartLength);
        }
    }
    return sRet;

}

WW8ReadFieldParams::WW8ReadFieldParams( const OUString& _rData )
    : aData( _rData )
    , nFnd( 0 )
    , nNext( 0 )
    , nSavPtr( 0 )
{
    /*
        erstmal nach einer oeffnenden Klammer oder einer Leerstelle oder einem
        Anfuehrungszeichen oder einem Backslash suchen, damit der Feldbefehl
        (also INCLUDEPICTURE bzw EINFUeGENGRAFIK bzw ...) ueberlesen wird
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


WW8ReadFieldParams::~WW8ReadFieldParams()
{

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

// ret: -2: NOT a '\' parameter but normal Text
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
        nNext = ++nFnd;             // und dahinter setzen
        return nRet;
    }

    if ( nSavPtr>0 && (aData[nSavPtr-1]=='"' || aData[nSavPtr-1]==0x201d ) )
    {
        --nSavPtr;
    }
    return -2;
}

// FindNextPara sucht naechsten Backslash-Parameter oder naechste Zeichenkette
// bis zum Blank oder naechsten "\" oder zum schliessenden Anfuehrungszeichen
// oder zum String-Ende von pStr.
//
// Ausgabe ppNext (falls ppNext != 0) Suchbeginn fuer naechsten Parameter bzw. 0
//
// Returnwert: 0 falls String-Ende erreicht,
//             ansonsten Anfang des Paramters bzw. der Zeichenkette
//
sal_Int32 WW8ReadFieldParams::FindNextStringPiece(const sal_Int32 nStart)
{
    const sal_Int32 nLen = aData.getLength();
    sal_Int32  n = nStart<0  ? nFnd : nStart;  // Anfang
    sal_Int32 n2;          // Ende

    nNext = -1;        // Default fuer nicht gefunden

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

    // Anfuehrungszeichen vor Para?
    if ( aData[n]=='"' || aData[n]==0x201c || aData[n]==132 || aData[n]==0x14 )
    {
        n++;                        // Anfuehrungszeichen ueberlesen
        n2 = n;                     // ab hier nach Ende suchen
        while(     (nLen > n2)
                && (aData[n2] != '"')
                && (aData[n2] != 0x201d)
                && (aData[n2] != 147)
                && (aData[n2] != 0x15) )
            n2++;                   // Ende d. Paras suchen
    }
    else                        // keine Anfuehrungszeichen
    {
        n2 = n;                     // ab hier nach Ende suchen
        while ( n2<nLen && aData[n2]!=' ' ) // Ende d. Paras suchen
        {
            if ( aData[n2]=='\\' )
            {
                if ( n2+1<nLen && aData[n2+1]=='\\' )
                    n2 += 2;        // Doppel-Backslash -> OK
                else
                {
                    if( n2 > n )
                        n2--;
                    break;          // einfach-Backslash -> Ende
                }
            }
            else
                n2++;               // kein Backslash -> OK
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
        const OUString sStart( sParams.getToken(0, '-', nIndex) );
        if (nIndex>=0)
        {
            nStart = sStart.toInt32();
            nEnd   = sParams.copy(nIndex).toInt32();
        }
    }
    if( pFrom ) *pFrom = nStart;
    if( pTo )   *pTo   = nEnd;

    return nStart && nEnd && (nMax >= nStart) && (nMax >= nEnd);
}

EquationResult Read_SubF_Combined(WW8ReadFieldParams& rReadParam)
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
        rReadParam.SkipToNextToken();
        // intentional fall-through
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

                                //Word disallows brackets in this field, which
                                //aids figuring out the case of an end of )) vs )
                                sal_Int32 nEnd = sPart.indexOf(')');

                                if (nBegin != -1 && nEnd != -1)
                                {
                                    sCombinedCharacters +=
                                        sPart.copy(nBegin+1,nEnd-nBegin-1);
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

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
