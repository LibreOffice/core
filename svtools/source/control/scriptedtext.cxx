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

#include <svtools/scriptedtext.hxx>
#include <vector>
#include <rtl/ustring.hxx>
#include <vcl/outdev.hxx>
#include <vcl/font.hxx>
#include <tools/debug.hxx>
#include <tools/gen.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>


using namespace ::std;
using namespace ::rtl;
using namespace ::com::sun::star;


//_____________________________________________________________________________

class SvtScriptedTextHelper_Impl
{
private:
    OutputDevice&               mrOutDevice;        /// The output device for drawing the text.
    Font                        maLatinFont;        /// The font for latin text portions.
    Font                        maAsianFont;        /// The font for asian text portions.
    Font                        maCmplxFont;        /// The font for complex text portions.
    Font                        maDefltFont;        /// The default font of the output device.
    OUString                    maText;             /// The text.

    vector< sal_Int32 >         maPosVec;           /// The start position of each text portion.
    vector< sal_Int16 >         maScriptVec;        /// The script type of each text portion.
    vector< sal_Int32 >         maWidthVec;         /// The output width of each text portion.
    Size                        maTextSize;         /// The size the text will take in the current output device.

                                /** Assignment operator not implemented to prevent usage. */
    SvtScriptedTextHelper_Impl& operator=( const SvtScriptedTextHelper_Impl& );

                                /** Gets the font of the given script type. */
    const Font&                 GetFont( sal_uInt16 _nScript ) const;
                                /** Sets a font on the output device depending on the script type. */
    inline void                 SetOutDevFont( sal_uInt16 _nScript )
                                    { mrOutDevice.SetFont( GetFont( _nScript ) ); }
                                /** Fills maPosVec with positions of all changes of script type.
                                    This method expects correctly initialized maPosVec and maScriptVec. */
    void                        CalculateSizes();
                                /** Fills maPosVec with positions of all changes of script type and
                                    maScriptVec with the script type of each portion. */
    void                        CalculateBreaks(
                                    const uno::Reference< i18n::XBreakIterator >& _xBreakIter );

public:
                                /** This constructor sets an output device and fonts for all script types. */
                                SvtScriptedTextHelper_Impl(
                                    OutputDevice& _rOutDevice,
                                    Font* _pLatinFont,
                                    Font* _pAsianFont,
                                    Font* _pCmplxFont );
                                /** Copy constructor. */
                                SvtScriptedTextHelper_Impl(
                                    const SvtScriptedTextHelper_Impl& _rCopy );
                                /** Destructor. */
                                ~SvtScriptedTextHelper_Impl();

                                /** Sets new fonts and recalculates the text width. */
    void                        SetFonts( Font* _pLatinFont, Font* _pAsianFont, Font* _pCmplxFont );
                                /** Sets a new text and calculates all script breaks and the text width. */
    void                        SetText(
                                    const OUString& _rText,
                                    const uno::Reference< i18n::XBreakIterator >& _xBreakIter );

                                /** Returns a size struct containing the width and height of the text in the current output device. */
    const Size&                 GetTextSize() const;

                                /** Draws the text in the current output device. */
    void                        DrawText( const Point& _rPos );
};


SvtScriptedTextHelper_Impl::SvtScriptedTextHelper_Impl(
        OutputDevice& _rOutDevice,
        Font* _pLatinFont, Font* _pAsianFont, Font* _pCmplxFont ) :
    mrOutDevice( _rOutDevice ),
    maLatinFont( _pLatinFont ? *_pLatinFont : _rOutDevice.GetFont() ),
    maAsianFont( _pAsianFont ? *_pAsianFont : _rOutDevice.GetFont() ),
    maCmplxFont( _pCmplxFont ? *_pCmplxFont : _rOutDevice.GetFont() ),
    maDefltFont( _rOutDevice.GetFont() )
{
}

SvtScriptedTextHelper_Impl::SvtScriptedTextHelper_Impl( const SvtScriptedTextHelper_Impl& _rCopy ) :
    mrOutDevice( _rCopy.mrOutDevice ),
    maLatinFont( _rCopy.maLatinFont ),
    maAsianFont( _rCopy.maAsianFont ),
    maCmplxFont( _rCopy.maCmplxFont ),
    maDefltFont( _rCopy.maDefltFont ),
    maText( _rCopy.maText ),
    maPosVec( _rCopy.maPosVec ),
    maScriptVec( _rCopy.maScriptVec ),
    maWidthVec( _rCopy.maWidthVec ),
    maTextSize( _rCopy.maTextSize )
{
}

SvtScriptedTextHelper_Impl::~SvtScriptedTextHelper_Impl()
{
}

const Font& SvtScriptedTextHelper_Impl::GetFont( sal_uInt16 _nScript ) const
{
    switch( _nScript )
    {
        case i18n::ScriptType::LATIN:       return maLatinFont;
        case i18n::ScriptType::ASIAN:       return maAsianFont;
        case i18n::ScriptType::COMPLEX:     return maCmplxFont;
    }
    return maDefltFont;
}

void SvtScriptedTextHelper_Impl::CalculateSizes()
{
    maTextSize.Width() = maTextSize.Height() = 0;
    maDefltFont = mrOutDevice.GetFont();

    // calculate text portion widths and total width
    maWidthVec.clear();
    if( !maPosVec.empty() )
    {
        DBG_ASSERT( maPosVec.size() - 1 == maScriptVec.size(),
            "SvtScriptedTextHelper_Impl::CalculateWidth - invalid vectors" );

        xub_StrLen nThisPos = static_cast< xub_StrLen >( maPosVec[ 0 ] );
        xub_StrLen nNextPos;
        sal_Int32 nPosVecSize = maPosVec.size();
        sal_Int32 nPosVecIndex = 1;

        sal_Int16 nScript;
        sal_Int32 nScriptVecIndex = 0;

        sal_Int32 nCurrWidth;

        while( nPosVecIndex < nPosVecSize )
        {
            nNextPos = static_cast< xub_StrLen >( maPosVec[ nPosVecIndex++ ] );
            nScript = maScriptVec[ nScriptVecIndex++ ];

            SetOutDevFont( nScript );
            nCurrWidth = mrOutDevice.GetTextWidth( maText, nThisPos, nNextPos - nThisPos );
            maWidthVec.push_back( nCurrWidth );
            maTextSize.Width() += nCurrWidth;
            nThisPos = nNextPos;
        }
    }

    // calculate maximum font height
    SetOutDevFont( i18n::ScriptType::LATIN );
    maTextSize.Height() = std::max( maTextSize.Height(), mrOutDevice.GetTextHeight() );
    SetOutDevFont( i18n::ScriptType::ASIAN );
    maTextSize.Height() = std::max( maTextSize.Height(), mrOutDevice.GetTextHeight() );
    SetOutDevFont( i18n::ScriptType::COMPLEX );
    maTextSize.Height() = std::max( maTextSize.Height(), mrOutDevice.GetTextHeight() );

    mrOutDevice.SetFont( maDefltFont );
}

void SvtScriptedTextHelper_Impl::CalculateBreaks( const uno::Reference< i18n::XBreakIterator >& _xBreakIter )
{
    maPosVec.clear();
    maScriptVec.clear();

    DBG_ASSERT( _xBreakIter.is(), "SvtScriptedTextHelper_Impl::CalculateBreaks - no break iterator" );

    sal_Int32 nLen = maText.getLength();
    if( nLen )
    {
        if( _xBreakIter.is() )
        {
            sal_Int32 nThisPos = 0;         // first position of this portion
            sal_Int32 nNextPos = 0;         // first position of next portion
            sal_Int16 nPortScript;          // script type of this portion
            do
            {
                nPortScript = _xBreakIter->getScriptType( maText, nThisPos );
                nNextPos = _xBreakIter->endOfScript( maText, nThisPos, nPortScript );

                switch( nPortScript )
                {
                    case i18n::ScriptType::LATIN:
                    case i18n::ScriptType::ASIAN:
                    case i18n::ScriptType::COMPLEX:
                        maPosVec.push_back( nThisPos );
                        maScriptVec.push_back( nPortScript );
                    break;
                    default:
                    {
/* *** handling of weak characters ***
- first portion is weak: Use OutputDevice::HasGlyphs() to find the correct font
- weak portion follows another portion: Script type of preceding portion is used */
                        if( maPosVec.empty() )
                        {
                            sal_Int32 nCharIx = 0;
                            sal_Int32 nNextCharIx = 0;
                            sal_Int16 nScript;
                            do
                            {
                                nScript = i18n::ScriptType::LATIN;
                                while( (nScript != i18n::ScriptType::WEAK) && (nCharIx == nNextCharIx) )
                                {
                                    nNextCharIx = mrOutDevice.HasGlyphs( GetFont( nScript ), maText, sal::static_int_cast< sal_uInt16 >(nCharIx), sal::static_int_cast< sal_uInt16 >(nNextPos - nCharIx) );
                                    if( nCharIx == nNextCharIx )
                                        ++nScript;
                                }
                                if( nNextCharIx == nCharIx )
                                    ++nNextCharIx;

                                maPosVec.push_back( nCharIx );
                                maScriptVec.push_back( nScript );
                                nCharIx = nNextCharIx;
                            }
                            while( nCharIx < nNextPos );
                        }
                        // nothing to do for following portions
                    }
                }
                nThisPos = nNextPos;
            }
            while( (0 <= nThisPos) && (nThisPos < nLen) );
        }
        else            // no break iterator: whole text LATIN
        {
            maPosVec.push_back( 0 );
            maScriptVec.push_back( i18n::ScriptType::LATIN );
        }

        // push end position of last portion
        if( !maPosVec.empty() )
            maPosVec.push_back( nLen );
    }
    CalculateSizes();
}

void SvtScriptedTextHelper_Impl::SetFonts( Font* _pLatinFont, Font* _pAsianFont, Font* _pCmplxFont )
{
    maLatinFont = _pLatinFont ? *_pLatinFont : maDefltFont;
    maAsianFont = _pAsianFont ? *_pAsianFont : maDefltFont;
    maCmplxFont = _pCmplxFont ? *_pCmplxFont : maDefltFont;
    CalculateSizes();
}

void SvtScriptedTextHelper_Impl::SetText( const OUString& _rText, const uno::Reference< i18n::XBreakIterator >& _xBreakIter )
{
    maText = _rText;
    CalculateBreaks( _xBreakIter );
}

const Size& SvtScriptedTextHelper_Impl::GetTextSize() const
{
    return maTextSize;
}

void SvtScriptedTextHelper_Impl::DrawText( const Point& _rPos )
{
    if( maText.isEmpty() || maPosVec.empty() )
        return;

    DBG_ASSERT( maPosVec.size() - 1 == maScriptVec.size(), "SvtScriptedTextHelper_Impl::DrawText - invalid vectors" );
    DBG_ASSERT( maScriptVec.size() == maWidthVec.size(), "SvtScriptedTextHelper_Impl::DrawText - invalid vectors" );

    maDefltFont = mrOutDevice.GetFont();
    Point aCurrPos( _rPos );
    xub_StrLen nThisPos = static_cast< xub_StrLen >( maPosVec[ 0 ] );
    xub_StrLen nNextPos;
    sal_Int32 nPosVecSize = maPosVec.size();
    sal_Int32 nPosVecIndex = 1;

    sal_Int16 nScript;
    sal_Int32 nVecIndex = 0;

    while( nPosVecIndex < nPosVecSize )
    {
        nNextPos = static_cast< xub_StrLen >( maPosVec[ nPosVecIndex++ ] );
        nScript = maScriptVec[ nVecIndex ];

        SetOutDevFont( nScript );
        mrOutDevice.DrawText( aCurrPos, maText, nThisPos, nNextPos - nThisPos );
        aCurrPos.X() += maWidthVec[ nVecIndex++ ];
        aCurrPos.X() += mrOutDevice.GetTextHeight() / 5;   // add 20% of font height as portion spacing
        nThisPos = nNextPos;
    }
    mrOutDevice.SetFont( maDefltFont );
}


//_____________________________________________________________________________

SvtScriptedTextHelper::SvtScriptedTextHelper( OutputDevice& _rOutDevice ) :
    mpImpl( new SvtScriptedTextHelper_Impl( _rOutDevice, NULL, NULL, NULL ) )
{
}

SvtScriptedTextHelper::SvtScriptedTextHelper( const SvtScriptedTextHelper& _rCopy ) :
    mpImpl( new SvtScriptedTextHelper_Impl( *_rCopy.mpImpl ) )
{
}

SvtScriptedTextHelper::~SvtScriptedTextHelper()
{
    delete mpImpl;
}

void SvtScriptedTextHelper::SetFonts( Font* _pLatinFont, Font* _pAsianFont, Font* _pCmplxFont )
{
    mpImpl->SetFonts( _pLatinFont, _pAsianFont, _pCmplxFont );
}

void SvtScriptedTextHelper::SetDefaultFont()
{
    mpImpl->SetFonts( NULL, NULL, NULL );
}

void SvtScriptedTextHelper::SetText( const OUString& _rText, const uno::Reference< i18n::XBreakIterator >& _xBreakIter )
{
    mpImpl->SetText( _rText, _xBreakIter );
}

const Size& SvtScriptedTextHelper::GetTextSize() const
{
    return mpImpl->GetTextSize();
}

void SvtScriptedTextHelper::DrawText( const Point& _rPos )
{
    mpImpl->DrawText( _rPos );
}


//_____________________________________________________________________________

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
