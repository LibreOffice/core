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
#include <hintids.hxx>
#include <comphelper/string.hxx>
#include <svl/itemiter.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <svx/svdobj.hxx>
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
#include <callnk.hxx>
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
#include "txtfrm.hxx"
#include <wrong.hxx>
#include <calbck.hxx>
#include <vcl/window.hxx>
#include <docufld.hxx>

using namespace ::com::sun::star;

void SwCursorShell::MoveCursorToNum()
{
    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursorSaveState aSaveState( *m_pCurrentCursor );
    if( ActionPend() )
        return;
    SET_CURR_SHELL( this );
    // try to set cursor onto this position, at half of the char-
    // SRectangle's height
    Point aPt( m_pCurrentCursor->GetPtPos() );
    SwContentFrame * pFrame = m_pCurrentCursor->GetContentNode()->getLayoutFrame( GetLayout(), &aPt,
                                                m_pCurrentCursor->GetPoint() );
    pFrame->GetCharRect( m_aCharRect, *m_pCurrentCursor->GetPoint() );
    pFrame->Calc(GetOut());
    if( pFrame->IsVertical() )
    {
        aPt.setX(m_aCharRect.Center().getX());
        aPt.setY(pFrame->Frame().Top() + GetUpDownX());
    }
    else
    {
        aPt.setY(m_aCharRect.Center().getY());
        aPt.setX(pFrame->Frame().Left() + GetUpDownX());
    }
    pFrame->GetCursorOfst( m_pCurrentCursor->GetPoint(), aPt );
    if ( !m_pCurrentCursor->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                                nsSwCursorSelOverFlags::SELOVER_CHANGEPOS ))
    {
        UpdateCursor(SwCursorShell::UPDOWN |
                SwCursorShell::SCROLLWIN | SwCursorShell::CHKRANGE |
                SwCursorShell::READONLY );
    }
}

/// go to next/previous point on the same level
bool SwCursorShell::GotoNextNum()
{
    if (!SwDoc::GotoNextNum( *m_pCurrentCursor->GetPoint() ))
        return false;
    MoveCursorToNum();
    return true;
}

bool SwCursorShell::GotoPrevNum()
{
    if (!SwDoc::GotoPrevNum( *m_pCurrentCursor->GetPoint() ))
        return false;
    MoveCursorToNum();
    return true;
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

    if( pFrame )
    {
        SET_CURR_SHELL( this );
        // get header frame
        SwCallLink aLk( *this ); // watch Cursor-Moves
        SwCursor *pTmpCursor = getShellCursor( true );
        SwCursorSaveState aSaveState( *pTmpCursor );
        pFrame->Calc(GetOut());
        Point aPt( pFrame->Frame().Pos() + pFrame->Prt().Pos() );
        pFrame->GetCursorOfst( pTmpCursor->GetPoint(), aPt );
        if( !pTmpCursor->IsSelOvr() )
            UpdateCursor();
        else
            pFrame = nullptr;
    }
    return nullptr != pFrame;
}

/// jump from content to footer
bool SwCursorShell::GotoFooterText()
{
    const SwPageFrame* pFrame = GetCurrFrame()->FindPageFrame();
    if( pFrame )
    {
        const SwFrame* pLower = pFrame->GetLastLower();

        while( pLower && !pLower->IsFooterFrame() )
            pLower = pLower->GetLower();
        // found footer, search 1. content frame
        while( pLower && !pLower->IsContentFrame() )
            pLower = pLower->GetLower();

        if( pLower )
        {
            SwCursor *pTmpCursor = getShellCursor( true );
            SET_CURR_SHELL( this );
            // get position in footer
            SwCallLink aLk( *this ); // watch Cursor-Moves
            SwCursorSaveState aSaveState( *pTmpCursor );
            pLower->Calc(GetOut());
            Point aPt( pLower->Frame().Pos() + pLower->Prt().Pos() );
            pLower->GetCursorOfst( pTmpCursor->GetPoint(), aPt );
            if( !pTmpCursor->IsSelOvr() )
                UpdateCursor();
            else
                pFrame = nullptr;
        }
        else
            pFrame = nullptr;
    }
    else
        pFrame = nullptr;
    return nullptr != pFrame;
}

bool SwCursorShell::SetCursorInHdFt( size_t nDescNo, bool bInHeader )
{
    bool bRet = false;
    SwDoc *pMyDoc = GetDoc();
    const SwPageDesc* pDesc = nullptr;

    SET_CURR_SHELL( this );

    if( SIZE_MAX == nDescNo )
    {
        // take the current one
        const SwPageFrame* pPage = GetCurrFrame()->FindPageFrame();
        if( pPage && pMyDoc->ContainsPageDesc(
                pPage->GetPageDesc(), &nDescNo) )
            pDesc = pPage->GetPageDesc();
    }
    else
        if (nDescNo < pMyDoc->GetPageDescCnt())
            pDesc = &pMyDoc->GetPageDesc( nDescNo );

    if( pDesc )
    {
        // check if the attribute exists
        const SwFormatContent* pCnt = nullptr;
        if( bInHeader )
        {
            // mirrored pages? ignore for now
            const SwFormatHeader& rHd = pDesc->GetMaster().GetHeader();
            if( rHd.GetHeaderFormat() )
                pCnt = &rHd.GetHeaderFormat()->GetContent();
        }
        else
        {
            const SwFormatFooter& rFt = pDesc->GetMaster().GetFooter();
            if( rFt.GetFooterFormat() )
                pCnt = &rFt.GetFooterFormat()->GetContent();
        }

        if( pCnt && pCnt->GetContentIdx() )
        {
            SwNodeIndex aIdx( *pCnt->GetContentIdx(), 1 );
            SwContentNode* pCNd = aIdx.GetNode().GetContentNode();
            if( !pCNd )
                pCNd = pMyDoc->GetNodes().GoNext( &aIdx );

            Point aPt( m_pCurrentCursor->GetPtPos() );

            if( pCNd && nullptr != pCNd->getLayoutFrame( GetLayout(), &aPt, nullptr, false ) )
            {
                // then we can set the cursor in here
                SwCallLink aLk( *this ); // watch Cursor-Moves
                SwCursorSaveState aSaveState( *m_pCurrentCursor );

                ClearMark();

                SwPosition& rPos = *m_pCurrentCursor->GetPoint();
                rPos.nNode = *pCNd;
                rPos.nContent.Assign( pCNd, 0 );

                bRet = !m_pCurrentCursor->IsSelOvr();
                if( bRet )
                    UpdateCursor( SwCursorShell::SCROLLWIN | SwCursorShell::CHKRANGE |
                                SwCursorShell::READONLY );
            }
        }
    }
    return bRet;
}

/// jump to the next index
bool SwCursorShell::GotoNextTOXBase( const OUString* pName )
{
    bool bRet = false;

    const SwSectionFormats& rFormats = GetDoc()->GetSections();
    SwContentNode* pFnd = nullptr;
    for( SwSectionFormats::size_type n = rFormats.size(); n; )
    {
        const SwSection* pSect = rFormats[ --n ]->GetSection();
        if (TOX_CONTENT_SECTION == pSect->GetType())
        {
            SwSectionNode const*const pSectNd(
                    pSect->GetFormat()->GetSectionNode());
            if (   pSectNd
                && m_pCurrentCursor->GetPoint()->nNode < pSectNd->GetIndex()
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
    if( pFnd )
    {
        SwCallLink aLk( *this ); // watch Cursor-Moves
        SwCursorSaveState aSaveState( *m_pCurrentCursor );
        m_pCurrentCursor->GetPoint()->nNode = *pFnd;
        m_pCurrentCursor->GetPoint()->nContent.Assign( pFnd, 0 );
        bRet = !m_pCurrentCursor->IsSelOvr();
        if( bRet )
            UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
    }
    return bRet;
}

/// jump to previous index
bool SwCursorShell::GotoPrevTOXBase( const OUString* pName )
{
    bool bRet = false;

    const SwSectionFormats& rFormats = GetDoc()->GetSections();
    SwContentNode* pFnd = nullptr;
    for( SwSectionFormats::size_type n = rFormats.size(); n; )
    {
        const SwSection* pSect = rFormats[ --n ]->GetSection();
        if (TOX_CONTENT_SECTION == pSect->GetType())
        {
            SwSectionNode const*const pSectNd(
                    pSect->GetFormat()->GetSectionNode());
            if (   pSectNd
                && m_pCurrentCursor->GetPoint()->nNode > pSectNd->EndOfSectionIndex()
                && (!pFnd  || pFnd->GetIndex() < pSectNd->GetIndex())
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

    if( pFnd )
    {
        SwCallLink aLk( *this ); // watch Cursor-Moves
        SwCursorSaveState aSaveState( *m_pCurrentCursor );
        m_pCurrentCursor->GetPoint()->nNode = *pFnd;
        m_pCurrentCursor->GetPoint()->nContent.Assign( pFnd, 0 );
        bRet = !m_pCurrentCursor->IsSelOvr();
        if( bRet )
            UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
    }
    return bRet;
}

/// jump to index of TOXMark
bool SwCursorShell::GotoTOXMarkBase()
{
    bool bRet = false;

    SwTOXMarks aMarks;
    sal_uInt16 nCnt = SwDoc::GetCurTOXMark( *m_pCurrentCursor->GetPoint(), aMarks );
    if( nCnt )
    {
        // Take the 1. and get the index type. Search in its dependency list
        // for the actual index
        const SwTOXType* pType = aMarks[0]->GetTOXType();
        SwIterator<SwTOXBase,SwTOXType> aIter( *pType );
        const SwSectionFormat* pSectFormat;

        for( SwTOXBase* pTOX = aIter.First(); pTOX; pTOX = aIter.Next() )
        {
            const SwSectionNode* pSectNd;
            if( dynamic_cast<const SwTOXBaseSection*>( pTOX) !=  nullptr &&
                nullptr != ( pSectFormat = static_cast<SwTOXBaseSection*>(pTOX)->GetFormat() ) &&
                nullptr != ( pSectNd = pSectFormat->GetSectionNode() ))
            {
                SwNodeIndex aIdx( *pSectNd, 1 );
                SwContentNode* pCNd = aIdx.GetNode().GetContentNode();
                if( !pCNd )
                    pCNd = GetDoc()->GetNodes().GoNext( &aIdx );
                const SwContentFrame* pCFrame;
                if( pCNd &&
                    pCNd->EndOfSectionIndex() < pSectNd->EndOfSectionIndex() &&
                    nullptr != ( pCFrame = pCNd->getLayoutFrame( GetLayout() ) ) &&
                    ( IsReadOnlyAvailable() || !pCFrame->IsProtected() ))
                {
                    SwCallLink aLk( *this ); // watch Cursor-Moves
                    SwCursorSaveState aSaveState( *m_pCurrentCursor );
                    m_pCurrentCursor->GetPoint()->nNode = *pCNd;
                    m_pCurrentCursor->GetPoint()->nContent.Assign( pCNd, 0 );
                    bRet = !m_pCurrentCursor->IsInProtectTable() &&
                            !m_pCurrentCursor->IsSelOvr();
                    if( bRet )
                        UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
                    break;
                }
            }
    }
    }
    return bRet;
}

/// Jump to next/previous table formula
/// Optionally it is possible to also jump to broken formulas
bool SwCursorShell::GotoNxtPrvTableFormula( bool bNext, bool bOnlyErrors )
{
    if( IsTableMode() )
        return false;

    bool bFnd = false;
    SwPosition& rPos = *m_pCurrentCursor->GetPoint();

    Point aPt;
    SwPosition aFndPos( GetDoc()->GetNodes().GetEndOfContent() );
    if( !bNext )
        aFndPos.nNode = 0;
    _SetGetExpField aFndGEF( aFndPos ), aCurGEF( rPos );

    {
        const SwNode* pSttNd = rPos.nNode.GetNode().FindTableBoxStartNode();
        if( pSttNd )
        {
            const SwTableBox* pTBox = pSttNd->FindTableNode()->GetTable().
                                        GetTableBox( pSttNd->GetIndex() );
            if( pTBox )
                aCurGEF = _SetGetExpField( *pTBox );
        }
    }

    if( rPos.nNode < GetDoc()->GetNodes().GetEndOfExtras() )
        // also at collection use only the first frame
        aCurGEF.SetBodyPos( *rPos.nNode.GetNode().GetContentNode()->getLayoutFrame( GetLayout(),
                                &aPt, &rPos, false ) );
    {
        sal_uInt32 n, nMaxItems = GetDoc()->GetAttrPool().GetItemCount2( RES_BOXATR_FORMULA );

        for( n = 0; n < nMaxItems; ++n )
        {
            const SwTableBox* pTBox;
            const SfxPoolItem* pItem;
            if( nullptr != (pItem = GetDoc()->GetAttrPool().GetItem2(
                                        RES_BOXATR_FORMULA, n ) ) &&
                nullptr != (pTBox = static_cast<const SwTableBoxFormula*>(pItem)->GetTableBox() ) &&
                pTBox->GetSttNd() &&
                pTBox->GetSttNd()->GetNodes().IsDocNodes() &&
                ( !bOnlyErrors ||
                  !static_cast<const SwTableBoxFormula*>(pItem)->HasValidBoxes() ) )
            {
                const SwContentFrame* pCFrame;
                SwNodeIndex aIdx( *pTBox->GetSttNd() );
                const SwContentNode* pCNd = GetDoc()->GetNodes().GoNext( &aIdx );
                if( pCNd && nullptr != ( pCFrame = pCNd->getLayoutFrame( GetLayout(), &aPt, nullptr, false ) ) &&
                    (IsReadOnlyAvailable() || !pCFrame->IsProtected() ))
                {
                    _SetGetExpField aCmp( *pTBox );
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

    if( bFnd )
    {
        SET_CURR_SHELL( this );
        SwCallLink aLk( *this ); // watch Cursor-Moves
        SwCursorSaveState aSaveState( *m_pCurrentCursor );

        aFndGEF.GetPosOfContent( rPos );
        m_pCurrentCursor->DeleteMark();

        bFnd = !m_pCurrentCursor->IsSelOvr();
        if( bFnd )
            UpdateCursor( SwCursorShell::SCROLLWIN | SwCursorShell::CHKRANGE |
                        SwCursorShell::READONLY );
    }
    return bFnd;
}

/// jump to next/previous index marker
bool SwCursorShell::GotoNxtPrvTOXMark( bool bNext )
{
    if( IsTableMode() )
        return false;

    bool bFnd = false;
    SwPosition& rPos = *m_pCurrentCursor->GetPoint();

    Point aPt;
    SwPosition aFndPos( GetDoc()->GetNodes().GetEndOfContent() );
    if( !bNext )
        aFndPos.nNode = 0;
    _SetGetExpField aFndGEF( aFndPos ), aCurGEF( rPos );

    if( rPos.nNode.GetIndex() < GetDoc()->GetNodes().GetEndOfExtras().GetIndex() )
        // also at collection use only the first frame
        aCurGEF.SetBodyPos( *rPos.nNode.GetNode().
                        GetContentNode()->getLayoutFrame( GetLayout(), &aPt, &rPos, false ) );

    {
        const SwTextNode* pTextNd;
        const SwTextTOXMark* pTextTOX;
        sal_uInt32 n, nMaxItems = GetDoc()->GetAttrPool().GetItemCount2( RES_TXTATR_TOXMARK );

        for( n = 0; n < nMaxItems; ++n )
        {
            const SfxPoolItem* pItem;
            const SwContentFrame* pCFrame;

            if( nullptr != (pItem = GetDoc()->GetAttrPool().GetItem2(
                                        RES_TXTATR_TOXMARK, n ) ) &&
                nullptr != (pTextTOX = static_cast<const SwTOXMark*>(pItem)->GetTextTOXMark() ) &&
                ( pTextNd = &pTextTOX->GetTextNode())->GetNodes().IsDocNodes() &&
                nullptr != ( pCFrame = pTextNd->getLayoutFrame( GetLayout(), &aPt, nullptr, false )) &&
                ( IsReadOnlyAvailable() || !pCFrame->IsProtected() ))
            {
                SwNodeIndex aNdIndex( *pTextNd ); // UNIX needs this object
                _SetGetExpField aCmp( aNdIndex, *pTextTOX, nullptr );
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

    if( bFnd )
    {
        SET_CURR_SHELL( this );
        SwCallLink aLk( *this ); // watch Cursor-Moves
        SwCursorSaveState aSaveState( *m_pCurrentCursor );

        aFndGEF.GetPosOfContent( rPos );

        bFnd = !m_pCurrentCursor->IsSelOvr();
        if( bFnd )
            UpdateCursor( SwCursorShell::SCROLLWIN | SwCursorShell::CHKRANGE |
                        SwCursorShell::READONLY );
    }
    return bFnd;
}

/// traveling between marks
const SwTOXMark& SwCursorShell::GotoTOXMark( const SwTOXMark& rStart,
                                            SwTOXSearch eDir )
{
    SET_CURR_SHELL( this );
    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursorSaveState aSaveState( *m_pCurrentCursor );

    const SwTOXMark& rNewMark = GetDoc()->GotoTOXMark( rStart, eDir,
                                                    IsReadOnlyAvailable() );
    // set position
    SwPosition& rPos = *GetCursor()->GetPoint();
    rPos.nNode = rNewMark.GetTextTOXMark()->GetTextNode();
    rPos.nContent.Assign( rPos.nNode.GetNode().GetContentNode(),
                         rNewMark.GetTextTOXMark()->GetStart() );

    if( !m_pCurrentCursor->IsSelOvr() )
        UpdateCursor( SwCursorShell::SCROLLWIN | SwCursorShell::CHKRANGE |
                    SwCursorShell::READONLY );

    return rNewMark;
}

/// jump to next/previous field type
void lcl_MakeFieldLst(
    _SetGetExpFields& rLst,
    const SwFieldType& rFieldType,
    const bool bInReadOnly,
    const bool bChkInpFlag = false )
{
    // always search the 1. frame
    Point aPt;
    SwTextField* pTextField = nullptr;
    SwIterator<SwFormatField,SwFieldType> aIter(rFieldType);
    for( SwFormatField* pFormatField = aIter.First(); pFormatField; pFormatField = aIter.Next() )
    {
        pTextField = pFormatField->GetTextField();
        if ( pTextField != nullptr
             && ( !bChkInpFlag
                  || static_cast<const SwSetExpField*>(pTextField->GetFormatField().GetField())->GetInputFlag() ) )
        {
            const SwTextNode& rTextNode = pTextField->GetTextNode();
            const SwContentFrame* pCFrame =
                rTextNode.getLayoutFrame( rTextNode.GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &aPt, nullptr, false );
            if ( pCFrame != nullptr
                 && ( bInReadOnly || !pCFrame->IsProtected() ) )
            {
                _SetGetExpField* pNew = new _SetGetExpField( SwNodeIndex( rTextNode ), pTextField );
                pNew->SetBodyPos( *pCFrame );
                rLst.insert( pNew );
            }
        }
    }
}

static _SetGetExpFields::const_iterator
lcl_FindField(bool & o_rFound, _SetGetExpFields const& rSrtLst,
        SwRootFrame *const pLayout, SwTextNode *const pTextNode,
        SwTextField *const pTextField, SwPosition const& rPos,
        sal_Int32 const nContentOffset)
{
    std::unique_ptr<_SetGetExpField> pSrch;
    std::unique_ptr<SwIndex> pIndex;
    if (-1 == nContentOffset)
    {
        pSrch.reset(new _SetGetExpField(rPos.nNode, pTextField, &rPos.nContent));
    }
    else
    {
        pIndex.reset(new SwIndex(rPos.nNode.GetNode().GetContentNode(), nContentOffset));
        pSrch.reset(new _SetGetExpField(rPos.nNode, pTextField, pIndex.get()));
    }

    if (rPos.nNode.GetIndex() < pTextNode->GetNodes().GetEndOfExtras().GetIndex())
    {
        // also at collection use only the first frame
        Point aPt;
        pSrch->SetBodyPos(*pTextNode->getLayoutFrame(pLayout, &aPt, &rPos, false));
    }

    _SetGetExpFields::const_iterator it = rSrtLst.lower_bound(pSrch.get());

    o_rFound = (it != rSrtLst.end()) && (**it == *pSrch);
    return it;
}

bool SwCursorShell::MoveFieldType(
    const SwFieldType* pFieldType,
    const bool bNext,
    const sal_uInt16 nResType,
    const bool bAddSetExpressionFieldsToInputFields )
{
    // sorted list of all fields
    _SetGetExpFields aSrtLst;

    if ( pFieldType )
    {
        if( RES_INPUTFLD != pFieldType->Which() && !pFieldType->HasWriterListeners() )
        {
            return false;
        }

        // found Modify object, add all fields to array
        ::lcl_MakeFieldLst( aSrtLst, *pFieldType, IsReadOnlyAvailable() );

        if( RES_INPUTFLD == pFieldType->Which() && bAddSetExpressionFieldsToInputFields )
        {
            // there are hidden input fields in the set exp. fields
            const SwFieldTypes& rFieldTypes = *mpDoc->getIDocumentFieldsAccess().GetFieldTypes();
            const size_t nSize = rFieldTypes.size();
            for( size_t i=0; i < nSize; ++i )
            {
                pFieldType = rFieldTypes[ i ];
                if ( RES_SETEXPFLD == pFieldType->Which() )
                {
                    ::lcl_MakeFieldLst( aSrtLst, *pFieldType, IsReadOnlyAvailable(), true );
                }
            }
        }
    }
    else
    {
        const SwFieldTypes& rFieldTypes = *mpDoc->getIDocumentFieldsAccess().GetFieldTypes();
        const size_t nSize = rFieldTypes.size();
        for( size_t i=0; i < nSize; ++i )
        {
            pFieldType = rFieldTypes[ i ];
            if( nResType == pFieldType->Which() )
            {
                ::lcl_MakeFieldLst( aSrtLst, *pFieldType, IsReadOnlyAvailable() );
            }
        }
    }

    // found no fields?
    if( aSrtLst.empty() )
        return false;

    _SetGetExpFields::const_iterator it;
    SwCursor* pCursor = getShellCursor( true );
    {
        // (1998): Always use field for search so that the right one is found as
        // well some are in frames that are anchored to a paragraph that has a
        // field
        const SwPosition& rPos = *pCursor->GetPoint();

        SwTextNode* pTNd = rPos.nNode.GetNode().GetTextNode();
        OSL_ENSURE( pTNd, "No ContentNode" );

        SwTextField * pTextField = pTNd->GetFieldTextAttrAt( rPos.nContent.GetIndex(), true );
        const bool bDelField = ( pTextField == nullptr );
        sal_Int32 nContentOffset = -1;

        if( bDelField )
        {
            // create dummy for the search
            SwFormatField* pFormatField = new SwFormatField( SwDateTimeField(
                static_cast<SwDateTimeFieldType*>(mpDoc->getIDocumentFieldsAccess().GetSysFieldType( RES_DATETIMEFLD ) ) ) );

            pTextField = new SwTextField( *pFormatField, rPos.nContent.GetIndex(),
                        mpDoc->IsClipBoard() );
            pTextField->ChgTextNode( pTNd );
        }
        else
        {
            // the cursor might be anywhere inside the input field,
            // but we will be searching for the field start
            if (pTextField->Which() == RES_TXTATR_INPUTFIELD
                    && rPos.nContent.GetIndex() != pTextField->GetStart())
                nContentOffset = pTextField->GetStart();
        }
        bool isSrch;
        it = lcl_FindField(isSrch, aSrtLst,
                GetLayout(), pTNd, pTextField, rPos, nContentOffset);

        if( bDelField )
        {
            delete static_cast<SwFormatField*>(&pTextField->GetAttr());
            delete pTextField;
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
    const _SetGetExpField& rFnd = **it;

    SET_CURR_SHELL( this );
    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursorSaveState aSaveState( *pCursor );

    rFnd.GetPosOfContent( *pCursor->GetPoint() );
    bool bRet = !m_pCurrentCursor->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                                     nsSwCursorSelOverFlags::SELOVER_TOGGLE );
    if( bRet )
        UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
    return bRet;
}

bool SwCursorShell::GotoFormatField( const SwFormatField& rField )
{
    bool bRet = false;
    if( rField.GetTextField() )
    {
        SET_CURR_SHELL( this );
        SwCallLink aLk( *this ); // watch Cursor-Moves

        SwCursor* pCursor = getShellCursor( true );
        SwCursorSaveState aSaveState( *pCursor );

        SwTextNode* pTNd = rField.GetTextField()->GetpTextNode();
        pCursor->GetPoint()->nNode = *pTNd;
        pCursor->GetPoint()->nContent.Assign( pTNd, rField.GetTextField()->GetStart() );

        bRet = !pCursor->IsSelOvr();
        if( bRet )
            UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
    }
    return bRet;
}

SwTextField * SwCursorShell::GetTextFieldAtPos(
    const SwPosition* pPos,
    const bool bIncludeInputFieldAtStart )
{
    SwTextField* pTextField = nullptr;

    SwTextNode * const pNode = pPos->nNode.GetNode().GetTextNode();
    if ( pNode != nullptr )
    {
        pTextField = pNode->GetFieldTextAttrAt( pPos->nContent.GetIndex(), bIncludeInputFieldAtStart );
    }

    return pTextField;
}

SwField* SwCursorShell::GetFieldAtCursor(
    const SwPaM* pCursor,
    const bool bIncludeInputFieldAtStart )
{
    SwField* pFieldAtCursor = nullptr;

    SwTextField* pTextField = GetTextFieldAtPos( pCursor->Start(), bIncludeInputFieldAtStart );
    if ( pTextField != nullptr
        && pCursor->Start()->nNode == pCursor->End()->nNode )
    {
        const sal_Int32 nTextFieldLength =
            pTextField->End() != nullptr
            ? *(pTextField->End()) - pTextField->GetStart()
            : 1;
        if ( ( pCursor->End()->nContent.GetIndex() - pCursor->Start()->nContent.GetIndex() ) <= nTextFieldLength )
        {
            pFieldAtCursor = const_cast<SwField*>(pTextField->GetFormatField().GetField());
        }
    }

    return pFieldAtCursor;
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
         && RES_TABLEFLD == pCurField->GetTyp()->Which() )
    {
        // TabellenFormel ? wandel internen in externen Namen um
        const SwTableNode* pTableNd = IsCursorInTable();
        static_cast<SwTableField*>(pCurField)->PtrToBoxNm( pTableNd ? &pTableNd->GetTable() : nullptr );
    }

    return pCurField;
}

bool SwCursorShell::CursorInsideInputField() const
{
    for(SwPaM& rCursor : GetCursor()->GetRingContainer())
    {
        if(dynamic_cast<const SwInputField*>(GetFieldAtCursor( &rCursor, false )))
            return true;
    }
    return false;
}

bool SwCursorShell::PosInsideInputField( const SwPosition& rPos )
{
    return dynamic_cast<const SwTextInputField*>(GetTextFieldAtPos( &rPos, false )) != nullptr;
}

bool SwCursorShell::DocPtInsideInputField( const Point& rDocPt ) const
{
    SwPosition aPos( *(GetCursor()->Start()) );
    Point aDocPt( rDocPt );
    if ( GetLayout()->GetCursorOfst( &aPos, aDocPt ) )
    {
        return PosInsideInputField( aPos );
    }
    return false;
}

sal_Int32 SwCursorShell::StartOfInputFieldAtPos( const SwPosition& rPos )
{
    const SwTextInputField* pTextInputField = dynamic_cast<const SwTextInputField*>(GetTextFieldAtPos( &rPos, true ));
    if ( pTextInputField == nullptr )
    {
        OSL_ENSURE( false, "<SwEditShell::StartOfInputFieldAtPos(..)> - no Input Field at given position" );
        return 0;
    }
    return pTextInputField->GetStart();
}

sal_Int32 SwCursorShell::EndOfInputFieldAtPos( const SwPosition& rPos )
{
    const SwTextInputField* pTextInputField = dynamic_cast<const SwTextInputField*>(GetTextFieldAtPos( &rPos, true ));
    if ( pTextInputField == nullptr )
    {
        OSL_ENSURE( false, "<SwEditShell::EndOfInputFieldAtPos(..)> - no Input Field at given position" );
        return 0;
    }
    return *(pTextInputField->End());
}

void SwCursorShell::GotoOutline( sal_uInt16 nIdx )
{
    SwCursor* pCursor = getShellCursor( true );

    SET_CURR_SHELL( this );
    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursorSaveState aSaveState( *pCursor );

    const SwNodes& rNds = GetDoc()->GetNodes();
    SwTextNode* pTextNd = rNds.GetOutLineNds()[ nIdx ]->GetTextNode();
    pCursor->GetPoint()->nNode = *pTextNd;
    pCursor->GetPoint()->nContent.Assign( pTextNd, 0 );

    if( !pCursor->IsSelOvr() )
        UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
}

bool SwCursorShell::GotoOutline( const OUString& rName )
{
    SwCursor* pCursor = getShellCursor( true );

    SET_CURR_SHELL( this );
    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursorSaveState aSaveState( *pCursor );

    bool bRet = false;
    if( mpDoc->GotoOutline( *pCursor->GetPoint(), rName ) && !pCursor->IsSelOvr() )
    {
        UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
        bRet = true;
    }
    return bRet;
}

/// jump to next node with outline num.
bool SwCursorShell::GotoNextOutline()
{
    SwCursor* pCursor = getShellCursor( true );
    const SwNodes& rNds = GetDoc()->GetNodes();

    SwNode* pNd = &(pCursor->GetNode());
    sal_uInt16 nPos;
    if( rNds.GetOutLineNds().Seek_Entry( pNd, &nPos ))
        ++nPos;

    if( nPos == rNds.GetOutLineNds().size() )
        return false;

    pNd = rNds.GetOutLineNds()[ nPos ];

    SET_CURR_SHELL( this );
    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursorSaveState aSaveState( *pCursor );
    pCursor->GetPoint()->nNode = *pNd;
    pCursor->GetPoint()->nContent.Assign( pNd->GetTextNode(), 0 );

    bool bRet = !pCursor->IsSelOvr();
    if( bRet )
        UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
    return bRet;
}

/// jump to previous node with outline num.
bool SwCursorShell::GotoPrevOutline()
{
    SwCursor* pCursor = getShellCursor( true );
    const SwNodes& rNds = GetDoc()->GetNodes();

    SwNode* pNd = &(pCursor->GetNode());
    sal_uInt16 nPos;
    bool bRet = rNds.GetOutLineNds().Seek_Entry(pNd, &nPos);
    if (bRet && nPos)
    {
        --nPos; // before

        pNd = rNds.GetOutLineNds()[ nPos ];
        if( pNd->GetIndex() > pCursor->GetPoint()->nNode.GetIndex() )
            return false;

        SET_CURR_SHELL( this );
        SwCallLink aLk( *this ); // watch Cursor-Moves
        SwCursorSaveState aSaveState( *pCursor );
        pCursor->GetPoint()->nNode = *pNd;
        pCursor->GetPoint()->nContent.Assign( pNd->GetTextNode(), 0 );

        bRet = !pCursor->IsSelOvr();
        if( bRet )
            UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
    }
    return bRet;
}

/// search "outline position" before previous outline node at given level
sal_uInt16 SwCursorShell::GetOutlinePos( sal_uInt8 nLevel )
{
    SwPaM* pCursor = getShellCursor( true );
    const SwNodes& rNds = GetDoc()->GetNodes();

    SwNode* pNd = &(pCursor->GetNode());
    sal_uInt16 nPos;
    if( rNds.GetOutLineNds().Seek_Entry( pNd, &nPos ))
        nPos++; // is at correct position; take next for while

    while( nPos-- ) // check the one in front of the current
    {
        pNd = rNds.GetOutLineNds()[ nPos ];

        if( pNd->GetTextNode()->GetAttrOutlineLevel()-1 <= nLevel )
            return nPos;

    }
    return USHRT_MAX; // no more left
}

bool SwCursorShell::MakeOutlineSel( sal_uInt16 nSttPos, sal_uInt16 nEndPos,
                                  bool bWithChildren )
{
    const SwNodes& rNds = GetDoc()->GetNodes();
    const SwOutlineNodes& rOutlNds = rNds.GetOutLineNds();
    if( rOutlNds.empty() )
        return false;

    SET_CURR_SHELL( this );
    SwCallLink aLk( *this ); // watch Cursor-Moves

    if( nSttPos > nEndPos ) // parameters switched?
    {
        OSL_ENSURE( false, "Start > End for array access" );
        sal_uInt16 nTmp = nSttPos;
        nSttPos = nEndPos;
        nEndPos = nTmp;
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

    KillPams();

    SwCursorSaveState aSaveState( *m_pCurrentCursor );

    // set end to the end of the previous content node
    m_pCurrentCursor->GetPoint()->nNode = *pSttNd;
    m_pCurrentCursor->GetPoint()->nContent.Assign( pSttNd->GetContentNode(), 0 );
    m_pCurrentCursor->SetMark();
    m_pCurrentCursor->GetPoint()->nNode = *pEndNd;
    m_pCurrentCursor->Move( fnMoveBackward, fnGoNode ); // end of predecessor

    // and everything is already selected
    bool bRet = !m_pCurrentCursor->IsSelOvr();
    if( bRet )
        UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
    return bRet;
}

/// jump to reference marker
bool SwCursorShell::GotoRefMark( const OUString& rRefMark, sal_uInt16 nSubType,
                                    sal_uInt16 nSeqNo )
{
    SET_CURR_SHELL( this );
    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursorSaveState aSaveState( *m_pCurrentCursor );

    sal_Int32 nPos = -1;
    SwTextNode* pTextNd = SwGetRefFieldType::FindAnchor( GetDoc(), rRefMark,
                                                    nSubType, nSeqNo, &nPos );
    if( pTextNd && pTextNd->GetNodes().IsDocNodes() )
    {
        m_pCurrentCursor->GetPoint()->nNode = *pTextNd;
        m_pCurrentCursor->GetPoint()->nContent.Assign( pTextNd, nPos );

        if( !m_pCurrentCursor->IsSelOvr() )
        {
            UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
            return true;
        }
    }
    return false;
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
    SET_CURR_SHELL( this );
    bool bRet = false;

    if( !IsTableMode() )
    {
        Point aPt( rPt );
        SwPosition aPos( *m_pCurrentCursor->GetPoint() );

        SwTextNode* pTextNd;
        SwContentFrame *pFrame(nullptr);
        SwTextAttr* pTextAttr;
        SwCursorMoveState aTmpState;
        aTmpState.m_bFieldInfo = true;
        aTmpState.m_bExactOnly = !( SwContentAtPos::SW_OUTLINE & rContentAtPos.eContentAtPos );
        aTmpState.m_bContentCheck = (SwContentAtPos::SW_CONTENT_CHECK & rContentAtPos.eContentAtPos) != 0;
        aTmpState.m_bSetInReadOnly = IsReadOnlyAvailable();

        SwSpecialPos aSpecialPos;
        aTmpState.m_pSpecialPos = ( SwContentAtPos::SW_SMARTTAG & rContentAtPos.eContentAtPos ) ?
                                &aSpecialPos : nullptr;

        const bool bCursorFoundExact = GetLayout()->GetCursorOfst( &aPos, aPt, &aTmpState );
        pTextNd = aPos.nNode.GetNode().GetTextNode();

        const SwNodes& rNds = GetDoc()->GetNodes();
        if( pTextNd
            && SwContentAtPos::SW_OUTLINE & rContentAtPos.eContentAtPos
            && !rNds.GetOutLineNds().empty() )
        {
            const SwTextNode* pONd = pTextNd->FindOutlineNodeOfLevel( MAXLEVEL-1);
            if( pONd )
            {
                rContentAtPos.eContentAtPos = SwContentAtPos::SW_OUTLINE;
                rContentAtPos.sStr = pONd->GetExpandText( 0, -1, true, true );
                bRet = true;
            }
        }
        else if ( SwContentAtPos::SW_CONTENT_CHECK & rContentAtPos.eContentAtPos
                  && bCursorFoundExact )
        {
            bRet = true;
        }
        else if( pTextNd
                 && SwContentAtPos::SW_NUMLABEL & rContentAtPos.eContentAtPos)
        {
            bRet = aTmpState.m_bInNumPortion;
            rContentAtPos.aFnd.pNode = pTextNd;

            Size aSizeLogic(aTmpState.m_nInNumPostionOffset, 0);
            Size aSizePixel = GetWin()->LogicToPixel(aSizeLogic);
            rContentAtPos.nDist = aSizePixel.Width();
        }
        else if( bCursorFoundExact && pTextNd )
        {
            if( !aTmpState.m_bPosCorr )
            {
                if ( !bRet
                     && SwContentAtPos::SW_SMARTTAG & rContentAtPos.eContentAtPos
                     && !aTmpState.m_bFootnoteNoInfo )
                {
                    const SwWrongList* pSmartTagList = pTextNd->GetSmartTags();
                    sal_Int32 nCurrent = aPos.nContent.GetIndex();
                    const sal_Int32 nBegin = nCurrent;
                    sal_Int32 nLen = 1;

                    if ( pSmartTagList && pSmartTagList->InWrongWord( nCurrent, nLen ) && !pTextNd->IsSymbol(nBegin) )
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
                            if( m_pCurrentCursor->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION | nsSwCursorSelOverFlags::SELOVER_TOGGLE) )
                                bRet = false;
                            else
                                UpdateCursor();
                        }
                        if( bRet )
                        {
                            rContentAtPos.eContentAtPos = SwContentAtPos::SW_SMARTTAG;

                            if( pFieldRect && nullptr != ( pFrame = pTextNd->getLayoutFrame( GetLayout(), &aPt ) ) )
                                pFrame->GetCharRect( *pFieldRect, aPos, &aTmpState );
                        }
                    }
                }

                if ( !bRet
                     && ( SwContentAtPos::SW_FIELD | SwContentAtPos::SW_CLICKFIELD ) & rContentAtPos.eContentAtPos
                     && !aTmpState.m_bFootnoteNoInfo )
                {
                    pTextAttr = pTextNd->GetFieldTextAttrAt( aPos.nContent.GetIndex() );
                    const SwField* pField = pTextAttr != nullptr
                                          ? pTextAttr->GetFormatField().GetField()
                                          : nullptr;
                    if ( SwContentAtPos::SW_CLICKFIELD & rContentAtPos.eContentAtPos
                         && pField && !pField->HasClickHdl() )
                    {
                        pField = nullptr;
                    }

                    if ( pField )
                    {
                        if( pFieldRect && nullptr != ( pFrame = pTextNd->getLayoutFrame( GetLayout(), &aPt ) ) )
                            pFrame->GetCharRect( *pFieldRect, aPos, &aTmpState );

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
                                if( SwContentAtPos::SW_FIELD & rContentAtPos.eContentAtPos
                                    || RES_JUMPEDITFLD == pField->Which() )
                                    pField = nullptr;
                            }
                            else
                                UpdateCursor();
                        }
                        else if( RES_TABLEFLD == pField->Which() &&
                            static_cast<const SwTableField*>(pField)->IsIntrnlName() )
                        {
                            // create from internal (for CORE) the external
                            // (for UI) formula
                            const SwTableNode* pTableNd = pTextNd->FindTableNode();
                            if( pTableNd )        // steht in einer Tabelle
                                const_cast<SwTableField*>(static_cast<const SwTableField*>(pField))->PtrToBoxNm( &pTableNd->GetTable() );
                        }
                    }

                    if( pField )
                    {
                        rContentAtPos.aFnd.pField = pField;
                        rContentAtPos.pFndTextAttr = pTextAttr;
                        rContentAtPos.eContentAtPos = SwContentAtPos::SW_FIELD;
                        bRet = true;
                    }
                }

                if( !bRet && SwContentAtPos::SW_FORMCTRL & rContentAtPos.eContentAtPos )
                {
                    IDocumentMarkAccess* pMarksAccess = GetDoc()->getIDocumentMarkAccess( );
                    sw::mark::IFieldmark* pFieldBookmark = pMarksAccess->getFieldmarkFor( aPos );
                    if( bCursorFoundExact && pTextNd && pFieldBookmark) {
                        rContentAtPos.eContentAtPos = SwContentAtPos::SW_FORMCTRL;
                        rContentAtPos.aFnd.pFieldmark = pFieldBookmark;
                        bRet=true;
                    }
                }

                if( !bRet && SwContentAtPos::SW_FTN & rContentAtPos.eContentAtPos )
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
                            rContentAtPos.eContentAtPos = SwContentAtPos::SW_FTN;
                    }
                    else if ( nullptr != ( pTextAttr = pTextNd->GetTextAttrForCharAt(
                        aPos.nContent.GetIndex(), RES_TXTATR_FTN )) )
                    {
                        bRet = true;
                        if( bSetCursor )
                        {
                            SwCallLink aLk( *this ); // watch Cursor-Moves
                            SwCursorSaveState aSaveState( *m_pCurrentCursor );
                            m_pCurrentCursor->GetPoint()->nNode = *static_cast<SwTextFootnote*>(pTextAttr)->GetStartNode();
                            SwContentNode* pCNd = GetDoc()->GetNodes().GoNextSection(
                                &m_pCurrentCursor->GetPoint()->nNode,
                                true, !IsReadOnlyAvailable() );

                            if( pCNd )
                            {
                                m_pCurrentCursor->GetPoint()->nContent.Assign( pCNd, 0 );
                                if( m_pCurrentCursor->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                                    nsSwCursorSelOverFlags::SELOVER_TOGGLE ))
                                    bRet = false;
                                else
                                    UpdateCursor();
                            }
                            else
                                bRet = false;
                        }

                        if( bRet )
                        {
                            rContentAtPos.eContentAtPos = SwContentAtPos::SW_FTN;
                            rContentAtPos.pFndTextAttr = pTextAttr;
                            rContentAtPos.aFnd.pAttr = &pTextAttr->GetAttr();

                            if( pFieldRect && nullptr != ( pFrame = pTextNd->getLayoutFrame( GetLayout(), &aPt ) ) )
                                pFrame->GetCharRect( *pFieldRect, aPos, &aTmpState );
                        }
                    }
                }

                if( !bRet
                    && ( SwContentAtPos::SW_TOXMARK | SwContentAtPos::SW_REFMARK ) & rContentAtPos.eContentAtPos
                    && !aTmpState.m_bFootnoteNoInfo )
                {
                    pTextAttr = nullptr;
                    if( SwContentAtPos::SW_TOXMARK & rContentAtPos.eContentAtPos )
                    {
                        ::std::vector<SwTextAttr *> const marks(
                            pTextNd->GetTextAttrsAt(
                               aPos.nContent.GetIndex(), RES_TXTATR_TOXMARK));
                        if (marks.size())
                        {   // hmm... can only return 1 here
                            pTextAttr = *marks.begin();
                        }
                    }

                    if( !pTextAttr &&
                        SwContentAtPos::SW_REFMARK & rContentAtPos.eContentAtPos )
                    {
                        ::std::vector<SwTextAttr *> const marks(
                            pTextNd->GetTextAttrsAt(
                               aPos.nContent.GetIndex(), RES_TXTATR_REFMARK));
                        if (marks.size())
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
                            if( m_pCurrentCursor->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION | nsSwCursorSelOverFlags::SELOVER_TOGGLE ) )
                                bRet = false;
                            else
                                UpdateCursor();
                        }

                        if( bRet )
                        {
                            const sal_Int32* pEnd = pTextAttr->GetEnd();
                            if( pEnd )
                                rContentAtPos.sStr =
                                    pTextNd->GetExpandText( pTextAttr->GetStart(), *pEnd - pTextAttr->GetStart() );
                            else if( RES_TXTATR_TOXMARK == pTextAttr->Which())
                                rContentAtPos.sStr =
                                    pTextAttr->GetTOXMark().GetAlternativeText();

                            rContentAtPos.eContentAtPos =
                                RES_TXTATR_TOXMARK == pTextAttr->Which()
                                ? SwContentAtPos::SW_TOXMARK
                                : SwContentAtPos::SW_REFMARK;
                            rContentAtPos.pFndTextAttr = pTextAttr;
                            rContentAtPos.aFnd.pAttr = &pTextAttr->GetAttr();

                            if( pFieldRect && nullptr != ( pFrame = pTextNd->getLayoutFrame( GetLayout(), &aPt ) ) )
                                pFrame->GetCharRect( *pFieldRect, aPos, &aTmpState );
                        }
                    }
                }

                if ( !bRet
                     && SwContentAtPos::SW_INETATTR & rContentAtPos.eContentAtPos
                     && !aTmpState.m_bFootnoteNoInfo )
                {
                    pTextAttr = pTextNd->GetTextAttrAt(
                            aPos.nContent.GetIndex(), RES_TXTATR_INETFMT);
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
                            if( m_pCurrentCursor->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                                nsSwCursorSelOverFlags::SELOVER_TOGGLE) )
                                bRet = false;
                            else
                                UpdateCursor();
                        }
                        if( bRet )
                        {
                            rContentAtPos.sStr = pTextNd->GetExpandText(
                                pTextAttr->GetStart(),
                                *pTextAttr->GetEnd() - pTextAttr->GetStart() );

                            rContentAtPos.aFnd.pAttr = &pTextAttr->GetAttr();
                            rContentAtPos.eContentAtPos = SwContentAtPos::SW_INETATTR;
                            rContentAtPos.pFndTextAttr = pTextAttr;

                            if( pFieldRect && nullptr != ( pFrame = pTextNd->getLayoutFrame( GetLayout(), &aPt ) ) )
                                pFrame->GetCharRect( *pFieldRect, aPos, &aTmpState );
                        }
                    }
                }

                if( !bRet && SwContentAtPos::SW_REDLINE & rContentAtPos.eContentAtPos )
                {
                    const SwRangeRedline* pRedl = GetDoc()->getIDocumentRedlineAccess().GetRedline(aPos, nullptr);
                    if( pRedl )
                    {
                        rContentAtPos.aFnd.pRedl = pRedl;
                        rContentAtPos.eContentAtPos = SwContentAtPos::SW_REDLINE;
                        rContentAtPos.pFndTextAttr = nullptr;
                        bRet = true;

                        if( pFieldRect && nullptr != ( pFrame = pTextNd->getLayoutFrame( GetLayout(), &aPt ) ) )
                            pFrame->GetCharRect( *pFieldRect, aPos, &aTmpState );
                    }
                }
            }

            if( !bRet
                 && ( SwContentAtPos::SW_TABLEBOXFML & rContentAtPos.eContentAtPos
#ifdef DBG_UTIL
                      || SwContentAtPos::SW_TABLEBOXVALUE & rContentAtPos.eContentAtPos
#endif
                ) )
            {
                const SwTableNode* pTableNd;
                const SwTableBox* pBox;
                const SwStartNode* pSttNd = pTextNd->FindTableBoxStartNode();
                const SfxPoolItem* pItem;
                if( pSttNd && nullptr != ( pTableNd = pTextNd->FindTableNode()) &&
                    nullptr != ( pBox = pTableNd->GetTable().GetTableBox(
                    pSttNd->GetIndex() )) &&
#ifdef DBG_UTIL
                    ( SfxItemState::SET == pBox->GetFrameFormat()->GetItemState(
                    RES_BOXATR_FORMULA, false, &pItem ) ||
                    SfxItemState::SET == pBox->GetFrameFormat()->GetItemState(
                    RES_BOXATR_VALUE, false, &pItem ))
#else
                    SfxItemState::SET == pBox->GetFrameFormat()->GetItemState(
                    RES_BOXATR_FORMULA, false, &pItem )
#endif
                    )
                {
                    SwFrame* pF = pTextNd->getLayoutFrame( GetLayout(), &aPt );
                    if( pF )
                    {
                        // then the CellFrame
                        pFrame = static_cast<SwContentFrame*>(pF);
                        while( pF && !pF->IsCellFrame() )
                            pF = pF->GetUpper();
                    }

                    if( aTmpState.m_bPosCorr )
                    {
                        if( pF && !pF->Frame().IsInside( aPt ))
                            pF = nullptr;
                    }
                    else if( !pF )
                        pF = pFrame;

                    if( pF ) // only then it is valid
                    {
                        // create from internal (for CORE) the external
                        // (for UI) formula
                        rContentAtPos.eContentAtPos = SwContentAtPos::SW_TABLEBOXFML;
#ifdef DBG_UTIL
                        if( RES_BOXATR_VALUE == pItem->Which() )
                            rContentAtPos.eContentAtPos = SwContentAtPos::SW_TABLEBOXVALUE;
                        else
#endif
                            const_cast<SwTableBoxFormula*>(static_cast<const SwTableBoxFormula*>(pItem))->PtrToBoxNm( &pTableNd->GetTable() );

                        bRet = true;
                        if( bSetCursor )
                        {
                            SwCallLink aLk( *this ); // watch Cursor-Moves
                            SwCursorSaveState aSaveState( *m_pCurrentCursor );
                            *m_pCurrentCursor->GetPoint() = aPos;
                            if( m_pCurrentCursor->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                                nsSwCursorSelOverFlags::SELOVER_TOGGLE) )
                                bRet = false;
                            else
                                UpdateCursor();
                        }

                        if( bRet )
                        {
                            if( pFieldRect )
                            {
                                *pFieldRect = pF->Prt();
                                *pFieldRect += pF->Frame().Pos();
                            }
                            rContentAtPos.pFndTextAttr = nullptr;
                            rContentAtPos.aFnd.pAttr = pItem;
                        }
                    }
                }
            }

#ifdef DBG_UTIL
            if( !bRet && SwContentAtPos::SW_CURR_ATTRS & rContentAtPos.eContentAtPos )
            {
                const sal_Int32 n = aPos.nContent.GetIndex();
                SfxItemSet aSet( GetDoc()->GetAttrPool(), POOLATTR_BEGIN,
                    POOLATTR_END - 1 );
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
                rContentAtPos.sStr += OUString::number( aPos.nNode.GetIndex());
                rContentAtPos.sStr += ":";
                rContentAtPos.sStr += OUString::number( aPos.nContent.GetIndex());
                rContentAtPos.sStr += ")";
                rContentAtPos.sStr += "\nParagraph Style: ";
                rContentAtPos.sStr += pTextNd->GetFormatColl()->GetName();
                if( pTextNd->GetCondFormatColl() )
                {
                    rContentAtPos.sStr += "\nConditional Style: " + pTextNd->GetCondFormatColl()->GetName();
                }

                if( aSet.Count() )
                {
                    OUString sAttrs;
                    SfxItemIter aIter( aSet );
                    const SfxPoolItem* pItem = aIter.FirstItem();
                    while( true )
                    {
                        if( !IsInvalidItem( pItem ))
                        {
                            OUString aStr;
                            GetDoc()->GetAttrPool().GetPresentation( *pItem,
                                SFX_MAPUNIT_CM, aStr );
                            if (!sAttrs.isEmpty())
                                sAttrs += ", ";
                            sAttrs += aStr;
                        }
                        if( aIter.IsAtEnd() )
                            break;
                        pItem = aIter.NextItem();
                    }
                    if (!sAttrs.isEmpty())
                    {
                        if( !rContentAtPos.sStr.isEmpty() )
                            rContentAtPos.sStr += "\n";
                        rContentAtPos.sStr += "Attr: " + sAttrs;
                    }
                }
                bRet = true;
                rContentAtPos.eContentAtPos = SwContentAtPos::SW_CURR_ATTRS;
            }
#endif
        }
    }

    if( !bRet )
    {
        rContentAtPos.eContentAtPos = SwContentAtPos::SW_NOTHING;
        rContentAtPos.aFnd.pField = nullptr;
    }
    return bRet;
}

// #i90516#
const SwPostItField* SwCursorShell::GetPostItFieldAtCursor() const
{
    const SwPostItField* pPostItField = nullptr;

    if ( !IsTableMode() )
    {
        const SwPosition* pCursorPos = _GetCursor()->GetPoint();
        const SwTextNode* pTextNd = pCursorPos->nNode.GetNode().GetTextNode();
        if ( pTextNd )
        {
            SwTextAttr* pTextAttr = pTextNd->GetFieldTextAttrAt( pCursorPos->nContent.GetIndex() );
            const SwField* pField = pTextAttr != nullptr ? pTextAttr->GetFormatField().GetField() : nullptr;
            if ( pField && pField->Which()== RES_POSTITFLD )
            {
                pPostItField = static_cast<const SwPostItField*>(pField);
            }
        }
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
        case SW_FIELD:
        case SW_CLICKFIELD:
            pNd = static_txtattr_cast<SwTextField const*>(pFndTextAttr)->GetpTextNode();
            break;

        case SW_FTN:
            pNd = &static_cast<const SwTextFootnote*>(pFndTextAttr)->GetTextNode();
            break;

        case SW_INETATTR:
            pNd = static_txtattr_cast<SwTextINetFormat const*>(pFndTextAttr)->GetpTextNode();
            break;

        default:
            break;
        }
    }

    const SwContentFrame* pFrame;
    return pNd && ( pNd->IsInProtectSect() ||
                    ( nullptr != ( pFrame = pNd->getLayoutFrame( pNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), nullptr, nullptr, false)) &&
                        pFrame->IsProtected() ));
}

bool SwContentAtPos::IsInRTLText()const
{
    bool bRet = false;
    const SwTextNode* pNd = nullptr;
    if (pFndTextAttr && (eContentAtPos == SW_FTN))
    {
        const SwTextFootnote* pTextFootnote = static_cast<const SwTextFootnote*>(pFndTextAttr);
        if(pTextFootnote->GetStartNode())
        {
            SwStartNode* pSttNd = pTextFootnote->GetStartNode()->GetNode().GetStartNode();
            SwPaM aTemp( *pSttNd );
            aTemp.Move(fnMoveForward, fnGoNode);
            SwContentNode* pContentNode = aTemp.GetContentNode();
            if(pContentNode && pContentNode->IsTextNode())
                pNd = pContentNode->GetTextNode();
        }
    }
    if(pNd)
    {
        SwIterator<SwTextFrame,SwTextNode> aIter(*pNd);
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
    }
    return bRet;
}

bool SwCursorShell::SelectText( const sal_Int32 nStart,
                                 const sal_Int32 nEnd )
{
    SET_CURR_SHELL( this );
    bool bRet = false;

    SwCallLink aLk( *this );
    SwCursorSaveState aSaveState( *m_pCurrentCursor );

    SwPosition& rPos = *m_pCurrentCursor->GetPoint();
    m_pCurrentCursor->DeleteMark();
    rPos.nContent = nStart;
    m_pCurrentCursor->SetMark();
    rPos.nContent = nEnd;

    if( !m_pCurrentCursor->IsSelOvr() )
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
    SET_CURR_SHELL( this );
    bool bRet = false;

    if( !IsTableMode() )
    {
        if( !pTextAttr )
        {
            SwPosition& rPos = *m_pCurrentCursor->GetPoint();
            SwTextNode* pTextNd = rPos.nNode.GetNode().GetTextNode();
            pTextAttr = (pTextNd)
                ? pTextNd->GetTextAttrAt(rPos.nContent.GetIndex(),
                        static_cast<RES_TXTATR>(nWhich),
                        (bExpand) ? SwTextNode::EXPAND : SwTextNode::DEFAULT)
                : nullptr;
        }

        if( pTextAttr )
        {
            const sal_Int32* pEnd = pTextAttr->End();
            bRet = SelectText( pTextAttr->GetStart(), ( pEnd ? *pEnd : pTextAttr->GetStart() + 1 ) );
        }
    }
    return bRet;
}

bool SwCursorShell::GotoINetAttr( const SwTextINetFormat& rAttr )
{
    bool bRet = false;
    if( rAttr.GetpTextNode() )
    {
        SwCursor* pCursor = getShellCursor( true );

        SET_CURR_SHELL( this );
        SwCallLink aLk( *this ); // watch Cursor-Moves
        SwCursorSaveState aSaveState( *pCursor );

        pCursor->GetPoint()->nNode = *rAttr.GetpTextNode();
        pCursor->GetPoint()->nContent.Assign( const_cast<SwTextNode*>(rAttr.GetpTextNode()),
                                            rAttr.GetStart() );
        bRet = !pCursor->IsSelOvr();
        if( bRet )
            UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
    }
    return bRet;
}

const SwFormatINetFormat* SwCursorShell::FindINetAttr( const OUString& rName ) const
{
    return mpDoc->FindINetAttr( rName );
}

bool SwCursorShell::GetShadowCursorPos( const Point& rPt, SwFillMode eFillMode,
                                SwRect& rRect, sal_Int16& rOrient )
{

    SET_CURR_SHELL( this );
    bool bRet = false;

    if (!IsTableMode() && !HasSelection()
        && GetDoc()->GetIDocumentUndoRedo().DoesUndo())
    {
        Point aPt( rPt );
        SwPosition aPos( *m_pCurrentCursor->GetPoint() );

        SwFillCursorPos aFPos( eFillMode );
        SwCursorMoveState aTmpState( &aFPos );

        if( GetLayout()->GetCursorOfst( &aPos, aPt, &aTmpState ) &&
            !aPos.nNode.GetNode().IsProtect())
        {
            // start position in protected section?
            rRect = aFPos.aCursor;
            rOrient = aFPos.eOrient;
            bRet = true;
        }
    }
    return bRet;
}

bool SwCursorShell::SetShadowCursorPos( const Point& rPt, SwFillMode eFillMode )
{
    SET_CURR_SHELL( this );
    bool bRet = false;

    if (!IsTableMode() && !HasSelection()
        && GetDoc()->GetIDocumentUndoRedo().DoesUndo())
    {
        Point aPt( rPt );
        SwPosition aPos( *m_pCurrentCursor->GetPoint() );

        SwFillCursorPos aFPos( eFillMode );
        SwCursorMoveState aTmpState( &aFPos );

        if( GetLayout()->GetCursorOfst( &aPos, aPt, &aTmpState ) )
        {
            SwCallLink aLk( *this ); // watch Cursor-Moves
            StartAction();

            SwContentNode* pCNd = aPos.nNode.GetNode().GetContentNode();
            SwUndoId nUndoId = UNDO_INS_FROM_SHADOWCRSR;
            // If only the paragraph attributes "Adjust" or "LRSpace" are set,
            // then the following should not delete those again.
            if( 0 == aFPos.nParaCnt + aFPos.nColumnCnt &&
                ( FILL_INDENT == aFPos.eMode ||
                  ( text::HoriOrientation::NONE != aFPos.eOrient &&
                    0 == aFPos.nTabCnt + aFPos.nSpaceCnt )) &&
                pCNd && pCNd->Len() )
                nUndoId = UNDO_EMPTY;

            GetDoc()->GetIDocumentUndoRedo().StartUndo( nUndoId, nullptr );

            SwTextFormatColl* pNextFormat = nullptr;
            SwTextNode* pTNd = pCNd ? pCNd->GetTextNode() : nullptr;
            if( pTNd )
                pNextFormat = &pTNd->GetTextColl()->GetNextTextFormatColl();

            const SwSectionNode* pSectNd = pCNd ? pCNd->FindSectionNode() : nullptr;
            if( pSectNd && aFPos.nParaCnt )
            {
                SwNodeIndex aEnd( aPos.nNode, 1 );
                while( aEnd.GetNode().IsEndNode() &&
                        &aEnd.GetNode() !=
                        pSectNd->EndOfSectionNode() )
                    ++aEnd;

                if( aEnd.GetNode().IsEndNode() &&
                    pCNd->Len() == aPos.nContent.GetIndex() )
                    aPos.nNode = *pSectNd->EndOfSectionNode();
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
                            SvxFormatBreakItem( SVX_BREAK_COLUMN_BEFORE, RES_BREAK ) );
                }
            }

            *m_pCurrentCursor->GetPoint() = aPos;
            switch( aFPos.eMode )
            {
            case FILL_INDENT:
                if( nullptr != (pCNd = aPos.nNode.GetNode().GetContentNode() ))
                {
                    SfxItemSet aSet( GetDoc()->GetAttrPool(),
                                    RES_LR_SPACE, RES_LR_SPACE,
                                    RES_PARATR_ADJUST, RES_PARATR_ADJUST,
                                    0 );
                    SvxLRSpaceItem aLR( static_cast<const SvxLRSpaceItem&>(
                                        pCNd->GetAttr( RES_LR_SPACE ) ) );
                    aLR.SetTextLeft( aFPos.nTabCnt );
                    aLR.SetTextFirstLineOfst( 0 );
                    aSet.Put( aLR );

                    const SvxAdjustItem& rAdj = static_cast<const SvxAdjustItem&>(pCNd->
                                        GetAttr( RES_PARATR_ADJUST ));
                    if( SVX_ADJUST_LEFT != rAdj.GetAdjust() )
                        aSet.Put( SvxAdjustItem( SVX_ADJUST_LEFT, RES_PARATR_ADJUST ) );

                    GetDoc()->getIDocumentContentOperations().InsertItemSet( *m_pCurrentCursor, aSet );
                }
                else {
                    OSL_ENSURE( false, "No ContentNode" );
                }
                break;

            case FILL_TAB:
            case FILL_SPACE:
                {
                    OUStringBuffer sInsert;
                    if (aFPos.nTabCnt)
                        comphelper::string::padToLength(sInsert, aFPos.nTabCnt, '\t');
                    if (aFPos.nSpaceCnt)
                        comphelper::string::padToLength(sInsert, sInsert.getLength() + aFPos.nSpaceCnt, ' ');
                    if (!sInsert.isEmpty())
                        GetDoc()->getIDocumentContentOperations().InsertString( *m_pCurrentCursor, sInsert.makeStringAndClear());
                }
                SAL_FALLTHROUGH; // still need to set orientation
            case FILL_MARGIN:
                if( text::HoriOrientation::NONE != aFPos.eOrient )
                {
                    SvxAdjustItem aAdj( SVX_ADJUST_LEFT, RES_PARATR_ADJUST );
                    switch( aFPos.eOrient )
                    {
                    case text::HoriOrientation::CENTER:
                        aAdj.SetAdjust( SVX_ADJUST_CENTER );
                        break;
                    case text::HoriOrientation::RIGHT:
                        aAdj.SetAdjust( SVX_ADJUST_RIGHT );
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

            bRet = true;
        }
    }
    return bRet;
}

const SwRangeRedline* SwCursorShell::SelNextRedline()
{
    const SwRangeRedline* pFnd = nullptr;
    if( !IsTableMode() )
    {
        SET_CURR_SHELL( this );
        SwCallLink aLk( *this ); // watch Cursor-Moves
        SwCursorSaveState aSaveState( *m_pCurrentCursor );

        pFnd = GetDoc()->getIDocumentRedlineAccess().SelNextRedline( *m_pCurrentCursor );
        if( pFnd && !m_pCurrentCursor->IsInProtectTable() && !m_pCurrentCursor->IsSelOvr() )
            UpdateCursor( SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
        else
            pFnd = nullptr;
    }
    return pFnd;
}

const SwRangeRedline* SwCursorShell::SelPrevRedline()
{
    const SwRangeRedline* pFnd = nullptr;
    if( !IsTableMode() )
    {
        SET_CURR_SHELL( this );
        SwCallLink aLk( *this ); // watch Cursor-Moves
        SwCursorSaveState aSaveState( *m_pCurrentCursor );

        pFnd = GetDoc()->getIDocumentRedlineAccess().SelPrevRedline( *m_pCurrentCursor );
        if( pFnd && !m_pCurrentCursor->IsInProtectTable() && !m_pCurrentCursor->IsSelOvr() )
            UpdateCursor( SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
        else
            pFnd = nullptr;
    }
    return pFnd;
}

const SwRangeRedline* SwCursorShell::_GotoRedline( sal_uInt16 nArrPos, bool bSelect )
{
    const SwRangeRedline* pFnd = nullptr;
    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursorSaveState aSaveState( *m_pCurrentCursor );

    pFnd = GetDoc()->getIDocumentRedlineAccess().GetRedlineTable()[ nArrPos ];
    if( pFnd )
    {
        *m_pCurrentCursor->GetPoint() = *pFnd->Start();

        SwContentNode* pCNd;
        SwNodeIndex* pIdx = &m_pCurrentCursor->GetPoint()->nNode;
        if( !pIdx->GetNode().IsContentNode() &&
            nullptr != ( pCNd = GetDoc()->GetNodes().GoNextSection( pIdx,
                                    true, IsReadOnlyAvailable() )) )
        {
            if( *pIdx <= pFnd->End()->nNode )
                m_pCurrentCursor->GetPoint()->nContent.Assign( pCNd, 0 );
            else
                pFnd = nullptr;
        }

        if( pFnd && bSelect )
        {
            m_pCurrentCursor->SetMark();
            if( nsRedlineType_t::REDLINE_FMTCOLL == pFnd->GetType() )
            {
                pCNd = pIdx->GetNode().GetContentNode();
                m_pCurrentCursor->GetPoint()->nContent.Assign( pCNd, pCNd->Len() );
                m_pCurrentCursor->GetMark()->nContent.Assign( pCNd, 0 );
            }
            else
                *m_pCurrentCursor->GetPoint() = *pFnd->End();

            pIdx = &m_pCurrentCursor->GetPoint()->nNode;
            if( !pIdx->GetNode().IsContentNode() &&
                nullptr != ( pCNd = SwNodes::GoPrevSection( pIdx,
                                            true, IsReadOnlyAvailable() )) )
            {
                if( *pIdx >= m_pCurrentCursor->GetMark()->nNode )
                    m_pCurrentCursor->GetPoint()->nContent.Assign( pCNd, pCNd->Len() );
                else
                    pFnd = nullptr;
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
    }
    return pFnd;
}

const SwRangeRedline* SwCursorShell::GotoRedline( sal_uInt16 nArrPos, bool bSelect )
{
    const SwRangeRedline* pFnd = nullptr;
    if( !IsTableMode() )
    {
        SET_CURR_SHELL( this );

        const SwRedlineTable& rTable = GetDoc()->getIDocumentRedlineAccess().GetRedlineTable();
        const SwRangeRedline* pTmp = rTable[ nArrPos ];
        sal_uInt16 nSeqNo = pTmp->GetSeqNo();
        if( nSeqNo && bSelect )
        {
            bool bCheck = false;
            int nLoopCnt = 2;
            sal_uInt16 nArrSavPos = nArrPos;

            do {
                pTmp = _GotoRedline( nArrPos, true );

                if( !pFnd )
                    pFnd = pTmp;

                if( pTmp && bCheck )
                {
                    // Check for overlaps. These can happen when FormatColl-
                    // Redlines were stretched over a whole paragraph
                    SwPaM* pCur = m_pCurrentCursor;
                    SwPaM* pNextPam = pCur->GetNext();
                    SwPosition* pCStt = pCur->Start(), *pCEnd = pCur->End();
                    while( pCur != pNextPam )
                    {
                        const SwPosition *pNStt = pNextPam->Start(),
                                         *pNEnd = pNextPam->End();

                        bool bDel = true;
                        switch( ::ComparePosition( *pCStt, *pCEnd,
                                                   *pNStt, *pNEnd ))
                        {
                        case POS_INSIDE:         // Pos1 is completely in Pos2
                            if( !pCur->HasMark() )
                            {
                                pCur->SetMark();
                                *pCur->GetMark() = *pNStt;
                            }
                            else
                                *pCStt = *pNStt;
                            *pCEnd = *pNEnd;
                            break;

                        case POS_OUTSIDE:        // Pos2 is completely in Pos1
                        case POS_EQUAL:          // Pos1 has same size as Pos2
                            break;

                        case POS_OVERLAP_BEFORE: // Pos1 overlaps Pos2 at beginning
                            if( !pCur->HasMark() )
                                pCur->SetMark();
                            *pCEnd = *pNEnd;
                            break;
                        case POS_OVERLAP_BEHIND: // Pos1 overlaps Pos2 at end
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
                            SwPaM* pPrevPam = static_cast<SwPaM*>(pNextPam->GetPrev());
                            delete pNextPam;
                            pNextPam = pPrevPam;
                        }
                        pNextPam = pNextPam->GetNext();
                    }
                }

                sal_uInt16 nFndPos = 2 == nLoopCnt
                                    ? rTable.FindNextOfSeqNo( nArrPos )
                                    : rTable.FindPrevOfSeqNo( nArrPos );
                if( USHRT_MAX != nFndPos ||
                    ( 0 != ( --nLoopCnt ) && USHRT_MAX != (
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
        }
        else
            pFnd = _GotoRedline( nArrPos, bSelect );
    }
    return pFnd;
}

bool SwCursorShell::SelectNxtPrvHyperlink( bool bNext )
{
    SwNodes& rNds = GetDoc()->GetNodes();
    const SwNode* pBodyEndNd = &rNds.GetEndOfContent();
    const SwNode* pBodySttNd = pBodyEndNd->StartOfSectionNode();
    sal_uLong nBodySttNdIdx = pBodySttNd->GetIndex();
    Point aPt;

    _SetGetExpField aCmpPos( SwPosition( bNext ? *pBodyEndNd : *pBodySttNd ) );
    _SetGetExpField aCurPos( bNext ? *m_pCurrentCursor->End() : *m_pCurrentCursor->Start() );
    if( aCurPos.GetNode() < nBodySttNdIdx )
    {
        const SwContentNode* pCNd = aCurPos.GetNodeFromContent()->GetContentNode();
        SwContentFrame* pFrame;
        if( pCNd && nullptr != ( pFrame = pCNd->getLayoutFrame( GetLayout(), &aPt )) )
            aCurPos.SetBodyPos( *pFrame );
    }

    // check first all the hyperlink fields
    {
        const SwTextNode* pTextNd;
        const SwCharFormats* pFormats = GetDoc()->GetCharFormats();
        for( SwCharFormats::size_type n = pFormats->size(); 1 < n; )
        {
            SwIterator<SwTextINetFormat,SwCharFormat> aIter(*(*pFormats)[--n]);

            for( SwTextINetFormat* pFnd = aIter.First(); pFnd; pFnd = aIter.Next() )
                if( nullptr != ( pTextNd = pFnd->GetpTextNode()) &&
                    pTextNd->GetNodes().IsDocNodes() )
                {
                    SwTextINetFormat& rAttr = *pFnd;
                    SwPosition aTmpPos( *pTextNd );
                    _SetGetExpField aPos( aTmpPos.nNode, rAttr );
                    SwContentFrame* pFrame;
                    if( pTextNd->GetIndex() < nBodySttNdIdx &&
                        nullptr != ( pFrame = pTextNd->getLayoutFrame( GetLayout(), &aPt )) )
                        aPos.SetBodyPos( *pFrame );

                    if( bNext
                        ? ( aPos < aCmpPos && aCurPos < aPos )
                        : ( aCmpPos < aPos && aPos < aCurPos ))
                    {
                        OUString sText( pTextNd->GetExpandText( rAttr.GetStart(),
                                        *rAttr.GetEnd() - rAttr.GetStart() ) );

                        sText = comphelper::string::remove(sText, 0x0a);
                        sText = comphelper::string::strip(sText, ' ');

                        if( !sText.isEmpty() )
                            aCmpPos = aPos;
                    }
                }
        }
    }

    // then check all the Flys with a URL or image map
    {
        const SwFrameFormats* pFormats = GetDoc()->GetSpzFrameFormats();
        for( SwFrameFormats::size_type n = 0, nEnd = pFormats->size(); n < nEnd; ++n )
        {
            SwFlyFrameFormat* pFormat = static_cast<SwFlyFrameFormat*>((*pFormats)[ n ]);
            const SwFormatURL& rURLItem = pFormat->GetURL();
            if( rURLItem.GetMap() || !rURLItem.GetURL().isEmpty() )
            {
                SwFlyFrame* pFly = pFormat->GetFrame( &aPt );
                SwPosition aTmpPos( *pBodySttNd );
                if( pFly &&
                    GetBodyTextNode( *GetDoc(), aTmpPos, *pFly->GetLower() ) )
                {
                    _SetGetExpField aPos( *pFormat, &aTmpPos );

                    if( bNext
                            ? ( aPos < aCmpPos && aCurPos < aPos )
                            : ( aCmpPos < aPos && aPos < aCurPos ))
                        aCmpPos = aPos;
                }
            }
        }
    }

    // found any URL ?
    bool bRet = false;
    const SwTextINetFormat* pFndAttr = aCmpPos.GetINetFormat();
    const SwFlyFrameFormat* pFndFormat = aCmpPos.GetFlyFormat();
    if( pFndAttr || pFndFormat )
    {
        SET_CURR_SHELL( this );
        SwCallLink aLk( *this );

        // found a text attribute ?
        if( pFndAttr )
        {
            SwCursorSaveState aSaveState( *m_pCurrentCursor );

            aCmpPos.GetPosOfContent( *m_pCurrentCursor->GetPoint() );
            m_pCurrentCursor->DeleteMark();
            m_pCurrentCursor->SetMark();
            m_pCurrentCursor->GetPoint()->nContent = *pFndAttr->End();

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
                static_cast<SwFEShell*>(this)->SelectFlyFrame( *pFly, true );
                MakeSelVisible();
                bRet = true;
            }
        }
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
