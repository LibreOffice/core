/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/awt/Size.hpp>
#include <rtl/ustring.hxx>
#include <rtl/strbuf.hxx>
#include <unotools/fontcvt.hxx>
#include <unotools/fontdefs.hxx>
#include <vcl/svapp.hxx>
#include <vcl/salbtype.hxx>
#include <filter/msfilter/escherex.hxx>
#include <filter/msfilter/util.hxx>
#include <memory>
#include <unordered_map>

namespace msfilter {
namespace util {

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

sal_uInt32 BGRToRGB(sal_uInt32 nColor)
{
    sal_uInt8
        r(static_cast<sal_uInt8>(nColor&0xFF)),
        g(static_cast<sal_uInt8>((nColor>>8)&0xFF)),
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
          Well we don't have an available substition, and we're
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
    OString color( "auto" );

    if ( rColor != COL_AUTO )
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

sal_Int32 PaperSizeConv::getMSPaperSizeIndex( const css::awt::Size& rSize )
{
    // Need to find the best match for current size
    sal_Int32 nDeltaWidth = 0;
    sal_Int32 nDeltaHeight = 0;

    sal_Int32 nPaperSizeIndex = 0; // Undefined
    const ApiPaperSize* pItem = spPaperSizeTable;
    const ApiPaperSize* pEnd =  spPaperSizeTable + SAL_N_ELEMENTS( spPaperSizeTable );
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
    if ( nMSOPaperIndex  < 0 || nMSOPaperIndex > sal_Int32(SAL_N_ELEMENTS( spPaperSizeTable )) - 1 )
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
        (void)rReadParam.SkipToNextToken();
        SAL_FALLTHROUGH;
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

struct CustomShapeTypeTranslationTable
{
    const char* sOOo;
    const char* sMSO;
};

static const CustomShapeTypeTranslationTable pCustomShapeTypeTranslationTable[] =
{
    // { "non-primitive", mso_sptMin },
    { "frame", "frame" },
    { "rectangle", "rect" },
    { "round-rectangle", "roundRect" },
    { "ellipse", "ellipse" },
    { "diamond", "diamond" },
    { "isosceles-triangle", "triangle" },
    { "right-triangle", "rtTriangle" },
    { "parallelogram", "parallelogram" },
    { "trapezoid", "trapezoid" },
    { "hexagon", "hexagon" },
    { "octagon", "octagon" },
    { "cross", "plus" },
    { "star5", "star5" },
    { "right-arrow", "rightArrow" },
    // { "mso-spt14", mso_sptThickArrow },
    { "pentagon-right", "homePlate" },
    { "cube", "cube" },
    // { "mso-spt17", mso_sptBalloon },
    // { "mso-spt18", mso_sptSeal },
    { "mso-spt19", "arc" },
    { "mso-spt20", "line" },
    { "mso-spt21", "plaque" },
    { "can", "can" },
    { "ring", "donut" },
    { "mso-spt24", "textPlain" },
    { "mso-spt25", "textStop" },
    { "mso-spt26", "textTriangle" },
    { "mso-spt27", "textCanDown" },
    { "mso-spt28", "textWave1" },
    { "mso-spt29", "textArchUpPour" },
    { "mso-spt30", "textCanDown" },
    { "mso-spt31", "textArchUp" },
    { "mso-spt32", "straightConnector1" },
    { "mso-spt33", "bentConnector2" },
    { "mso-spt34", "bentConnector3" },
    { "mso-spt35", "bentConnector4" },
    { "mso-spt36", "bentConnector5" },
    { "mso-spt37", "curvedConnector2" },
    { "mso-spt38", "curvedConnector3" },
    { "mso-spt39", "curvedConnector4" },
    { "mso-spt40", "curvedConnector5" },
    { "mso-spt41", "callout1" },
    { "mso-spt42", "callout2" },
    { "mso-spt43", "callout3" },
    { "mso-spt44", "accentCallout1" },
    { "mso-spt45", "accentCallout2" },
    { "mso-spt46", "accentCallout3" },
    { "line-callout-1", "borderCallout1" },
    { "line-callout-2", "borderCallout2" },
    { "line-callout-3", "borderCallout3" },
    { "mso-spt49", "borderCallout3" },
    { "mso-spt50", "accentBorderCallout1" },
    { "mso-spt51", "accentBorderCallout2" },
    { "mso-spt52", "accentBorderCallout3" },
    { "mso-spt53", "ribbon" },
    { "mso-spt54", "ribbon2" },
    { "chevron", "chevron" },
    { "pentagon", "pentagon" },
    { "forbidden", "noSmoking" },
    { "star8", "star8" },
    { "mso-spt59", "star16" },
    { "mso-spt60", "star32" },
    { "rectangular-callout", "wedgeRectCallout" },
    { "round-rectangular-callout", "wedgeRoundRectCallout" },
    { "round-callout", "wedgeEllipseCallout" },
    { "mso-spt64", "wave" },
    { "paper", "foldedCorner" },
    { "left-arrow", "leftArrow" },
    { "down-arrow", "downArrow" },
    { "up-arrow", "upArrow" },
    { "left-right-arrow", "leftRightArrow" },
    { "up-down-arrow", "upDownArrow" },
    { "mso-spt71", "irregularSeal1" },
    { "bang", "irregularSeal2" },
    { "lightning", "lightningBolt" },
    { "heart", "heart" },
    { "quad-arrow", "quadArrow" },
    { "left-arrow-callout", "leftArrowCallout" },
    { "right-arrow-callout", "rightArrowCallout" },
    { "up-arrow-callout", "upArrowCallout" },
    { "down-arrow-callout", "downArrowCallout" },
    { "left-right-arrow-callout", "leftRightArrowCallout" },
    { "up-down-arrow-callout", "upDownArrowCallout" },
    { "quad-arrow-callout", "quadArrowCallout" },
    { "quad-bevel", "bevel" },
    { "left-bracket", "leftBracket" },
    { "right-bracket", "rightBracket" },
    { "left-brace", "leftBrace" },
    { "right-brace", "rightBrace" },
    { "mso-spt89", "leftUpArrow" },
    { "mso-spt90", "bentUpArrow" },
    { "mso-spt91", "bentArrow" },
    { "star24", "star24" },
    { "striped-right-arrow", "stripedRightArrow" },
    { "notched-right-arrow", "notchedRightArrow" },
    { "block-arc", "blockArc" },
    { "smiley", "smileyFace" },
    { "vertical-scroll", "verticalScroll" },
    { "horizontal-scroll", "horizontalScroll" },
    { "circular-arrow", "circularArrow" },
    { "mso-spt100", "pie" }, // looks like MSO_SPT is wrong here
    { "mso-spt101", "uturnArrow" },
    { "mso-spt102", "curvedRightArrow" },
    { "mso-spt103", "curvedLeftArrow" },
    { "mso-spt104", "curvedUpArrow" },
    { "mso-spt105", "curvedDownArrow" },
    { "cloud-callout", "cloudCallout" },
    { "mso-spt107", "ellipseRibbon" },
    { "mso-spt108", "ellipseRibbon2" },
    { "flowchart-process", "flowChartProcess" },
    { "flowchart-decision", "flowChartDecision" },
    { "flowchart-data", "flowChartInputOutput" },
    { "flowchart-predefined-process", "flowChartPredefinedProcess" },
    { "flowchart-internal-storage", "flowChartInternalStorage" },
    { "flowchart-document", "flowChartDocument" },
    { "flowchart-multidocument", "flowChartMultidocument" },
    { "flowchart-terminator", "flowChartTerminator" },
    { "flowchart-preparation", "flowChartPreparation" },
    { "flowchart-manual-input", "flowChartManualInput" },
    { "flowchart-manual-operation", "flowChartManualOperation" },
    { "flowchart-connector", "flowChartConnector" },
    { "flowchart-card", "flowChartPunchedCard" },
    { "flowchart-punched-tape", "flowChartPunchedTape" },
    { "flowchart-summing-junction", "flowChartSummingJunction" },
    { "flowchart-or", "flowChartOr" },
    { "flowchart-collate", "flowChartCollate" },
    { "flowchart-sort", "flowChartSort" },
    { "flowchart-extract", "flowChartExtract" },
    { "flowchart-merge", "flowChartMerge" },
    { "mso-spt129", "flowChartOfflineStorage" },
    { "flowchart-stored-data", "flowChartOnlineStorage" },
    { "flowchart-sequential-access", "flowChartMagneticTape" },
    { "flowchart-magnetic-disk", "flowChartMagneticDisk" },
    { "flowchart-direct-access-storage", "flowChartMagneticDrum" },
    { "flowchart-display", "flowChartDisplay" },
    { "flowchart-delay", "flowChartDelay" },
    // { "fontwork-plain-text", "textPlainText" },
    // { "fontwork-stop", "textStop" },
    // { "fontwork-triangle-up", "textTriangle" },
    // { "fontwork-triangle-down", "textTriangleInverted" },
    // { "fontwork-chevron-up", "textChevron" },
    // { "fontwork-chevron-down", "textChevronInverted" },
    // { "mso-spt142", "textRingInside" },
    // { "mso-spt143", "textRingOutside" },
    // { "fontwork-arch-up-curve", "textArchUpCurve" },
    // { "fontwork-arch-down-curve", "textArchDownCurve" },
    // { "fontwork-circle-curve", "textCircleCurve" },
    // { "fontwork-open-circle-curve", "textButtonCurve" },
    // { "fontwork-arch-up-pour", "textArchUpPour" },
    // { "fontwork-arch-down-pour", "textArchDownPour" },
    // { "fontwork-circle-pour", "textCirclePour" },
    // { "fontwork-open-circle-pour", "textButtonPour" },
    // { "fontwork-curve-up", "textCurveUp" },
    // { "fontwork-curve-down", "textCurveDown" },
    // { "fontwork-fade-up-and-right", "textCascadeUp" },
    // { "fontwork-fade-up-and-left", "textCascadeDown" },
    // { "fontwork-wave", "textWave1" },
    // { "mso-spt157", "textWave2" },
    // { "mso-spt158", "textWave3" },
    // { "mso-spt159", "textWave4" },
    // { "fontwork-inflate", "textInflate" },
    // { "mso-spt161", "textDeflate" },
    // { "mso-spt162", "textInflateBottom" },
    // { "mso-spt163", "textDeflateBottom" },
    // { "mso-spt164", "textInflateTop" },
    // { "mso-spt165", "textDeflateTop" },
    // { "mso-spt166", "textDeflateInflate" },
    // { "mso-spt167", "textDeflateInflateDeflate" },
    // { "fontwork-fade-right", "textFadeRight" },
    // { "fontwork-fade-left", "textFadeLeft" },
    // { "fontwork-fade-up", "textFadeUp" },
    // { "fontwork-fade-down", "textFadeDown" },
    // { "fontwork-slant-up", "textSlantUp" },
    // { "fontwork-slant-down", "textSlantDown" },
    // { "mso-spt174", "textCanUp" },
    // { "mso-spt175", "textCanDown" },
    { "flowchart-alternate-process", "flowChartAlternateProcess" },
    { "flowchart-off-page-connector", "flowChartOffpageConnector" },
    { "mso-spt178", "callout1" },
    { "mso-spt179", "accentCallout1" },
    { "mso-spt180", "borderCallout1" },
    { "mso-spt182", "leftRightUpArrow" },
    { "sun", "sun" },
    { "moon", "moon" },
    { "bracket-pair", "bracketPair" },
    { "brace-pair", "bracePair" },
    { "star4", "star4" },
    { "mso-spt188", "doubleWave" },
    { "mso-spt189", "actionButtonBlank" },
    { "mso-spt190", "actionButtonHome" },
    { "mso-spt191", "actionButtonHelp" },
    { "mso-spt192", "actionButtonInformation" },
    { "mso-spt193", "actionButtonForwardNext" },
    { "mso-spt194", "actionButtonBackPrevious" },
    { "mso-spt195", "actionButtonEnd" },
    { "mso-spt196", "actionButtonBeginning" },
    { "mso-spt197", "actionButtonReturn" },
    { "mso-spt198", "actionButtonDocument" },
    { "mso-spt199", "actionButtonSound" },
    { "mso-spt200", "actionButtonMovie" },
    // { "mso-spt201", "hostControl" },
    { "mso-spt202", "rect" },
    { "ooxml-actionButtonSound", "actionButtonSound" },
    { "ooxml-borderCallout1", "borderCallout1" },
    { "ooxml-plaqueTabs", "plaqueTabs" },
    { "ooxml-curvedLeftArrow", "curvedLeftArrow" },
    { "ooxml-octagon", "octagon" },
    { "ooxml-leftRightRibbon", "leftRightRibbon" },
    { "ooxml-actionButtonInformation", "actionButtonInformation" },
    { "ooxml-bentConnector5", "bentConnector5" },
    { "ooxml-circularArrow", "circularArrow" },
    { "ooxml-downArrowCallout", "downArrowCallout" },
    { "ooxml-mathMinus", "mathMinus" },
    { "ooxml-gear9", "gear9" },
    { "ooxml-round1Rect", "round1Rect" },
    { "ooxml-sun", "sun" },
    { "ooxml-plaque", "plaque" },
    { "ooxml-chevron", "chevron" },
    { "ooxml-flowChartPreparation", "flowChartPreparation" },
    { "ooxml-diagStripe", "diagStripe" },
    { "ooxml-pentagon", "pentagon" },
    { "ooxml-funnel", "funnel" },
    { "ooxml-chartStar", "chartStar" },
    { "ooxml-accentBorderCallout1", "accentBorderCallout1" },
    { "ooxml-notchedRightArrow", "notchedRightArrow" },
    { "ooxml-rightBracket", "rightBracket" },
    { "ooxml-flowChartOffpageConnector", "flowChartOffpageConnector" },
    { "ooxml-leftRightArrow", "leftRightArrow" },
    { "ooxml-decagon", "decagon" },
    { "ooxml-actionButtonHelp", "actionButtonHelp" },
    { "ooxml-star24", "star24" },
    { "ooxml-mathDivide", "mathDivide" },
    { "ooxml-curvedConnector4", "curvedConnector4" },
    { "ooxml-flowChartOr", "flowChartOr" },
    { "ooxml-borderCallout3", "borderCallout3" },
    { "ooxml-upDownArrowCallout", "upDownArrowCallout" },
    { "ooxml-flowChartDecision", "flowChartDecision" },
    { "ooxml-leftRightArrowCallout", "leftRightArrowCallout" },
    { "ooxml-flowChartManualOperation", "flowChartManualOperation" },
    { "ooxml-snipRoundRect", "snipRoundRect" },
    { "ooxml-mathPlus", "mathPlus" },
    { "ooxml-actionButtonForwardNext", "actionButtonForwardNext" },
    { "ooxml-can", "can" },
    { "ooxml-foldedCorner", "foldedCorner" },
    { "ooxml-star32", "star32" },
    { "ooxml-flowChartInternalStorage", "flowChartInternalStorage" },
    { "ooxml-upDownArrow", "upDownArrow" },
    { "ooxml-irregularSeal2", "irregularSeal2" },
    { "ooxml-mathEqual", "mathEqual" },
    { "ooxml-star12", "star12" },
    { "ooxml-uturnArrow", "uturnArrow" },
    { "ooxml-squareTabs", "squareTabs" },
    { "ooxml-leftRightUpArrow", "leftRightUpArrow" },
    { "ooxml-homePlate", "homePlate" },
    { "ooxml-dodecagon", "dodecagon" },
    { "ooxml-leftArrowCallout", "leftArrowCallout" },
    { "ooxml-chord", "chord" },
    { "ooxml-quadArrowCallout", "quadArrowCallout" },
    { "ooxml-actionButtonBeginning", "actionButtonBeginning" },
    { "ooxml-ellipse", "ellipse" },
    { "ooxml-actionButtonEnd", "actionButtonEnd" },
    { "ooxml-arc", "arc" },
    { "ooxml-star16", "star16" },
    { "ooxml-parallelogram", "parallelogram" },
    { "ooxml-bevel", "bevel" },
    { "ooxml-roundRect", "roundRect" },
    { "ooxml-accentCallout1", "accentCallout1" },
    { "ooxml-flowChartSort", "flowChartSort" },
    { "ooxml-star8", "star8" },
    { "ooxml-flowChartAlternateProcess", "flowChartAlternateProcess" },
    { "ooxml-moon", "moon" },
    { "ooxml-star6", "star6" },
    { "ooxml-round2SameRect", "round2SameRect" },
    { "ooxml-nonIsoscelesTrapezoid", "nonIsoscelesTrapezoid" },
    { "ooxml-diamond", "diamond" },
    { "ooxml-ellipseRibbon", "ellipseRibbon" },
    { "ooxml-callout2", "callout2" },
    { "ooxml-pie", "pie" },
    { "ooxml-star4", "star4" },
    { "ooxml-flowChartPredefinedProcess", "flowChartPredefinedProcess" },
    { "ooxml-flowChartPunchedTape", "flowChartPunchedTape" },
    { "ooxml-curvedConnector2", "curvedConnector2" },
    { "ooxml-bentConnector3", "bentConnector3" },
    { "ooxml-cornerTabs", "cornerTabs" },
    { "ooxml-hexagon", "hexagon" },
    { "ooxml-flowChartConnector", "flowChartConnector" },
    { "ooxml-flowChartMagneticDisk", "flowChartMagneticDisk" },
    { "ooxml-heart", "heart" },
    { "ooxml-ribbon2", "ribbon2" },
    { "ooxml-bracePair", "bracePair" },
    { "ooxml-flowChartExtract", "flowChartExtract" },
    { "ooxml-actionButtonHome", "actionButtonHome" },
    { "ooxml-accentBorderCallout3", "accentBorderCallout3" },
    { "ooxml-flowChartOfflineStorage", "flowChartOfflineStorage" },
    { "ooxml-irregularSeal1", "irregularSeal1" },
    { "ooxml-quadArrow", "quadArrow" },
    { "ooxml-leftBrace", "leftBrace" },
    { "ooxml-leftBracket", "leftBracket" },
    { "ooxml-blockArc", "blockArc" },
    { "ooxml-curvedConnector3", "curvedConnector3" },
    { "ooxml-wedgeRoundRectCallout", "wedgeRoundRectCallout" },
    { "ooxml-actionButtonMovie", "actionButtonMovie" },
    { "ooxml-flowChartOnlineStorage", "flowChartOnlineStorage" },
    { "ooxml-gear6", "gear6" },
    { "ooxml-halfFrame", "halfFrame" },
    { "ooxml-snip2SameRect", "snip2SameRect" },
    { "ooxml-triangle", "triangle" },
    { "ooxml-teardrop", "teardrop" },
    { "ooxml-flowChartDocument", "flowChartDocument" },
    { "ooxml-rightArrowCallout", "rightArrowCallout" },
    { "ooxml-rightBrace", "rightBrace" },
    { "ooxml-chartPlus", "chartPlus" },
    { "ooxml-flowChartManualInput", "flowChartManualInput" },
    { "ooxml-flowChartMerge", "flowChartMerge" },
    { "ooxml-line", "line" },
    { "ooxml-downArrow", "downArrow" },
    { "ooxml-upArrow", "upArrow" },
    { "ooxml-curvedDownArrow", "curvedDownArrow" },
    { "ooxml-actionButtonReturn", "actionButtonReturn" },
    { "ooxml-flowChartInputOutput", "flowChartInputOutput" },
    { "ooxml-bracketPair", "bracketPair" },
    { "ooxml-smileyFace", "smileyFace" },
    { "ooxml-actionButtonBlank", "actionButtonBlank" },
    { "ooxml-wave", "wave" },
    { "ooxml-swooshArrow", "swooshArrow" },
    { "ooxml-flowChartSummingJunction", "flowChartSummingJunction" },
    { "ooxml-lightningBolt", "lightningBolt" },
    { "ooxml-flowChartDisplay", "flowChartDisplay" },
    { "ooxml-actionButtonBackPrevious", "actionButtonBackPrevious" },
    { "ooxml-frame", "frame" },
    { "ooxml-rtTriangle", "rtTriangle" },
    { "ooxml-flowChartMagneticTape", "flowChartMagneticTape" },
    { "ooxml-curvedRightArrow", "curvedRightArrow" },
    { "ooxml-leftUpArrow", "leftUpArrow" },
    { "ooxml-wedgeEllipseCallout", "wedgeEllipseCallout" },
    { "ooxml-doubleWave", "doubleWave" },
    { "ooxml-bentArrow", "bentArrow" },
    { "ooxml-star10", "star10" },
    { "ooxml-leftArrow", "leftArrow" },
    { "ooxml-curvedUpArrow", "curvedUpArrow" },
    { "ooxml-snip1Rect", "snip1Rect" },
    { "ooxml-ellipseRibbon2", "ellipseRibbon2" },
    { "ooxml-plus", "plus" },
    { "ooxml-accentCallout3", "accentCallout3" },
    { "ooxml-leftCircularArrow", "leftCircularArrow" },
    { "ooxml-rightArrow", "rightArrow" },
    { "ooxml-flowChartPunchedCard", "flowChartPunchedCard" },
    { "ooxml-snip2DiagRect", "snip2DiagRect" },
    { "ooxml-verticalScroll", "verticalScroll" },
    { "ooxml-star7", "star7" },
    { "ooxml-chartX", "chartX" },
    { "ooxml-cloud", "cloud" },
    { "ooxml-cube", "cube" },
    { "ooxml-round2DiagRect", "round2DiagRect" },
    { "ooxml-flowChartMultidocument", "flowChartMultidocument" },
    { "ooxml-actionButtonDocument", "actionButtonDocument" },
    { "ooxml-flowChartTerminator", "flowChartTerminator" },
    { "ooxml-flowChartDelay", "flowChartDelay" },
    { "ooxml-curvedConnector5", "curvedConnector5" },
    { "ooxml-horizontalScroll", "horizontalScroll" },
    { "ooxml-bentConnector4", "bentConnector4" },
    { "ooxml-leftRightCircularArrow", "leftRightCircularArrow" },
    { "ooxml-wedgeRectCallout", "wedgeRectCallout" },
    { "ooxml-accentCallout2", "accentCallout2" },
    { "ooxml-flowChartMagneticDrum", "flowChartMagneticDrum" },
    { "ooxml-corner", "corner" },
    { "ooxml-borderCallout2", "borderCallout2" },
    { "ooxml-donut", "donut" },
    { "ooxml-flowChartCollate", "flowChartCollate" },
    { "ooxml-mathNotEqual", "mathNotEqual" },
    { "ooxml-bentConnector2", "bentConnector2" },
    { "ooxml-mathMultiply", "mathMultiply" },
    { "ooxml-heptagon", "heptagon" },
    { "ooxml-rect", "rect" },
    { "ooxml-accentBorderCallout2", "accentBorderCallout2" },
    { "ooxml-pieWedge", "pieWedge" },
    { "ooxml-upArrowCallout", "upArrowCallout" },
    { "ooxml-flowChartProcess", "flowChartProcess" },
    { "ooxml-star5", "star5" },
    { "ooxml-lineInv", "lineInv" },
    { "ooxml-straightConnector1", "straightConnector1" },
    { "ooxml-stripedRightArrow", "stripedRightArrow" },
    { "ooxml-callout3", "callout3" },
    { "ooxml-bentUpArrow", "bentUpArrow" },
    { "ooxml-noSmoking", "noSmoking" },
    { "ooxml-trapezoid", "trapezoid" },
    { "ooxml-cloudCallout", "cloudCallout" },
    { "ooxml-callout1", "callout1" },
    { "ooxml-ribbon", "ribbon" },
    { "ooxml-rect", "rect" },
};

static struct {
    const char* sDML;
    MSO_SPT nVML;
} const pDMLToVMLTable[] = {
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

typedef std::unordered_map< const char*, const char*, rtl::CStringHash, rtl::CStringEqual> CustomShapeTypeTranslationHashMap;
static CustomShapeTypeTranslationHashMap* pCustomShapeTypeTranslationHashMap = nullptr;

const char* GetOOXMLPresetGeometry( const char* sShapeType )
{
    if( pCustomShapeTypeTranslationHashMap == nullptr )
    {
        pCustomShapeTypeTranslationHashMap = new CustomShapeTypeTranslationHashMap;
        for(const msfilter::util::CustomShapeTypeTranslationTable& i : pCustomShapeTypeTranslationTable)
        {
            (*pCustomShapeTypeTranslationHashMap)[ i.sOOo ] = i.sMSO;
        }
    }
    CustomShapeTypeTranslationHashMap::iterator i(
        pCustomShapeTypeTranslationHashMap->find(sShapeType));
    return i == pCustomShapeTypeTranslationHashMap->end() ? "rect" : i->second;
}

typedef std::unordered_map< const char*, MSO_SPT, rtl::CStringHash, rtl::CStringEqual> DMLToVMLTranslationHashMap;
static DMLToVMLTranslationHashMap* pDMLToVMLMap;

MSO_SPT GETVMLShapeType(const OString& aType)
{
    const char* pDML = GetOOXMLPresetGeometry(aType.getStr());

    if (!pDMLToVMLMap)
    {
        pDMLToVMLMap = new DMLToVMLTranslationHashMap;
        for (auto& i : pDMLToVMLTable)
            (*pDMLToVMLMap)[i.sDML] = i.nVML;
    }

    DMLToVMLTranslationHashMap::iterator i(pDMLToVMLMap->find(pDML));
    return i == pDMLToVMLMap->end() ? mso_sptNil : i->second;
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
        static const Color aColArr[ 16 ] = {
            COL_BLACK,      COL_LIGHTBLUE,  COL_LIGHTCYAN,  COL_LIGHTGREEN,
            COL_LIGHTMAGENTA,COL_LIGHTRED,  COL_YELLOW,     COL_WHITE,
            COL_BLUE,       COL_CYAN,       COL_GREEN,      COL_MAGENTA,
            COL_RED,        COL_BROWN,      COL_GRAY,       COL_LIGHTGRAY
        };
        BitmapPalette aBmpPal(16);
        for( sal_uInt16 i = 0; i < 16; ++i )
            aBmpPal[i] = aColArr[ i ];

        nCol = static_cast< sal_uInt8 >(GetBestIndex(aBmpPal, rCol) + 1);
        break;
    }
    return nCol;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
