/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: edglbldc.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:45:27 $
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

BOOL SwEditShell::IsGlobalDoc() const
{
    return getIDocumentSettingAccess()->get(IDocumentSettingAccess::GLOBAL_DOCUMENT);
}

void SwEditShell::SetGlblDocSaveLinks( BOOL bFlag )
{
    getIDocumentSettingAccess()->set(IDocumentSettingAccess::GLOBAL_DOCUMENT_SAVE_LINKS, bFlag);
    if( !GetDoc()->IsModified() )   // Bug 57028
        GetDoc()->SetUndoNoResetModified();
    GetDoc()->SetModified();
}

BOOL SwEditShell::IsGlblDocSaveLinks() const
{
    return getIDocumentSettingAccess()->get(IDocumentSettingAccess::GLOBAL_DOCUMENT_SAVE_LINKS);
}

USHORT SwEditShell::GetGlobalDocContent( SwGlblDocContents& rArr ) const
{
    if( rArr.Count() )
        rArr.DeleteAndDestroy( 0, rArr.Count() );

    if( !getIDocumentSettingAccess()->get(IDocumentSettingAccess::GLOBAL_DOCUMENT) )
        return 0;

    // dann alle gelinkten Bereiche auf der obersten Ebene
    SwDoc* pMyDoc = GetDoc();
    const SwSectionFmts& rSectFmts = pMyDoc->GetSections();
    USHORT n;

    for( n = rSectFmts.Count(); n; )
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
    ULONG nSttIdx = pMyDoc->GetNodes().GetEndOfExtras().GetIndex() + 2;
    for( n = 0; n < rArr.Count(); ++n )
    {
        const SwGlblDocContent& rNew = *rArr[ n ];
        // suche von StartPos bis rNew.DocPos nach einem Content Node.
        // Existiert dieser, so muss ein DummyEintrag eingefuegt werden.
        for( ; nSttIdx < rNew.GetDocPos(); ++nSttIdx )
            if( ( pNd = pMyDoc->GetNodes()[ nSttIdx ])->IsCntntNode()
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
        nSttIdx = pMyDoc->GetNodes()[ rNew.GetDocPos() ]->EndOfSectionIndex();
        ++nSttIdx;
    }

    // sollte man das Ende auch noch setzen??
    if( rArr.Count() )
    {
        ULONG nNdEnd = pMyDoc->GetNodes().GetEndOfContent().GetIndex();
        for( ; nSttIdx < nNdEnd; ++nSttIdx )
            if( ( pNd = pMyDoc->GetNodes()[ nSttIdx ])->IsCntntNode()
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
                    pMyDoc->GetNodes().GetEndOfExtras().GetIndex() + 2 );
        rArr.Insert( pNew );
    }
    return rArr.Count();
}

BOOL SwEditShell::InsertGlobalDocContent( const SwGlblDocContent& rInsPos,
                                            const SwSection& rNew )
{
    if( !getIDocumentSettingAccess()->get(IDocumentSettingAccess::GLOBAL_DOCUMENT) )
        return FALSE;

    SET_CURR_SHELL( this );
    StartAllAction();

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr || IsTableMode() )
        ClearMark();

    SwPosition& rPos = *pCrsr->GetPoint();
    rPos.nNode = rInsPos.GetDocPos();

    BOOL bEndUndo = FALSE;
    SwDoc* pMyDoc = GetDoc();
    SwTxtNode* pTxtNd = pMyDoc->GetNodes()[ rPos.nNode ]->GetTxtNode();
    if( pTxtNd )
        rPos.nContent.Assign( pTxtNd, 0 );
    else
    {
        bEndUndo = TRUE;
        pMyDoc->StartUndo( UNDO_START, NULL );
        rPos.nNode--;
        pMyDoc->AppendTxtNode( rPos );
        pCrsr->SetMark();
    }

    InsertSection( rNew );

    if( bEndUndo )
        pMyDoc->EndUndo( UNDO_END, NULL );
    EndAllAction();

    return TRUE;
}

BOOL SwEditShell::InsertGlobalDocContent( const SwGlblDocContent& rInsPos,
                                            const SwTOXBase& rTOX )
{
    if( !getIDocumentSettingAccess()->get(IDocumentSettingAccess::GLOBAL_DOCUMENT) )
        return FALSE;

    SET_CURR_SHELL( this );
    StartAllAction();

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr || IsTableMode() )
        ClearMark();

    SwPosition& rPos = *pCrsr->GetPoint();
    rPos.nNode = rInsPos.GetDocPos();

    BOOL bEndUndo = FALSE;
    SwDoc* pMyDoc = GetDoc();
    SwTxtNode* pTxtNd = rPos.nNode.GetNode().GetTxtNode();
    if( pTxtNd && pTxtNd->GetTxt().Len() && rPos.nNode.GetIndex() + 1 !=
        pMyDoc->GetNodes().GetEndOfContent().GetIndex() )
        rPos.nContent.Assign( pTxtNd, 0 );
    else
    {
        bEndUndo = TRUE;
        pMyDoc->StartUndo( UNDO_START, NULL );
        rPos.nNode--;
        pMyDoc->AppendTxtNode( rPos );
    }

    InsertTableOf( rTOX );

    if( bEndUndo )
        pMyDoc->EndUndo( UNDO_END, NULL );
    EndAllAction();

    return TRUE;
}

BOOL SwEditShell::InsertGlobalDocContent( const SwGlblDocContent& rInsPos )
{
    if( !getIDocumentSettingAccess()->get(IDocumentSettingAccess::GLOBAL_DOCUMENT) )
        return FALSE;

    SET_CURR_SHELL( this );
    StartAllAction();

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr || IsTableMode() )
        ClearMark();

    SwPosition& rPos = *pCrsr->GetPoint();
    rPos.nNode = rInsPos.GetDocPos() - 1;
    rPos.nContent.Assign( 0, 0 );

    SwDoc* pMyDoc = GetDoc();
    pMyDoc->AppendTxtNode( rPos );
    EndAllAction();
    return TRUE;
}

BOOL SwEditShell::DeleteGlobalDocContent( const SwGlblDocContents& rArr ,
                                            USHORT nDelPos )
{
    if( !getIDocumentSettingAccess()->get(IDocumentSettingAccess::GLOBAL_DOCUMENT) )
        return FALSE;

    SET_CURR_SHELL( this );
    StartAllAction();
    StartUndo( UNDO_START );

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr || IsTableMode() )
        ClearMark();

    SwPosition& rPos = *pCrsr->GetPoint();

    SwDoc* pMyDoc = GetDoc();
    const SwGlblDocContent& rDelPos = *rArr[ nDelPos ];
    ULONG nDelIdx = rDelPos.GetDocPos();
    if( 1 == rArr.Count() )
    {
        // ein Node muss aber da bleiben!
        rPos.nNode = nDelIdx - 1;
        rPos.nContent.Assign( 0, 0 );

        pMyDoc->AppendTxtNode( rPos );
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
                rPos.nNode = pMyDoc->GetNodes().GetEndOfContent();
            rPos.nNode--;
            if( !pMyDoc->DelFullPara( *pCrsr ) )
                Delete();
        }
        break;

    case GLBLDOC_TOXBASE:
        {
            SwTOXBaseSection* pTOX = (SwTOXBaseSection*)rDelPos.GetTOX();
            pMyDoc->DeleteTOX( *pTOX, TRUE );
        }
        break;

    case GLBLDOC_SECTION:
        {
            SwSectionFmt* pSectFmt = (SwSectionFmt*)rDelPos.GetSection()->GetFmt();
            pMyDoc->DelSectionFmt( pSectFmt, TRUE );
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
    if( !getIDocumentSettingAccess()->get(IDocumentSettingAccess::GLOBAL_DOCUMENT) ||
        nFromPos >= rArr.Count() || nToPos > rArr.Count() ||
        nInsPos > rArr.Count() || nFromPos >= nToPos ||
        ( nFromPos <= nInsPos && nInsPos <= nToPos ) )
        return FALSE;

    SET_CURR_SHELL( this );
    StartAllAction();

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr || IsTableMode() )
        ClearMark();

    SwDoc* pMyDoc = GetDoc();
    SwNodeRange aRg( pMyDoc->GetNodes(), rArr[ nFromPos ]->GetDocPos() );
    if( nToPos < rArr.Count() )
        aRg.aEnd = rArr[ nToPos ]->GetDocPos();
    else
        aRg.aEnd = pMyDoc->GetNodes().GetEndOfContent();

    SwNodeIndex aInsPos( pMyDoc->GetNodes() );
    if( nInsPos < rArr.Count() )
        aInsPos = rArr[ nInsPos ]->GetDocPos();
    else
        aInsPos  = pMyDoc->GetNodes().GetEndOfContent();

    BOOL bRet = pMyDoc->Move( aRg, aInsPos, IDocumentContentOperations::DOC_MOVEALLFLYS );

    EndAllAction();
    return bRet;
}

BOOL SwEditShell::GotoGlobalDocContent( const SwGlblDocContent& rPos )
{
    if( !getIDocumentSettingAccess()->get(IDocumentSettingAccess::GLOBAL_DOCUMENT) )
        return FALSE;

    SET_CURR_SHELL( this );
    SttCrsrMove();

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr || IsTableMode() )
        ClearMark();

    SwPosition& rCrsrPos = *pCrsr->GetPoint();
    rCrsrPos.nNode = rPos.GetDocPos();

    SwDoc* pMyDoc = GetDoc();
    SwCntntNode* pCNd = pMyDoc->GetNodes()[ rCrsrPos.nNode ]->GetCntntNode();
    if( !pCNd )
        pCNd = pMyDoc->GetNodes().GoNext( &rCrsrPos.nNode );

    rCrsrPos.nContent.Assign( pCNd, 0 );

    EndCrsrMove();
    return TRUE;
}

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



