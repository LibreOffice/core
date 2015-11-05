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

#include <hintids.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/svxfont.hxx>

#include <vcl/outdev.hxx>
#include <com/sun/star/i18n/CharType.hpp>
#include <com/sun/star/i18n/WordType.hpp>

#include <vcl/print.hxx>
#include <fntcache.hxx>
#include <swfont.hxx>
#include <breakit.hxx>
#include <txtfrm.hxx>
#include <scriptinfo.hxx>
#include <fntcap.hxx>

using namespace css::i18n;

// The information encapsulated in SwCapitalInfo is required
// by the ::Do functions. They contain the information about
// the original string, whereas rDo.GetInf() contains information
// about the display string.
class SwCapitalInfo
{
public:
    explicit SwCapitalInfo( const OUString& rOrigText ) :
        rString( rOrigText ), nIdx( 0 ), nLen( 0 ) {};
    const OUString& rString;
    sal_Int32 nIdx;
    sal_Int32 nLen;
};

// rFnt: required for CalcCaseMap
// rOrigString: The original string
// nOfst: Position of the substring in rOrigString
// nLen: Length if the substring in rOrigString
// nIdx: Referes to a position in the display string and should be mapped
//       to a position in rOrigString
sal_Int32 sw_CalcCaseMap( const SwFont& rFnt,
                            const OUString& rOrigString,
                            sal_Int32 nOfst,
                            sal_Int32 nLen,
                            sal_Int32 nIdx )
{
    int j = 0;
    const sal_Int32 nEnd = nOfst + nLen;
    OSL_ENSURE( nEnd <= rOrigString.getLength(), "sw_CalcCaseMap: Wrong parameters" );

    // special case for title case:
    const bool bTitle = SVX_CASEMAP_TITEL == rFnt.GetCaseMap() &&
                        g_pBreakIt->GetBreakIter().is();
    for ( sal_Int32 i = nOfst; i < nEnd; ++i )
    {
        OUString aTmp(rOrigString.copy(i, 1));

        if ( !bTitle ||
             g_pBreakIt->GetBreakIter()->isBeginWord(
                 rOrigString, i,
                 g_pBreakIt->GetLocale( rFnt.GetLanguage() ),
                 WordType::ANYWORD_IGNOREWHITESPACES ) )
            aTmp = rFnt.GetActualFont().CalcCaseMap( aTmp );

        j += aTmp.getLength();

        if ( j > nIdx )
            return i;
    }

    return nOfst + nLen;
}

class SwDoCapitals
{
protected:
    SwDrawTextInfo &rInf;
    SwCapitalInfo* pCapInf; // referes to additional information
                           // required by the ::Do function
    explicit SwDoCapitals ( SwDrawTextInfo &rInfo ) : rInf( rInfo ), pCapInf( 0 ) { }
    ~SwDoCapitals() {}
public:
    virtual void Init( SwFntObj *pUpperFont, SwFntObj *pLowerFont ) = 0;
    virtual void Do() = 0;
    inline OutputDevice& GetOut() { return rInf.GetOut(); }
    inline SwDrawTextInfo& GetInf() { return rInf; }
    inline SwCapitalInfo* GetCapInf() const { return pCapInf; }
    inline void SetCapInf( SwCapitalInfo& rNew ) { pCapInf = &rNew; }
};

class SwDoGetCapitalSize : public SwDoCapitals
{
protected:
    Size aTextSize;
public:
    explicit SwDoGetCapitalSize( SwDrawTextInfo &rInfo ) : SwDoCapitals ( rInfo ) { }
    virtual ~SwDoGetCapitalSize() {}
    virtual void Init( SwFntObj *pUpperFont, SwFntObj *pLowerFont ) override;
    virtual void Do() override;
    const Size &GetSize() const { return aTextSize; }
};

void SwDoGetCapitalSize::Init( SwFntObj *, SwFntObj * )
{
    aTextSize.Height() = 0;
    aTextSize.Width() = 0;
}

void SwDoGetCapitalSize::Do()
{
    aTextSize.Width() += rInf.GetSize().Width();
    if( rInf.GetUpper() )
        aTextSize.Height() = rInf.GetSize().Height();
}

Size SwSubFont::GetCapitalSize( SwDrawTextInfo& rInf )
{
    // Start:
    const long nOldKern = rInf.GetKern();
    rInf.SetKern( CheckKerning() );
    Point aPos;
    rInf.SetPos( aPos );
    rInf.SetSpace( 0 );
    rInf.SetDrawSpace( false );
    SwDoGetCapitalSize aDo( rInf );
    DoOnCapitals( aDo );
    Size aTextSize( aDo.GetSize() );

    // End:
    if( !aTextSize.Height() )
    {
        SV_STAT( nGetTextSize );
        aTextSize.Height() = short ( rInf.GetpOut()->GetTextHeight() );
    }
    rInf.SetKern( nOldKern );
    return aTextSize;
}

class SwDoGetCapitalBreak : public SwDoCapitals
{
protected:
    long nTextWidth;
    sal_Int32 m_nBreak;
public:
    SwDoGetCapitalBreak( SwDrawTextInfo &rInfo, long const nWidth)
        :   SwDoCapitals ( rInfo )
        ,   nTextWidth( nWidth )
        ,   m_nBreak( -1 )
        { }
    virtual ~SwDoGetCapitalBreak() {}
    virtual void Init( SwFntObj *pUpperFont, SwFntObj *pLowerFont ) override;
    virtual void Do() override;
    sal_Int32 getBreak() const { return m_nBreak; }
};

void SwDoGetCapitalBreak::Init( SwFntObj *, SwFntObj * )
{
}

void SwDoGetCapitalBreak::Do()
{
    if ( nTextWidth )
    {
        if ( rInf.GetSize().Width() < nTextWidth )
            nTextWidth -= rInf.GetSize().Width();
        else
        {
            sal_Int32 nEnd = rInf.GetEnd();
            m_nBreak = GetOut().GetTextBreak( rInf.GetText(), nTextWidth,
                               rInf.GetIdx(), rInf.GetLen(), rInf.GetKern() );

            if (m_nBreak > nEnd || m_nBreak < 0)
                m_nBreak = nEnd;

            // m_nBreak may be relative to the display string. It has to be
            // calculated relative to the original string:
            if ( GetCapInf()  )
            {
                if ( GetCapInf()->nLen != rInf.GetLen() )
                    m_nBreak = sw_CalcCaseMap( *rInf.GetFont(),
                                              GetCapInf()->rString,
                                              GetCapInf()->nIdx,
                                              GetCapInf()->nLen, m_nBreak );
                else
                    m_nBreak = m_nBreak + GetCapInf()->nIdx;
            }

            nTextWidth = 0;
        }
    }
}

sal_Int32 SwFont::GetCapitalBreak( SwViewShell const * pSh, const OutputDevice* pOut,
    const SwScriptInfo* pScript, const OUString& rText, long const nTextWidth,
    const sal_Int32 nIdx, const sal_Int32 nLen )
{
    // Start:
    Point aPos( 0, 0 );
    SwDrawTextInfo aInfo(pSh, *const_cast<OutputDevice*>(pOut), pScript, rText, nIdx, nLen,
        0, false);
    aInfo.SetPos( aPos );
    aInfo.SetSpace( 0 );
    aInfo.SetWrong( NULL );
    aInfo.SetGrammarCheck( NULL );
    aInfo.SetSmartTags( NULL );
    aInfo.SetDrawSpace( false );
    aInfo.SetKern( CheckKerning() );
    aInfo.SetKanaComp( pScript ? 0 : 100 );
    aInfo.SetFont( this );

    SwDoGetCapitalBreak aDo(aInfo, nTextWidth);
    DoOnCapitals( aDo );
    return aDo.getBreak();
}

class SwDoDrawCapital : public SwDoCapitals
{
protected:
    SwFntObj *pUpperFnt;
    SwFntObj *pLowerFnt;
public:
    explicit SwDoDrawCapital( SwDrawTextInfo &rInfo ) :
        SwDoCapitals( rInfo ), pUpperFnt(0), pLowerFnt(0)
        { }
    virtual ~SwDoDrawCapital() {}
    virtual void Init( SwFntObj *pUpperFont, SwFntObj *pLowerFont ) override;
    virtual void Do() override;
    void DrawSpace( Point &rPos );
};

void SwDoDrawCapital::Init( SwFntObj *pUpperFont, SwFntObj *pLowerFont )
{
    pUpperFnt = pUpperFont;
    pLowerFnt = pLowerFont;
}

void SwDoDrawCapital::Do()
{
    SV_STAT( nDrawText );
    const sal_uInt16 nOrgWidth = rInf.GetWidth();
    rInf.SetWidth( sal_uInt16(rInf.GetSize().Width()) );
    if ( rInf.GetUpper() )
        pUpperFnt->DrawText( rInf );
    else
    {
        bool bOldBullet = rInf.GetBullet();
        rInf.SetBullet( false );
        pLowerFnt->DrawText( rInf );
        rInf.SetBullet( bOldBullet );
    }

    OSL_ENSURE( pUpperFnt, "No upper font, dying soon!");
    rInf.Shift( pUpperFnt->GetFont().GetOrientation() );
    rInf.SetWidth( nOrgWidth );
}

void SwDoDrawCapital::DrawSpace( Point &rPos )
{
    long nDiff = rInf.GetPos().X() - rPos.X();

    Point aPos( rPos );
    const bool bSwitchL2R = rInf.GetFrm()->IsRightToLeft() &&
                          ! rInf.IsIgnoreFrmRTL();

    if ( bSwitchL2R )
       rInf.GetFrm()->SwitchLTRtoRTL( aPos );

    const ComplexTextLayoutMode nMode = rInf.GetpOut()->GetLayoutMode();
    const bool bBidiPor = ( bSwitchL2R !=
                            ( TEXT_LAYOUT_DEFAULT != ( TEXT_LAYOUT_BIDI_RTL & nMode ) ) );

    if ( bBidiPor )
        nDiff = -nDiff;

    if ( rInf.GetFrm()->IsVertical() )
        rInf.GetFrm()->SwitchHorizontalToVertical( aPos );

    if ( nDiff )
    {
        rInf.ApplyAutoColor();
        GetOut().DrawStretchText( aPos, nDiff,
            OUString("  "), 0, 2 );
    }
    rPos.X() = rInf.GetPos().X() + rInf.GetWidth();
}

void SwSubFont::DrawCapital( SwDrawTextInfo &rInf )
{
    // Es wird vorausgesetzt, dass rPos bereits kalkuliert ist!
    // hochgezogen in SwFont: const Point aPos( CalcPos(rPos) );
    rInf.SetDrawSpace( GetUnderline() != UNDERLINE_NONE ||
                       GetOverline()  != UNDERLINE_NONE ||
                       GetStrikeout() != STRIKEOUT_NONE );
    SwDoDrawCapital aDo( rInf );
    DoOnCapitals( aDo );
}

class SwDoCapitalCrsrOfst : public SwDoCapitals
{
protected:
    SwFntObj *pUpperFnt;
    SwFntObj *pLowerFnt;
    sal_Int32 nCrsr;
    sal_uInt16 nOfst;
public:
    SwDoCapitalCrsrOfst( SwDrawTextInfo &rInfo, const sal_uInt16 nOfs ) :
        SwDoCapitals( rInfo ), pUpperFnt(0), pLowerFnt(0), nCrsr( 0 ), nOfst( nOfs )
        { }
    virtual ~SwDoCapitalCrsrOfst() {}
    virtual void Init( SwFntObj *pUpperFont, SwFntObj *pLowerFont ) override;
    virtual void Do() override;

    inline sal_Int32 GetCrsr(){ return nCrsr; }
};

void SwDoCapitalCrsrOfst::Init( SwFntObj *pUpperFont, SwFntObj *pLowerFont )
{
    pUpperFnt = pUpperFont;
    pLowerFnt = pLowerFont;
}

void SwDoCapitalCrsrOfst::Do()
{
    if ( nOfst )
    {
        if ( static_cast<long>(nOfst) > rInf.GetSize().Width() )
        {
            nOfst -= rInf.GetSize().Width();
            nCrsr = nCrsr + rInf.GetLen();
        }
        else
        {
            SwDrawTextInfo aDrawInf( rInf.GetShell(), *rInf.GetpOut(),
                                     rInf.GetScriptInfo(),
                                     rInf.GetText(),
                                     rInf.GetIdx(),
                                     rInf.GetLen(), 0, false );
            aDrawInf.SetOfst( nOfst );
            aDrawInf.SetKern( rInf.GetKern() );
            aDrawInf.SetKanaComp( rInf.GetKanaComp() );
            aDrawInf.SetFrm( rInf.GetFrm() );
            aDrawInf.SetFont( rInf.GetFont() );

            if ( rInf.GetUpper() )
            {
                aDrawInf.SetSpace( 0 );
                nCrsr = nCrsr + pUpperFnt->GetCrsrOfst( aDrawInf );
            }
            else
            {
                aDrawInf.SetSpace( rInf.GetSpace() );
                nCrsr = nCrsr + pLowerFnt->GetCrsrOfst( aDrawInf );
            }
            nOfst = 0;
        }
    }
}

sal_Int32 SwSubFont::GetCapitalCrsrOfst( SwDrawTextInfo& rInf )
{
    const long nOldKern = rInf.GetKern();
    rInf.SetKern( CheckKerning() );
    SwDoCapitalCrsrOfst aDo( rInf, rInf.GetOfst() );
    Point aPos;
    rInf.SetPos( aPos );
    rInf.SetDrawSpace( false );
    DoOnCapitals( aDo );
    rInf.SetKern( nOldKern );
    return aDo.GetCrsr();
}

class SwDoDrawStretchCapital : public SwDoDrawCapital
{
    const sal_Int32 nStrLen;
    const sal_uInt16 nCapWidth;
    const sal_uInt16 nOrgWidth;
public:
    virtual void Do() override;

    SwDoDrawStretchCapital( SwDrawTextInfo &rInfo, const sal_uInt16 nCapitalWidth )
            : SwDoDrawCapital( rInfo ),
              nStrLen( rInfo.GetLen() ),
              nCapWidth( nCapitalWidth ),
              nOrgWidth( rInfo.GetWidth() )
        { }

    virtual ~SwDoDrawStretchCapital() {}
};

void SwDoDrawStretchCapital::Do()
{
    SV_STAT( nDrawStretchText );
    long nPartWidth = rInf.GetSize().Width();

    if( rInf.GetLen() )
    {
        // 4023: Kapitaelchen und Kerning.
        long nDiff = long(nOrgWidth) - long(nCapWidth);
        if( nDiff )
        {
            nDiff *= rInf.GetLen();
            nDiff /= nStrLen;
            nDiff += nPartWidth;
            if( 0 < nDiff )
                nPartWidth = nDiff;
        }

        rInf.ApplyAutoColor();

        Point aPos( rInf.GetPos() );
        const bool bSwitchL2R = rInf.GetFrm()->IsRightToLeft() &&
                              ! rInf.IsIgnoreFrmRTL();

        if ( bSwitchL2R )
            rInf.GetFrm()->SwitchLTRtoRTL( aPos );

        if ( rInf.GetFrm()->IsVertical() )
            rInf.GetFrm()->SwitchHorizontalToVertical( aPos );

        // Optimierung:
        if( 1 >= rInf.GetLen() )
            GetOut().DrawText( aPos, rInf.GetText(), rInf.GetIdx(),
                rInf.GetLen() );
        else
            GetOut().DrawStretchText( aPos, nPartWidth,
                                rInf.GetText(), rInf.GetIdx(), rInf.GetLen() );
    }
    const_cast<Point&>(rInf.GetPos()).X() += nPartWidth;
}

void SwSubFont::DrawStretchCapital( SwDrawTextInfo &rInf )
{
    // Es wird vorausgesetzt, dass rPos bereits kalkuliert ist!
    // hochgezogen in SwFont: const Point aPos( CalcPos(rPos) );

    if( rInf.GetLen() == COMPLETE_STRING )
        rInf.SetLen( rInf.GetText().getLength() );

    const Point aOldPos = rInf.GetPos();
    const sal_uInt16 nCapWidth = (sal_uInt16)( GetCapitalSize( rInf ).Width() );
    rInf.SetPos(aOldPos);

    rInf.SetDrawSpace( GetUnderline() != UNDERLINE_NONE ||
                       GetOverline()  != UNDERLINE_NONE ||
                       GetStrikeout() != STRIKEOUT_NONE );
    SwDoDrawStretchCapital aDo( rInf, nCapWidth );
    DoOnCapitals( aDo );
}

void SwSubFont::DoOnCapitals( SwDoCapitals &rDo )
{
    OSL_ENSURE( pLastFont, "SwFont::DoOnCapitals: No LastFont?!" );

    long nKana = 0;
    const OUString aText( CalcCaseMap( rDo.GetInf().GetText() ) );
    sal_Int32 nMaxPos = std::min( rDo.GetInf().GetText().getLength() - rDo.GetInf().GetIdx(),
                             rDo.GetInf().GetLen() );
    rDo.GetInf().SetLen( nMaxPos );

    const OUString oldText = rDo.GetInf().GetText();
    rDo.GetInf().SetText( aText );
    sal_Int32 nPos = rDo.GetInf().GetIdx();
    sal_Int32 nOldPos = nPos;
    nMaxPos = nMaxPos + nPos;

    // Look if the length of the original text and the ToUpper-converted
    // text is different. If yes, do special handling.
    OUString aNewText;
    SwCapitalInfo aCapInf(oldText);
    bool bCaseMapLengthDiffers(aText.getLength() != oldText.getLength());
    if ( bCaseMapLengthDiffers )
        rDo.SetCapInf( aCapInf );

    SwFntObj *pOldLast = pLastFont;
    SwFntAccess *pBigFontAccess = NULL;
    SwFntObj *pBigFont;
    SwFntAccess *pSpaceFontAccess = NULL;
    SwFntObj *pSpaceFont = NULL;

    const void *pMagic2 = NULL;
    sal_uInt16 nIndex2 = 0;
    SwSubFont aFont( *this );
    Point aStartPos( rDo.GetInf().GetPos() );

    const bool bTextLines = aFont.GetUnderline() != UNDERLINE_NONE
                         || aFont.GetOverline()  != UNDERLINE_NONE
                         || aFont.GetStrikeout() != STRIKEOUT_NONE;
    const bool bWordWise = bTextLines && aFont.IsWordLineMode() &&
                           rDo.GetInf().GetDrawSpace();
    const long nTmpKern = rDo.GetInf().GetKern();

    if ( bTextLines )
    {
        if ( bWordWise )
        {
            aFont.SetWordLineMode( false );
            pSpaceFontAccess = new SwFntAccess( pMagic2, nIndex2, &aFont,
                                                rDo.GetInf().GetShell() );
            pSpaceFont = pSpaceFontAccess->Get();
        }
        else
            pSpaceFont = pLastFont;

        // Wir basteln uns einen Font fuer die Grossbuchstaben:
        aFont.SetUnderline( UNDERLINE_NONE );
        aFont.SetOverline( UNDERLINE_NONE );
        aFont.SetStrikeout( STRIKEOUT_NONE );
        pMagic2 = NULL;
        nIndex2 = 0;
        pBigFontAccess = new SwFntAccess( pMagic2, nIndex2, &aFont,
                                          rDo.GetInf().GetShell() );
        pBigFont = pBigFontAccess->Get();
    }
    else
        pBigFont = pLastFont;

    // Older LO versions had 66 as the small caps percentage size, later changed to 80,
    // therefore a backwards compatibility option is kept (otherwise layout is changed).
    // NOTE: There are more uses of SMALL_CAPS_PERCENTAGE in editeng, but it seems they
    // do not matter for Writer (and if they did it'd be pretty ugly to propagate
    // the option there).
    int smallCapsPercentage = m_bSmallCapsPercentage66 ? 66 : SMALL_CAPS_PERCENTAGE;
    aFont.SetProportion( (aFont.GetPropr() * smallCapsPercentage ) / 100 );
    pMagic2 = NULL;
    nIndex2 = 0;
    SwFntAccess *pSmallFontAccess = new SwFntAccess( pMagic2, nIndex2, &aFont,
                                                     rDo.GetInf().GetShell() );
    SwFntObj *pSmallFont = pSmallFontAccess->Get();

    rDo.Init( pBigFont, pSmallFont );
    OutputDevice* pOutSize = pSmallFont->GetPrt();
    if( !pOutSize )
        pOutSize = &rDo.GetOut();
    OutputDevice* pOldOut = &rDo.GetOut();

    const LanguageType eLng = LANGUAGE_DONTKNOW == GetLanguage()
                            ? LANGUAGE_SYSTEM : GetLanguage();

    if( nPos < nMaxPos )
    {
        nPos = g_pBreakIt->GetBreakIter()->endOfCharBlock(
                        oldText, nPos,
            g_pBreakIt->GetLocale( eLng ), CharType::LOWERCASE_LETTER);
        if (nPos < 0)
            nPos = nOldPos;
        else if( nPos > nMaxPos )
            nPos = nMaxPos;
    }

    while( nOldPos < nMaxPos )
    {

        //  The lower ones...
        if( nOldPos != nPos )
        {
            SV_STAT( nGetTextSize );
            pLastFont = pSmallFont;
            pLastFont->SetDevFont( rDo.GetInf().GetShell(), rDo.GetOut() );

            // #107816#, #i14820#
            if( bCaseMapLengthDiffers )
            {
                // Build an own 'changed' string for the given part of the
                // source string and use it. That new string may differ in length
                // from the source string.
                const OUString aSnippet(oldText.copy(nOldPos, nPos - nOldPos));
                aNewText = CalcCaseMap( aSnippet );
                aCapInf.nIdx = nOldPos;
                aCapInf.nLen = nPos - nOldPos;
                rDo.GetInf().SetIdx( 0 );
                rDo.GetInf().SetLen( aNewText.getLength() );
                rDo.GetInf().SetText( aNewText );
            }
            else
            {
                rDo.GetInf().SetIdx( nOldPos );
                rDo.GetInf().SetLen( nPos - nOldPos );
            }

            rDo.GetInf().SetUpper( false );
            rDo.GetInf().SetOut( *pOutSize );
            Size aPartSize = pSmallFont->GetTextSize( rDo.GetInf() );
            nKana += rDo.GetInf().GetKanaDiff();
            rDo.GetInf().SetOut( *pOldOut );
            if( nTmpKern && nPos < nMaxPos )
                aPartSize.Width() += nTmpKern;
            rDo.GetInf().SetSize( aPartSize );
            rDo.Do();
            nOldPos = nPos;
        }
        nPos = g_pBreakIt->GetBreakIter()->nextCharBlock(
                            oldText, nPos,
               g_pBreakIt->GetLocale( eLng ), CharType::LOWERCASE_LETTER);
        if (nPos < 0 || nPos > nMaxPos)
            nPos = nMaxPos;
        OSL_ENSURE( nPos, "nextCharBlock not implemented?" );
#if OSL_DEBUG_LEVEL > 1
        if( !nPos )
            nPos = nMaxPos;
#endif
        // The upper ones...
        if( nOldPos != nPos )
        {
            const long nSpaceAdd = rDo.GetInf().GetSpace() / SPACING_PRECISION_FACTOR;

            do
            {
                rDo.GetInf().SetUpper( true );
                pLastFont = pBigFont;
                pLastFont->SetDevFont( rDo.GetInf().GetShell(), rDo.GetOut() );
                sal_Int32 nTmp;
                if( bWordWise )
                {
                    nTmp = nOldPos;
                    while (nTmp < nPos && CH_BLANK == oldText[nTmp])
                        ++nTmp;
                    if( nOldPos < nTmp )
                    {
                        pLastFont = pSpaceFont;
                        pLastFont->SetDevFont( rDo.GetInf().GetShell(),
                                               rDo.GetOut() );
                        static_cast<SwDoDrawCapital&>(rDo).DrawSpace( aStartPos );
                        pLastFont = pBigFont;
                        pLastFont->SetDevFont( rDo.GetInf().GetShell(),
                                               rDo.GetOut() );

                        // #107816#, #i14820#
                        if( bCaseMapLengthDiffers )
                        {
                            // Build an own 'changed' string for the given part of the
                            // source string and use it. That new string may differ in length
                            // from the source string.
                            const OUString aSnippet(oldText.copy(nOldPos, nTmp - nOldPos));
                            aNewText = CalcCaseMap( aSnippet );
                            aCapInf.nIdx = nOldPos;
                            aCapInf.nLen = nTmp - nOldPos;
                            rDo.GetInf().SetIdx( 0 );
                            rDo.GetInf().SetLen( aNewText.getLength() );
                            rDo.GetInf().SetText( aNewText );
                        }
                        else
                        {
                            rDo.GetInf().SetIdx( nOldPos );
                            rDo.GetInf().SetLen( nTmp - nOldPos );
                        }

                        rDo.GetInf().SetOut( *pOutSize );
                        Size aPartSize = pBigFont->GetTextSize( rDo.GetInf() );
                        nKana += rDo.GetInf().GetKanaDiff();
                        rDo.GetInf().SetOut( *pOldOut );
                        if( nSpaceAdd )
                            aPartSize.Width() += nSpaceAdd * ( nTmp - nOldPos );
                        if( nTmpKern && nPos < nMaxPos )
                            aPartSize.Width() += nTmpKern;
                        rDo.GetInf().SetSize( aPartSize );
                        rDo.Do();
                        aStartPos = rDo.GetInf().GetPos();
                        nOldPos = nTmp;
                    }

                    while (nTmp < nPos && CH_BLANK != oldText[nTmp])
                        ++nTmp;
                }
                else
                    nTmp = nPos;
                if( nTmp > nOldPos )
                {
                      // #107816#, #i14820#
                    if( bCaseMapLengthDiffers )
                    {
                        // Build an own 'changed' string for the given part of the
                        // source string and use it. That new string may differ in length
                        // from the source string.
                        const OUString aSnippet(oldText.copy(nOldPos, nTmp - nOldPos));
                        aNewText = CalcCaseMap( aSnippet );
                        aCapInf.nIdx = nOldPos;
                        aCapInf.nLen = nTmp - nOldPos;
                        rDo.GetInf().SetIdx( 0 );
                        rDo.GetInf().SetLen( aNewText.getLength() );
                        rDo.GetInf().SetText( aNewText );
                    }
                    else
                    {
                        rDo.GetInf().SetIdx( nOldPos );
                        rDo.GetInf().SetLen( nTmp - nOldPos );
                    }

                    rDo.GetInf().SetOut( *pOutSize );
                    Size aPartSize = pBigFont->GetTextSize( rDo.GetInf() );
                    nKana += rDo.GetInf().GetKanaDiff();
                    rDo.GetInf().SetOut( *pOldOut );
                    if( !bWordWise && rDo.GetInf().GetSpace() )
                    {
                        for( sal_Int32 nI = nOldPos; nI < nPos; ++nI )
                        {
                            if (CH_BLANK == oldText[nI])
                                aPartSize.Width() += nSpaceAdd;
                        }
                    }
                    if( nTmpKern && nPos < nMaxPos )
                        aPartSize.Width() += nTmpKern;
                    rDo.GetInf().SetSize( aPartSize );
                    rDo.Do();
                    nOldPos = nTmp;
                }
            } while( nOldPos != nPos );
        }
        nPos = g_pBreakIt->GetBreakIter()->endOfCharBlock(
                            oldText, nPos,
               g_pBreakIt->GetLocale( eLng ), CharType::LOWERCASE_LETTER);
        if (nPos < 0 || nPos > nMaxPos)
            nPos = nMaxPos;
        OSL_ENSURE( nPos, "endOfCharBlock not implemented?" );
#if OSL_DEBUG_LEVEL > 1
        if( !nPos )
            nPos = nMaxPos;
#endif
    }

    // Aufraeumen:
    if( pBigFont != pOldLast )
        delete pBigFontAccess;

    if( bTextLines )
    {
        if( rDo.GetInf().GetDrawSpace() )
        {
            pLastFont = pSpaceFont;
            pLastFont->SetDevFont( rDo.GetInf().GetShell(), rDo.GetOut() );
            static_cast<SwDoDrawCapital&>( rDo ).DrawSpace( aStartPos );
        }
        if ( bWordWise )
            delete pSpaceFontAccess;
    }
    pLastFont = pOldLast;
    pLastFont->SetDevFont( rDo.GetInf().GetShell(), rDo.GetOut() );

    delete pSmallFontAccess;
    rDo.GetInf().SetText(oldText);
    rDo.GetInf().SetKanaDiff( nKana );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
