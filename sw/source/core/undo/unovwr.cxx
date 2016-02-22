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
#include <tools/resid.hxx>
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
#include <comcore.hrc>
#include <undo.hrc>

using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;

SwUndoOverwrite::SwUndoOverwrite( SwDoc* pDoc, SwPosition& rPos,
                                    sal_Unicode cIns )
    : SwUndo(UNDO_OVERWRITE),
      pRedlSaveData( nullptr ), bGroup( false )
{
    if( !pDoc->getIDocumentRedlineAccess().IsIgnoreRedline() && !pDoc->getIDocumentRedlineAccess().GetRedlineTable().empty() )
    {
        SwPaM aPam( rPos.nNode, rPos.nContent.GetIndex(),
                    rPos.nNode, rPos.nContent.GetIndex()+1 );
        pRedlSaveData = new SwRedlineSaveDatas;
        if( !FillSaveData( aPam, *pRedlSaveData, false ))
        {
            delete pRedlSaveData;
            pRedlSaveData = nullptr;
        }
    }

    nSttNode = rPos.nNode.GetIndex();
    nSttContent = rPos.nContent.GetIndex();

    SwTextNode* pTextNd = rPos.nNode.GetNode().GetTextNode();
    OSL_ENSURE( pTextNd, "Overwrite not in a TextNode?" );

    bInsChar = true;
    sal_Int32 nTextNdLen = pTextNd->GetText().getLength();
    if( nSttContent < nTextNdLen )     // no pure insert?
    {
        aDelStr += OUString( pTextNd->GetText()[nSttContent] );
        if( !pHistory )
            pHistory = new SwHistory;
        SwRegHistory aRHst( *pTextNd, pHistory );
        pHistory->CopyAttr( pTextNd->GetpSwpHints(), nSttNode, 0,
                            nTextNdLen, false );
        ++rPos.nContent;
        bInsChar = false;
    }

    bool bOldExpFlg = pTextNd->IsIgnoreDontExpand();
    pTextNd->SetIgnoreDontExpand( true );

    pTextNd->InsertText( OUString(cIns), rPos.nContent,
            SwInsertFlags::EMPTYEXPAND );
    aInsStr += OUString( cIns );

    if( !bInsChar )
    {
        const SwIndex aTmpIndex( rPos.nContent, -2 );
        pTextNd->EraseText( aTmpIndex, 1 );
    }
    pTextNd->SetIgnoreDontExpand( bOldExpFlg );

    bCacheComment = false;
}

SwUndoOverwrite::~SwUndoOverwrite()
{
    delete pRedlSaveData;
}

bool SwUndoOverwrite::CanGrouping( SwDoc* pDoc, SwPosition& rPos,
                                    sal_Unicode cIns )
{
// What is with only inserted characters?

    // Only deletion of single chars can be combined.
    if( rPos.nNode != nSttNode || aInsStr.isEmpty()  ||
        ( !bGroup && aInsStr.getLength() != 1 ))
        return false;

    // Is the node a TextNode at all?
    SwTextNode * pDelTextNd = rPos.nNode.GetNode().GetTextNode();
    if( !pDelTextNd ||
        (pDelTextNd->GetText().getLength() != rPos.nContent.GetIndex() &&
            rPos.nContent.GetIndex() != ( nSttContent + aInsStr.getLength() )))
        return false;

    CharClass& rCC = GetAppCharClass();

    // ask the char that should be inserted
    if (( CH_TXTATR_BREAKWORD == cIns || CH_TXTATR_INWORD == cIns ) ||
        rCC.isLetterNumeric( OUString( cIns ), 0 ) !=
        rCC.isLetterNumeric( aInsStr, aInsStr.getLength()-1 ) )
        return false;

    {
        SwRedlineSaveDatas aTmpSav;
        SwPaM aPam( rPos.nNode, rPos.nContent.GetIndex(),
                    rPos.nNode, rPos.nContent.GetIndex()+1 );

        const bool bSaved = FillSaveData( aPam, aTmpSav, false );

        bool bOk = ( !pRedlSaveData && !bSaved ) ||
                   ( pRedlSaveData && bSaved &&
                        SwUndo::CanRedlineGroup( *pRedlSaveData, aTmpSav,
                            nSttContent > rPos.nContent.GetIndex() ));
        // aTmpSav.DeleteAndDestroyAll();
        if( !bOk )
            return false;

        pDoc->getIDocumentRedlineAccess().DeleteRedline( aPam, false, USHRT_MAX );
    }

    // both 'overwrites' can be combined so 'move' the corresponding character
    if( !bInsChar )
    {
        if (rPos.nContent.GetIndex() < pDelTextNd->GetText().getLength())
        {
            aDelStr += OUString( pDelTextNd->GetText()[rPos.nContent.GetIndex()] );
            ++rPos.nContent;
        }
        else
            bInsChar = true;
    }

    bool bOldExpFlg = pDelTextNd->IsIgnoreDontExpand();
    pDelTextNd->SetIgnoreDontExpand( true );

    OUString const ins( pDelTextNd->InsertText(OUString(cIns), rPos.nContent,
            SwInsertFlags::EMPTYEXPAND) );
    assert(ins.getLength() == 1); // check in SwDoc::Overwrite => cannot fail
    (void) ins;
    aInsStr += OUString( cIns );

    if( !bInsChar )
    {
        const SwIndex aTmpIndex( rPos.nContent, -2 );
        pDelTextNd->EraseText( aTmpIndex, 1 );
    }
    pDelTextNd->SetIgnoreDontExpand( bOldExpFlg );

    bGroup = true;
    return true;
}

void SwUndoOverwrite::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    SwPaM *const pAktPam(& rContext.GetCursorSupplier().CreateNewShellCursor());

    pAktPam->DeleteMark();
    pAktPam->GetPoint()->nNode = nSttNode;
    SwTextNode* pTextNd = pAktPam->GetNode().GetTextNode();
    OSL_ENSURE( pTextNd, "Overwrite not in a TextNode?" );
    SwIndex& rIdx = pAktPam->GetPoint()->nContent;
    rIdx.Assign( pTextNd, nSttContent );

    SwAutoCorrExceptWord* pACEWord = pDoc->GetAutoCorrExceptWord();
    if( pACEWord )
    {
        if( 1 == aInsStr.getLength() && 1 == aDelStr.getLength() )
            pACEWord->CheckChar( *pAktPam->GetPoint(), aDelStr[0] );
        pDoc->SetAutoCorrExceptWord( nullptr );
    }

    // If there was not only a overwrite but also an insert, delete the surplus
    if( aInsStr.getLength() > aDelStr.getLength() )
    {
        rIdx += aDelStr.getLength();
        pTextNd->EraseText( rIdx, aInsStr.getLength() - aDelStr.getLength() );
        rIdx = nSttContent;
    }

    if( !aDelStr.isEmpty() )
    {
        bool bOldExpFlg = pTextNd->IsIgnoreDontExpand();
        pTextNd->SetIgnoreDontExpand( true );

        ++rIdx;
        for( sal_Int32 n = 0; n < aDelStr.getLength(); n++  )
        {
            // do it individually, to keep the attributes!
            OUString aTmpStr(aDelStr[n]);
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

    if( pHistory )
    {
        if( pTextNd->GetpSwpHints() )
            pTextNd->ClearSwpHintsArr( false );
        pHistory->TmpRollback( pDoc, 0, false );
    }

    if( pAktPam->GetMark()->nContent.GetIndex() != nSttContent )
    {
        pAktPam->SetMark();
        pAktPam->GetMark()->nContent = nSttContent;
    }

    if( pRedlSaveData )
        SetSaveData( *pDoc, *pRedlSaveData );
}

void SwUndoOverwrite::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwPaM *const pAktPam = & rContext.GetRepeatPaM();
    if( aInsStr.isEmpty() || pAktPam->HasMark() )
        return;

    SwDoc & rDoc = rContext.GetDoc();

    {
        ::sw::GroupUndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());
        rDoc.getIDocumentContentOperations().Overwrite(*pAktPam, OUString(aInsStr[0]));
    }
    for( sal_Int32 n = 1; n < aInsStr.getLength(); ++n )
        rDoc.getIDocumentContentOperations().Overwrite( *pAktPam, OUString(aInsStr[n]) );
}

void SwUndoOverwrite::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    SwPaM *const pAktPam(& rContext.GetCursorSupplier().CreateNewShellCursor());

    pAktPam->DeleteMark();
    pAktPam->GetPoint()->nNode = nSttNode;
    SwTextNode* pTextNd = pAktPam->GetNode().GetTextNode();
    OSL_ENSURE( pTextNd, "Overwrite not in TextNode?" );
    SwIndex& rIdx = pAktPam->GetPoint()->nContent;

    if( pRedlSaveData )
    {
        rIdx.Assign( pTextNd, nSttContent );
        pAktPam->SetMark();
        pAktPam->GetMark()->nContent += aInsStr.getLength();
        pDoc->getIDocumentRedlineAccess().DeleteRedline( *pAktPam, false, USHRT_MAX );
        pAktPam->DeleteMark();
    }
    rIdx.Assign( pTextNd, !aDelStr.isEmpty() ? nSttContent+1 : nSttContent );

    bool bOldExpFlg = pTextNd->IsIgnoreDontExpand();
    pTextNd->SetIgnoreDontExpand( true );

    for( sal_Int32 n = 0; n < aInsStr.getLength(); n++  )
    {
        // do it individually, to keep the attributes!
        OUString const ins(
                pTextNd->InsertText( OUString(aInsStr[n]), rIdx,
                SwInsertFlags::EMPTYEXPAND) );
        assert(ins.getLength() == 1); // cannot fail
        (void) ins;
        if( n < aDelStr.getLength() )
        {
            rIdx -= 2;
            pTextNd->EraseText( rIdx, 1 );
            rIdx += n+1 < aDelStr.getLength() ? 2 : 1;
        }
    }
    pTextNd->SetIgnoreDontExpand( bOldExpFlg );

    // get back old start position from UndoNodes array
    if( pHistory )
        pHistory->SetTmpEnd( pHistory->Count() );
    if( pAktPam->GetMark()->nContent.GetIndex() != nSttContent )
    {
        pAktPam->SetMark();
        pAktPam->GetMark()->nContent = nSttContent;
    }
}

SwRewriter SwUndoOverwrite::GetRewriter() const
{
    SwRewriter aResult;

    OUString aString;

    aString += SW_RES(STR_START_QUOTE);
    aString += ShortenString(aInsStr, nUndoStringLength,
                             OUString(SW_RES(STR_LDOTS)));
    aString += SW_RES(STR_END_QUOTE);

    aResult.AddRule(UndoArg1, aString);

    return aResult;
}

struct _UndoTransliterate_Data
{
    OUString        sText;
    SwHistory*      pHistory;
    Sequence< sal_Int32 >*  pOffsets;
    sal_uLong           nNdIdx;
    sal_Int32      nStart, nLen;

    _UndoTransliterate_Data( sal_uLong nNd, sal_Int32 nStt, sal_Int32 nStrLen, const OUString& rText )
        : sText( rText ), pHistory( nullptr ), pOffsets( nullptr ),
        nNdIdx( nNd ), nStart( nStt ), nLen( nStrLen )
    {}
    ~_UndoTransliterate_Data() { delete pOffsets; delete pHistory; }

    void SetChangeAtNode( SwDoc& rDoc );
};

SwUndoTransliterate::SwUndoTransliterate(
    const SwPaM& rPam,
    const utl::TransliterationWrapper& rTrans )
    : SwUndo( UNDO_TRANSLITERATE ), SwUndRng( rPam ), nType( rTrans.getType() )
{
}

SwUndoTransliterate::~SwUndoTransliterate()
{
    for (size_t i = 0; i < aChanges.size();  ++i)
        delete aChanges[i];
}

void SwUndoTransliterate::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    // since the changes were added to the vector from the end of the string/node towards
    // the start, we need to revert them from the start towards the end now to keep the
    // offset information of the undo data in sync with the changing text.
    // Thus we need to iterate from the end of the vector to the start
    for (sal_Int32 i = aChanges.size() - 1; i >= 0;  --i)
        aChanges[i]->SetChangeAtNode( rDoc );

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

void SwUndoTransliterate::DoTransliterate(SwDoc & rDoc, SwPaM & rPam)
{
    utl::TransliterationWrapper aTrans( ::comphelper::getProcessComponentContext(), nType );
    rDoc.getIDocumentContentOperations().TransliterateText( rPam, aTrans );
}

void SwUndoTransliterate::AddChanges( SwTextNode& rTNd,
                    sal_Int32 nStart, sal_Int32 nLen,
                    uno::Sequence <sal_Int32>& rOffsets )
{
    long nOffsLen = rOffsets.getLength();
    _UndoTransliterate_Data* pNew = new _UndoTransliterate_Data(
                        rTNd.GetIndex(), nStart, (sal_Int32)nOffsLen,
                        rTNd.GetText().copy(nStart, nLen));

    aChanges.push_back( pNew );

    const sal_Int32* pOffsets = rOffsets.getConstArray();
    // where did we need less memory ?
    const sal_Int32* p = pOffsets;
    for( long n = 0; n < nOffsLen; ++n, ++p )
    if( *p != ( nStart + n ))
    {
        // create the Offset array
        pNew->pOffsets = new Sequence <sal_Int32> ( nLen );
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
        for (size_t i = 0; i + 1 < aChanges.size(); ++i)    // check all changes but not the current one
        {
            _UndoTransliterate_Data* pD = aChanges[i];
            if( pD->nNdIdx == pNew->nNdIdx && pD->pHistory )
            {
                // same node and have a history?
                pNew->pHistory = pD->pHistory;
                pD->pHistory = nullptr;
                break;          // more can't exist
            }
        }

        if( !pNew->pHistory )
        {
            pNew->pHistory = new SwHistory;
            SwRegHistory aRHst( rTNd, pNew->pHistory );
            pNew->pHistory->CopyAttr( rTNd.GetpSwpHints(),
                    pNew->nNdIdx, 0, rTNd.GetText().getLength(), false );
        }
        break;
    }
}

void _UndoTransliterate_Data::SetChangeAtNode( SwDoc& rDoc )
{
    SwTextNode* pTNd = rDoc.GetNodes()[ nNdIdx ]->GetTextNode();
    if( pTNd )
    {
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
