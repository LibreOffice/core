/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_FILTER_MSFILTER_UTIL_HXX
#define INCLUDED_FILTER_MSFILTER_UTIL_HXX

#include <filter/msfilter/msfilterdllapi.h>
#include <svx/msdffdef.hxx>
#include <rtl/string.hxx>
#include <rtl/textenc.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <tools/datetime.hxx>

class Color;

namespace com { namespace sun { namespace star {
    namespace awt { struct Size; }
    namespace lang { struct Locale; }
} } }

namespace msfilter {
namespace util {

/// Returns the best-fit default 8bit encoding for a given locale
/// i.e. useful when dealing with legacy formats which use legacy text encodings without recording
/// what the encoding is, but you know or can guess the language
MSFILTER_DLLPUBLIC rtl_TextEncoding getBestTextEncodingFromLocale(const css::lang::Locale &rLocale);

/// Convert a color in BGR format to RGB.
MSFILTER_DLLPUBLIC sal_uInt32 BGRToRGB(sal_uInt32 nColour);

/** Convert from DTTM to Writer's DateTime
  */
MSFILTER_DLLPUBLIC DateTime DTTM2DateTime( long lDTTM );

/** Convert DateTime to xsd::dateTime string.

I guess there must be an implementation of this somewhere in LO, but I failed
to find it, unfortunately :-(
*/

/// Given a cBullet in encoding r_ioChrSet and fontname r_ioFontName return a
/// suitable new Bullet and change r_ioChrSet and r_ioFontName to form the
/// best-fit replacement in terms of default available MSOffice symbol
/// fonts.
///
/// Used to map from [Open|Star]Symbol to some Windows font or other.
MSFILTER_DLLPUBLIC sal_Unicode bestFitOpenSymbolToMSFont(sal_Unicode cBullet,
    rtl_TextEncoding& r_ioChrSet, OUString& r_ioFontName);


/**
 * Converts tools Color to HTML color (without leading hashmark).
 *
 * @param rColor color to convert
 */
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
    static sal_Int32 getMSPaperSizeIndex( const css::awt::Size& rSize );
    static const ApiPaperSize& getApiSizeForMSPaperSizeIndex( sal_Int32 nMSOPaperIndex );
};

/**
 * Finds the quoted text in a field instruction text.
 *
 * Example: SEQ "Figure" \someoption -> "Figure"
 */
MSFILTER_DLLPUBLIC OUString findQuotedText( const OUString& rCommand, const sal_Char* cStartQuote, const sal_Unicode uEndQuote );

class MSFILTER_DLLPUBLIC WW8ReadFieldParams
{
private:
    const OUString aData;
    sal_Int32 nFnd;
    sal_Int32 nNext;
    sal_Int32 nSavPtr;
public:
    WW8ReadFieldParams( const OUString& rData );

    bool GoToTokenParam();
    sal_Int32 SkipToNextToken();
    sal_Int32 GetTokenSttPtr() const   { return nFnd;  }

    sal_Int32 FindNextStringPiece( sal_Int32 _nStart = -1 );
    bool GetTokenSttFromTo(sal_Int32* _pFrom, sal_Int32* _pTo, sal_Int32 _nMax);

    OUString GetResult() const;
};

struct MSFILTER_DLLPUBLIC EquationResult
{
    OUString sResult;
    OUString sType;
};

MSFILTER_DLLPUBLIC EquationResult ParseCombinedChars(const OUString& rStr);

/// Similar to EnhancedCustomShapeTypeNames::Get(), but it also supports OOXML types and returns a drawingML string.
MSFILTER_DLLPUBLIC const char* GetOOXMLPresetGeometry( const char* sShapeType );

/// Similar to EnhancedCustomShapeTypeNames::Get(), but returns an MSO_SPT (binary / VML type).
MSFILTER_DLLPUBLIC MSO_SPT GETVMLShapeType(const OString& aType);

/**
 * The following function checks if a MSO shapetype is allowed to have textboxcontent.
 *
 * @param nShapeType shape to check
 */
MSFILTER_DLLPUBLIC bool HasTextBoxContent(sal_uInt32 nShapeType);

/**
 * Convert the input color value to an ico value (0..16)
 *
 * @param[in]   rCol       input color for conversion
 *
 * @return                 ico value [0..16]
**/
MSFILTER_DLLPUBLIC sal_uInt8 TransColToIco( const Color& rCol );

}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
