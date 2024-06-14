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

#include <memory>
#include <utility>
#include <hintids.hxx>
#include <comphelper/string.hxx>
#include <svl/itemiter.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <svx/svdobj.hxx>
#include <osl/diagnose.h>
#include <crsrsh.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <pagefrm.hxx>
#include <cntfrm.hxx>
#include <rootfrm.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <fldbas.hxx>
#include <swtable.hxx>
#include <docary.hxx>
#include <txtfld.hxx>
#include <fmtfld.hxx>
#include <txtftn.hxx>
#include <txtinet.hxx>
#include <fmtinfmt.hxx>
#include <txttxmrk.hxx>
#include <frmfmt.hxx>
#include <flyfrm.hxx>
#include <viscrs.hxx>
#include "callnk.hxx"
#include <doctxm.hxx>
#include <docfld.hxx>
#include <expfld.hxx>
#include <reffld.hxx>
#include <flddat.hxx>
#include <cellatr.hxx>
#include <swundo.hxx>
#include <redline.hxx>
#include <fmtcntnt.hxx>
#include <fmthdft.hxx>
#include <pagedesc.hxx>
#include <fesh.hxx>
#include <charfmt.hxx>
#include <fmturl.hxx>
#include <txtfrm.hxx>
#include <wrong.hxx>
#include <calbck.hxx>
#include <unotools/intlwrapper.hxx>
#include <docufld.hxx>
#include <svx/srchdlg.hxx>
#include <frameformats.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <textcontentcontrol.hxx>

using namespace ::com::sun::star;

void SwCursorShell::MoveCursorToNum()
{
    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursorSaveState aSaveState( *m_pCurrentCursor );
    if( ActionPend() )
        return;
    CurrShell aCurr( this );
    // try to set cursor onto this position, at half of the char-
    // SRectangle's height
    Point aPt( m_pCurrentCursor->GetPtPos() );
    std::pair<Point, bool> const tmp(aPt, true);
    SwContentFrame * pFrame = m_pCurrentCursor->GetPointContentNode()->getLayoutFrame(
                GetLayout(), m_pCurrentCursor->GetPoint(), &tmp);
    pFrame->GetCharRect( m_aCharRect, *m_pCurrentCursor->GetPoint() );
    pFrame->Calc(GetOut());
    if( pFrame->IsVertical() )
    {
        aPt.setX(m_aCharRect.Center().getX());
        aPt.setY(pFrame->getFrameArea().Top() + GetUpDownX());
    }
    else
    {
        aPt.setY(m_aCharRect.Center().getY());
        aPt.setX(pFrame->getFrameArea().Left() + GetUpDownX());
    }
    pFrame->GetModelPositionForViewPoint( m_pCurrentCursor->GetPoint(), aPt );
    if ( !m_pCurrentCursor->IsSelOvr( SwCursorSelOverFlags::Toggle |
                                SwCursorSelOverFlags::ChangePos ))
    {
        UpdateCursor(SwCursorShell::UPDOWN |
                SwCursorShell::SCROLLWIN | SwCursorShell::CHKRANGE |
                SwCursorShell::READONLY );
    }
}

/// go to next/previous point on the same level
void SwCursorShell::GotoNextNum()
{
    if (!SwDoc::GotoNextNum(*m_pCurrentCursor->GetPoint(), GetLayout()))
        return;
    MoveCursorToNum();
}

void SwCursorShell::GotoPrevNum()
{
    if (!SwDoc::GotoPrevNum(*m_pCurrentCursor->GetPoint(), GetLayout()))
        return;
    MoveCursorToNum();
}

/// jump from content to header
bool SwCursorShell::GotoHeaderText()
{
    const SwFrame* pFrame = GetCurrFrame()->FindPageFrame();
    while( pFrame && !pFrame->IsHeaderFrame() )
        pFrame = pFrame->GetLower();
    // found header, search 1. content frame
    while( pFrame && !pFrame->IsContentFrame() )
        pFrame = pFrame->GetLower();

    if( !pFrame )
        return false;

    CurrShell aCurr( this );
    // get header frame
    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursor *pTmpCursor = getShellCursor( true );
    SwCursorSaveState aSaveState( *pTmpCursor );
    pFrame->Calc(GetOut());
    Point aPt( pFrame->getFrameArea().Pos() + pFrame->getFramePrintArea().Pos() );
    pFrame->GetModelPositionForViewPoint( pTmpCursor->GetPoint(), aPt );
    if( !pTmpCursor->IsSelOvr() )
        UpdateCursor();
    else
        pFrame = nullptr;
    return nullptr != pFrame;
}

/// jump from content to footer
bool SwCursorShell::GotoFooterText()
{
    const SwPageFrame* pFrame = GetCurrFrame()->FindPageFrame();
    if( !pFrame )
        return false;

    const SwFrame* pLower = pFrame->GetLastLower();

    while( pLower && !pLower->IsFooterFrame() )
        pLower = pLower->GetLower();
    // found footer, search 1. content frame
    while( pLower && !pLower->IsContentFrame() )
        pLower = pLower->GetLower();

    if( !pLower )
        return false;

    SwCursor *pTmpCursor = getShellCursor( true );
    CurrShell aCurr( this );
    // get position in footer
    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursorSaveState aSaveState( *pTmpCursor );
    pLower->Calc(GetOut());
    Point aPt( pLower->getFrameArea().Pos() + pLower->getFramePrintArea().Pos() );
    pLower->GetModelPositionForViewPoint( pTmpCursor->GetPoint(), aPt );
    if( !pTmpCursor->IsSelOvr() )
        UpdateCursor();
    else
        pFrame = nullptr;
    return nullptr != pFrame;
}

bool SwCursorShell::SetCursorInHdFt(size_t nDescNo, bool bInHeader, bool bEven, bool bFirst)
{
    SwDoc *pMyDoc = GetDoc();
    const SwPageDesc* pDesc = nullptr;

    CurrShell aCurr( this );

    if( SIZE_MAX == nDescNo )
    {
        // take the current one
        const SwContentFrame *pCurrFrame = GetCurrFrame();
        const SwPageFrame* pPage = (pCurrFrame == nullptr) ? nullptr : pCurrFrame->FindPageFrame();
        if( pPage && pMyDoc->ContainsPageDesc(
                pPage->GetPageDesc(), &nDescNo) )
            pDesc = pPage->GetPageDesc();
    }
    else
        if (nDescNo < pMyDoc->GetPageDescCnt())
            pDesc = &pMyDoc->GetPageDesc( nDescNo );

    if( !pDesc )
        return false;

    // check if the attribute exists
    const SwFormatContent* pCnt = nullptr;
    if( bInHeader )
    {
        const SwFormatHeader& rHd
            = bEven ? bFirst ? pDesc->GetFirstLeft().GetHeader() : pDesc->GetLeft().GetHeader()
                    : bFirst ? pDesc->GetFirstMaster().GetHeader() : pDesc->GetMaster().GetHeader();
        if( rHd.GetHeaderFormat() )
            pCnt = &rHd.GetHeaderFormat()->GetContent();
    }
    else
    {
        const SwFormatFooter& rFt
            = bEven ? bFirst ? pDesc->GetFirstLeft().GetFooter() : pDesc->GetLeft().GetFooter()
                    : bFirst ? pDesc->GetFirstMaster().GetFooter() : pDesc->GetMaster().GetFooter();
        if( rFt.GetFooterFormat() )
            pCnt = &rFt.GetFooterFormat()->GetContent();
    }

    if( !pCnt || !pCnt->GetContentIdx() )
        return false;

    SwNodeIndex aIdx( *pCnt->GetContentIdx(), 1 );
    SwContentNode* pCNd = aIdx.GetNode().GetContentNode();
    if( !pCNd )
        pCNd = pMyDoc->GetNodes().GoNext( &aIdx );

    Point aPt( m_pCurrentCursor->GetPtPos() );

    std::pair<Point, bool> const tmp(aPt, false);
    if (!pCNd || nullptr == pCNd->getLayoutFrame(GetLayout(), nullptr, &tmp))
        return false;

    // then we can set the cursor in here
    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursorSaveState aSaveState( *m_pCurrentCursor );

    ClearMark();

    SwPosition& rPos = *m_pCurrentCursor->GetPoint();
    rPos.Assign( *pCNd );

    if (m_pCurrentCursor->IsSelOvr())
        return false;

    UpdateCursor( SwCursorShell::SCROLLWIN | SwCursorShell::CHKRANGE |
                SwCursorShell::READONLY );
    return true;
}

/// jump to the next index
bool SwCursorShell::GotoNextTOXBase( const OUString* pName )
{
    const SwSectionFormats& rFormats = GetDoc()->GetSections();
    SwContentNode* pFnd = nullptr;
    for( SwSectionFormats::size_type n = rFormats.size(); n; )
    {
        const SwSection* pSect = rFormats[ --n ]->GetSection();
        if (SectionType::ToxContent == pSect->GetType())
        {
            SwSectionNode const*const pSectNd(
                    pSect->GetFormat()->GetSectionNode());
            if (   pSectNd
                && m_pCurrentCursor->GetPoint()->GetNode() < *pSectNd
                && (!pFnd  || pFnd->GetIndex() > pSectNd->GetIndex())
                && (!pName || *pName ==
                    static_cast<SwTOXBaseSection const*>(pSect)->GetTOXName()))
            {
                SwNodeIndex aIdx(*pSectNd, 1);
                SwContentNode* pCNd = aIdx.GetNode().GetContentNode();
                if (!pCNd)
                    pCNd = GetDoc()->GetNodes().GoNext( &aIdx );
                if (pCNd &&
                    pCNd->EndOfSectionIndex() <= pSectNd->EndOfSectionIndex())
                {
                    SwContentFrame const*const pCFrame(
                            pCNd->getLayoutFrame(GetLayout()));
                    if (pCFrame &&
                        (IsReadOnlyAvailable() || !pCFrame->IsProtected()))
                    {
                        pFnd = pCNd;
                    }
                }
            }
        }
    }
    if( !pFnd )
        return false;
    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursorSaveState aSaveState( *m_pCurrentCursor );
    m_pCurrentCursor->GetPoint()->Assign( *pFnd );
    bool bRet = !m_pCurrentCursor->IsSelOvr();
    if( bRet )
        UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
    return bRet;
}

/// jump to previous index
bool SwCursorShell::GotoPrevTOXBase( const OUString* pName )
{
    const SwSectionFormats& rFormats = GetDoc()->GetSections();
    SwContentNode* pFnd = nullptr;
    for( SwSectionFormats::size_type n = rFormats.size(); n; )
    {
        const SwSection* pSect = rFormats[ --n ]->GetSection();
        if (SectionType::ToxContent == pSect->GetType())
        {
            SwSectionNode const*const pSectNd(
                    pSect->GetFormat()->GetSectionNode());
            if (   pSectNd
                && m_pCurrentCursor->GetPoint()->GetNode() > *pSectNd->EndOfSectionNode()
                && (!pFnd  || *pFnd < *pSectNd)
                && (!pName || *pName ==
                    static_cast<SwTOXBaseSection const*>(pSect)->GetTOXName()))
            {
                SwNodeIndex aIdx(*pSectNd, 1);
                SwContentNode* pCNd = aIdx.GetNode().GetContentNode();
                if (!pCNd)
                    pCNd = GetDoc()->GetNodes().GoNext( &aIdx );
                if (pCNd &&
                    pCNd->EndOfSectionIndex() <= pSectNd->EndOfSectionIndex())
                {
                    SwContentFrame const*const pCFrame(
                            pCNd->getLayoutFrame(GetLayout()));
                    if (pCFrame &&
                        (IsReadOnlyAvailable() || !pCFrame->IsProtected()))
                    {
                        pFnd = pCNd;
                    }
                }
            }
        }
    }

    if( !pFnd )
        return false;

    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursorSaveState aSaveState( *m_pCurrentCursor );
    m_pCurrentCursor->GetPoint()->Assign(*pFnd);
    bool bRet = !m_pCurrentCursor->IsSelOvr();
    if( bRet )
        UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
    return bRet;
}

/// jump to index of TOXMark
void SwCursorShell::GotoTOXMarkBase()
{
    SwTOXMarks aMarks;
    sal_uInt16 nCnt = SwDoc::GetCurTOXMark(*m_pCurrentCursor->GetPoint(), aMarks);
    if(!nCnt)
        return;
    // Take the 1. and get the index type. Ask it for the actual index.
    const SwTOXType* pType = aMarks[0]->GetTOXType();
    auto pContentFrame = pType->FindContentFrame(*GetDoc(), *GetLayout());
    if(!pContentFrame)
        return;
    SwCallLink aLk(*this); // watch Cursor-Moves
    SwCursorSaveState aSaveState(*m_pCurrentCursor);
    assert(pContentFrame->IsTextFrame());
    *m_pCurrentCursor->GetPoint() = static_cast<SwTextFrame const*>(pContentFrame)->MapViewToModelPos(TextFrameIndex(0));
    if(!m_pCurrentCursor->IsInProtectTable() && !m_pCurrentCursor->IsSelOvr())
        UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
}

/// Jump to next/previous table formula
/// Optionally it is possible to also jump to broken formulas
bool SwCursorShell::GotoNxtPrvTableFormula( bool bNext, bool bOnlyErrors )
{
    SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::Empty );

    if( IsTableMode() )
        return false;

    bool bFnd = false;
    SwPosition aOldPos = *m_pCurrentCursor->GetPoint();
    SwPosition& rPos = *m_pCurrentCursor->GetPoint();

    Point aPt;
    SwPosition aFndPos( GetDoc()->GetNodes().GetEndOfContent() );
    if( !bNext )
        aFndPos.Assign(SwNodeOffset(0));
    SetGetExpField aFndGEF( aFndPos ), aCurGEF( rPos );

    {
        const SwNode* pSttNd = rPos.GetNode().FindTableBoxStartNode();
        if( pSttNd )
        {
            const SwTableBox* pTBox = pSttNd->FindTableNode()->GetTable().
                                        GetTableBox( pSttNd->GetIndex() );
            if( pTBox )
                aCurGEF = SetGetExpField( *pTBox );
        }
    }

    if( rPos.GetNode() < GetDoc()->GetNodes().GetEndOfExtras() )
    {
        // also at collection use only the first frame
        std::pair<Point, bool> const tmp(aPt, false);
        aCurGEF.SetBodyPos( *rPos.GetNode().GetContentNode()->getLayoutFrame( GetLayout(),
                                &rPos, &tmp) );
    }

    const registeredSfxPoolItems& rSurrogates(GetDoc()->GetAttrPool().GetItemSurrogates(RES_BOXATR_FORMULA));
    const sal_uInt32 nMaxItems(rSurrogates.size());
    if( nMaxItems > 0 )
    {
        sal_uInt8 nMaxDo = 2;
        do {
            for (const SfxPoolItem* pItem : rSurrogates)
            {
                const SwTableBox* pTBox;
                auto pFormulaItem = dynamic_cast<const SwTableBoxFormula*>(pItem);
                if( !pFormulaItem )
                    continue;
                pTBox = pFormulaItem->GetTableBox();
                if( pTBox &&
                    pTBox->GetSttNd() &&
                    pTBox->GetSttNd()->GetNodes().IsDocNodes() &&
                    ( !bOnlyErrors ||
                      !pFormulaItem->HasValidBoxes() ) )
                {
                    SwNodeIndex aIdx( *pTBox->GetSttNd() );
                    const SwContentNode* pCNd = GetDoc()->GetNodes().GoNext( &aIdx );
                    std::pair<Point, bool> const tmp(aPt, false);
                    if (pCNd)
                    {
                        const SwContentFrame* pCFrame = pCNd->getLayoutFrame(GetLayout(), nullptr, &tmp);
                        if (pCFrame && (IsReadOnlyAvailable() || !pCFrame->IsProtected() ))
                        {
                            SetGetExpField aCmp( *pTBox );
                            aCmp.SetBodyPos( *pCFrame );

                            if( bNext ? ( aCurGEF < aCmp && aCmp < aFndGEF )
                                    : ( aCmp < aCurGEF && aFndGEF < aCmp ))
                            {
                                aFndGEF = aCmp;
                                bFnd = true;
                            }
                        }
                    }
                }
            }
            if( !bFnd )
            {
                if( bNext )
                {
                    rPos.Assign(SwNodeOffset(0), 0);
                    aCurGEF = SetGetExpField( rPos );
                    SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::EndWrapped );
                }
                else
                {
                    aCurGEF = SetGetExpField( SwPosition( GetDoc()->GetNodes().GetEndOfContent() ) );
                    SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::StartWrapped );
                }
            }
        } while( !bFnd && --nMaxDo );
    }

    if( !bFnd )
    {
        rPos = aOldPos;
        SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::NavElementNotFound );
        return false;
    }

    CurrShell aCurr( this );
    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursorSaveState aSaveState( *m_pCurrentCursor );

    aFndGEF.GetPosOfContent( rPos );
    m_pCurrentCursor->DeleteMark();

    bFnd = !m_pCurrentCursor->IsSelOvr();
    if( bFnd )
        UpdateCursor( SwCursorShell::SCROLLWIN | SwCursorShell::CHKRANGE |
                    SwCursorShell::READONLY );

    return bFnd;
}

/// jump to next/previous index marker
bool SwCursorShell::GotoNxtPrvTOXMark( bool bNext )
{
    SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::Empty );

    if( IsTableMode() )
        return false;

    bool bFnd = false;
    SwPosition& rPos = *m_pCurrentCursor->GetPoint();

    Point aPt;
    SwPosition aFndPos( GetDoc()->GetNodes().GetEndOfContent() );
    if( !bNext )
        aFndPos.Assign(SwNodeOffset(0));
    SetGetExpField aFndGEF( aFndPos ), aCurGEF( rPos );

    if( rPos.GetNodeIndex() < GetDoc()->GetNodes().GetEndOfExtras().GetIndex() )
    {
        // also at collection use only the first frame
        std::pair<Point, bool> const tmp(aPt, false);
        aCurGEF.SetBodyPos( *rPos.GetNode().
                    GetContentNode()->getLayoutFrame(GetLayout(), &rPos, &tmp));
    }

    const SwTextNode* pTextNd;
    const SwTextTOXMark* pTextTOX;
    const registeredSfxPoolItems& rSurrogates(GetDoc()->GetAttrPool().GetItemSurrogates(RES_TXTATR_TOXMARK));
    const sal_uInt32 nMaxItems(rSurrogates.size());
    if( nMaxItems == 0 )
    {
        SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::NavElementNotFound );
        return false;
    }

    do {
        for (const SfxPoolItem* pItem : rSurrogates)
        {
            auto pToxMarkItem = dynamic_cast<const SwTOXMark*>(pItem);
            if( !pToxMarkItem )
                continue;
            pTextTOX = pToxMarkItem->GetTextTOXMark();
            if( !pTextTOX )
                continue;
            pTextNd = &pTextTOX->GetTextNode();
            if( !pTextNd->GetNodes().IsDocNodes() )
                continue;
            std::pair<Point, bool> const tmp(aPt, false);
            const SwContentFrame* pCFrame = pTextNd->getLayoutFrame(GetLayout(), nullptr, &tmp);
            if( pCFrame && ( IsReadOnlyAvailable() || !pCFrame->IsProtected() ))
            {
                SetGetExpField aCmp( *pTextNd, *pTextTOX );
                aCmp.SetBodyPos( *pCFrame );

                if( bNext ? ( aCurGEF < aCmp && aCmp < aFndGEF )
                          : ( aCmp < aCurGEF && aFndGEF < aCmp ))
                {
                    aFndGEF = aCmp;
                    bFnd = true;
                }
            }
        }
        if( !bFnd )
        {
            if ( bNext )
            {
                rPos.Assign(SwNodeOffset(0), 0);
                aCurGEF = SetGetExpField( rPos );
                SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::EndWrapped );
            }
            else
            {
                aCurGEF = SetGetExpField( SwPosition( GetDoc()->GetNodes().GetEndOfContent() ) );
                SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::StartWrapped );
            }
        }
    } while ( !bFnd );

    CurrShell aCurr( this );
    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursorSaveState aSaveState( *m_pCurrentCursor );

    aFndGEF.GetPosOfContent( rPos );

    bFnd = !m_pCurrentCursor->IsSelOvr();
    if( bFnd )
        UpdateCursor( SwCursorShell::SCROLLWIN | SwCursorShell::CHKRANGE |
                    SwCursorShell::READONLY );
    return bFnd;
}

/// traveling between marks
const SwTOXMark& SwCursorShell::GotoTOXMark( const SwTOXMark& rStart,
                                            SwTOXSearch eDir )
{
    CurrShell aCurr( this );
    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursorSaveState aSaveState( *m_pCurrentCursor );

    const SwTOXMark& rNewMark = GetDoc()->GotoTOXMark( rStart, eDir,
                                                    IsReadOnlyAvailable() );
    // set position
    SwPosition& rPos = *GetCursor()->GetPoint();
    rPos.Assign(rNewMark.GetTextTOXMark()->GetTextNode(),
                 rNewMark.GetTextTOXMark()->GetStart() );
    GetCursor()->DeleteMark(); // tdf#158783 prevent UpdateCursor resetting point

    if( !m_pCurrentCursor->IsSelOvr() )
        UpdateCursor( SwCursorShell::SCROLLWIN | SwCursorShell::CHKRANGE |
                    SwCursorShell::READONLY );

    return rNewMark;
}

/// jump to next/previous field type
static void lcl_MakeFieldLst(
    SetGetExpFields& rLst,
    const SwFieldType& rFieldType,
    const bool bInReadOnly,
    const bool bChkInpFlag = false )
{
    // always search the 1. frame
    Point aPt;
    std::vector<SwFormatField*> vFields;
    rFieldType.GatherFields(vFields, false);
    for(SwFormatField* pFormatField: vFields)
    {
        SwTextField* pTextField = pFormatField->GetTextField();
        if ( pTextField != nullptr
             && ( !bChkInpFlag
                  || static_cast<const SwSetExpField*>(pTextField->GetFormatField().GetField())->GetInputFlag() ) )
        {
            const SwTextNode& rTextNode = pTextField->GetTextNode();
            std::pair<Point, bool> const tmp(aPt, false);
            const SwContentFrame* pCFrame =
                rTextNode.getLayoutFrame(
                    rTextNode.GetDoc().getIDocumentLayoutAccess().GetCurrentLayout(),
                    nullptr, &tmp);
            if ( pCFrame != nullptr
                 && ( bInReadOnly || !pCFrame->IsProtected() ) )
            {
                std::unique_ptr<SetGetExpField> pNew(new SetGetExpField( rTextNode, pTextField ));
                pNew->SetBodyPos( *pCFrame );
                rLst.insert( std::move(pNew) );
            }
        }
    }
}

static SetGetExpFields::const_iterator
lcl_FindField(bool & o_rFound, SetGetExpFields const& rSrtLst,
        SwRootFrame const *const pLayout, SwTextNode *const pTextNode,
        SwTextField const *const pTextField, SwPosition const& rPos,
        sal_Int32 const nContentOffset)
{
    std::optional<SetGetExpField> oSrch;
    if (-1 == nContentOffset)
    {
        oSrch.emplace(rPos.GetNode(), pTextField, rPos.GetContentIndex());
    }
    else
    {
        oSrch.emplace(rPos.GetNode(), pTextField, nContentOffset);
    }

    if (rPos.GetNodeIndex() < pTextNode->GetNodes().GetEndOfExtras().GetIndex())
    {
        // also at collection use only the first frame
        Point aPt;
        std::pair<Point, bool> const tmp(aPt, false);
        oSrch->SetBodyPos(*pTextNode->getLayoutFrame(pLayout, &rPos, &tmp));
    }

    SetGetExpFields::const_iterator it = rSrtLst.lower_bound(&*oSrch);

    o_rFound = (it != rSrtLst.end()) && (**it == *oSrch);
    return it;
}

bool SwCursorShell::MoveFieldType(
    const SwFieldType* pFieldType,
    const bool bNext,
    const SwFieldIds nResType,
    const bool bAddSetExpressionFieldsToInputFields )
{
    // sorted list of all fields
    SetGetExpFields aSrtLst;

    if ( pFieldType )
    {
        if( SwFieldIds::Input != pFieldType->Which() && !pFieldType->HasWriterListeners() )
        {
            return false;
        }

        // found Modify object, add all fields to array
        ::lcl_MakeFieldLst( aSrtLst, *pFieldType, IsReadOnlyAvailable() );

        if( SwFieldIds::Input == pFieldType->Which() && bAddSetExpressionFieldsToInputFields )
        {
            // there are hidden input fields in the set exp. fields
            const SwFieldTypes& rFieldTypes = *mxDoc->getIDocumentFieldsAccess().GetFieldTypes();
            const size_t nSize = rFieldTypes.size();
            for( size_t i=0; i < nSize; ++i )
            {
                pFieldType = rFieldTypes[ i ].get();
                if ( SwFieldIds::SetExp == pFieldType->Which() )
                {
                    ::lcl_MakeFieldLst( aSrtLst, *pFieldType, IsReadOnlyAvailable(), true );
                }
            }
        }
    }
    else
    {
        const SwFieldTypes& rFieldTypes = *mxDoc->getIDocumentFieldsAccess().GetFieldTypes();
        const size_t nSize = rFieldTypes.size();
        const bool bAllFieldTypes = nResType == SwFieldIds::Unknown;
        for( size_t i=0; i < nSize; ++i )
        {
            pFieldType = rFieldTypes[ i ].get();
            if (bAllFieldTypes || nResType == pFieldType->Which())
            {
                ::lcl_MakeFieldLst( aSrtLst, *pFieldType, IsReadOnlyAvailable() );
            }
        }
    }

    // found no fields?
    if( aSrtLst.empty() )
        return false;

    SetGetExpFields::const_iterator it;
    SwCursor* pCursor = getShellCursor( true );
    {
        // (1998): Always use field for search so that the right one is found as
        // well some are in frames that are anchored to a paragraph that has a
        // field
        const SwPosition& rPos = *pCursor->GetPoint();

        SwTextNode* pTNd = rPos.GetNode().GetTextNode();
        OSL_ENSURE( pTNd, "No ContentNode" );

        SwTextField * pTextField = pTNd->GetFieldTextAttrAt(rPos.GetContentIndex(), ::sw::GetTextAttrMode::Default);
        const bool bDelField = ( pTextField == nullptr );
        sal_Int32 nContentOffset = -1;

        if( bDelField )
        {
            // create dummy for the search
            SwFormatField* pFormatField = new SwFormatField( SwDateTimeField(
                static_cast<SwDateTimeFieldType*>(mxDoc->getIDocumentFieldsAccess().GetSysFieldType( SwFieldIds::DateTime ) ) ) );

            pTextField = new SwTextField( *pFormatField, rPos.GetContentIndex(),
                        mxDoc->IsClipBoard() );
            pTextField->ChgTextNode( pTNd );
        }
        else
        {
            // the cursor might be anywhere inside the input field,
            // but we will be searching for the field start
            if (pTextField->Which() == RES_TXTATR_INPUTFIELD
                    && rPos.GetContentIndex() != pTextField->GetStart())
                nContentOffset = pTextField->GetStart();
        }
        bool isSrch;
        it = lcl_FindField(isSrch, aSrtLst,
                GetLayout(), pTNd, pTextField, rPos, nContentOffset);

        if( bDelField )
        {
            auto const pFormat(static_cast<SwFormatField*>(&pTextField->GetAttr()));
            delete pTextField;
            delete pFormat;
        }

        if( it != aSrtLst.end() && isSrch ) // found
        {
            if( bNext )
            {
                if( ++it == aSrtLst.end() )
                    return false; // already at the end
            }
            else
            {
                if( it == aSrtLst.begin() )
                    return false; // no more steps backward possible
                --it;
            }
        }
        else // not found
        {
            if( bNext )
            {
                if( it == aSrtLst.end() )
                    return false;
            }
            else
            {
                if( it == aSrtLst.begin() )
                    return false; // no more steps backward possible
                --it;
            }
        }
    }
    const SetGetExpField& rFnd = **it;

    CurrShell aCurr( this );
    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursorSaveState aSaveState( *pCursor );

    rFnd.GetPosOfContent( *pCursor->GetPoint() );
    bool bRet = !m_pCurrentCursor->IsSelOvr( SwCursorSelOverFlags::CheckNodeSection |
                                     SwCursorSelOverFlags::Toggle );
    if( bRet )
        UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
    return bRet;
}

bool SwCursorShell::GotoFootnoteAnchor(const SwTextFootnote& rTextFootnote)
{
    if (SwWrtShell* pWrtSh = dynamic_cast<SwWrtShell*>(this))
        pWrtSh->addCurrentPosition();

    bool bRet = false;
    SwCursor* pCursor = getShellCursor(true);

    CurrShell aCurr(this);
    SwCallLink aLk(*this); // watch Cursor-Moves
    SwCursorSaveState aSaveState(*pCursor);

    pCursor->GetPoint()->Assign(rTextFootnote.GetTextNode(),
                                rTextFootnote.GetStart());
    bRet = !pCursor->IsSelOvr();
    if (bRet)
        UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
    return bRet;
}

bool SwCursorShell::GotoFormatContentControl(const SwFormatContentControl& rContentControl)
{
    std::shared_ptr<SwContentControl> pContentControl = rContentControl.GetContentControl();
    const SwTextContentControl* pTextContentControl = pContentControl->GetTextAttr();
    if (!pTextContentControl)
        return false;

    CurrShell aCurr(this);
    SwCallLink aLink(*this);

    SwCursor* pCursor = getShellCursor(true);
    SwCursorSaveState aSaveState(*pCursor);

    SwTextNode* pTextNode = pContentControl->GetTextNode();
    // Don't select the text attribute itself at the start.
    sal_Int32 nStart = pTextContentControl->GetStart() + 1;
    pCursor->GetPoint()->Assign(*pTextNode, nStart);

    bool bRet = true;
    // select contents for certain controls or conditions
    if (pContentControl->GetShowingPlaceHolder() || pContentControl->GetCheckbox()
        || pContentControl->GetSelectedListItem() || pContentControl->GetSelectedDate())
    {
        pCursor->SetMark();
        // Don't select the CH_TXTATR_BREAKWORD itself at the end.
        sal_Int32 nEnd = *pTextContentControl->End() - 1;
        pCursor->GetMark()->Assign(*pTextNode, nEnd);
        bRet = !pCursor->IsSelOvr();
    }
    else
        ClearMark();

    if (bRet)
    {
        UpdateCursor(SwCursorShell::SCROLLWIN | SwCursorShell::CHKRANGE
                     | SwCursorShell::READONLY);
    }

    return bRet;
}

/**
 * Go to the next (or previous) form control, based first on tabIndex and then paragraph position,
 * where a tabIndex of 1 is first, 0 is last, and -1 is excluded.
 */
void SwCursorShell::GotoFormControl(bool bNext)
{
    // (note: this only applies to modern content controls and legacy fieldmarks,
    //  since activeX richText controls aren't exposed to SW keystrokes)

    struct FormControlSort
    {
        bool operator()(std::pair<const SwPosition&, sal_uInt32> rLHS,
                        std::pair<const SwPosition&, sal_uInt32> rRHS) const
        {
            assert(rLHS.second && rRHS.second && "tabIndex zero must be changed to SAL_MAX_UINT32");
            //first compare tabIndexes where 1 has the priority.
            if (rLHS.second < rRHS.second)
                return true;
            if (rLHS.second > rRHS.second)
                return false;

            // when tabIndexes are equal (and they usually are) then sort by paragraph position
            return rLHS.first < rRHS.first;
        }
    };
    std::map<std::pair<SwPosition, sal_uInt32>,
             std::pair<SwTextContentControl*, sw::mark::IFieldmark*>, FormControlSort>  aFormMap;

    // add all of the eligible modern Content Controls into a sorted map
    SwContentControlManager& rManager = GetDoc()->GetContentControlManager();
    for (size_t  i = 0; i < rManager.GetCount(); ++i)
    {
        SwTextContentControl* pTCC = rManager.UnsortedGet(i);
        if (!pTCC || !pTCC->GetTextNode())
            continue;
        auto pCC = pTCC->GetContentControl().GetContentControl();

        // -1 indicates the control should not participate in keyboard tab navigation
        if (pCC && pCC->GetTabIndex() == SAL_MAX_UINT32)
            continue;

        const SwPosition nPos(*pTCC->GetTextNode(), pTCC->GetStart());

        // since 0 is the lowest priority (1 is the highest), and -1 has already been excluded,
        // use SAL_MAX_UINT32 as zero's tabIndex so that automatic sorting is correct.
        sal_uInt32 nTabIndex = pCC && pCC->GetTabIndex() ? pCC->GetTabIndex() : SAL_MAX_UINT32;

        const std::pair<SwTextContentControl*, sw::mark::IFieldmark*> pFormControl(pTCC, nullptr);
        aFormMap[std::make_pair(nPos, nTabIndex)] = pFormControl;
    }

    if (aFormMap.begin() == aFormMap.end())
    {
        // only legacy fields exist. Avoid reprocessing everything and use legacy code path.
        GotoFieldmark(bNext ? GetFieldmarkAfter(/*Loop=*/true) : GetFieldmarkBefore(/*Loop=*/true));
        return;
    }

    // add all of the legacy form field controls into the sorted map
    IDocumentMarkAccess* pMarkAccess = GetDoc()->getIDocumentMarkAccess();
    for (auto it = pMarkAccess->getFieldmarksBegin(); it != pMarkAccess->getFieldmarksEnd(); ++it)
    {
        auto pFieldMark = dynamic_cast<sw::mark::IFieldmark*>(*it);
        assert(pFieldMark);
        std::pair<SwTextContentControl*, sw::mark::IFieldmark*> pFormControl(nullptr, pFieldMark);
        // legacy form fields do not have (functional) tabIndexes - use lowest priority for them
        aFormMap[std::make_pair((*it)->GetMarkStart(), SAL_MAX_UINT32)] = pFormControl;
    }

    if (aFormMap.begin() == aFormMap.end())
        return;

    // Identify the current location in the document, and the current tab index priority

    // A content control could contain a Fieldmark, so check for legacy fieldmarks first
    sw::mark::IFieldmark* pFieldMark = GetCurrentFieldmark();
    SwTextContentControl* pTCC = !pFieldMark ? CursorInsideContentControl() : nullptr;

    auto pCC = pTCC ? pTCC->GetContentControl().GetContentControl() : nullptr;
    const sal_Int32 nCurTabIndex = pCC && pCC->GetTabIndex() ? pCC->GetTabIndex() : SAL_MAX_UINT32;

    SwPosition nCurPos(*GetCursor()->GetPoint());
    if (pFieldMark)
        nCurPos = pFieldMark->GetMarkStart();
    else if (pTCC && pTCC->GetTextNode())
        nCurPos = SwPosition(*pTCC->GetTextNode(), pTCC->GetStart());

    // Find the previous (or next) tab control and navigate to it
    const std::pair<SwPosition, sal_uInt32> nOldPos(nCurPos, nCurTabIndex);

    // lower_bound acts like find, and returns a pointer to nFindPos if it exists,
    // otherwise it will point to the previous entry.
    auto aNewPos = aFormMap.lower_bound(nOldPos);
    if (bNext && aNewPos != aFormMap.end())
        ++aNewPos;
    else if (!bNext && aNewPos != aFormMap.end() && aNewPos->first == nOldPos)
    {
        // Found the current position - need to return previous
        if (aNewPos == aFormMap.begin())
            aNewPos = aFormMap.end(); // prepare to loop around
        else
            --aNewPos;
    }

    if (aNewPos == aFormMap.end())
    {
        // Loop around to the other side
        if (bNext)
            aNewPos = aFormMap.begin();
        else
            --aNewPos;
    }

    // the entry contains a pointer to either a Content Control (first) or Fieldmark (second)
    if (aNewPos->second.first)
    {
        auto& rFCC = static_cast<SwFormatContentControl&>(aNewPos->second.first->GetAttr());
        GotoFormatContentControl(rFCC);
    }
    else
    {
        assert(aNewPos->second.second);
        GotoFieldmark(aNewPos->second.second);
    }
}

bool SwCursorShell::GotoFormatField( const SwFormatField& rField )
{
    SwTextField const*const pTextField(rField.GetTextField());
    if (!pTextField
        || (GetLayout()->IsHideRedlines()
             && sw::IsFieldDeletedInModel(
                 GetDoc()->getIDocumentRedlineAccess(), *pTextField)))
        return false;

    CurrShell aCurr( this );
    SwCallLink aLk( *this ); // watch Cursor-Moves

    SwCursor* pCursor = getShellCursor( true );
    SwCursorSaveState aSaveState( *pCursor );

    SwTextNode* pTNd = pTextField->GetpTextNode();
    pCursor->GetPoint()->Assign(*pTNd, pTextField->GetStart() );

    bool bRet = !pCursor->IsSelOvr();
    if( bRet )
        UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
    if (&pCursor->GetPoint()->GetNode() != pTNd)
    {
        // tdf#161346 failed to move to field
        return false;
    }
    return bRet;
}

SwTextField * SwCursorShell::GetTextFieldAtPos(
    const SwPosition* pPos,
    ::sw::GetTextAttrMode const eMode)
{
    SwTextField* pTextField = nullptr;

    SwTextNode * const pNode = pPos->GetNode().GetTextNode();
    if ( pNode != nullptr )
    {
        pTextField = pNode->GetFieldTextAttrAt(pPos->GetContentIndex(), eMode);
    }

    return pTextField;
}

SwTextField* SwCursorShell::GetTextFieldAtCursor(
    const SwPaM* pCursor,
    ::sw::GetTextAttrMode const eMode)
{
    SwTextField* pTextField = GetTextFieldAtPos(pCursor->Start(), eMode);
    if ( !pTextField
        || pCursor->Start()->GetNode() != pCursor->End()->GetNode() )
        return nullptr;

    SwTextField* pFieldAtCursor = nullptr;
    const sal_Int32 nTextFieldLength =
        pTextField->End() != nullptr
        ? *(pTextField->End()) - pTextField->GetStart()
        : 1;
    if ( ( pCursor->End()->GetContentIndex() - pCursor->Start()->GetContentIndex() ) <= nTextFieldLength )
    {
        pFieldAtCursor = pTextField;
    }

    return pFieldAtCursor;
}

SwField* SwCursorShell::GetFieldAtCursor(
    const SwPaM *const pCursor,
    const bool bIncludeInputFieldAtStart)
{
    SwTextField *const pField(GetTextFieldAtCursor(pCursor,
        bIncludeInputFieldAtStart ? ::sw::GetTextAttrMode::Default : ::sw::GetTextAttrMode::Expand));
    return pField
        ? const_cast<SwField*>(pField->GetFormatField().GetField())
        : nullptr;
}

SwField* SwCursorShell::GetCurField( const bool bIncludeInputFieldAtStart ) const
{
    SwPaM* pCursor = GetCursor();
    if ( pCursor->IsMultiSelection() )
    {
        // multi selection not handled.
        return nullptr;
    }

    SwField* pCurField = GetFieldAtCursor( pCursor, bIncludeInputFieldAtStart );
    if ( pCurField != nullptr
         && SwFieldIds::Table == pCurField->GetTyp()->Which() )
    {
        // table formula? convert internal name into external
        const SwTableNode* pTableNd = IsCursorInTable();
        static_cast<SwTableField*>(pCurField)->PtrToBoxNm( pTableNd ? &pTableNd->GetTable() : nullptr );
    }

    return pCurField;
}

bool SwCursorShell::CursorInsideInputField() const
{
    for(SwPaM& rCursor : GetCursor()->GetRingContainer())
    {
        if (dynamic_cast<const SwTextInputField*>(GetTextFieldAtCursor(&rCursor, ::sw::GetTextAttrMode::Parent)))
            return true;
    }
    return false;
}

SwTextContentControl* SwCursorShell::CursorInsideContentControl() const
{
    for (SwPaM& rCursor : GetCursor()->GetRingContainer())
    {
        const SwPosition* pStart = rCursor.Start();
        SwTextNode* pTextNode = pStart->GetNode().GetTextNode();
        if (!pTextNode)
        {
            continue;
        }

        sal_Int32 nIndex = pStart->GetContentIndex();
        if (SwTextAttr* pAttr = pTextNode->GetTextAttrAt(nIndex, RES_TXTATR_CONTENTCONTROL, ::sw::GetTextAttrMode::Parent))
        {
            return static_txtattr_cast<SwTextContentControl*>(pAttr);
        }
    }

    return nullptr;
}

bool SwCursorShell::PosInsideInputField( const SwPosition& rPos )
{
    return dynamic_cast<const SwTextInputField*>(GetTextFieldAtPos(&rPos, ::sw::GetTextAttrMode::Parent)) != nullptr;
}

bool SwCursorShell::DocPtInsideInputField( const Point& rDocPt ) const
{
    SwPosition aPos( *(GetCursor()->Start()) );
    Point aDocPt( rDocPt );
    if ( GetLayout()->GetModelPositionForViewPoint( &aPos, aDocPt ) )
    {
        return PosInsideInputField( aPos );
    }
    return false;
}

sal_Int32 SwCursorShell::StartOfInputFieldAtPos( const SwPosition& rPos )
{
    const SwTextInputField* pTextInputField = dynamic_cast<const SwTextInputField*>(GetTextFieldAtPos(&rPos, ::sw::GetTextAttrMode::Default));
    assert(pTextInputField != nullptr
        && "<SwEditShell::StartOfInputFieldAtPos(..)> - no Input Field at given position");
    return pTextInputField->GetStart();
}

sal_Int32 SwCursorShell::EndOfInputFieldAtPos( const SwPosition& rPos )
{
    const SwTextInputField* pTextInputField = dynamic_cast<const SwTextInputField*>(GetTextFieldAtPos(&rPos, ::sw::GetTextAttrMode::Default));
    assert(pTextInputField != nullptr
        && "<SwEditShell::EndOfInputFieldAtPos(..)> - no Input Field at given position");
    return *(pTextInputField->End());
}

void SwCursorShell::GotoOutline( SwOutlineNodes::size_type nIdx )
{
    SwCursor* pCursor = getShellCursor( true );

    CurrShell aCurr( this );
    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursorSaveState aSaveState( *pCursor );

    const SwNodes& rNds = GetDoc()->GetNodes();
    SwTextNode* pTextNd = rNds.GetOutLineNds()[ nIdx ]->GetTextNode();
    pCursor->GetPoint()->Assign(*pTextNd);

    if( !pCursor->IsSelOvr() )
        UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
}

bool SwCursorShell::GotoOutline( const OUString& rName )
{
    SwCursor* pCursor = getShellCursor( true );

    CurrShell aCurr( this );
    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursorSaveState aSaveState( *pCursor );

    bool bRet = false;
    if (mxDoc->GotoOutline(*pCursor->GetPoint(), rName, GetLayout())
        && !pCursor->IsSelOvr())
    {
        UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
        bRet = true;
    }
    return bRet;
}

/// jump to next node with outline num.
bool SwCursorShell::GotoNextOutline()
{
    const SwNodes& rNds = GetDoc()->GetNodes();

    if ( rNds.GetOutLineNds().empty() )
    {
        SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::NavElementNotFound );
        return false;
    }

    SwCursor* pCursor = getShellCursor( true );
    SwNode* pNd = &(pCursor->GetPointNode());
    SwOutlineNodes::size_type nPos;
    bool bUseFirst = !rNds.GetOutLineNds().Seek_Entry( pNd, &nPos );
    SwOutlineNodes::size_type const nStartPos(nPos);

    do
    {
        if (!bUseFirst)
        {
            ++nPos;
        }
        if (rNds.GetOutLineNds().size() <= nPos)
        {
            nPos = 0;
        }

        if (bUseFirst)
        {
            bUseFirst = false;
        }
        else
        {
            if (nPos == nStartPos)
            {
                SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::NavElementNotFound );
                return false;
            }
        }

        pNd = rNds.GetOutLineNds()[ nPos ];
    }
    while (!sw::IsParaPropsNode(*GetLayout(), *pNd->GetTextNode()));

    if (nPos < nStartPos)
    {
        SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::EndWrapped );
    }
    else
    {
        SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::Empty );
    }

    CurrShell aCurr( this );
    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursorSaveState aSaveState( *pCursor );
    pCursor->GetPoint()->Assign(*pNd);

    bool bRet = !pCursor->IsSelOvr();
    if( bRet )
        UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
    return bRet;
}

/// jump to previous node with outline num.
bool SwCursorShell::GotoPrevOutline()
{
    const SwNodes& rNds = GetDoc()->GetNodes();

    if ( rNds.GetOutLineNds().empty() )
    {
        SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::NavElementNotFound );
        return false;
    }

    SwCursor* pCursor = getShellCursor( true );
    SwNode* pNd = &(pCursor->GetPointNode());
    SwOutlineNodes::size_type nPos;
    (void)rNds.GetOutLineNds().Seek_Entry(pNd, &nPos);
    SwOutlineNodes::size_type const nStartPos(nPos);

    do
    {
        if (nPos == 0)
        {
            nPos = rNds.GetOutLineNds().size() - 1;
        }
        else
        {
            --nPos; // before
        }
        if (nPos == nStartPos)
        {
            pNd = nullptr;
            break;
        }

        pNd = rNds.GetOutLineNds()[ nPos ];
    }
    while (!sw::IsParaPropsNode(*GetLayout(), *pNd->GetTextNode()));

    if (!pNd)
    {
        SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::NavElementNotFound );
        return false;
    }

    if (nStartPos < nPos)
    {
        SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::StartWrapped );
    }
    else
    {
        SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::Empty );
    }
    CurrShell aCurr( this );
    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursorSaveState aSaveState( *pCursor );
    pCursor->GetPoint()->Assign(*pNd);

    bool bRet = !pCursor->IsSelOvr();
    if( bRet )
        UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
    return bRet;
}

/// search "outline position" before previous outline node at given level
SwOutlineNodes::size_type SwCursorShell::GetOutlinePos(sal_uInt8 nLevel, SwPaM* pPaM)
{
    SwPaM* pCursor = pPaM ? pPaM : getShellCursor(true);
    const SwNodes& rNds = GetDoc()->GetNodes();

    SwNode* pNd = &(pCursor->GetPointNode());
    SwOutlineNodes::size_type nPos;
    if( rNds.GetOutLineNds().Seek_Entry( pNd, &nPos ))
        nPos++; // is at correct position; take next for while

    while( nPos-- ) // check the one in front of the current
    {
        pNd = rNds.GetOutLineNds()[ nPos ];

        if (sw::IsParaPropsNode(*GetLayout(), *pNd->GetTextNode())
            && pNd->GetTextNode()->GetAttrOutlineLevel()-1 <= nLevel)
        {
            if (pNd->GetIndex() < rNds.GetEndOfExtras().GetIndex()
                    && pCursor->GetPointNode().GetIndex() > rNds.GetEndOfExtras().GetIndex())
            {
                // node found in extras but cursor position is not in extras
                return SwOutlineNodes::npos;
            }
            return nPos;
        }
    }
    return SwOutlineNodes::npos; // no more left
}

void SwCursorShell::MakeOutlineSel(SwOutlineNodes::size_type nSttPos, SwOutlineNodes::size_type nEndPos,
                                  bool bWithChildren , bool bKillPams)
{
    const SwNodes& rNds = GetDoc()->GetNodes();
    const SwOutlineNodes& rOutlNds = rNds.GetOutLineNds();
    if( rOutlNds.empty() )
        return;

    CurrShell aCurr( this );
    SwCallLink aLk( *this ); // watch Cursor-Moves

    if( nSttPos > nEndPos ) // parameters switched?
    {
        OSL_ENSURE( false, "Start > End for array access" );
        std::swap(nSttPos, nEndPos);
    }

    SwNode* pSttNd = rOutlNds[ nSttPos ];
    SwNode* pEndNd = rOutlNds[ nEndPos ];

    if( bWithChildren )
    {
        const int nLevel = pEndNd->GetTextNode()->GetAttrOutlineLevel()-1;
        for( ++nEndPos; nEndPos < rOutlNds.size(); ++nEndPos )
        {
            pEndNd = rOutlNds[ nEndPos ];
            const int nNxtLevel = pEndNd->GetTextNode()->GetAttrOutlineLevel()-1;
            if( nNxtLevel <= nLevel )
                break; // EndPos is now on the next one
        }
    }
    // if without children then set onto next one
    else if( ++nEndPos < rOutlNds.size() )
        pEndNd = rOutlNds[ nEndPos ];

    if( nEndPos == rOutlNds.size() ) // no end found
        pEndNd = &rNds.GetEndOfContent();

    if( bKillPams )
        KillPams();

    SwCursorSaveState aSaveState( *m_pCurrentCursor );

    // set end to the end of the previous content node
    m_pCurrentCursor->GetPoint()->Assign(*pSttNd);
    m_pCurrentCursor->SetMark();
    m_pCurrentCursor->GetPoint()->Assign(*pEndNd);
    m_pCurrentCursor->Move( fnMoveBackward, GoInNode ); // end of predecessor

    // and everything is already selected
    bool bRet = !m_pCurrentCursor->IsSelOvr();
    if( bRet )
        UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
}

/// jump to reference marker
bool SwCursorShell::GotoRefMark( const OUString& rRefMark, sal_uInt16 nSubType,
                                    sal_uInt16 nSeqNo, sal_uInt16 nFlags )
{
    CurrShell aCurr( this );
    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursorSaveState aSaveState( *m_pCurrentCursor );

    sal_Int32 nPos = -1;

    SwPaM* pCursor = GetCursor();
    SwPosition* pPos = pCursor->GetPoint();
    SwTextNode* pRefTextNd = pPos->GetNode().GetTextNode();
    SwContentFrame* pRefFrame = GetCurrFrame();

    SwTextNode* pTextNd = SwGetRefFieldType::FindAnchor(GetDoc(), rRefMark,
                                nSubType, nSeqNo, nFlags, &nPos, nullptr, GetLayout(), pRefTextNd, pRefFrame);
    if( !pTextNd || !pTextNd->GetNodes().IsDocNodes() )
        return false;

    m_pCurrentCursor->GetPoint()->Assign(*pTextNd, nPos );

    if( m_pCurrentCursor->IsSelOvr() )
        return false;

    UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
    return true;
}

bool SwCursorShell::IsPageAtPos( const Point &rPt ) const
{
    if( GetLayout() )
        return nullptr != GetLayout()->GetPageAtPos( rPt );
    return false;
}

bool SwCursorShell::GetContentAtPos( const Point& rPt,
                                   SwContentAtPos& rContentAtPos,
                                   bool bSetCursor,
                                   SwRect* pFieldRect )
{
    CurrShell aCurr( this );
    bool bRet = false;

    if( IsTableMode() )
    {
        rContentAtPos.eContentAtPos = IsAttrAtPos::NONE;
        rContentAtPos.aFnd.pField = nullptr;
        return false;
    }

    Point aPt( rPt );
    SwPosition aPos( *m_pCurrentCursor->GetPoint() );

    SwTextNode* pTextNd;
    SwCursorMoveState aTmpState;
    aTmpState.m_bFieldInfo = true;
    aTmpState.m_bExactOnly = !( IsAttrAtPos::Outline & rContentAtPos.eContentAtPos );
    aTmpState.m_bContentCheck = bool(IsAttrAtPos::ContentCheck & rContentAtPos.eContentAtPos);
    aTmpState.m_bSetInReadOnly = IsReadOnlyAvailable();

    SwSpecialPos aSpecialPos;
    aTmpState.m_pSpecialPos = ( IsAttrAtPos::SmartTag & rContentAtPos.eContentAtPos ) ?
                            &aSpecialPos : nullptr;

    const bool bCursorFoundExact = GetLayout()->GetModelPositionForViewPoint( &aPos, aPt, &aTmpState );
    pTextNd = aPos.GetNode().GetTextNode();

    const SwNodes& rNds = GetDoc()->GetNodes();
    if( pTextNd
        && IsAttrAtPos::Outline & rContentAtPos.eContentAtPos
        && !rNds.GetOutLineNds().empty() )
    {
        // only for nodes in outline nodes
        SwOutlineNodes::size_type nPos = 0;
        bool bFoundOutline = rNds.GetOutLineNds().Seek_Entry(pTextNd, &nPos);
        if (!bFoundOutline && nPos && (IsAttrAtPos::AllowContaining & rContentAtPos.eContentAtPos))
        {
            // nPos points to the first found outline node not before pTextNd, or to end();
            // when bFoundOutline is false, and nPos is not 0, it means that there were
            // outline nodes before pTextNd, and nPos-1 points to the last of those.
            pTextNd = rNds.GetOutLineNds()[nPos - 1]->GetTextNode();
            bFoundOutline = true;
        }
        if (bFoundOutline)
        {
            rContentAtPos.eContentAtPos = IsAttrAtPos::Outline;
            rContentAtPos.sStr = sw::GetExpandTextMerged(GetLayout(), *pTextNd, true, false, ExpandMode::ExpandFootnote);
            rContentAtPos.aFnd.pNode = pTextNd;
            bRet = true;
        }
    }
    else if ( IsAttrAtPos::ContentCheck & rContentAtPos.eContentAtPos
              && bCursorFoundExact )
    {
        bRet = true;
    }
    else if( pTextNd
             && IsAttrAtPos::NumLabel & rContentAtPos.eContentAtPos)
    {
        bRet = aTmpState.m_bInNumPortion;
        rContentAtPos.aFnd.pNode = sw::GetParaPropsNode(*GetLayout(), aPos.GetNode());

        Size aSizeLogic(aTmpState.m_nInNumPortionOffset, 0);
        Size aSizePixel = GetWin()->LogicToPixel(aSizeLogic);
        rContentAtPos.nDist = aSizePixel.Width();
    }
    else if( bCursorFoundExact && pTextNd )
    {
        SwContentFrame *pFrame(nullptr);
        if( !aTmpState.m_bPosCorr )
        {
            SwTextAttr* pTextAttr;
            if ( IsAttrAtPos::SmartTag & rContentAtPos.eContentAtPos
                 && !aTmpState.m_bFootnoteNoInfo )
            {
                const SwWrongList* pSmartTagList = pTextNd->GetSmartTags();
                sal_Int32 nCurrent = aPos.GetContentIndex();
                const sal_Int32 nBegin = nCurrent;
                sal_Int32 nLen = 1;

                if (pSmartTagList && pSmartTagList->InWrongWord(nCurrent, nLen) && !pTextNd->IsSymbolAt(nBegin))
                {
                    const sal_uInt16 nIndex = pSmartTagList->GetWrongPos( nBegin );
                    const SwWrongList* pSubList = pSmartTagList->SubList( nIndex );
                    if ( pSubList )
                    {
                        nCurrent = aTmpState.m_pSpecialPos->nCharOfst;

                        if ( pSubList->InWrongWord( nCurrent, nLen ) )
                            bRet = true;
                    }
                    else
                        bRet = true;

                    if( bRet && bSetCursor )
                    {
                        SwCursorSaveState aSaveState( *m_pCurrentCursor );
                        SwCallLink aLk( *this ); // watch Cursor-Moves
                        m_pCurrentCursor->DeleteMark();
                        *m_pCurrentCursor->GetPoint() = aPos;
                        if( m_pCurrentCursor->IsSelOvr( SwCursorSelOverFlags::CheckNodeSection | SwCursorSelOverFlags::Toggle) )
                            bRet = false;
                        else
                            UpdateCursor();
                    }
                    if( bRet )
                    {
                        rContentAtPos.eContentAtPos = IsAttrAtPos::SmartTag;

                        std::pair<Point, bool> tmp(aPt, true);
                        if (pFieldRect)
                        {
                            pFrame = pTextNd->getLayoutFrame(GetLayout(), nullptr, &tmp);
                            if (pFrame)
                                pFrame->GetCharRect( *pFieldRect, aPos, &aTmpState );
                        }
                    }
                }
            }

            if ( !bRet
                 && ( IsAttrAtPos::Field | IsAttrAtPos::ClickField ) & rContentAtPos.eContentAtPos
                 && !aTmpState.m_bFootnoteNoInfo )
            {
                pTextAttr = pTextNd->GetFieldTextAttrAt( aPos.GetContentIndex() );
                const SwField* pField = pTextAttr != nullptr
                                      ? pTextAttr->GetFormatField().GetField()
                                      : nullptr;
                if ( IsAttrAtPos::ClickField & rContentAtPos.eContentAtPos
                     && pField && !pField->HasClickHdl() )
                {
                    pField = nullptr;
                }

                if ( pField )
                {
                    if (pFieldRect)
                    {
                        std::pair<Point, bool> tmp(aPt, true);
                        pFrame = pTextNd->getLayoutFrame(GetLayout(), nullptr, &tmp);
                        if (pFrame)
                        {
                            //tdf#116397 now that we looking for the bounds of the field drop the SmartTag
                            //index within field setting so we don't the bounds of the char within the field
                            SwSpecialPos* pSpecialPos = aTmpState.m_pSpecialPos;
                            aTmpState.m_pSpecialPos = nullptr;
                            pFrame->GetCharRect( *pFieldRect, aPos, &aTmpState );
                            aTmpState.m_pSpecialPos = pSpecialPos;
                        }
                    }

                    if( bSetCursor )
                    {
                        SwCallLink aLk( *this ); // watch Cursor-Moves
                        SwCursorSaveState aSaveState( *m_pCurrentCursor );
                        m_pCurrentCursor->DeleteMark();
                        *m_pCurrentCursor->GetPoint() = aPos;
                        if( m_pCurrentCursor->IsSelOvr() )
                        {
                            // allow click fields in protected sections
                            // only placeholder is not possible
                            if( IsAttrAtPos::Field & rContentAtPos.eContentAtPos
                                || SwFieldIds::JumpEdit == pField->Which() )
                                pField = nullptr;
                        }
                        else
                            UpdateCursor();
                    }
                    else if( SwFieldIds::Table == pField->Which() &&
                        static_cast<const SwTableField*>(pField)->IsIntrnlName() )
                    {
                        // create from internal (for CORE) the external
                        // (for UI) formula
                        const SwTableNode* pTableNd = pTextNd->FindTableNode();
                        if( pTableNd )        // is in a table
                            const_cast<SwTableField*>(static_cast<const SwTableField*>(pField))->PtrToBoxNm( &pTableNd->GetTable() );
                    }
                }

                if( pField )
                {
                    rContentAtPos.aFnd.pField = pField;
                    rContentAtPos.pFndTextAttr = pTextAttr;
                    rContentAtPos.eContentAtPos = IsAttrAtPos::Field;
                    bRet = true;
                }
            }

            if( !bRet && IsAttrAtPos::FormControl & rContentAtPos.eContentAtPos )
            {
                IDocumentMarkAccess* pMarksAccess = GetDoc()->getIDocumentMarkAccess( );
                sw::mark::IFieldmark* pFieldBookmark = pMarksAccess->getInnerFieldmarkFor(aPos);
                if (bCursorFoundExact && pFieldBookmark)
                {
                    rContentAtPos.eContentAtPos = IsAttrAtPos::FormControl;
                    rContentAtPos.aFnd.pFieldmark = pFieldBookmark;
                    bRet=true;
                }
            }

            if (!bRet && rContentAtPos.eContentAtPos & IsAttrAtPos::ContentControl)
            {
                SwTextAttr* pAttr = pTextNd->GetTextAttrAt(
                    aPos.GetContentIndex(), RES_TXTATR_CONTENTCONTROL, ::sw::GetTextAttrMode::Parent);
                if (pAttr)
                {
                    rContentAtPos.eContentAtPos = IsAttrAtPos::ContentControl;
                    rContentAtPos.pFndTextAttr = pAttr;
                    bRet = true;
                }
            }

            if( !bRet && IsAttrAtPos::Ftn & rContentAtPos.eContentAtPos )
            {
                if( aTmpState.m_bFootnoteNoInfo )
                {
                    // over the footnote's char
                    bRet = true;
                    if( bSetCursor )
                    {
                        *m_pCurrentCursor->GetPoint() = aPos;
                        if( !GotoFootnoteAnchor() )
                            bRet = false;
                    }
                    if( bRet )
                        rContentAtPos.eContentAtPos = IsAttrAtPos::Ftn;
                }
                else if ( nullptr != ( pTextAttr = pTextNd->GetTextAttrForCharAt(
                    aPos.GetContentIndex(), RES_TXTATR_FTN )) )
                {
                    bRet = true;
                    if( bSetCursor )
                    {
                        if (SwWrtShell* pWrtSh = dynamic_cast<SwWrtShell*>(this))
                            pWrtSh->addCurrentPosition();

                        SwCallLink aLk( *this ); // watch Cursor-Moves
                        SwCursorSaveState aSaveState( *m_pCurrentCursor );
                        m_pCurrentCursor->GetPoint()->Assign( *static_cast<SwTextFootnote*>(pTextAttr)->GetStartNode() );
                        SwContentNode* pCNd = GetDoc()->GetNodes().GoNextSection(
                            m_pCurrentCursor->GetPoint(),
                            true, !IsReadOnlyAvailable() );

                        if( pCNd )
                        {
                            if( m_pCurrentCursor->IsSelOvr( SwCursorSelOverFlags::CheckNodeSection |
                                SwCursorSelOverFlags::Toggle ))
                                bRet = false;
                            else
                                UpdateCursor();
                        }
                        else
                            bRet = false;
                    }

                    if( bRet )
                    {
                        rContentAtPos.eContentAtPos = IsAttrAtPos::Ftn;
                        rContentAtPos.pFndTextAttr = pTextAttr;
                        rContentAtPos.aFnd.pAttr = &pTextAttr->GetAttr();

                        if (pFieldRect)
                        {
                            std::pair<Point, bool> tmp(aPt, true);
                            pFrame = pTextNd->getLayoutFrame(GetLayout(), nullptr, &tmp);
                            if (pFrame)
                                pFrame->GetCharRect( *pFieldRect, aPos, &aTmpState );
                        }
                    }
                }
            }

            if( !bRet
                && ( IsAttrAtPos::ToxMark | IsAttrAtPos::RefMark ) & rContentAtPos.eContentAtPos
                && !aTmpState.m_bFootnoteNoInfo )
            {
                pTextAttr = nullptr;
                if( IsAttrAtPos::ToxMark & rContentAtPos.eContentAtPos )
                {
                    std::vector<SwTextAttr *> const marks(
                        pTextNd->GetTextAttrsAt(
                           aPos.GetContentIndex(), RES_TXTATR_TOXMARK));
                    if (!marks.empty())
                    {   // hmm... can only return 1 here
                        pTextAttr = *marks.begin();
                    }
                }

                if( !pTextAttr &&
                    IsAttrAtPos::RefMark & rContentAtPos.eContentAtPos )
                {
                    std::vector<SwTextAttr *> const marks(
                        pTextNd->GetTextAttrsAt(
                           aPos.GetContentIndex(), RES_TXTATR_REFMARK));
                    if (!marks.empty())
                    {   // hmm... can only return 1 here
                        pTextAttr = *marks.begin();
                    }
                }

                if( pTextAttr )
                {
                    bRet = true;
                    if( bSetCursor )
                    {
                        SwCallLink aLk( *this ); // watch Cursor-Moves
                        SwCursorSaveState aSaveState( *m_pCurrentCursor );
                        m_pCurrentCursor->DeleteMark();
                        *m_pCurrentCursor->GetPoint() = aPos;
                        if( m_pCurrentCursor->IsSelOvr( SwCursorSelOverFlags::CheckNodeSection | SwCursorSelOverFlags::Toggle ) )
                            bRet = false;
                        else
                            UpdateCursor();
                    }

                    if( bRet )
                    {
                        const sal_Int32* pEnd = pTextAttr->GetEnd();
                        if( pEnd )
                            rContentAtPos.sStr =
                                pTextNd->GetExpandText(GetLayout(), pTextAttr->GetStart(), *pEnd - pTextAttr->GetStart());
                        else if( RES_TXTATR_TOXMARK == pTextAttr->Which())
                            rContentAtPos.sStr =
                                pTextAttr->GetTOXMark().GetAlternativeText();

                        rContentAtPos.eContentAtPos =
                            RES_TXTATR_TOXMARK == pTextAttr->Which()
                            ? IsAttrAtPos::ToxMark
                            : IsAttrAtPos::RefMark;
                        rContentAtPos.pFndTextAttr = pTextAttr;
                        rContentAtPos.aFnd.pAttr = &pTextAttr->GetAttr();

                        std::pair<Point, bool> tmp(aPt, true);
                        if (pFieldRect)
                        {
                            pFrame = pTextNd->getLayoutFrame(GetLayout(), nullptr, &tmp);
                            if (pFrame)
                                pFrame->GetCharRect( *pFieldRect, aPos, &aTmpState );
                        }
                    }
                }
            }

            if ( !bRet
                 && IsAttrAtPos::InetAttr & rContentAtPos.eContentAtPos
                 && !aTmpState.m_bFootnoteNoInfo )
            {
                sal_Int32 index = aPos.GetContentIndex();
                pTextAttr = pTextNd->GetTextAttrAt(index, RES_TXTATR_INETFMT);

                // "detect" only INetAttrs with URLs
                if( pTextAttr && !pTextAttr->GetINetFormat().GetValue().isEmpty() )
                {
                    bRet = true;
                    if( bSetCursor )
                    {
                        SwCursorSaveState aSaveState( *m_pCurrentCursor );
                        SwCallLink aLk( *this ); // watch Cursor-Moves
                        m_pCurrentCursor->DeleteMark();
                        *m_pCurrentCursor->GetPoint() = aPos;
                        if( m_pCurrentCursor->IsSelOvr( SwCursorSelOverFlags::CheckNodeSection |
                            SwCursorSelOverFlags::Toggle) )
                            bRet = false;
                        else
                            UpdateCursor();
                    }
                    if( bRet )
                    {
                        const sal_Int32 nSt = pTextAttr->GetStart();
                        const sal_Int32 nEnd = *pTextAttr->End();

                        rContentAtPos.sStr = pTextNd->GetExpandText(GetLayout(), nSt, nEnd-nSt);

                        rContentAtPos.aFnd.pAttr = &pTextAttr->GetAttr();
                        rContentAtPos.eContentAtPos = IsAttrAtPos::InetAttr;
                        rContentAtPos.pFndTextAttr = pTextAttr;

                        if (pFieldRect)
                        {
                            std::pair<Point, bool> tmp(aPt, true);
                            pFrame = pTextNd->getLayoutFrame(GetLayout(), nullptr, &tmp);
                            if (pFrame)
                            {
                                //get bounding box of range
                                SwRect aStart;
                                SwPosition aStartPos(*pTextNd, nSt);
                                pFrame->GetCharRect(aStart, aStartPos, &aTmpState);
                                SwRect aEnd;
                                SwPosition aEndPos(*pTextNd, nEnd);
                                pFrame->GetCharRect(aEnd, aEndPos, &aTmpState);
                                if (aStart.Top() != aEnd.Top() || aStart.Bottom() != aEnd.Bottom())
                                {
                                    aStart.Left(pFrame->getFrameArea().Left());
                                    aEnd.Right(pFrame->getFrameArea().Right());
                                }
                                *pFieldRect = aStart.Union(aEnd);
                            }
                        }
                    }
                }
            }

            if( !bRet && IsAttrAtPos::Redline & rContentAtPos.eContentAtPos )
            {
                const SwRangeRedline* pRedl = GetDoc()->getIDocumentRedlineAccess().GetRedline(aPos, nullptr);

                if( pRedl )
                {
                    rContentAtPos.aFnd.pRedl = pRedl;
                    rContentAtPos.eContentAtPos = IsAttrAtPos::Redline;
                    rContentAtPos.pFndTextAttr = nullptr;
                    bRet = true;

                    if (pFieldRect)
                    {
                        std::pair<Point, bool> tmp(aPt, true);
                        pFrame = pTextNd->getLayoutFrame(GetLayout(), nullptr, &tmp);
                        if( pFrame )
                        {
                            // not sure if this should be limited to one
                            // paragraph, or mark the entire redline; let's
                            // leave it limited to one for now...
                            sal_Int32 nStart;
                            sal_Int32 nEnd;
                            pRedl->CalcStartEnd(pTextNd->GetIndex(), nStart, nEnd);
                            if (nStart == COMPLETE_STRING)
                            {
                                // consistency: found pRedl, so there must be
                                // something in pTextNd
                                assert(nEnd != COMPLETE_STRING);
                                nStart = 0;
                            }
                            if (nEnd == COMPLETE_STRING)
                            {
                                nEnd = pTextNd->Len();
                            }
                            //get bounding box of range
                            SwRect aStart;
                            pFrame->GetCharRect(aStart, SwPosition(*pTextNd, nStart), &aTmpState);
                            SwRect aEnd;
                            pFrame->GetCharRect(aEnd, SwPosition(*pTextNd, nEnd), &aTmpState);
                            if (aStart.Top() != aEnd.Top() || aStart.Bottom() != aEnd.Bottom())
                            {
                                aStart.Left(pFrame->getFrameArea().Left());
                                aEnd.Right(pFrame->getFrameArea().Right());
                            }
                            *pFieldRect = aStart.Union(aEnd);
                        }
                    }
                }
            }
        }

        if( !bRet && ( ( IsAttrAtPos::TableRedline & rContentAtPos.eContentAtPos ) ||
                     ( IsAttrAtPos::TableColRedline & rContentAtPos.eContentAtPos ) ) )
        {
            const SwTableNode* pTableNd;
            const SwTableBox* pBox;
            const SwTableLine* pTableLine;
            const SwStartNode* pSttNd = pTextNd->FindTableBoxStartNode();
            if( pSttNd && nullptr != ( pTableNd = pTextNd->FindTableNode()) &&
                nullptr != ( pBox = pTableNd->GetTable().GetTableBox(
                pSttNd->GetIndex() )) &&
                nullptr != ( pTableLine = pBox->GetUpper() ) &&
                ( RedlineType::None != pBox->GetRedlineType() ||
                RedlineType::None != pTableLine->GetRedlineType() ) )
            {
                const SwRedlineTable& aRedlineTable = GetDoc()->getIDocumentRedlineAccess().GetRedlineTable();
                if ( RedlineType::None != pTableLine->GetRedlineType() )
                {
                    SwRedlineTable::size_type nPos = 0;
                    nPos = pTableLine->UpdateTextChangesOnly(nPos);
                    if ( nPos != SwRedlineTable::npos )
                    {
                        rContentAtPos.aFnd.pRedl = aRedlineTable[nPos];
                        rContentAtPos.eContentAtPos = IsAttrAtPos::TableRedline;
                        bRet = true;
                    }
                }
                else
                {
                    SwRedlineTable::size_type n = 0;
                    SwNodeIndex aIdx( *pSttNd, 1 );
                    const SwPosition aBoxStart(aIdx);
                    const SwRangeRedline* pFnd = aRedlineTable.FindAtPosition( aBoxStart, n, /*next=*/true );
                    if( pFnd && RedlineType::Delete == pFnd->GetType() )
                    {
                        rContentAtPos.aFnd.pRedl = aRedlineTable[n];
                        rContentAtPos.eContentAtPos = IsAttrAtPos::TableColRedline;
                        bRet = true;
                    }
                }
            }
        }

        if( !bRet
             && ( IsAttrAtPos::TableBoxFml & rContentAtPos.eContentAtPos
#ifdef DBG_UTIL
                  || IsAttrAtPos::TableBoxValue & rContentAtPos.eContentAtPos
#endif
            ) )
        {
            const SwTableNode* pTableNd;
            const SwTableBox* pBox;
            const SwStartNode* pSttNd = pTextNd->FindTableBoxStartNode();
            const SwTableBoxFormula* pItem;
#ifdef DBG_UTIL
            const SwTableBoxValue* pItem2 = nullptr;
#endif
            if( pSttNd && nullptr != ( pTableNd = pTextNd->FindTableNode()) &&
                nullptr != ( pBox = pTableNd->GetTable().GetTableBox(
                pSttNd->GetIndex() )) &&
#ifdef DBG_UTIL
                ( (pItem = pBox->GetFrameFormat()->GetItemIfSet( RES_BOXATR_FORMULA, false )) ||
                  (pItem2 = pBox->GetFrameFormat()->GetItemIfSet( RES_BOXATR_VALUE, false )) )
#else
                (pItem = pBox->GetFrameFormat()->GetItemIfSet( RES_BOXATR_FORMULA, false ))
#endif
                )
            {
                std::pair<Point, bool> tmp(aPt, true);
                SwFrame* pF = pTextNd->getLayoutFrame(GetLayout(), nullptr, &tmp);
                if( pF )
                {
                    // then the CellFrame
                    pFrame = static_cast<SwContentFrame*>(pF);
                    while( pF && !pF->IsCellFrame() )
                        pF = pF->GetUpper();
                }

                if( aTmpState.m_bPosCorr )
                {
                    if( pF && !pF->getFrameArea().Contains( aPt ))
                        pF = nullptr;
                }
                else if( !pF )
                    pF = pFrame;

                if( pF ) // only then it is valid
                {
                    // create from internal (for CORE) the external
                    // (for UI) formula
                    rContentAtPos.eContentAtPos = IsAttrAtPos::TableBoxFml;
#ifdef DBG_UTIL
                    if( pItem2 )
                        rContentAtPos.eContentAtPos = IsAttrAtPos::TableBoxValue;
                    else
#endif
                        const_cast<SwTableBoxFormula&>(*pItem).PtrToBoxNm( &pTableNd->GetTable() );

                    bRet = true;
                    if( bSetCursor )
                    {
                        SwCallLink aLk( *this ); // watch Cursor-Moves
                        SwCursorSaveState aSaveState( *m_pCurrentCursor );
                        *m_pCurrentCursor->GetPoint() = aPos;
                        if( m_pCurrentCursor->IsSelOvr( SwCursorSelOverFlags::CheckNodeSection |
                            SwCursorSelOverFlags::Toggle) )
                            bRet = false;
                        else
                            UpdateCursor();
                    }

                    if( bRet )
                    {
                        if( pFieldRect )
                        {
                            *pFieldRect = pF->getFramePrintArea();
                            *pFieldRect += pF->getFrameArea().Pos();
                        }
                        rContentAtPos.pFndTextAttr = nullptr;
                        rContentAtPos.aFnd.pAttr = pItem;
                    }
                }
            }
        }

#ifdef DBG_UTIL
        if( !bRet && IsAttrAtPos::CurrAttrs & rContentAtPos.eContentAtPos )
        {
            const sal_Int32 n = aPos.GetContentIndex();
            SfxItemSetFixed<POOLATTR_BEGIN, POOLATTR_END - 1>  aSet( GetDoc()->GetAttrPool() );
            if( pTextNd->GetpSwpHints() )
            {
                for( size_t i = 0; i < pTextNd->GetSwpHints().Count(); ++i )
                {
                    const SwTextAttr* pHt = pTextNd->GetSwpHints().Get(i);
                    const sal_Int32 nAttrStart = pHt->GetStart();
                    if( nAttrStart > n ) // over the section
                        break;

                    if( nullptr != pHt->End() && (
                        ( nAttrStart < n &&
                        ( pHt->DontExpand() ? n < *pHt->End()
                        : n <= *pHt->End() )) ||
                        ( n == nAttrStart &&
                        ( nAttrStart == *pHt->End() || !n ))) )
                    {
                        aSet.Put( pHt->GetAttr() );
                    }
                }
                if( pTextNd->HasSwAttrSet() &&
                    pTextNd->GetpSwAttrSet()->Count() )
                {
                    SfxItemSet aFormatSet( pTextNd->GetSwAttrSet() );
                    // remove all from format set that are also in TextSet
                    aFormatSet.Differentiate( aSet );
                    // now merge all together
                    aSet.Put( aFormatSet );
                }
            }
            else
                pTextNd->SwContentNode::GetAttr( aSet );

            rContentAtPos.sStr = "Pos: (";
            rContentAtPos.sStr += OUString::number( sal_Int32(aPos.GetNodeIndex()));
            rContentAtPos.sStr += ":";
            rContentAtPos.sStr += OUString::number( aPos.GetContentIndex());
            rContentAtPos.sStr += ")";
            rContentAtPos.sStr += "\nParagraph Style: ";
            rContentAtPos.sStr += pTextNd->GetFormatColl()->GetName();
            if( pTextNd->GetCondFormatColl() )
            {
                rContentAtPos.sStr += "\nConditional Style: " + pTextNd->GetCondFormatColl()->GetName();
            }

            if( aSet.Count() )
            {
                OUStringBuffer sAttrs;
                SfxItemIter aIter( aSet );
                const SfxPoolItem* pItem = aIter.GetCurItem();
                const IntlWrapper aInt(SvtSysLocale().GetUILanguageTag());
                do
                {
                    if( !IsInvalidItem( pItem ))
                    {
                        OUString aStr;
                        GetDoc()->GetAttrPool().GetPresentation(*pItem,
                            MapUnit::MapCM, aStr, aInt);
                        if (!sAttrs.isEmpty())
                            sAttrs.append(", ");
                        sAttrs.append(aStr);
                    }
                    pItem = aIter.NextItem();
                } while (pItem);
                if (!sAttrs.isEmpty())
                {
                    if( !rContentAtPos.sStr.isEmpty() )
                        rContentAtPos.sStr += "\n";
                    rContentAtPos.sStr += "Attr: " + sAttrs;
                }
            }
            bRet = true;
            rContentAtPos.eContentAtPos = IsAttrAtPos::CurrAttrs;
        }
#endif
    }

    if( !bRet )
    {
        rContentAtPos.eContentAtPos = IsAttrAtPos::NONE;
        rContentAtPos.aFnd.pField = nullptr;
    }
    return bRet;
}

// #i90516#
const SwPostItField* SwCursorShell::GetPostItFieldAtCursor() const
{
    if ( IsTableMode() )
        return nullptr;

    const SwPosition* pCursorPos = GetCursor_()->GetPoint();
    const SwTextNode* pTextNd = pCursorPos->GetNode().GetTextNode();
    if ( !pTextNd )
        return nullptr;

    const SwPostItField* pPostItField = nullptr;
    SwTextAttr* pTextAttr = pTextNd->GetFieldTextAttrAt( pCursorPos->GetContentIndex() );
    const SwField* pField = pTextAttr != nullptr ? pTextAttr->GetFormatField().GetField() : nullptr;
    if ( pField && pField->Which()== SwFieldIds::Postit )
    {
        pPostItField = static_cast<const SwPostItField*>(pField);
    }

    return pPostItField;
}

/// is the node in a protected section?
bool SwContentAtPos::IsInProtectSect() const
{
    const SwTextNode* pNd = nullptr;
    if( pFndTextAttr )
    {
        switch( eContentAtPos )
        {
        case IsAttrAtPos::Field:
        case IsAttrAtPos::ClickField:
            pNd = static_txtattr_cast<SwTextField const*>(pFndTextAttr)->GetpTextNode();
            break;

        case IsAttrAtPos::Ftn:
            pNd = &static_cast<const SwTextFootnote*>(pFndTextAttr)->GetTextNode();
            break;

        case IsAttrAtPos::InetAttr:
            pNd = static_txtattr_cast<SwTextINetFormat const*>(pFndTextAttr)->GetpTextNode();
            break;

        default:
            break;
        }
    }

    if( !pNd )
        return false;
    if( pNd->IsInProtectSect() )
        return true;

    const SwContentFrame* pFrame = pNd->getLayoutFrame(pNd->GetDoc().getIDocumentLayoutAccess().GetCurrentLayout(), nullptr, nullptr);
    return pFrame && pFrame->IsProtected() ;
}

bool SwContentAtPos::IsInRTLText()const
{
    const SwTextNode* pNd = nullptr;
    if (!pFndTextAttr || (eContentAtPos != IsAttrAtPos::Ftn))
        return false;

    const SwTextFootnote* pTextFootnote = static_cast<const SwTextFootnote*>(pFndTextAttr);
    if(!pTextFootnote->GetStartNode())
        return false;

    SwStartNode* pSttNd = pTextFootnote->GetStartNode()->GetNode().GetStartNode();
    SwPaM aTemp( *pSttNd );
    aTemp.Move(fnMoveForward, GoInNode);
    SwContentNode* pContentNode = aTemp.GetPointContentNode();
    if(pContentNode && pContentNode->IsTextNode())
        pNd = pContentNode->GetTextNode();
    if(!pNd)
        return false;

    bool bRet = false;
    SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> aIter(*pNd);
    SwTextFrame* pTmpFrame = aIter.First();
    while( pTmpFrame )
    {
        if ( !pTmpFrame->IsFollow())
        {
            bRet = pTmpFrame->IsRightToLeft();
            break;
        }
        pTmpFrame = aIter.Next();
    }
    return bRet;
}

bool SwCursorShell::SelectTextModel( const sal_Int32 nStart,
                                 const sal_Int32 nEnd )
{
    CurrShell aCurr( this );
    bool bRet = false;

    SwCallLink aLk( *this );
    SwCursorSaveState aSaveState( *m_pCurrentCursor );

    SwPosition& rPos = *m_pCurrentCursor->GetPoint();
    m_pCurrentCursor->DeleteMark();
    rPos.SetContent(nStart);
    m_pCurrentCursor->SetMark();
    rPos.SetContent(nEnd);

    if( !m_pCurrentCursor->IsSelOvr() )
    {
        UpdateCursor();
        bRet = true;
    }

    return bRet;
}

TextFrameIndex SwCursorShell::GetCursorPointAsViewIndex() const
{
    SwPosition const*const pPos(GetCursor()->GetPoint());
    SwTextNode const*const pTextNode(pPos->GetNode().GetTextNode());
    assert(pTextNode);
    SwTextFrame const*const pFrame(static_cast<SwTextFrame const*>(pTextNode->getLayoutFrame(GetLayout())));
    assert(pFrame);
    return pFrame->MapModelToViewPos(*pPos);
}

bool SwCursorShell::SelectTextView(TextFrameIndex const nStart,
                                 TextFrameIndex const nEnd)
{
    CurrShell aCurr( this );
    bool bRet = false;

    SwCallLink aLk( *this );
    SwCursorSaveState aSaveState( *m_pCurrentCursor );

    SwPosition& rPos = *m_pCurrentCursor->GetPoint();
    m_pCurrentCursor->DeleteMark();
    // indexes must correspond to cursor point!
    SwTextFrame const*const pFrame(static_cast<SwTextFrame const*>(m_pCurrentCursor->GetPoint()->GetNode().GetTextNode()->getLayoutFrame(GetLayout())));
    assert(pFrame);
    rPos = pFrame->MapViewToModelPos(nStart);
    m_pCurrentCursor->SetMark();
    rPos = pFrame->MapViewToModelPos(nEnd);

    if (!m_pCurrentCursor->IsSelOvr())
    {
        UpdateCursor();
        bRet = true;
    }

    return bRet;
}

bool SwCursorShell::SelectTextAttr( sal_uInt16 nWhich,
                                     bool bExpand,
                                     const SwTextAttr* pTextAttr )
{
    CurrShell aCurr( this );

    if( IsTableMode() )
        return false;

    if( !pTextAttr )
    {
        SwPosition& rPos = *m_pCurrentCursor->GetPoint();
        SwTextNode* pTextNd = rPos.GetNode().GetTextNode();
        pTextAttr = pTextNd
            ? pTextNd->GetTextAttrAt(rPos.GetContentIndex(),
                    nWhich,
                    bExpand ? ::sw::GetTextAttrMode::Expand : ::sw::GetTextAttrMode::Default)
            : nullptr;
    }
    if( !pTextAttr )
        return false;

    const sal_Int32* pEnd = pTextAttr->End();
    bool bRet = SelectTextModel(pTextAttr->GetStart(), (pEnd ? *pEnd : pTextAttr->GetStart() + 1));
    return bRet;
}

bool SwCursorShell::GotoINetAttr( const SwTextINetFormat& rAttr )
{
    if( !rAttr.GetpTextNode() )
        return false;
    SwCursor* pCursor = getShellCursor( true );

    CurrShell aCurr( this );
    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursorSaveState aSaveState( *pCursor );

    pCursor->GetPoint()->Assign(*rAttr.GetpTextNode(), rAttr.GetStart() );
    bool bRet = !pCursor->IsSelOvr();
    if( bRet )
        UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
    return bRet;
}

const SwFormatINetFormat* SwCursorShell::FindINetAttr( std::u16string_view rName ) const
{
    return mxDoc->FindINetAttr( rName );
}

bool SwCursorShell::GetShadowCursorPos( const Point& rPt, SwFillMode eFillMode,
                                SwRect& rRect, sal_Int16& rOrient )
{

    CurrShell aCurr( this );

    if (IsTableMode() || HasSelection()
        || !GetDoc()->GetIDocumentUndoRedo().DoesUndo())
        return false;

    Point aPt( rPt );
    SwPosition aPos( *m_pCurrentCursor->GetPoint() );

    SwFillCursorPos aFPos( eFillMode );
    SwCursorMoveState aTmpState( &aFPos );

    bool bRet = false;
    if( GetLayout()->GetModelPositionForViewPoint( &aPos, aPt, &aTmpState ) &&
        !aPos.GetNode().IsProtect())
    {
        // start position in protected section?
        rRect = aFPos.aCursor;
        rOrient = aFPos.eOrient;
        bRet = true;
    }
    return bRet;
}

bool SwCursorShell::SetShadowCursorPos( const Point& rPt, SwFillMode eFillMode )
{
    CurrShell aCurr( this );

    if (IsTableMode() || HasSelection()
        || !GetDoc()->GetIDocumentUndoRedo().DoesUndo())
        return false;

    Point aPt( rPt );
    SwPosition aPos( *m_pCurrentCursor->GetPoint() );

    SwFillCursorPos aFPos( eFillMode );
    SwCursorMoveState aTmpState( &aFPos );

    if( !GetLayout()->GetModelPositionForViewPoint( &aPos, aPt, &aTmpState ) )
        return false;

    SwCallLink aLk( *this ); // watch Cursor-Moves
    StartAction();

    SwContentNode* pCNd = aPos.GetNode().GetContentNode();
    SwUndoId nUndoId = SwUndoId::INS_FROM_SHADOWCRSR;
    // If only the paragraph attributes "Adjust" or "LRSpace" are set,
    // then the following should not delete those again.
    if( 0 == aFPos.nParaCnt + aFPos.nColumnCnt &&
        ( SwFillMode::Indent == aFPos.eMode ||
          ( text::HoriOrientation::NONE != aFPos.eOrient &&
            0 == aFPos.nTabCnt + aFPos.nSpaceCnt )) &&
        pCNd && pCNd->Len() )
        nUndoId = SwUndoId::EMPTY;

    GetDoc()->GetIDocumentUndoRedo().StartUndo( nUndoId, nullptr );

    SwTextFormatColl* pNextFormat = nullptr;
    SwTextNode* pTNd = pCNd ? pCNd->GetTextNode() : nullptr;
    if( pTNd )
        pNextFormat = &pTNd->GetTextColl()->GetNextTextFormatColl();

    const SwSectionNode* pSectNd = pCNd ? pCNd->FindSectionNode() : nullptr;
    if( pSectNd && aFPos.nParaCnt )
    {
        SwNodeIndex aEnd( aPos.GetNode(), 1 );
        while( aEnd.GetNode().IsEndNode() &&
                &aEnd.GetNode() !=
                pSectNd->EndOfSectionNode() )
            ++aEnd;

        if( aEnd.GetNode().IsEndNode() &&
            pCNd->Len() == aPos.GetContentIndex() )
            aPos.Assign( *pSectNd->EndOfSectionNode() );
    }

    for( sal_uInt16 n = 0; n < aFPos.nParaCnt + aFPos.nColumnCnt; ++n )
    {
        GetDoc()->getIDocumentContentOperations().AppendTextNode( aPos );
        if( !n && pNextFormat )
        {
            *m_pCurrentCursor->GetPoint() = aPos;
            GetDoc()->SetTextFormatColl( *m_pCurrentCursor, pNextFormat, false );
        }
        if( n < aFPos.nColumnCnt )
        {
            *m_pCurrentCursor->GetPoint() = aPos;
            GetDoc()->getIDocumentContentOperations().InsertPoolItem( *m_pCurrentCursor,
                    SvxFormatBreakItem( SvxBreak::ColumnBefore, RES_BREAK ) );
        }
    }

    *m_pCurrentCursor->GetPoint() = aPos;
    switch( aFPos.eMode )
    {
    case SwFillMode::Indent:
        if( nullptr != (pCNd = aPos.GetNode().GetContentNode() ))
        {
            assert(pCNd->IsTextNode()); // ???
            SfxItemSetFixed<
                    RES_PARATR_ADJUST, RES_PARATR_ADJUST,
                    RES_MARGIN_FIRSTLINE, RES_MARGIN_TEXTLEFT> aSet(GetDoc()->GetAttrPool());
            SvxFirstLineIndentItem firstLine(pCNd->GetAttr(RES_MARGIN_FIRSTLINE));
            SvxTextLeftMarginItem leftMargin(pCNd->GetAttr(RES_MARGIN_TEXTLEFT));
            firstLine.SetTextFirstLineOffset(0);
            leftMargin.SetTextLeft(aFPos.nTabCnt);
            aSet.Put(firstLine);
            aSet.Put(leftMargin);

            const SvxAdjustItem& rAdj = pCNd->GetAttr(RES_PARATR_ADJUST);
            if( SvxAdjust::Left != rAdj.GetAdjust() )
                aSet.Put( SvxAdjustItem( SvxAdjust::Left, RES_PARATR_ADJUST ) );

            GetDoc()->getIDocumentContentOperations().InsertItemSet( *m_pCurrentCursor, aSet );
        }
        else {
            OSL_ENSURE( false, "No ContentNode" );
        }
        break;

    case SwFillMode::Tab:
    case SwFillMode::TabSpace:
    case SwFillMode::Space:
        {
            OUStringBuffer sInsert;
            if (aFPos.eMode == SwFillMode::Space)
            {
                comphelper::string::padToLength(sInsert, sInsert.getLength() + aFPos.nSpaceOnlyCnt, ' ');
            }
            else
            {
                if (aFPos.nTabCnt)
                    comphelper::string::padToLength(sInsert, aFPos.nTabCnt, '\t');
                if (aFPos.nSpaceCnt)
                    comphelper::string::padToLength(sInsert, sInsert.getLength() + aFPos.nSpaceCnt, ' ');
            }
            if (!sInsert.isEmpty())
                GetDoc()->getIDocumentContentOperations().InsertString( *m_pCurrentCursor, sInsert.makeStringAndClear());
        }
        [[fallthrough]]; // still need to set orientation
    case SwFillMode::Margin:
        if( text::HoriOrientation::NONE != aFPos.eOrient )
        {
            SvxAdjustItem aAdj( SvxAdjust::Left, RES_PARATR_ADJUST );
            switch( aFPos.eOrient )
            {
            case text::HoriOrientation::CENTER:
                aAdj.SetAdjust( SvxAdjust::Center );
                break;
            case text::HoriOrientation::RIGHT:
                aAdj.SetAdjust( SvxAdjust::Right );
                break;
            default:
                break;
            }
            GetDoc()->getIDocumentContentOperations().InsertPoolItem( *m_pCurrentCursor, aAdj );
        }
        break;
    }

    GetDoc()->GetIDocumentUndoRedo().EndUndo( nUndoId, nullptr );
    EndAction();

    return true;
}

const SwRangeRedline* SwCursorShell::SelNextRedline()
{
    if( IsTableMode() )
        return nullptr;

    CurrShell aCurr( this );
    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursorSaveState aSaveState( *m_pCurrentCursor );

    // ensure point is at the end so alternating SelNext/SelPrev works
    NormalizePam(false);
    const SwRangeRedline* pFnd = GetDoc()->getIDocumentRedlineAccess().SelNextRedline( *m_pCurrentCursor );

    // at the end of the document, go to the start of the document, and try again
    if ( !pFnd )
    {
        GetDoc()->GetDocShell()->GetWrtShell()->StartOfSection();
        pFnd = GetDoc()->getIDocumentRedlineAccess().SelNextRedline( *m_pCurrentCursor );
    }

    if( pFnd && !m_pCurrentCursor->IsInProtectTable() && !m_pCurrentCursor->IsSelOvr() )
        UpdateCursor( SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
    else
        pFnd = nullptr;
    return pFnd;
}

const SwRangeRedline* SwCursorShell::SelPrevRedline()
{
    if( IsTableMode() )
        return nullptr;

    CurrShell aCurr( this );
    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursorSaveState aSaveState( *m_pCurrentCursor );

    // ensure point is at the start so alternating SelNext/SelPrev works
    NormalizePam(true);
    const SwRangeRedline* pFnd = GetDoc()->getIDocumentRedlineAccess().SelPrevRedline( *m_pCurrentCursor );

    // at the start of the document, go to the end of the document, and try again
    if ( !pFnd )
    {
        GetDoc()->GetDocShell()->GetWrtShell()->EndOfSection();
        pFnd = GetDoc()->getIDocumentRedlineAccess().SelPrevRedline( *m_pCurrentCursor );
    }

    if( pFnd && !m_pCurrentCursor->IsInProtectTable() && !m_pCurrentCursor->IsSelOvr() )
        UpdateCursor( SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
    else
        pFnd = nullptr;
    return pFnd;
}

const SwRangeRedline* SwCursorShell::GotoRedline_( SwRedlineTable::size_type nArrPos, bool bSelect )
{
    const SwRangeRedline* pFnd = nullptr;
    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursorSaveState aSaveState( *m_pCurrentCursor );

    pFnd = GetDoc()->getIDocumentRedlineAccess().GetRedlineTable()[ nArrPos ];
    if( !pFnd )
        return nullptr;

    *m_pCurrentCursor->GetPoint() = *pFnd->Start();

    SwPosition* pPtPos = m_pCurrentCursor->GetPoint();
    if( !pPtPos->GetNode().IsContentNode() )
    {
        SwContentNode* pCNd = GetDoc()->GetNodes().GoNextSection( pPtPos,
                                true, IsReadOnlyAvailable() );
        if( pCNd )
        {
            if( pPtPos->GetNode() <= pFnd->End()->GetNode() )
                pPtPos->SetContent( 0 );
            else
                pFnd = nullptr;
        }
    }

    if( pFnd && bSelect )
    {
        m_pCurrentCursor->SetMark();
        if( RedlineType::FmtColl == pFnd->GetType() )
        {
            SwContentNode* pCNd = pPtPos->GetNode().GetContentNode();
            m_pCurrentCursor->GetPoint()->SetContent( pCNd->Len() );
            m_pCurrentCursor->GetMark()->Assign( *pCNd, 0 );
        }
        else
            *m_pCurrentCursor->GetPoint() = *pFnd->End();

        pPtPos = m_pCurrentCursor->GetPoint();
        if( !pPtPos->GetNode().IsContentNode() )
        {
            SwContentNode* pCNd = SwNodes::GoPrevSection( pPtPos,
                                        true, IsReadOnlyAvailable() );
            if( pCNd )
            {
                if( pPtPos->GetNode() >= m_pCurrentCursor->GetMark()->GetNode() )
                    pPtPos->SetContent( pCNd->Len() );
                else
                    pFnd = nullptr;
            }
        }
    }

    if( !pFnd )
    {
        m_pCurrentCursor->DeleteMark();
        m_pCurrentCursor->RestoreSavePos();
    }
    else if( bSelect && *m_pCurrentCursor->GetMark() == *m_pCurrentCursor->GetPoint() )
        m_pCurrentCursor->DeleteMark();

    if( pFnd && !m_pCurrentCursor->IsInProtectTable() && !m_pCurrentCursor->IsSelOvr() )
        UpdateCursor( SwCursorShell::SCROLLWIN | SwCursorShell::CHKRANGE
                    | SwCursorShell::READONLY );
    else
    {
        pFnd = nullptr;
        if( bSelect )
            m_pCurrentCursor->DeleteMark();
    }
    return pFnd;
}

const SwRangeRedline* SwCursorShell::GotoRedline( SwRedlineTable::size_type nArrPos, bool bSelect )
{
    const SwRangeRedline* pFnd = nullptr;
    if( IsTableMode() )
        return nullptr;

    CurrShell aCurr( this );

    const SwRedlineTable& rTable = GetDoc()->getIDocumentRedlineAccess().GetRedlineTable();
    const SwRangeRedline* pTmp = rTable[ nArrPos ];
    sal_uInt16 nSeqNo = pTmp->GetSeqNo();
    if( !nSeqNo || !bSelect )
    {
        pFnd = GotoRedline_( nArrPos, bSelect );
        return pFnd;
    }

    bool bCheck = false;
    int nLoopCnt = 2;
    SwRedlineTable::size_type nArrSavPos = nArrPos;

    do {
        pTmp = GotoRedline_( nArrPos, true );

        if( !pFnd )
            pFnd = pTmp;

        if( pTmp && bCheck )
        {
            // Check for overlaps. These can happen when FormatColl-
            // Redlines were stretched over a whole paragraph
            SwPaM* pCur = m_pCurrentCursor;
            SwPaM* pNextPam = pCur->GetNext();
            auto [pCStt, pCEnd] = pCur->StartEnd(); // SwPosition*
            while( pCur != pNextPam )
            {
                auto [pNStt, pNEnd] = pNextPam->StartEnd(); // SwPosition*

                bool bDel = true;
                switch( ::ComparePosition( *pCStt, *pCEnd,
                                           *pNStt, *pNEnd ))
                {
                case SwComparePosition::Inside:         // Pos1 is completely in Pos2
                    if( !pCur->HasMark() )
                    {
                        pCur->SetMark();
                        *pCur->GetMark() = *pNStt;
                    }
                    else
                        *pCStt = *pNStt;
                    *pCEnd = *pNEnd;
                    break;

                case SwComparePosition::Outside:        // Pos2 is completely in Pos1
                case SwComparePosition::Equal:          // Pos1 has same size as Pos2
                    break;

                case SwComparePosition::OverlapBefore: // Pos1 overlaps Pos2 at beginning
                    if( !pCur->HasMark() )
                        pCur->SetMark();
                    *pCEnd = *pNEnd;
                    break;
                case SwComparePosition::OverlapBehind: // Pos1 overlaps Pos2 at end
                    if( !pCur->HasMark() )
                    {
                        pCur->SetMark();
                        *pCur->GetMark() = *pNStt;
                    }
                    else
                        *pCStt = *pNStt;
                    break;

                default:
                    bDel = false;
                }

                if( bDel )
                {
                    // not needed anymore
                    SwPaM* pPrevPam = pNextPam->GetPrev();
                    delete pNextPam;
                    pNextPam = pPrevPam;
                }
                pNextPam = pNextPam->GetNext();
            }
        }

        SwRedlineTable::size_type nFndPos = 2 == nLoopCnt
                            ? rTable.FindNextOfSeqNo( nArrPos )
                            : rTable.FindPrevOfSeqNo( nArrPos );
        if( SwRedlineTable::npos != nFndPos ||
            ( 0 != ( --nLoopCnt ) && SwRedlineTable::npos != (
                    nFndPos = rTable.FindPrevOfSeqNo( nArrSavPos ))) )
        {
            if( pTmp )
            {
                // create new cursor
                CreateCursor();
                bCheck = true;
            }
            nArrPos = nFndPos;
        }
        else
            nLoopCnt = 0;

    } while( nLoopCnt );
    return pFnd;
}

bool SwCursorShell::SelectNxtPrvHyperlink( bool bNext )
{
    SwNodes& rNds = GetDoc()->GetNodes();
    const SwNode* pBodyEndNd = &rNds.GetEndOfContent();
    const SwNode* pBodySttNd = pBodyEndNd->StartOfSectionNode();
    SwNodeOffset nBodySttNdIdx = pBodySttNd->GetIndex();
    Point aPt;

    SetGetExpField aCmpPos( SwPosition( bNext ? *pBodyEndNd : *pBodySttNd ) );
    SetGetExpField aCurPos( bNext ? *m_pCurrentCursor->End() : *m_pCurrentCursor->Start() );
    if( aCurPos.GetNode() < nBodySttNdIdx )
    {
        const SwContentNode* pCNd = aCurPos.GetNodeFromContent()->GetContentNode();
        std::pair<Point, bool> tmp(aPt, true);
        if (pCNd)
        {
            SwContentFrame* pFrame = pCNd->getLayoutFrame(GetLayout(), nullptr, &tmp);
            if( pFrame )
                aCurPos.SetBodyPos( *pFrame );
        }
    }

    // check first all the hyperlink fields
    {
        const SwTextNode* pTextNd;
        const SwCharFormats* pFormats = GetDoc()->GetCharFormats();
        for( SwCharFormats::size_type n = pFormats->size(); 1 < n; )
        {
            SwIterator<SwTextINetFormat,SwCharFormat> aIter(*(*pFormats)[--n]);

            for( SwTextINetFormat* pFnd = aIter.First(); pFnd; pFnd = aIter.Next() )
            {
                pTextNd = pFnd->GetpTextNode();
                if( pTextNd && pTextNd->GetNodes().IsDocNodes() )
                {
                    SwTextINetFormat& rAttr = *pFnd;
                    SetGetExpField aPos( *pTextNd, rAttr );
                    if (pTextNd->GetIndex() < nBodySttNdIdx)
                    {
                        std::pair<Point, bool> tmp(aPt, true);
                        SwContentFrame* pFrame = pTextNd->getLayoutFrame(GetLayout(), nullptr, &tmp);
                        if (pFrame)
                        {
                            aPos.SetBodyPos( *pFrame );
                        }
                    }

                    if( bNext
                        ? ( aPos < aCmpPos && aCurPos < aPos )
                        : ( aCmpPos < aPos && aPos < aCurPos ))
                    {
                        OUString sText(pTextNd->GetExpandText(GetLayout(),
                                        rAttr.GetStart(),
                                        *rAttr.GetEnd() - rAttr.GetStart() ) );

                        sText = sText.replaceAll("\x0a", "");
                        sText = comphelper::string::strip(sText, ' ');

                        if( !sText.isEmpty() )
                            aCmpPos = aPos;
                    }
                }
            }
        }
    }

    // then check all the Flys with a URL or image map
    {
        for(sw::SpzFrameFormat* pSpz: *GetDoc()->GetSpzFrameFormats())
        {
            auto pFormat = static_cast<SwFlyFrameFormat*>(pSpz);
            const SwFormatURL& rURLItem = pFormat->GetURL();
            if( rURLItem.GetMap() || !rURLItem.GetURL().isEmpty() )
            {
                SwFlyFrame* pFly = pFormat->GetFrame( &aPt );
                SwPosition aTmpPos( *pBodySttNd );
                if( pFly &&
                    GetBodyTextNode( *GetDoc(), aTmpPos, *pFly->GetLower() ) )
                {
                    SetGetExpField aPos( *pFormat, &aTmpPos );

                    if( bNext
                            ? ( aPos < aCmpPos && aCurPos < aPos )
                            : ( aCmpPos < aPos && aPos < aCurPos ))
                        aCmpPos = aPos;
                }
            }
        }
    }

    // found any URL ?
    const SwTextINetFormat* pFndAttr = aCmpPos.GetINetFormat();
    const SwFlyFrameFormat* pFndFormat = aCmpPos.GetFlyFormat();
    if( !pFndAttr && !pFndFormat )
        return false;

    CurrShell aCurr( this );
    SwCallLink aLk( *this );

    bool bRet = false;
    // found a text attribute ?
    if( pFndAttr )
    {
        SwCursorSaveState aSaveState( *m_pCurrentCursor );

        aCmpPos.GetPosOfContent( *m_pCurrentCursor->GetPoint() );
        m_pCurrentCursor->DeleteMark();
        m_pCurrentCursor->SetMark();
        m_pCurrentCursor->GetPoint()->SetContent( *pFndAttr->End() );

        if( !m_pCurrentCursor->IsInProtectTable() && !m_pCurrentCursor->IsSelOvr() )
        {
            UpdateCursor( SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|
                                SwCursorShell::READONLY );
            bRet = true;
        }
    }
    // found a draw object ?
    else if( RES_DRAWFRMFMT == pFndFormat->Which() )
    {
        const SdrObject* pSObj = pFndFormat->FindSdrObject();
        if (pSObj)
        {
            static_cast<SwFEShell*>(this)->SelectObj( pSObj->GetCurrentBoundRect().Center() );
            MakeSelVisible();
            bRet = true;
        }
    }
    else // then is it a fly
    {
        SwFlyFrame* pFly = pFndFormat->GetFrame(&aPt);
        if( pFly )
        {
            static_cast<SwFEShell*>(this)->SelectFlyFrame( *pFly );
            MakeSelVisible();
            bRet = true;
        }
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
