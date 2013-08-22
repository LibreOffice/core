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


#include <UndoInsert.hxx>

#include <hintids.hxx>
#include <unotools/charclass.hxx>
#include <sot/storage.hxx>
#include <editeng/keepitem.hxx>
#include <svx/svdobj.hxx>
#include <xmloff/odffields.hxx>

#include <docsh.hxx>
#include <fmtcntnt.hxx>
#include <fmtanchr.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <swundo.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <UndoCore.hxx>
#include <UndoDelete.hxx>
#include <UndoAttribute.hxx>
#include <rolbck.hxx>
#include <ndgrf.hxx>
#include <ndole.hxx>
#include <grfatr.hxx>
#include <cntfrm.hxx>
#include <flyfrm.hxx>
#include <fesh.hxx>
#include <swtable.hxx>
#include <redline.hxx>
#include <docary.hxx>
#include <acorrect.hxx>
#include <dcontact.hxx>

#include <comcore.hrc> // #111827#
#include <undo.hrc>

using namespace ::com::sun::star;


//------------------------------------------------------------

// INSERT

String * SwUndoInsert::GetTxtFromDoc() const
{
    String * pResult = NULL;

    SwNodeIndex aNd( pDoc->GetNodes(), nNode);
    SwCntntNode* pCNd = aNd.GetNode().GetCntntNode();
    SwPaM aPaM( *pCNd, nCntnt );

    aPaM.SetMark();

    if( pCNd->IsTxtNode() )
    {
        OUString sTxt = static_cast<SwTxtNode*>(pCNd)->GetTxt();

        sal_Int32 nStart = nCntnt-nLen;
        sal_Int32 nLength = nLen;

        if (nStart < 0)
        {
            nLength += nStart;
            nStart = 0;
        }

        pResult = new String(sTxt.copy(nStart, nLength));
    }

    return pResult;
}

void SwUndoInsert::Init(const SwNodeIndex & rNd)
{
    // consider Redline
    pDoc = rNd.GetNode().GetDoc();
    if( pDoc->IsRedlineOn() )
    {
        pRedlData = new SwRedlineData( nsRedlineType_t::REDLINE_INSERT,
                                       pDoc->GetRedlineAuthor() );
        SetRedlineMode( pDoc->GetRedlineMode() );
    }

    pUndoTxt = GetTxtFromDoc();

    bCacheComment = false;
}

// #111827#
SwUndoInsert::SwUndoInsert( const SwNodeIndex& rNd, xub_StrLen nCnt,
            xub_StrLen nL,
            const IDocumentContentOperations::InsertFlags nInsertFlags,
            sal_Bool bWDelim )
    : SwUndo(UNDO_TYPING), pTxt( 0 ), pRedlData( 0 ),
        nNode( rNd.GetIndex() ), nCntnt(nCnt), nLen(nL),
        bIsWordDelim( bWDelim ), bIsAppend( sal_False )
    , m_bWithRsid(false)
    , m_nInsertFlags(nInsertFlags)
{
    Init(rNd);
}

// #111827#
SwUndoInsert::SwUndoInsert( const SwNodeIndex& rNd )
    : SwUndo(UNDO_SPLITNODE), pTxt( 0 ),
        pRedlData( 0 ), nNode( rNd.GetIndex() ), nCntnt(0), nLen(1),
        bIsWordDelim( sal_False ), bIsAppend( sal_True )
    , m_bWithRsid(false)
    , m_nInsertFlags(IDocumentContentOperations::INS_EMPTYEXPAND)
{
    Init(rNd);
}

// Check if the next Insert can be combined with the current one. If so
// change the length and InsPos. As a result, SwDoc::Inser will not add a
// new object into the Undo list.

sal_Bool SwUndoInsert::CanGrouping( sal_Unicode cIns )
{
    if( !bIsAppend && bIsWordDelim ==
        !GetAppCharClass().isLetterNumeric( OUString( cIns )) )
    {
        nLen++;
        nCntnt++;

        if (pUndoTxt)
            pUndoTxt->Insert(cIns);

        return sal_True;
    }
    return sal_False;
}

sal_Bool SwUndoInsert::CanGrouping( const SwPosition& rPos )
{
    sal_Bool bRet = sal_False;
    if( nNode == rPos.nNode.GetIndex() &&
        nCntnt == rPos.nContent.GetIndex() )
    {
        // consider Redline
        SwDoc& rDoc = *rPos.nNode.GetNode().GetDoc();
        if( ( ~nsRedlineMode_t::REDLINE_SHOW_MASK & rDoc.GetRedlineMode() ) ==
            ( ~nsRedlineMode_t::REDLINE_SHOW_MASK & GetRedlineMode() ) )
        {
            bRet = sal_True;

            // than there is or was still an active Redline:
            // Check if there is another Redline at the InsPosition. If the
            // same exists only once, it can be combined.
            const SwRedlineTbl& rTbl = rDoc.GetRedlineTbl();
            if( !rTbl.empty() )
            {
                SwRedlineData aRData( nsRedlineType_t::REDLINE_INSERT, rDoc.GetRedlineAuthor() );
                const SwIndexReg* pIReg = rPos.nContent.GetIdxReg();
                SwIndex* pIdx;
                for( sal_uInt16 i = 0; i < rTbl.size(); ++i )
                {
                    SwRedline* pRedl = rTbl[ i ];
                    if( pIReg == (pIdx = &pRedl->End()->nContent)->GetIdxReg() &&
                        nCntnt == pIdx->GetIndex() )
                    {
                        if( !pRedl->HasMark() || !pRedlData ||
                            *pRedl != *pRedlData || *pRedl != aRData )
                        {
                            bRet = sal_False;
                            break;
                        }
                    }
                }
            }
        }
    }
    return bRet;
}

SwUndoInsert::~SwUndoInsert()
{
    if (m_pUndoNodeIndex) // delete the section from UndoNodes array
    {
        // Insert saves the content in IconSection
        SwNodes& rUNds = m_pUndoNodeIndex->GetNodes();
        rUNds.Delete(*m_pUndoNodeIndex,
            rUNds.GetEndOfExtras().GetIndex() - m_pUndoNodeIndex->GetIndex());
        m_pUndoNodeIndex.reset();
    }
    else if( pTxt )     // the inserted text
        delete pTxt;
    delete pRedlData;
    delete pUndoTxt;
}

void SwUndoInsert::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pTmpDoc = & rContext.GetDoc();
    SwPaM *const pPam(& rContext.GetCursorSupplier().CreateNewShellCursor());

    if( bIsAppend )
    {
        pPam->GetPoint()->nNode = nNode;

        if( IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ))
        {
            pPam->GetPoint()->nContent.Assign( pPam->GetCntntNode(), 0 );
            pPam->SetMark();
            pPam->Move( fnMoveBackward );
            pPam->Exchange();
            pTmpDoc->DeleteRedline( *pPam, true, USHRT_MAX );
        }
        pPam->DeleteMark();
        pTmpDoc->DelFullPara( *pPam );
        pPam->GetPoint()->nContent.Assign( pPam->GetCntntNode(), 0 );
    }
    else
    {
        sal_uLong nNd = nNode;
        xub_StrLen nCnt = nCntnt;
        if( nLen )
        {
            SwNodeIndex aNd( pTmpDoc->GetNodes(), nNode);
            SwCntntNode* pCNd = aNd.GetNode().GetCntntNode();
            SwPaM aPaM( *pCNd, nCntnt );

            aPaM.SetMark();

            SwTxtNode * const pTxtNode( pCNd->GetTxtNode() );
            if ( pTxtNode )
            {
                aPaM.GetPoint()->nContent -= nLen;
                if( IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ))
                    pTmpDoc->DeleteRedline( aPaM, true, USHRT_MAX );
                if (m_bWithRsid)
                {
                    // RSID was added: remove any CHARFMT/AUTOFMT that may be
                    // set on the deleted text; EraseText will leave empty
                    // ones behind otherwise
                    pTxtNode->DeleteAttributes(RES_TXTATR_AUTOFMT,
                        aPaM.GetPoint()->nContent.GetIndex(),
                        aPaM.GetMark()->nContent.GetIndex());
                    pTxtNode->DeleteAttributes(RES_TXTATR_CHARFMT,
                        aPaM.GetPoint()->nContent.GetIndex(),
                        aPaM.GetMark()->nContent.GetIndex());
                }
                RemoveIdxFromRange( aPaM, sal_False );
                pTxt = new String( pTxtNode->GetTxt().copy(nCntnt-nLen, nLen) );
                pTxtNode->EraseText( aPaM.GetPoint()->nContent, nLen );

                // Undo deletes fieldmarks in two step: first the end then the start position.
                // Once the start position is deleted, make sure the fieldmark itself is deleted as well.
                if (nLen == 1)
                {
                    IDocumentMarkAccess* const pMarkAccess = pTmpDoc->getIDocumentMarkAccess();
                    for ( IDocumentMarkAccess::const_iterator_t i = pMarkAccess->getMarksBegin(); i != pMarkAccess->getMarksEnd(); ++i)
                    {
                        sw::mark::IMark* pMark = i->get();
                        if (pMark->GetMarkStart() == *aPaM.GetPoint() && pMark->GetMarkStart().nContent == aPaM.GetPoint()->nContent)
                        {
                            sw::mark::IFieldmark* pFieldmark = dynamic_cast<sw::mark::IFieldmark*>(pMark);
                            if (pFieldmark && pFieldmark->GetFieldname() == ODF_COMMENTRANGE)
                            {
                                pTmpDoc->getIDocumentMarkAccess()->deleteMark(pMark);
                                break;
                            }
                        }
                    }
                }
            }
            else                // otherwise Graphics/OLE/Text/...
            {
                aPaM.Move(fnMoveBackward);
                if( IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ))
                    pTmpDoc->DeleteRedline( aPaM, true, USHRT_MAX );
                RemoveIdxFromRange( aPaM, sal_False );
            }

            nNd = aPaM.GetPoint()->nNode.GetIndex();
            nCnt = aPaM.GetPoint()->nContent.GetIndex();

            if( !pTxt )
            {
                m_pUndoNodeIndex.reset(
                        new SwNodeIndex(pDoc->GetNodes().GetEndOfContent()));
                MoveToUndoNds(aPaM, m_pUndoNodeIndex.get());
            }
            nNode = aPaM.GetPoint()->nNode.GetIndex();
            nCntnt = aPaM.GetPoint()->nContent.GetIndex();
        }

        // set cursor to Undo range
        pPam->DeleteMark();

        pPam->GetPoint()->nNode = nNd;
        pPam->GetPoint()->nContent.Assign(
                pPam->GetPoint()->nNode.GetNode().GetCntntNode(), nCnt );
    }

    DELETEZ(pUndoTxt);
}


void SwUndoInsert::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pTmpDoc = & rContext.GetDoc();
    SwPaM *const pPam(& rContext.GetCursorSupplier().CreateNewShellCursor());
    pPam->DeleteMark();

    if( bIsAppend )
    {
        pPam->GetPoint()->nNode = nNode - 1;
        pTmpDoc->AppendTxtNode( *pPam->GetPoint() );

        pPam->SetMark();
        pPam->Move( fnMoveBackward );
        pPam->Exchange();

        if( pRedlData && IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ))
        {
            RedlineMode_t eOld = pTmpDoc->GetRedlineMode();
            pTmpDoc->SetRedlineMode_intern((RedlineMode_t)(eOld & ~nsRedlineMode_t::REDLINE_IGNORE));
            pTmpDoc->AppendRedline( new SwRedline( *pRedlData, *pPam ), true);
            pTmpDoc->SetRedlineMode_intern( eOld );
        }
        else if( !( nsRedlineMode_t::REDLINE_IGNORE & GetRedlineMode() ) &&
                !pTmpDoc->GetRedlineTbl().empty() )
            pTmpDoc->SplitRedline( *pPam );

        pPam->DeleteMark();
    }
    else
    {
        pPam->GetPoint()->nNode = nNode;
        SwCntntNode *const pCNd =
            pPam->GetPoint()->nNode.GetNode().GetCntntNode();
        pPam->GetPoint()->nContent.Assign( pCNd, nCntnt );

        if( nLen )
        {
            sal_Bool bMvBkwrd = MovePtBackward( *pPam );

            if( pTxt )
            {
                SwTxtNode *const pTxtNode = pCNd->GetTxtNode();
                OSL_ENSURE( pTxtNode, "where is my textnode ?" );
                OUString const ins(
                    pTxtNode->InsertText( *pTxt, pPam->GetMark()->nContent,
                    m_nInsertFlags) );
                assert(ins.getLength() == pTxt->Len()); // must succeed
                DELETEZ( pTxt );
                if (m_bWithRsid) // re-insert RSID
                {
                    SwPaM pam(*pPam->GetMark(), 0); // mark -> point
                    pTmpDoc->UpdateRsid(pam, ins.getLength());
                }
            }
            else
            {
                // re-insert content again (first detach m_pUndoNodeIndex!)
                sal_uLong const nMvNd = m_pUndoNodeIndex->GetIndex();
                m_pUndoNodeIndex.reset();
                MoveFromUndoNds(*pTmpDoc, nMvNd, *pPam->GetMark());
            }
            nNode = pPam->GetMark()->nNode.GetIndex();
            nCntnt = pPam->GetMark()->nContent.GetIndex();

            MovePtForward( *pPam, bMvBkwrd );
            pPam->Exchange();
            if( pRedlData && IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ))
            {
                RedlineMode_t eOld = pTmpDoc->GetRedlineMode();
                pTmpDoc->SetRedlineMode_intern((RedlineMode_t)(eOld & ~nsRedlineMode_t::REDLINE_IGNORE));
                pTmpDoc->AppendRedline( new SwRedline( *pRedlData,
                                            *pPam ), true);
                pTmpDoc->SetRedlineMode_intern( eOld );
            }
            else if( !( nsRedlineMode_t::REDLINE_IGNORE & GetRedlineMode() ) &&
                    !pTmpDoc->GetRedlineTbl().empty() )
                pTmpDoc->SplitRedline(*pPam);
        }
    }

    pUndoTxt = GetTxtFromDoc();
}

void SwUndoInsert::RepeatImpl(::sw::RepeatContext & rContext)
{
    if( !nLen )
        return;

    SwDoc & rDoc = rContext.GetDoc();
    SwNodeIndex aNd( rDoc.GetNodes(), nNode );
    SwCntntNode* pCNd = aNd.GetNode().GetCntntNode();

    if( !bIsAppend && 1 == nLen )       // >1 than always Text, otherwise Graphics/OLE/Text/...
    {
        SwPaM aPaM( *pCNd, nCntnt );
        aPaM.SetMark();
        aPaM.Move(fnMoveBackward);
        pCNd = aPaM.GetCntntNode();
    }

// What happens with the possible selected range ???

    switch( pCNd->GetNodeType() )
    {
    case ND_TEXTNODE:
        if( bIsAppend )
        {
            rDoc.AppendTxtNode( *rContext.GetRepeatPaM().GetPoint() );
        }
        else
        {
            OUString const aTxt( static_cast<SwTxtNode*>(pCNd)->GetTxt() );
            ::sw::GroupUndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());
            rDoc.InsertString( rContext.GetRepeatPaM(),
                aTxt.copy(nCntnt - nLen, nLen) );
        }
        break;
    case ND_GRFNODE:
        {
            SwGrfNode* pGrfNd = (SwGrfNode*)pCNd;
            OUString sFile;
            OUString sFilter;
            if( pGrfNd->IsGrfLink() )
                pGrfNd->GetFileFilterNms( &sFile, &sFilter );

            rDoc.Insert( rContext.GetRepeatPaM(), sFile, sFilter,
                                &pGrfNd->GetGrf(),
                                0/* Graphics collection*/, NULL, NULL );
        }
        break;

    case ND_OLENODE:
        {
            // StarView does not yet provide an option to copy a StarOBJ
            SvStorageRef aRef = new SvStorage( aEmptyStr );
            SwOLEObj& rSwOLE = (SwOLEObj&)((SwOLENode*)pCNd)->GetOLEObj();

            // temporary storage until object is inserted
            // TODO/MBA: seems that here a physical copy is done - not as in drawing layer! Testing!
            // TODO/LATER: Copying through the container would copy the replacement image as well
            comphelper::EmbeddedObjectContainer aCnt;
            OUString aName = aCnt.CreateUniqueObjectName();
            if ( aCnt.StoreEmbeddedObject( rSwOLE.GetOleRef(), aName, sal_True ) )
            {
                uno::Reference < embed::XEmbeddedObject > aNew = aCnt.GetEmbeddedObject( aName );
                rDoc.Insert( rContext.GetRepeatPaM(),
                    svt::EmbeddedObjectRef( aNew,
                        static_cast<SwOLENode*>(pCNd)->GetAspect() ),
                    NULL, NULL, NULL );
            }

            break;
        }
    }
}

// #111827#
SwRewriter SwUndoInsert::GetRewriter() const
{
    SwRewriter aResult;
    String * pStr = NULL;
    bool bDone = false;

    if (pTxt)
        pStr = pTxt;
    else if (pUndoTxt)
        pStr = pUndoTxt;

    if (pStr)
    {
        String aString = ShortenString(DenoteSpecialCharacters(*pStr),
                                       nUndoStringLength,
                                       String(SW_RES(STR_LDOTS)));

        aResult.AddRule(UndoArg1, aString);

        bDone = true;
    }

    if ( ! bDone )
    {
        aResult.AddRule(UndoArg1, String("??", RTL_TEXTENCODING_ASCII_US));
    }

    return aResult;
}


// SwUndoReplace /////////////////////////////////////////////////////////


class SwUndoReplace::Impl
    : private SwUndoSaveCntnt
{
    OUString m_sOld;
    OUString m_sIns;
    sal_uLong m_nSttNd, m_nEndNd, m_nOffset;
    xub_StrLen m_nSttCnt, m_nEndCnt, m_nSetPos, m_nSelEnd;
    bool m_bSplitNext : 1;
    bool m_bRegExp : 1;
    // metadata references for paragraph and following para (if m_bSplitNext)
    ::boost::shared_ptr< ::sfx2::MetadatableUndo > m_pMetadataUndoStart;
    ::boost::shared_ptr< ::sfx2::MetadatableUndo > m_pMetadataUndoEnd;

public:
    Impl(SwPaM const& rPam, OUString const& rIns, bool const bRegExp);
    virtual ~Impl()
    {
    }

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    void SetEnd(SwPaM const& rPam);

    OUString const& GetOld() const { return m_sOld; }
    OUString const& GetIns() const { return m_sIns; }
};


SwUndoReplace::SwUndoReplace(SwPaM const& rPam,
        OUString const& rIns, bool const bRegExp)
    : SwUndo( UNDO_REPLACE )
    , m_pImpl(new Impl(rPam, rIns, bRegExp))
{
}

SwUndoReplace::~SwUndoReplace()
{
}

void SwUndoReplace::UndoImpl(::sw::UndoRedoContext & rContext)
{
    m_pImpl->UndoImpl(rContext);
}

void SwUndoReplace::RedoImpl(::sw::UndoRedoContext & rContext)
{
    m_pImpl->RedoImpl(rContext);
}

SwRewriter
MakeUndoReplaceRewriter(sal_uLong const occurrences,
        OUString const& sOld, OUString const& sNew)
{
    SwRewriter aResult;

    if (1 < occurrences)
    {
        aResult.AddRule(UndoArg1, OUString::number(occurrences));
        aResult.AddRule(UndoArg2, String(SW_RES(STR_OCCURRENCES_OF)));

        String aTmpStr;
        aTmpStr += String(SW_RES(STR_START_QUOTE));
        aTmpStr += ShortenString(sOld, nUndoStringLength,
                                 SW_RES(STR_LDOTS));
        aTmpStr += String(SW_RES(STR_END_QUOTE));
        aResult.AddRule(UndoArg3, aTmpStr);
    }
    else if (1 == occurrences)
    {
        {
            String aTmpStr;

            aTmpStr += String(SW_RES(STR_START_QUOTE));
            // #i33488 #
            aTmpStr += ShortenString(sOld, nUndoStringLength,
                                     SW_RES(STR_LDOTS));
            aTmpStr += String(SW_RES(STR_END_QUOTE));
            aResult.AddRule(UndoArg1, aTmpStr);
        }

        aResult.AddRule(UndoArg2, String(SW_RES(STR_YIELDS)));

        {
            String aTmpStr;

            aTmpStr += String(SW_RES(STR_START_QUOTE));
            // #i33488 #
            aTmpStr += ShortenString(sNew, nUndoStringLength,
                                     SW_RES(STR_LDOTS));
            aTmpStr += String(SW_RES(STR_END_QUOTE));
            aResult.AddRule(UndoArg3, aTmpStr);
        }
    }

    return aResult;
}

// #111827#
SwRewriter SwUndoReplace::GetRewriter() const
{
    return MakeUndoReplaceRewriter(1, m_pImpl->GetOld(), m_pImpl->GetIns());
}

void SwUndoReplace::SetEnd(SwPaM const& rPam)
{
    m_pImpl->SetEnd(rPam);
}

SwUndoReplace::Impl::Impl(
        SwPaM const& rPam, OUString const& rIns, bool const bRegExp)
    : m_sIns( rIns )
    , m_nOffset( 0 )
    , m_bRegExp(bRegExp)
{

    const SwPosition * pStt( rPam.Start() );
    const SwPosition * pEnd( rPam.End() );

    m_nSttNd = m_nEndNd = pStt->nNode.GetIndex();
    m_nSttCnt = pStt->nContent.GetIndex();
    m_nSelEnd = m_nEndCnt = pEnd->nContent.GetIndex();

    m_bSplitNext = m_nSttNd != pEnd->nNode.GetIndex();

    SwTxtNode* pNd = pStt->nNode.GetNode().GetTxtNode();
    OSL_ENSURE( pNd, "Dude, where's my TextNode?" );

    pHistory = new SwHistory;
    DelCntntIndex( *rPam.GetMark(), *rPam.GetPoint() );

    m_nSetPos = pHistory->Count();

    sal_uLong nNewPos = pStt->nNode.GetIndex();
    m_nOffset = m_nSttNd - nNewPos;

    if ( pNd->GetpSwpHints() )
    {
        pHistory->CopyAttr( pNd->GetpSwpHints(), nNewPos, 0,
                            pNd->GetTxt().getLength(), true );
    }

    if ( m_bSplitNext )
    {
        if( pNd->HasSwAttrSet() )
            pHistory->CopyFmtAttr( *pNd->GetpSwAttrSet(), nNewPos );
        pHistory->Add( pNd->GetTxtColl(), nNewPos, ND_TEXTNODE );

        SwTxtNode* pNext = pEnd->nNode.GetNode().GetTxtNode();
        sal_uLong nTmp = pNext->GetIndex();
        pHistory->CopyAttr( pNext->GetpSwpHints(), nTmp, 0,
                            pNext->GetTxt().getLength(), true );
        if( pNext->HasSwAttrSet() )
            pHistory->CopyFmtAttr( *pNext->GetpSwAttrSet(), nTmp );
        pHistory->Add( pNext->GetTxtColl(),nTmp, ND_TEXTNODE );
        // METADATA: store
        m_pMetadataUndoStart = pNd  ->CreateUndo();
        m_pMetadataUndoEnd   = pNext->CreateUndo();
    }

    if( !pHistory->Count() )
        delete pHistory, pHistory = 0;

    xub_StrLen nECnt = m_bSplitNext ? pNd->GetTxt().getLength()
        : pEnd->nContent.GetIndex();
    m_sOld = pNd->GetTxt().copy( m_nSttCnt, nECnt - m_nSttCnt );
}

void SwUndoReplace::Impl::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    SwPaM & rPam(rContext.GetCursorSupplier().CreateNewShellCursor());
    rPam.DeleteMark();

    SwTxtNode* pNd = pDoc->GetNodes()[ m_nSttNd - m_nOffset ]->GetTxtNode();
    OSL_ENSURE( pNd, "Dude, where's my TextNode?" );

    SwAutoCorrExceptWord* pACEWord = pDoc->GetAutoCorrExceptWord();
    if( pACEWord )
    {
        if ((1 == m_sIns.getLength()) && (1 == m_sOld.getLength()))
        {
            SwPosition aPos( *pNd ); aPos.nContent.Assign( pNd, m_nSttCnt );
            pACEWord->CheckChar( aPos, m_sOld[ 0 ] );
        }
        pDoc->SetAutoCorrExceptWord( 0 );
    }

    SwIndex aIdx( pNd, m_nSttCnt );
    // don't look at m_sIns for deletion, maybe it was not completely inserted
    {
        rPam.GetPoint()->nNode = *pNd;
        rPam.GetPoint()->nContent.Assign( pNd, m_nSttCnt );
        rPam.SetMark();
        rPam.GetPoint()->nNode = m_nEndNd - m_nOffset;
        rPam.GetPoint()->nContent.Assign( rPam.GetCntntNode(), m_nEndCnt );
        // move it out of the way so it is not registered at deleted node
        aIdx.Assign(0, 0);

        pDoc->DeleteAndJoin( rPam );
        rPam.DeleteMark();
        pNd = rPam.GetNode()->GetTxtNode();
        OSL_ENSURE( pNd, "Dude, where's my TextNode?" );
        aIdx.Assign( pNd, m_nSttCnt );
    }

    if( m_bSplitNext )
    {
        SwPosition aPos( *pNd, aIdx );
        pDoc->SplitNode( aPos, false );
        pNd->RestoreMetadata(m_pMetadataUndoEnd);
        pNd = pDoc->GetNodes()[ m_nSttNd - m_nOffset ]->GetTxtNode();
        aIdx.Assign( pNd, m_nSttCnt );
        // METADATA: restore
        pNd->RestoreMetadata(m_pMetadataUndoStart);
    }

    if (!m_sOld.isEmpty())
    {
        OUString const ins( pNd->InsertText( m_sOld, aIdx ) );
        assert(ins.getLength() == m_sOld.getLength()); // must succeed
    }

    if( pHistory )
    {
        if( pNd->GetpSwpHints() )
            pNd->ClearSwpHintsArr( true );

        pHistory->TmpRollback( pDoc, m_nSetPos, false );
        if ( m_nSetPos ) // there were footnotes/FlyFrames
        {
            // are there others than these?
            if( m_nSetPos < pHistory->Count() )
            {
                // than save those attributes as well
                SwHistory aHstr;
                aHstr.Move( 0, pHistory, m_nSetPos );
                pHistory->Rollback( pDoc );
                pHistory->Move( 0, &aHstr );
            }
            else
            {
                pHistory->Rollback( pDoc );
                DELETEZ( pHistory );
            }
        }
    }

    rPam.GetPoint()->nNode = m_nSttNd;
    rPam.GetPoint()->nContent = aIdx;
}

void SwUndoReplace::Impl::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwPaM & rPam(rContext.GetCursorSupplier().CreateNewShellCursor());
    rPam.DeleteMark();
    rPam.GetPoint()->nNode = m_nSttNd;

    SwTxtNode* pNd = rPam.GetPoint()->nNode.GetNode().GetTxtNode();
    OSL_ENSURE( pNd, "Dude, where's my TextNode?" );
    rPam.GetPoint()->nContent.Assign( pNd, m_nSttCnt );
    rPam.SetMark();
    if( m_bSplitNext )
    {
        rPam.GetPoint()->nNode = m_nSttNd + 1;
        pNd = rPam.GetPoint()->nNode.GetNode().GetTxtNode();
    }
    rPam.GetPoint()->nContent.Assign( pNd, m_nSelEnd );

    if( pHistory )
    {
        SwHistory* pSave = pHistory;
        SwHistory aHst;
        pHistory = &aHst;
        DelCntntIndex( *rPam.GetMark(), *rPam.GetPoint() );
        m_nSetPos = pHistory->Count();

        pHistory = pSave;
        pHistory->Move( 0, &aHst );
    }
    else
    {
        pHistory = new SwHistory;
        DelCntntIndex( *rPam.GetMark(), *rPam.GetPoint() );
        m_nSetPos = pHistory->Count();
        if( !m_nSetPos )
            delete pHistory, pHistory = 0;
    }

    rDoc.ReplaceRange( rPam, m_sIns, m_bRegExp );
    rPam.DeleteMark();
}

void SwUndoReplace::Impl::SetEnd(SwPaM const& rPam)
{
    const SwPosition* pEnd = rPam.End();
    m_nEndNd = m_nOffset + pEnd->nNode.GetIndex();
    m_nEndCnt = pEnd->nContent.GetIndex();
}


// SwUndoReRead //////////////////////////////////////////////////////////


SwUndoReRead::SwUndoReRead( const SwPaM& rPam, const SwGrfNode& rGrfNd )
    : SwUndo( UNDO_REREAD ), nPos( rPam.GetPoint()->nNode.GetIndex() )
{
    SaveGraphicData( rGrfNd );
}


SwUndoReRead::~SwUndoReRead()
{
    delete pGrf;
    delete pNm;
    delete pFltr;
}


void SwUndoReRead::SetAndSave(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwGrfNode* pGrfNd = rDoc.GetNodes()[ nPos ]->GetGrfNode();

    if( !pGrfNd )
        return ;

    // cache the old values
    Graphic* pOldGrf = pGrf;
    OUString* pOldNm = pNm;
    OUString* pOldFltr = pFltr;
    sal_uInt16 nOldMirr = nMirr;

    SaveGraphicData( *pGrfNd );
    if( pOldNm )
    {
        pGrfNd->ReRead( *pOldNm, pFltr ? *pFltr : OUString(), 0, 0, sal_True );
        delete pOldNm;
        delete pOldFltr;
    }
    else
    {
        pGrfNd->ReRead( aEmptyStr, aEmptyStr, pOldGrf, 0, sal_True );
        delete pOldGrf;
    }

    if( RES_MIRROR_GRAPH_DONT != nOldMirr )
        pGrfNd->SetAttr( SwMirrorGrf() );

    rContext.SetSelections(pGrfNd->GetFlyFmt(), 0);
}


void SwUndoReRead::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SetAndSave(rContext);
}


void SwUndoReRead::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SetAndSave(rContext);
}


void SwUndoReRead::SaveGraphicData( const SwGrfNode& rGrfNd )
{
    if( rGrfNd.IsGrfLink() )
    {
        pNm = new OUString;
        pFltr = new OUString;
        rGrfNd.GetFileFilterNms( pNm, pFltr );
        pGrf = 0;
    }
    else
    {
        ((SwGrfNode&)rGrfNd).SwapIn( sal_True );
        pGrf = new Graphic( rGrfNd.GetGrf() );
        pNm = pFltr = 0;
    }
    nMirr = rGrfNd.GetSwAttrSet().GetMirrorGrf().GetValue();
}


SwUndoInsertLabel::SwUndoInsertLabel( const SwLabelType eTyp,
                                      const String &rTxt,
                                      const String& rSeparator,
                                      const String& rNumberSeparator,
                                      const sal_Bool bBef,
                                      const sal_uInt16 nInitId,
                                      const String& rCharacterStyle,
                                      const sal_Bool bCpyBorder )
    : SwUndo( UNDO_INSERTLABEL ),
      sText( rTxt ),
      sSeparator( rSeparator ),
      sNumberSeparator( rNumberSeparator ),//#i61007# order of captions
      sCharacterStyle( rCharacterStyle ),
      nFldId( nInitId ),
      eType( eTyp ),
      nLayerId( 0 ),
      bBefore( bBef ),
      bCpyBrd( bCpyBorder )
{
    bUndoKeep = sal_False;
    OBJECT.pUndoFly = 0;
    OBJECT.pUndoAttr = 0;
}

SwUndoInsertLabel::~SwUndoInsertLabel()
{
    if( LTYPE_OBJECT == eType || LTYPE_DRAW == eType )
    {
        delete OBJECT.pUndoFly;
        delete OBJECT.pUndoAttr;
    }
    else
        delete NODE.pUndoInsNd;
}

void SwUndoInsertLabel::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    if( LTYPE_OBJECT == eType || LTYPE_DRAW == eType )
    {
        OSL_ENSURE( OBJECT.pUndoAttr && OBJECT.pUndoFly, "Pointer not initialized" );
        SwFrmFmt* pFmt;
        SdrObject *pSdrObj = 0;
        if( OBJECT.pUndoAttr &&
            0 != (pFmt = (SwFrmFmt*)OBJECT.pUndoAttr->GetFmt( rDoc )) &&
            ( LTYPE_DRAW != eType ||
              0 != (pSdrObj = pFmt->FindSdrObject()) ) )
        {
            OBJECT.pUndoAttr->UndoImpl(rContext);
            OBJECT.pUndoFly->UndoImpl(rContext);
            if( LTYPE_DRAW == eType )
            {
                pSdrObj->SetLayer( nLayerId );
            }
        }
    }
    else if( NODE.nNode )
    {
        if ( eType == LTYPE_TABLE && bUndoKeep )
        {
            SwTableNode *pNd = rDoc.GetNodes()[
                        rDoc.GetNodes()[NODE.nNode-1]->StartOfSectionIndex()]->GetTableNode();
            if ( pNd )
                pNd->GetTable().GetFrmFmt()->ResetFmtAttr( RES_KEEP );
        }
        SwPaM aPam( rDoc.GetNodes().GetEndOfContent() );
        aPam.GetPoint()->nNode = NODE.nNode;
        aPam.SetMark();
        aPam.GetPoint()->nNode = NODE.nNode + 1;
        NODE.pUndoInsNd = new SwUndoDelete( aPam, sal_True );
    }
}


void SwUndoInsertLabel::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    if( LTYPE_OBJECT == eType || LTYPE_DRAW == eType )
    {
        OSL_ENSURE( OBJECT.pUndoAttr && OBJECT.pUndoFly, "Pointer not initialized" );
        SwFrmFmt* pFmt;
        SdrObject *pSdrObj = 0;
        if( OBJECT.pUndoAttr &&
            0 != (pFmt = (SwFrmFmt*)OBJECT.pUndoAttr->GetFmt( rDoc )) &&
            ( LTYPE_DRAW != eType ||
              0 != (pSdrObj = pFmt->FindSdrObject()) ) )
        {
            OBJECT.pUndoFly->RedoImpl(rContext);
            OBJECT.pUndoAttr->RedoImpl(rContext);
            if( LTYPE_DRAW == eType )
            {
                pSdrObj->SetLayer( nLayerId );
                if( pSdrObj->GetLayer() == rDoc.GetHellId() )
                    pSdrObj->SetLayer( rDoc.GetHeavenId() );
                // OD 02.07.2003 #108784#
                else if( pSdrObj->GetLayer() == rDoc.GetInvisibleHellId() )
                    pSdrObj->SetLayer( rDoc.GetInvisibleHeavenId() );
            }
        }
    }
    else if( NODE.pUndoInsNd )
    {
        if ( eType == LTYPE_TABLE && bUndoKeep )
        {
            SwTableNode *pNd = rDoc.GetNodes()[
                        rDoc.GetNodes()[NODE.nNode-1]->StartOfSectionIndex()]->GetTableNode();
            if ( pNd )
                pNd->GetTable().GetFrmFmt()->SetFmtAttr( SvxFmtKeepItem(sal_True, RES_KEEP) );
        }
        NODE.pUndoInsNd->UndoImpl(rContext);
        delete NODE.pUndoInsNd, NODE.pUndoInsNd = 0;
    }
}

void SwUndoInsertLabel::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    const SwPosition& rPos = *rContext.GetRepeatPaM().GetPoint();

    sal_uLong nIdx = 0;

    SwCntntNode* pCNd = rPos.nNode.GetNode().GetCntntNode();
    if( pCNd )
        switch( eType )
        {
        case LTYPE_TABLE:
            {
                const SwTableNode* pTNd = pCNd->FindTableNode();
                if( pTNd )
                    nIdx = pTNd->GetIndex();
            }
            break;

        case LTYPE_FLY:
        case LTYPE_OBJECT:
            {
                SwFlyFrm* pFly;
                SwCntntFrm *pCnt = pCNd->getLayoutFrm( rDoc.GetCurrentLayout() );
                if( pCnt && 0 != ( pFly = pCnt->FindFlyFrm() ) )
                    nIdx = pFly->GetFmt()->GetCntnt().GetCntntIdx()->GetIndex();
            }
            break;
        case LTYPE_DRAW:
            break;
        }

    if( nIdx )
    {
        rDoc.InsertLabel( eType, sText, sSeparator, sNumberSeparator, bBefore,
            nFldId, nIdx, sCharacterStyle, bCpyBrd );
    }
}

// #111827#
SwRewriter SwUndoInsertLabel::GetRewriter() const
{
    SwRewriter aRewriter;

    String aTmpStr;

    aTmpStr += String(SW_RES(STR_START_QUOTE));
    aTmpStr += ShortenString(sText, nUndoStringLength,
                             String(SW_RES(STR_LDOTS)));
    aTmpStr += String(SW_RES(STR_END_QUOTE));

    aRewriter.AddRule(UndoArg1, aTmpStr);

    return aRewriter;
}

void SwUndoInsertLabel::SetFlys( SwFrmFmt& rOldFly, SfxItemSet& rChgSet,
                                SwFrmFmt& rNewFly )
{
    if( LTYPE_OBJECT == eType || LTYPE_DRAW == eType )
    {
        SwUndoFmtAttrHelper aTmp( rOldFly, false );
        rOldFly.SetFmtAttr( rChgSet );
        if ( aTmp.GetUndo() )
        {
            OBJECT.pUndoAttr = aTmp.ReleaseUndo();
        }
        OBJECT.pUndoFly = new SwUndoInsLayFmt( &rNewFly,0,0 );
    }
}

void SwUndoInsertLabel::SetDrawObj( sal_uInt8 nLId )
{
    if( LTYPE_DRAW == eType )
    {
        nLayerId = nLId;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
