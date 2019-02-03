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

#include <officecfg/Office/Common.hxx>

#include <comphelper/processfactory.hxx>

#include <unotools/charclass.hxx>

#include <editeng/boxitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/acorrcfg.hxx>

#include <swwait.hxx>
#include <fmtpdsc.hxx>
#include <fmtanchr.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <DocumentRedlineManager.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <redline.hxx>
#include <unocrsr.hxx>
#include <docary.hxx>
#include <editsh.hxx>
#include <index.hxx>
#include <pam.hxx>
#include <edimp.hxx>
#include <fesh.hxx>
#include <swundo.hxx>
#include <poolfmt.hxx>
#include <ndtxt.hxx>
#include <rootfrm.hxx>
#include <txtfrm.hxx>
#include <frminf.hxx>
#include <pagedesc.hxx>
#include <paratr.hxx>
#include <swtable.hxx>
#include <acorrect.hxx>
#include <shellres.hxx>
#include <section.hxx>
#include <frmatr.hxx>
#include <charatr.hxx>
#include <mdiexp.hxx>
#include <strings.hrc>
#include <comcore.hxx>
#include <numrule.hxx>
#include <itabenum.hxx>

#include <memory>

using namespace ::com::sun::star;

//JP 16.12.99: definition:
//      from pos cPosEnDash to cPosEmDash all chars changed to em dashes,
//      from pos cPosEmDash to cPosEnd    all chars changed to em dashes
//      all other chars are changed to the user configuration

const sal_Unicode pBulletChar[6] = { '+', '*', '-', 0x2013, 0x2014, 0 };
const int cnPosEnDash = 2, cnPosEmDash = 4;

const sal_Unicode cStarSymbolEnDash = 0x2013;
const sal_Unicode cStarSymbolEmDash = 0x2014;

SvxSwAutoFormatFlags* SwEditShell::s_pAutoFormatFlags = nullptr;

// Number of num-/bullet-paragraph templates. MAXLEVEL will soon be raised
// to x, but not the number of templates. (Artifact from <= 4.0)
const sal_uInt16 cnNumBullColls = 4;

class SwAutoFormat
{
    SvxSwAutoFormatFlags m_aFlags;
    SwPaM m_aDelPam;            // a Pam that can be used
    SwNodeIndex m_aNdIdx;       // the index on the current TextNode
    SwNodeIndex m_aEndNdIdx;    // index on the end of the area

    SwEditShell* m_pEditShell;
    SwDoc* m_pDoc;
    SwTextNode* m_pCurTextNd;     // the current TextNode
    SwTextFrame* m_pCurTextFrame;     // frame of the current TextNode
    sal_uLong m_nEndNdIdx;      // for the percentage-display
    mutable std::unique_ptr<CharClass> m_pCharClass; // Character classification
    mutable LanguageType m_eCharClassLang;

    sal_uInt16 m_nRedlAutoFormatSeqId;

    enum
    {
        NONE = 0,
        DELIM = 1,
        DIGIT = 2,
        CHG = 4,
        LOWER_ALPHA = 8,
        UPPER_ALPHA = 16,
        LOWER_ROMAN = 32,
        UPPER_ROMAN = 64,
        NO_DELIM = (DIGIT|LOWER_ALPHA|UPPER_ALPHA|LOWER_ROMAN|UPPER_ROMAN)
    };

    bool m_bEnd : 1;
    bool m_bMoreLines : 1;

    CharClass& GetCharClass( LanguageType eLang ) const
    {
        if( !m_pCharClass || eLang != m_eCharClassLang )
        {
            m_pCharClass.reset( new CharClass( LanguageTag( eLang ) ) );
            m_eCharClassLang = eLang;
        }
        return *m_pCharClass;
    }

    static bool IsSpace( const sal_Unicode c )
        { return (' ' == c || '\t' == c || 0x0a == c|| 0x3000 == c /* Jap. space */); }

    void SetColl( sal_uInt16 nId, bool bHdLineOrText = false );
    void GoNextPara();
    bool HasObjects(const SwTextFrame &);

    // TextNode methods
    const SwTextFrame * GetNextNode(bool isCheckEnd = true) const;
    static bool IsEmptyLine(const SwTextFrame & rFrame)
    {
        return rFrame.GetText().isEmpty()
            || rFrame.GetText().getLength() == GetLeadingBlanks(rFrame.GetText());
    }

    bool IsOneLine(const SwTextFrame &) const;
    bool IsFastFullLine(const SwTextFrame &) const;
    bool IsNoAlphaLine(const SwTextFrame &) const;
    bool IsEnumericChar(const SwTextFrame &) const;
    static bool IsBlanksInString(const SwTextFrame&);
    sal_uInt16 CalcLevel(const SwTextFrame&, sal_uInt16 *pDigitLvl = nullptr) const;
    sal_Int32 GetBigIndent(TextFrameIndex & rCurrentSpacePos) const;

    static OUString DelLeadingBlanks(const OUString& rStr);
    static OUString DelTrailingBlanks( const OUString& rStr );
    static sal_Int32 GetLeadingBlanks( const OUString& rStr );
    static sal_Int32 GetTrailingBlanks( const OUString& rStr );

    bool IsFirstCharCapital(const SwTextFrame & rNd) const;
    sal_uInt16 GetDigitLevel(const SwTextFrame& rFrame, TextFrameIndex& rPos,
                            OUString* pPrefix = nullptr, OUString* pPostfix = nullptr,
                            OUString* pNumTypes = nullptr ) const;
    /// get the FORMATTED TextFrame
    SwTextFrame* GetFrame( const SwTextNode& rTextNd ) const;
    SwTextFrame * EnsureFormatted(SwTextFrame const&) const;

    void BuildIndent();
    void BuildText();
    void BuildTextIndent();
    void BuildEnum( sal_uInt16 nLvl, sal_uInt16 nDigitLevel );
    void BuildNegIndent( SwTwips nSpaces );
    void BuildHeadLine( sal_uInt16 nLvl );

    static bool HasBreakAttr(const SwTextFrame &);
    void DeleteSel( SwPaM& rPam );
    void DeleteSelImpl(SwPaM & rDelPam, SwPaM & rPamToCorrect);
    bool DeleteJoinCurNextPara(SwTextFrame const* pNextFrame, bool bIgnoreLeadingBlanks = false);
    /// delete in the node start and/or end
    void DeleteLeadingTrailingBlanks( bool bStart = true, bool bEnd = true );
    void DelEmptyLine( bool bTstNextPara = true );
    /// when using multiline paragraphs delete the "left" and/or
    /// "right" margins
    void DelMoreLinesBlanks( bool bWithLineBreaks = false );
    /// join with the previous paragraph
    void JoinPrevPara();
    /// execute AutoCorrect on current TextNode
    void AutoCorrect(TextFrameIndex nSttPos = TextFrameIndex(0));

    bool CanJoin(const SwTextFrame * pNextFrame) const
    {
        return !m_bEnd && pNextFrame
            && !IsEmptyLine(*pNextFrame)
            && !IsNoAlphaLine(*pNextFrame)
            && !IsEnumericChar(*pNextFrame)
            // check the last / first nodes here...
            && ((COMPLETE_STRING - 50 - pNextFrame->GetTextNodeFirst()->GetText().getLength())
                > (m_pCurTextFrame->GetMergedPara()
                      ? m_pCurTextFrame->GetMergedPara()->pLastNode
                      : m_pCurTextNd)->GetText().getLength())
            && !HasBreakAttr(*pNextFrame);
    }

    /// is a dot at the end ??
    static bool IsSentenceAtEnd(const SwTextFrame & rTextFrame);

    bool DoUnderline();
    bool DoTable();

    void SetRedlineText_( sal_uInt16 nId );
    bool SetRedlineText( sal_uInt16 nId ) {
        if( m_aFlags.bWithRedlining )
            SetRedlineText_( nId );
        return true;
    }
    void ClearRedlineText() {
        if( m_aFlags.bWithRedlining )
            m_pDoc->GetDocumentRedlineManager().SetAutoFormatRedlineComment(nullptr);
    }

public:
    SwAutoFormat( SwEditShell* pEdShell, SvxSwAutoFormatFlags const & rFlags,
                SwNodeIndex const * pSttNd = nullptr, SwNodeIndex const * pEndNd = nullptr );
};

static const sal_Unicode* StrChr( const sal_Unicode* pSrc, sal_Unicode c )
{
    while( *pSrc && *pSrc != c )
        ++pSrc;
    return *pSrc ? pSrc : nullptr;
}

SwTextFrame* SwAutoFormat::GetFrame( const SwTextNode& rTextNd ) const
{
    // get the Frame
    const SwContentFrame *pFrame = rTextNd.getLayoutFrame( m_pEditShell->GetLayout() );
    assert(pFrame && "For Autoformat a Layout is needed");
    return EnsureFormatted(*static_cast<SwTextFrame const*>(pFrame));
}

SwTextFrame * SwAutoFormat::EnsureFormatted(SwTextFrame const& rFrame) const
{
    SwTextFrame *const pFrame(const_cast<SwTextFrame*>(&rFrame));
    if( m_aFlags.bAFormatByInput && !pFrame->isFrameAreaDefinitionValid() )
    {
        DisableCallbackAction a(*pFrame->getRootFrame());
        SwRect aTmpFrame( pFrame->getFrameArea() );
        SwRect aTmpPrt( pFrame->getFramePrintArea() );
        pFrame->Calc(pFrame->getRootFrame()->GetCurrShell()->GetOut());

        if( pFrame->getFrameArea() != aTmpFrame || pFrame->getFramePrintArea() != aTmpPrt ||
            !pFrame->GetPaintSwRect().IsEmpty())
        {
            pFrame->SetCompletePaint();
        }
    }

    return pFrame->GetFormatted();
}

void SwAutoFormat::SetRedlineText_( sal_uInt16 nActionId )
{
    OUString sText;
    sal_uInt16 nSeqNo = 0;
    if( STR_AUTOFMTREDL_END > nActionId )
    {
        sText = SwViewShell::GetShellRes()->GetAutoFormatNameLst()[ nActionId ];
        switch( nActionId )
        {
        case STR_AUTOFMTREDL_SET_NUMBULLET:
        case STR_AUTOFMTREDL_DEL_MORELINES:

        // AutoCorrect actions
        case STR_AUTOFMTREDL_USE_REPLACE:
        case STR_AUTOFMTREDL_CPTL_STT_WORD:
        case STR_AUTOFMTREDL_CPTL_STT_SENT:
        case STR_AUTOFMTREDL_TYPO:
        case STR_AUTOFMTREDL_UNDER:
        case STR_AUTOFMTREDL_BOLD:
        case STR_AUTOFMTREDL_FRACTION:
        case STR_AUTOFMTREDL_DASH:
        case STR_AUTOFMTREDL_ORDINAL:
        case STR_AUTOFMTREDL_NON_BREAK_SPACE:
            nSeqNo = ++m_nRedlAutoFormatSeqId;
            break;
        }
    }
#if OSL_DEBUG_LEVEL > 0
    else
        sText = "Action text is missing";
#endif

    m_pDoc->GetDocumentRedlineManager().SetAutoFormatRedlineComment( &sText, nSeqNo );
}

void SwAutoFormat::GoNextPara()
{
    SwNode* pNewNd = nullptr;
    do {
        // has to be checked twice before and after incrementation
        if( m_aNdIdx.GetIndex() >= m_aEndNdIdx.GetIndex() )
        {
            m_bEnd = true;
            return;
        }

        sw::GotoNextLayoutTextFrame(m_aNdIdx, m_pEditShell->GetLayout());
        if( m_aNdIdx.GetIndex() >= m_aEndNdIdx.GetIndex() )
        {
            m_bEnd = true;
            return;
        }
        else
            pNewNd = &m_aNdIdx.GetNode();

        // not a TextNode ->
        //      TableNode   : skip table
        //      NoTextNode   : skip nodes
        //      EndNode     : at the end, terminate
        if( pNewNd->IsEndNode() )
        {
            m_bEnd = true;
            return;
        }
        else if( pNewNd->IsTableNode() )
            m_aNdIdx = *pNewNd->EndOfSectionNode();
        else if( pNewNd->IsSectionNode() )
        {
            const SwSection& rSect = pNewNd->GetSectionNode()->GetSection();
            if( rSect.IsHiddenFlag() || rSect.IsProtectFlag() )
                m_aNdIdx = *pNewNd->EndOfSectionNode();
        }
    } while( !pNewNd->IsTextNode() );

    if( !m_aFlags.bAFormatByInput )
        ::SetProgressState( m_aNdIdx.GetIndex() + m_nEndNdIdx - m_aEndNdIdx.GetIndex(),
                            m_pDoc->GetDocShell() );

    m_pCurTextNd = static_cast<SwTextNode*>(pNewNd);
    m_pCurTextFrame = GetFrame( *m_pCurTextNd );
}

bool SwAutoFormat::HasObjects(const SwTextFrame & rFrame)
{
    // Is there something bound to the paragraph in the paragraph
    // like Frames, DrawObjects, ...
    bool bRet = false;
    const SwFrameFormats& rFormats = *m_pDoc->GetSpzFrameFormats();
    for( auto pFrameFormat : rFormats )
    {
        const SwFormatAnchor& rAnchor = pFrameFormat->GetAnchor();
        if ((RndStdIds::FLY_AT_PAGE != rAnchor.GetAnchorId()) &&
            rAnchor.GetContentAnchor() &&
            sw::FrameContainsNode(rFrame, rAnchor.GetContentAnchor()->nNode.GetIndex()))
        {
            bRet = true;
            break;
        }
    }
    return bRet;
}

const SwTextFrame* SwAutoFormat::GetNextNode(bool const isCheckEnd) const
{
    SwNodeIndex tmp(m_aNdIdx);
    sw::GotoNextLayoutTextFrame(tmp, m_pEditShell->GetLayout());
    if ((isCheckEnd && m_aEndNdIdx <= tmp) || !tmp.GetNode().IsTextNode())
        return nullptr;
    // note: the returned frame is not necessarily formatted, have to call
    // EnsureFormatted for that
    return static_cast<SwTextFrame*>(tmp.GetNode().GetTextNode()->getLayoutFrame(m_pEditShell->GetLayout()));
}

bool SwAutoFormat::IsOneLine(const SwTextFrame & rFrame) const
{
    SwTextFrameInfo aFInfo( EnsureFormatted(rFrame) );
    return aFInfo.IsOneLine();
}

bool SwAutoFormat::IsFastFullLine(const SwTextFrame & rFrame) const
{
    bool bRet = m_aFlags.bRightMargin;
    if( bRet )
    {
        SwTextFrameInfo aFInfo( EnsureFormatted(rFrame) );
        bRet = aFInfo.IsFilled( m_aFlags.nRightMargin );
    }
    return bRet;
}

bool SwAutoFormat::IsEnumericChar(const SwTextFrame& rFrame) const
{
    const OUString& rText = rFrame.GetText();
    TextFrameIndex nBlanks(GetLeadingBlanks(rText));
    const TextFrameIndex nLen = TextFrameIndex(rText.getLength()) - nBlanks;
    if( !nLen )
        return false;

    // -, +, * separated by blank ??
    if (TextFrameIndex(2) < nLen && IsSpace(rText[sal_Int32(nBlanks) + 1]))
    {
        if (StrChr(pBulletChar, rText[sal_Int32(nBlanks)]))
            return true;
        // Should there be a symbol font at the position?
        SwTextFrameInfo aFInfo( EnsureFormatted(rFrame) );
        if (aFInfo.IsBullet(nBlanks))
            return true;
    }

    // 1.) / 1. / 1.1.1 / (1). / (1) / ....
    return USHRT_MAX != GetDigitLevel(rFrame, nBlanks);
}

bool SwAutoFormat::IsBlanksInString(const SwTextFrame& rFrame)
{
    // Search more than 5 consecutive blanks/tabs in the string.
    OUString sTmp( DelLeadingBlanks(rFrame.GetText()) );
    const sal_Int32 nLen = sTmp.getLength();
    sal_Int32 nIdx = 0;
    while (nIdx < nLen)
    {
        // Skip non-blanks
        while (nIdx < nLen && !IsSpace(sTmp[nIdx])) ++nIdx;
        if (nIdx == nLen)
            return false;
        // Then count consecutive blanks
        const sal_Int32 nFirst = nIdx;
        while (nIdx < nLen && IsSpace(sTmp[nIdx])) ++nIdx;
        // And exit if enough consecutive blanks were found
        if (nIdx-nFirst > 5)
            return true;
    }
    return false;
}

sal_uInt16 SwAutoFormat::CalcLevel(const SwTextFrame & rFrame,
        sal_uInt16 *const pDigitLvl) const
{
    sal_uInt16 nLvl = 0, nBlnk = 0;
    const OUString& rText = rFrame.GetText();
    if( pDigitLvl )
        *pDigitLvl = USHRT_MAX;

    if (RES_POOLCOLL_TEXT_MOVE == rFrame.GetTextNodeForParaProps()->GetTextColl()->GetPoolFormatId())
    {
        if( m_aFlags.bAFormatByInput )
        {
            // this is very non-obvious: on the *first* invocation of
            // AutoFormat, the node will have the tabs (any number) converted
            // to a fixed indent in BuildTextIndent(), and the number of tabs
            // is stored in the node;
            // on the *second* invocation of AutoFormat, CalcLevel() will
            // retrieve the stored number, and it will be used by
            // BuildHeadLine() to select the corresponding heading style.
            nLvl = rFrame.GetTextNodeForParaProps()->GetAutoFormatLvl();
            const_cast<SwTextNode *>(rFrame.GetTextNodeForParaProps())->SetAutoFormatLvl(0);
            if( nLvl )
                return nLvl;
        }
        ++nLvl;
    }

    for (TextFrameIndex n = TextFrameIndex(0),
                     nEnd = TextFrameIndex(rText.getLength()); n < nEnd; ++n)
    {
        switch (rText[sal_Int32(n)])
        {
        case ' ':   if( 3 == ++nBlnk )
                    {
                        ++nLvl;
                        nBlnk = 0;
                    }
                    break;
        case '\t':  ++nLvl;
                    nBlnk = 0;
                    break;
        default:
            if( pDigitLvl )
                // test 1.) / 1. / 1.1.1 / (1). / (1) / ....
                *pDigitLvl = GetDigitLevel(rFrame, n);
            return nLvl;
        }
    }
    return nLvl;
}

sal_Int32 SwAutoFormat::GetBigIndent(TextFrameIndex & rCurrentSpacePos) const
{
    SwTextFrameInfo aFInfo( m_pCurTextFrame );
    const SwTextFrame* pNextFrame = nullptr;

    if( !m_bMoreLines )
    {
        pNextFrame = GetNextNode();
        if (!CanJoin(pNextFrame) || !IsOneLine(*pNextFrame))
            return 0;

        pNextFrame = EnsureFormatted(*pNextFrame);
    }

    return aFInfo.GetBigIndent( rCurrentSpacePos, pNextFrame );
}

bool SwAutoFormat::IsNoAlphaLine(const SwTextFrame & rFrame) const
{
    const OUString& rStr = rFrame.GetText();
    if( rStr.isEmpty() )
        return false;
    // or better: determine via number of AlphaNum and !AlphaNum characters
    sal_Int32 nANChar = 0, nBlnk = 0;

    for (TextFrameIndex n = TextFrameIndex(0),
                     nEnd = TextFrameIndex(rStr.getLength()); n < nEnd; ++n)
        if (IsSpace(rStr[sal_Int32(n)]))
            ++nBlnk;
        else
        {
            auto const pair = rFrame.MapViewToModel(n);
            CharClass& rCC = GetCharClass(pair.first->GetSwAttrSet().GetLanguage().GetLanguage());
            if (rCC.isLetterNumeric(rStr, sal_Int32(n)))
                ++nANChar;
        }

    // If there are 75% of non-alphanumeric characters, then true
    sal_uLong nLen = rStr.getLength() - nBlnk;
    nLen = ( nLen * 3 ) / 4;            // long overflow, if the strlen > sal_uInt16
    return sal_Int32(nLen) < (rStr.getLength() - nANChar - nBlnk);
}

bool SwAutoFormat::DoUnderline()
{
    if( !m_aFlags.bSetBorder )
        return false;

    OUString const& rText(m_pCurTextFrame->GetText());
    int eState = 0;
    sal_Int32 nCnt = 0;
    while (nCnt < rText.getLength())
    {
        int eTmp = 0;
        switch (rText[nCnt])
        {
            case '-': eTmp = 1; break;
            case '_': eTmp = 2; break;
            case '=': eTmp = 3; break;
            case '*': eTmp = 4; break;
            case '~': eTmp = 5; break;
            case '#': eTmp = 6; break;
            default:
                return false;
        }
        if( 0 == eState )
            eState = eTmp;
        else if( eState != eTmp )
            return false;
        ++nCnt;
    }

    if( 2 < nCnt )
    {
        // then underline the previous paragraph if one exists
        DelEmptyLine( false ); // -> point will be on end of current paragraph
        // WARNING: rText may be deleted now, m_pCurTextFrame may be nullptr
        m_aDelPam.SetMark();
        // apply to last node & rely on InsertItemSet to apply it to props-node
        m_aDelPam.GetMark()->nContent = 0;

        editeng::SvxBorderLine aLine;
        switch( eState )
        {
        case 1:         // single, 0.05 pt
            aLine.SetBorderLineStyle(SvxBorderLineStyle::SOLID);
            aLine.SetWidth( DEF_LINE_WIDTH_0 );
            break;
        case 2:         // single, 1.0 pt
            aLine.SetBorderLineStyle(SvxBorderLineStyle::SOLID);
            aLine.SetWidth( DEF_LINE_WIDTH_1 );
            break;
        case 3:         // double, 1.0 pt
            aLine.SetBorderLineStyle(SvxBorderLineStyle::DOUBLE);
            aLine.SetWidth( DEF_LINE_WIDTH_1 );
            break;
        case 4:         // double (thick/thin), 4.0 pt
            aLine.SetBorderLineStyle(SvxBorderLineStyle::THICKTHIN_SMALLGAP);
            aLine.SetWidth( DEF_LINE_WIDTH_3  );
            break;
        case 5:         // double (thin/thick), 4.0 pt
            aLine.SetBorderLineStyle(SvxBorderLineStyle::THINTHICK_SMALLGAP);
            aLine.SetWidth( DEF_LINE_WIDTH_3 );
            break;
        case 6:         // double, 2.5 pt
            aLine.SetBorderLineStyle(SvxBorderLineStyle::DOUBLE);
            aLine.SetWidth( DEF_LINE_WIDTH_2 );
            break;
        }
        SfxItemSet aSet(m_pDoc->GetAttrPool(),
                    svl::Items<RES_PARATR_CONNECT_BORDER, RES_PARATR_CONNECT_BORDER,
                    RES_BOX, RES_BOX>{});
        aSet.Put( SwParaConnectBorderItem( false ) );
        SvxBoxItem aBox( RES_BOX );
        aBox.SetLine( &aLine, SvxBoxItemLine::BOTTOM );
        aBox.SetDistance(42, SvxBoxItemLine::BOTTOM );     // ~0,75 mm
        aSet.Put(aBox);
        m_pDoc->getIDocumentContentOperations().InsertItemSet(m_aDelPam, aSet,
                SetAttrMode::DEFAULT, m_pEditShell->GetLayout());

        m_aDelPam.DeleteMark();
    }
    return 2 < nCnt;
}

bool SwAutoFormat::DoTable()
{
    if( !m_aFlags.bCreateTable || !m_aFlags.bAFormatByInput ||
        m_pCurTextNd->FindTableNode() )
        return false;

    const OUString& rTmp = m_pCurTextFrame->GetText();
    TextFrameIndex nSttPlus(GetLeadingBlanks(rTmp));
    TextFrameIndex nEndPlus(GetTrailingBlanks(rTmp));
    sal_Unicode cChar;

    if (TextFrameIndex(2) > nEndPlus - nSttPlus
        || ('+' != (cChar = rTmp[sal_Int32(nSttPlus)]) && '|' != cChar)
        || ('+' != (cChar = rTmp[sal_Int32(nEndPlus) - 1]) && '|' != cChar))
        return false;

    SwTextFrameInfo aInfo( m_pCurTextFrame );

    TextFrameIndex n = nSttPlus;
    std::vector<sal_uInt16> aPosArr;

    while (n < TextFrameIndex(rTmp.getLength()))
    {
        switch (rTmp[sal_Int32(n)])
        {
        case '-':
        case '_':
        case '=':
        case ' ':
        case '\t':
            break;

        case '+':
        case '|':
            aPosArr.push_back( static_cast<sal_uInt16>(aInfo.GetCharPos(n)) );
            break;

        default:
            return false;
        }
        if( ++n == nEndPlus )
            break;
    }

    if( 1 < aPosArr.size() )
    {
        // get the text node's alignment
        sal_uInt16 nColCnt = aPosArr.size() - 1;
        SwTwips nSttPos = aPosArr[ 0 ];
        sal_Int16 eHori;
        switch (m_pCurTextFrame->GetTextNodeForParaProps()->GetSwAttrSet().GetAdjust().GetAdjust())
        {
        case SvxAdjust::Center:     eHori = text::HoriOrientation::CENTER;    break;
        case SvxAdjust::Right:      eHori = text::HoriOrientation::RIGHT;     break;

        default:
            if( nSttPos )
            {
                eHori = text::HoriOrientation::NONE;
                // then - as last - we need to add the current frame width into the array
                aPosArr.push_back( static_cast<sal_uInt16>(m_pCurTextFrame->getFrameArea().Width()) );
            }
            else
                eHori = text::HoriOrientation::LEFT;
            break;
        }

        // then create a table that matches the character
        DelEmptyLine();
        // WARNING: rTmp may be deleted now, m_pCurTextFrame may be nullptr
        SwNodeIndex aIdx( m_aDelPam.GetPoint()->nNode );
        m_aDelPam.Move( fnMoveForward );
        m_pDoc->InsertTable( SwInsertTableOptions( SwInsertTableFlags::All , 1 ),
                           *m_aDelPam.GetPoint(), 1, nColCnt, eHori,
                           nullptr, &aPosArr );
        m_aDelPam.GetPoint()->nNode = aIdx;
    }
    return 1 < aPosArr.size();
}

OUString SwAutoFormat::DelLeadingBlanks( const OUString& rStr )
{
    sal_Int32 nL, n;
    for( nL = rStr.getLength(), n = 0; n < nL && IsSpace( rStr[n] ); ++n )
        ;
    if( n ) // no Spaces
        return rStr.copy(n);
    return rStr;
}

OUString SwAutoFormat::DelTrailingBlanks( const OUString& rStr )
{
    sal_Int32 nL = rStr.getLength(), n = nL;
    if( !nL )
        return rStr;

    while( --n && IsSpace( rStr[ n ] )  )
        ;
    if( n+1 != nL ) // no Spaces
        return rStr.copy( 0, n+1 );
    return rStr;
}

sal_Int32 SwAutoFormat::GetLeadingBlanks( const OUString& rStr )
{
    sal_Int32 nL;
    sal_Int32 n;

    for( nL = rStr.getLength(), n = 0; n < nL && IsSpace( rStr[ n ] ); ++n )
        ;
    return n;
}

sal_Int32 SwAutoFormat::GetTrailingBlanks( const OUString& rStr )
{
    sal_Int32 nL = rStr.getLength(), n = nL;
    if( !nL )
        return 0;

    while( --n && IsSpace( rStr[ n ] )  )
        ;
    return ++n;
}

bool SwAutoFormat::IsFirstCharCapital(const SwTextFrame& rFrame) const
{
    const OUString& rText = rFrame.GetText();
    for (TextFrameIndex n = TextFrameIndex(0),
                     nEnd = TextFrameIndex(rText.getLength()); n < nEnd; ++n)
        if (!IsSpace(rText[sal_Int32(n)]))
        {
            auto const pair = rFrame.MapViewToModel(n);
            CharClass& rCC = GetCharClass( pair.first->GetSwAttrSet().
                                        GetLanguage().GetLanguage() );
            sal_Int32 nCharType = rCC.getCharacterType(rText, sal_Int32(n));
            return CharClass::isLetterType( nCharType ) &&
                   0 != ( i18n::KCharacterType::UPPER &
                                                    nCharType );
        }
    return false;
}

sal_uInt16
SwAutoFormat::GetDigitLevel(const SwTextFrame& rFrame, TextFrameIndex& rPos,
        OUString* pPrefix, OUString* pPostfix, OUString* pNumTypes ) const
{

    // check for 1.) / 1. / 1.1.1 / (1). / (1) / ....
    const OUString& rText = rFrame.GetText();
    sal_Int32 nPos(rPos);
    int eScan = NONE;

    sal_uInt16 nStart = 0;
    sal_uInt8 nDigitLvl = 0, nDigitCnt = 0;
    // count number of parenthesis to assure a sensible order is found
    sal_uInt16 nOpeningParentheses = 0;
    sal_uInt16 nClosingParentheses = 0;

    while (nPos < rText.getLength() && nDigitLvl < MAXLEVEL - 1)
    {
        auto const pair = rFrame.MapViewToModel(TextFrameIndex(nPos));
        CharClass& rCC = GetCharClass(pair.first->GetSwAttrSet().GetLanguage().GetLanguage());
        const sal_Unicode cCurrentChar = rText[nPos];
        if( ('0' <= cCurrentChar &&  '9' >= cCurrentChar) ||
            (0xff10 <= cCurrentChar &&  0xff19 >= cCurrentChar) )
        {
            if( eScan & DELIM )
            {
                if( eScan & CHG )   // not if it starts with a number
                {
                    ++nDigitLvl;
                    if( pPostfix )
                        *pPostfix += "\x01";
                }

                if( pNumTypes )
                    *pNumTypes += OUStringLiteral1('0' + SVX_NUM_ARABIC);

                eScan = eScan | CHG;
            }
            else if( pNumTypes && !(eScan & DIGIT) )
                *pNumTypes += OUStringLiteral1('0' + SVX_NUM_ARABIC);

            eScan &= ~DELIM;        // remove Delim
            if( 0 != (eScan & ~CHG) && DIGIT != (eScan & ~CHG))
                return USHRT_MAX;

            eScan |= DIGIT;         // add Digit
            if( 3 == ++nDigitCnt )  // more than 2 numbers are not an enum anymore
                return USHRT_MAX;

            nStart *= 10;
            nStart += cCurrentChar <= '9' ? cCurrentChar - '0' : cCurrentChar - 0xff10;
        }
        else if( rCC.isAlpha( rText, nPos ) )
        {
            bool bIsUpper =
                0 != ( i18n::KCharacterType::UPPER &
                                        rCC.getCharacterType( rText, nPos ));
            sal_Unicode cLow = rCC.lowercase(rText, nPos, 1)[0], cNumTyp;
            int eTmpScan;

            // Roman numbers are "mdclxvi". Since we want to start numbering with c or d more often,
            // convert first to characters and later to roman numbers if needed.
            if( 256 > cLow  && strchr( "mdclxvi", cLow ) )
            {
                if( bIsUpper )
                {
                    cNumTyp = '0' + SVX_NUM_ROMAN_UPPER;
                    eTmpScan = UPPER_ROMAN;
                }
                else
                {
                    cNumTyp = '0' + SVX_NUM_ROMAN_LOWER;
                    eTmpScan = LOWER_ROMAN;
                }
            }
            else if( bIsUpper )
            {
                cNumTyp = '0' + SVX_NUM_CHARS_UPPER_LETTER;
                eTmpScan = UPPER_ALPHA;
            }
            else
            {
                cNumTyp = '0' + SVX_NUM_CHARS_LOWER_LETTER;
                eTmpScan = LOWER_ALPHA;
            }

            // Switch to roman numbers (only for c/d!)
            if( 1 == nDigitCnt && ( eScan & (UPPER_ALPHA|LOWER_ALPHA) ) &&
                ( 3 == nStart || 4 == nStart) && 256 > cLow &&
                strchr( "mdclxvi", cLow ) &&
                (( eScan & UPPER_ALPHA ) ? (eTmpScan & (UPPER_ALPHA|UPPER_ROMAN))
                                         : (eTmpScan & (LOWER_ALPHA|LOWER_ROMAN))) )
            {
                sal_Unicode c = '0';
                nStart = 3 == nStart ? 100 : 500;
                if( UPPER_ALPHA == eTmpScan )
                {
                    eTmpScan = UPPER_ROMAN;
                    c += SVX_NUM_ROMAN_UPPER;
                }
                else
                {
                    eTmpScan = LOWER_ROMAN;
                    c += SVX_NUM_ROMAN_LOWER;
                }

                eScan = (eScan & ~(UPPER_ALPHA|LOWER_ALPHA)) | eTmpScan;
                if( pNumTypes )
                    (*pNumTypes) = pNumTypes->replaceAt( pNumTypes->getLength() - 1, 1, OUString(c) );
            }

            if( eScan & DELIM )
            {
                if( eScan & CHG )   // not if it starts with a number
                {
                    ++nDigitLvl;
                    if( pPostfix )
                        *pPostfix += "\x01";
                }

                if( pNumTypes )
                    *pNumTypes += OUStringLiteral1(cNumTyp);
                eScan = eScan | CHG;
            }
            else if( pNumTypes && !(eScan & eTmpScan) )
                *pNumTypes += OUStringLiteral1(cNumTyp);

            eScan &= ~DELIM;        // remove Delim

            // if another type is set, stop here
            if( 0 != ( eScan & ~CHG ) && eTmpScan != ( eScan & ~CHG ))
                return USHRT_MAX;

            if( eTmpScan & (UPPER_ALPHA | LOWER_ALPHA) )
            {
                // allow characters only if they appear once
                return USHRT_MAX;
            }
            else
            {
                // roman numbers, check if valid characters
                sal_uInt16 nVal;
                bool bError = false;
                switch( cLow )
                {
                case 'm':   nVal = 1000; goto CHECK_ROMAN_1;
                case 'd':   nVal =  500; goto CHECK_ROMAN_5;
                case 'c':   nVal =  100; goto CHECK_ROMAN_1;
                case 'l':   nVal =   50; goto CHECK_ROMAN_5;
                case 'x':   nVal =   10; goto CHECK_ROMAN_1;
                case 'v':   nVal =    5; goto CHECK_ROMAN_5;

CHECK_ROMAN_1:
                    {
                        int nMod5 = nStart % (nVal * 5);
                        int nLast = nStart % nVal;
                        int n10 = nVal / 10;

                        if( nMod5 == ((3 * nVal) + n10 ) ||
                            nMod5 == ((4 * nVal) + n10 ) ||
                            nLast == n10 )
                            nStart = static_cast<sal_uInt16>(nStart + (n10 * 8));
                        else if( nMod5 == 0 ||
                                 nMod5 == (1 * nVal) ||
                                 nMod5 == (2 * nVal) )
                            nStart = nStart + nVal;
                        else
                            bError = true;
                    }
                    break;

CHECK_ROMAN_5:
                    {
                        if( ( nStart / nVal ) & 1 )
                            bError = true;
                        else
                        {
                            int nMod = nStart % nVal;
                            int n10 = nVal / 5;
                            if( n10 == nMod )
                                nStart = static_cast<sal_uInt16>(nStart + (3 * n10));
                            else if( 0 == nMod )
                                nStart = nStart + nVal;
                            else
                                bError = true;
                        }
                    }
                    break;

                case 'i':
                        if( nStart % 5 >= 3 )
                            bError = true;
                        else
                            nStart += 1;
                        break;

                default:
                    bError = true;
                }

                if( bError )
                    return USHRT_MAX;
            }
            eScan |= eTmpScan;          // add Digit
            ++nDigitCnt;
        }
        else if( (256 > cCurrentChar &&
                 strchr( ".)(", cCurrentChar )) ||
                 0x3002 == cCurrentChar /* Chinese trad. dot */||
                 0xff0e == cCurrentChar /* Japanese dot */||
                 0xFF08 == cCurrentChar /* opening bracket Chin./Jap.*/||
                 0xFF09 == cCurrentChar )/* closing bracket Chin./Jap. */
        {
            if(cCurrentChar == '(' || cCurrentChar == 0xFF09)
                nOpeningParentheses++;
            else if(cCurrentChar == ')'|| cCurrentChar == 0xFF08)
                nClosingParentheses++;
            // only if no numbers were read until here
            if( pPrefix && !( eScan & ( NO_DELIM | CHG )) )
                *pPrefix += OUStringLiteral1(rText[nPos]);
            else if( pPostfix )
                *pPostfix += OUStringLiteral1(rText[nPos]);

            if( NO_DELIM & eScan )
            {
                eScan |= CHG;
                if( pPrefix )
                    *pPrefix += "\x01" + OUString::number( nStart );
            }
            eScan &= ~NO_DELIM;     // remove Delim
            eScan |= DELIM;         // add Digit
            nDigitCnt = 0;
            nStart = 0;
        }
        else
            break;
        ++nPos;
    }
    if (!( CHG & eScan ) || rPos == TextFrameIndex(nPos) ||
        nPos == rText.getLength() || !IsSpace(rText[nPos]) ||
        (nOpeningParentheses > nClosingParentheses))
        return USHRT_MAX;

    if( (NO_DELIM & eScan) && pPrefix )     // do not forget the last one
        *pPrefix += "\x01" + OUString::number( nStart );

    rPos = TextFrameIndex(nPos);
    return nDigitLvl;       // 0 .. 9 (MAXLEVEL - 1)
}

void SwAutoFormat::SetColl( sal_uInt16 nId, bool bHdLineOrText )
{
    m_aDelPam.DeleteMark();
    m_aDelPam.GetPoint()->nNode = *m_pCurTextFrame->GetTextNodeForParaProps();
    m_aDelPam.GetPoint()->nContent.Assign(m_aDelPam.GetPoint()->nNode.GetNode().GetContentNode(), 0);

    // keep hard tabs, alignment, language, hyphenation, DropCaps and nearly all frame attributes
    SfxItemSet aSet(
        m_pDoc->GetAttrPool(),
        svl::Items<
            RES_CHRATR_LANGUAGE, RES_CHRATR_LANGUAGE,
            RES_PARATR_ADJUST, RES_PARATR_ADJUST,
            RES_PARATR_TABSTOP, RES_PARATR_DROP,
            RES_BACKGROUND, RES_SHADOW>{});

    if (m_aDelPam.GetPoint()->nNode.GetNode().GetTextNode()->HasSwAttrSet())
    {
        aSet.Put(*m_aDelPam.GetPoint()->nNode.GetNode().GetTextNode()->GetpSwAttrSet());
        // take HeaderLine/TextBody only if centered or right aligned, otherwise only justification
        SvxAdjustItem const * pAdj;
        if( SfxItemState::SET == aSet.GetItemState( RES_PARATR_ADJUST,
                        false, reinterpret_cast<const SfxPoolItem**>(&pAdj) ))
        {
            SvxAdjust eAdj = pAdj->GetAdjust();
            if( bHdLineOrText ? (SvxAdjust::Right != eAdj &&
                                 SvxAdjust::Center != eAdj)
                              : SvxAdjust::Block != eAdj )
                aSet.ClearItem( RES_PARATR_ADJUST );
        }
    }

    m_pDoc->SetTextFormatCollByAutoFormat( *m_aDelPam.GetPoint(), nId, &aSet );
}

static bool HasSelBlanks(
        SwTextFrame const*const pStartFrame, TextFrameIndex & rStartIndex,
        SwTextFrame const*const pEndFrame, TextFrameIndex & rEndIndex)
{
    if (TextFrameIndex(0) < rEndIndex
        && rEndIndex < TextFrameIndex(pEndFrame->GetText().getLength())
        && ' ' == pEndFrame->GetText()[sal_Int32(rEndIndex) - 1])
    {
        --rEndIndex;
        return true;
    }
    if (rStartIndex < TextFrameIndex(pStartFrame->GetText().getLength())
        && ' ' == pStartFrame->GetText()[sal_Int32(rStartIndex)])
    {
        ++rStartIndex;
        return true;
    }
    return false;
}

bool SwAutoFormat::HasBreakAttr(const SwTextFrame& rTextFrame)
{
    const SfxItemSet *const pSet = rTextFrame.GetTextNodeFirst()->GetpSwAttrSet();
    if( !pSet )
        return false;

    const SfxPoolItem* pItem;
    if( SfxItemState::SET == pSet->GetItemState( RES_BREAK, false, &pItem )
        && SvxBreak::NONE != static_cast<const SvxFormatBreakItem*>(pItem)->GetBreak() )
        return true;

    if( SfxItemState::SET == pSet->GetItemState( RES_PAGEDESC, false, &pItem )
        && static_cast<const SwFormatPageDesc*>(pItem)->GetPageDesc()
        && UseOnPage::NONE != static_cast<const SwFormatPageDesc*>(pItem)->GetPageDesc()->GetUseOn() )
        return true;
    return false;
}

/// Is there a dot at the end?
bool SwAutoFormat::IsSentenceAtEnd(const SwTextFrame & rTextFrame)
{
    const OUString& rStr = rTextFrame.GetText();
    sal_Int32 n = rStr.getLength();
    if( !n )
        return true;

    while( --n && IsSpace( rStr[ n ] ) )
        ;
    return '.' == rStr[ n ];
}

/// Delete beginning and/or end in a node
void SwAutoFormat::DeleteLeadingTrailingBlanks(bool bStart, bool bEnd)
{
    if( m_aFlags.bAFormatByInput
        ? m_aFlags.bAFormatByInpDelSpacesAtSttEnd
        : m_aFlags.bAFormatDelSpacesAtSttEnd )
    {
        // delete blanks at the end of the current and at the beginning of the next one
        m_aDelPam.DeleteMark();
        TextFrameIndex nPos(GetLeadingBlanks(m_pCurTextFrame->GetText()));
        if (bStart && TextFrameIndex(0) != nPos)
        {
            *m_aDelPam.GetPoint() = m_pCurTextFrame->MapViewToModelPos(TextFrameIndex(0));
            m_aDelPam.SetMark();
            *m_aDelPam.GetPoint() = m_pCurTextFrame->MapViewToModelPos(nPos);
            DeleteSel( m_aDelPam );
            m_aDelPam.DeleteMark();
        }
        nPos = TextFrameIndex(GetTrailingBlanks(m_pCurTextFrame->GetText()));
        if (bEnd && TextFrameIndex(m_pCurTextFrame->GetText().getLength()) != nPos)
        {
            *m_aDelPam.GetPoint() = m_pCurTextFrame->MapViewToModelPos(
                    TextFrameIndex(m_pCurTextFrame->GetText().getLength()));
            m_aDelPam.SetMark();
            *m_aDelPam.GetPoint() = m_pCurTextFrame->MapViewToModelPos(nPos);
            DeleteSel( m_aDelPam );
            m_aDelPam.DeleteMark();
        }
    }
}

namespace sw {

bool GetRanges(std::vector<std::shared_ptr<SwUnoCursor>> & rRanges,
        SwDoc & rDoc, SwPaM const& rDelPam)
{
    bool isNoRedline(true);
    SwRedlineTable::size_type tmp;
    IDocumentRedlineAccess const& rIDRA(rDoc.getIDocumentRedlineAccess());
    if (!(rIDRA.GetRedlineFlags() & RedlineFlags::ShowDelete))
    {
        return isNoRedline;
    }
    rIDRA.GetRedline(*rDelPam.Start(), &tmp);
    SwPosition const* pCurrent(rDelPam.Start());
    for ( ; tmp < rIDRA.GetRedlineTable().size(); ++tmp)
    {
        SwRangeRedline const*const pRedline(rIDRA.GetRedlineTable()[tmp]);
        if (*rDelPam.End() <= *pRedline->Start())
        {
            break;
        }
        if (*pRedline->End() <= *rDelPam.Start())
        {
            continue;
        }
        if (pRedline->GetType() == nsRedlineType_t::REDLINE_DELETE)
        {
            assert(*pRedline->Start() != *pRedline->End());
            isNoRedline = false;
            if (*pCurrent < *pRedline->Start())
            {
                rRanges.push_back(rDoc.CreateUnoCursor(*pCurrent));
                rRanges.back()->SetMark();
                *rRanges.back()->GetPoint() = *pRedline->Start();
            }
            pCurrent = pRedline->End();
        }
    }
    if (!isNoRedline && *pCurrent < *rDelPam.End())
    {
        rRanges.push_back(rDoc.CreateUnoCursor(*pCurrent));
        rRanges.back()->SetMark();
        *rRanges.back()->GetPoint() = *rDelPam.End();
    }
    return isNoRedline;
}

} // namespace sw

void SwAutoFormat::DeleteSel(SwPaM & rDelPam)
{
    std::vector<std::shared_ptr<SwUnoCursor>> ranges; // need correcting cursor
    if (GetRanges(ranges, *m_pDoc, rDelPam))
    {
        DeleteSelImpl(rDelPam, rDelPam);
    }
    else
    {
        for (auto const& pCursor : ranges)
        {
            DeleteSelImpl(*pCursor, rDelPam);
        }
    }
}

void SwAutoFormat::DeleteSelImpl(SwPaM & rDelPam, SwPaM & rPamToCorrect)
{
    if (m_aFlags.bWithRedlining || &rDelPam != &rPamToCorrect)
    {
        // Add to Shell-Cursor-Ring so that DelPam will be moved as well!
        SwPaM* pShCursor = m_pEditShell->GetCursor_();
        SwPaM aTmp( *m_pCurTextNd, 0, pShCursor );

        SwPaM* pPrev = rPamToCorrect.GetPrev();
        rPamToCorrect.GetRingContainer().merge( pShCursor->GetRingContainer() );

        m_pEditShell->DeleteSel( rDelPam );

        // and remove Pam again:
        SwPaM* p;
        SwPaM* pNext = &rPamToCorrect;
        do {
            p = pNext;
            pNext = p->GetNext();
            p->MoveTo( &rPamToCorrect );
        } while( p != pPrev );

        m_aNdIdx = aTmp.GetPoint()->nNode;
        m_pCurTextNd = m_aNdIdx.GetNode().GetTextNode();
        m_pCurTextFrame = GetFrame(*m_pCurTextNd); // keep it up to date
    }
    else
        m_pEditShell->DeleteSel( rDelPam );
}

bool SwAutoFormat::DeleteJoinCurNextPara(SwTextFrame const*const pNextFrame,
        bool const bIgnoreLeadingBlanks)
{
    // delete blanks at the end of the current and at the beginning of the next one
    m_aDelPam.DeleteMark();
    TextFrameIndex nTrailingPos(GetTrailingBlanks(m_pCurTextFrame->GetText()));

    SwTextFrame const*const pEndFrame(pNextFrame ? pNextFrame : m_pCurTextFrame);
    TextFrameIndex nLeadingPos(0);
    if (pNextFrame)
    {
        nLeadingPos = TextFrameIndex(
            bIgnoreLeadingBlanks ? 0 : GetLeadingBlanks(pNextFrame->GetText()));
    }
    else
    {
        nLeadingPos = TextFrameIndex(m_pCurTextFrame->GetText().getLength());
    }

    // Is there a Blank at the beginning or end?
    // Do not delete it, it will be inserted again.
    bool bHasBlnks = HasSelBlanks(m_pCurTextFrame, nTrailingPos, pEndFrame, nLeadingPos);

    *m_aDelPam.GetPoint() = m_pCurTextFrame->MapViewToModelPos(nTrailingPos);
    m_aDelPam.SetMark();
    *m_aDelPam.GetPoint() = pEndFrame->MapViewToModelPos(nLeadingPos);

    if( *m_aDelPam.GetPoint() != *m_aDelPam.GetMark() )
        DeleteSel( m_aDelPam );
    m_aDelPam.DeleteMark();
    // note: keep m_aDelPam point at insert pos. for clients

    return !bHasBlnks;
}

void SwAutoFormat::DelEmptyLine( bool bTstNextPara )
{
    SetRedlineText( STR_AUTOFMTREDL_DEL_EMPTY_PARA );
    // delete blanks in empty paragraph
    m_aDelPam.DeleteMark();
    *m_aDelPam.GetPoint() = m_pCurTextFrame->MapViewToModelPos(
            TextFrameIndex(m_pCurTextFrame->GetText().getLength()));
    m_aDelPam.SetMark();

    m_aDelPam.GetMark()->nNode = m_pCurTextFrame->GetTextNodeFirst()->GetIndex() - 1;
    SwTextNode* pTNd = m_aDelPam.GetNode( false ).GetTextNode();
    if( pTNd )
        // first use the previous text node
        m_aDelPam.GetMark()->nContent.Assign(pTNd, pTNd->GetText().getLength());
    else if( bTstNextPara )
    {
        // then try the next (at the beginning of a Doc, table cells, frames, ...)
        m_aDelPam.GetMark()->nNode = (m_pCurTextFrame->GetMergedPara()
                    ? m_pCurTextFrame->GetMergedPara()->pLastNode
                    : m_pCurTextNd
                )->GetIndex() + 1;
        pTNd = m_aDelPam.GetNode( false ).GetTextNode();
        if( pTNd )
        {
            m_aDelPam.GetMark()->nContent.Assign( pTNd, 0 );
            *m_aDelPam.GetPoint() = m_pCurTextFrame->MapViewToModelPos(TextFrameIndex(0));
        }
    }
    else
    {
        *m_aDelPam.GetMark() = m_pCurTextFrame->MapViewToModelPos(TextFrameIndex(0));
        pTNd = m_pCurTextNd;
    }
    if( pTNd )
        DeleteSel( m_aDelPam );

    m_aDelPam.DeleteMark();
    ClearRedlineText();
    // note: this likely has deleted m_pCurTextFrame - update it...
    m_pCurTextNd = m_aNdIdx.GetNode().GetTextNode();
    m_pCurTextFrame = m_pCurTextNd ? GetFrame( *m_pCurTextNd ) : nullptr;
}

void SwAutoFormat::DelMoreLinesBlanks( bool bWithLineBreaks )
{
    if( m_aFlags.bAFormatByInput
        ? m_aFlags.bAFormatByInpDelSpacesBetweenLines
        : m_aFlags.bAFormatDelSpacesBetweenLines )
    {
        // delete all blanks on the left and right of the indentation
        m_aDelPam.DeleteMark();

        SwTextFrameInfo aFInfo( m_pCurTextFrame );
        std::vector<std::pair<TextFrameIndex, TextFrameIndex>> spaces;
        aFInfo.GetSpaces(spaces, !m_aFlags.bAFormatByInput || bWithLineBreaks);

        for (auto & rSpaceRange : spaces)
        {
            assert(rSpaceRange.first != rSpaceRange.second);
            bool const bHasBlanks = HasSelBlanks(
                    m_pCurTextFrame, rSpaceRange.first,
                    m_pCurTextFrame, rSpaceRange.second);
            if (rSpaceRange.first != rSpaceRange.second)
            {
                *m_aDelPam.GetPoint() = m_pCurTextFrame->MapViewToModelPos(rSpaceRange.first);
                m_aDelPam.SetMark();
                *m_aDelPam.GetPoint() = m_pCurTextFrame->MapViewToModelPos(rSpaceRange.second);
                DeleteSel(m_aDelPam);
                if (!bHasBlanks)
                {
                    m_pDoc->getIDocumentContentOperations().InsertString(m_aDelPam, OUString(' '));
                }
                m_aDelPam.DeleteMark();
            }
        }
    }
}

void SwAutoFormat::JoinPrevPara()
{
    m_aDelPam.DeleteMark();
    m_aDelPam.GetPoint()->nNode = *m_pCurTextFrame->GetTextNodeFirst();
    m_aDelPam.GetPoint()->nContent.Assign(m_pCurTextFrame->GetTextNodeFirst(), 0);
    m_aDelPam.SetMark();

    --m_aDelPam.GetPoint()->nNode;
    SwTextNode* pTNd = m_aDelPam.GetNode().GetTextNode();
    if( pTNd )
    {
        // use the previous text node first
        m_aDelPam.GetPoint()->nContent.Assign(pTNd, pTNd->GetText().getLength());
        DeleteSel( m_aDelPam );
    }
    m_aDelPam.DeleteMark();
}

void SwAutoFormat::BuildIndent()
{
    SetRedlineText( STR_AUTOFMTREDL_SET_TMPL_INDENT );

    // read all succeeding paragraphs that belong to this indentation
    bool bBreak = true;
    if( m_bMoreLines )
        DelMoreLinesBlanks( true );
    else
        bBreak = !IsFastFullLine(*m_pCurTextFrame)
                || IsBlanksInString(*m_pCurTextFrame)
                || IsSentenceAtEnd(*m_pCurTextFrame);
    SetColl( RES_POOLCOLL_TEXT_IDENT );
    if( !bBreak )
    {
        SetRedlineText( STR_AUTOFMTREDL_DEL_MORELINES );
        const SwTextFrame * pNextFrame = GetNextNode();
        if (pNextFrame && !m_bEnd)
        {
            do {
                bBreak = !IsFastFullLine(*pNextFrame)
                    || IsBlanksInString(*pNextFrame)
                    || IsSentenceAtEnd(*pNextFrame);
                if (DeleteJoinCurNextPara(pNextFrame))
                {
                    m_pDoc->getIDocumentContentOperations().InsertString( m_aDelPam, OUString(' ') );
                }
                if( bBreak )
                    break;
                pNextFrame = GetNextNode();
            }
            while (CanJoin(pNextFrame)
                && !CalcLevel(*pNextFrame));
        }
    }
    DeleteLeadingTrailingBlanks();
    AutoCorrect();
}

void SwAutoFormat::BuildTextIndent()
{
    SetRedlineText( STR_AUTOFMTREDL_SET_TMPL_TEXT_INDENT);
    // read all succeeding paragraphs that belong to this indentation
    bool bBreak = true;
    if( m_bMoreLines )
        DelMoreLinesBlanks( true );
    else
        bBreak = !IsFastFullLine(*m_pCurTextFrame)
               || IsBlanksInString(*m_pCurTextFrame)
               || IsSentenceAtEnd(*m_pCurTextFrame);

    if( m_aFlags.bAFormatByInput )
    {
        const_cast<SwTextNode*>(m_pCurTextFrame->GetTextNodeForParaProps())->SetAutoFormatLvl(
                static_cast<sal_uInt8>(CalcLevel(*m_pCurTextFrame)));
    }

    SetColl( RES_POOLCOLL_TEXT_MOVE );
    if( !bBreak )
    {
        SetRedlineText( STR_AUTOFMTREDL_DEL_MORELINES );
        const SwTextFrame * pNextFrame = GetNextNode();
        while (CanJoin(pNextFrame) &&
               CalcLevel(*pNextFrame))
        {
            bBreak = !IsFastFullLine(*pNextFrame)
                    || IsBlanksInString(*pNextFrame)
                    || IsSentenceAtEnd(*pNextFrame);
            if (DeleteJoinCurNextPara(pNextFrame))
            {
                m_pDoc->getIDocumentContentOperations().InsertString( m_aDelPam, OUString(' ') );
            }
            if( bBreak )
                break;
            pNextFrame = GetNextNode();
        }
    }
    DeleteLeadingTrailingBlanks();
    AutoCorrect();
}

void SwAutoFormat::BuildText()
{
    SetRedlineText( STR_AUTOFMTREDL_SET_TMPL_TEXT );
    // read all succeeding paragraphs that belong to this text without indentation
    bool bBreak = true;
    if( m_bMoreLines )
        DelMoreLinesBlanks();
    else
        bBreak = !IsFastFullLine(*m_pCurTextFrame)
                || IsBlanksInString(*m_pCurTextFrame)
                || IsSentenceAtEnd(*m_pCurTextFrame);
    SetColl( RES_POOLCOLL_TEXT, true );
    if( !bBreak )
    {
        SetRedlineText( STR_AUTOFMTREDL_DEL_MORELINES );
        const SwTextFrame * pNextFrame = GetNextNode();
        while (CanJoin(pNextFrame) &&
               !CalcLevel(*pNextFrame))
        {
            bBreak = !IsFastFullLine(*pNextFrame)
                    || IsBlanksInString(*pNextFrame)
                    || IsSentenceAtEnd(*pNextFrame);
            if (DeleteJoinCurNextPara(pNextFrame))
            {
                m_pDoc->getIDocumentContentOperations().InsertString( m_aDelPam, OUString(' ') );
            }
            if( bBreak )
                break;
            const SwTextFrame *const pCurrNode = pNextFrame;
            pNextFrame = GetNextNode();
            if (!pNextFrame || pCurrNode == pNextFrame)
                break;
        }
    }
    DeleteLeadingTrailingBlanks();
    AutoCorrect();
}

void SwAutoFormat::BuildEnum( sal_uInt16 nLvl, sal_uInt16 nDigitLevel )
{
    SetRedlineText( STR_AUTOFMTREDL_SET_NUMBULLET );

    bool bBreak = true;

    // first, determine current indentation and frame width
    SwTwips nFrameWidth = m_pCurTextFrame->getFramePrintArea().Width();
    SwTwips nLeftTextPos;
    {
        TextFrameIndex nPos(0);
        while (nPos < TextFrameIndex(m_pCurTextFrame->GetText().getLength())
               && IsSpace(m_pCurTextFrame->GetText()[sal_Int32(nPos)]))
        {
            ++nPos;
        }

        SwTextFrameInfo aInfo( m_pCurTextFrame );
        nLeftTextPos = aInfo.GetCharPos(nPos);
        nLeftTextPos -= m_pCurTextFrame->GetTextNodeForParaProps()->GetSwAttrSet().GetLRSpace().GetLeft();
    }

    if( m_bMoreLines )
        DelMoreLinesBlanks();
    else
        bBreak = !IsFastFullLine(*m_pCurTextFrame)
                || IsBlanksInString(*m_pCurTextFrame)
                || IsSentenceAtEnd(*m_pCurTextFrame);
    bool bRTL = m_pEditShell->IsInRightToLeftText();
    DeleteLeadingTrailingBlanks();

    bool bChgBullet = false, bChgEnum = false;
    TextFrameIndex nAutoCorrPos(0);

    // if numbering is set, get the current one
    SwNumRule aRule( m_pDoc->GetUniqueNumRuleName(),
                     // #i89178#
                     numfunc::GetDefaultPositionAndSpaceMode() );

    const SwNumRule* pCur = nullptr;
    if (m_aFlags.bSetNumRule)
    {
        pCur = m_pCurTextFrame->GetTextNodeForParaProps()->GetNumRule();
        if (pCur)
        {
            aRule = *pCur;
        }
    }

    // replace bullet character with defined one
    const OUString& rStr = m_pCurTextFrame->GetText();
    TextFrameIndex nTextStt(0);
    const sal_Unicode* pFndBulletChr = nullptr;
    if (m_aFlags.bChgEnumNum && 2 < rStr.getLength())
        pFndBulletChr = StrChr(pBulletChar, rStr[sal_Int32(nTextStt)]);
    if (nullptr != pFndBulletChr && IsSpace(rStr[sal_Int32(nTextStt) + 1]))
    {
        if( m_aFlags.bAFormatByInput )
        {
            if( m_aFlags.bSetNumRule )
            {
                SwCharFormat* pCFormat = m_pDoc->getIDocumentStylePoolAccess().GetCharFormatFromPool(
                                            RES_POOLCHR_BUL_LEVEL );
                bChgBullet = true;
                // Was the format already somewhere adjusted?
                if( !aRule.GetNumFormat( nLvl ) )
                {
                    int nBulletPos = pFndBulletChr - pBulletChar;
                    sal_Unicode cBullChar;
                    const vcl::Font* pBullFnt( nullptr );
                    if( nBulletPos < cnPosEnDash )
                    {
                        cBullChar = m_aFlags.cBullet;
                        pBullFnt = &m_aFlags.aBulletFont;
                    }
                    else
                    {
                        cBullChar = nBulletPos < cnPosEmDash
                                        ? cStarSymbolEnDash
                                        : cStarSymbolEmDash;
                        // #i63395#
                        // Only apply user defined default bullet font
                        if ( numfunc::IsDefBulletFontUserDefined() )
                        {
                            pBullFnt = &numfunc::GetDefBulletFont();
                        }
                    }

                    sal_Int32 nAbsPos = lBullIndent;
                    SwTwips nSpaceSteps = nLvl
                                            ? nLeftTextPos / nLvl
                                            : lBullIndent;
                    for( sal_uInt8 n = 0; n < MAXLEVEL; ++n, nAbsPos = nAbsPos + nSpaceSteps )
                    {
                        SwNumFormat aFormat( aRule.Get( n ) );
                        aFormat.SetBulletFont( pBullFnt );
                        aFormat.SetBulletChar( cBullChar );
                        aFormat.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
                        // #i93908# clear suffix for bullet lists
                        aFormat.SetPrefix(OUString());
                        aFormat.SetSuffix(OUString());
                        aFormat.SetFirstLineOffset( lBullFirstLineOffset );
                        aFormat.SetAbsLSpace( nAbsPos );
                        if( !aFormat.GetCharFormat() )
                            aFormat.SetCharFormat( pCFormat );
                        if( bRTL )
                            aFormat.SetNumAdjust( SvxAdjust::Right );

                        aRule.Set( n, aFormat );

                        if( n == nLvl &&
                            nFrameWidth < ( nSpaceSteps * MAXLEVEL ) )
                            nSpaceSteps = ( nFrameWidth - nLeftTextPos ) /
                                                ( MAXLEVEL - nLvl );
                    }
                }
            }
        }
        else
        {
            bChgBullet = true;
            SetColl( static_cast<sal_uInt16>(RES_POOLCOLL_BUL_LEVEL1 + ( std::min( nLvl, cnNumBullColls ) * 4 )) );
        }
    }
    else
    {
        // Then it is a numbering

        //JP 21.11.97: The NumLevel is either the DigitLevel or, if the latter is not existent or 0,
        //             it is determined by the indentation level.

        OUString aPostfix, aPrefix, aNumTypes;
        nDigitLevel = GetDigitLevel(*m_pCurTextFrame, nTextStt,
                                            &aPrefix, &aPostfix, &aNumTypes);
        if (USHRT_MAX != nDigitLevel)
        {
            bChgEnum = true;

            // Level 0 and Indentation, determine level by left indentation and default NumIndent
            if( !nDigitLevel && nLeftTextPos )
                nLvl = std::min( sal_uInt16( nLeftTextPos / lNumIndent ),
                            sal_uInt16( MAXLEVEL - 1 ) );
            else
                nLvl = nDigitLevel;
        }

        if( bChgEnum && m_aFlags.bSetNumRule )
        {
            if( !pCur )         // adjust NumRule if it is new
            {
                SwCharFormat* pCFormat = m_pDoc->getIDocumentStylePoolAccess().GetCharFormatFromPool(
                                            RES_POOLCHR_NUM_LEVEL );

                sal_Int32 nPrefixIdx{ 0 };
                if( !nDigitLevel )
                {
                    SwNumFormat aFormat( aRule.Get( nLvl ) );
                    aFormat.SetPrefix( aPrefix.getToken( 0, u'\x0001', nPrefixIdx ));
                    aFormat.SetStart( static_cast<sal_uInt16>(aPrefix.getToken( 0, u'\x0001', nPrefixIdx ).toInt32()));
                    aFormat.SetSuffix( aPostfix.getToken( 0, u'\x0001' ));
                    aFormat.SetIncludeUpperLevels( 0 );

                    if( !aFormat.GetCharFormat() )
                        aFormat.SetCharFormat( pCFormat );

                    if( !aNumTypes.isEmpty() )
                        aFormat.SetNumberingType(static_cast<SvxNumType>(aNumTypes[ 0 ] - '0'));

                    if( bRTL )
                        aFormat.SetNumAdjust( SvxAdjust::Right );
                    aRule.Set( nLvl, aFormat );
                }
                else
                {
                    auto const nSpaceSteps = nLvl ? nLeftTextPos / nLvl : 0;
                    sal_uInt16 n;
                    sal_Int32 nPostfixIdx{ 0 };
                    for( n = 0; n <= nLvl; ++n )
                    {
                        SwNumFormat aFormat( aRule.Get( n ) );

                        if( !n )
                            aFormat.SetPrefix( aPrefix.getToken( 0, u'\x0001', nPrefixIdx )); // token 0, read only on first loop
                        aFormat.SetStart( static_cast<sal_uInt16>(aPrefix.getToken( 0, u'\x0001', nPrefixIdx ).toInt32() ));
                        aFormat.SetSuffix( aPostfix.getToken( 0, u'\x0001', nPostfixIdx ));
                        aFormat.SetIncludeUpperLevels( MAXLEVEL );
                        if( n < aNumTypes.getLength() )
                            aFormat.SetNumberingType(static_cast<SvxNumType>(aNumTypes[ n ] - '0'));

                        aFormat.SetAbsLSpace( nSpaceSteps * n
                                            + lNumIndent );

                        if( !aFormat.GetCharFormat() )
                            aFormat.SetCharFormat( pCFormat );
                        if( bRTL )
                            aFormat.SetNumAdjust( SvxAdjust::Right );

                        aRule.Set( n, aFormat );
                    }

                    // Does it fit completely into the frame?
                    bool bDefStep = nFrameWidth < (nSpaceSteps * MAXLEVEL);
                    for( ; n < MAXLEVEL; ++n )
                    {
                        SwNumFormat aFormat( aRule.Get( n ) );
                        aFormat.SetIncludeUpperLevels( MAXLEVEL );
                        if( bDefStep )
                            aFormat.SetAbsLSpace( nLeftTextPos +
                                SwNumRule::GetNumIndent(static_cast<sal_uInt8>(n-nLvl)));
                        else
                            aFormat.SetAbsLSpace( nSpaceSteps * n
                                                + lNumIndent );
                        aRule.Set( n, aFormat );
                    }
                }
            }
        }
        else if( !m_aFlags.bAFormatByInput )
            SetColl( static_cast<sal_uInt16>(RES_POOLCOLL_NUM_LEVEL1 + ( std::min( nLvl, cnNumBullColls ) * 4 ) ));
        else
            bChgEnum = false;
    }

    if ( bChgEnum || bChgBullet )
    {
        m_aDelPam.DeleteMark();
        m_aDelPam.GetPoint()->nNode = *m_pCurTextFrame->GetTextNodeForParaProps();

        if( m_aFlags.bSetNumRule )
        {
            if( m_aFlags.bAFormatByInput )
            {
                m_aDelPam.SetMark();
                SwTextFrame const*const pNextFrame = GetNextNode(false);
                assert(pNextFrame);
                m_aDelPam.GetMark()->nNode = *pNextFrame->GetTextNodeForParaProps();
                m_aDelPam.GetNode(false).GetTextNode()->SetAttrListLevel( nLvl );
            }

            const_cast<SwTextNode*>(m_pCurTextFrame->GetTextNodeForParaProps())->SetAttrListLevel(nLvl);

            // start new list
            m_pDoc->SetNumRule(m_aDelPam, aRule, true, m_pEditShell->GetLayout());
            m_aDelPam.DeleteMark();

            *m_aDelPam.GetPoint() = m_pCurTextFrame->MapViewToModelPos(TextFrameIndex(0));
        }
        else
        {
            *m_aDelPam.GetPoint() = m_pCurTextFrame->MapViewToModelPos(
                        bChgEnum ? nTextStt : TextFrameIndex(0));
        }
        m_aDelPam.SetMark();

        if ( bChgBullet )
            nTextStt += TextFrameIndex(2);

        while (nTextStt < TextFrameIndex(rStr.getLength()) && IsSpace(rStr[sal_Int32(nTextStt)]))
            nTextStt++;

        *m_aDelPam.GetPoint() = m_pCurTextFrame->MapViewToModelPos(nTextStt);
        DeleteSel( m_aDelPam );

        if( !m_aFlags.bSetNumRule )
        {
            OUString sChgStr('\t');
            if( bChgBullet )
                sChgStr = OUStringLiteral1( m_aFlags.cBullet ) + sChgStr;
            m_pDoc->getIDocumentContentOperations().InsertString( m_aDelPam, sChgStr );

            SfxItemSet aSet( m_pDoc->GetAttrPool(), aTextNodeSetRange );
            *m_aDelPam.GetPoint() = m_pCurTextFrame->MapViewToModelPos(TextFrameIndex(0));
            assert(&m_aDelPam.GetPoint()->nNode.GetNode() == m_pCurTextFrame->GetTextNodeForParaProps());
            if( bChgBullet )
            {
                m_aDelPam.SetMark();
                *m_aDelPam.GetPoint() = m_pCurTextFrame->MapViewToModelPos(TextFrameIndex(1));
                SetAllScriptItem( aSet,
                     SvxFontItem( m_aFlags.aBulletFont.GetFamilyType(),
                                  m_aFlags.aBulletFont.GetFamilyName(),
                                  m_aFlags.aBulletFont.GetStyleName(),
                                  m_aFlags.aBulletFont.GetPitch(),
                                  m_aFlags.aBulletFont.GetCharSet(),
                                  RES_CHRATR_FONT ) );
                m_pDoc->SetFormatItemByAutoFormat( m_aDelPam, aSet );
                m_aDelPam.DeleteMark();
                nAutoCorrPos = TextFrameIndex(2);
                aSet.ClearItem();
            }
            SvxTabStopItem aTStops( RES_PARATR_TABSTOP );
            aTStops.Insert( SvxTabStop( 0 ) );
            aSet.Put( aTStops );
            assert(&m_aDelPam.GetPoint()->nNode.GetNode() == m_pCurTextFrame->GetTextNodeForParaProps());
            m_pDoc->SetFormatItemByAutoFormat( m_aDelPam, aSet );
        }
    }

    if( bBreak )
    {
        AutoCorrect( nAutoCorrPos );       /* Offset due to Bullet + Tab */
        return;
    }

    const SwTextFrame * pNextFrame = GetNextNode();
    while (CanJoin(pNextFrame)
            && nLvl == CalcLevel(*pNextFrame))
    {
        SetRedlineText( STR_AUTOFMTREDL_DEL_MORELINES );
        bBreak = !IsFastFullLine(*pNextFrame)
                || IsBlanksInString(*pNextFrame)
                || IsSentenceAtEnd(*pNextFrame);
        if (DeleteJoinCurNextPara(pNextFrame))
        {
            m_pDoc->getIDocumentContentOperations().InsertString( m_aDelPam, OUString(' ') );
        }
        if( bBreak )
            break;
        const SwTextFrame *const pCurrNode = pNextFrame;
        pNextFrame = GetNextNode();
        if (!pNextFrame || pCurrNode == pNextFrame)
            break;
    }
    DeleteLeadingTrailingBlanks( false );
    AutoCorrect( nAutoCorrPos );
}

void SwAutoFormat::BuildNegIndent( SwTwips nSpaces )
{
    SetRedlineText( STR_AUTOFMTREDL_SET_TMPL_NEG_INDENT );
    // Test of contraposition (n words, divided by spaces/tabs, with same indentation in 2nd line)

    // read all succeeding paragraphs that belong to this enumeration
    bool bBreak = true;
    TextFrameIndex nSpacePos(0);
    const sal_Int32 nTextPos = GetBigIndent( nSpacePos );
    if( m_bMoreLines )
        DelMoreLinesBlanks( true );
    else
        bBreak = !IsFastFullLine(*m_pCurTextFrame)
            || (!nTextPos && IsBlanksInString(*m_pCurTextFrame))
            || IsSentenceAtEnd(*m_pCurTextFrame);

    SetColl( static_cast<sal_uInt16>( nTextPos
                ? RES_POOLCOLL_CONFRONTATION
                : RES_POOLCOLL_TEXT_NEGIDENT ) );

    if( nTextPos )
    {
        const OUString& rStr = m_pCurTextFrame->GetText();
        bool bInsTab = true;

        if ('\t' == rStr[sal_Int32(nSpacePos) + 1]) // leave tab alone
        {
            --nSpacePos;
            bInsTab = false;
        }

        TextFrameIndex nSpaceStt = nSpacePos;
        while (nSpaceStt && IsSpace(rStr[sal_Int32(--nSpaceStt)]))
            ;
        ++nSpaceStt;

        if (bInsTab && '\t' == rStr[sal_Int32(nSpaceStt)]) // leave tab alone
        {
            ++nSpaceStt;
            bInsTab = false;
        }

        m_aDelPam.DeleteMark();
        *m_aDelPam.GetPoint() = m_pCurTextFrame->MapViewToModelPos(nSpacePos);

        // delete old Spaces, etc.
        if( nSpaceStt < nSpacePos )
        {
            m_aDelPam.SetMark();
            *m_aDelPam.GetMark() = m_pCurTextFrame->MapViewToModelPos(nSpaceStt);
            DeleteSel( m_aDelPam );
            if( bInsTab )
            {
                m_pDoc->getIDocumentContentOperations().InsertString( m_aDelPam, OUString('\t') );
            }
        }
    }

    if( !bBreak )
    {
        SetRedlineText( STR_AUTOFMTREDL_DEL_MORELINES );
        SwTextFrameInfo aFInfo( m_pCurTextFrame );
        const SwTextFrame * pNextFrame = GetNextNode();
        while (CanJoin(pNextFrame) &&
                20 < std::abs( static_cast<long>(nSpaces - aFInfo.SetFrame(
                                EnsureFormatted(*pNextFrame)).GetLineStart()) )
            )
        {
            bBreak = !IsFastFullLine(*pNextFrame)
                   || IsBlanksInString(*pNextFrame)
                   || IsSentenceAtEnd(*pNextFrame);
            if (DeleteJoinCurNextPara(pNextFrame))
            {
                m_pDoc->getIDocumentContentOperations().InsertString( m_aDelPam, OUString(' ') );
            }
            if( bBreak )
                break;
            pNextFrame = GetNextNode();
        }
    }
    DeleteLeadingTrailingBlanks();
    AutoCorrect();
}

void SwAutoFormat::BuildHeadLine( sal_uInt16 nLvl )
{
    if( m_aFlags.bWithRedlining )
    {
        OUString sText(SwViewShell::GetShellRes()->GetAutoFormatNameLst()[
                                    STR_AUTOFMTREDL_SET_TMPL_HEADLINE ] );
        sText = sText.replaceAll( "$(ARG1)", OUString::number( nLvl + 1 ) );
        m_pDoc->GetDocumentRedlineManager().SetAutoFormatRedlineComment( &sText );
    }

    SetColl( static_cast<sal_uInt16>(RES_POOLCOLL_HEADLINE1 + nLvl ), true );
    if( m_aFlags.bAFormatByInput )
    {
        SwTextFormatColl& rNxtColl = m_pCurTextFrame->GetTextNodeForParaProps()->GetTextColl()->GetNextTextFormatColl();

        JoinPrevPara();

        DeleteLeadingTrailingBlanks( true, false );
        const SwTextFrame *const pNextFrame = GetNextNode(false);
        (void)DeleteJoinCurNextPara(pNextFrame, true);

        m_aDelPam.DeleteMark();
        m_aDelPam.GetPoint()->nNode = *GetNextNode(false)->GetTextNodeForParaProps();
        m_aDelPam.GetPoint()->nContent.Assign( m_aDelPam.GetContentNode(), 0 );
        m_pDoc->SetTextFormatColl( m_aDelPam, &rNxtColl );
    }
    else
    {
        DeleteLeadingTrailingBlanks();
        AutoCorrect();
    }
}

/// Start autocorrection for the current TextNode
void SwAutoFormat::AutoCorrect(TextFrameIndex nPos)
{
    SvxAutoCorrect* pATst = SvxAutoCorrCfg::Get().GetAutoCorrect();
    ACFlags aSvxFlags = pATst->GetFlags( );
    bool bReplaceQuote( aSvxFlags & ACFlags::ChgQuotes );
    bool bReplaceSglQuote( aSvxFlags & ACFlags::ChgSglQuotes );

    if( m_aFlags.bAFormatByInput ||
        (!m_aFlags.bAutoCorrect && !bReplaceQuote && !bReplaceSglQuote &&
        !m_aFlags.bCapitalStartSentence && !m_aFlags.bCapitalStartWord &&
        !m_aFlags.bChgOrdinalNumber &&
        !m_aFlags.bChgToEnEmDash && !m_aFlags.bSetINetAttr &&
        !m_aFlags.bChgWeightUnderl && !m_aFlags.bAddNonBrkSpace) )
        return;

    const OUString* pText = &m_pCurTextFrame->GetText();
    if (TextFrameIndex(pText->getLength()) <= nPos)
        return;

    bool bGetLanguage = m_aFlags.bChgOrdinalNumber ||
                        m_aFlags.bChgToEnEmDash || m_aFlags.bSetINetAttr ||
                        m_aFlags.bCapitalStartWord || m_aFlags.bCapitalStartSentence ||
                        m_aFlags.bAddNonBrkSpace;

    m_aDelPam.DeleteMark();
    *m_aDelPam.GetPoint() = m_pCurTextFrame->MapViewToModelPos(TextFrameIndex(0));

    SwAutoCorrDoc aACorrDoc( *m_pEditShell, m_aDelPam );

    SwTextFrameInfo aFInfo( nullptr );

    TextFrameIndex nSttPos, nLastBlank = nPos;
    bool bFirst = m_aFlags.bCapitalStartSentence, bFirstSent = bFirst;
    sal_Unicode cChar = 0;
    bool bNbspRunNext = false;

    CharClass& rAppCC = GetAppCharClass();

    do {
        while (nPos < TextFrameIndex(pText->getLength())
                && IsSpace(cChar = (*pText)[sal_Int32(nPos)]))
            ++nPos;
        if (nPos == TextFrameIndex(pText->getLength()))
            break;      // that's it

        if( ( ( bReplaceQuote && '\"' == cChar ) ||
              ( bReplaceSglQuote && '\'' == cChar ) ) &&
            (!nPos || ' ' == (*pText)[sal_Int32(nPos)-1]))
        {

            // note: special case symbol fonts !!!
            if( !aFInfo.GetFrame() )
                aFInfo.SetFrame( GetFrame( *m_pCurTextNd ) );
            if( !aFInfo.IsBullet( nPos ))
            {
                SetRedlineText( STR_AUTOFMTREDL_TYPO );
                *m_aDelPam.GetPoint() = m_pCurTextFrame->MapViewToModelPos(nPos);
                bool bSetHardBlank = false;

                OUString sReplace( pATst->GetQuote( aACorrDoc,
                                    sal_Int32(nPos), cChar, true ));

                m_aDelPam.SetMark();
                m_aDelPam.GetPoint()->nContent = m_aDelPam.GetMark()->nContent.GetIndex() + 1;
                if( 2 == sReplace.getLength() && ' ' == sReplace[ 1 ])
                {
                    sReplace = sReplace.copy( 0, 1 );
                    bSetHardBlank = true;
                }
                m_pDoc->getIDocumentContentOperations().ReplaceRange( m_aDelPam, sReplace, false );

                if( m_aFlags.bWithRedlining )
                {
                    m_aNdIdx = m_aDelPam.GetPoint()->nNode;
                    m_pCurTextNd = m_aNdIdx.GetNode().GetTextNode();
                    m_pCurTextFrame = GetFrame( *m_pCurTextNd );
                    pText = &m_pCurTextFrame->GetText();
                    m_aDelPam.SetMark();
                    aFInfo.SetFrame( nullptr );
                }

                nPos += TextFrameIndex(sReplace.getLength() - 1);
                m_aDelPam.DeleteMark();
                if( bSetHardBlank )
                {
                    m_pDoc->getIDocumentContentOperations().InsertString( m_aDelPam, OUString(CHAR_HARDBLANK) );
                    ++nPos;
                }
            }
        }

        bool bCallACorr = false;
        int bBreak = 0;
        if (nPos && IsSpace((*pText)[sal_Int32(nPos) - 1]))
            nLastBlank = nPos;
        for (nSttPos = nPos; !bBreak && nPos < TextFrameIndex(pText->getLength()); ++nPos)
        {
            cChar = (*pText)[sal_Int32(nPos)];
            switch (cChar)
            {
            case '\"':
            case '\'':
                if( ( cChar == '\"' && bReplaceQuote ) || ( cChar == '\'' && bReplaceSglQuote ) )
                {
                    // consider Symbolfonts!
                    if( !aFInfo.GetFrame() )
                        aFInfo.SetFrame( GetFrame( *m_pCurTextNd ) );
                    if( !aFInfo.IsBullet( nPos ))
                    {
                        SetRedlineText( STR_AUTOFMTREDL_TYPO );
                        bool bSetHardBlank = false;
                        *m_aDelPam.GetPoint() = m_pCurTextFrame->MapViewToModelPos(nPos);
                        OUString sReplace( pATst->GetQuote( aACorrDoc,
                                            sal_Int32(nPos), cChar, false) );

                        if( 2 == sReplace.getLength() && ' ' == sReplace[ 0 ])
                        {
                            sReplace = sReplace.copy( 1 );
                            bSetHardBlank = true;
                        }

                        m_aDelPam.SetMark();
                        m_aDelPam.GetPoint()->nContent = m_aDelPam.GetMark()->nContent.GetIndex() + 1;
                        m_pDoc->getIDocumentContentOperations().ReplaceRange( m_aDelPam, sReplace, false );

                        if( m_aFlags.bWithRedlining )
                        {
                            m_aNdIdx = m_aDelPam.GetPoint()->nNode;
                            m_pCurTextNd = m_aNdIdx.GetNode().GetTextNode();
                            m_pCurTextFrame = GetFrame( *m_pCurTextNd );
                            pText = &m_pCurTextFrame->GetText();
                            m_aDelPam.SetMark();
                            m_aDelPam.DeleteMark();
                            aFInfo.SetFrame( nullptr );
                        }

                        nPos += TextFrameIndex(sReplace.getLength() - 1);
                        m_aDelPam.DeleteMark();

                        if( bSetHardBlank )
                        {
                            *m_aDelPam.GetPoint() = m_pCurTextFrame->MapViewToModelPos(nPos);
                            m_pDoc->getIDocumentContentOperations().InsertString( m_aDelPam, OUString(CHAR_HARDBLANK) );
                            ++nPos;
                            *m_aDelPam.GetPoint() = m_pCurTextFrame->MapViewToModelPos(nPos);
                        }
                    }
                }
                break;
            case '*':
            case '_':
                if( m_aFlags.bChgWeightUnderl )
                {
                    // consider Symbolfonts!
                    if( !aFInfo.GetFrame() )
                        aFInfo.SetFrame( GetFrame( *m_pCurTextNd ) );
                    if( !aFInfo.IsBullet( nPos ))
                    {
                        SetRedlineText( '*' == cChar
                                            ? STR_AUTOFMTREDL_BOLD
                                            : STR_AUTOFMTREDL_UNDER );

                        sal_Unicode cBlank = nSttPos ? (*pText)[sal_Int32(nSttPos) - 1] : 0;
                        *m_aDelPam.GetPoint() = m_pCurTextFrame->MapViewToModelPos(nPos);

                        if (pATst->FnChgWeightUnderl(aACorrDoc, *pText, sal_Int32(nPos)))
                        {
                            if( m_aFlags.bWithRedlining )
                            {
                                m_aNdIdx = m_aDelPam.GetPoint()->nNode;
                                m_pCurTextNd = m_aNdIdx.GetNode().GetTextNode();
                                m_pCurTextFrame = GetFrame( *m_pCurTextNd );
                                pText = &m_pCurTextFrame->GetText();
                                m_aDelPam.SetMark();
                                m_aDelPam.DeleteMark();
                                aFInfo.SetFrame( nullptr );
                            }
                            //#125102# in case of the mode RedlineFlags::ShowDelete the ** are still contained in pText
                            if(!(m_pDoc->getIDocumentRedlineAccess().GetRedlineFlags() & RedlineFlags::ShowDelete))
                                nPos = m_pCurTextFrame->MapModelToViewPos(*m_aDelPam.GetPoint()) - TextFrameIndex(1);
                            // Was a character deleted before starting?
                            if (cBlank && cBlank != (*pText)[sal_Int32(nSttPos) - 1])
                                --nSttPos;
                        }
                    }
                }
                break;
            case '/':
                if ( m_aFlags.bAddNonBrkSpace )
                {
                    LanguageType eLang = bGetLanguage
                        ? m_pCurTextFrame->GetLangOfChar(nSttPos, 0, true)
                        : LANGUAGE_SYSTEM;

                    SetRedlineText( STR_AUTOFMTREDL_NON_BREAK_SPACE );
                    if (pATst->FnAddNonBrkSpace(aACorrDoc, *pText, sal_Int32(nPos), eLang, bNbspRunNext))
                        --nPos;
                }
                break;

            case '.':
            case '!':
            case '?':
                if( m_aFlags.bCapitalStartSentence )
                    bFirstSent = true;
                [[fallthrough]];
            default:
                if (!(rAppCC.isLetterNumeric(*pText, sal_Int32(nPos))
                        || '/' == cChar )) //  '/' should not be a word separator (e.g. '1/2' needs to be handled as one word for replacement)
                {
                    --nPos;     // revert ++nPos which was decremented in for loop
                    ++bBreak;
                }
                break;
            }
        }

        if( nPos == nSttPos )
        {
            if (++nPos == TextFrameIndex(pText->getLength()))
                bCallACorr = true;
        }
        else
            bCallACorr = true;

        if( bCallACorr )
        {
            *m_aDelPam.GetPoint() = m_pCurTextFrame->MapViewToModelPos(nPos);
            SetRedlineText( STR_AUTOFMTREDL_USE_REPLACE );
            if( m_aFlags.bAutoCorrect &&
                aACorrDoc.ChgAutoCorrWord(reinterpret_cast<sal_Int32&>(nSttPos), sal_Int32(nPos), *pATst, nullptr))
            {
                nPos = m_pCurTextFrame->MapModelToViewPos(*m_aDelPam.GetPoint()) - TextFrameIndex(1);

                if( m_aFlags.bWithRedlining )
                {
                    m_aNdIdx = m_aDelPam.GetPoint()->nNode;
                    m_pCurTextNd = m_aNdIdx.GetNode().GetTextNode();
                    m_pCurTextFrame = GetFrame( *m_pCurTextNd );
                    pText = &m_pCurTextFrame->GetText();
                    m_aDelPam.SetMark();
                    m_aDelPam.DeleteMark();
                }

                continue;       // do not check further
            }

            LanguageType eLang = bGetLanguage
                    ? m_pCurTextFrame->GetLangOfChar(nSttPos, 0, true)
                    : LANGUAGE_SYSTEM;

            if ( m_aFlags.bAddNonBrkSpace )
            {
                SetRedlineText( STR_AUTOFMTREDL_NON_BREAK_SPACE );
                pATst->FnAddNonBrkSpace(aACorrDoc, *pText, sal_Int32(nPos), eLang, bNbspRunNext);
            }

            if( ( m_aFlags.bChgOrdinalNumber &&
                    SetRedlineText( STR_AUTOFMTREDL_ORDINAL ) &&
                    pATst->FnChgOrdinalNumber(aACorrDoc, *pText, sal_Int32(nSttPos), sal_Int32(nPos), eLang)) ||
                ( m_aFlags.bChgToEnEmDash &&
                    SetRedlineText( STR_AUTOFMTREDL_DASH ) &&
                    pATst->FnChgToEnEmDash(aACorrDoc, *pText, sal_Int32(nSttPos), sal_Int32(nPos), eLang)) ||
                ( m_aFlags.bSetINetAttr &&
                    (nPos == TextFrameIndex(pText->getLength()) || IsSpace((*pText)[sal_Int32(nPos)])) &&
                    SetRedlineText( STR_AUTOFMTREDL_DETECT_URL ) &&
                    pATst->FnSetINetAttr(aACorrDoc, *pText, sal_Int32(nLastBlank), sal_Int32(nPos), eLang)))
            {
                nPos = m_pCurTextFrame->MapModelToViewPos(*m_aDelPam.GetPoint()) - TextFrameIndex(1);
            }
            else
            {
                // two capital letters at the beginning of a word?
                if( m_aFlags.bCapitalStartWord )
                {
                    SetRedlineText( STR_AUTOFMTREDL_CPTL_STT_WORD );
                    pATst->FnCapitalStartWord(aACorrDoc, *pText, sal_Int32(nSttPos), sal_Int32(nPos), eLang);
                }
                // capital letter at the beginning of a sentence?
                if( m_aFlags.bCapitalStartSentence && bFirst )
                {
                    SetRedlineText( STR_AUTOFMTREDL_CPTL_STT_SENT );
                    pATst->FnCapitalStartSentence(aACorrDoc, *pText, true, sal_Int32(nSttPos), sal_Int32(nPos), eLang);
                }

                bFirst = bFirstSent;
                bFirstSent = false;

                if( m_aFlags.bWithRedlining )
                {
                    m_aNdIdx = m_aDelPam.GetPoint()->nNode;
                    m_pCurTextNd = m_aNdIdx.GetNode().GetTextNode();
                    m_pCurTextFrame = GetFrame( *m_pCurTextNd );
                    pText = &m_pCurTextFrame->GetText();
                    m_aDelPam.SetMark();
                    m_aDelPam.DeleteMark();
                }
            }
        }
    }
    while (nPos < TextFrameIndex(pText->getLength()));
    ClearRedlineText();
}

SwAutoFormat::SwAutoFormat( SwEditShell* pEdShell, SvxSwAutoFormatFlags const & rFlags,
                            SwNodeIndex const * pSttNd, SwNodeIndex const * pEndNd )
    : m_aFlags( rFlags ),
    m_aDelPam( pEdShell->GetDoc()->GetNodes().GetEndOfExtras() ),
    m_aNdIdx( pEdShell->GetDoc()->GetNodes().GetEndOfExtras(), +1 ),
    m_aEndNdIdx( pEdShell->GetDoc()->GetNodes().GetEndOfContent() ),
    m_pEditShell( pEdShell ),
    m_pDoc( pEdShell->GetDoc() ),
    m_pCurTextNd( nullptr ), m_pCurTextFrame( nullptr ),
    m_nRedlAutoFormatSeqId( 0 )
{
    OSL_ENSURE( (pSttNd && pEndNd) || (!pSttNd && !pEndNd),
            "Got no area" );

    if( m_aFlags.bSetNumRule && !m_aFlags.bAFormatByInput )
        m_aFlags.bSetNumRule = false;

    bool bReplaceStyles = !m_aFlags.bAFormatByInput || m_aFlags.bReplaceStyles;

    const SwTextFrame * pNextFrame = nullptr;
    bool bNxtEmpty = false;
    bool bNxtAlpha = false;
    sal_uInt16 nNxtLevel = 0;
    bool bEmptyLine;

    // set area for autoformatting
    if( pSttNd )
    {
        m_aNdIdx = *pSttNd;
        // for GoNextPara, one paragraph prior to that
        sw::GotoPrevLayoutTextFrame(m_aNdIdx, m_pEditShell->GetLayout());
        m_aEndNdIdx = *pEndNd;
        sw::GotoNextLayoutTextFrame(m_aEndNdIdx, m_pEditShell->GetLayout());

        // check the previous TextNode
        SwTextFrame const*const pPrevFrame = m_aNdIdx.GetNode().GetTextNode()
            ? static_cast<SwTextFrame const*>(m_aNdIdx.GetNode().GetTextNode()->getLayoutFrame(m_pEditShell->GetLayout()))
            : nullptr;
        bEmptyLine = !pPrevFrame
                    || IsEmptyLine(*pPrevFrame)
                    || IsNoAlphaLine(*pPrevFrame);
    }
    else
        bEmptyLine = true;      // at document beginning

    m_bEnd = false;

    // set value for percentage display
    m_nEndNdIdx = m_aEndNdIdx.GetIndex();

    if( !m_aFlags.bAFormatByInput )
    {
        m_nEndNdIdx = m_aEndNdIdx.GetIndex();
        ::StartProgress( STR_STATSTR_AUTOFORMAT, m_aNdIdx.GetIndex(),
                         m_nEndNdIdx,
                         m_pDoc->GetDocShell() );
    }

    RedlineFlags eRedlMode = m_pDoc->getIDocumentRedlineAccess().GetRedlineFlags(), eOldMode = eRedlMode;
    if( m_aFlags.bWithRedlining )
    {
        m_pDoc->SetAutoFormatRedline( true );
        eRedlMode = RedlineFlags::On | (eOldMode & RedlineFlags::ShowMask);
    }
    else
      eRedlMode = RedlineFlags::Ignore | (eOldMode & RedlineFlags::ShowMask);
    m_pDoc->getIDocumentRedlineAccess().SetRedlineFlags( eRedlMode );

    // save undo state (might be turned off)
    bool const bUndoState = m_pDoc->GetIDocumentUndoRedo().DoesUndo();

    // If multiple lines, then do not merge with next paragraph
    m_bMoreLines = false;

    sal_uInt16 nLastCalcHeadLvl = 0;
    sal_uInt16 nLastHeadLvl = USHRT_MAX;
    sal_uInt16 nLevel = 0;
    sal_uInt16 nDigitLvl = 0;

    // set defaults
    SwTextFrameInfo aFInfo( nullptr );

    enum Format_Status
    {
        READ_NEXT_PARA, // -> ISEND, TST_EMPTY_LINE
        TST_EMPTY_LINE, // -> READ_NEXT_PARA, TST_ALPHA_LINE
        TST_ALPHA_LINE, // -> READ_NEXT_PARA, GET_ALL_INFO, IS_END
        GET_ALL_INFO,   // -> READ_NEXT_PARA, IS_ONE_LINE, TST_ENUMERIC, HAS_FMTCOLL
        IS_ONE_LINE,    // -> READ_NEXT_PARA, TST_ENUMERIC
        TST_ENUMERIC,   // -> READ_NEXT_PARA, TST_IDENT, TST_NEG_IDENT
        TST_IDENT,      // -> READ_NEXT_PARA, TST_TXT_BODY
        TST_NEG_IDENT,  // -> READ_NEXT_PARA, TST_TXT_BODY
        TST_TXT_BODY,   // -> READ_NEXT_PARA
        HAS_FMTCOLL,    // -> READ_NEXT_PARA
        IS_END
    } eStat;

    // This is the automat for autoformatting
    eStat = READ_NEXT_PARA;
    while( !m_bEnd )
    {
        switch( eStat )
        {
        case READ_NEXT_PARA:
            {
                GoNextPara();
                eStat = m_bEnd ? IS_END : TST_EMPTY_LINE;
            }
            break;

        case TST_EMPTY_LINE:
            if (IsEmptyLine(*m_pCurTextFrame))
            {
                if (m_aFlags.bDelEmptyNode && !HasObjects(*m_pCurTextFrame))
                {
                    bEmptyLine = true;
                    sal_uLong nOldCnt = m_pDoc->GetNodes().Count();
                    DelEmptyLine();
                    // Was there really a deletion of a node?
                    if( nOldCnt != m_pDoc->GetNodes().Count() )
                    {
                        // do not skip the next paragraph
                        sw::GotoPrevLayoutTextFrame(m_aNdIdx, m_pEditShell->GetLayout());
                    }
                }
                eStat = READ_NEXT_PARA;
            }
            else
                eStat = TST_ALPHA_LINE;
            break;

        case TST_ALPHA_LINE:
            if (IsNoAlphaLine(*m_pCurTextFrame))
            {
                // recognize a table definition +---+---+
                if( m_aFlags.bAFormatByInput && m_aFlags.bCreateTable && DoTable() )
                {
                    //JP 30.09.96: DoTable() builds on PopCursor and MoveCursor after AutoFormat!
                    pEdShell->Pop(SwCursorShell::PopMode::DeleteCurrent);
                    *pEdShell->GetCursor() = m_aDelPam;
                    pEdShell->Push();

                    eStat = IS_END;
                    break;
                }

                // Check for 3 "---" or "===". In this case, the previous paragraph should be
                // underlined and the current be deleted!
                if( !DoUnderline() && bReplaceStyles )
                {
                    SetColl( RES_POOLCOLL_STANDARD, true );
                    bEmptyLine = true;
                }
                eStat = READ_NEXT_PARA;
            }
            else
                eStat = GET_ALL_INFO;
            break;

        case GET_ALL_INFO:
            {
                if (m_pCurTextFrame->GetTextNodeForParaProps()->GetNumRule())
                {
                    // do nothing in numbering, go to next
                    bEmptyLine = false;
                    eStat = READ_NEXT_PARA;
                    // delete all blanks at beginning/end and in between
                    //JP 29.04.98: first only "all in between"
                    DelMoreLinesBlanks();
                    break;
                }

                aFInfo.SetFrame( m_pCurTextFrame );

                // so far: if there were templates assigned, keep these and go to next node
                sal_uInt16 nPoolId = m_pCurTextFrame->GetTextNodeForParaProps()->GetTextColl()->GetPoolFormatId();
                if( IsPoolUserFormat( nPoolId )
                        ? !m_aFlags.bChgUserColl
                        : ( RES_POOLCOLL_STANDARD != nPoolId &&
                           ( !m_aFlags.bAFormatByInput ||
                            (RES_POOLCOLL_TEXT_MOVE != nPoolId &&
                             RES_POOLCOLL_TEXT != nPoolId )) ))
                {
                    eStat = HAS_FMTCOLL;
                    break;
                }

                // check for hard spaces or LRSpaces set by the template
                if( IsPoolUserFormat( nPoolId ) ||
                    RES_POOLCOLL_STANDARD == nPoolId )
                {
                    short nSz;
                    SvxLRSpaceItem const * pLRSpace;
                    if (SfxItemState::SET == m_pCurTextFrame->GetTextNodeForParaProps()->GetSwAttrSet().
                        GetItemState( RES_LR_SPACE, true,
                                        reinterpret_cast<const SfxPoolItem**>(&pLRSpace) ) &&
                        ( 0 != (nSz = pLRSpace->GetTextFirstLineOfst()) ||
                            0 != pLRSpace->GetTextLeft() ) )
                    {
                        // exception: numbering/enumeration can have an indentation
                        if (IsEnumericChar(*m_pCurTextFrame))
                        {
                            nLevel = CalcLevel(*m_pCurTextFrame, &nDigitLvl);
                            if( nLevel >= MAXLEVEL )
                                nLevel = MAXLEVEL-1;
                            BuildEnum( nLevel, nDigitLvl );
                            eStat = READ_NEXT_PARA;
                            break;
                        }

                        // never merge (maybe only indent as exception)
                        m_bMoreLines = true;

                        if( bReplaceStyles )
                        {
                            // then use one of our templates
                            if( 0 < nSz )           // positive 1st line indentation
                                BuildIndent();
                            else if( 0 > nSz )      // negative 1st line indentation
                                BuildNegIndent( aFInfo.GetLineStart() );
                            else if( pLRSpace->GetTextLeft() )   // is indentation
                                BuildTextIndent();
                        }
                        eStat = READ_NEXT_PARA;
                        break;
                    }
                }

                nLevel = CalcLevel( *m_pCurTextFrame, &nDigitLvl );
                m_bMoreLines = !IsOneLine(*m_pCurTextFrame);
                // note: every use of pNextFrame in following states, until the
                // next READ_NEXT_PARA, relies on this update
                pNextFrame = GetNextNode();
                if (pNextFrame)
                {
                    bNxtEmpty = IsEmptyLine(*pNextFrame);
                    bNxtAlpha = IsNoAlphaLine(*pNextFrame);
                    nNxtLevel = CalcLevel(*pNextFrame);

                    if (!bEmptyLine && HasBreakAttr(*m_pCurTextFrame))
                        bEmptyLine = true;
                    if (!bNxtEmpty && HasBreakAttr(*pNextFrame))
                        bNxtEmpty = true;

                }
                else
                {
                    bNxtEmpty = false;
                    bNxtAlpha = false;
                    nNxtLevel = 0;
                }
                eStat = !m_bMoreLines ? IS_ONE_LINE : TST_ENUMERIC;
            }
            break;

        case IS_ONE_LINE:
            {
                eStat = TST_ENUMERIC;
                if( !bReplaceStyles )
                    break;

                const OUString sClrStr( DelLeadingBlanks(m_pCurTextFrame->GetText()) );

                if( sClrStr.isEmpty() )
                {
                    bEmptyLine = true;
                    eStat = READ_NEXT_PARA;
                    break;      // read next paragraph
                }

                // check if headline
                if (!bEmptyLine || !IsFirstCharCapital(*m_pCurTextFrame)
                    || IsBlanksInString(*m_pCurTextFrame))
                    break;

                bEmptyLine = false;
                const OUString sEndClrStr( DelTrailingBlanks(sClrStr) );
                const sal_Unicode cLast = sEndClrStr[sEndClrStr.getLength() - 1];

                // not, then check if headline
                if( ':' == cLast )
                {
                    BuildHeadLine( 2 );
                    eStat = READ_NEXT_PARA;
                    break;
                }
                else if( 256 <= cLast || !strchr( ",.;", cLast ) )
                {
                    if( bNxtEmpty || bNxtAlpha
                        || (pNextFrame && IsEnumericChar(*pNextFrame)))
                    {

                        // one level below?
                        if( nLevel >= MAXLEVEL )
                            nLevel = MAXLEVEL-1;

                        if( USHRT_MAX == nLastHeadLvl )
                            nLastHeadLvl = 0;
                        else if( nLastCalcHeadLvl < nLevel )
                        {
                            if( nLastHeadLvl+1 < MAXLEVEL )
                                ++nLastHeadLvl;
                        }
                        // one level above?
                        else if( nLastCalcHeadLvl > nLevel )
                        {
                            if( nLastHeadLvl )
                                --nLastHeadLvl;
                        }
                        nLastCalcHeadLvl = nLevel;

                        if( m_aFlags.bAFormatByInput )
                            BuildHeadLine( nLevel );
                        else
                            BuildHeadLine( nLastHeadLvl );
                        eStat = READ_NEXT_PARA;
                        break;
                    }
                }
            }
            break;

        case TST_ENUMERIC:
            {
                bEmptyLine = false;
                if (IsEnumericChar(*m_pCurTextFrame))
                {
                    if( nLevel >= MAXLEVEL )
                        nLevel = MAXLEVEL-1;
                    BuildEnum( nLevel, nDigitLvl );
                    eStat = READ_NEXT_PARA;
                }
                else if( bReplaceStyles )
                    eStat = nLevel ? TST_IDENT : TST_NEG_IDENT;
                else
                    eStat = READ_NEXT_PARA;
            }
            break;

        case TST_IDENT:
            // Spaces at the beginning, check again for indentation
            if( m_bMoreLines && nLevel )
            {
                SwTwips nSz = aFInfo.GetFirstIndent();
                if( 0 < nSz )           // positive 1st line indentation
                    BuildIndent();
                else if( 0 > nSz )      // negative 1st line indentation
                    BuildNegIndent( aFInfo.GetLineStart() );
                else                    // is indentation
                    BuildTextIndent();
                eStat = READ_NEXT_PARA;
            }
            else if (nLevel && pNextFrame &&
                     !bNxtEmpty && !bNxtAlpha && !nNxtLevel &&
                     !IsEnumericChar(*pNextFrame))
            {
                // is an indentation
                BuildIndent();
                eStat = READ_NEXT_PARA;
            }
            else
                eStat = TST_TXT_BODY;
            break;

        case TST_NEG_IDENT:
            // no spaces at the beginning, check again for negative indentation
            {
                if( m_bMoreLines && !nLevel )
                {
                    SwTwips nSz = aFInfo.GetFirstIndent();
                    if( 0 < nSz )           // positive 1st line indentation
                        BuildIndent();
                    else if( 0 > nSz )      // negative 1st line indentation
                        BuildNegIndent( aFInfo.GetLineStart() );
                    else                    // is _no_ indentation
                        BuildText();
                    eStat = READ_NEXT_PARA;
                }
                else if (!nLevel && pNextFrame &&
                         !bNxtEmpty && !bNxtAlpha && nNxtLevel &&
                         !IsEnumericChar(*pNextFrame))
                {
                    // is a negative indentation
                    BuildNegIndent( aFInfo.GetLineStart() );
                    eStat = READ_NEXT_PARA;
                }
                else
                    eStat = TST_TXT_BODY;
            }
            break;

        case TST_TXT_BODY:
            {
                if( m_bMoreLines )
                {
                    SwTwips nSz = aFInfo.GetFirstIndent();
                    if( 0 < nSz )           // positive 1st line indentation
                        BuildIndent();
                    else if( 0 > nSz )      // negative 1st line indentation
                        BuildNegIndent( aFInfo.GetLineStart() );
                    else if( nLevel )       // is indentation
                        BuildTextIndent();
                    else
                        BuildText();
                }
                else if( nLevel )
                    BuildTextIndent();
                else
                    BuildText();
                eStat = READ_NEXT_PARA;
            }
            break;

        case HAS_FMTCOLL:
            {
                // so far: if there were templates assigned, keep these and go to next node
                bEmptyLine = false;
                eStat = READ_NEXT_PARA;
                // delete all blanks at beginning/end and in between
                //JP 29.04.98: first only "all in between"
                DelMoreLinesBlanks();

                // handle hard attributes
                if (m_pCurTextFrame->GetTextNodeForParaProps()->HasSwAttrSet())
                {
                    short nSz;
                    SvxLRSpaceItem const * pLRSpace;
                    if( bReplaceStyles &&
                        SfxItemState::SET == m_pCurTextFrame->GetTextNodeForParaProps()->GetSwAttrSet().
                        GetItemState( RES_LR_SPACE, false,
                                        reinterpret_cast<const SfxPoolItem**>(&pLRSpace) ) &&
                        ( 0 != (nSz = pLRSpace->GetTextFirstLineOfst()) ||
                            0 != pLRSpace->GetTextLeft() ) )
                    {
                        // then use one of our templates
                        if( 0 < nSz )           // positive 1st line indentation
                            BuildIndent();
                        else if( 0 > nSz )      // negative 1st line indentation
                        {
                            BuildNegIndent( aFInfo.GetLineStart() );
                        }
                        else if( pLRSpace->GetTextLeft() )   // is indentation
                            BuildTextIndent();
                        else
                            BuildText();
                    }
                }
            }
            break;

        case IS_END:
            m_bEnd = true;
            break;
        }
    }

    if( m_aFlags.bWithRedlining )
        m_pDoc->SetAutoFormatRedline( false );
    m_pDoc->getIDocumentRedlineAccess().SetRedlineFlags( eOldMode );

    // restore undo (in case it has been changed)
    m_pDoc->GetIDocumentUndoRedo().DoUndo(bUndoState);

    // disable display of percentage again
    if( !m_aFlags.bAFormatByInput )
        ::EndProgress( m_pDoc->GetDocShell() );
}

void SwEditShell::AutoFormat( const SvxSwAutoFormatFlags* pAFlags )
{
    std::unique_ptr<SwWait> pWait;

    SET_CURR_SHELL( this );
    StartAllAction();
    StartUndo( SwUndoId::AUTOFORMAT );

    SvxSwAutoFormatFlags aAFFlags;     // use default values or add params?
    if( pAFlags )
    {
        aAFFlags = *pAFlags;
        if( !aAFFlags.bAFormatByInput )
            pWait.reset(new SwWait( *GetDoc()->GetDocShell(), true ));
    }

    SwPaM* pCursor = GetCursor();
    // There are more than one or a selection is open
    if( pCursor->GetNext() != pCursor || pCursor->HasMark() )
    {
        for(SwPaM& rPaM : GetCursor()->GetRingContainer())
        {
            if( rPaM.HasMark() )
            {
                SwAutoFormat aFormat( this, aAFFlags, &(rPaM.Start()->nNode),
                                     &(rPaM.End()->nNode) );
            }
        }
    }
    else
    {
        SwAutoFormat aFormat( this, aAFFlags );
    }

    EndUndo( SwUndoId::AUTOFORMAT );
    EndAllAction();
}

void SwEditShell::AutoFormatBySplitNode()
{
    SET_CURR_SHELL( this );
    SwPaM* pCursor = GetCursor();
    if( pCursor->IsMultiSelection() || !pCursor->Move( fnMoveBackward, GoInNode ) )
        return;

    StartAllAction();
    StartUndo( SwUndoId::AUTOFORMAT );

    bool bRange = false;
    pCursor->SetMark();
    SwIndex* pContent = &pCursor->GetMark()->nContent;
    if( pContent->GetIndex() )
    {
        *pContent = 0;
        bRange = true;
    }
    else
    {
        // then go one node backwards
        SwNodeIndex aNdIdx(pCursor->GetMark()->nNode);
        sw::GotoPrevLayoutTextFrame(aNdIdx, GetLayout());
        SwTextNode* pTextNd = aNdIdx.GetNode().GetTextNode();
        if (pTextNd && !pTextNd->GetText().isEmpty())
        {
            pContent->Assign( pTextNd, 0 );
            pCursor->GetMark()->nNode = aNdIdx;
            bRange = true;
        }
    }

    if( bRange )
    {
        Push();     // save cursor

        SvxSwAutoFormatFlags aAFFlags = *GetAutoFormatFlags(); // use default values so far

        SwAutoFormat aFormat( this, aAFFlags, &pCursor->GetMark()->nNode,
                                &pCursor->GetPoint()->nNode );
        SvxAutoCorrect* pACorr = SvxAutoCorrCfg::Get().GetAutoCorrect();
        if( pACorr && !pACorr->IsAutoCorrFlag( ACFlags::CapitalStartSentence | ACFlags::CapitalStartWord |
                                ACFlags::AddNonBrkSpace | ACFlags::ChgOrdinalNumber |
                                ACFlags::ChgToEnEmDash | ACFlags::SetINetAttr | ACFlags::Autocorrect ))
            pACorr = nullptr;

        if( pACorr )
            AutoCorrect( *pACorr,false, u'\0' );

        //JP 30.09.96: DoTable() builds on PopCursor and MoveCursor!
        Pop(PopMode::DeleteCurrent);
        pCursor = GetCursor();
    }
    pCursor->DeleteMark();
    pCursor->Move( fnMoveForward, GoInNode );

    EndUndo( SwUndoId::AUTOFORMAT );
    EndAllAction();

}

SvxSwAutoFormatFlags* SwEditShell::GetAutoFormatFlags()
{
    if (!s_pAutoFormatFlags)
        s_pAutoFormatFlags = new SvxSwAutoFormatFlags;

    return s_pAutoFormatFlags;
}

void SwEditShell::SetAutoFormatFlags(SvxSwAutoFormatFlags const * pFlags)
{
    SvxSwAutoFormatFlags* pEditFlags = GetAutoFormatFlags();

    pEditFlags->bSetNumRule     = pFlags->bSetNumRule;
    pEditFlags->bChgEnumNum     = pFlags->bChgEnumNum;
    pEditFlags->bSetBorder      = pFlags->bSetBorder;
    pEditFlags->bCreateTable    = pFlags->bCreateTable;
    pEditFlags->bReplaceStyles  = pFlags->bReplaceStyles;
    pEditFlags->bAFormatByInpDelSpacesAtSttEnd =
                                    pFlags->bAFormatByInpDelSpacesAtSttEnd;
    pEditFlags->bAFormatByInpDelSpacesBetweenLines =
                                    pFlags->bAFormatByInpDelSpacesBetweenLines;

    //JP 15.12.98: copy BulletChar and Font into "normal" ones
    //             because AutoFormat can only work with the latter!
    pEditFlags->cBullet             = pFlags->cByInputBullet;
    pEditFlags->aBulletFont         = pFlags->aByInputBulletFont;
    pEditFlags->cByInputBullet      = pFlags->cByInputBullet;
    pEditFlags->aByInputBulletFont  = pFlags->aByInputBulletFont;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
