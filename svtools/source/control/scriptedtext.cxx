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
#include <com/sun/star/i18n/XBreakIterator.hpp>


using namespace ::std;
using namespace ::com::sun::star;


class SvtScriptedTextHelper_Impl
{
private:
    OutputDevice&               mrOutDevice;        /// The output device for drawing the text.
    vcl::Font                   maLatinFont;        /// The font for latin text portions.
    vcl::Font                   maAsianFont;        /// The font for asian text portions.
    vcl::Font                   maCmplxFont;        /// The font for complex text portions.
    vcl::Font const             maDefltFont;        /// The default font of the output device.
    OUString                    maText;             /// The text.

    vector< sal_Int32 >         maPosVec;           /// The start position of each text portion.
    vector< sal_Int16 >         maScriptVec;        /// The script type of each text portion.
    vector< sal_Int32 >         maWidthVec;         /// The output width of each text portion.
    Size                        maTextSize;         /// The size the text will take in the current output device.

                                /** Gets the font of the given script type. */
    const vcl::Font&            GetFont( sal_uInt16 _nScript ) const;
                                /** Sets a font on the output device depending on the script type. */
    void                 SetOutDevFont( sal_uInt16 _nScript )
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
    explicit                    SvtScriptedTextHelper_Impl(
                                    OutputDevice& _rOutDevice );

                                /** Sets new fonts and recalculates the text width. */
    void                        SetFonts( vcl::Font const * _pLatinFont, vcl::Font const * _pAsianFont, vcl::Font const * _pCmplxFont );
                                /** Sets a new text and calculates all script breaks and the text width. */
    void                        SetText(
                                    const OUString& _rText,
                                    const uno::Reference< i18n::XBreakIterator >& _xBreakIter );

                                /** Returns a size struct containing the width and height of the text in the current output device. */
    const Size&                 GetTextSize() const { return maTextSize;}

                                /** Draws the text in the current output device. */
    void                        DrawText( const Point& _rPos );
};


SvtScriptedTextHelper_Impl::SvtScriptedTextHelper_Impl(
        OutputDevice& _rOutDevice ) :
    mrOutDevice( _rOutDevice ),
    maLatinFont( _rOutDevice.GetFont() ),
    maAsianFont( _rOutDevice.GetFont() ),
    maCmplxFont( _rOutDevice.GetFont() ),
    maDefltFont( _rOutDevice.GetFont() )
{
}

const vcl::Font& SvtScriptedTextHelper_Impl::GetFont( sal_uInt16 _nScript ) const
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
    maTextSize.setWidth(0);
    maTextSize.setHeight(0);
    mrOutDevice.Push(PushFlags::FONT | PushFlags::TEXTCOLOR);

    // calculate text portion widths and total width
    maWidthVec.clear();
    if( !maPosVec.empty() )
    {
        DBG_ASSERT( maPosVec.size() - 1 == maScriptVec.size(),
            "SvtScriptedTextHelper_Impl::CalculateWidth - invalid vectors" );

        sal_Int32 nThisPos = maPosVec[ 0 ];
        sal_Int32 nNextPos;
        sal_Int32 nPosVecSize = maPosVec.size();
        sal_Int32 nPosVecIndex = 1;

        sal_Int16 nScript;
        sal_Int32 nScriptVecIndex = 0;

        sal_Int32 nCurrWidth;

        while( nPosVecIndex < nPosVecSize )
        {
            nNextPos = maPosVec[ nPosVecIndex++ ];
            nScript = maScriptVec[ nScriptVecIndex++ ];

            SetOutDevFont( nScript );
            nCurrWidth = mrOutDevice.GetTextWidth( maText, nThisPos, nNextPos - nThisPos );
            maWidthVec.push_back( nCurrWidth );
            maTextSize.AdjustWidth(nCurrWidth );
            nThisPos = nNextPos;
        }
    }

    // calculate maximum font height
    SetOutDevFont( i18n::ScriptType::LATIN );
    maTextSize.setHeight( std::max( maTextSize.Height(), mrOutDevice.GetTextHeight() ) );
    SetOutDevFont( i18n::ScriptType::ASIAN );
    maTextSize.setHeight( std::max( maTextSize.Height(), mrOutDevice.GetTextHeight() ) );
    SetOutDevFont( i18n::ScriptType::COMPLEX );
    maTextSize.setHeight( std::max( maTextSize.Height(), mrOutDevice.GetTextHeight() ) );

    mrOutDevice.Pop();
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
                                    nNextCharIx = mrOutDevice.HasGlyphs( GetFont( nScript ), maText, nCharIx, nNextPos - nCharIx );
                                    if( nCharIx == nNextCharIx )
                                        ++nScript;
                                }
                                if( nNextCharIx == nCharIx )
                                    ++nNextCharIx;

                                maPosVec.push_back( nCharIx );
                                maScriptVec.push_back( nScript );
                                nCharIx = nNextCharIx;
                            }
                            while( nCharIx < nNextPos && nCharIx != -1 );
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

void SvtScriptedTextHelper_Impl::SetFonts( vcl::Font const * _pLatinFont, vcl::Font const * _pAsianFont, vcl::Font const * _pCmplxFont )
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


void SvtScriptedTextHelper_Impl::DrawText( const Point& _rPos )
{
    if( maText.isEmpty() || maPosVec.empty() )
        return;

    DBG_ASSERT( maPosVec.size() - 1 == maScriptVec.size(), "SvtScriptedTextHelper_Impl::DrawText - invalid vectors" );
    DBG_ASSERT( maScriptVec.size() == maWidthVec.size(), "SvtScriptedTextHelper_Impl::DrawText - invalid vectors" );

    mrOutDevice.Push(PushFlags::FONT | PushFlags::TEXTCOLOR);

    Point aCurrPos( _rPos );
    sal_Int32 nThisPos = maPosVec[ 0 ];
    sal_Int32 nNextPos;
    sal_Int32 nPosVecSize = maPosVec.size();
    sal_Int32 nPosVecIndex = 1;

    sal_Int16 nScript;
    sal_Int32 nVecIndex = 0;

    while( nPosVecIndex < nPosVecSize )
    {
        nNextPos = maPosVec[ nPosVecIndex++ ];
        nScript = maScriptVec[ nVecIndex ];

        SetOutDevFont( nScript );
        mrOutDevice.DrawText( aCurrPos, maText, nThisPos, nNextPos - nThisPos );
        aCurrPos.AdjustX(maWidthVec[ nVecIndex++ ] );
        aCurrPos.AdjustX(mrOutDevice.GetTextHeight() / 5 );   // add 20% of font height as portion spacing
        nThisPos = nNextPos;
    }

    mrOutDevice.Pop();
}


SvtScriptedTextHelper::SvtScriptedTextHelper( OutputDevice& _rOutDevice ) :
    mpImpl( new SvtScriptedTextHelper_Impl( _rOutDevice ) )
{
}

SvtScriptedTextHelper::SvtScriptedTextHelper( const SvtScriptedTextHelper& _rCopy ) :
    mpImpl( new SvtScriptedTextHelper_Impl( *_rCopy.mpImpl ) )
{
}

SvtScriptedTextHelper::~SvtScriptedTextHelper()
{
}

void SvtScriptedTextHelper::SetFonts( vcl::Font const * _pLatinFont, vcl::Font const * _pAsianFont, vcl::Font const * _pCmplxFont )
{
    mpImpl->SetFonts( _pLatinFont, _pAsianFont, _pCmplxFont );
}

void SvtScriptedTextHelper::SetDefaultFont()
{
    mpImpl->SetFonts( nullptr, nullptr, nullptr );
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
