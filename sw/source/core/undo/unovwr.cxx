/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <UndoOverwrite.hxx>
#include <tools/resid.hxx>
#include <unotools/charclass.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <comphelper/processfactory.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IShellCursorSupplier.hxx>
#include <swundo.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <UndoCore.hxx>
#include <rolbck.hxx>
#include <acorrect.hxx>
#include <docary.hxx>
#include <comcore.hrc> // #111827#
#include <undo.hrc>

using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;

SwUndoOverwrite::SwUndoOverwrite( SwDoc* pDoc, SwPosition& rPos,
                                    sal_Unicode cIns )
    : SwUndo(UNDO_OVERWRITE),
      pRedlSaveData( 0 ), bGroup( sal_False )
{
    if( !pDoc->IsIgnoreRedline() && !pDoc->GetRedlineTbl().empty() )
    {
        SwPaM aPam( rPos.nNode, rPos.nContent.GetIndex(),
                    rPos.nNode, rPos.nContent.GetIndex()+1 );
        pRedlSaveData = new SwRedlineSaveDatas;
        if( !FillSaveData( aPam, *pRedlSaveData, sal_False ))
            delete pRedlSaveData, pRedlSaveData = 0;
    }

    nSttNode = rPos.nNode.GetIndex();
    nSttCntnt = rPos.nContent.GetIndex();

    SwTxtNode* pTxtNd = rPos.nNode.GetNode().GetTxtNode();
    OSL_ENSURE( pTxtNd, "Overwrite not in a TextNode?" );

    bInsChar = sal_True;
    xub_StrLen nTxtNdLen = pTxtNd->GetTxt().Len();
    if( nSttCntnt < nTxtNdLen )     // no pure insert?
    {
        aDelStr.Insert( pTxtNd->GetTxt().GetChar( nSttCntnt ) );
        if( !pHistory )
            pHistory = new SwHistory;
        SwRegHistory aRHst( *pTxtNd, pHistory );
        pHistory->CopyAttr( pTxtNd->GetpSwpHints(), nSttNode, 0,
                            nTxtNdLen, false );
        rPos.nContent++;
        bInsChar = sal_False;
    }

    sal_Bool bOldExpFlg = pTxtNd->IsIgnoreDontExpand();
    pTxtNd->SetIgnoreDontExpand( sal_True );

    pTxtNd->InsertText( rtl::OUString(cIns), rPos.nContent,
            IDocumentContentOperations::INS_EMPTYEXPAND );
    aInsStr.Insert( cIns );

    if( !bInsChar )
    {
        const SwIndex aTmpIndex( rPos.nContent, -2 );
        pTxtNd->EraseText( aTmpIndex, 1 );
    }
    pTxtNd->SetIgnoreDontExpand( bOldExpFlg );

    bCacheComment = false;
}

SwUndoOverwrite::~SwUndoOverwrite()
{
    delete pRedlSaveData;
}

sal_Bool SwUndoOverwrite::CanGrouping( SwDoc* pDoc, SwPosition& rPos,
                                    sal_Unicode cIns )
{
// What is with only inserted characters?

    // Only deletion of single chars can be combined.
    if( rPos.nNode != nSttNode || !aInsStr.Len()  ||
        ( !bGroup && aInsStr.Len() != 1 ))
        return sal_False;

    // Is the node a TextNode at all?
    SwTxtNode * pDelTxtNd = rPos.nNode.GetNode().GetTxtNode();
    if( !pDelTxtNd ||
        ( pDelTxtNd->GetTxt().Len() != rPos.nContent.GetIndex() &&
            rPos.nContent.GetIndex() != ( nSttCntnt + aInsStr.Len() )))
        return sal_False;

    CharClass& rCC = GetAppCharClass();

    // ask the char that should be inserted
    if (( CH_TXTATR_BREAKWORD == cIns || CH_TXTATR_INWORD == cIns ) ||
        rCC.isLetterNumeric( rtl::OUString( cIns ), 0 ) !=
        rCC.isLetterNumeric( aInsStr, aInsStr.Len()-1 ) )
        return sal_False;

    {
        SwRedlineSaveDatas* pTmpSav = new SwRedlineSaveDatas;
        SwPaM aPam( rPos.nNode, rPos.nContent.GetIndex(),
                    rPos.nNode, rPos.nContent.GetIndex()+1 );

        if( !FillSaveData( aPam, *pTmpSav, sal_False ))
            delete pTmpSav, pTmpSav = 0;

        sal_Bool bOk = ( !pRedlSaveData && !pTmpSav ) ||
                   ( pRedlSaveData && pTmpSav &&
                        SwUndo::CanRedlineGroup( *pRedlSaveData, *pTmpSav,
                            nSttCntnt > rPos.nContent.GetIndex() ));
        delete pTmpSav;
        if( !bOk )
            return sal_False;

        pDoc->DeleteRedline( aPam, false, USHRT_MAX );
    }

    // both 'overwrites' can be combined so 'move' the corresponding character
    if( !bInsChar )
    {
        if( rPos.nContent.GetIndex() < pDelTxtNd->GetTxt().Len() )
        {
            aDelStr.Insert( pDelTxtNd->GetTxt().GetChar(rPos.nContent.GetIndex()) );
            rPos.nContent++;
        }
        else
            bInsChar = sal_True;
    }

    sal_Bool bOldExpFlg = pDelTxtNd->IsIgnoreDontExpand();
    pDelTxtNd->SetIgnoreDontExpand( sal_True );

    pDelTxtNd->InsertText( rtl::OUString(cIns), rPos.nContent,
            IDocumentContentOperations::INS_EMPTYEXPAND );
    aInsStr.Insert( cIns );

    if( !bInsChar )
    {
        const SwIndex aTmpIndex( rPos.nContent, -2 );
        pDelTxtNd->EraseText( aTmpIndex, 1 );
    }
    pDelTxtNd->SetIgnoreDontExpand( bOldExpFlg );

    bGroup = sal_True;
    return sal_True;
}

void SwUndoOverwrite::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    SwPaM *const pAktPam(& rContext.GetCursorSupplier().CreateNewShellCursor());

    pAktPam->DeleteMark();
    pAktPam->GetPoint()->nNode = nSttNode;
    SwTxtNode* pTxtNd = pAktPam->GetNode()->GetTxtNode();
    OSL_ENSURE( pTxtNd, "Overwrite not in a TextNode?" );
    SwIndex& rIdx = pAktPam->GetPoint()->nContent;
    rIdx.Assign( pTxtNd, nSttCntnt );

    SwAutoCorrExceptWord* pACEWord = pDoc->GetAutoCorrExceptWord();
    if( pACEWord )
    {
        if( 1 == aInsStr.Len() && 1 == aDelStr.Len() )
            pACEWord->CheckChar( *pAktPam->GetPoint(), aDelStr.GetChar( 0 ) );
        pDoc->SetAutoCorrExceptWord( 0 );
    }

    // If there was not only a overwrite but also an insert, delete the surplus
    if( aInsStr.Len() > aDelStr.Len() )
    {
        rIdx += aDelStr.Len();
        pTxtNd->EraseText( rIdx, aInsStr.Len() - aDelStr.Len() );
        rIdx = nSttCntnt;
    }

    if( aDelStr.Len() )
    {
        String aTmpStr = rtl::OUString('1');
        sal_Unicode* pTmpStr = aTmpStr.GetBufferAccess();

        sal_Bool bOldExpFlg = pTxtNd->IsIgnoreDontExpand();
        pTxtNd->SetIgnoreDontExpand( sal_True );

        ++rIdx;
        for( xub_StrLen n = 0; n < aDelStr.Len(); n++  )
        {
            // do it individually, to keep the attributes!
            *pTmpStr = aDelStr.GetChar( n );
            pTxtNd->InsertText( aTmpStr, rIdx /*???, SETATTR_NOTXTATRCHR*/ );
            rIdx -= 2;
            pTxtNd->EraseText( rIdx, 1 );
            rIdx += 2;
        }
        pTxtNd->SetIgnoreDontExpand( bOldExpFlg );
        rIdx--;
    }

    if( pHistory )
    {
        if( pTxtNd->GetpSwpHints() )
            pTxtNd->ClearSwpHintsArr( false );
        pHistory->TmpRollback( pDoc, 0, false );
    }

    if( pAktPam->GetMark()->nContent.GetIndex() != nSttCntnt )
    {
        pAktPam->SetMark();
        pAktPam->GetMark()->nContent = nSttCntnt;
    }

    if( pRedlSaveData )
        SetSaveData( *pDoc, *pRedlSaveData );
}

void SwUndoOverwrite::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwPaM *const pAktPam = & rContext.GetRepeatPaM();
    if( !aInsStr.Len() || pAktPam->HasMark() )
        return;

    SwDoc & rDoc = rContext.GetDoc();

    {
        ::sw::GroupUndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());
        rDoc.Overwrite(*pAktPam, rtl::OUString(aInsStr.GetChar(0)));
    }
    for( xub_StrLen n = 1; n < aInsStr.Len(); ++n )
        rDoc.Overwrite( *pAktPam, rtl::OUString(aInsStr.GetChar(n)) );
}

void SwUndoOverwrite::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    SwPaM *const pAktPam(& rContext.GetCursorSupplier().CreateNewShellCursor());

    pAktPam->DeleteMark();
    pAktPam->GetPoint()->nNode = nSttNode;
    SwTxtNode* pTxtNd = pAktPam->GetNode()->GetTxtNode();
    OSL_ENSURE( pTxtNd, "Overwrite not in TextNode?" );
    SwIndex& rIdx = pAktPam->GetPoint()->nContent;

    if( pRedlSaveData )
    {
        rIdx.Assign( pTxtNd, nSttCntnt );
        pAktPam->SetMark();
        pAktPam->GetMark()->nContent += aInsStr.Len();
        pDoc->DeleteRedline( *pAktPam, false, USHRT_MAX );
        pAktPam->DeleteMark();
    }
    rIdx.Assign( pTxtNd, aDelStr.Len() ? nSttCntnt+1 : nSttCntnt );

    sal_Bool bOldExpFlg = pTxtNd->IsIgnoreDontExpand();
    pTxtNd->SetIgnoreDontExpand( sal_True );

    for( xub_StrLen n = 0; n < aInsStr.Len(); n++  )
    {
        // do it individually, to keep the attributes!
        pTxtNd->InsertText( rtl::OUString(aInsStr.GetChar(n)), rIdx,
                IDocumentContentOperations::INS_EMPTYEXPAND );
        if( n < aDelStr.Len() )
        {
            rIdx -= 2;
            pTxtNd->EraseText( rIdx, 1 );
            rIdx += n+1 < aDelStr.Len() ? 2 : 1;
        }
    }
    pTxtNd->SetIgnoreDontExpand( bOldExpFlg );

    // get back old start position from UndoNodes array
    if( pHistory )
        pHistory->SetTmpEnd( pHistory->Count() );
    if( pAktPam->GetMark()->nContent.GetIndex() != nSttCntnt )
    {
        pAktPam->SetMark();
        pAktPam->GetMark()->nContent = nSttCntnt;
    }
}

SwRewriter SwUndoOverwrite::GetRewriter() const
{
    SwRewriter aResult;

    String aString;

    aString += String(SW_RES(STR_START_QUOTE));
    aString += ShortenString(aInsStr, nUndoStringLength,
                             String(SW_RES(STR_LDOTS)));
    aString += String(SW_RES(STR_END_QUOTE));

    aResult.AddRule(UndoArg1, aString);

    return aResult;
}

struct _UndoTransliterate_Data
{
    String          sText;
    SwHistory*      pHistory;
    Sequence< sal_Int32 >*  pOffsets;
    sal_uLong           nNdIdx;
    xub_StrLen      nStart, nLen;

    _UndoTransliterate_Data( sal_uLong nNd, xub_StrLen nStt, xub_StrLen nStrLen, const String& rTxt )
        : sText( rTxt ), pHistory( 0 ), pOffsets( 0 ),
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
    utl::TransliterationWrapper aTrans( ::comphelper::getProcessServiceFactory(), nType );
    rDoc.TransliterateText( rPam, aTrans );
}

void SwUndoTransliterate::AddChanges( SwTxtNode& rTNd,
                    xub_StrLen nStart, xub_StrLen nLen,
                    uno::Sequence <sal_Int32>& rOffsets )
{
    long nOffsLen = rOffsets.getLength();
    _UndoTransliterate_Data* pNew = new _UndoTransliterate_Data(
                        rTNd.GetIndex(), nStart, (xub_StrLen)nOffsLen,
                        rTNd.GetTxt().Copy( nStart, nLen ));

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
                pD->pHistory = 0;
                break;          // more can't exist
            }
        }

        if( !pNew->pHistory )
        {
            pNew->pHistory = new SwHistory;
            SwRegHistory aRHst( rTNd, pNew->pHistory );
            pNew->pHistory->CopyAttr( rTNd.GetpSwpHints(),
                    pNew->nNdIdx, 0, rTNd.GetTxt().Len(), false );
        }
        break;
    }
}

void _UndoTransliterate_Data::SetChangeAtNode( SwDoc& rDoc )
{
    SwTxtNode* pTNd = rDoc.GetNodes()[ nNdIdx ]->GetTxtNode();
    if( pTNd )
    {
        Sequence <sal_Int32> aOffsets( pOffsets ? pOffsets->getLength() : nLen );
        if( pOffsets )
            aOffsets = *pOffsets;
        else
        {
            sal_Int32* p = aOffsets.getArray();
            for( xub_StrLen n = 0; n < nLen; ++n, ++p )
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
