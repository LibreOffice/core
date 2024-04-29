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

#include <vcl/outdev.hxx>
#include <com/sun/star/i18n/CharType.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>

#include <fntcache.hxx>
#include <swfont.hxx>
#include <breakit.hxx>
#include <txtfrm.hxx>
#include <scriptinfo.hxx>
#include <fntcap.hxx>

using namespace ::com::sun::star::i18n;

namespace {

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
    TextFrameIndex nIdx;
    TextFrameIndex nLen;
};

}

// rFnt: required for CalcCaseMap
// rOrigString: The original string
// nOfst: Position of the substring in rOrigString
// nLen: Length if the substring in rOrigString
// nIdx: Refers to a position in the display string and should be mapped
//       to a position in rOrigString
TextFrameIndex sw_CalcCaseMap(const SwFont& rFnt,
                            const OUString& rOrigString,
                            TextFrameIndex const nOfst,
                            TextFrameIndex const nLen,
                            TextFrameIndex const nIdx)
{
    int j = 0;
    const TextFrameIndex nEnd = nOfst + nLen;
    OSL_ENSURE( sal_Int32(nEnd) <= rOrigString.getLength(), "sw_CalcCaseMap: Wrong parameters" );

    // special case for title case:
    const bool bTitle = SvxCaseMap::Capitalize == rFnt.GetCaseMap();
    for (TextFrameIndex i = nOfst; i < nEnd; ++i)
    {
        OUString aTmp(rOrigString.copy(sal_Int32(i), 1));

        if ( !bTitle ||
             g_pBreakIt->GetBreakIter()->isBeginWord(
                 rOrigString, sal_Int32(i),
                 g_pBreakIt->GetLocale( rFnt.GetLanguage() ),
                 WordType::ANYWORD_IGNOREWHITESPACES ) )
            aTmp = rFnt.GetActualFont().CalcCaseMap( aTmp );

        j += aTmp.getLength();

        if (TextFrameIndex(j) > nIdx)
            return i;
    }

    return nOfst + nLen;
}

class SwDoCapitals
{
protected:
    SwDrawTextInfo &m_rInf;
    SwCapitalInfo* m_pCapInf; // refers to additional information
                           // required by the ::Do function
    explicit SwDoCapitals ( SwDrawTextInfo &rInfo ) : m_rInf( rInfo ), m_pCapInf( nullptr ) { }
    ~SwDoCapitals() {}
public:
    virtual void Init( SwFntObj *pUpperFont, SwFntObj *pLowerFont ) = 0;
    virtual void Do() = 0;
    OutputDevice& GetOut() { return m_rInf.GetOut(); }
    SwDrawTextInfo& GetInf() { return m_rInf; }
    SwCapitalInfo* GetCapInf() const { return m_pCapInf; }
    void SetCapInf( SwCapitalInfo& rNew ) { m_pCapInf = &rNew; }
};

namespace {

class SwDoGetCapitalSize : public SwDoCapitals
{
protected:
    Size m_aTextSize;

public:
    explicit SwDoGetCapitalSize( SwDrawTextInfo &rInfo ) : SwDoCapitals ( rInfo ) { }
    virtual ~SwDoGetCapitalSize() {}
    virtual void Init( SwFntObj *pUpperFont, SwFntObj *pLowerFont ) override;
    virtual void Do() override;
    const Size& GetSize() const { return m_aTextSize; }
};

}

void SwDoGetCapitalSize::Init( SwFntObj *, SwFntObj * )
{
    m_aTextSize.setHeight(0);
    m_aTextSize.setWidth(0);
}

void SwDoGetCapitalSize::Do()
{
    m_aTextSize.AdjustWidth(m_rInf.GetSize().Width());
    if( m_rInf.GetUpper() )
        m_aTextSize.setHeight(m_rInf.GetSize().Height());
}

Size SwSubFont::GetCapitalSize( SwDrawTextInfo& rInf )
{
    // Start:
    const tools::Long nOldKern = rInf.GetKern();
    rInf.SetKern( CheckKerning() );
    rInf.SetPos( Point() );
    rInf.SetSpace( 0 );
    rInf.SetDrawSpace( false );
    SwDoGetCapitalSize aDo( rInf );
    DoOnCapitals( aDo );
    Size aTextSize( aDo.GetSize() );

    // End:
    if( !aTextSize.Height() )
    {
        SV_STAT( nGetTextSize );
        aTextSize.setHeight( short ( rInf.GetpOut()->GetTextHeight() ) );
    }
    rInf.SetKern( nOldKern );
    return aTextSize;
}

namespace {

class SwDoGetCapitalBreak : public SwDoCapitals
{
protected:
    tools::Long m_nTextWidth;
    TextFrameIndex m_nBreak;

public:
    SwDoGetCapitalBreak(SwDrawTextInfo& rInfo, tools::Long const nWidth)
        : SwDoCapitals(rInfo)
        , m_nTextWidth(nWidth)
        , m_nBreak(-1)
    { }
    virtual ~SwDoGetCapitalBreak() {}
    virtual void Init( SwFntObj *pUpperFont, SwFntObj *pLowerFont ) override;
    virtual void Do() override;
    TextFrameIndex getBreak() const { return m_nBreak; }
};

}

void SwDoGetCapitalBreak::Init( SwFntObj *, SwFntObj * )
{
}

void SwDoGetCapitalBreak::Do()
{
    if (!m_nTextWidth)
        return;

    if (m_rInf.GetSize().Width() < m_nTextWidth)
        m_nTextWidth -= m_rInf.GetSize().Width();
    else
    {
        TextFrameIndex nEnd = m_rInf.GetEnd();
        m_nBreak = TextFrameIndex(GetOut().GetTextBreak(m_rInf.GetText(), m_nTextWidth,
                                                        sal_Int32(m_rInf.GetIdx()),
                                                        sal_Int32(m_rInf.GetLen()), m_rInf.GetKern()));

        if (m_nBreak > nEnd || m_nBreak < TextFrameIndex(0))
            m_nBreak = nEnd;

        // m_nBreak may be relative to the display string. It has to be
        // calculated relative to the original string:
        if ( GetCapInf()  )
        {
            if ( GetCapInf()->nLen != m_rInf.GetLen() )
                m_nBreak = sw_CalcCaseMap( *m_rInf.GetFont(),
                                          GetCapInf()->rString,
                                          GetCapInf()->nIdx,
                                          GetCapInf()->nLen, m_nBreak );
            else
                m_nBreak = m_nBreak + GetCapInf()->nIdx;
        }

        m_nTextWidth = 0;
    }
}

TextFrameIndex SwFont::GetCapitalBreak( SwViewShell const * pSh, const OutputDevice* pOut,
    const SwScriptInfo* pScript, const OUString& rText, tools::Long const nTextWidth,
    TextFrameIndex const nIdx, TextFrameIndex const nLen)
{
    // Start:
    Point aPos( 0, 0 );
    SwDrawTextInfo aInfo(pSh, *const_cast<OutputDevice*>(pOut), pScript, rText, nIdx, nLen,
                         /*layout context*/ std::nullopt, 0, false);
    aInfo.SetPos( aPos );
    aInfo.SetSpace( 0 );
    aInfo.SetWrong( nullptr );
    aInfo.SetGrammarCheck( nullptr );
    aInfo.SetSmartTags( nullptr );
    aInfo.SetDrawSpace( false );
    aInfo.SetKern( CheckKerning() );
    aInfo.SetKanaComp( pScript ? 0 : 100 );
    aInfo.SetFont( this );

    SwDoGetCapitalBreak aDo(aInfo, nTextWidth);
    DoOnCapitals( aDo );
    return aDo.getBreak();
}

namespace {

class SwDoDrawCapital : public SwDoCapitals
{
protected:
    SwFntObj* m_pUpperFnt;
    SwFntObj* m_pLowerFnt;

public:
    explicit SwDoDrawCapital(SwDrawTextInfo& rInfo)
        : SwDoCapitals(rInfo)
        , m_pUpperFnt(nullptr)
        , m_pLowerFnt(nullptr)
    { }
    virtual ~SwDoDrawCapital() {}
    virtual void Init( SwFntObj *pUpperFont, SwFntObj *pLowerFont ) override;
    virtual void Do() override;
    void DrawSpace( Point &rPos );
};

}

void SwDoDrawCapital::Init( SwFntObj *pUpperFont, SwFntObj *pLowerFont )
{
    m_pUpperFnt = pUpperFont;
    m_pLowerFnt = pLowerFont;
}

void SwDoDrawCapital::Do()
{
    SV_STAT( nDrawText );
    const sal_uInt16 nOrgWidth = m_rInf.GetWidth();
    m_rInf.SetWidth( sal_uInt16(m_rInf.GetSize().Width()) );
    if ( m_rInf.GetUpper() )
        m_pUpperFnt->DrawText(m_rInf);
    else
    {
        bool bOldBullet = m_rInf.GetBullet();
        m_rInf.SetBullet( false );
        m_pLowerFnt->DrawText(m_rInf);
        m_rInf.SetBullet( bOldBullet );
    }

    OSL_ENSURE(m_pUpperFnt, "No upper font, dying soon!");
    m_rInf.Shift(m_pUpperFnt->GetFont().GetOrientation());
    m_rInf.SetWidth( nOrgWidth );
}

void SwDoDrawCapital::DrawSpace( Point &rPos )
{
    tools::Long nDiff = m_rInf.GetPos().X() - rPos.X();

    Point aPos( rPos );
    const bool bSwitchL2R = m_rInf.GetFrame()->IsRightToLeft() &&
                          ! m_rInf.IsIgnoreFrameRTL();

    if ( bSwitchL2R )
       m_rInf.GetFrame()->SwitchLTRtoRTL( aPos );

    const vcl::text::ComplexTextLayoutFlags nMode = m_rInf.GetpOut()->GetLayoutMode();
    const bool bBidiPor = ( bSwitchL2R !=
                            ( vcl::text::ComplexTextLayoutFlags::Default != ( vcl::text::ComplexTextLayoutFlags::BiDiRtl & nMode ) ) );

    if ( bBidiPor )
        nDiff = -nDiff;

    if ( m_rInf.GetFrame()->IsVertical() )
        m_rInf.GetFrame()->SwitchHorizontalToVertical( aPos );

    if ( nDiff )
    {
        m_rInf.ApplyAutoColor();
        GetOut().DrawStretchText( aPos, nDiff,
            "  ", 0, 2 );
    }
    rPos.setX( m_rInf.GetPos().X() + m_rInf.GetWidth() );
}

void SwSubFont::DrawCapital( SwDrawTextInfo &rInf )
{
    // Precondition: rInf.GetPos() has already been calculated

    rInf.SetDrawSpace( GetUnderline() != LINESTYLE_NONE ||
                       GetOverline()  != LINESTYLE_NONE ||
                       GetStrikeout() != STRIKEOUT_NONE );
    SwDoDrawCapital aDo( rInf );
    DoOnCapitals( aDo );
}

namespace {

class SwDoCapitalCursorOfst : public SwDoCapitals
{
protected:
    SwFntObj* m_pUpperFnt;
    SwFntObj* m_pLowerFnt;
    TextFrameIndex m_nCursor;
    sal_uInt16 m_nOfst;

public:
    SwDoCapitalCursorOfst(SwDrawTextInfo& rInfo, const sal_uInt16 nOfs)
        : SwDoCapitals(rInfo)
        , m_pUpperFnt(nullptr)
        , m_pLowerFnt(nullptr)
        , m_nCursor(0)
        , m_nOfst(nOfs)
    { }
    virtual ~SwDoCapitalCursorOfst() {}
    virtual void Init( SwFntObj *pUpperFont, SwFntObj *pLowerFont ) override;
    virtual void Do() override;

    TextFrameIndex GetCursor() const { return m_nCursor; }
};

}

void SwDoCapitalCursorOfst::Init( SwFntObj *pUpperFont, SwFntObj *pLowerFont )
{
    m_pUpperFnt = pUpperFont;
    m_pLowerFnt = pLowerFont;
}

void SwDoCapitalCursorOfst::Do()
{
    if (!m_nOfst)
        return;

    if (static_cast<tools::Long>(m_nOfst) > m_rInf.GetSize().Width())
    {
        m_nOfst -= m_rInf.GetSize().Width();
        m_nCursor = m_nCursor + m_rInf.GetLen();
    }
    else
    {
        SwDrawTextInfo aDrawInf(m_rInf.GetShell(), *m_rInf.GetpOut(), m_rInf.GetScriptInfo(),
                                m_rInf.GetText(), m_rInf.GetIdx(), m_rInf.GetLen(),
                                /*layout context*/ std::nullopt, 0, false);
        aDrawInf.SetOffset(m_nOfst);
        aDrawInf.SetKern( m_rInf.GetKern() );
        aDrawInf.SetKanaComp( m_rInf.GetKanaComp() );
        aDrawInf.SetFrame( m_rInf.GetFrame() );
        aDrawInf.SetFont( m_rInf.GetFont() );

        if ( m_rInf.GetUpper() )
        {
            aDrawInf.SetSpace( 0 );
            m_nCursor = m_nCursor + m_pUpperFnt->GetModelPositionForViewPoint(aDrawInf);
        }
        else
        {
            aDrawInf.SetSpace( m_rInf.GetSpace() );
            m_nCursor = m_nCursor + m_pLowerFnt->GetModelPositionForViewPoint(aDrawInf);
        }
        m_nOfst = 0;
    }
}

TextFrameIndex SwSubFont::GetCapitalCursorOfst( SwDrawTextInfo& rInf )
{
    const tools::Long nOldKern = rInf.GetKern();
    rInf.SetKern( CheckKerning() );
    SwDoCapitalCursorOfst aDo( rInf, rInf.GetOffset() );
    rInf.SetPos( Point() );
    rInf.SetDrawSpace( false );
    DoOnCapitals( aDo );
    rInf.SetKern( nOldKern );
    return aDo.GetCursor();
}

namespace {

class SwDoDrawStretchCapital : public SwDoDrawCapital
{
    const TextFrameIndex m_nStrLen;
    const sal_uInt16 m_nCapWidth;
    const sal_uInt16 m_nOrgWidth;

public:
    virtual void Do() override;

    SwDoDrawStretchCapital(SwDrawTextInfo& rInfo, const sal_uInt16 nCapitalWidth)
        : SwDoDrawCapital(rInfo)
        , m_nStrLen(rInfo.GetLen())
        , m_nCapWidth(nCapitalWidth)
        , m_nOrgWidth(rInfo.GetWidth())
    { }
};

}

void SwDoDrawStretchCapital::Do()
{
    SV_STAT( nDrawStretchText );
    tools::Long nPartWidth = m_rInf.GetSize().Width();

    if( m_rInf.GetLen() )
    {
        // small caps and kerning
        tools::Long nDiff = tools::Long(m_nOrgWidth) - tools::Long(m_nCapWidth);
        if( nDiff )
        {
            nDiff *= sal_Int32(m_rInf.GetLen());
            nDiff /= sal_Int32(m_nStrLen);
            nDiff += nPartWidth;
            if( 0 < nDiff )
                nPartWidth = nDiff;
        }

        m_rInf.ApplyAutoColor();

        Point aPos( m_rInf.GetPos() );
        const bool bSwitchL2R = m_rInf.GetFrame()->IsRightToLeft() &&
                              ! m_rInf.IsIgnoreFrameRTL();

        if ( bSwitchL2R )
            m_rInf.GetFrame()->SwitchLTRtoRTL( aPos );

        if ( m_rInf.GetFrame()->IsVertical() )
            m_rInf.GetFrame()->SwitchHorizontalToVertical( aPos );

        // Optimise:
        if (TextFrameIndex(1) >= m_rInf.GetLen())
            GetOut().DrawText(aPos, m_rInf.GetText(), sal_Int32(m_rInf.GetIdx()),
                sal_Int32(m_rInf.GetLen()));
        else
            GetOut().DrawStretchText(aPos, nPartWidth, m_rInf.GetText(),
                    sal_Int32(m_rInf.GetIdx()), sal_Int32(m_rInf.GetLen()));
    }
    const_cast<Point&>(m_rInf.GetPos()).AdjustX(nPartWidth );
}

void SwSubFont::DrawStretchCapital( SwDrawTextInfo &rInf )
{
    // Precondition: rInf.GetPos() has already been calculated

    if (rInf.GetLen() == TextFrameIndex(COMPLETE_STRING))
        rInf.SetLen(TextFrameIndex(rInf.GetText().getLength()));

    const Point aOldPos = rInf.GetPos();
    const sal_uInt16 nCapWidth = o3tl::narrowing<sal_uInt16>( GetCapitalSize( rInf ).Width() );
    rInf.SetPos(aOldPos);

    rInf.SetDrawSpace( GetUnderline() != LINESTYLE_NONE ||
                       GetOverline()  != LINESTYLE_NONE ||
                       GetStrikeout() != STRIKEOUT_NONE );
    SwDoDrawStretchCapital aDo( rInf, nCapWidth );
    DoOnCapitals( aDo );
}

void SwSubFont::DoOnCapitals( SwDoCapitals &rDo )
{
    OSL_ENSURE( pLastFont, "SwFont::DoOnCapitals: No LastFont?!" );

    tools::Long nKana = 0;
    const OUString aText( CalcCaseMap( rDo.GetInf().GetText() ) );
    TextFrameIndex nMaxPos = std::min(
        TextFrameIndex(rDo.GetInf().GetText().getLength()) - rDo.GetInf().GetIdx(),
                             rDo.GetInf().GetLen() );
    rDo.GetInf().SetLen( nMaxPos );

    const OUString oldText = rDo.GetInf().GetText();
    rDo.GetInf().SetText( aText );
    TextFrameIndex nPos = rDo.GetInf().GetIdx();
    TextFrameIndex nOldPos = nPos;
    nMaxPos = nMaxPos + nPos;

    // Look if the length of the original text and the ToUpper-converted
    // text is different. If yes, do special handling.
    SwCapitalInfo aCapInf(oldText);
    bool bCaseMapLengthDiffers(aText.getLength() != oldText.getLength());
    if ( bCaseMapLengthDiffers )
        rDo.SetCapInf( aCapInf );

    SwFntObj *pOldLast = pLastFont;
    std::optional<SwFntAccess> oBigFontAccess;
    SwFntObj *pBigFont;
    std::optional<SwFntAccess> oSpaceFontAccess;
    SwFntObj *pSpaceFont = nullptr;

    const void* nFontCacheId2 = nullptr;
    sal_uInt16 nIndex2 = 0;
    SwSubFont aFont( *this );
    Point aStartPos( rDo.GetInf().GetPos() );

    const bool bTextLines = aFont.GetUnderline() != LINESTYLE_NONE
                         || aFont.GetOverline()  != LINESTYLE_NONE
                         || aFont.GetStrikeout() != STRIKEOUT_NONE;
    const bool bWordWise = bTextLines && aFont.IsWordLineMode() &&
                           rDo.GetInf().GetDrawSpace();
    const tools::Long nTmpKern = rDo.GetInf().GetKern();

    if ( bTextLines )
    {
        if ( bWordWise )
        {
            aFont.SetWordLineMode( false );
            oSpaceFontAccess.emplace( nFontCacheId2, nIndex2, &aFont,
                                                rDo.GetInf().GetShell() );
            pSpaceFont = oSpaceFontAccess->Get();
        }
        else
            pSpaceFont = pLastFont;

        // Construct a font for the capitals:
        aFont.SetUnderline( LINESTYLE_NONE );
        aFont.SetOverline( LINESTYLE_NONE );
        aFont.SetStrikeout( STRIKEOUT_NONE );
        nFontCacheId2 = nullptr;
        nIndex2 = 0;
        oBigFontAccess.emplace( nFontCacheId2, nIndex2, &aFont,
                                          rDo.GetInf().GetShell() );
        pBigFont = oBigFontAccess->Get();
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
    nFontCacheId2 = nullptr;
    nIndex2 = 0;
    std::optional<SwFntAccess> oSmallFontAccess( std::in_place, nFontCacheId2, nIndex2, &aFont,
                                                     rDo.GetInf().GetShell() );
    SwFntObj *pSmallFont = oSmallFontAccess->Get();

    rDo.Init( pBigFont, pSmallFont );
    OutputDevice* pOutSize = pSmallFont->GetPrt();
    if( !pOutSize )
        pOutSize = &rDo.GetOut();
    OutputDevice* pOldOut = &rDo.GetOut();

    const LanguageType eLng = LANGUAGE_DONTKNOW == GetLanguage()
                            ? LANGUAGE_SYSTEM : GetLanguage();

    if( nPos < nMaxPos )
    {
        nPos = TextFrameIndex(g_pBreakIt->GetBreakIter()->endOfCharBlock(
                        oldText, sal_Int32(nPos),
            g_pBreakIt->GetLocale( eLng ), CharType::LOWERCASE_LETTER));
        if (nPos < TextFrameIndex(0))
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
                const OUString aNewText = CalcCaseMap(
                    oldText.copy(sal_Int32(nOldPos), sal_Int32(nPos-nOldPos)));
                aCapInf.nIdx = nOldPos;
                aCapInf.nLen = nPos - nOldPos;
                rDo.GetInf().SetTextIdxLen(aNewText, TextFrameIndex(0), TextFrameIndex(aNewText.getLength()));
            }
            else
            {
                rDo.GetInf().SetIdxLen(nOldPos, nPos - nOldPos);
            }

            rDo.GetInf().SetUpper( false );
            rDo.GetInf().SetOut( *pOutSize );
            Size aPartSize = pSmallFont->GetTextSize( rDo.GetInf() );
            nKana += rDo.GetInf().GetKanaDiff();
            rDo.GetInf().SetOut( *pOldOut );
            if( nTmpKern && nPos < nMaxPos )
                aPartSize.AdjustWidth(nTmpKern );
            rDo.GetInf().SetSize( aPartSize );
            rDo.Do();
            nOldPos = nPos;
        }
        nPos = TextFrameIndex(g_pBreakIt->GetBreakIter()->nextCharBlock(
                            oldText, sal_Int32(nPos),
               g_pBreakIt->GetLocale( eLng ), CharType::LOWERCASE_LETTER));
        if (nPos < TextFrameIndex(0) || nPos > nMaxPos)
            nPos = nMaxPos;
        OSL_ENSURE( nPos, "nextCharBlock not implemented?" );
#if OSL_DEBUG_LEVEL > 1
        if( !nPos )
            nPos = nMaxPos;
#endif
        // The upper ones...
        if( nOldPos != nPos )
        {
            const tools::Long nSpaceAdd = rDo.GetInf().GetSpace() / SPACING_PRECISION_FACTOR;

            do
            {
                rDo.GetInf().SetUpper( true );
                pLastFont = pBigFont;
                pLastFont->SetDevFont( rDo.GetInf().GetShell(), rDo.GetOut() );
                TextFrameIndex nTmp;
                if( bWordWise )
                {
                    nTmp = nOldPos;
                    while (nTmp < nPos && CH_BLANK == oldText[sal_Int32(nTmp)])
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
                            const OUString aNewText = CalcCaseMap(
                                oldText.copy(sal_Int32(nOldPos), sal_Int32(nTmp-nOldPos)));
                            aCapInf.nIdx = nOldPos;
                            aCapInf.nLen = nTmp - nOldPos;
                            rDo.GetInf().SetIdx(TextFrameIndex(0));
                            rDo.GetInf().SetLen(TextFrameIndex(aNewText.getLength()));
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
                            aPartSize.AdjustWidth(nSpaceAdd * sal_Int32(nTmp - nOldPos));
                        if( nTmpKern && nPos < nMaxPos )
                            aPartSize.AdjustWidth(nTmpKern );
                        rDo.GetInf().SetSize( aPartSize );
                        rDo.Do();
                        aStartPos = rDo.GetInf().GetPos();
                        nOldPos = nTmp;
                    }

                    while (nTmp < nPos && CH_BLANK != oldText[sal_Int32(nTmp)])
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
                        const OUString aNewText = CalcCaseMap(
                            oldText.copy(sal_Int32(nOldPos), sal_Int32(nTmp-nOldPos)));
                        aCapInf.nIdx = nOldPos;
                        aCapInf.nLen = nTmp - nOldPos;
                        rDo.GetInf().SetTextIdxLen( aNewText, TextFrameIndex(0), TextFrameIndex(aNewText.getLength()));
                    }
                    else
                    {
                        rDo.GetInf().SetIdxLen( nOldPos, nTmp - nOldPos );
                    }

                    rDo.GetInf().SetOut( *pOutSize );
                    Size aPartSize = pBigFont->GetTextSize( rDo.GetInf() );
                    nKana += rDo.GetInf().GetKanaDiff();
                    rDo.GetInf().SetOut( *pOldOut );
                    if( !bWordWise && rDo.GetInf().GetSpace() )
                    {
                        for (TextFrameIndex nI = nOldPos; nI < nPos; ++nI)
                        {
                            if (CH_BLANK == oldText[sal_Int32(nI)])
                                aPartSize.AdjustWidth(nSpaceAdd );
                        }
                    }
                    if( nTmpKern && nPos < nMaxPos )
                        aPartSize.AdjustWidth(nTmpKern );
                    rDo.GetInf().SetSize( aPartSize );
                    rDo.Do();
                    nOldPos = nTmp;
                }
            } while( nOldPos != nPos );
        }
        nPos = TextFrameIndex(g_pBreakIt->GetBreakIter()->endOfCharBlock(
                            oldText, sal_Int32(nPos),
               g_pBreakIt->GetLocale( eLng ), CharType::LOWERCASE_LETTER));
        if (nPos < TextFrameIndex(0) || nPos > nMaxPos)
            nPos = nMaxPos;
        OSL_ENSURE( nPos, "endOfCharBlock not implemented?" );
#if OSL_DEBUG_LEVEL > 1
        if( !nPos )
            nPos = nMaxPos;
#endif
    }

    // clean up:
    if( pBigFont != pOldLast )
        oBigFontAccess.reset();

    if( bTextLines )
    {
        if( rDo.GetInf().GetDrawSpace() )
        {
            pLastFont = pSpaceFont;
            pLastFont->SetDevFont( rDo.GetInf().GetShell(), rDo.GetOut() );
            static_cast<SwDoDrawCapital&>( rDo ).DrawSpace( aStartPos );
        }
        if ( bWordWise )
            oSpaceFontAccess.reset();
    }
    pLastFont = pOldLast;
    pLastFont->SetDevFont( rDo.GetInf().GetShell(), rDo.GetOut() );

    oSmallFontAccess.reset();
    rDo.GetInf().SetText(oldText);
    rDo.GetInf().SetKanaDiff( nKana );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
