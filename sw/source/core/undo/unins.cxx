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

#include <docsh.hxx>
#include <fmtcntnt.hxx>
#include <fmtanchr.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
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
#include <fmtautofmt.hxx>
#include <fchrfmt.hxx>

#include <strings.hrc>

using namespace ::com::sun::star;

// INSERT

boost::optional<OUString> SwUndoInsert::GetTextFromDoc() const
{
    boost::optional<OUString> aResult;

    SwNodeIndex aNd( pDoc->GetNodes(), nNode);
    SwContentNode* pCNd = aNd.GetNode().GetContentNode();

    if( pCNd->IsTextNode() )
    {
        OUString sText = pCNd->GetTextNode()->GetText();

        sal_Int32 nStart = nContent-nLen;
        sal_Int32 nLength = nLen;

        if (nStart < 0)
        {
            nLength += nStart;
            nStart = 0;
        }

        aResult = sText.copy(nStart, nLength);
    }

    return aResult;
}

void SwUndoInsert::Init(const SwNodeIndex & rNd)
{
    // consider Redline
    pDoc = rNd.GetNode().GetDoc();
    if( pDoc->getIDocumentRedlineAccess().IsRedlineOn() )
    {
        pRedlData.reset( new SwRedlineData( nsRedlineType_t::REDLINE_INSERT,
                                       pDoc->getIDocumentRedlineAccess().GetRedlineAuthor() ) );
        SetRedlineFlags( pDoc->getIDocumentRedlineAccess().GetRedlineFlags() );
    }

    maUndoText = GetTextFromDoc();

    bCacheComment = false;
}

SwUndoInsert::SwUndoInsert( const SwNodeIndex& rNd, sal_Int32 nCnt,
            sal_Int32 nL,
            const SwInsertFlags nInsertFlags,
            bool bWDelim )
    : SwUndo(SwUndoId::TYPING, rNd.GetNode().GetDoc()),
        nNode( rNd.GetIndex() ), nContent(nCnt), nLen(nL),
        bIsWordDelim( bWDelim ), bIsAppend( false )
    , m_bWithRsid(false)
    , m_nInsertFlags(nInsertFlags)
{
    Init(rNd);
}

SwUndoInsert::SwUndoInsert( const SwNodeIndex& rNd )
    : SwUndo(SwUndoId::SPLITNODE, rNd.GetNode().GetDoc()),
        nNode( rNd.GetIndex() ), nContent(0), nLen(1),
        bIsWordDelim( false ), bIsAppend( true )
    , m_bWithRsid(false)
    , m_nInsertFlags(SwInsertFlags::EMPTYEXPAND)
{
    Init(rNd);
}

// Check if the next Insert can be combined with the current one. If so
// change the length and InsPos. As a result, SwDoc::Inser will not add a
// new object into the Undo list.

bool SwUndoInsert::CanGrouping( sal_Unicode cIns )
{
    if( !bIsAppend && bIsWordDelim ==
        !GetAppCharClass().isLetterNumeric( OUString( cIns )) )
    {
        nLen++;
        nContent++;

        if (maUndoText)
            (*maUndoText) += OUStringLiteral1(cIns);

        return true;
    }
    return false;
}

bool SwUndoInsert::CanGrouping( const SwPosition& rPos )
{
    bool bRet = false;
    if( nNode == rPos.nNode.GetIndex() &&
        nContent == rPos.nContent.GetIndex() )
    {
        // consider Redline
        SwDoc& rDoc = *rPos.nNode.GetNode().GetDoc();
        if( ( ~RedlineFlags::ShowMask & rDoc.getIDocumentRedlineAccess().GetRedlineFlags() ) ==
            ( ~RedlineFlags::ShowMask & GetRedlineFlags() ) )
        {
            bRet = true;

            // than there is or was still an active Redline:
            // Check if there is another Redline at the InsPosition. If the
            // same exists only once, it can be combined.
            const SwRedlineTable& rTable = rDoc.getIDocumentRedlineAccess().GetRedlineTable();
            if( !rTable.empty() )
            {
                SwRedlineData aRData( nsRedlineType_t::REDLINE_INSERT, rDoc.getIDocumentRedlineAccess().GetRedlineAuthor() );
                const SwIndexReg* pIReg = rPos.nContent.GetIdxReg();
                for(SwRangeRedline* pRedl : rTable)
                {
                    SwIndex* pIdx = &pRedl->End()->nContent;
                    if( pIReg == pIdx->GetIdxReg() &&
                        nContent == pIdx->GetIndex() )
                    {
                        if( !pRedl->HasMark() || !pRedlData ||
                            *pRedl != *pRedlData || *pRedl != aRData )
                        {
                            bRet = false;
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
    else     // the inserted text
    {
        maText.reset();
    }
    pRedlData.reset();
}

void SwUndoInsert::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pTmpDoc = & rContext.GetDoc();
    SwPaM *const pPam(& rContext.GetCursorSupplier().CreateNewShellCursor());

    if( bIsAppend )
    {
        pPam->GetPoint()->nNode = nNode;

        if( IDocumentRedlineAccess::IsRedlineOn( GetRedlineFlags() ))
        {
            pPam->GetPoint()->nContent.Assign( pPam->GetContentNode(), 0 );
            pPam->SetMark();
            pPam->Move( fnMoveBackward );
            pPam->Exchange();
            pTmpDoc->getIDocumentRedlineAccess().DeleteRedline( *pPam, true, USHRT_MAX );
        }
        pPam->DeleteMark();
        pTmpDoc->getIDocumentContentOperations().DelFullPara( *pPam );
        pPam->GetPoint()->nContent.Assign( pPam->GetContentNode(), 0 );
    }
    else
    {
        sal_uLong nNd = nNode;
        sal_Int32 nCnt = nContent;
        if( nLen )
        {
            SwNodeIndex aNd( pTmpDoc->GetNodes(), nNode);
            SwContentNode* pCNd = aNd.GetNode().GetContentNode();
            SwPaM aPaM( *pCNd, nContent );

            aPaM.SetMark();

            SwTextNode * const pTextNode( pCNd->GetTextNode() );
            if ( pTextNode )
            {
                aPaM.GetPoint()->nContent -= nLen;
                if( IDocumentRedlineAccess::IsRedlineOn( GetRedlineFlags() ))
                    pTmpDoc->getIDocumentRedlineAccess().DeleteRedline( aPaM, true, USHRT_MAX );
                if (m_bWithRsid)
                {
                    // RSID was added: remove any CHARFMT/AUTOFMT that may be
                    // set on the deleted text; EraseText will leave empty
                    // ones behind otherwise
                    pTextNode->DeleteAttributes(RES_TXTATR_AUTOFMT,
                        aPaM.GetPoint()->nContent.GetIndex(),
                        aPaM.GetMark()->nContent.GetIndex());
                    pTextNode->DeleteAttributes(RES_TXTATR_CHARFMT,
                        aPaM.GetPoint()->nContent.GetIndex(),
                        aPaM.GetMark()->nContent.GetIndex());
                }
                RemoveIdxFromRange( aPaM, false );
                maText = pTextNode->GetText().copy(nContent-nLen, nLen);
                pTextNode->EraseText( aPaM.GetPoint()->nContent, nLen );
            }
            else                // otherwise Graphics/OLE/Text/...
            {
                aPaM.Move(fnMoveBackward);
                if( IDocumentRedlineAccess::IsRedlineOn( GetRedlineFlags() ))
                    pTmpDoc->getIDocumentRedlineAccess().DeleteRedline( aPaM, true, USHRT_MAX );
                RemoveIdxFromRange( aPaM, false );
            }

            nNd = aPaM.GetPoint()->nNode.GetIndex();
            nCnt = aPaM.GetPoint()->nContent.GetIndex();

            if (!maText)
            {
                m_pUndoNodeIndex.reset(
                        new SwNodeIndex(pDoc->GetNodes().GetEndOfContent()));
                MoveToUndoNds(aPaM, m_pUndoNodeIndex.get());
            }
            nNode = aPaM.GetPoint()->nNode.GetIndex();
            nContent = aPaM.GetPoint()->nContent.GetIndex();
        }

        // set cursor to Undo range
        pPam->DeleteMark();

        pPam->GetPoint()->nNode = nNd;
        pPam->GetPoint()->nContent.Assign(
                pPam->GetPoint()->nNode.GetNode().GetContentNode(), nCnt );
    }

    maUndoText.reset();
}

void SwUndoInsert::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pTmpDoc = & rContext.GetDoc();
    SwPaM *const pPam(& rContext.GetCursorSupplier().CreateNewShellCursor());
    pPam->DeleteMark();

    if( bIsAppend )
    {
        pPam->GetPoint()->nNode = nNode - 1;
        pTmpDoc->getIDocumentContentOperations().AppendTextNode( *pPam->GetPoint() );

        pPam->SetMark();
        pPam->Move( fnMoveBackward );
        pPam->Exchange();

        if( pRedlData && IDocumentRedlineAccess::IsRedlineOn( GetRedlineFlags() ))
        {
            RedlineFlags eOld = pTmpDoc->getIDocumentRedlineAccess().GetRedlineFlags();
            pTmpDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern(eOld & ~RedlineFlags::Ignore);
            pTmpDoc->getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( *pRedlData, *pPam ), true);
            pTmpDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
        }
        else if( !( RedlineFlags::Ignore & GetRedlineFlags() ) &&
                !pTmpDoc->getIDocumentRedlineAccess().GetRedlineTable().empty() )
            pTmpDoc->getIDocumentRedlineAccess().SplitRedline( *pPam );

        pPam->DeleteMark();
    }
    else
    {
        pPam->GetPoint()->nNode = nNode;
        SwContentNode *const pCNd =
            pPam->GetPoint()->nNode.GetNode().GetContentNode();
        pPam->GetPoint()->nContent.Assign( pCNd, nContent );

        if( nLen )
        {
            const bool bMvBkwrd = MovePtBackward( *pPam );

            if (maText)
            {
                SwTextNode *const pTextNode = pCNd->GetTextNode();
                OSL_ENSURE( pTextNode, "where is my textnode ?" );
                OUString const ins(
                    pTextNode->InsertText( *maText, pPam->GetMark()->nContent,
                    m_nInsertFlags) );
                assert(ins.getLength() == maText->getLength()); // must succeed
                maText.reset();
                if (m_bWithRsid) // re-insert RSID
                {
                    SwPaM pam(*pPam->GetMark(), nullptr); // mark -> point
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
            nContent = pPam->GetMark()->nContent.GetIndex();

            MovePtForward( *pPam, bMvBkwrd );
            pPam->Exchange();
            if( pRedlData && IDocumentRedlineAccess::IsRedlineOn( GetRedlineFlags() ))
            {
                RedlineFlags eOld = pTmpDoc->getIDocumentRedlineAccess().GetRedlineFlags();
                pTmpDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern(eOld & ~RedlineFlags::Ignore);
                pTmpDoc->getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( *pRedlData,
                                            *pPam ), true);
                pTmpDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
            }
            else if( !( RedlineFlags::Ignore & GetRedlineFlags() ) &&
                    !pTmpDoc->getIDocumentRedlineAccess().GetRedlineTable().empty() )
                pTmpDoc->getIDocumentRedlineAccess().SplitRedline(*pPam);
        }
    }

    maUndoText = GetTextFromDoc();
}

void SwUndoInsert::RepeatImpl(::sw::RepeatContext & rContext)
{
    if( !nLen )
        return;

    SwDoc & rDoc = rContext.GetDoc();
    SwNodeIndex aNd( rDoc.GetNodes(), nNode );
    SwContentNode* pCNd = aNd.GetNode().GetContentNode();

    if( !bIsAppend && 1 == nLen )       // >1 than always Text, otherwise Graphics/OLE/Text/...
    {
        SwPaM aPaM( *pCNd, nContent );
        aPaM.SetMark();
        aPaM.Move(fnMoveBackward);
        pCNd = aPaM.GetContentNode();
    }

// What happens with the possible selected range ???

    switch( pCNd->GetNodeType() )
    {
    case SwNodeType::Text:
        if( bIsAppend )
        {
            rDoc.getIDocumentContentOperations().AppendTextNode( *rContext.GetRepeatPaM().GetPoint() );
        }
        else
        {
            OUString const aText( pCNd->GetTextNode()->GetText() );
            ::sw::GroupUndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());
            rDoc.getIDocumentContentOperations().InsertString( rContext.GetRepeatPaM(),
                aText.copy(nContent - nLen, nLen) );
        }
        break;
    case SwNodeType::Grf:
        {
            SwGrfNode* pGrfNd = static_cast<SwGrfNode*>(pCNd);
            OUString sFile;
            OUString sFilter;
            if( pGrfNd->IsGrfLink() )
                pGrfNd->GetFileFilterNms( &sFile, &sFilter );

            rDoc.getIDocumentContentOperations().InsertGraphic(
                                rContext.GetRepeatPaM(), sFile, sFilter,
                                &pGrfNd->GetGrf(),
                                nullptr/* Graphics collection*/, nullptr, nullptr );
        }
        break;

    case SwNodeType::Ole:
        {
            // StarView does not yet provide an option to copy a StarOBJ
            tools::SvRef<SotStorage> aRef = new SotStorage( OUString() );
            SwOLEObj& rSwOLE = static_cast<SwOLENode*>(pCNd)->GetOLEObj();

            // temporary storage until object is inserted
            // TODO/MBA: seems that here a physical copy is done - not as in drawing layer! Testing!
            // TODO/LATER: Copying through the container would copy the replacement image as well
            comphelper::EmbeddedObjectContainer aCnt;
            OUString aName = aCnt.CreateUniqueObjectName();
            if (aCnt.StoreEmbeddedObject(rSwOLE.GetOleRef(), aName, true, OUString(), OUString()))
            {
                uno::Reference < embed::XEmbeddedObject > aNew = aCnt.GetEmbeddedObject( aName );
                rDoc.getIDocumentContentOperations().InsertEmbObject(
                    rContext.GetRepeatPaM(),
                    svt::EmbeddedObjectRef( aNew,
                        static_cast<SwOLENode*>(pCNd)->GetAspect() ),
                    nullptr );
            }

            break;
        }

    default: break;
    }
}

SwRewriter SwUndoInsert::GetRewriter() const
{
    SwRewriter aResult;
    boost::optional<OUString> aStr;
    bool bDone = false;

    if (maText)
        aStr = maText;
    else if (maUndoText)
        aStr = maUndoText;

    if (aStr)
    {
        OUString aString = ShortenString(DenoteSpecialCharacters(*aStr),
                                       nUndoStringLength,
                                       SwResId(STR_LDOTS));

        aResult.AddRule(UndoArg1, aString);

        bDone = true;
    }

    if ( ! bDone )
    {
        aResult.AddRule(UndoArg1, "??");
    }

    return aResult;
}

class SwUndoReplace::Impl
    : private SwUndoSaveContent
{
    OUString m_sOld;
    OUString const m_sIns;
    sal_uLong m_nSttNd, m_nEndNd, m_nOffset;
    sal_Int32 m_nSttCnt, m_nEndCnt, m_nSetPos, m_nSelEnd;
    bool m_bSplitNext : 1;
    bool const m_bRegExp : 1;
    // metadata references for paragraph and following para (if m_bSplitNext)
    std::shared_ptr< ::sfx2::MetadatableUndo > m_pMetadataUndoStart;
    std::shared_ptr< ::sfx2::MetadatableUndo > m_pMetadataUndoEnd;

public:
    Impl(SwPaM const& rPam, OUString const& rIns, bool const bRegExp);
    virtual ~Impl()
    {
    }

    void UndoImpl( ::sw::UndoRedoContext & );
    void RedoImpl( ::sw::UndoRedoContext & );

    void SetEnd(SwPaM const& rPam);

    OUString const& GetOld() const { return m_sOld; }
    OUString const& GetIns() const { return m_sIns; }
};

SwUndoReplace::SwUndoReplace(SwPaM const& rPam,
        OUString const& rIns, bool const bRegExp)
    : SwUndo( SwUndoId::REPLACE, rPam.GetDoc() )
    , m_pImpl(std::make_unique<Impl>(rPam, rIns, bRegExp))
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
        aResult.AddRule(UndoArg2, SwResId(STR_OCCURRENCES_OF));

        OUString aTmpStr;
        aTmpStr += SwResId(STR_START_QUOTE);
        aTmpStr += ShortenString(sOld, nUndoStringLength,
                                 SwResId(STR_LDOTS));
        aTmpStr += SwResId(STR_END_QUOTE);
        aResult.AddRule(UndoArg3, aTmpStr);
    }
    else if (1 == occurrences)
    {
        {
            OUString aTmpStr;

            aTmpStr += SwResId(STR_START_QUOTE);
            // #i33488 #
            aTmpStr += ShortenString(sOld, nUndoStringLength,
                                     SwResId(STR_LDOTS));
            aTmpStr += SwResId(STR_END_QUOTE);
            aResult.AddRule(UndoArg1, aTmpStr);
        }

        aResult.AddRule(UndoArg2, SwResId(STR_YIELDS));

        {
            OUString aTmpStr;

            aTmpStr += SwResId(STR_START_QUOTE);
            // #i33488 #
            aTmpStr += ShortenString(sNew, nUndoStringLength,
                                     SwResId(STR_LDOTS));
            aTmpStr += SwResId(STR_END_QUOTE);
            aResult.AddRule(UndoArg3, aTmpStr);
        }
    }

    return aResult;
}

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

    SwTextNode* pNd = pStt->nNode.GetNode().GetTextNode();
    OSL_ENSURE( pNd, "Dude, where's my TextNode?" );

    pHistory.reset( new SwHistory );
    DelContentIndex( *rPam.GetMark(), *rPam.GetPoint() );

    m_nSetPos = pHistory->Count();

    sal_uLong nNewPos = pStt->nNode.GetIndex();
    m_nOffset = m_nSttNd - nNewPos;

    if ( pNd->GetpSwpHints() )
    {
        pHistory->CopyAttr( pNd->GetpSwpHints(), nNewPos, 0,
                            pNd->GetText().getLength(), true );
    }

    if ( m_bSplitNext )
    {
        if( pNd->HasSwAttrSet() )
            pHistory->CopyFormatAttr( *pNd->GetpSwAttrSet(), nNewPos );
        pHistory->Add( pNd->GetTextColl(), nNewPos, SwNodeType::Text );

        SwTextNode* pNext = pEnd->nNode.GetNode().GetTextNode();
        sal_uLong nTmp = pNext->GetIndex();
        pHistory->CopyAttr( pNext->GetpSwpHints(), nTmp, 0,
                            pNext->GetText().getLength(), true );
        if( pNext->HasSwAttrSet() )
            pHistory->CopyFormatAttr( *pNext->GetpSwAttrSet(), nTmp );
        pHistory->Add( pNext->GetTextColl(),nTmp, SwNodeType::Text );
        // METADATA: store
        m_pMetadataUndoStart = pNd  ->CreateUndo();
        m_pMetadataUndoEnd   = pNext->CreateUndo();
    }

    if( !pHistory->Count() )
    {
        pHistory.reset();
    }

    const sal_Int32 nECnt = m_bSplitNext ? pNd->GetText().getLength()
        : pEnd->nContent.GetIndex();
    m_sOld = pNd->GetText().copy( m_nSttCnt, nECnt - m_nSttCnt );
}

void SwUndoReplace::Impl::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    SwPaM & rPam(rContext.GetCursorSupplier().CreateNewShellCursor());
    rPam.DeleteMark();

    SwTextNode* pNd = pDoc->GetNodes()[ m_nSttNd - m_nOffset ]->GetTextNode();
    OSL_ENSURE( pNd, "Dude, where's my TextNode?" );

    SwAutoCorrExceptWord* pACEWord = pDoc->GetAutoCorrExceptWord();
    if( pACEWord )
    {
        if ((1 == m_sIns.getLength()) && (1 == m_sOld.getLength()))
        {
            SwPosition aPos( *pNd ); aPos.nContent.Assign( pNd, m_nSttCnt );
            pACEWord->CheckChar( aPos, m_sOld[ 0 ] );
        }
        pDoc->SetAutoCorrExceptWord( nullptr );
    }

    SwIndex aIdx( pNd, m_nSttCnt );
    // don't look at m_sIns for deletion, maybe it was not completely inserted
    {
        rPam.GetPoint()->nNode = *pNd;
        rPam.GetPoint()->nContent.Assign( pNd, m_nSttCnt );
        rPam.SetMark();
        rPam.GetPoint()->nNode = m_nEndNd - m_nOffset;
        rPam.GetPoint()->nContent.Assign( rPam.GetContentNode(), m_nEndCnt );
        // move it out of the way so it is not registered at deleted node
        aIdx.Assign(nullptr, 0);

        pDoc->getIDocumentContentOperations().DeleteAndJoin( rPam );
        rPam.DeleteMark();
        pNd = rPam.GetNode().GetTextNode();
        OSL_ENSURE( pNd, "Dude, where's my TextNode?" );
        aIdx.Assign( pNd, m_nSttCnt );
    }

    if( m_bSplitNext )
    {
        SwPosition aPos( *pNd, aIdx );
        pDoc->getIDocumentContentOperations().SplitNode( aPos, false );
        pNd->RestoreMetadata(m_pMetadataUndoEnd);
        pNd = pDoc->GetNodes()[ m_nSttNd - m_nOffset ]->GetTextNode();
        aIdx.Assign( pNd, m_nSttCnt );
        // METADATA: restore
        pNd->RestoreMetadata(m_pMetadataUndoStart);
    }

    if (!m_sOld.isEmpty())
    {
        OUString const ins( pNd->InsertText( m_sOld, aIdx ) );
        assert(ins.getLength() == m_sOld.getLength()); // must succeed
        (void) ins;
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
                aHstr.Move( 0, pHistory.get(), m_nSetPos );
                pHistory->Rollback( pDoc );
                pHistory->Move( 0, &aHstr );
            }
            else
            {
                pHistory->Rollback( pDoc );
                pHistory.reset();
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

    SwTextNode* pNd = rPam.GetPoint()->nNode.GetNode().GetTextNode();
    OSL_ENSURE( pNd, "Dude, where's my TextNode?" );
    rPam.GetPoint()->nContent.Assign( pNd, m_nSttCnt );
    rPam.SetMark();
    if( m_bSplitNext )
    {
        rPam.GetPoint()->nNode = m_nSttNd + 1;
        pNd = rPam.GetPoint()->nNode.GetNode().GetTextNode();
    }
    rPam.GetPoint()->nContent.Assign( pNd, m_nSelEnd );

    if( pHistory )
    {
        auto xSave = std::make_unique<SwHistory>();
        std::swap(pHistory, xSave);

        DelContentIndex( *rPam.GetMark(), *rPam.GetPoint() );
        m_nSetPos = pHistory->Count();

        std::swap(xSave, pHistory);
        pHistory->Move(0, xSave.get());
    }
    else
    {
        pHistory.reset( new SwHistory );
        DelContentIndex( *rPam.GetMark(), *rPam.GetPoint() );
        m_nSetPos = pHistory->Count();
        if( !m_nSetPos )
        {
            pHistory.reset();
        }
    }

    rDoc.getIDocumentContentOperations().ReplaceRange( rPam, m_sIns, m_bRegExp );
    rPam.DeleteMark();
}

void SwUndoReplace::Impl::SetEnd(SwPaM const& rPam)
{
    const SwPosition* pEnd = rPam.End();
    m_nEndNd = m_nOffset + pEnd->nNode.GetIndex();
    m_nEndCnt = pEnd->nContent.GetIndex();
}

SwUndoReRead::SwUndoReRead( const SwPaM& rPam, const SwGrfNode& rGrfNd )
    : SwUndo( SwUndoId::REREAD, rPam.GetDoc() ), nPos( rPam.GetPoint()->nNode.GetIndex() )
{
    SaveGraphicData( rGrfNd );
}

SwUndoReRead::~SwUndoReRead()
{
}

void SwUndoReRead::SetAndSave(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwGrfNode* pGrfNd = rDoc.GetNodes()[ nPos ]->GetGrfNode();

    if( !pGrfNd )
        return ;

    // cache the old values
    std::unique_ptr<Graphic> pOldGrf( pGrf ? new Graphic(*pGrf) : nullptr);
    boost::optional<OUString> aOldNm = maNm;
    boost::optional<OUString> aOldFltr = maFltr;
    MirrorGraph nOldMirr = nMirr;
    // since all of them are cleared/modified by SaveGraphicData:
    SaveGraphicData( *pGrfNd );

    if( aOldNm )
    {
        pGrfNd->ReRead( *aOldNm, maFltr ? *maFltr : OUString() );
    }
    else
    {
        pGrfNd->ReRead( OUString(), OUString(), pOldGrf.get() );
    }

    if( MirrorGraph::Dont != nOldMirr )
        pGrfNd->SetAttr( SwMirrorGrf() );

    rContext.SetSelections(pGrfNd->GetFlyFormat(), nullptr);
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
        maNm = OUString();
        maFltr = OUString();
        rGrfNd.GetFileFilterNms(&*maNm, &*maFltr);
        pGrf.reset();
    }
    else
    {
        pGrf.reset( new Graphic( rGrfNd.GetGrf(true) ) );
        maNm.reset();
        maFltr.reset();
    }
    nMirr = rGrfNd.GetSwAttrSet().GetMirrorGrf().GetValue();
}

SwUndoInsertLabel::SwUndoInsertLabel( const SwLabelType eTyp,
                                      const OUString &rText,
                                      const OUString& rSeparator,
                                      const OUString& rNumberSeparator,
                                      const bool bBef,
                                      const sal_uInt16 nInitId,
                                      const OUString& rCharacterStyle,
                                      const bool bCpyBorder,
                                      const SwDoc* pDoc )
    : SwUndo( SwUndoId::INSERTLABEL, pDoc ),
      sText( rText ),
      sSeparator( rSeparator ),
      sNumberSeparator( rNumberSeparator ),//#i61007# order of captions
      sCharacterStyle( rCharacterStyle ),
      nFieldId( nInitId ),
      eType( eTyp ),
      nLayerId( 0 ),
      bBefore( bBef ),
      bCpyBrd( bCpyBorder )
{
    bUndoKeep = false;
    OBJECT.pUndoFly = nullptr;
    OBJECT.pUndoAttr = nullptr;
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
        SwFrameFormat* pFormat;
        SdrObject *pSdrObj = nullptr;
        if( OBJECT.pUndoAttr &&
            nullptr != (pFormat = static_cast<SwFrameFormat*>(OBJECT.pUndoAttr->GetFormat( rDoc ))) &&
            ( LTYPE_DRAW != eType ||
              nullptr != (pSdrObj = pFormat->FindSdrObject()) ) )
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
                pNd->GetTable().GetFrameFormat()->ResetFormatAttr( RES_KEEP );
        }
        SwPaM aPam( rDoc.GetNodes().GetEndOfContent() );
        aPam.GetPoint()->nNode = NODE.nNode;
        aPam.SetMark();
        aPam.GetPoint()->nNode = NODE.nNode + 1;
        NODE.pUndoInsNd = new SwUndoDelete( aPam, true );
    }
}

void SwUndoInsertLabel::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    if( LTYPE_OBJECT == eType || LTYPE_DRAW == eType )
    {
        OSL_ENSURE( OBJECT.pUndoAttr && OBJECT.pUndoFly, "Pointer not initialized" );
        SwFrameFormat* pFormat;
        SdrObject *pSdrObj = nullptr;
        if( OBJECT.pUndoAttr &&
            nullptr != (pFormat = static_cast<SwFrameFormat*>(OBJECT.pUndoAttr->GetFormat( rDoc ))) &&
            ( LTYPE_DRAW != eType ||
              nullptr != (pSdrObj = pFormat->FindSdrObject()) ) )
        {
            OBJECT.pUndoFly->RedoImpl(rContext);
            OBJECT.pUndoAttr->RedoImpl(rContext);
            if( LTYPE_DRAW == eType )
            {
                pSdrObj->SetLayer( nLayerId );
                if( pSdrObj->GetLayer() == rDoc.getIDocumentDrawModelAccess().GetHellId() )
                    pSdrObj->SetLayer( rDoc.getIDocumentDrawModelAccess().GetHeavenId() );
                // OD 02.07.2003 #108784#
                else if( pSdrObj->GetLayer() == rDoc.getIDocumentDrawModelAccess().GetInvisibleHellId() )
                    pSdrObj->SetLayer( rDoc.getIDocumentDrawModelAccess().GetInvisibleHeavenId() );
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
                pNd->GetTable().GetFrameFormat()->SetFormatAttr( SvxFormatKeepItem(true, RES_KEEP) );
        }
        NODE.pUndoInsNd->UndoImpl(rContext);
        delete NODE.pUndoInsNd;
        NODE.pUndoInsNd = nullptr;
    }
}

void SwUndoInsertLabel::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    const SwPosition& rPos = *rContext.GetRepeatPaM().GetPoint();

    sal_uLong nIdx = 0;

    SwContentNode* pCNd = rPos.nNode.GetNode().GetContentNode();
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
                SwFlyFrame* pFly;
                SwContentFrame *pCnt = pCNd->getLayoutFrame( rDoc.getIDocumentLayoutAccess().GetCurrentLayout() );
                if( pCnt && nullptr != ( pFly = pCnt->FindFlyFrame() ) )
                    nIdx = pFly->GetFormat()->GetContent().GetContentIdx()->GetIndex();
            }
            break;
        case LTYPE_DRAW:
            break;
        }

    if( nIdx )
    {
        rDoc.InsertLabel( eType, sText, sSeparator, sNumberSeparator, bBefore,
            nFieldId, nIdx, sCharacterStyle, bCpyBrd );
    }
}

SwRewriter SwUndoInsertLabel::GetRewriter() const
{
    return CreateRewriter(sText);
}

SwRewriter SwUndoInsertLabel::CreateRewriter(const OUString &rStr)
{
    SwRewriter aRewriter;

    OUString aTmpStr;

    if (!rStr.isEmpty())
    {
        aTmpStr += SwResId(STR_START_QUOTE);
        aTmpStr += ShortenString(rStr, nUndoStringLength,
                                 SwResId(STR_LDOTS));
        aTmpStr += SwResId(STR_END_QUOTE);
    }

    aRewriter.AddRule(UndoArg1, aTmpStr);

    return aRewriter;
}

void SwUndoInsertLabel::SetFlys( SwFrameFormat& rOldFly, SfxItemSet const & rChgSet,
                                SwFrameFormat& rNewFly )
{
    if( LTYPE_OBJECT == eType || LTYPE_DRAW == eType )
    {
        SwUndoFormatAttrHelper aTmp( rOldFly, false );
        rOldFly.SetFormatAttr( rChgSet );
        if ( aTmp.GetUndo() )
        {
            OBJECT.pUndoAttr = aTmp.ReleaseUndo().release();
        }
        OBJECT.pUndoFly = new SwUndoInsLayFormat( &rNewFly,0,0 );
    }
}

void SwUndoInsertLabel::SetDrawObj( SdrLayerID nLId )
{
    if( LTYPE_DRAW == eType )
    {
        nLayerId = nLId;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
