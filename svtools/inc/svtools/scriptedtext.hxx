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

#ifndef _SVTOOLS_SCRIPTEDTEXT_HXX
#define _SVTOOLS_SCRIPTEDTEXT_HXX

#include "svtools/svtdllapi.h"
#include <tools/gen.hxx>
#include <com/sun/star/i18n/XBreakIterator.hpp>


namespace rtl { class OUString; }
class OutputDevice;
class Font;
class SvtScriptedTextHelper_Impl;


//_____________________________________________________________________________

/**
This class provides drawing text with different script types on any output devices.
*/
class SVT_DLLPUBLIC SvtScriptedTextHelper
{
private:
    SvtScriptedTextHelper_Impl* mpImpl;             /// Implementation of class functionality.

                                /** Assignment operator not implemented to prevent usage. */
    SvtScriptedTextHelper&      operator=( const SvtScriptedTextHelper& );

public:
                                /** Constructor sets an output device and no fonts.
                                    @param  _rOutDevice
                                    A reference to an output device. */
                                SvtScriptedTextHelper( OutputDevice& _rOutDevice );

                                /** Constructor sets an output device and fonts for all script types.
                                    @param  _rOutDevice
                                    A reference to an output device.
                                    @param  _pLatinFont
                                    The font for latin characters.
                                    @param  _pAsianFont
                                    The font for asian characters.
                                    @param  _pCmplxFont
                                    The font for complex text layout. */
                                SvtScriptedTextHelper(
                                    OutputDevice& _rOutDevice,
                                    Font* _pLatinFont,
                                    Font* _pAsianFont,
                                    Font* _pCmplxFont );

                                /** Copy constructor. */
                                SvtScriptedTextHelper(
                                    const SvtScriptedTextHelper& _rCopy );

                                /** Destructor. */
    virtual                     ~SvtScriptedTextHelper();

                                /** Sets new fonts and recalculates the text width.
                                    @param  _pLatinFont
                                    The font for latin characters.
                                    @param  _pAsianFont
                                    The font for asian characters.
                                    @param  _pCmplxFont
                                    The font for complex text layout. */
    void                        SetFonts( Font* _pLatinFont, Font* _pAsianFont, Font* _pCmplxFont );

                                /** Sets the default font of the current output device to all script types. */
    void                        SetDefaultFont();

                                /** Sets a new text and calculates all script breaks and the text width.
                                    @param  _rText
                                    The new text.
                                    @param  _xBreakIter
                                    The break iterator for iterating through the script portions. */
    void                        SetText(
                                    const ::rtl::OUString& _rText,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XBreakIterator >& _xBreakIter );

                                /** Returns the previously set text.
                                    @return  The current text. */
    const ::rtl::OUString&      GetText() const;

                                /** Returns the calculated width the text will take in the current output device.
                                    @return  The calculated text width. */
    sal_Int32                   GetTextWidth() const;

                                /** Returns the maximum height the text will take in the current output device.
                                    @return  The maximum text height. */
    sal_Int32                   GetTextHeight() const;

                                /** Returns a size struct containing the width and height of the text in the current output device.
                                    @return  A size struct with the text dimensions. */
    const Size&                 GetTextSize() const;

                                /** Draws the text in the current output device.
                                    @param _rPos
                                    The position of the top left edge of the text. */
    void                        DrawText( const Point& _rPos );
};

//_____________________________________________________________________________

#endif

