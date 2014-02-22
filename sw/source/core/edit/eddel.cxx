/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <hintids.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <editsh.hxx>
#include <cntfrm.hxx>
#include <pam.hxx>
#include <swundo.hxx>
#include <edimp.hxx>
#include <IMark.hxx>
#include <docary.hxx>
#include <SwRewriter.hxx>
#include <globals.hrc>

#include <comcore.hrc>
#include <list>

void SwEditShell::DeleteSel( SwPaM& rPam, sal_Bool* pUndo )
{
    bool bSelectAll = StartsWithTable() && ExtendedSelectedAll(/*bFootnotes =*/ false);
    
    if( !rPam.HasMark() || *rPam.GetPoint() == *rPam.GetMark())
        return;

    
    
    
    
    
    if( rPam.GetNode()->FindTableNode() &&
        rPam.GetNode()->StartOfSectionNode() !=
        rPam.GetNode(false)->StartOfSectionNode() && !bSelectAll )
    {
        
        if( pUndo && !*pUndo )
        {
            GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );
            *pUndo = sal_True;
        }
        SwPaM aDelPam( *rPam.Start() );
        const SwPosition* pEndSelPos = rPam.End();
        do {
            aDelPam.SetMark();
            SwNode* pNd = aDelPam.GetNode();
            const SwNode& rEndNd = *pNd->EndOfSectionNode();
            if( pEndSelPos->nNode.GetIndex() <= rEndNd.GetIndex() )
            {
                *aDelPam.GetPoint() = *pEndSelPos;
                pEndSelPos = 0;     
            }
            else
            {
                
                aDelPam.GetPoint()->nNode = rEndNd;
                aDelPam.Move( fnMoveBackward, fnGoCntnt );
            }
            
            if( !pNd->IsCntntNode() ||
                !pNd->IsInProtectSect() )
            {
                
                GetDoc()->DeleteAndJoin( aDelPam );
                SaveTblBoxCntnt( aDelPam.GetPoint() );
            }

            if( !pEndSelPos ) 
                break;
            aDelPam.DeleteMark();
            aDelPam.Move( fnMoveForward, fnGoCntnt ); 
        } while( pEndSelPos );
    }
    else
    {
        
        GetDoc()->DeleteAndJoin( rPam );
        SaveTblBoxCntnt( rPam.GetPoint() );
    }

    
    rPam.DeleteMark();
}

long SwEditShell::Delete()
{
    SET_CURR_SHELL( this );
    long nRet = 0;
    if( !HasReadonlySel() )
    {
        StartAllAction();

        sal_Bool bUndo = GetCrsr()->GetNext() != GetCrsr();
        if( bUndo ) 
        {
            SwRewriter aRewriter;
            aRewriter.AddRule(UndoArg1, SW_RESSTR(STR_MULTISEL));

            GetDoc()->GetIDocumentUndoRedo().StartUndo(UNDO_DELETE, &aRewriter);
        }

        FOREACHPAM_START(GetCrsr())
            DeleteSel( *PCURCRSR, &bUndo );
        FOREACHPAM_END()

        
        if( bUndo )
        {
            GetDoc()->GetIDocumentUndoRedo().EndUndo(UNDO_END, 0);
        }
        EndAllAction();
        nRet = 1;
    }
    return nRet;
}

long SwEditShell::Copy( SwEditShell* pDestShell )
{
    if( !pDestShell )
        pDestShell = this;

    SET_CURR_SHELL( pDestShell );

    
    std::list< boost::shared_ptr<SwPosition> > aInsertList;

    
    {
        SwPosition * pPos = 0;
        boost::shared_ptr<SwPosition> pInsertPos;
        sal_uInt16 nMove = 0;
        FOREACHPAM_START(GetCrsr())

            if( !pPos )
            {
                if( pDestShell == this )
                {
                    
                    PCURCRSR->DeleteMark();
                    pPos = (SwPosition*)PCURCRSR->GetPoint();
                    continue;
                }
                else
                    pPos = pDestShell->GetCrsr()->GetPoint();
            }
            if( IsBlockMode() )
            {   
                
                if( nMove )
                {
                    SwCursor aCrsr( *pPos, 0, false);
                    if( aCrsr.UpDown( sal_False, nMove, 0, 0 ) )
                    {
                        pInsertPos.reset( new SwPosition( *aCrsr.GetPoint() ) );
                        aInsertList.push_back( pInsertPos );
                    }
                }
                else
                    pInsertPos.reset( new SwPosition( *pPos ) );
                ++nMove;
            }
            SwPosition *pTmp = IsBlockMode() ? pInsertPos.get() : pPos;
            
            if( pDestShell->GetDoc() == GetDoc() &&
                *PCURCRSR->Start() <= *pTmp && *pTmp < *PCURCRSR->End() )
                return sal_False;
        FOREACHPAM_END()
    }

    pDestShell->StartAllAction();
    SwPosition *pPos = 0;
    sal_Bool bRet = sal_False;
    bool bFirstMove = true;
    SwNodeIndex aSttNdIdx( pDestShell->GetDoc()->GetNodes() );
    sal_Int32 nSttCntIdx = 0;
    
    std::list< boost::shared_ptr<SwPosition> >::iterator pNextInsert = aInsertList.begin();

    pDestShell->GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );
    FOREACHPAM_START(GetCrsr())

        if( !pPos )
        {
            if( pDestShell == this )
            {
                
                PCURCRSR->DeleteMark();
                pPos = (SwPosition*)PCURCRSR->GetPoint();
                continue;
            }
            else
                pPos = pDestShell->GetCrsr()->GetPoint();
        }
        if( !bFirstMove )
        {
            if( pNextInsert != aInsertList.end() )
            {
                pPos = pNextInsert->get();
                ++pNextInsert;
            }
            else if( IsBlockMode() )
                GetDoc()->SplitNode( *pPos, false );
        }

        
        if( !PCURCRSR->HasMark() || *PCURCRSR->GetPoint() == *PCURCRSR->GetMark() )
            continue;

        if( bFirstMove )
        {
            
            aSttNdIdx = pPos->nNode.GetIndex()-1;
            nSttCntIdx = pPos->nContent.GetIndex();
            bFirstMove = false;
        }

        const bool bSuccess( GetDoc()->CopyRange( *PCURCRSR, *pPos, false ) );
        if (!bSuccess)
            continue;

        SwPaM aInsertPaM(*pPos, SwPosition(aSttNdIdx));
        pDestShell->GetDoc()->MakeUniqueNumRules(aInsertPaM);

        bRet = sal_True;
    FOREACHPAM_END()


    
    if( !bFirstMove )
    {
        SwPaM* pCrsr = pDestShell->GetCrsr();
        pCrsr->SetMark();
        pCrsr->GetPoint()->nNode = aSttNdIdx.GetIndex()+1;
        pCrsr->GetPoint()->nContent.Assign( pCrsr->GetCntntNode(),nSttCntIdx);
        pCrsr->Exchange();
    }
    else
    {
        
        pDestShell->GetCrsr()->SetMark();
        pDestShell->GetCrsr()->DeleteMark();
    }
#if OSL_DEBUG_LEVEL > 0

{
    SwPaM* pCmp = (SwPaM*)pDestShell->GetCrsr();        
    do {
        OSL_ENSURE( pCmp->GetPoint()->nContent.GetIdxReg()
                    == pCmp->GetCntntNode(), "Point in wrong Node" );
        OSL_ENSURE( pCmp->GetMark()->nContent.GetIdxReg()
                    == pCmp->GetCntntNode(false), "Mark in wrong Node" );
        bool bTst = *pCmp->GetPoint() == *pCmp->GetMark();
        (void) bTst;
    } while( pDestShell->GetCrsr() != ( pCmp = (SwPaM*)pCmp->GetNext() ) );
}
#endif

    
    pDestShell->GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
    pDestShell->EndAllAction();

    pDestShell->SaveTblBoxCntnt( pDestShell->GetCrsr()->GetPoint() );

    return (long)bRet;
}

/** Replace a selected area in a text node with a given string.
 *
 * Intended for "search & replace".
 *
 * @param bRegExpRplc if <true> replace tabs (\\t) and replace with found string (not \&).
 *                    E.g. [Fnd: "zzz", Repl: "xx\t\\t..&..\&"] --> "xx\t<Tab>..zzz..&"
 */
sal_Bool SwEditShell::Replace( const OUString& rNewStr, sal_Bool bRegExpRplc )
{
    SET_CURR_SHELL( this );

    sal_Bool bRet = sal_False;
    if( !HasReadonlySel() )
    {
        StartAllAction();
        GetDoc()->GetIDocumentUndoRedo().StartUndo(UNDO_EMPTY, NULL);

        FOREACHPAM_START(GetCrsr())
            if( PCURCRSR->HasMark() && *PCURCRSR->GetMark() != *PCURCRSR->GetPoint() )
            {
                bRet = GetDoc()->ReplaceRange( *PCURCRSR, rNewStr, bRegExpRplc )
                    || bRet;
                SaveTblBoxCntnt( PCURCRSR->GetPoint() );
            }
        FOREACHPAM_END()

        
        GetDoc()->GetIDocumentUndoRedo().EndUndo(UNDO_EMPTY, NULL);
        EndAllAction();
    }
    return bRet;
}


sal_Bool SwEditShell::DelFullPara()
{
    sal_Bool bRet = sal_False;
    if( !IsTableMode() )
    {
        SwPaM* pCrsr = GetCrsr();
        
        if( pCrsr->GetNext() == pCrsr && !HasReadonlySel() )
        {
            SET_CURR_SHELL( this );
            StartAllAction();
            bRet = GetDoc()->DelFullPara( *pCrsr );
            EndAllAction();
        }
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
