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

#include <editeng/svxfont.hxx>

#include <vcl/glyphitemcache.hxx>
#include <vcl/metric.hxx>
#include <vcl/outdev.hxx>
#include <vcl/print.hxx>
#include <tools/debug.hxx>
#include <tools/gen.hxx>
#include <tools/poly.hxx>
#include <unotools/charclass.hxx>
#include <com/sun/star/i18n/KCharacterType.hpp>
#include <editeng/escapementitem.hxx>
#include <editeng/smallcaps.hxx>
#include <sal/log.hxx>
#include <limits>

static tools::Long GetTextArray( const OutputDevice* pOut, const OUString& rStr, KernArray* pDXAry,
                                 sal_Int32 nIndex, sal_Int32 nLen )

{
    const SalLayoutGlyphs* layoutGlyphs = SalLayoutGlyphsCache::self()->GetLayoutGlyphs(pOut, rStr, nIndex, nLen);
    return basegfx::fround<tools::Long>(
        pOut->GetTextArray(rStr, pDXAry, nIndex, nLen, true, nullptr, layoutGlyphs).nWidth);
}

SvxFont::SvxFont()
{
    nEsc = 0;
    nPropr = 100;
    eCaseMap = SvxCaseMap::NotMapped;
    SetLanguage(LANGUAGE_SYSTEM);
}

SvxFont::SvxFont( const vcl::Font &rFont )
    : Font( rFont )
{
    nEsc = 0;
    nPropr = 100;
    eCaseMap = SvxCaseMap::NotMapped;
    SetLanguage(LANGUAGE_SYSTEM);
}

SvxFont::SvxFont( const SvxFont &rFont )
    : Font( rFont )
{
    nEsc  = rFont.GetEscapement();
    nPropr = rFont.GetPropr();
    eCaseMap = rFont.GetCaseMap();
    SetLanguage(rFont.GetLanguage());
}

void SvxFont::SetNonAutoEscapement(short nNewEsc, const OutputDevice* pOutDev)
{
    nEsc = nNewEsc;
    if ( abs(nEsc) == DFLT_ESC_AUTO_SUPER )
    {
        double fAutoAscent = .8;
        double fAutoDescent = .2;
        if ( pOutDev )
        {
            const FontMetric& rFontMetric = pOutDev->GetFontMetric();
            double fFontHeight = rFontMetric.GetAscent() + rFontMetric.GetDescent();
            if ( fFontHeight )
            {
                fAutoAscent = rFontMetric.GetAscent() / fFontHeight;
                fAutoDescent = rFontMetric.GetDescent() / fFontHeight;
            }
        }

        if ( nEsc == DFLT_ESC_AUTO_SUPER )
            nEsc = fAutoAscent * (100 - nPropr);
        else //DFLT_ESC_AUTO_SUB
            nEsc = fAutoDescent * -(100 - nPropr);
    }

    if ( nEsc > MAX_ESC_POS )
        nEsc = MAX_ESC_POS;
    else if  ( nEsc < -MAX_ESC_POS )
        nEsc = -MAX_ESC_POS;
}

tools::Polygon SvxFont::DrawArrow( OutputDevice &rOut, const tools::Rectangle& rRect,
    const Size& rSize, const Color& rCol, bool bLeftOrTop, bool bVertical )
{
    tools::Polygon aPoly;
    Point aTmp;
    Point aNxt;
    if (bVertical)
    {
        tools::Long nLeft = ((rRect.Left() + rRect.Right()) / 2) - (rSize.Height() / 2);
        tools::Long nRight = ((rRect.Left() + rRect.Right()) / 2) + (rSize.Height() / 2);
        tools::Long nMid = (rRect.Left() + rRect.Right()) / 2;
        tools::Long nTop = ((rRect.Top() + rRect.Bottom()) / 2) - (rSize.Height() / 2);
        tools::Long nBottom = nTop + rSize.Height();
        if (nTop < rRect.Top())
        {
            if (bLeftOrTop)
            {
                nTop = rRect.Top();
                nBottom = rRect.Bottom();
            }
            else
            {
                nTop = rRect.Bottom();
                nBottom = rRect.Bottom() - (rSize.Height() / 2);
            }
        }
        aTmp.setX(nRight);
        aTmp.setY(nBottom);
        aNxt.setX(nMid);
        aNxt.setY(nTop);
        aPoly.Insert(0, aTmp);
        aPoly.Insert(0, aNxt);
        aTmp.setX(nLeft);
        aPoly.Insert(0, aTmp);
    }
    else
    {
        tools::Long nLeft = (rRect.Left() + rRect.Right() - rSize.Width()) / 2;
        tools::Long nRight = nLeft + rSize.Width();
        tools::Long nMid = (rRect.Top() + rRect.Bottom()) / 2;
        tools::Long nTop = nMid - rSize.Height() / 2;
        tools::Long nBottom = nTop + rSize.Height();
        if (nLeft < rRect.Left())
        {
            nLeft = rRect.Left();
            nRight = rRect.Right();
        }
        aTmp.setX(bLeftOrTop ? nLeft : nRight);
        aTmp.setY(nMid);
        aNxt.setX(bLeftOrTop ? nRight : nLeft);
        aNxt.setY(nTop);
        aPoly.Insert(0, aTmp);
        aPoly.Insert(0, aNxt);
        aNxt.setY(nBottom);
        aPoly.Insert(0, aNxt);
    }
    Color aOldLineColor = rOut.GetLineColor();
    Color aOldFillColor = rOut.GetFillColor();
    rOut.SetFillColor( rCol );
    rOut.SetLineColor( COL_BLACK );
    rOut.DrawPolygon( aPoly );
    rOut.DrawLine( aTmp, aNxt );
    rOut.SetLineColor( aOldLineColor );
    rOut.SetFillColor( aOldFillColor );
    return aPoly;
}

OUString SvxFont::CalcCaseMap(const OUString &rTxt) const
{
    if (!IsCaseMap() || rTxt.isEmpty())
        return rTxt;
    OUString aTxt(rTxt);
    // I still have to get the language
    const LanguageType eLang = LANGUAGE_DONTKNOW == GetLanguage()
                             ? LANGUAGE_SYSTEM : GetLanguage();

    CharClass aCharClass(( LanguageTag(eLang) ));

    switch( eCaseMap )
    {
        case SvxCaseMap::SmallCaps:
        case SvxCaseMap::Uppercase:
        {
            aTxt = aCharClass.uppercase( aTxt );
            break;
        }

        case SvxCaseMap::Lowercase:
        {
            aTxt = aCharClass.lowercase( aTxt );
            break;
        }
        case SvxCaseMap::Capitalize:
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
            SAL_WARN( "editeng", "SvxFont::CaseMapTxt: unknown casemap");
            break;
        }
    }
    return aTxt;
}

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

    CharClass   aCharClass(( LanguageTag(eLang) ));
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
            if ( nCharacterType & css::i18n::KCharacterType::UPPER )
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


void SvxFont::SetPhysFont(OutputDevice& rOut) const
{
    const vcl::Font& rCurrentFont = rOut.GetFont();
    if ( nPropr == 100 )
    {
        if ( !rCurrentFont.IsSameInstance( *this ) )
            rOut.SetFont( *this );
    }
    else
    {
        Font aNewFont( *this );
        Size aSize( aNewFont.GetFontSize() );
        aNewFont.SetFontSize( Size( aSize.Width() * nPropr / 100,
                                    aSize.Height() * nPropr / 100 ) );
        if ( !rCurrentFont.IsSameInstance( aNewFont ) )
            rOut.SetFont( aNewFont );
    }
}

vcl::Font SvxFont::ChgPhysFont(OutputDevice& rOut) const
{
    vcl::Font aOldFont(rOut.GetFont());
    SetPhysFont(rOut);
    return aOldFont;
}

Size SvxFont::GetPhysTxtSize( const OutputDevice *pOut, const OUString &rTxt,
                         const sal_Int32 nIdx, const sal_Int32 nLen ) const
{
    if ( !IsCaseMap() && !IsFixKerning() )
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
        sal_Int32 nWidth(0);

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

    if( IsFixKerning() && ( nLen > 1 ) )
    {
        auto nKern = GetFixKerning();
        KernArray aDXArray;
        GetTextArray(pOut, rTxt, &aDXArray, nIdx, nLen);
        tools::Long nOldValue = aDXArray[0];
        sal_Int32 nSpaceCount = 0;
        for(sal_Int32 i = 1; i < nLen; ++i)
        {
            if (aDXArray[i] != nOldValue)
            {
                nOldValue = aDXArray[i];
                ++nSpaceCount;
            }
        }
        aTxtSize.AdjustWidth( nSpaceCount * tools::Long( nKern ) );
    }

    return aTxtSize;
}

Size SvxFont::GetPhysTxtSize( const OutputDevice *pOut )
{
    if ( !IsCaseMap() && !IsFixKerning() )
        return Size( pOut->GetTextWidth( u""_ustr ), pOut->GetTextHeight() );

    Size aTxtSize;
    aTxtSize.setHeight( pOut->GetTextHeight() );
    if ( !IsCaseMap() )
        aTxtSize.setWidth( pOut->GetTextWidth( u""_ustr ) );
    else
        aTxtSize.setWidth( pOut->GetTextWidth( CalcCaseMap( u""_ustr ) ) );

    return aTxtSize;
}

Size SvxFont::QuickGetTextSize( const OutputDevice *pOut, const OUString &rTxt,
                         const sal_Int32 nIdx, const sal_Int32 nLen, KernArray* pDXArray, bool bStacked ) const
{
    if ( !IsCaseMap() && !IsFixKerning() )
    {
        SAL_INFO( "editeng.quicktextsize", "SvxFont::QuickGetTextSize before GetTextArray(): Case map: " << IsCaseMap() << " Fix kerning: " << IsFixKerning());
        Size aTxtSize( GetTextArray( pOut, rTxt, pDXArray, nIdx, nLen ),
                     pOut->GetTextHeight() );
        SAL_INFO( "editeng.quicktextsize", "SvxFont::QuickGetTextSize after GetTextArray(): Text length: " << nLen << " Text size: " << aTxtSize.Width() << "x" << aTxtSize.Height());
        return aTxtSize;
    }

    KernArray aDXArray;

    // We always need pDXArray to count the number of kern spaces
    if (!pDXArray && IsFixKerning() && nLen > 1)
    {
        pDXArray = &aDXArray;
        aDXArray.reserve(nLen);
    }

    Size aTxtSize;
    aTxtSize.setHeight( pOut->GetTextHeight() );
    SAL_INFO( "editeng.quicktextsize", "SvxFont::QuickGetTextSize before GetTextArray(): Case map: " << IsCaseMap() << " Fix kerning: " << IsFixKerning());
    if ( !IsCaseMap() )
        aTxtSize.setWidth( GetTextArray( pOut, rTxt, pDXArray, nIdx, nLen ) );
    else
    {
        if (IsCapital() && !rTxt.isEmpty())
            aTxtSize = GetCapitalSize(pOut, rTxt, pDXArray, nIdx, nLen);
        else
            aTxtSize.setWidth( GetTextArray( pOut, CalcCaseMap( rTxt ),
                               pDXArray, nIdx, nLen ) );
    }
    SAL_INFO( "editeng.quicktextsize", "SvxFont::QuickGetTextSize after GetTextArray(): Text length: " << nLen << " Text size: " << aTxtSize.Width() << "x" << aTxtSize.Height());

    if( IsFixKerning() && ( nLen > 1 ) && !bStacked)
    {
        auto nKern = GetFixKerning();
        tools::Long nOldValue = (*pDXArray)[0];
        tools::Long nSpaceSum = nKern;
        pDXArray->adjust(0, nSpaceSum);

        for ( sal_Int32 i = 1; i < nLen; i++ )
        {
            if ( (*pDXArray)[i] != nOldValue )
            {
                nOldValue = (*pDXArray)[i];
                nSpaceSum += nKern;
            }
            pDXArray->adjust(i, nSpaceSum);
        }

        // The last one is a nKern too big:
        nOldValue = (*pDXArray)[nLen - 1];
        tools::Long nNewValue = nOldValue - nKern;
        for ( sal_Int32 i = nLen - 1; i >= 0 && (*pDXArray)[i] == nOldValue; --i)
            pDXArray->set(i, nNewValue);

        aTxtSize.AdjustWidth(nSpaceSum - nKern);
    }

    return aTxtSize;
}

Size SvxFont::GetTextSize(const OutputDevice& rOut, const OUString &rTxt,
                          const sal_Int32 nIdx, const sal_Int32 nLen) const
{
    sal_Int32 nTmp = nLen;
    if ( nTmp == SAL_MAX_INT32 )   // already initialized?
        nTmp = rTxt.getLength();
    Font aOldFont( ChgPhysFont(const_cast<OutputDevice&>(rOut)));
    Size aTxtSize;
    if( IsCapital() && !rTxt.isEmpty() )
    {
        aTxtSize = GetCapitalSize(&rOut, rTxt, nullptr, nIdx, nTmp);
    }
    else aTxtSize = GetPhysTxtSize(&rOut,rTxt,nIdx,nTmp);
    const_cast<OutputDevice&>(rOut).SetFont(aOldFont);
    return aTxtSize;
}

static void DrawTextArray( OutputDevice* pOut, const Point& rStartPt, const OUString& rStr,
                           std::span<const sal_Int32> pDXAry,
                           std::span<const sal_Bool> pKashidaAry,
                           sal_Int32 nIndex, sal_Int32 nLen )
{
    const SalLayoutGlyphs* layoutGlyphs = SalLayoutGlyphsCache::self()->GetLayoutGlyphs(pOut, rStr, nIndex, nLen);
    pOut->DrawTextArray(rStartPt, rStr, pDXAry, pKashidaAry, nIndex, nLen, SalLayoutFlags::NONE, layoutGlyphs);
}

void SvxFont::QuickDrawText( OutputDevice *pOut,
    const Point &rPos, const OUString &rTxt,
    const sal_Int32 nIdx, const sal_Int32 nLen,
    std::span<const sal_Int32> pDXArray,
    std::span<const sal_Bool> pKashidaArray) const
{

    // Font has to be selected in OutputDevice...
    if ( !IsCaseMap() && !IsCapital() && !IsFixKerning() && !IsEsc() )
    {
        DrawTextArray( pOut, rPos, rTxt, pDXArray, pKashidaArray, nIdx, nLen );
        return;
    }

    Point aPos( rPos );

    if ( nEsc )
    {
        tools::Long nDiff = GetFontSize().Height();
        nDiff *= nEsc;
        nDiff /= 100;

        if ( !IsVertical() )
            aPos.AdjustY( -nDiff );
        else
            aPos.AdjustX(nDiff );
    }

    if( IsCapital() )
    {
        DrawCapital( pOut, aPos, rTxt, pDXArray, pKashidaArray, nIdx, nLen );
    }
    else
    {
        if ( IsFixKerning() && pDXArray.empty() )
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
                DrawTextArray( pOut, aPos, rTxt, pDXArray, pKashidaArray, nIdx, nLen );
            else
                DrawTextArray( pOut, aPos, CalcCaseMap( rTxt ), pDXArray, pKashidaArray, nIdx, nLen );
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
        {
            nTmpEsc = .8 * (100 - nPropr);
            assert (nTmpEsc == DFLT_ESC_SUPER && "I'm sure this formula needs to be changed, but how to confirm that???");
            nTmpEsc = DFLT_ESC_SUPER;
        }
        else if( DFLT_ESC_AUTO_SUB == nEsc )
        {
            nTmpEsc = .2 * -(100 - nPropr);
            assert (nTmpEsc == -20 && "I'm sure this formula needs to be changed, but how to confirm that???");
            nTmpEsc = -20;
        }
        else
            nTmpEsc = nEsc;
        Size aSize = GetFontSize();
        aPos.AdjustY( -(( nTmpEsc * aSize.Height() ) / 100) );
    }
    Font aOldFont( ChgPhysFont(*pOut) );
    Font aOldPrnFont( ChgPhysFont(*pPrinter) );

    if ( IsCapital() )
        DrawCapital( pOut, aPos, rTxt, {}, {}, nIdx, nTmp );
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
    return *this;
}

namespace {

class SvxDoGetCapitalSize : public SvxDoCapitals
{
protected:
    VclPtr<OutputDevice> pOut;
    SvxFont*    pFont;
    Size        aTxtSize;
    short       nKern;
    KernArray*  pDXAry;
public:
      SvxDoGetCapitalSize( SvxFont *_pFnt, const OutputDevice *_pOut,
                           const OUString &_rTxt, KernArray* _pDXAry, const sal_Int32 _nIdx,
                           const sal_Int32 _nLen, const short _nKrn )
            : SvxDoCapitals( _rTxt, _nIdx, _nLen ),
              pOut( const_cast<OutputDevice*>(_pOut) ),
              pFont( _pFnt ),
              nKern( _nKrn ),
              pDXAry( _pDXAry )
    {
        if (pDXAry)
        {
            pDXAry->clear();
            pDXAry->reserve(_nLen);
        }
    }

    virtual void Do( const OUString &rTxt, const sal_Int32 nIdx,
                     const sal_Int32 nLen, const bool bUpper ) override;

    const Size &GetSize() const { return aTxtSize; };
};

}

void SvxDoGetCapitalSize::Do( const OUString &_rTxt, const sal_Int32 _nIdx,
                              const sal_Int32 _nLen, const bool bUpper )
{
    Size aPartSize;
    sal_uInt8 nProp(0);
    if ( !bUpper )
    {
        nProp = pFont->GetPropr();
        pFont->SetProprRel( SMALL_CAPS_PERCENTAGE );
        pFont->SetPhysFont( *pOut );
    }

    if (pDXAry)
    {
        KernArray aKernArray;
        aPartSize.setWidth(basegfx::fround<tools::Long>(
            pOut->GetTextArray(_rTxt, &aKernArray, _nIdx, _nLen).nWidth));
        assert(pDXAry->get_factor() == aKernArray.get_factor());
        auto& dest = pDXAry->get_subunit_array();
        sal_Int32 nStart = dest.empty() ? 0 : dest.back();
        size_t nSrcLen = aKernArray.size();
        dest.reserve(dest.size() + nSrcLen);
        const auto& src = aKernArray.get_subunit_array();
        for (size_t i = 0; i < nSrcLen; ++i)
            dest.push_back(src[i] + nStart);
    }
    else
    {
        aPartSize.setWidth( pOut->GetTextWidth( _rTxt, _nIdx, _nLen ) );
    }

    aPartSize.setHeight( pOut->GetTextHeight() );

    if ( !bUpper )
    {
        aTxtSize.setHeight( aPartSize.Height() );
        pFont->SetPropr( nProp );
        pFont->SetPhysFont( *pOut );
    }

    aTxtSize.AdjustWidth(aPartSize.Width() );
    aTxtSize.AdjustWidth( _nLen * tools::Long( nKern ) );
}

Size SvxFont::GetCapitalSize( const OutputDevice *pOut, const OUString &rTxt, KernArray* pDXAry,
                             const sal_Int32 nIdx, const sal_Int32 nLen) const
{
    // Start:
    SvxDoGetCapitalSize aDo( const_cast<SvxFont *>(this), pOut, rTxt, pDXAry, nIdx, nLen, GetFixKerning() );
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

namespace {

class SvxDoDrawCapital : public SvxDoCapitals
{
protected:
    VclPtr<OutputDevice> pOut;
    SvxFont *pFont;
    Point aPos;
    Point aSpacePos;
    short nKern;
    std::span<const sal_Int32> pDXArray;
    std::span<const sal_Bool> pKashidaArray;
public:
    SvxDoDrawCapital( SvxFont *pFnt, OutputDevice *_pOut, const OUString &_rTxt,
                      std::span<const sal_Int32> _pDXArray,
                      std::span<const sal_Bool> _pKashidaArray,
                      const sal_Int32 _nIdx, const sal_Int32 _nLen,
                      const Point &rPos, const short nKrn )
        : SvxDoCapitals( _rTxt, _nIdx, _nLen ),
          pOut( _pOut ),
          pFont( pFnt ),
          aPos( rPos ),
          aSpacePos( rPos ),
          nKern( nKrn ),
          pDXArray(_pDXArray),
          pKashidaArray(_pKashidaArray)
        { }
    virtual void DoSpace( const bool bDraw ) override;
    virtual void SetSpace() override;
    virtual void Do( const OUString &rTxt, const sal_Int32 nIdx,
                     const sal_Int32 nLen, const bool bUpper ) override;
};

}

void SvxDoDrawCapital::DoSpace( const bool bDraw )
{
    if ( !(bDraw || pFont->IsWordLineMode()) )
        return;

    sal_Int32 nDiff = static_cast<sal_Int32>(aPos.X() - aSpacePos.X());
    if ( nDiff )
    {
        bool bWordWise = pFont->IsWordLineMode();
        bool bTrans = pFont->IsTransparent();
        pFont->SetWordLineMode( false );
        pFont->SetTransparent( true );
        pFont->SetPhysFont(*pOut);
        pOut->DrawStretchText( aSpacePos, nDiff, u"  "_ustr, 0, 2 );
        pFont->SetWordLineMode( bWordWise );
        pFont->SetTransparent( bTrans );
        pFont->SetPhysFont(*pOut);
    }
}

void SvxDoDrawCapital::SetSpace()
{
    if ( pFont->IsWordLineMode() )
        aSpacePos.setX( aPos.X() );
}

void SvxDoDrawCapital::Do( const OUString &_rTxt, const sal_Int32 nSpanIdx,
                           const sal_Int32 nSpanLen, const bool bUpper)
{
    sal_uInt8 nProp = 0;

    // Set the desired font
    FontLineStyle eUnder = pFont->GetUnderline();
    FontLineStyle eOver = pFont->GetOverline();
    FontStrikeout eStrike = pFont->GetStrikeout();
    pFont->SetUnderline( LINESTYLE_NONE );
    pFont->SetOverline( LINESTYLE_NONE );
    pFont->SetStrikeout( STRIKEOUT_NONE );
    if ( !bUpper )
    {
        nProp = pFont->GetPropr();
        pFont->SetProprRel( SMALL_CAPS_PERCENTAGE );
    }
    pFont->SetPhysFont(*pOut);

    if (pDXArray.empty())
    {
        auto nWidth = pOut->GetTextWidth(_rTxt, nSpanIdx, nSpanLen);
        if (nKern)
        {
            aPos.AdjustX(nKern/2);
            if (nSpanLen)
                nWidth += (nSpanLen * nKern);
        }
        pOut->DrawStretchText(aPos, nWidth-nKern, _rTxt, nSpanIdx, nSpanLen);
        // in this case we move aPos along to be the start of each subspan
        aPos.AdjustX(nWidth-(nKern/2) );
    }
    else
    {
        const sal_Int32 nStartOffset = nSpanIdx - nIdx;
        sal_Int32 nStartX = nStartOffset ? pDXArray[nStartOffset - 1] : 0;

        Point aStartPos(aPos.X() + nStartX, aPos.Y());

        std::vector<sal_Int32> aDXArray;
        aDXArray.reserve(nSpanLen);
        for (sal_Int32 i = 0; i < nSpanLen; ++i)
            aDXArray.push_back(pDXArray[nStartOffset + i] - nStartX);

        auto aKashidaArray = !pKashidaArray.empty() ?
            std::span<const sal_Bool>(pKashidaArray.data() + nStartOffset, nSpanLen) :
            std::span<const sal_Bool>();

        DrawTextArray(pOut, aStartPos, _rTxt, aDXArray, aKashidaArray, nSpanIdx, nSpanLen);
        // in this case we leave aPos at the start and use the DXArray to find the start
        // of each subspan
    }

    // Restore Font
    pFont->SetUnderline( eUnder );
    pFont->SetOverline( eOver );
    pFont->SetStrikeout( eStrike );
    if ( !bUpper )
        pFont->SetPropr( nProp );
    pFont->SetPhysFont(*pOut);
}

/*************************************************************************
 * SvxFont::DrawCapital() draws the uppercase letter.
 *************************************************************************/

void SvxFont::DrawCapital( OutputDevice *pOut,
               const Point &rPos, const OUString &rTxt,
               std::span<const sal_Int32> pDXArray,
               std::span<const sal_Bool> pKashidaArray,
               const sal_Int32 nIdx, const sal_Int32 nLen ) const
{
    SvxDoDrawCapital aDo(const_cast<SvxFont *>(this), pOut,
                         rTxt, pDXArray, pKashidaArray,
                         nIdx, nLen, rPos, GetFixKerning());
    DoOnCapitals( aDo );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
