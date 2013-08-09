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

#include <ftnidx.hxx>
#include <rootfrm.hxx>
#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <pam.hxx>
#include <pagedesc.hxx>
#include <charfmt.hxx>
#include <UndoAttribute.hxx>
#include <hints.hxx>
#include <rolbck.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <ndtxt.hxx>
#include <poolfmt.hxx>
#include <ftninfo.hxx>

/*********************** SwFtnInfo ***************************/

SwEndNoteInfo& SwEndNoteInfo::operator=(const SwEndNoteInfo& rInfo)
{
    if( rInfo.GetFtnTxtColl() )
        rInfo.GetFtnTxtColl()->Add(this);
    else if ( GetRegisteredIn())
        GetRegisteredInNonConst()->Remove(this);

    if ( rInfo.aPageDescDep.GetRegisteredIn() )
        ((SwModify*)rInfo.aPageDescDep.GetRegisteredIn())->Add( &aPageDescDep );
    else if ( aPageDescDep.GetRegisteredIn() )
        ((SwModify*)aPageDescDep.GetRegisteredIn())->Remove( &aPageDescDep );

    if ( rInfo.aCharFmtDep.GetRegisteredIn() )
        ((SwModify*)rInfo.aCharFmtDep.GetRegisteredIn())->Add( &aCharFmtDep );
    else if ( aCharFmtDep.GetRegisteredIn() )
        ((SwModify*)aCharFmtDep.GetRegisteredIn())->Remove( &aCharFmtDep );

    if ( rInfo.aAnchorCharFmtDep.GetRegisteredIn() )
        ((SwModify*)rInfo.aAnchorCharFmtDep.GetRegisteredIn())->Add(
                                                    &aAnchorCharFmtDep );
    else if( aAnchorCharFmtDep.GetRegisteredIn() )
        ((SwModify*)aAnchorCharFmtDep.GetRegisteredIn())->Remove(
                                                    &aAnchorCharFmtDep );

    aFmt = rInfo.aFmt;
    nFtnOffset = rInfo.nFtnOffset;
    m_bEndNote = rInfo.m_bEndNote;
    sPrefix = rInfo.sPrefix;
    sSuffix = rInfo.sSuffix;
    return *this;
}

bool SwEndNoteInfo::operator==( const SwEndNoteInfo& rInfo ) const
{
    return  aPageDescDep.GetRegisteredIn() ==
                                rInfo.aPageDescDep.GetRegisteredIn() &&
            aCharFmtDep.GetRegisteredIn() ==
                                rInfo.aCharFmtDep.GetRegisteredIn() &&
            aAnchorCharFmtDep.GetRegisteredIn() ==
                                rInfo.aAnchorCharFmtDep.GetRegisteredIn() &&
            GetFtnTxtColl() == rInfo.GetFtnTxtColl() &&
            aFmt.GetNumberingType() == rInfo.aFmt.GetNumberingType() &&
            nFtnOffset == rInfo.nFtnOffset &&
            m_bEndNote == rInfo.m_bEndNote &&
            sPrefix == rInfo.sPrefix &&
            sSuffix == rInfo.sSuffix;
}

SwEndNoteInfo::SwEndNoteInfo(const SwEndNoteInfo& rInfo) :
    SwClient( rInfo.GetFtnTxtColl() ),
    aPageDescDep( this, 0 ),
    aCharFmtDep( this, 0 ),
    aAnchorCharFmtDep( this, 0 ),
    sPrefix( rInfo.sPrefix ),
    sSuffix( rInfo.sSuffix ),
    m_bEndNote( true ),
    aFmt( rInfo.aFmt ),
    nFtnOffset( rInfo.nFtnOffset )
{
    if( rInfo.aPageDescDep.GetRegisteredIn() )
        ((SwModify*)rInfo.aPageDescDep.GetRegisteredIn())->Add( &aPageDescDep );

    if( rInfo.aCharFmtDep.GetRegisteredIn() )
        ((SwModify*)rInfo.aCharFmtDep.GetRegisteredIn())->Add( &aCharFmtDep );

    if( rInfo.aAnchorCharFmtDep.GetRegisteredIn() )
        ((SwModify*)rInfo.aAnchorCharFmtDep.GetRegisteredIn())->Add(
                &aAnchorCharFmtDep );
}

SwEndNoteInfo::SwEndNoteInfo(SwTxtFmtColl *pFmt) :
    SwClient(pFmt),
    aPageDescDep( this, 0 ),
    aCharFmtDep( this, 0 ),
    aAnchorCharFmtDep( this, 0 ),
    m_bEndNote( true ),
    nFtnOffset( 0 )
{
    aFmt.SetNumberingType(SVX_NUM_ROMAN_LOWER);
}

SwPageDesc *SwEndNoteInfo::GetPageDesc( SwDoc &rDoc ) const
{
    if ( !aPageDescDep.GetRegisteredIn() )
    {
        SwPageDesc *pDesc = rDoc.GetPageDescFromPool( static_cast<sal_uInt16>(
            m_bEndNote ? RES_POOLPAGE_ENDNOTE   : RES_POOLPAGE_FOOTNOTE ) );
        pDesc->Add( &((SwClient&)aPageDescDep) );
    }

    return (SwPageDesc*)( aPageDescDep.GetRegisteredIn() );
}

bool SwEndNoteInfo::KnowsPageDesc() const
{
    return (aPageDescDep.GetRegisteredIn() != 0);
}

bool SwEndNoteInfo::DependsOn( const SwPageDesc* pDesc ) const
{
    return ( aPageDescDep.GetRegisteredIn() == pDesc );
}

void SwEndNoteInfo::ChgPageDesc( SwPageDesc *pDesc )
{
    pDesc->Add( &((SwClient&)aPageDescDep) );
}

void SwEndNoteInfo::SetFtnTxtColl(SwTxtFmtColl& rFmt)
{
    rFmt.Add(this);
}

SwCharFmt* SwEndNoteInfo::GetCharFmt(SwDoc &rDoc) const
{
    if ( !aCharFmtDep.GetRegisteredIn() )
    {
        SwCharFmt* pFmt = rDoc.GetCharFmtFromPool( static_cast<sal_uInt16>(
            m_bEndNote ? RES_POOLCHR_ENDNOTE : RES_POOLCHR_FOOTNOTE ) );
        pFmt->Add( &((SwClient&)aCharFmtDep) );
    }
    return (SwCharFmt*)aCharFmtDep.GetRegisteredIn();
}

void SwEndNoteInfo::SetCharFmt( SwCharFmt* pChFmt )
{
    OSL_ENSURE(pChFmt, "no CharFmt?");
    pChFmt->Add( &((SwClient&)aCharFmtDep) );
}

SwCharFmt* SwEndNoteInfo::GetAnchorCharFmt(SwDoc &rDoc) const
{
    if( !aAnchorCharFmtDep.GetRegisteredIn() )
    {
        SwCharFmt* pFmt = rDoc.GetCharFmtFromPool( static_cast<sal_uInt16>(
            m_bEndNote ? RES_POOLCHR_ENDNOTE_ANCHOR : RES_POOLCHR_FOOTNOTE_ANCHOR ) );
        pFmt->Add( &((SwClient&)aAnchorCharFmtDep) );
    }
    return (SwCharFmt*)aAnchorCharFmtDep.GetRegisteredIn();
}

void SwEndNoteInfo::SetAnchorCharFmt( SwCharFmt* pChFmt )
{
    OSL_ENSURE(pChFmt, "no CharFmt?");
    pChFmt->Add( &((SwClient&)aAnchorCharFmtDep) );
}

void SwEndNoteInfo::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0 ;

    if( RES_ATTRSET_CHG == nWhich ||
        RES_FMT_CHG == nWhich )
    {
        SwDoc* pDoc;
        if( aCharFmtDep.GetRegisteredIn() )
            pDoc = ((SwCharFmt*)aCharFmtDep.GetRegisteredIn())->GetDoc();
        else
            pDoc = ((SwCharFmt*)aAnchorCharFmtDep.GetRegisteredIn())->GetDoc();
        SwFtnIdxs& rFtnIdxs = pDoc->GetFtnIdxs();
        for( sal_uInt16 nPos = 0; nPos < rFtnIdxs.size(); ++nPos )
        {
            SwTxtFtn *pTxtFtn = rFtnIdxs[ nPos ];
            const SwFmtFtn &rFtn = pTxtFtn->GetFtn();
            if ( rFtn.IsEndNote() == m_bEndNote )
            {
                pTxtFtn->SetNumber( rFtn.GetNumber(), &rFtn.GetNumStr());
            }
        }
    }
    else
        CheckRegistration( pOld, pNew );
}

SwFtnInfo& SwFtnInfo::operator=(const SwFtnInfo& rInfo)
{
    SwEndNoteInfo::operator=(rInfo);
    aQuoVadis =  rInfo.aQuoVadis;
    aErgoSum = rInfo.aErgoSum;
    ePos = rInfo.ePos;
    eNum = rInfo.eNum;
    return *this;
}


bool SwFtnInfo::operator==( const SwFtnInfo& rInfo ) const
{
    return  ePos == rInfo.ePos &&
            eNum == rInfo.eNum &&
            SwEndNoteInfo::operator==(rInfo) &&
            aQuoVadis == rInfo.aQuoVadis &&
            aErgoSum == rInfo.aErgoSum;
}


SwFtnInfo::SwFtnInfo(const SwFtnInfo& rInfo) :
    SwEndNoteInfo( rInfo ),
    aQuoVadis( rInfo.aQuoVadis ),
    aErgoSum( rInfo.aErgoSum ),
    ePos( rInfo.ePos ),
    eNum( rInfo.eNum )
{
    m_bEndNote = false;
}

SwFtnInfo::SwFtnInfo(SwTxtFmtColl *pFmt) :
    SwEndNoteInfo( pFmt ),
    ePos( FTNPOS_PAGE ),
    eNum( FTNNUM_DOC )
{
    aFmt.SetNumberingType(SVX_NUM_ARABIC);
    m_bEndNote = false;
}

/*********************** SwDoc ***************************/

void SwDoc::SetFtnInfo(const SwFtnInfo& rInfo)
{
    SwRootFrm* pTmpRoot = GetCurrentLayout();//swmod 080219
    if( !(GetFtnInfo() == rInfo) )
    {
        const SwFtnInfo &rOld = GetFtnInfo();

        if (GetIDocumentUndoRedo().DoesUndo())
        {
            GetIDocumentUndoRedo().AppendUndo( new SwUndoFootNoteInfo(rOld) );
        }

        bool bFtnPos  = rInfo.ePos != rOld.ePos;
        bool bFtnDesc = rOld.ePos == FTNPOS_CHAPTER &&
                            rInfo.GetPageDesc( *this ) != rOld.GetPageDesc( *this );
        bool bExtra   = rInfo.aQuoVadis != rOld.aQuoVadis ||
                            rInfo.aErgoSum != rOld.aErgoSum ||
                            rInfo.aFmt.GetNumberingType() != rOld.aFmt.GetNumberingType() ||
                            rInfo.GetPrefix() != rOld.GetPrefix() ||
                            rInfo.GetSuffix() != rOld.GetSuffix();
        SwCharFmt *pOldChrFmt = rOld.GetCharFmt( *this ),
                  *pNewChrFmt = rInfo.GetCharFmt( *this );
        bool bFtnChrFmts = pOldChrFmt != pNewChrFmt;

        *mpFtnInfo = rInfo;

        if (pTmpRoot)
        {
            std::set<SwRootFrm*> aAllLayouts = GetAllLayouts();//swmod 080304
            if ( bFtnPos )
                //pTmpRoot->RemoveFtns();
                std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::mem_fun(&SwRootFrm::AllRemoveFtns));//swmod 080305
            else
            {
                //pTmpRoot->UpdateFtnNums();
                std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::mem_fun(&SwRootFrm::UpdateFtnNums));//swmod 080304
                if ( bFtnDesc )
                    //pTmpRoot->CheckFtnPageDescs( FALSE );
                    std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::bind2nd(std::mem_fun(&SwRootFrm::CheckFtnPageDescs), sal_False));//swmod 080304
                if ( bExtra )
                {
                    // For messages regarding ErgoSum etc. we save the extra code and use the
                    // available methods.
                    SwFtnIdxs& rFtnIdxs = GetFtnIdxs();
                    for( sal_uInt16 nPos = 0; nPos < rFtnIdxs.size(); ++nPos )
                    {
                        SwTxtFtn *pTxtFtn = rFtnIdxs[ nPos ];
                        const SwFmtFtn &rFtn = pTxtFtn->GetFtn();
                        if ( !rFtn.IsEndNote() )
                            pTxtFtn->SetNumber( rFtn.GetNumber(), &rFtn.GetNumStr());
                    }
                }
            }
        }   //swmod 080219
        if( FTNNUM_PAGE != rInfo.eNum )
            GetFtnIdxs().UpdateAllFtn();
        else if( bFtnChrFmts )
        {
            SwFmtChg aOld( pOldChrFmt );
            SwFmtChg aNew( pNewChrFmt );
            mpFtnInfo->ModifyNotification( &aOld, &aNew );
        }

        // #i81002# no update during loading
        if ( !IsInReading() )
        {
            UpdateRefFlds(NULL);
        }
        SetModified();
    }
}

void SwDoc::SetEndNoteInfo(const SwEndNoteInfo& rInfo)
{
    SwRootFrm* pTmpRoot = GetCurrentLayout();//swmod 080219
    if( !(GetEndNoteInfo() == rInfo) )
    {
        if(GetIDocumentUndoRedo().DoesUndo())
        {
            SwUndo *const pUndo( new SwUndoEndNoteInfo( GetEndNoteInfo() ) );
            GetIDocumentUndoRedo().AppendUndo(pUndo);
        }

        bool bNumChg  = rInfo.nFtnOffset != GetEndNoteInfo().nFtnOffset;
        // this seems to be an optimization: UpdateAllFtn() is only called
        // if the offset changes; if the offset is the same,
        // but type/prefix/suffix changes, just set new numbers.
        bool const bExtra = !bNumChg &&
                (   (rInfo.aFmt.GetNumberingType() !=
                        GetEndNoteInfo().aFmt.GetNumberingType())
                ||  (rInfo.GetPrefix() != GetEndNoteInfo().GetPrefix())
                ||  (rInfo.GetSuffix() != GetEndNoteInfo().GetSuffix())
                );
        bool bFtnDesc = rInfo.GetPageDesc( *this ) !=
                            GetEndNoteInfo().GetPageDesc( *this );
        SwCharFmt *pOldChrFmt = GetEndNoteInfo().GetCharFmt( *this ),
                  *pNewChrFmt = rInfo.GetCharFmt( *this );
        bool bFtnChrFmts = pOldChrFmt != pNewChrFmt;

        *mpEndNoteInfo = rInfo;

        if ( pTmpRoot )
        {
            if ( bFtnDesc )
                //pTmpRoot->CheckFtnPageDescs( TRUE );
            {
                std::set<SwRootFrm*> aAllLayouts = GetAllLayouts();
                std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::bind2nd(std::mem_fun(&SwRootFrm::CheckFtnPageDescs), sal_True));//swmod 080304
            }
            if ( bExtra )
            {
                // For messages regarding ErgoSum etc. we save the extra code and use the
                // available methods.
                SwFtnIdxs& rFtnIdxs = GetFtnIdxs();
                for( sal_uInt16 nPos = 0; nPos < rFtnIdxs.size(); ++nPos )
                {
                    SwTxtFtn *pTxtFtn = rFtnIdxs[ nPos ];
                    const SwFmtFtn &rFtn = pTxtFtn->GetFtn();
                    if ( rFtn.IsEndNote() )
                        pTxtFtn->SetNumber( rFtn.GetNumber(), &rFtn.GetNumStr());
                }
            }
        }   //swmod 080219
        if( bNumChg )
            GetFtnIdxs().UpdateAllFtn();
        else if( bFtnChrFmts )
        {
            SwFmtChg aOld( pOldChrFmt );
            SwFmtChg aNew( pNewChrFmt );
            mpEndNoteInfo->ModifyNotification( &aOld, &aNew );
        }

        // #i81002# no update during loading
        if ( !IsInReading() )
        {
            UpdateRefFlds(NULL);
        }
        SetModified();
    }
}

bool SwDoc::SetCurFtn( const SwPaM& rPam, const String& rNumStr,
                       sal_uInt16 nNumber, bool bIsEndNote )
{
    SwFtnIdxs& rFtnArr = GetFtnIdxs();
    SwRootFrm* pTmpRoot = GetCurrentLayout();//swmod 080219

    const SwPosition* pStt = rPam.Start(), *pEnd = rPam.End();
    const sal_uLong nSttNd = pStt->nNode.GetIndex();
    const xub_StrLen nSttCnt = pStt->nContent.GetIndex();
    const sal_uLong nEndNd = pEnd->nNode.GetIndex();
    const xub_StrLen nEndCnt = pEnd->nContent.GetIndex();

    sal_uInt16 nPos;
    rFtnArr.SeekEntry( pStt->nNode, &nPos );

    SwUndoChangeFootNote* pUndo = 0;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().ClearRedo(); // AppendUndo far below, so leave it
        pUndo = new SwUndoChangeFootNote( rPam, rNumStr, nNumber, bIsEndNote );
    }

    SwTxtFtn* pTxtFtn;
    sal_uLong nIdx;
    bool bChg = false;
    bool bTypeChgd = false;
    sal_uInt16 n = nPos;        // save
    while( nPos < rFtnArr.size() &&
            (( nIdx = _SwTxtFtn_GetIndex((pTxtFtn = rFtnArr[ nPos++ ] )))
                < nEndNd || ( nIdx == nEndNd &&
                nEndCnt >= *pTxtFtn->GetStart() )) )
        if( nIdx > nSttNd || ( nIdx == nSttNd &&
                nSttCnt <= *pTxtFtn->GetStart() ) )
        {
            const SwFmtFtn& rFtn = pTxtFtn->GetFtn();
            if( rFtn.GetNumStr() != rNumStr ||
                rFtn.IsEndNote() != bIsEndNote )
            {
                bChg = true;
                if ( pUndo )
                {
                    pUndo->GetHistory().Add( *pTxtFtn );
                }

                pTxtFtn->SetNumber( nNumber, &rNumStr );
                if( rFtn.IsEndNote() != bIsEndNote )
                {
                    ((SwFmtFtn&)rFtn).SetEndNote( bIsEndNote );
                    bTypeChgd = true;
                    pTxtFtn->CheckCondColl();
                    //#i11339# dispose UNO wrapper when a footnote is changed to an endnote or vice versa
                    SwPtrMsgPoolItem aMsgHint( RES_FOOTNOTE_DELETED, (void*)&pTxtFtn->GetAttr() );
                    GetUnoCallBack()->ModifyNotification( &aMsgHint, &aMsgHint );
                }
            }
        }

    nPos = n;       // There are more in the front!
    while( nPos &&
            (( nIdx = _SwTxtFtn_GetIndex((pTxtFtn = rFtnArr[ --nPos ] )))
                > nSttNd || ( nIdx == nSttNd &&
                nSttCnt <= *pTxtFtn->GetStart() )) )
        if( nIdx < nEndNd || ( nIdx == nEndNd &&
            nEndCnt >= *pTxtFtn->GetStart() ) )
        {
            const SwFmtFtn& rFtn = pTxtFtn->GetFtn();
            if( rFtn.GetNumStr() != rNumStr ||
                rFtn.IsEndNote() != bIsEndNote )
            {
                bChg = true;
                if ( pUndo )
                {
                    pUndo->GetHistory().Add( *pTxtFtn );
                }

                pTxtFtn->SetNumber( nNumber, &rNumStr );
                if( rFtn.IsEndNote() != bIsEndNote )
                {
                    ((SwFmtFtn&)rFtn).SetEndNote( bIsEndNote );
                    bTypeChgd = true;
                    pTxtFtn->CheckCondColl();
                }
            }
        }

    // Who needs to be triggered?
    if( bChg )
    {
        if( pUndo )
        {
            GetIDocumentUndoRedo().AppendUndo(pUndo);
        }

        if ( bTypeChgd )
            rFtnArr.UpdateAllFtn();
        if( FTNNUM_PAGE != GetFtnInfo().eNum )
        {
            if ( !bTypeChgd )
                rFtnArr.UpdateAllFtn();
        }
        else if( pTmpRoot )
            //
        {
            std::set<SwRootFrm*> aAllLayouts = GetAllLayouts();
            std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::mem_fun(&SwRootFrm::UpdateFtnNums));
        }   //swmod 080304pTmpRoot->UpdateFtnNums();    //swmod 080219
        SetModified();
    }
    else
        delete pUndo;
    return bChg;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
