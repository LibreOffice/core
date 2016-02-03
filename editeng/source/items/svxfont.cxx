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

#include <comphelper/string.hxx>
#include <vcl/outdev.hxx>
#include <vcl/print.hxx>
#include <tools/gen.hxx>
#include <tools/poly.hxx>
#include <unotools/charclass.hxx>
#include <editeng/unolingu.hxx>
#include <com/sun/star/i18n/KCharacterType.hpp>
#include <editeng/svxfont.hxx>
#include <editeng/escapementitem.hxx>

SvxFont::SvxFont()
{
    nKern = nEsc = 0;
    nPropr = 100;
    eCaseMap = SVX_CASEMAP_NOT_MAPPED;
    SetLanguage(LANGUAGE_SYSTEM);
}

SvxFont::SvxFont( const vcl::Font &rFont )
    : Font( rFont )
{
    nKern = nEsc = 0;
    nPropr = 100;
    eCaseMap = SVX_CASEMAP_NOT_MAPPED;
    SetLanguage(LANGUAGE_SYSTEM);
}

SvxFont::SvxFont( const SvxFont &rFont )
    : Font( rFont )
{
    nKern = rFont.GetFixKerning();
    nEsc  = rFont.GetEscapement();
    nPropr = rFont.GetPropr();
    eCaseMap = rFont.GetCaseMap();
    SetLanguage(rFont.GetLanguage());
}

void SvxFont::DrawArrow( OutputDevice &rOut, const Rectangle& rRect,
    const Size& rSize, const Color& rCol, bool bLeft )
{
    long nLeft = ( rRect.Left() + rRect.Right() - rSize.Width() )/ 2;
    long nRight = nLeft + rSize.Width();
    long nMid = ( rRect.Top() + rRect.Bottom() ) / 2;
    long nTop = nMid - rSize.Height() / 2;
    long nBottom = nTop + rSize.Height();
    if( nLeft < rRect.Left() )
    {
        nLeft = rRect.Left();
        nRight = rRect.Right();
    }
    if( nTop < rRect.Top() )
    {
        nTop = rRect.Top();
        nBottom = rRect.Bottom();
    }
    tools::Polygon aPoly;
    Point aTmp( bLeft ? nLeft : nRight, nMid );
    Point aNxt( bLeft ? nRight : nLeft, nTop );
    aPoly.Insert( 0, aTmp );
    aPoly.Insert( 0, aNxt );
    aNxt.Y() = nBottom;
    aPoly.Insert( 0, aNxt );
    aPoly.Insert( 0, aTmp );
    Color aOldLineColor = rOut.GetLineColor();
    Color aOldFillColor = rOut.GetFillColor();
    rOut.SetFillColor( rCol );
    rOut.SetLineColor( Color( COL_BLACK ) );
    rOut.DrawPolygon( aPoly );
    rOut.DrawLine( aTmp, aNxt );
    rOut.SetLineColor( aOldLineColor );
    rOut.SetFillColor( aOldFillColor );
}


OUString SvxFont::CalcCaseMap(const OUString &rTxt) const
{
    if (!IsCaseMap() || rTxt.isEmpty())
        return rTxt;
    OUString aTxt(rTxt);
    // I still have to get the language
    const LanguageType eLang = LANGUAGE_DONTKNOW == GetLanguage()
                             ? LANGUAGE_SYSTEM : GetLanguage();

    LanguageTag aLanguageTag(eLang);
    CharClass aCharClass( aLanguageTag );

    switch( eCaseMap )
    {
        case SVX_CASEMAP_KAPITAELCHEN:
        case SVX_CASEMAP_VERSALIEN:
        {
            aTxt = aCharClass.uppercase( aTxt );
            break;
        }

        case SVX_CASEMAP_GEMEINE:
        {
            aTxt = aCharClass.lowercase( aTxt );
            break;
        }
        case SVX_CASEMAP_TITEL:
        {
            // Every beginning of a word is capitalized,  the rest of the word
            // is taken over as is.
            // Bug: if the attribute starts in the middle of the word.
            bool bBlank = true;

            for (sal_Int32 i = 0; i < aTxt.getLength(); ++i)
            {
                if( aTxt[i] == ' ' || aTxt[i] == '\t')
                    bBlank = true;
                else
                {
                    if (bBlank)
                    {
                        OUString sTitle(aCharClass.uppercase(OUString(aTxt[i])));
                        aTxt = aTxt.replaceAt(i, 1, sTitle);
                    }
                    bBlank = false;
                }
            }
            break;
        }
        default:
        {
            DBG_ASSERT(false, "SvxFont::CaseMapTxt: unknown casemap");
            break;
        }
    }
    return aTxt;
}

/*************************************************************************
 *                      class SvxDoCapitals
 * The virtual Method Do si called by SvxFont::DoOnCapitals alternately
 * the uppercase and lowercase parts. The derivate of SvxDoCapitals fills
 * this method with life.
 *************************************************************************/

class SvxDoCapitals
{
protected:
    VclPtr<OutputDevice> pOut;
    const OUString &rTxt;
    const sal_Int32 nIdx;
    const sal_Int32 nLen;

public:
    SvxDoCapitals( OutputDevice *_pOut, const OUString &_rTxt,
                   const sal_Int32 _nIdx, const sal_Int32 _nLen )
        : pOut(_pOut), rTxt(_rTxt), nIdx(_nIdx), nLen(_nLen)
        { }

    virtual ~SvxDoCapitals() {}

    virtual void DoSpace( const bool bDraw );
    virtual void SetSpace();
    virtual void Do( const OUString &rTxt,
                     const sal_Int32 nIdx, const sal_Int32 nLen,
                     const bool bUpper ) = 0;

    const OUString &GetTxt() const { return rTxt; }
    sal_Int32 GetIdx() const { return nIdx; }
    sal_Int32 GetLen() const { return nLen; }
};

void SvxDoCapitals::DoSpace( const bool /*bDraw*/ ) { }

void SvxDoCapitals::SetSpace() { }

/*************************************************************************
 *                  SvxFont::DoOnCapitals() const
 * Decomposes the String into uppercase and lowercase letters and then
 * calls the method SvxDoCapitals::Do( ).
 *************************************************************************/

void SvxFont::DoOnCapitals(SvxDoCapitals &rDo) const
{
    const OUString &rTxt = rDo.GetTxt();
    const sal_Int32 nIdx = rDo.GetIdx();
    const sal_Int32 nLen = rDo.GetLen();

    const OUString aTxt( CalcCaseMap( rTxt ) );
    const sal_Int32 nTxtLen = std::min( rTxt.getLength(), nLen );
    sal_Int32 nPos = 0;
    sal_Int32 nOldPos = nPos;

    // Test if string length differ between original and CaseMapped
    bool bCaseMapLengthDiffers(aTxt.getLength() != rTxt.getLength());

    const LanguageType eLang = LANGUAGE_DONTKNOW == GetLanguage()
                             ? LANGUAGE_SYSTEM : GetLanguage();

    LanguageTag aLanguageTag(eLang);
    CharClass   aCharClass( aLanguageTag );
    OUString    aCharString;

    while( nPos < nTxtLen )
    {
        // first in turn are the uppercase letters

        // There are characters that are both upper- and lower-case L (eg blank)
        // Such ambiguities lead to chaos, this is why these characters are
        // allocated to the lowercase characters!

        while( nPos < nTxtLen )
        {
            aCharString = rTxt.copy( nPos + nIdx, 1 );
            sal_Int32 nCharacterType = aCharClass.getCharacterType( aCharString, 0 );
            if ( nCharacterType & css::i18n::KCharacterType::LOWER )
                break;
            if ( ! ( nCharacterType & css::i18n::KCharacterType::UPPER ) )
                break;
            ++nPos;
        }
        if( nOldPos != nPos )
        {
            if(bCaseMapLengthDiffers)
            {
                // If strings differ work preparing the necessary snippet to address that
                // potential difference
                const OUString aSnippet = rTxt.copy(nIdx + nOldPos, nPos-nOldPos);
                OUString aNewText = CalcCaseMap(aSnippet);

                rDo.Do( aNewText, 0, aNewText.getLength(), true );
            }
            else
            {
                rDo.Do( aTxt, nIdx + nOldPos, nPos-nOldPos, true );
            }

            nOldPos = nPos;
        }
        // Now the lowercase are processed (without blanks)
        while( nPos < nTxtLen )
        {
            sal_uInt32  nCharacterType = aCharClass.getCharacterType( aCharString, 0 );
            if ( ( nCharacterType & css::i18n::KCharacterType::UPPER ) )
                break;
            if ( aCharString == " " )
                break;
            if( ++nPos < nTxtLen )
                aCharString = rTxt.copy( nPos + nIdx, 1 );
        }
        if( nOldPos != nPos )
        {
            if(bCaseMapLengthDiffers)
            {
                // If strings differ work preparing the necessary snippet to address that
                // potential difference
                const OUString aSnippet = rTxt.copy(nIdx + nOldPos, nPos - nOldPos);
                OUString aNewText = CalcCaseMap(aSnippet);

                rDo.Do( aNewText, 0, aNewText.getLength(), false );
            }
            else
            {
                rDo.Do( aTxt, nIdx + nOldPos, nPos-nOldPos, false );
            }

            nOldPos = nPos;
        }
        // Now the blanks are<processed
        while( nPos < nTxtLen && aCharString == " " && ++nPos < nTxtLen )
            aCharString = rTxt.copy( nPos + nIdx, 1 );

        if( nOldPos != nPos )
        {
            rDo.DoSpace( false );

            if(bCaseMapLengthDiffers)
            {
                // If strings differ work preparing the necessary snippet to address that
                // potential difference
                const OUString aSnippet = rTxt.copy(nIdx + nOldPos, nPos - nOldPos);
                OUString aNewText = CalcCaseMap(aSnippet);

                rDo.Do( aNewText, 0, aNewText.getLength(), false );
            }
            else
            {
                rDo.Do( aTxt, nIdx + nOldPos, nPos - nOldPos, false );
            }

            nOldPos = nPos;
            rDo.SetSpace();
        }
    }
    rDo.DoSpace( true );
}


void SvxFont::SetPhysFont( OutputDevice *pOut ) const
{
    const vcl::Font& rCurrentFont = pOut->GetFont();
    if ( nPropr == 100 )
    {
        if ( !rCurrentFont.IsSameInstance( *this ) )
            pOut->SetFont( *this );
    }
    else
    {
        Font aNewFont( *this );
        Size aSize( aNewFont.GetFontSize() );
        aNewFont.SetFontSize( Size( aSize.Width() * nPropr / 100L,
                                    aSize.Height() * nPropr / 100L ) );
        if ( !rCurrentFont.IsSameInstance( aNewFont ) )
            pOut->SetFont( aNewFont );
    }
}


vcl::Font SvxFont::ChgPhysFont( OutputDevice *pOut ) const
{
    vcl::Font aOldFont( pOut->GetFont() );
    SetPhysFont( pOut );
    return aOldFont;
}


Size SvxFont::GetPhysTxtSize( const OutputDevice *pOut, const OUString &rTxt,
                         const sal_Int32 nIdx, const sal_Int32 nLen ) const
{
    if ( !IsCaseMap() && !IsKern() )
        return Size( pOut->GetTextWidth( rTxt, nIdx, nLen ),
                     pOut->GetTextHeight() );

    Size aTxtSize;
    aTxtSize.setHeight( pOut->GetTextHeight() );
    if ( !IsCaseMap() )
        aTxtSize.setWidth( pOut->GetTextWidth( rTxt, nIdx, nLen ) );
    else
    {
        const OUString aNewText = CalcCaseMap(rTxt);
        bool bCaseMapLengthDiffers(aNewText.getLength() != rTxt.getLength());
        sal_Int32 nWidth(0L);

        if(bCaseMapLengthDiffers)
        {
            // If strings differ work preparing the necessary snippet to address that
            // potential difference
            const OUString aSnippet = rTxt.copy(nIdx, nLen);
            OUString _aNewText = CalcCaseMap(aSnippet);
            nWidth = pOut->GetTextWidth( _aNewText, 0, _aNewText.getLength() );
        }
        else
        {
            nWidth = pOut->GetTextWidth( aNewText, nIdx, nLen );
        }

        aTxtSize.setWidth(nWidth);
    }

    if( IsKern() && ( nLen > 1 ) )
        aTxtSize.Width() += ( ( nLen-1 ) * long( nKern ) );

    return aTxtSize;
}

Size SvxFont::GetPhysTxtSize( const OutputDevice *pOut, const OUString &rTxt )
{
    if ( !IsCaseMap() && !IsKern() )
        return Size( pOut->GetTextWidth( rTxt ), pOut->GetTextHeight() );

    Size aTxtSize;
    aTxtSize.setHeight( pOut->GetTextHeight() );
    if ( !IsCaseMap() )
        aTxtSize.setWidth( pOut->GetTextWidth( rTxt ) );
    else
        aTxtSize.setWidth( pOut->GetTextWidth( CalcCaseMap( rTxt ) ) );

    if( IsKern() && ( rTxt.getLength() > 1 ) )
        aTxtSize.Width() += ( ( rTxt.getLength()-1 ) * long( nKern ) );

    return aTxtSize;
}

Size SvxFont::QuickGetTextSize( const OutputDevice *pOut, const OUString &rTxt,
                         const sal_Int32 nIdx, const sal_Int32 nLen, long* pDXArray ) const
{
    if ( !IsCaseMap() && !IsKern() )
        return Size( pOut->GetTextArray( rTxt, pDXArray, nIdx, nLen ),
                     pOut->GetTextHeight() );

    Size aTxtSize;
    aTxtSize.setHeight( pOut->GetTextHeight() );
    if ( !IsCaseMap() )
        aTxtSize.setWidth( pOut->GetTextArray( rTxt, pDXArray, nIdx, nLen ) );
    else
        aTxtSize.setWidth( pOut->GetTextArray( CalcCaseMap( rTxt ),
                           pDXArray, nIdx, nLen ) );

    if( IsKern() && ( nLen > 1 ) )
    {
        aTxtSize.Width() += ( ( nLen-1 ) * long( nKern ) );

        if ( pDXArray )
        {
            for ( sal_Int32 i = 0; i < nLen; i++ )
                pDXArray[i] += ( (i+1) * long( nKern ) );
            // The last one is a nKern too big:
            pDXArray[nLen-1] -= nKern;
        }
    }
    return aTxtSize;
}


Size SvxFont::GetTextSize( const OutputDevice *pOut, const OUString &rTxt,
                         const sal_Int32 nIdx, const sal_Int32 nLen ) const
{
    sal_Int32 nTmp = nLen;
    if ( nTmp == SAL_MAX_INT32 )   // already initialized?
        nTmp = rTxt.getLength();
    Font aOldFont( ChgPhysFont(const_cast<OutputDevice *>(pOut)) );
    Size aTxtSize;
    if( IsCapital() && !rTxt.isEmpty() )
    {
        aTxtSize = GetCapitalSize( pOut, rTxt, nIdx, nTmp );
    }
    else aTxtSize = GetPhysTxtSize(pOut,rTxt,nIdx,nTmp);
    const_cast<OutputDevice *>(pOut)->SetFont( aOldFont );
    return aTxtSize;
}


void SvxFont::QuickDrawText( OutputDevice *pOut,
    const Point &rPos, const OUString &rTxt,
    const sal_Int32 nIdx, const sal_Int32 nLen, const long* pDXArray ) const
{

    // Font has to be selected in OutputDevice...
    if ( !IsCaseMap() && !IsCapital() && !IsKern() && !IsEsc() )
    {
        pOut->DrawTextArray( rPos, rTxt, pDXArray, nIdx, nLen );
        return;
    }

    Point aPos( rPos );

    if ( nEsc )
    {
        long nDiff = GetFontSize().Height();
        nDiff *= nEsc;
        nDiff /= 100;

        if ( !IsVertical() )
            aPos.Y() -= nDiff;
        else
            aPos.X() += nDiff;
    }

    if( IsCapital() )
    {
        DBG_ASSERT( !pDXArray, "DrawCapital not for TextArray!" );
        DrawCapital( pOut, aPos, rTxt, nIdx, nLen );
    }
    else
    {
        if ( IsKern() && !pDXArray )
        {
            Size aSize = GetPhysTxtSize( pOut, rTxt, nIdx, nLen );

            if ( !IsCaseMap() )
                pOut->DrawStretchText( aPos, aSize.Width(), rTxt, nIdx, nLen );
            else
                pOut->DrawStretchText( aPos, aSize.Width(), CalcCaseMap( rTxt ), nIdx, nLen );
        }
        else
        {
            if ( !IsCaseMap() )
                pOut->DrawTextArray( aPos, rTxt, pDXArray, nIdx, nLen );
            else
                pOut->DrawTextArray( aPos, CalcCaseMap( rTxt ), pDXArray, nIdx, nLen );
        }
    }
}


void SvxFont::DrawPrev( OutputDevice *pOut, Printer* pPrinter,
                        const Point &rPos, const OUString &rTxt,
                        const sal_Int32 nIdx, const sal_Int32 nLen ) const
{
    if ( !nLen || rTxt.isEmpty() )
        return;
    sal_Int32 nTmp = nLen;

    if ( nTmp == SAL_MAX_INT32 )   // already initialized?
        nTmp = rTxt.getLength();
    Point aPos( rPos );

    if ( nEsc )
    {
        short nTmpEsc;
        if( DFLT_ESC_AUTO_SUPER == nEsc )
            nTmpEsc = 33;
        else if( DFLT_ESC_AUTO_SUB == nEsc )
            nTmpEsc = -20;
        else
            nTmpEsc = nEsc;
        Size aSize = ( this->GetFontSize() );
        aPos.Y() -= ( ( nTmpEsc * long( aSize.Height() ) ) / 100L );
    }
    Font aOldFont( ChgPhysFont( pOut ) );
    Font aOldPrnFont( ChgPhysFont( pPrinter ) );

    if ( IsCapital() )
        DrawCapital( pOut, aPos, rTxt, nIdx, nTmp );
    else
    {
        Size aSize = GetPhysTxtSize( pPrinter, rTxt, nIdx, nTmp );

        if ( !IsCaseMap() )
            pOut->DrawStretchText( aPos, aSize.Width(), rTxt, nIdx, nTmp );
        else
        {
            const OUString aNewText = CalcCaseMap(rTxt);
            bool bCaseMapLengthDiffers(aNewText.getLength() != rTxt.getLength());

            if(bCaseMapLengthDiffers)
            {
                // If strings differ work preparing the necessary snippet to address that
                // potential difference
                const OUString aSnippet(rTxt.copy( nIdx, nTmp));
                OUString _aNewText = CalcCaseMap(aSnippet);

                pOut->DrawStretchText( aPos, aSize.Width(), _aNewText, 0, _aNewText.getLength() );
            }
            else
            {
                pOut->DrawStretchText( aPos, aSize.Width(), CalcCaseMap( rTxt ), nIdx, nTmp );
            }
        }
    }
    pOut->SetFont(aOldFont);
    pPrinter->SetFont( aOldPrnFont );
}


SvxFont& SvxFont::operator=( const vcl::Font& rFont )
{
    Font::operator=( rFont );
    return *this;
}

SvxFont& SvxFont::operator=( const SvxFont& rFont )
{
    Font::operator=( rFont );
    eCaseMap = rFont.eCaseMap;
    nEsc = rFont.nEsc;
    nPropr = rFont.nPropr;
    nKern = rFont.nKern;
    return *this;
}

class SvxDoGetCapitalSize : public SvxDoCapitals
{
protected:
    SvxFont*    pFont;
    Size        aTxtSize;
    short       nKern;
public:
      SvxDoGetCapitalSize( SvxFont *_pFnt, const OutputDevice *_pOut,
                           const OUString &_rTxt, const sal_Int32 _nIdx,
                           const sal_Int32 _nLen, const short _nKrn )
            : SvxDoCapitals( const_cast<OutputDevice*>(_pOut), _rTxt, _nIdx, _nLen ),
              pFont( _pFnt ),
              nKern( _nKrn )
            { }

    virtual ~SvxDoGetCapitalSize() {}

    virtual void Do( const OUString &rTxt, const sal_Int32 nIdx,
                     const sal_Int32 nLen, const bool bUpper ) override;

    const Size &GetSize() const { return aTxtSize; };
};

void SvxDoGetCapitalSize::Do( const OUString &_rTxt, const sal_Int32 _nIdx,
                              const sal_Int32 _nLen, const bool bUpper )
{
    Size aPartSize;
    if ( !bUpper )
    {
        sal_uInt8 nProp = pFont->GetPropr();
        pFont->SetProprRel( SMALL_CAPS_PERCENTAGE );
        pFont->SetPhysFont( pOut );
        aPartSize.setWidth( pOut->GetTextWidth( _rTxt, _nIdx, _nLen ) );
        aPartSize.setHeight( pOut->GetTextHeight() );
        aTxtSize.Height() = aPartSize.Height();
        pFont->SetPropr( nProp );
        pFont->SetPhysFont( pOut );
    }
    else
    {
        aPartSize.setWidth( pOut->GetTextWidth( _rTxt, _nIdx, _nLen ) );
        aPartSize.setHeight( pOut->GetTextHeight() );
    }
    aTxtSize.Width() += aPartSize.Width();
    aTxtSize.Width() += ( _nLen * long( nKern ) );
}

Size SvxFont::GetCapitalSize( const OutputDevice *pOut, const OUString &rTxt,
                             const sal_Int32 nIdx, const sal_Int32 nLen) const
{
    // Start:
    SvxDoGetCapitalSize aDo( const_cast<SvxFont *>(this), pOut, rTxt, nIdx, nLen, nKern );
    DoOnCapitals( aDo );
    Size aTxtSize( aDo.GetSize() );

    // End:
    if( !aTxtSize.Height() )
    {
        aTxtSize.setWidth( 0 );
        aTxtSize.setHeight( pOut->GetTextHeight() );
    }
    return aTxtSize;
}

class SvxDoDrawCapital : public SvxDoCapitals
{
protected:
    SvxFont *pFont;
    Point aPos;
    Point aSpacePos;
    short nKern;
public:
    SvxDoDrawCapital( SvxFont *pFnt, OutputDevice *_pOut, const OUString &_rTxt,
                      const sal_Int32 _nIdx, const sal_Int32 _nLen,
                      const Point &rPos, const short nKrn )
        : SvxDoCapitals( _pOut, _rTxt, _nIdx, _nLen ),
          pFont( pFnt ),
          aPos( rPos ),
          aSpacePos( rPos ),
          nKern( nKrn )
        { }
    virtual void DoSpace( const bool bDraw ) override;
    virtual void SetSpace() override;
    virtual void Do( const OUString &rTxt, const sal_Int32 nIdx,
                     const sal_Int32 nLen, const bool bUpper ) override;
};

void SvxDoDrawCapital::DoSpace( const bool bDraw )
{
    if ( bDraw || pFont->IsWordLineMode() )
    {
        sal_uLong nDiff = (sal_uLong)(aPos.X() - aSpacePos.X());
        if ( nDiff )
        {
            bool bWordWise = pFont->IsWordLineMode();
            bool bTrans = pFont->IsTransparent();
            pFont->SetWordLineMode( false );
            pFont->SetTransparent( true );
            pFont->SetPhysFont( pOut );
            pOut->DrawStretchText( aSpacePos, nDiff, "  ", 0, 2 );
            pFont->SetWordLineMode( bWordWise );
            pFont->SetTransparent( bTrans );
            pFont->SetPhysFont( pOut );
        }
    }
}

void SvxDoDrawCapital::SetSpace()
{
    if ( pFont->IsWordLineMode() )
        aSpacePos.X() = aPos.X();
}

void SvxDoDrawCapital::Do( const OUString &_rTxt, const sal_Int32 _nIdx,
                           const sal_Int32 _nLen, const bool bUpper)
{
    sal_uInt8 nProp = 0;
    Size aPartSize;

    // Set the desired font
    FontLineStyle eUnder = pFont->GetUnderline();
    FontStrikeout eStrike = pFont->GetStrikeout();
    pFont->SetUnderline( LINESTYLE_NONE );
    pFont->SetStrikeout( STRIKEOUT_NONE );
    if ( !bUpper )
    {
        nProp = pFont->GetPropr();
        pFont->SetProprRel( SMALL_CAPS_PERCENTAGE );
    }
    pFont->SetPhysFont( pOut );

    aPartSize.setWidth( pOut->GetTextWidth( _rTxt, _nIdx, _nLen ) );
    aPartSize.setHeight( pOut->GetTextHeight() );
    long nWidth = aPartSize.Width();
    if ( nKern )
    {
        aPos.X() += (nKern/2);
        if ( _nLen ) nWidth += (_nLen*long(nKern));
    }
    pOut->DrawStretchText(aPos,nWidth-nKern,_rTxt,_nIdx,_nLen);

    // Restore Font
    pFont->SetUnderline( eUnder );
    pFont->SetStrikeout( eStrike );
    if ( !bUpper )
        pFont->SetPropr( nProp );
    pFont->SetPhysFont( pOut );

    aPos.X() += nWidth-(nKern/2);
}

/*************************************************************************
 * SvxFont::DrawCapital() draws the uppercase letter.
 *************************************************************************/

void SvxFont::DrawCapital( OutputDevice *pOut,
               const Point &rPos, const OUString &rTxt,
               const sal_Int32 nIdx, const sal_Int32 nLen ) const
{
    SvxDoDrawCapital aDo( const_cast<SvxFont *>(this),pOut,rTxt,nIdx,nLen,rPos,nKern );
    DoOnCapitals( aDo );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
