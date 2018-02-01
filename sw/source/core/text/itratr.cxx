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

#include <sal/config.h>

#include <algorithm>

#include <hintids.hxx>
#include <editeng/charscaleitem.hxx>
#include <txtatr.hxx>
#include <sfx2/printer.hxx>
#include <svx/svdobj.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include <fmtornt.hxx>
#include <fmtflcnt.hxx>
#include <fmtcntnt.hxx>
#include <fmtftn.hxx>
#include <frmatr.hxx>
#include <frmfmt.hxx>
#include <fmtfld.hxx>
#include <doc.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <viewsh.hxx>
#include <rootfrm.hxx>
#include <docary.hxx>
#include <ndtxt.hxx>
#include <dcontact.hxx>
#include <fldbas.hxx>
#include <pam.hxx>
#include "itratr.hxx"
#include <htmltbl.hxx>
#include <swtable.hxx>
#include "redlnitr.hxx"
#include <fmtsrnd.hxx>
#include "itrtxt.hxx"
#include <breakit.hxx>
#include <com/sun/star/i18n/WordType.hpp>
#include <editeng/lrspitem.hxx>
#include <calbck.hxx>

using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star;

void SwAttrIter::Chg( SwTextAttr const *pHt )
{
    assert(pHt && m_pFont && "No attribute of font available for change");
    if( m_pRedline && m_pRedline->IsOn() )
        m_pRedline->ChangeTextAttr( m_pFont, *pHt, true );
    else
        m_aAttrHandler.PushAndChg( *pHt, *m_pFont );
    m_nChgCnt++;
}

void SwAttrIter::Rst( SwTextAttr const *pHt )
{
    assert(pHt && m_pFont && "No attribute of font available for reset");
    // get top from stack after removing pHt
    if( m_pRedline && m_pRedline->IsOn() )
        m_pRedline->ChangeTextAttr( m_pFont, *pHt, false );
    else
        m_aAttrHandler.PopAndChg( *pHt, *m_pFont );
    m_nChgCnt--;
}

SwAttrIter::~SwAttrIter()
{
    delete m_pRedline;
    delete m_pFont;
}

/**
 * Returns the attribute for a position
 *
 * Only if the attribute is exactly at the position @param nPos and
 * does not have an EndIndex
 *
 * We need this function for attributes which should alter formatting without
 * changing the content of the string.
 * Such "degenerated" attributes are e.g.: fields which retain expanded text and
 * line-bound Frames.
 * In order to avoid ambiguities between different such attributes, we insert a
 * special character at the start of the string, when creating such an attribute.
 * The Formatter later on encounters such a special character and retrieves the
 * degenerate attribute via GetAttr().
 */
SwTextAttr *SwAttrIter::GetAttr( const sal_Int32 nPosition ) const
{
    return (m_pTextNode) ? m_pTextNode->GetTextAttrForCharAt(nPosition) : nullptr;
}

bool SwAttrIter::SeekAndChgAttrIter( const sal_Int32 nNewPos, OutputDevice* pOut )
{
    bool bChg = m_nStartIndex && nNewPos == m_nPosition ? m_pFont->IsFntChg() : Seek( nNewPos );
    if ( m_pLastOut.get() != pOut )
    {
        m_pLastOut = pOut;
        m_pFont->SetFntChg( true );
        bChg = true;
    }
    if( bChg )
    {
        // if the change counter is zero, we know the MagicNo of the wanted font
        if ( !m_nChgCnt && !m_nPropFont )
            m_pFont->SetMagic( m_aMagicNo[ m_pFont->GetActual() ],
                m_aFontIdx[ m_pFont->GetActual() ], m_pFont->GetActual() );
        m_pFont->ChgPhysFnt( m_pViewShell, *pOut );
    }

    return bChg;
}

bool SwAttrIter::IsSymbol( const sal_Int32 nNewPos )
{
    Seek( nNewPos );
    if ( !m_nChgCnt && !m_nPropFont )
        m_pFont->SetMagic( m_aMagicNo[ m_pFont->GetActual() ],
            m_aFontIdx[ m_pFont->GetActual() ], m_pFont->GetActual() );
    return m_pFont->IsSymbol( m_pViewShell );
}

bool SwAttrIter::SeekStartAndChgAttrIter( OutputDevice* pOut, const bool bParaFont )
{
    if ( m_pRedline && m_pRedline->ExtOn() )
        m_pRedline->LeaveExtend( *m_pFont, 0 );

    // reset font to its original state
    m_aAttrHandler.Reset();
    m_aAttrHandler.ResetFont( *m_pFont );

    m_nStartIndex = 0;
    m_nEndIndex = 0;
    m_nPosition = 0;
    m_nChgCnt = 0;
    if( m_nPropFont )
        m_pFont->SetProportion( m_nPropFont );
    if( m_pRedline )
    {
        m_pRedline->Clear( m_pFont );
        if( !bParaFont )
            m_nChgCnt = m_nChgCnt + m_pRedline->Seek( *m_pFont, 0, USHRT_MAX );
        else
            m_pRedline->Reset();
    }

    if ( m_pHints && !bParaFont )
    {
        SwTextAttr *pTextAttr;
        // While we've not reached the end of the StartArray && the TextAttribute starts at position 0...
        while ( ( m_nStartIndex < m_pHints->Count() ) &&
                !((pTextAttr = m_pHints->Get(m_nStartIndex))->GetStart()) )
        {
            // open the TextAttributes
            Chg( pTextAttr );
            m_nStartIndex++;
        }
    }

    bool bChg = m_pFont->IsFntChg();
    if ( m_pLastOut.get() != pOut )
    {
        m_pLastOut = pOut;
        m_pFont->SetFntChg( true );
        bChg = true;
    }
    if( bChg )
    {
        // if the application counter is zero, we know the MagicNo of the wanted font
        if ( !m_nChgCnt && !m_nPropFont )
            m_pFont->SetMagic( m_aMagicNo[ m_pFont->GetActual() ],
                m_aFontIdx[ m_pFont->GetActual() ], m_pFont->GetActual() );
        m_pFont->ChgPhysFnt( m_pViewShell, *pOut );
    }
    return bChg;
}

// AMA: New AttrIter Nov 94
void SwAttrIter::SeekFwd( const sal_Int32 nNewPos )
{
    SwTextAttr *pTextAttr;

    if ( m_nStartIndex ) // If attributes have been opened at all ...
    {
        // Close attributes that are currently open, but stop at nNewPos+1

        // As long as we've not yet reached the end of EndArray and the
        // TextAttribute ends before or at the new position ...
        while ( ( m_nEndIndex < m_pHints->Count() ) &&
                (*(pTextAttr=m_pHints->GetSortedByEnd(m_nEndIndex))->GetAnyEnd()<=nNewPos))
        {
            // Close the TextAttributes, whose StartPos were before or at
            // the old nPos and are currently open
            if (pTextAttr->GetStart() <= m_nPosition)  Rst( pTextAttr );
            m_nEndIndex++;
        }
    }
    else // skip the not opened ends
    {
        while ( (m_nEndIndex < m_pHints->Count()) &&
                (*m_pHints->GetSortedByEnd(m_nEndIndex)->GetAnyEnd() <= nNewPos) )
        {
            m_nEndIndex++;
        }
    }

    // As long as we've not yet reached the end of EndArray and the
    // TextAttribute ends before or at the new position...
    while ( ( m_nStartIndex < m_pHints->Count() ) &&
            ((pTextAttr=m_pHints->Get(m_nStartIndex))->GetStart()<=nNewPos) )
    {

        // open the TextAttributes, whose ends lie behind the new position
        if ( *pTextAttr->GetAnyEnd() > nNewPos )  Chg( pTextAttr );
        m_nStartIndex++;
    }

}

bool SwAttrIter::Seek( const sal_Int32 nNewPos )
{
    if ( m_pRedline && m_pRedline->ExtOn() )
        m_pRedline->LeaveExtend( *m_pFont, nNewPos );

    if( m_pHints )
    {
        if( !nNewPos || nNewPos < m_nPosition )
        {
            if( m_pRedline )
                m_pRedline->Clear( nullptr );

            // reset font to its original state
            m_aAttrHandler.Reset();
            m_aAttrHandler.ResetFont( *m_pFont );

            if( m_nPropFont )
                m_pFont->SetProportion( m_nPropFont );
            m_nStartIndex = 0;
            m_nEndIndex = 0;
            m_nPosition = 0;
            m_nChgCnt = 0;

            // Attention!
            // resetting the font here makes it necessary to apply any
            // changes for extended input directly to the font
            if ( m_pRedline && m_pRedline->ExtOn() )
            {
                m_pRedline->UpdateExtFont( *m_pFont );
                ++m_nChgCnt;
            }
        }
        SeekFwd( nNewPos );
    }

    m_pFont->SetActual( SwScriptInfo::WhichFont( nNewPos, nullptr, m_pScriptInfo ) );

    if( m_pRedline )
        m_nChgCnt = m_nChgCnt + m_pRedline->Seek( *m_pFont, nNewPos, m_nPosition );
    m_nPosition = nNewPos;

    if( m_nPropFont )
        m_pFont->SetProportion( m_nPropFont );

    return m_pFont->IsFntChg();
}

sal_Int32 SwAttrIter::GetNextAttr( ) const
{
    sal_Int32 nNext = COMPLETE_STRING;
    if( m_pHints )
    {
        // are there attribute starts left?
        for (size_t i = m_nStartIndex; i < m_pHints->Count(); ++i)
        {
            SwTextAttr *const pAttr(m_pHints->Get(i));
            if (!pAttr->IsFormatIgnoreStart())
            {
                nNext = pAttr->GetStart();
                break;
            }
        }
        // are there attribute ends left?
        for (size_t i = m_nEndIndex; i < m_pHints->Count(); ++i)
        {
            SwTextAttr *const pAttr(m_pHints->GetSortedByEnd(i));
            if (!pAttr->IsFormatIgnoreEnd())
            {
                sal_Int32 const nNextEnd = *pAttr->GetAnyEnd();
                nNext = std::min(nNext, nNextEnd); // pick nearest one
                break;
            }
        }
    }
    if (m_pTextNode!=nullptr) {
        // TODO: maybe use hints like FieldHints for this instead of looking at the text...
        const sal_Int32 l = std::min(nNext, m_pTextNode->Len());
        sal_Int32 p=m_nPosition;
        const sal_Unicode* aStr = m_pTextNode->GetText().getStr();
        while (p<l)
        {
            sal_Unicode aChar = aStr[p];
            if (aChar < CH_TXT_ATR_FORMELEMENT
                || aChar > CH_TXT_ATR_FIELDEND)
            {
                ++p;
            }
            else
            {
                break;
            }
        }
        if ((p<l && p>m_nPosition) || nNext<=p)
        nNext=p;
        else
        nNext=p+1;
    }
    if( m_pRedline )
        return m_pRedline->GetNextRedln( nNext );
    return nNext;
}

class SwMinMaxArgs
{
public:
    VclPtr<OutputDevice> pOut;
    SwViewShell const * pSh;
    sal_uLong &rMin;
    sal_uLong &rAbsMin;
    long nRowWidth;
    long nWordWidth;
    long nWordAdd;
    sal_Int32 nNoLineBreak;
    SwMinMaxArgs( OutputDevice* pOutI, SwViewShell const * pShI, sal_uLong& rMinI, sal_uLong &rAbsI )
        : pOut( pOutI ), pSh( pShI ), rMin( rMinI ), rAbsMin( rAbsI )
        { nRowWidth = nWordWidth = nWordAdd = 0; nNoLineBreak = COMPLETE_STRING; }
    void Minimum( long nNew ) const { if( static_cast<long>(rMin) < nNew ) rMin = nNew; }
    void NewWord() { nWordAdd = nWordWidth = 0; }
};

static bool lcl_MinMaxString( SwMinMaxArgs& rArg, SwFont* pFnt, const OUString &rText,
    sal_Int32 nIdx, sal_Int32 nEnd )
{
    bool bRet = false;
    while( nIdx < nEnd )
    {
        sal_Int32 nStop = nIdx;
        LanguageType eLang = pFnt->GetLanguage();
        assert(g_pBreakIt && g_pBreakIt->GetBreakIter().is());

        bool bClear = CH_BLANK == rText[ nStop ];
        Boundary aBndry( g_pBreakIt->GetBreakIter()->getWordBoundary( rText, nIdx,
                         g_pBreakIt->GetLocale( eLang ),
                         WordType::DICTIONARY_WORD, true ) );
        nStop = aBndry.endPos;
        if( nIdx <= aBndry.startPos && nIdx && nIdx-1 != rArg.nNoLineBreak )
            rArg.NewWord();
        if( nStop == nIdx )
            ++nStop;
        if( nStop > nEnd )
            nStop = nEnd;

        SwDrawTextInfo aDrawInf( rArg.pSh, *rArg.pOut, nullptr, rText, nIdx, nStop - nIdx );
        long nAktWidth = pFnt->GetTextSize_( aDrawInf ).Width();
        rArg.nRowWidth += nAktWidth;
        if( bClear )
            rArg.NewWord();
        else
        {
            rArg.nWordWidth += nAktWidth;
            if( static_cast<long>(rArg.rAbsMin) < rArg.nWordWidth )
                rArg.rAbsMin = rArg.nWordWidth;
            rArg.Minimum( rArg.nWordWidth + rArg.nWordAdd );
            bRet = true;
        }
        nIdx = nStop;
    }
    return bRet;
}

bool SwTextNode::IsSymbol( const sal_Int32 nBegin ) const
{
    SwScriptInfo aScriptInfo;
    SwAttrIter aIter( *const_cast<SwTextNode*>(this), aScriptInfo );
    aIter.Seek( nBegin );
    return aIter.GetFnt()->IsSymbol( getIDocumentLayoutAccess().GetCurrentViewShell() );
}

class SwMinMaxNodeArgs
{
public:
    sal_uLong nMaxWidth;    // sum of all frame widths
    long nMinWidth;         // biggest frame
    long nLeftRest;         // space not already covered by frames in the left margin
    long nRightRest;        // space not already covered by frames in the right margin
    long nLeftDiff;         // Min/Max-difference of the frame in the left margin
    long nRightDiff;        // Min/Max-difference of the frame in the right margin
    sal_uLong nIndx;        // index of the node
    void Minimum( long nNew ) { if( nNew > nMinWidth ) nMinWidth = nNew; }
};

static void lcl_MinMaxNode( SwFrameFormat* pNd, SwMinMaxNodeArgs* pIn )
{
    const SwFormatAnchor& rFormatA = pNd->GetAnchor();

    if ((RndStdIds::FLY_AT_PARA != rFormatA.GetAnchorId()) &&
        (RndStdIds::FLY_AT_CHAR != rFormatA.GetAnchorId()))
    {
        return;
    }

    const SwPosition *pPos = rFormatA.GetContentAnchor();
    OSL_ENSURE(pPos && pIn, "Unexpected NULL arguments");
    if (!pPos || !pIn || pIn->nIndx != pPos->nNode.GetIndex())
        return;

    long nMin, nMax;
    SwHTMLTableLayout *pLayout = nullptr;
    const bool bIsDrawFrameFormat = pNd->Which()==RES_DRAWFRMFMT;
    if( !bIsDrawFrameFormat )
    {
        // Does the frame contain a table at the start or the end?
        const SwNodes& rNodes = pNd->GetDoc()->GetNodes();
        const SwFormatContent& rFlyContent = pNd->GetContent();
        sal_uLong nStt = rFlyContent.GetContentIdx()->GetIndex();
        SwTableNode* pTableNd = rNodes[nStt+1]->GetTableNode();
        if( !pTableNd )
        {
            SwNode *pNd2 = rNodes[nStt];
            pNd2 = rNodes[pNd2->EndOfSectionIndex()-1];
            if( pNd2->IsEndNode() )
                pTableNd = pNd2->StartOfSectionNode()->GetTableNode();
        }

        if( pTableNd )
            pLayout = pTableNd->GetTable().GetHTMLTableLayout();
    }

    const SwFormatHoriOrient& rOrient = pNd->GetHoriOrient();
    sal_Int16 eHoriOri = rOrient.GetHoriOrient();

    long nDiff;
    if( pLayout )
    {
        nMin = pLayout->GetMin();
        nMax = pLayout->GetMax();
        nDiff = nMax - nMin;
    }
    else
    {
        if( bIsDrawFrameFormat )
        {
            const SdrObject* pSObj = pNd->FindSdrObject();
            if( pSObj )
                nMin = pSObj->GetCurrentBoundRect().GetWidth();
            else
                nMin = 0;

        }
        else
        {
            const SwFormatFrameSize &rSz = pNd->GetFrameSize();
            nMin = rSz.GetWidth();
        }
        nMax = nMin;
        nDiff = 0;
    }

    const SvxLRSpaceItem &rLR = pNd->GetLRSpace();
    nMin += rLR.GetLeft();
    nMin += rLR.GetRight();
    nMax += rLR.GetLeft();
    nMax += rLR.GetRight();

    if( css::text::WrapTextMode_THROUGH == pNd->GetSurround().GetSurround() )
    {
        pIn->Minimum( nMin );
        return;
    }

    // Frames, which are left- or right-aligned are only party considered
    // when calculating the maximum, since the border is already being considered.
    // Only if the frame extends into the text body, this part is being added
    switch( eHoriOri )
    {
        case text::HoriOrientation::RIGHT:
        {
            if( nDiff )
            {
                pIn->nRightRest -= pIn->nRightDiff;
                pIn->nRightDiff = nDiff;
            }
            if( text::RelOrientation::FRAME != rOrient.GetRelationOrient() )
            {
                if( pIn->nRightRest > 0 )
                    pIn->nRightRest = 0;
            }
            pIn->nRightRest -= nMin;
            break;
        }
        case text::HoriOrientation::LEFT:
        {
            if( nDiff )
            {
                pIn->nLeftRest -= pIn->nLeftDiff;
                pIn->nLeftDiff = nDiff;
            }
            if( text::RelOrientation::FRAME != rOrient.GetRelationOrient() &&
                pIn->nLeftRest < 0 )
                pIn->nLeftRest = 0;
            pIn->nLeftRest -= nMin;
            break;
        }
        default:
        {
            pIn->nMaxWidth += nMax;
            pIn->Minimum( nMin );
        }
    }
}

#define FLYINCNT_MIN_WIDTH 284

/**
 * Changing this method very likely requires changing of GetScalingOfSelectedText
 */
void SwTextNode::GetMinMaxSize( sal_uLong nIndex, sal_uLong& rMin, sal_uLong &rMax,
                               sal_uLong& rAbsMin ) const
{
    SwViewShell const * pSh = GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    OutputDevice* pOut = nullptr;
    if( pSh )
        pOut = pSh->GetWin();
    if( !pOut )
        pOut = Application::GetDefaultDevice();

    MapMode aOldMap( pOut->GetMapMode() );
    pOut->SetMapMode( MapMode( MapUnit::MapTwip ) );

    rMin = 0;
    rMax = 0;
    rAbsMin = 0;

    const SvxLRSpaceItem &rSpace = GetSwAttrSet().GetLRSpace();
    long nLROffset = rSpace.GetTextLeft() + GetLeftMarginWithNum( true );
    short nFLOffs;
    // For enumerations a negative first line indentation is probably filled already
    if( !GetFirstLineOfsWithNum( nFLOffs ) || nFLOffs > nLROffset )
        nLROffset = nFLOffs;

    SwMinMaxNodeArgs aNodeArgs;
    aNodeArgs.nMinWidth = 0;
    aNodeArgs.nMaxWidth = 0;
    aNodeArgs.nLeftRest = nLROffset;
    aNodeArgs.nRightRest = rSpace.GetRight();
    aNodeArgs.nLeftDiff = 0;
    aNodeArgs.nRightDiff = 0;
    if( nIndex )
    {
        SwFrameFormats* pTmp = const_cast<SwFrameFormats*>(GetDoc()->GetSpzFrameFormats());
        if( pTmp )
        {
            aNodeArgs.nIndx = nIndex;
            for( SwFrameFormat *pFormat : *pTmp )
                lcl_MinMaxNode( pFormat, &aNodeArgs );
        }
    }
    if( aNodeArgs.nLeftRest < 0 )
        aNodeArgs.Minimum( nLROffset - aNodeArgs.nLeftRest );
    aNodeArgs.nLeftRest -= aNodeArgs.nLeftDiff;
    if( aNodeArgs.nLeftRest < 0 )
        aNodeArgs.nMaxWidth -= aNodeArgs.nLeftRest;

    if( aNodeArgs.nRightRest < 0 )
        aNodeArgs.Minimum( rSpace.GetRight() - aNodeArgs.nRightRest );
    aNodeArgs.nRightRest -= aNodeArgs.nRightDiff;
    if( aNodeArgs.nRightRest < 0 )
        aNodeArgs.nMaxWidth -= aNodeArgs.nRightRest;

    SwScriptInfo aScriptInfo;
    SwAttrIter aIter( *const_cast<SwTextNode*>(this), aScriptInfo );
    sal_Int32 nIdx = 0;
    aIter.SeekAndChgAttrIter( nIdx, pOut );
    sal_Int32 nLen = m_Text.getLength();
    long nAktWidth = 0;
    long nAdd = 0;
    SwMinMaxArgs aArg( pOut, pSh, rMin, rAbsMin );
    while( nIdx < nLen )
    {
        sal_Int32 nNextChg = aIter.GetNextAttr();
        sal_Int32 nStop = aScriptInfo.NextScriptChg( nIdx );
        if( nNextChg > nStop )
            nNextChg = nStop;
        SwTextAttr *pHint = nullptr;
        sal_Unicode cChar = CH_BLANK;
        nStop = nIdx;
        while( nStop < nLen && nStop < nNextChg &&
               CH_TAB != ( cChar = m_Text[nStop] ) &&
               CH_BREAK != cChar && CHAR_HARDBLANK != cChar &&
               CHAR_HARDHYPHEN != cChar && CHAR_SOFTHYPHEN != cChar &&
               !pHint )
        {
            if( ( CH_TXTATR_BREAKWORD != cChar && CH_TXTATR_INWORD != cChar )
                || ( nullptr == ( pHint = aIter.GetAttr( nStop ) ) ) )
                ++nStop;
        }
        if ( lcl_MinMaxString( aArg, aIter.GetFnt(), m_Text, nIdx, nStop ) )
        {
            nAdd = 20;
        }
        nIdx = nStop;
        aIter.SeekAndChgAttrIter( nIdx, pOut );
        switch( cChar )
        {
            case CH_BREAK  :
            {
                if( static_cast<long>(rMax) < aArg.nRowWidth )
                    rMax = aArg.nRowWidth;
                aArg.nRowWidth = 0;
                aArg.NewWord();
                aIter.SeekAndChgAttrIter( ++nIdx, pOut );
            }
            break;
            case CH_TAB    :
            {
                aArg.NewWord();
                aIter.SeekAndChgAttrIter( ++nIdx, pOut );
            }
            break;
            case CHAR_SOFTHYPHEN:
                ++nIdx;
            break;
            case CHAR_HARDBLANK:
            case CHAR_HARDHYPHEN:
            {
                OUString sTmp( cChar );
                SwDrawTextInfo aDrawInf( getIDocumentLayoutAccess().GetCurrentViewShell(),
                    *pOut, nullptr, sTmp, 0, 1, 0, false );
                nAktWidth = aIter.GetFnt()->GetTextSize_( aDrawInf ).Width();
                aArg.nWordWidth += nAktWidth;
                aArg.nRowWidth += nAktWidth;
                if( static_cast<long>(rAbsMin) < aArg.nWordWidth )
                    rAbsMin = aArg.nWordWidth;
                aArg.Minimum( aArg.nWordWidth + aArg.nWordAdd );
                aArg.nNoLineBreak = nIdx++;
            }
            break;
            case CH_TXTATR_BREAKWORD:
            case CH_TXTATR_INWORD:
            {
                if( !pHint )
                    break;
                long nOldWidth = aArg.nWordWidth;
                long nOldAdd = aArg.nWordAdd;
                aArg.NewWord();

                switch( pHint->Which() )
                {
                    case RES_TXTATR_FLYCNT :
                    {
                        SwFrameFormat *pFrameFormat = pHint->GetFlyCnt().GetFrameFormat();
                        const SvxLRSpaceItem &rLR = pFrameFormat->GetLRSpace();
                        if( RES_DRAWFRMFMT == pFrameFormat->Which() )
                        {
                            const SdrObject* pSObj = pFrameFormat->FindSdrObject();
                            if( pSObj )
                                nAktWidth = pSObj->GetCurrentBoundRect().GetWidth();
                            else
                                nAktWidth = 0;
                        }
                        else
                        {
                            const SwFormatFrameSize& rTmpSize = pFrameFormat->GetFrameSize();
                            if( RES_FLYFRMFMT == pFrameFormat->Which()
                                && rTmpSize.GetWidthPercent() )
                            {
                                // This is a hack for the following situation: In the paragraph there's a
                                // text frame with relative size. Then let's take 0.5 cm as minimum width
                                // and USHRT_MAX as maximum width
                                // It were cleaner and maybe necessary later on to iterate over the content
                                // of the text frame and call GetMinMaxSize recursively
                                nAktWidth = FLYINCNT_MIN_WIDTH; // 0.5 cm
                                rMax = std::max(rMax, sal_uLong(USHRT_MAX));
                            }
                            else
                                nAktWidth = pFrameFormat->GetFrameSize().GetWidth();
                        }
                        nAktWidth += rLR.GetLeft();
                        nAktWidth += rLR.GetRight();
                        aArg.nWordAdd = nOldWidth + nOldAdd;
                        aArg.nWordWidth = nAktWidth;
                        aArg.nRowWidth += nAktWidth;
                        if( static_cast<long>(rAbsMin) < aArg.nWordWidth )
                            rAbsMin = aArg.nWordWidth;
                        aArg.Minimum( aArg.nWordWidth + aArg.nWordAdd );
                        break;
                    }
                    case RES_TXTATR_FTN :
                    {
                        const OUString aText = pHint->GetFootnote().GetNumStr();
                        if( lcl_MinMaxString( aArg, aIter.GetFnt(), aText, 0,
                            aText.getLength() ) )
                            nAdd = 20;
                        break;
                    }

                    case RES_TXTATR_FIELD :
                    case RES_TXTATR_ANNOTATION :
                        {
                            SwField *pField = const_cast<SwField*>(pHint->GetFormatField().GetField());
                            const OUString aText = pField->ExpandField(true);
                            if( lcl_MinMaxString( aArg, aIter.GetFnt(), aText, 0,
                                aText.getLength() ) )
                                nAdd = 20;
                            break;
                        }
                    default: aArg.nWordWidth = nOldWidth;
                        aArg.nWordAdd = nOldAdd;

                }
                aIter.SeekAndChgAttrIter( ++nIdx, pOut );
            }
            break;
        }
    }
    if( static_cast<long>(rMax) < aArg.nRowWidth )
        rMax = aArg.nRowWidth;

    nLROffset += rSpace.GetRight();

    rAbsMin += nLROffset;
    rAbsMin += nAdd;
    rMin += nLROffset;
    rMin += nAdd;
    if( static_cast<long>(rMin) < aNodeArgs.nMinWidth )
        rMin = aNodeArgs.nMinWidth;
    if( static_cast<long>(rAbsMin) < aNodeArgs.nMinWidth )
        rAbsMin = aNodeArgs.nMinWidth;
    rMax += aNodeArgs.nMaxWidth;
    rMax += nLROffset;
    rMax += nAdd;
    if( rMax < rMin ) // e.g. Frames with flow through only contribute to the minimum
        rMax = rMin;
    pOut->SetMapMode( aOldMap );
}

/**
 * Calculates the width of the text part specified by nStt and nEnd,
 * the height of the line containing nStt is divided by this width,
 * indicating the scaling factor, if the text part is rotated.
 * Having CH_BREAKs in the text part, this method returns the scaling
 * factor for the longest of the text parts separated by the CH_BREAK
 *
 * Changing this method very likely requires changing of "GetMinMaxSize"
 */
sal_uInt16 SwTextNode::GetScalingOfSelectedText( sal_Int32 nStt, sal_Int32 nEnd )
    const
{
    SwViewShell const * pSh = GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    OutputDevice* pOut = nullptr;

    if ( pSh )
        pOut = &pSh->GetRefDev();
    else
    {
        // Access via StarONE, there's no need for an existing or active shell
        if ( getIDocumentSettingAccess()->get(DocumentSettingId::HTML_MODE) )
            pOut = Application::GetDefaultDevice();
        else
            pOut = getIDocumentDeviceAccess().getReferenceDevice( true );
    }

    OSL_ENSURE( pOut, "GetScalingOfSelectedText without outdev" );

    MapMode aOldMap( pOut->GetMapMode() );
    pOut->SetMapMode( MapMode( MapUnit::MapTwip ) );

    if ( nStt == nEnd )
    {
        assert(g_pBreakIt && g_pBreakIt->GetBreakIter().is());

        SwScriptInfo aScriptInfo;
        SwAttrIter aIter( *const_cast<SwTextNode*>(this), aScriptInfo );
        aIter.SeekAndChgAttrIter( nStt, pOut );

        Boundary aBound =
            g_pBreakIt->GetBreakIter()->getWordBoundary( GetText(), nStt,
            g_pBreakIt->GetLocale( aIter.GetFnt()->GetLanguage() ),
            WordType::DICTIONARY_WORD, true );

        if ( nStt == aBound.startPos )
        {
            // cursor is at left or right border of word
            pOut->SetMapMode( aOldMap );
            return 100;
        }

        nStt = aBound.startPos;
        nEnd = aBound.endPos;

        if ( nStt == nEnd )
        {
            pOut->SetMapMode( aOldMap );
            return 100;
        }
    }

    SwScriptInfo aScriptInfo;
    SwAttrIter aIter( *const_cast<SwTextNode*>(this), aScriptInfo );

    // We do not want scaling attributes to be considered during this
    // calculation. For this, we push a temporary scaling attribute with
    // scaling value 100 and priority flag on top of the scaling stack
    SwAttrHandler& rAH = aIter.GetAttrHandler();
    SvxCharScaleWidthItem aItem(100, RES_CHRATR_SCALEW);
    SwTextAttrEnd aAttr( aItem, nStt, nEnd );
    aAttr.SetPriorityAttr( true );
    rAH.PushAndChg( aAttr, *(aIter.GetFnt()) );

    sal_Int32 nIdx = nStt;

    sal_uLong nWidth = 0;
    sal_uLong nProWidth = 0;

    while( nIdx < nEnd )
    {
        aIter.SeekAndChgAttrIter( nIdx, pOut );

        // scan for end of portion
        const sal_Int32 nNextChg = std::max(aIter.GetNextAttr(), aScriptInfo.NextScriptChg( nIdx ));

        sal_Int32 nStop = nIdx;
        sal_Unicode cChar = CH_BLANK;
        SwTextAttr* pHint = nullptr;

        // stop at special characters in [ nIdx, nNextChg ]
        while( nStop < nEnd && nStop < nNextChg )
        {
            cChar = m_Text[nStop];
            if (
                CH_TAB == cChar ||
                CH_BREAK == cChar ||
                CHAR_HARDBLANK == cChar ||
                CHAR_HARDHYPHEN == cChar ||
                CHAR_SOFTHYPHEN == cChar ||
                (
                  (CH_TXTATR_BREAKWORD == cChar || CH_TXTATR_INWORD == cChar) &&
                  (nullptr == (pHint = aIter.GetAttr(nStop)))
                )
               )
            {
                break;
            }
            else
                ++nStop;
        }

        // calculate text widths up to cChar
        if ( nStop > nIdx )
        {
            SwDrawTextInfo aDrawInf( pSh, *pOut, nullptr, GetText(), nIdx, nStop - nIdx );
            nProWidth += aIter.GetFnt()->GetTextSize_( aDrawInf ).Width();
        }

        nIdx = nStop;
        aIter.SeekAndChgAttrIter( nIdx, pOut );

        if ( cChar == CH_BREAK )
        {
            nWidth = std::max( nWidth, nProWidth );
            nProWidth = 0;
            nIdx++;
        }
        else if ( cChar == CH_TAB )
        {
            // tab receives width of one space
            OUString sTmp( CH_BLANK );
            SwDrawTextInfo aDrawInf( pSh, *pOut, nullptr, sTmp, 0, 1 );
            nProWidth += aIter.GetFnt()->GetTextSize_( aDrawInf ).Width();
            nIdx++;
        }
        else if ( cChar == CHAR_SOFTHYPHEN )
            ++nIdx;
        else if ( cChar == CHAR_HARDBLANK || cChar == CHAR_HARDHYPHEN )
        {
            OUString sTmp( cChar );
            SwDrawTextInfo aDrawInf( pSh, *pOut, nullptr, sTmp, 0, 1 );
            nProWidth += aIter.GetFnt()->GetTextSize_( aDrawInf ).Width();
            nIdx++;
        }
        else if ( pHint && ( cChar == CH_TXTATR_BREAKWORD || cChar == CH_TXTATR_INWORD ) )
        {
            switch( pHint->Which() )
            {
            case RES_TXTATR_FTN :
                {
                    const OUString aText = pHint->GetFootnote().GetNumStr();
                    SwDrawTextInfo aDrawInf( pSh, *pOut, nullptr, aText, 0, aText.getLength() );

                    nProWidth += aIter.GetFnt()->GetTextSize_( aDrawInf ).Width();
                    break;
                }

            case RES_TXTATR_FIELD :
            case RES_TXTATR_ANNOTATION :
                {
                    SwField *pField = const_cast<SwField*>(pHint->GetFormatField().GetField());
                    OUString const aText = pField->ExpandField(true);
                    SwDrawTextInfo aDrawInf( pSh, *pOut, nullptr, aText, 0, aText.getLength() );

                    nProWidth += aIter.GetFnt()->GetTextSize_( aDrawInf ).Width();
                    break;
                }

            default:
                {
                    // any suggestions for a default action?
                }
            } // end of switch
            nIdx++;
        } // end of while
    }

    nWidth = std::max( nWidth, nProWidth );

    // search for a text frame this node belongs to
    SwIterator<SwTextFrame,SwTextNode> aFrameIter( *this );
    SwTextFrame* pFrame = nullptr;
    for( SwTextFrame* pTmpFrame = aFrameIter.First(); pTmpFrame; pTmpFrame = aFrameIter.Next() )
    {
            if ( pTmpFrame->GetOfst() <= nStt &&
                ( !pTmpFrame->GetFollow() ||
                   pTmpFrame->GetFollow()->GetOfst() > nStt ) )
            {
                pFrame = pTmpFrame;
                break;
            }
        }

    // search for the line containing nStt
    if ( pFrame && pFrame->HasPara() )
    {
        SwTextInfo aInf( pFrame );
        SwTextIter aLine( pFrame, &aInf );
        aLine.CharToLine( nStt );
        pOut->SetMapMode( aOldMap );
        return static_cast<sal_uInt16>( nWidth ?
            ( ( 100 * aLine.GetCurr()->Height() ) / nWidth ) : 0 );
    }
    // no frame or no paragraph, we take the height of the character
    // at nStt as line height

    aIter.SeekAndChgAttrIter( nStt, pOut );
    pOut->SetMapMode( aOldMap );

    SwDrawTextInfo aDrawInf( pSh, *pOut, nullptr, GetText(), nStt, 1 );
    return static_cast<sal_uInt16>( nWidth ? ((100 * aIter.GetFnt()->GetTextSize_( aDrawInf ).Height()) / nWidth ) : 0 );
}

SwTwips SwTextNode::GetWidthOfLeadingTabs() const
{
    SwTwips nRet = 0;

    sal_Int32 nIdx = 0;

    while ( nIdx < GetText().getLength() )
    {
        const sal_Unicode cCh = GetText()[nIdx];
        if ( cCh!='\t' && cCh!=' ' )
        {
            break;
        }
        ++nIdx;
    }

    if ( nIdx > 0 )
    {
        SwPosition aPos( *this );
        aPos.nContent += nIdx;

        // Find the non-follow text frame:
        SwIterator<SwTextFrame,SwTextNode> aIter( *this );
        for( SwTextFrame* pFrame = aIter.First(); pFrame; pFrame = aIter.Next() )
        {
            // Only consider master frames:
            if ( !pFrame->IsFollow() )
            {
                SwRectFnSet aRectFnSet(pFrame);
                SwRect aRect;
                pFrame->GetCharRect( aRect, aPos );
                nRet = pFrame->IsRightToLeft() ?
                            aRectFnSet.GetPrtRight(*pFrame) - aRectFnSet.GetRight(aRect) :
                            aRectFnSet.GetLeft(aRect) - aRectFnSet.GetPrtLeft(*pFrame);
                break;
            }
        }
    }

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
