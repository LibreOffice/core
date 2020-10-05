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

#include <UndoOverwrite.hxx>
#include <unotools/charclass.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <comphelper/processfactory.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IShellCursorSupplier.hxx>
#include <swundo.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <UndoCore.hxx>
#include <rolbck.hxx>
#include <acorrect.hxx>
#include <docary.hxx>
#include <strings.hrc>

using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;

SwUndoOverwrite::SwUndoOverwrite( SwDoc& rDoc, SwPosition& rPos,
                                    sal_Unicode cIns )
    : SwUndo(SwUndoId::OVERWRITE, &rDoc),
      m_bGroup( false )
{
    SwTextNode *const pTextNd = rPos.nNode.GetNode().GetTextNode();
    assert(pTextNd);
    sal_Int32 const nTextNdLen = pTextNd->GetText().getLength();

    m_nStartNode = rPos.nNode.GetIndex();
    m_nStartContent = rPos.nContent.GetIndex();

    if( !rDoc.getIDocumentRedlineAccess().IsIgnoreRedline() && !rDoc.getIDocumentRedlineAccess().GetRedlineTable().empty() )
    {
        SwPaM aPam( rPos.nNode, rPos.nContent.GetIndex(),
                    rPos.nNode, rPos.nContent.GetIndex()+1 );
        m_pRedlSaveData.reset( new SwRedlineSaveDatas );
        if( !FillSaveData( aPam, *m_pRedlSaveData, false ))
        {
            m_pRedlSaveData.reset();
        }
        if (m_nStartContent < nTextNdLen)
        {
            rDoc.getIDocumentRedlineAccess().DeleteRedline(aPam, false, RedlineType::Any);
        }
    }

    m_bInsChar = true;
    if( m_nStartContent < nTextNdLen )     // no pure insert?
    {
        m_aDelStr += OUStringChar( pTextNd->GetText()[m_nStartContent] );
        if( !m_pHistory )
            m_pHistory.reset( new SwHistory );
        SwRegHistory aRHst( *pTextNd, m_pHistory.get() );
        m_pHistory->CopyAttr( pTextNd->GetpSwpHints(), m_nStartNode, 0,
                            nTextNdLen, false );
        ++rPos.nContent;
        m_bInsChar = false;
    }

    bool bOldExpFlg = pTextNd->IsIgnoreDontExpand();
    pTextNd->SetIgnoreDontExpand( true );

    pTextNd->InsertText( OUString(cIns), rPos.nContent,
            SwInsertFlags::EMPTYEXPAND );
    m_aInsStr += OUStringChar( cIns );

    if( !m_bInsChar )
    {
        const SwIndex aTmpIndex( rPos.nContent, -2 );
        pTextNd->EraseText( aTmpIndex, 1 );
    }
    pTextNd->SetIgnoreDontExpand( bOldExpFlg );

    m_bCacheComment = false;
}

SwUndoOverwrite::~SwUndoOverwrite()
{
}

bool SwUndoOverwrite::CanGrouping( SwDoc& rDoc, SwPosition& rPos,
                                    sal_Unicode cIns )
{
// What is with only inserted characters?

    // Only deletion of single chars can be combined.
    if( rPos.nNode != m_nStartNode || m_aInsStr.isEmpty()  ||
        ( !m_bGroup && m_aInsStr.getLength() != 1 ))
        return false;

    // Is the node a TextNode at all?
    SwTextNode * pDelTextNd = rPos.nNode.GetNode().GetTextNode();
    if( !pDelTextNd ||
        (pDelTextNd->GetText().getLength() != rPos.nContent.GetIndex() &&
            rPos.nContent.GetIndex() != ( m_nStartContent + m_aInsStr.getLength() )))
        return false;

    CharClass& rCC = GetAppCharClass();

    // ask the char that should be inserted
    if (( CH_TXTATR_BREAKWORD == cIns || CH_TXTATR_INWORD == cIns ) ||
        rCC.isLetterNumeric( OUString( cIns ), 0 ) !=
        rCC.isLetterNumeric( m_aInsStr, m_aInsStr.getLength()-1 ) )
        return false;

    if (!m_bInsChar && rPos.nContent.GetIndex() < pDelTextNd->GetText().getLength())
    {
        SwRedlineSaveDatas aTmpSav;
        SwPaM aPam( rPos.nNode, rPos.nContent.GetIndex(),
                    rPos.nNode, rPos.nContent.GetIndex()+1 );

        const bool bSaved = FillSaveData( aPam, aTmpSav, false );

        bool bOk = ( !m_pRedlSaveData && !bSaved ) ||
                   ( m_pRedlSaveData && bSaved &&
                        SwUndo::CanRedlineGroup( *m_pRedlSaveData, aTmpSav,
                            m_nStartContent > rPos.nContent.GetIndex() ));
        // aTmpSav.DeleteAndDestroyAll();
        if( !bOk )
            return false;

        rDoc.getIDocumentRedlineAccess().DeleteRedline( aPam, false, RedlineType::Any );
    }

    // both 'overwrites' can be combined so 'move' the corresponding character
    if( !m_bInsChar )
    {
        if (rPos.nContent.GetIndex() < pDelTextNd->GetText().getLength())
        {
            m_aDelStr += OUStringChar( pDelTextNd->GetText()[rPos.nContent.GetIndex()] );
            ++rPos.nContent;
        }
        else
            m_bInsChar = true;
    }

    bool bOldExpFlg = pDelTextNd->IsIgnoreDontExpand();
    pDelTextNd->SetIgnoreDontExpand( true );

    OUString const ins( pDelTextNd->InsertText(OUString(cIns), rPos.nContent,
            SwInsertFlags::EMPTYEXPAND) );
    assert(ins.getLength() == 1); // check in SwDoc::Overwrite => cannot fail
    (void) ins;
    m_aInsStr += OUStringChar( cIns );

    if( !m_bInsChar )
    {
        const SwIndex aTmpIndex( rPos.nContent, -2 );
        pDelTextNd->EraseText( aTmpIndex, 1 );
    }
    pDelTextNd->SetIgnoreDontExpand( bOldExpFlg );

    m_bGroup = true;
    return true;
}

void SwUndoOverwrite::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc& rDoc = rContext.GetDoc();
    SwPaM& rCurrentPam(rContext.GetCursorSupplier().CreateNewShellCursor());

    rCurrentPam.DeleteMark();
    rCurrentPam.GetPoint()->nNode = m_nStartNode;
    SwTextNode* pTextNd = rCurrentPam.GetNode().GetTextNode();
    assert(pTextNd);
    SwIndex& rIdx = rCurrentPam.GetPoint()->nContent;
    rIdx.Assign( pTextNd, m_nStartContent );

    SwAutoCorrExceptWord* pACEWord = rDoc.GetAutoCorrExceptWord();
    if( pACEWord )
    {
        if( 1 == m_aInsStr.getLength() && 1 == m_aDelStr.getLength() )
            pACEWord->CheckChar( *rCurrentPam.GetPoint(), m_aDelStr[0] );
        rDoc.SetAutoCorrExceptWord( nullptr );
    }

    // If there was not only an overwrite but also an insert, delete the surplus
    if( m_aInsStr.getLength() > m_aDelStr.getLength() )
    {
        rIdx += m_aDelStr.getLength();
        pTextNd->EraseText( rIdx, m_aInsStr.getLength() - m_aDelStr.getLength() );
        rIdx = m_nStartContent;
    }

    if( !m_aDelStr.isEmpty() )
    {
        bool bOldExpFlg = pTextNd->IsIgnoreDontExpand();
        pTextNd->SetIgnoreDontExpand( true );

        ++rIdx;
        for( sal_Int32 n = 0; n < m_aDelStr.getLength(); n++  )
        {
            // do it individually, to keep the attributes!
            OUString aTmpStr(m_aDelStr[n]);
            OUString const ins( pTextNd->InsertText(aTmpStr, rIdx) );
            assert(ins.getLength() == 1); // cannot fail
            (void) ins;
            rIdx -= 2;
            pTextNd->EraseText( rIdx, 1 );
            rIdx += 2;
        }
        pTextNd->SetIgnoreDontExpand( bOldExpFlg );
        --rIdx;
    }

    if( m_pHistory )
    {
        if( pTextNd->GetpSwpHints() )
            pTextNd->ClearSwpHintsArr( false );
        m_pHistory->TmpRollback( &rDoc, 0, false );
    }

    if( rCurrentPam.GetMark()->nContent.GetIndex() != m_nStartContent )
    {
        rCurrentPam.SetMark();
        rCurrentPam.GetMark()->nContent = m_nStartContent;
    }

    if( m_pRedlSaveData )
        SetSaveData( rDoc, *m_pRedlSaveData );
}

void SwUndoOverwrite::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwPaM& rCurrentPam = rContext.GetRepeatPaM();
    if( m_aInsStr.isEmpty() || rCurrentPam.HasMark() )
        return;

    SwDoc & rDoc = rContext.GetDoc();

    {
        ::sw::GroupUndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());
        rDoc.getIDocumentContentOperations().Overwrite(rCurrentPam, OUString(m_aInsStr[0]));
    }
    for( sal_Int32 n = 1; n < m_aInsStr.getLength(); ++n )
        rDoc.getIDocumentContentOperations().Overwrite(rCurrentPam, OUString(m_aInsStr[n]));
}

void SwUndoOverwrite::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc& rDoc = rContext.GetDoc();
    SwPaM& rCurrentPam(rContext.GetCursorSupplier().CreateNewShellCursor());

    rCurrentPam.DeleteMark();
    rCurrentPam.GetPoint()->nNode = m_nStartNode;
    SwTextNode* pTextNd = rCurrentPam.GetNode().GetTextNode();
    assert(pTextNd);
    SwIndex& rIdx = rCurrentPam.GetPoint()->nContent;

    if( m_pRedlSaveData )
    {
        rIdx.Assign( pTextNd, m_nStartContent );
        rCurrentPam.SetMark();
        rCurrentPam.GetMark()->nContent += m_aDelStr.getLength();
        rDoc.getIDocumentRedlineAccess().DeleteRedline( rCurrentPam, false, RedlineType::Any );
        rCurrentPam.DeleteMark();
    }
    rIdx.Assign( pTextNd, !m_aDelStr.isEmpty() ? m_nStartContent+1 : m_nStartContent );

    bool bOldExpFlg = pTextNd->IsIgnoreDontExpand();
    pTextNd->SetIgnoreDontExpand( true );

    for( sal_Int32 n = 0; n < m_aInsStr.getLength(); n++  )
    {
        // do it individually, to keep the attributes!
        OUString const ins(
                pTextNd->InsertText( OUString(m_aInsStr[n]), rIdx,
                SwInsertFlags::EMPTYEXPAND) );
        assert(ins.getLength() == 1); // cannot fail
        (void) ins;
        if( n < m_aDelStr.getLength() )
        {
            rIdx -= 2;
            pTextNd->EraseText( rIdx, 1 );
            rIdx += n+1 < m_aDelStr.getLength() ? 2 : 1;
        }
    }
    pTextNd->SetIgnoreDontExpand( bOldExpFlg );

    // get back old start position from UndoNodes array
    if( m_pHistory )
        m_pHistory->SetTmpEnd( m_pHistory->Count() );
    if( rCurrentPam.GetMark()->nContent.GetIndex() != m_nStartContent )
    {
        rCurrentPam.SetMark();
        rCurrentPam.GetMark()->nContent = m_nStartContent;
    }
}

SwRewriter SwUndoOverwrite::GetRewriter() const
{
    SwRewriter aResult;

    OUString aString = SwResId(STR_START_QUOTE) +
        ShortenString(m_aInsStr, nUndoStringLength, SwResId(STR_LDOTS)) +
        SwResId(STR_END_QUOTE);

    aResult.AddRule(UndoArg1, aString);

    return aResult;
}

struct UndoTransliterate_Data
{
    OUString        sText;
    std::unique_ptr<SwHistory> pHistory;
    std::unique_ptr<Sequence< sal_Int32 >> pOffsets;
    sal_uLong           nNdIdx;
    sal_Int32      nStart, nLen;

    UndoTransliterate_Data( sal_uLong nNd, sal_Int32 nStt, sal_Int32 nStrLen, const OUString& rText )
        : sText( rText ),
        nNdIdx( nNd ), nStart( nStt ), nLen( nStrLen )
    {}

    void SetChangeAtNode( SwDoc& rDoc );
};

SwUndoTransliterate::SwUndoTransliterate(
    const SwPaM& rPam,
    const utl::TransliterationWrapper& rTrans )
    : SwUndo( SwUndoId::TRANSLITERATE, &rPam.GetDoc() ), SwUndRng( rPam ), m_nType( rTrans.getType() )
{
}

SwUndoTransliterate::~SwUndoTransliterate()
{
}

void SwUndoTransliterate::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    // since the changes were added to the vector from the end of the string/node towards
    // the start, we need to revert them from the start towards the end now to keep the
    // offset information of the undo data in sync with the changing text.
    // Thus we need to iterate from the end of the vector to the start
    for (sal_Int32 i = m_aChanges.size() - 1; i >= 0;  --i)
        m_aChanges[i]->SetChangeAtNode( rDoc );

    AddUndoRedoPaM(rContext, true);
}

void SwUndoTransliterate::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwPaM & rPam( AddUndoRedoPaM(rContext) );
    DoTransliterate(rContext.GetDoc(), rPam);
}

void SwUndoTransliterate::RepeatImpl(::sw::RepeatContext & rContext)
{
    DoTransliterate(rContext.GetDoc(), rContext.GetRepeatPaM());
}

void SwUndoTransliterate::DoTransliterate(SwDoc & rDoc, SwPaM const & rPam)
{
    utl::TransliterationWrapper aTrans( ::comphelper::getProcessComponentContext(), m_nType );
    rDoc.getIDocumentContentOperations().TransliterateText( rPam, aTrans );
}

void SwUndoTransliterate::AddChanges( SwTextNode& rTNd,
                    sal_Int32 nStart, sal_Int32 nLen,
                    uno::Sequence <sal_Int32> const & rOffsets )
{
    long nOffsLen = rOffsets.getLength();
    UndoTransliterate_Data* pNew = new UndoTransliterate_Data(
                        rTNd.GetIndex(), nStart, static_cast<sal_Int32>(nOffsLen),
                        rTNd.GetText().copy(nStart, nLen));

    m_aChanges.push_back( std::unique_ptr<UndoTransliterate_Data>(pNew) );

    const sal_Int32* pOffsets = rOffsets.getConstArray();
    // where did we need less memory ?
    const sal_Int32* p = pOffsets;
    for( long n = 0; n < nOffsLen; ++n, ++p )
    if( *p != ( nStart + n ))
    {
        // create the Offset array
        pNew->pOffsets.reset( new Sequence <sal_Int32> ( nLen ) );
        sal_Int32* pIdx = pNew->pOffsets->getArray();
        p = pOffsets;
        long nMyOff, nNewVal = nStart;
        for( n = 0, nMyOff = nStart; n < nOffsLen; ++p, ++n, ++nMyOff )
        {
            if( *p < nMyOff )
            {
                // something is deleted
                nMyOff = *p;
                *(pIdx-1) = nNewVal++;
            }
            else if( *p > nMyOff )
            {
                for( ; *p > nMyOff; ++nMyOff )
                    *pIdx++ = nNewVal;
                --nMyOff;
                --n;
                --p;
            }
            else
                *pIdx++ = nNewVal++;
        }

        // and then we need to save the attributes/bookmarks
        // but this data must moved every time to the last in the chain!
        for (size_t i = 0; i + 1 < m_aChanges.size(); ++i)    // check all changes but not the current one
        {
            UndoTransliterate_Data* pD = m_aChanges[i].get();
            if( pD->nNdIdx == pNew->nNdIdx && pD->pHistory )
            {
                // same node and have a history?
                pNew->pHistory = std::move(pD->pHistory);
                break;          // more can't exist
            }
        }

        if( !pNew->pHistory )
        {
            pNew->pHistory.reset( new SwHistory );
            SwRegHistory aRHst( rTNd, pNew->pHistory.get() );
            pNew->pHistory->CopyAttr( rTNd.GetpSwpHints(),
                    pNew->nNdIdx, 0, rTNd.GetText().getLength(), false );
        }
        break;
    }
}

void UndoTransliterate_Data::SetChangeAtNode( SwDoc& rDoc )
{
    SwTextNode* pTNd = rDoc.GetNodes()[ nNdIdx ]->GetTextNode();
    if( !pTNd )
        return;

    Sequence <sal_Int32> aOffsets( pOffsets ? pOffsets->getLength() : nLen );
    if( pOffsets )
        aOffsets = *pOffsets;
    else
    {
        sal_Int32* p = aOffsets.getArray();
        for( sal_Int32 n = 0; n < nLen; ++n, ++p )
            *p = n + nStart;
    }
    pTNd->ReplaceTextOnly( nStart, nLen, sText, aOffsets );

    if( pHistory )
    {
        if( pTNd->GetpSwpHints() )
            pTNd->ClearSwpHintsArr( false );
        pHistory->TmpRollback( &rDoc, 0, false );
        pHistory->SetTmpEnd( pHistory->Count() );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
