/*************************************************************************
 *
 *  $RCSfile: edglbldc.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>       // fuer die UndoIds
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _DOCTXM_HXX
#include <doctxm.hxx>
#endif
#ifndef _EDGLBLDC_HXX
#include <edglbldc.hxx>
#endif


SV_IMPL_OP_PTRARR_SORT( SwGlblDocContents, SwGlblDocContentPtr )

void SwEditShell::SetGlobalDoc( BOOL bFlag )
{
    GetDoc()->SetGlobalDoc( bFlag );
}

BOOL SwEditShell::IsGlobalDoc() const
{
    return GetDoc()->IsGlobalDoc();
}

void SwEditShell::SetGlblDocSaveLinks( BOOL bFlag )
{
    GetDoc()->SetGlblDocSaveLinks( bFlag );
    if( !GetDoc()->IsModified() )   // Bug 57028
        GetDoc()->SetUndoNoResetModified();
    GetDoc()->SetModified();
}

BOOL SwEditShell::IsGlblDocSaveLinks() const
{
    return GetDoc()->IsGlblDocSaveLinks();
}

USHORT SwEditShell::GetGlobalDocContent( SwGlblDocContents& rArr ) const
{
    if( rArr.Count() )
        rArr.DeleteAndDestroy( 0, rArr.Count() );

    SwDoc* pDoc = GetDoc();
    if( !pDoc->IsGlobalDoc() )
        return 0;

    // dann alle gelinkten Bereiche auf der obersten Ebene
    const SwSectionFmts& rSectFmts = pDoc->GetSections();
    for( USHORT n = rSectFmts.Count(); n; )
    {
        const SwSection* pSect = rSectFmts[ --n ]->GetGlobalDocSection();
        if( pSect )
        {
            SwGlblDocContentPtr pNew;
            switch( pSect->GetType() )
            {
            case TOX_HEADER_SECTION:    break;      // ignore
            case TOX_CONTENT_SECTION:
                ASSERT( pSect->ISA( SwTOXBaseSection ), "keine TOXBaseSection!" );
                pNew = new SwGlblDocContent( (SwTOXBaseSection*)pSect );
                break;

            default:
                pNew = new SwGlblDocContent( pSect );
                break;
            }
            if( !rArr.Insert( pNew ) )
                delete pNew;
        }
    }

    // und als letztes die Dummies (sonstiger Text) einfuegen
    SwNode* pNd;
    ULONG nSttIdx = pDoc->GetNodes().GetEndOfExtras().GetIndex() + 2;
    for( n = 0; n < rArr.Count(); ++n )
    {
        const SwGlblDocContent& rNew = *rArr[ n ];
        // suche von StartPos bis rNew.DocPos nach einem Content Node.
        // Existiert dieser, so muss ein DummyEintrag eingefuegt werden.
        for( ; nSttIdx < rNew.GetDocPos(); ++nSttIdx )
            if( ( pNd = pDoc->GetNodes()[ nSttIdx ])->IsCntntNode()
                || pNd->IsSectionNode() || pNd->IsTableNode() )
            {
                SwGlblDocContentPtr pNew = new SwGlblDocContent( nSttIdx );
                if( !rArr.Insert( pNew ) )
                    delete pNew;
                else
                    ++n;        // auf die naechste Position
                break;
            }

        // StartPosition aufs Ende setzen
        nSttIdx = pDoc->GetNodes()[ rNew.GetDocPos() ]->EndOfSectionIndex();
        ++nSttIdx;
    }

    // sollte man das Ende auch noch setzen??
    if( rArr.Count() )
    {
        ULONG nNdEnd = pDoc->GetNodes().GetEndOfContent().GetIndex();
        for( ; nSttIdx < nNdEnd; ++nSttIdx )
            if( ( pNd = pDoc->GetNodes()[ nSttIdx ])->IsCntntNode()
                || pNd->IsSectionNode() || pNd->IsTableNode() )
            {
                SwGlblDocContentPtr pNew = new SwGlblDocContent( nSttIdx );
                if( !rArr.Insert( pNew ) )
                    delete pNew;
                break;
            }
    }
    else
    {
        SwGlblDocContentPtr pNew = new SwGlblDocContent(
                    pDoc->GetNodes().GetEndOfExtras().GetIndex() + 2 );
        rArr.Insert( pNew );
    }
    return rArr.Count();
}

BOOL SwEditShell::InsertGlobalDocContent( const SwGlblDocContent& rInsPos,
                                            const SwSection& rNew )
{
    SwDoc* pDoc = GetDoc();
    if( !pDoc->IsGlobalDoc() )
        return FALSE;

    SET_CURR_SHELL( this );
    StartAllAction();

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr || IsTableMode() )
        ClearMark();

    SwPosition& rPos = *pCrsr->GetPoint();
    rPos.nNode = rInsPos.GetDocPos();

    BOOL bEndUndo = FALSE;
    SwTxtNode* pTxtNd = pDoc->GetNodes()[ rPos.nNode ]->GetTxtNode();
    if( pTxtNd )
        rPos.nContent.Assign( pTxtNd, 0 );
    else
    {
        bEndUndo = TRUE;
        pDoc->StartUndo( UNDO_START );
        rPos.nNode--;
        pDoc->AppendTxtNode( rPos );
        pCrsr->SetMark();
    }

    InsertSection( rNew );

    if( bEndUndo )
        pDoc->EndUndo( UNDO_END );
    EndAllAction();

    return TRUE;
}

BOOL SwEditShell::InsertGlobalDocContent( const SwGlblDocContent& rInsPos,
                                            const SwTOXBase& rTOX )
{
    SwDoc* pDoc = GetDoc();
    if( !pDoc->IsGlobalDoc() )
        return FALSE;

    SET_CURR_SHELL( this );
    StartAllAction();

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr || IsTableMode() )
        ClearMark();

    SwPosition& rPos = *pCrsr->GetPoint();
    rPos.nNode = rInsPos.GetDocPos();

    BOOL bEndUndo = FALSE;
    SwTxtNode* pTxtNd = rPos.nNode.GetNode().GetTxtNode();
    if( pTxtNd && pTxtNd->GetTxt().Len() && rPos.nNode.GetIndex() + 1 !=
        pDoc->GetNodes().GetEndOfContent().GetIndex() )
        rPos.nContent.Assign( pTxtNd, 0 );
    else
    {
        bEndUndo = TRUE;
        pDoc->StartUndo( UNDO_START );
        rPos.nNode--;
        pDoc->AppendTxtNode( rPos );
    }

    InsertTableOf( rTOX );

    if( bEndUndo )
        pDoc->EndUndo( UNDO_END );
    EndAllAction();

    return TRUE;
}

BOOL SwEditShell::InsertGlobalDocContent( const SwGlblDocContent& rInsPos )
{
    SwDoc* pDoc = GetDoc();
    if( !pDoc->IsGlobalDoc() )
        return FALSE;

    SET_CURR_SHELL( this );
    StartAllAction();

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr || IsTableMode() )
        ClearMark();

    SwPosition& rPos = *pCrsr->GetPoint();
    rPos.nNode = rInsPos.GetDocPos() - 1;
    rPos.nContent.Assign( 0, 0 );

    pDoc->AppendTxtNode( rPos );
    EndAllAction();
    return TRUE;
}

BOOL SwEditShell::DeleteGlobalDocContent( const SwGlblDocContents& rArr ,
                                            USHORT nDelPos )
{
    SwDoc* pDoc = GetDoc();
    if( !pDoc->IsGlobalDoc() )
        return FALSE;

    SET_CURR_SHELL( this );
    StartAllAction();
    StartUndo( UNDO_START );

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr || IsTableMode() )
        ClearMark();

    SwPosition& rPos = *pCrsr->GetPoint();

    const SwGlblDocContent& rDelPos = *rArr[ nDelPos ];
    ULONG nDelIdx = rDelPos.GetDocPos();
    if( 1 == rArr.Count() )
    {
        // ein Node muss aber da bleiben!
        rPos.nNode = nDelIdx - 1;
        rPos.nContent.Assign( 0, 0 );

        pDoc->AppendTxtNode( rPos );
        ++nDelIdx;
    }

    switch( rDelPos.GetType() )
    {
    case GLBLDOC_UNKNOWN:
        {
            rPos.nNode = nDelIdx;
            pCrsr->SetMark();
            if( ++nDelPos < rArr.Count() )
                rPos.nNode = rArr[ nDelPos ]->GetDocPos();
            else
                rPos.nNode = pDoc->GetNodes().GetEndOfContent();
            rPos.nNode--;
            if( !pDoc->DelFullPara( *pCrsr ) )
                Delete();
        }
        break;

    case GLBLDOC_TOXBASE:
        {
            SwTOXBaseSection* pTOX = (SwTOXBaseSection*)rDelPos.GetTOX();
            pDoc->DeleteTOX( *pTOX, TRUE );
        }
        break;

    case GLBLDOC_SECTION:
        {
            SwSectionFmt* pSectFmt = (SwSectionFmt*)rDelPos.GetSection()->GetFmt();
            pDoc->DelSectionFmt( pSectFmt, TRUE );
        }
        break;
    }

    EndUndo( UNDO_END );
    EndAllAction();
    return TRUE;
}

BOOL SwEditShell::MoveGlobalDocContent( const SwGlblDocContents& rArr ,
                                        USHORT nFromPos, USHORT nToPos,
                                        USHORT nInsPos )
{
    SwDoc* pDoc = GetDoc();
    if( !pDoc->IsGlobalDoc() ||
        nFromPos >= rArr.Count() || nToPos > rArr.Count() ||
        nInsPos > rArr.Count() || nFromPos >= nToPos ||
        ( nFromPos <= nInsPos && nInsPos <= nToPos ) )
        return FALSE;

    SET_CURR_SHELL( this );
    StartAllAction();

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr || IsTableMode() )
        ClearMark();

    SwNodeRange aRg( pDoc->GetNodes(), rArr[ nFromPos ]->GetDocPos() );
    if( nToPos < rArr.Count() )
        aRg.aEnd = rArr[ nToPos ]->GetDocPos();
    else
        aRg.aEnd = pDoc->GetNodes().GetEndOfContent();

    SwNodeIndex aInsPos( pDoc->GetNodes() );
    if( nInsPos < rArr.Count() )
        aInsPos = rArr[ nInsPos ]->GetDocPos();
    else
        aInsPos  = pDoc->GetNodes().GetEndOfContent();

    BOOL bRet = pDoc->Move( aRg, aInsPos, DOC_MOVEALLFLYS );

    EndAllAction();
    return bRet;
}

BOOL SwEditShell::GotoGlobalDocContent( const SwGlblDocContent& rPos )
{
    SwDoc* pDoc = GetDoc();
    if( !pDoc->IsGlobalDoc() )
        return FALSE;

    SET_CURR_SHELL( this );
    SttCrsrMove();

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr || IsTableMode() )
        ClearMark();

    SwPosition& rCrsrPos = *pCrsr->GetPoint();
    rCrsrPos.nNode = rPos.GetDocPos();

    SwCntntNode* pCNd = pDoc->GetNodes()[ rCrsrPos.nNode ]->GetCntntNode();
    if( !pCNd )
        pCNd = pDoc->GetNodes().GoNext( &rCrsrPos.nNode );

    rCrsrPos.nContent.Assign( pCNd, 0 );

    EndCrsrMove();
    return TRUE;
}

// erzeuge Anhand der Outlines Teildokumente
BOOL SwEditShell::GenerateGlobalDoc( const String& rPath,
                                const SwTxtFmtColl* pSplitColl )
{
    SwWait aWait( *GetDoc()->GetDocShell(), TRUE );
    return GetDoc()->GenerateGlobalDoc( rPath, pSplitColl );
}

/*  */

SwGlblDocContent::SwGlblDocContent( ULONG nPos )
{
    eType = GLBLDOC_UNKNOWN;
    PTR.pTOX = 0;
    nDocPos = nPos;
}

SwGlblDocContent::SwGlblDocContent( const SwTOXBaseSection* pTOX )
{
    eType = GLBLDOC_TOXBASE;
    PTR.pTOX = pTOX;

    const SwSectionNode* pSectNd = pTOX->GetFmt()->GetSectionNode();
    nDocPos = pSectNd ? pSectNd->GetIndex() : 0;
}

SwGlblDocContent::SwGlblDocContent( const SwSection* pSect )
{
    eType = GLBLDOC_SECTION;
    PTR.pSect = pSect;

    const SwSectionNode* pSectNd = pSect->GetFmt()->GetSectionNode();
    nDocPos = pSectNd ? pSectNd->GetIndex() : 0;
}



