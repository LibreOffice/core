/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docftn.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 10:36:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#ifndef _FTNIDX_HXX //autogen
#include <ftnidx.hxx>
#endif
#ifndef _ROOTFRM_HXX //autogen
#include <rootfrm.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _PAM_HXX //autogen
#include <pam.hxx>
#endif
#ifndef _PAGEDESC_HXX //autogen
#include <pagedesc.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif
#ifndef _ROLBCK_HXX
#include <rolbck.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _FTNINFO_HXX
#include <ftninfo.hxx>
#endif

/*********************** SwFtnInfo ***************************/

SwEndNoteInfo& SwEndNoteInfo::operator=(const SwEndNoteInfo& rInfo)
{
    if( rInfo.GetFtnTxtColl() )
        rInfo.GetFtnTxtColl()->Add(this);
    else if ( pRegisteredIn)
        pRegisteredIn->Remove(this);

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
    bEndNote = rInfo.bEndNote;
    sPrefix = rInfo.sPrefix;
    sSuffix = rInfo.sSuffix;
    return *this;
}


BOOL SwEndNoteInfo::operator==( const SwEndNoteInfo& rInfo ) const
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
            bEndNote == rInfo.bEndNote &&
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
    bEndNote( TRUE ),
    aFmt( rInfo.aFmt ),
    nFtnOffset( rInfo.nFtnOffset )
{
    if( rInfo.GetPageDescDep()->GetRegisteredIn() )
        ((SwModify*)rInfo.GetPageDescDep()->GetRegisteredIn())->Add( &aPageDescDep );

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
    bEndNote( TRUE ),
    nFtnOffset( 0 )
{
    aFmt.SetNumberingType(SVX_NUM_ROMAN_LOWER);
}

SwPageDesc *SwEndNoteInfo::GetPageDesc( SwDoc &rDoc ) const
{
    if ( !aPageDescDep.GetRegisteredIn() )
    {
        SwPageDesc *pDesc = rDoc.GetPageDescFromPool( static_cast<sal_uInt16>(
            bEndNote ? RES_POOLPAGE_ENDNOTE : RES_POOLPAGE_FOOTNOTE ) );
        pDesc->Add( &((SwClient&)aPageDescDep) );
    }
    return (SwPageDesc*)aPageDescDep.GetRegisteredIn();
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
            bEndNote ? RES_POOLCHR_ENDNOTE : RES_POOLCHR_FOOTNOTE ) );
        pFmt->Add( &((SwClient&)aCharFmtDep) );
    }
    return (SwCharFmt*)aCharFmtDep.GetRegisteredIn();
}

void SwEndNoteInfo::SetCharFmt( SwCharFmt* pChFmt )
{
    DBG_ASSERT(pChFmt, "kein CharFmt?")
    pChFmt->Add( &((SwClient&)aCharFmtDep) );
}

SwCharFmt* SwEndNoteInfo::GetAnchorCharFmt(SwDoc &rDoc) const
{
    if( !aAnchorCharFmtDep.GetRegisteredIn() )
    {
        SwCharFmt* pFmt = rDoc.GetCharFmtFromPool( static_cast<sal_uInt16>(
            bEndNote ? RES_POOLCHR_ENDNOTE_ANCHOR : RES_POOLCHR_FOOTNOTE_ANCHOR ) );
        pFmt->Add( &((SwClient&)aAnchorCharFmtDep) );
    }
    return (SwCharFmt*)aAnchorCharFmtDep.GetRegisteredIn();
}

void SwEndNoteInfo::SetAnchorCharFmt( SwCharFmt* pChFmt )
{
    DBG_ASSERT(pChFmt, "kein CharFmt?")
    pChFmt->Add( &((SwClient&)aAnchorCharFmtDep) );
}

void SwEndNoteInfo::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
{
    USHORT nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0 ;

    if( RES_ATTRSET_CHG == nWhich ||
        RES_FMT_CHG == nWhich )
    {
        SwDoc* pDoc;
        if( aCharFmtDep.GetRegisteredIn() )
            pDoc = ((SwCharFmt*)aCharFmtDep.GetRegisteredIn())->GetDoc();
        else
            pDoc = ((SwCharFmt*)aAnchorCharFmtDep.GetRegisteredIn())->GetDoc();
        SwFtnIdxs& rFtnIdxs = pDoc->GetFtnIdxs();
        for( USHORT nPos = 0; nPos < rFtnIdxs.Count(); ++nPos )
        {
            SwTxtFtn *pTxtFtn = rFtnIdxs[ nPos ];
            const SwFmtFtn &rFtn = pTxtFtn->GetFtn();
            if ( rFtn.IsEndNote() == bEndNote)
                pTxtFtn->SetNumber( rFtn.GetNumber(), &rFtn.GetNumStr());
        }
    }
    else
        SwClient::Modify( pOld, pNew );
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


BOOL SwFtnInfo::operator==( const SwFtnInfo& rInfo ) const
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
    bEndNote = FALSE;
}

SwFtnInfo::SwFtnInfo(SwTxtFmtColl *pFmt) :
    SwEndNoteInfo( pFmt ),
    ePos( FTNPOS_PAGE ),
    eNum( FTNNUM_DOC )
{
    aFmt.SetNumberingType(SVX_NUM_ARABIC);
    bEndNote = FALSE;
}

/*********************** SwDoc ***************************/


void SwDoc::SetFtnInfo(const SwFtnInfo& rInfo)
{
    if( !(GetFtnInfo() == rInfo) )
    {
        const SwFtnInfo &rOld = GetFtnInfo();

        if( DoesUndo() )
        {
            ClearRedo();
            AppendUndo( new SwUndoFtnInfo( rOld ) );
        }

        BOOL bFtnPos  = rInfo.ePos != rOld.ePos;
        BOOL bFtnDesc = rOld.ePos == FTNPOS_CHAPTER &&
                            rInfo.GetPageDesc( *this ) != rOld.GetPageDesc( *this );
        BOOL bExtra   = rInfo.aQuoVadis != rOld.aQuoVadis ||
                            rInfo.aErgoSum != rOld.aErgoSum ||
                            rInfo.aFmt.GetNumberingType() != rOld.aFmt.GetNumberingType() ||
                            rInfo.GetPrefix() != rOld.GetPrefix() ||
                            rInfo.GetSuffix() != rOld.GetSuffix();
        SwCharFmt *pOldChrFmt = rOld.GetCharFmt( *this ),
                  *pNewChrFmt = rInfo.GetCharFmt( *this );
        BOOL bFtnChrFmts = pOldChrFmt != pNewChrFmt;

        *pFtnInfo = rInfo;

        if ( GetRootFrm() )
        {
            if ( bFtnPos )
                GetRootFrm()->RemoveFtns();
            else
            {
                GetRootFrm()->UpdateFtnNums();
                if ( bFtnDesc )
                    GetRootFrm()->CheckFtnPageDescs( FALSE );
                if ( bExtra )
                {
                    //Fuer die Benachrichtung bezueglich ErgoSum usw. sparen wir uns
                    //extra-Code und nutzen die vorhandenen Wege.
                    SwFtnIdxs& rFtnIdxs = GetFtnIdxs();
                    for( USHORT nPos = 0; nPos < rFtnIdxs.Count(); ++nPos )
                    {
                        SwTxtFtn *pTxtFtn = rFtnIdxs[ nPos ];
                        const SwFmtFtn &rFtn = pTxtFtn->GetFtn();
                        if ( !rFtn.IsEndNote() )
                            pTxtFtn->SetNumber( rFtn.GetNumber(), &rFtn.GetNumStr());
                    }
                }
            }
        }
        if( FTNNUM_PAGE != rInfo.eNum )
            GetFtnIdxs().UpdateAllFtn();
        else if( bFtnChrFmts )
        {
            SwFmtChg aOld( pOldChrFmt );
            SwFmtChg aNew( pNewChrFmt );
            pFtnInfo->Modify( &aOld, &aNew );
        }

        // --> OD 2008-01-09 #i81002#
        // no update during loading
        if ( !IsInReading() )
        {
            UpdateRefFlds(NULL);
        }
        SetModified();
    }
}

void SwDoc::SetEndNoteInfo(const SwEndNoteInfo& rInfo)
{
    if( !(GetEndNoteInfo() == rInfo) )
    {
        if( DoesUndo() )
        {
            ClearRedo();
            AppendUndo( new SwUndoEndNoteInfo( GetEndNoteInfo() ) );
        }

        BOOL bNumChg  = rInfo.nFtnOffset != GetEndNoteInfo().nFtnOffset;
        BOOL bExtra   = !bNumChg &&
                            rInfo.aFmt.GetNumberingType() != GetEndNoteInfo().aFmt.GetNumberingType()||
                            rInfo.GetPrefix() != GetEndNoteInfo().GetPrefix() ||
                            rInfo.GetSuffix() != GetEndNoteInfo().GetSuffix();
        BOOL bFtnDesc = rInfo.GetPageDesc( *this ) !=
                            GetEndNoteInfo().GetPageDesc( *this );
        SwCharFmt *pOldChrFmt = GetEndNoteInfo().GetCharFmt( *this ),
                  *pNewChrFmt = rInfo.GetCharFmt( *this );
        BOOL bFtnChrFmts = pOldChrFmt != pNewChrFmt;

        *pEndNoteInfo = rInfo;

        if ( GetRootFrm() )
        {
            if ( bFtnDesc )
                GetRootFrm()->CheckFtnPageDescs( TRUE );
            if ( bExtra )
            {
                //Fuer die Benachrichtung bezueglich ErgoSum usw. sparen wir uns
                //extra-Code und nutzen die vorhandenen Wege.
                SwFtnIdxs& rFtnIdxs = GetFtnIdxs();
                for( USHORT nPos = 0; nPos < rFtnIdxs.Count(); ++nPos )
                {
                    SwTxtFtn *pTxtFtn = rFtnIdxs[ nPos ];
                    const SwFmtFtn &rFtn = pTxtFtn->GetFtn();
                    if ( rFtn.IsEndNote() )
                        pTxtFtn->SetNumber( rFtn.GetNumber(), &rFtn.GetNumStr());
                }
            }
        }
        if( bNumChg )
            GetFtnIdxs().UpdateAllFtn();
        else if( bFtnChrFmts )
        {
            SwFmtChg aOld( pOldChrFmt );
            SwFmtChg aNew( pNewChrFmt );
            pEndNoteInfo->Modify( &aOld, &aNew );
        }

        // --> OD 2008-01-09 #i81002#
        // no update during loading
        if ( !IsInReading() )
        {
            UpdateRefFlds(NULL);
        }
        SetModified();
    }
}


BOOL SwDoc::SetCurFtn( const SwPaM& rPam, const String& rNumStr,
                        USHORT nNumber, BOOL bIsEndNote )
{
    SwFtnIdxs& rFtnArr = GetFtnIdxs();

    const SwPosition* pStt = rPam.Start(), *pEnd = rPam.End();
    const ULONG nSttNd = pStt->nNode.GetIndex();
    const xub_StrLen nSttCnt = pStt->nContent.GetIndex();
    const ULONG nEndNd = pEnd->nNode.GetIndex();
    const xub_StrLen nEndCnt = pEnd->nContent.GetIndex();

    USHORT nPos;
    rFtnArr.SeekEntry( pStt->nNode, &nPos );

    SwUndoChgFtn* pUndo = 0;
    if( DoesUndo() )
    {
        ClearRedo();
        pUndo = new SwUndoChgFtn( rPam, rNumStr, nNumber, bIsEndNote );
    }

    SwTxtFtn* pTxtFtn;
    ULONG nIdx;
    BOOL bChg = FALSE;
    BOOL bTypeChgd = FALSE;
    USHORT n = nPos;        // sichern
    while( nPos < rFtnArr.Count() &&
            (( nIdx = _SwTxtFtn_GetIndex((pTxtFtn = rFtnArr[ nPos++ ] )))
                < nEndNd || ( nIdx == nEndNd &&
                nEndCnt >= *pTxtFtn->GetStart() )) )
        if( nIdx > nSttNd || ( nIdx == nSttNd &&
                nSttCnt <= *pTxtFtn->GetStart() ) )
        {
            const SwFmtFtn& rFtn = pTxtFtn->GetFtn();
            if( /*rFtn.GetNumber() != nNumber ||*/
                rFtn.GetNumStr() != rNumStr ||
                rFtn.IsEndNote() != bIsEndNote )
            {
                bChg = TRUE;
                if( pUndo )
                    pUndo->GetHistory()->Add( *pTxtFtn );

                pTxtFtn->SetNumber( nNumber, &rNumStr );
                if( rFtn.IsEndNote() != bIsEndNote )
                {
                    ((SwFmtFtn&)rFtn).SetEndNote( bIsEndNote );
                    bTypeChgd = TRUE;
                    pTxtFtn->CheckCondColl();
                    //#i11339# dispose UNO wrapper when a footnote is changed to an endnote or vice versa
                    SwPtrMsgPoolItem aMsgHint( RES_FOOTNOTE_DELETED, (void*)&pTxtFtn->GetAttr() );
                    GetUnoCallBack()->Modify( &aMsgHint, &aMsgHint );
                }
            }
        }

    nPos = n;       // nach vorne gibt es auch noch welche !
    while( nPos &&
            (( nIdx = _SwTxtFtn_GetIndex((pTxtFtn = rFtnArr[ --nPos ] )))
                > nSttNd || ( nIdx == nSttNd &&
                nSttCnt <= *pTxtFtn->GetStart() )) )
        if( nIdx < nEndNd || ( nIdx == nEndNd &&
            nEndCnt >= *pTxtFtn->GetStart() ) )
        {
            const SwFmtFtn& rFtn = pTxtFtn->GetFtn();
            if( /*rFtn.GetNumber() != nNumber ||*/
                rFtn.GetNumStr() != rNumStr ||
                rFtn.IsEndNote() != bIsEndNote )
            {
                bChg = TRUE;
                if( pUndo )
                    pUndo->GetHistory()->Add( *pTxtFtn );

                pTxtFtn->SetNumber( nNumber, &rNumStr );
                if( rFtn.IsEndNote() != bIsEndNote )
                {
                    ((SwFmtFtn&)rFtn).SetEndNote( bIsEndNote );
                    bTypeChgd = TRUE;
                    pTxtFtn->CheckCondColl();
                }
            }
        }

    // wer muss angestossen werden ??
    if( bChg )
    {
        if( pUndo )
        {
            ClearRedo();
            AppendUndo( pUndo );
        }

        if ( bTypeChgd )
            rFtnArr.UpdateAllFtn();
        if( FTNNUM_PAGE != GetFtnInfo().eNum )
        {
            if ( !bTypeChgd )
                rFtnArr.UpdateAllFtn();
        }
        else if( GetRootFrm() )
            GetRootFrm()->UpdateFtnNums();
        SetModified();
    }
    else
        delete pUndo;
    return bChg;
}





