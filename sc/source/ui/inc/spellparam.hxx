/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: spellparam.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:52:30 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef SC_SPELLPARAM_HXX
#define SC_SPELLPARAM_HXX

#ifndef _SV_FONT_HXX
#include <vcl/font.hxx>
#endif

// ============================================================================

/** Specifiers for sheet conversion (functions iterating over the sheet and modifying cells). */
enum ScConversionType
{
    SC_CONVERSION_SPELLCHECK,       /// Spell checker.
    SC_CONVERSION_HANGULHANJA,      /// Hangul-Hanja converter.
    SC_CONVERSION_CHINESE_TRANSL    /// Chinese simplified/traditional converter.
};

// ---------------------------------------------------------------------------

/** Parameters for conversion. */
class ScConversionParam
{
public:
    /** Constructs an empty parameter struct with the passed conversion type. */
    explicit            ScConversionParam( ScConversionType eConvType );

    /** Constructs parameter struct for text conversion without changing the language. */
    explicit            ScConversionParam(
                            ScConversionType eConvType,
                            LanguageType eLang,
                            sal_Int32 nOptions,
                            bool bIsInteractive );

    /** Constructs parameter struct for text conversion with language change. */
    explicit            ScConversionParam(
                            ScConversionType eConvType,
                            LanguageType eSourceLang,
                            LanguageType eTargetLang,
                            const Font& rTargetFont,
                            sal_Int32 nOptions,
                            bool bIsInteractive );

    inline ScConversionType GetType() const     { return meConvType; }
    inline LanguageType GetSourceLang() const   { return meSourceLang; }
    inline LanguageType GetTargetLang() const   { return meTargetLang; }
    inline const Font*  GetTargetFont() const   { return mbUseTargetFont ? &maTargetFont : 0; }
    inline sal_Int32    GetOptions() const      { return mnOptions; }
    inline bool         IsInteractive() const   { return mbIsInteractive; }

private:
    ScConversionType    meConvType;         /// Type of the conversion.
    LanguageType        meSourceLang;       /// Source language for conversion.
    LanguageType        meTargetLang;       /// Target language for conversion.
    Font                maTargetFont;       /// Target font to be used if language has to be changed.
    sal_Int32           mnOptions;          /// Conversion options.
    bool                mbUseTargetFont;    /// True = Use maTargetFont to change font during conversion.
    bool                mbIsInteractive;    /// True = Text conversion has (specific) dialog that may be raised.
};

// ============================================================================

#endif

