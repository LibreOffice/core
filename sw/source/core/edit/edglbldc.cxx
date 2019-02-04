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

#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentState.hxx>
#include <editsh.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <docary.hxx>
#include <swwait.hxx>
#include <swundo.hxx>
#include <section.hxx>
#include <doctxm.hxx>
#include <edglbldc.hxx>
#include <osl/diagnose.h>

bool SwEditShell::IsGlobalDoc() const
{
    return getIDocumentSettingAccess().get(DocumentSettingId::GLOBAL_DOCUMENT);
}

void SwEditShell::SetGlblDocSaveLinks( bool bFlag )
{
    getIDocumentSettingAccess().set(DocumentSettingId::GLOBAL_DOCUMENT_SAVE_LINKS, bFlag);
    if( !GetDoc()->getIDocumentState().IsModified() )   // Bug 57028
    {
        GetDoc()->GetIDocumentUndoRedo().SetUndoNoResetModified();
    }
    GetDoc()->getIDocumentState().SetModified();
}

bool SwEditShell::IsGlblDocSaveLinks() const
{
    return getIDocumentSettingAccess().get(DocumentSettingId::GLOBAL_DOCUMENT_SAVE_LINKS);
}

void SwEditShell::GetGlobalDocContent( SwGlblDocContents& rArr ) const
{
    rArr.clear();

    if( !getIDocumentSettingAccess().get(DocumentSettingId::GLOBAL_DOCUMENT) )
        return;

    // then all linked areas on the topmost level
    SwDoc* pMyDoc = GetDoc();
    const SwSectionFormats& rSectFormats = pMyDoc->GetSections();

    for( auto n = rSectFormats.size(); n; )
    {
        const SwSection* pSect = rSectFormats[ --n ]->GetGlobalDocSection();
        if( pSect )
        {
            std::unique_ptr<SwGlblDocContent> pNew;
            switch( pSect->GetType() )
            {
            case TOX_HEADER_SECTION:
                break;      // ignore
            case TOX_CONTENT_SECTION:
                OSL_ENSURE( dynamic_cast<const SwTOXBaseSection*>( pSect) !=  nullptr, "no TOXBaseSection!" );
                pNew.reset(new SwGlblDocContent( static_cast<const SwTOXBaseSection*>(pSect) ));
                break;

            default:
                pNew.reset(new SwGlblDocContent( pSect ));
                break;
            }
            rArr.insert( std::move(pNew) );
        }
    }

    // and finally add the dummies (other text)
    SwNode* pNd;
    sal_uLong nSttIdx = pMyDoc->GetNodes().GetEndOfExtras().GetIndex() + 2;
    for( SwGlblDocContents::size_type n = 0; n < rArr.size(); ++n )
    {
        const SwGlblDocContent& rNew = *rArr[ n ];
        // Search from StartPos until rNew.DocPos for a content node.
        // If one exists then a dummy entry is needed.
        for( ; nSttIdx < rNew.GetDocPos(); ++nSttIdx )
            if( ( pNd = pMyDoc->GetNodes()[ nSttIdx ])->IsContentNode()
                || pNd->IsSectionNode() || pNd->IsTableNode() )
            {
                std::unique_ptr<SwGlblDocContent> pNew(new SwGlblDocContent( nSttIdx ));
                if( rArr.insert( std::move(pNew) ).second )
                    ++n; // to the next position
                break;
            }

        // set StartPosition to the end
        nSttIdx = pMyDoc->GetNodes()[ rNew.GetDocPos() ]->EndOfSectionIndex();
        ++nSttIdx;
    }

    // Should the end also be set?
    if( !rArr.empty() )
    {
        sal_uLong nNdEnd = pMyDoc->GetNodes().GetEndOfContent().GetIndex();
        for( ; nSttIdx < nNdEnd; ++nSttIdx )
            if( ( pNd = pMyDoc->GetNodes()[ nSttIdx ])->IsContentNode()
                || pNd->IsSectionNode() || pNd->IsTableNode() )
            {
                rArr.insert( std::make_unique<SwGlblDocContent>( nSttIdx ) );
                break;
            }
    }
    else
    {
        std::unique_ptr<SwGlblDocContent> pNew(new SwGlblDocContent(
                    pMyDoc->GetNodes().GetEndOfExtras().GetIndex() + 2 ));
        rArr.insert( std::move(pNew) );
    }
}

void SwEditShell::InsertGlobalDocContent( const SwGlblDocContent& rInsPos,
        SwSectionData & rNew)
{
    if( !getIDocumentSettingAccess().get(DocumentSettingId::GLOBAL_DOCUMENT) )
        return;

    SET_CURR_SHELL( this );
    StartAllAction();

    SwPaM* pCursor = GetCursor();
    if( pCursor->GetNext() != pCursor || IsTableMode() )
        ClearMark();

    SwPosition& rPos = *pCursor->GetPoint();
    rPos.nNode = rInsPos.GetDocPos();

    bool bEndUndo = false;
    SwDoc* pMyDoc = GetDoc();
    SwTextNode *const pTextNd = rPos.nNode.GetNode().GetTextNode();
    if( pTextNd )
        rPos.nContent.Assign( pTextNd, 0 );
    else
    {
        bEndUndo = true;
        pMyDoc->GetIDocumentUndoRedo().StartUndo( SwUndoId::START, nullptr );
        --rPos.nNode;
        pMyDoc->getIDocumentContentOperations().AppendTextNode( rPos );
        pCursor->SetMark();
    }

    InsertSection( rNew );

    if( bEndUndo )
    {
        pMyDoc->GetIDocumentUndoRedo().EndUndo( SwUndoId::END, nullptr );
    }
    EndAllAction();
}

bool SwEditShell::InsertGlobalDocContent( const SwGlblDocContent& rInsPos,
                                            const SwTOXBase& rTOX )
{
    if( !getIDocumentSettingAccess().get(DocumentSettingId::GLOBAL_DOCUMENT) )
        return false;

    SET_CURR_SHELL( this );
    StartAllAction();

    SwPaM* pCursor = GetCursor();
    if( pCursor->GetNext() != pCursor || IsTableMode() )
        ClearMark();

    SwPosition& rPos = *pCursor->GetPoint();
    rPos.nNode = rInsPos.GetDocPos();

    bool bEndUndo = false;
    SwDoc* pMyDoc = GetDoc();
    SwTextNode* pTextNd = rPos.nNode.GetNode().GetTextNode();
    if (pTextNd && pTextNd->GetText().getLength() && rPos.nNode.GetIndex() + 1 !=
        pMyDoc->GetNodes().GetEndOfContent().GetIndex() )
        rPos.nContent.Assign( pTextNd, 0 );
    else
    {
        bEndUndo = true;
        pMyDoc->GetIDocumentUndoRedo().StartUndo( SwUndoId::START, nullptr );
        --rPos.nNode;
        pMyDoc->getIDocumentContentOperations().AppendTextNode( rPos );
    }

    InsertTableOf( rTOX );

    if( bEndUndo )
    {
        pMyDoc->GetIDocumentUndoRedo().EndUndo( SwUndoId::END, nullptr );
    }
    EndAllAction();

    return true;
}

bool SwEditShell::InsertGlobalDocContent( const SwGlblDocContent& rInsPos )
{
    if( !getIDocumentSettingAccess().get(DocumentSettingId::GLOBAL_DOCUMENT) )
        return false;

    SET_CURR_SHELL( this );
    StartAllAction();

    SwPaM* pCursor = GetCursor();
    if( pCursor->GetNext() != pCursor || IsTableMode() )
        ClearMark();

    SwPosition& rPos = *pCursor->GetPoint();
    rPos.nNode = rInsPos.GetDocPos() - 1;
    rPos.nContent.Assign( nullptr, 0 );

    SwDoc* pMyDoc = GetDoc();
    pMyDoc->getIDocumentContentOperations().AppendTextNode( rPos );
    EndAllAction();
    return true;
}

void SwEditShell::DeleteGlobalDocContent( const SwGlblDocContents& rArr ,
                                            size_t nDelPos )
{
    if( !getIDocumentSettingAccess().get(DocumentSettingId::GLOBAL_DOCUMENT) )
        return;

    SET_CURR_SHELL( this );
    StartAllAction();
    StartUndo( SwUndoId::START );

    SwPaM* pCursor = GetCursor();
    if( pCursor->GetNext() != pCursor || IsTableMode() )
        ClearMark();

    SwPosition& rPos = *pCursor->GetPoint();

    SwDoc* pMyDoc = GetDoc();
    const SwGlblDocContent& rDelPos = *rArr[ nDelPos ];
    sal_uLong nDelIdx = rDelPos.GetDocPos();
    if( 1 == rArr.size() )
    {
        // we need at least one node!
        rPos.nNode = nDelIdx - 1;
        rPos.nContent.Assign( nullptr, 0 );

        pMyDoc->getIDocumentContentOperations().AppendTextNode( rPos );
        ++nDelIdx;
    }

    switch( rDelPos.GetType() )
    {
    case GLBLDOC_UNKNOWN:
        {
            rPos.nNode = nDelIdx;
            pCursor->SetMark();
            if( ++nDelPos < rArr.size() )
                rPos.nNode = rArr[ nDelPos ]->GetDocPos();
            else
                rPos.nNode = pMyDoc->GetNodes().GetEndOfContent();
            --rPos.nNode;
            if( !pMyDoc->getIDocumentContentOperations().DelFullPara( *pCursor ) )
                Delete();
        }
        break;

    case GLBLDOC_TOXBASE:
        {
            const SwTOXBaseSection* pTOX = static_cast<const SwTOXBaseSection*>(rDelPos.GetTOX());
            pMyDoc->DeleteTOX( *pTOX, true );
        }
        break;

    case GLBLDOC_SECTION:
        {
            SwSectionFormat* pSectFormat = const_cast<SwSectionFormat*>(rDelPos.GetSection()->GetFormat());
            pMyDoc->DelSectionFormat( pSectFormat, true );
        }
        break;
    }

    EndUndo( SwUndoId::END );
    EndAllAction();
}

bool SwEditShell::MoveGlobalDocContent( const SwGlblDocContents& rArr ,
                                        size_t nFromPos, size_t nToPos,
                                        size_t nInsPos )
{
    if( !getIDocumentSettingAccess().get(DocumentSettingId::GLOBAL_DOCUMENT) ||
        nFromPos >= rArr.size() || nToPos > rArr.size() ||
        nInsPos > rArr.size() || nFromPos >= nToPos ||
        ( nFromPos <= nInsPos && nInsPos <= nToPos ) )
        return false;

    SET_CURR_SHELL( this );
    StartAllAction();

    SwPaM* pCursor = GetCursor();
    if( pCursor->GetNext() != pCursor || IsTableMode() )
        ClearMark();

    SwDoc* pMyDoc = GetDoc();
    SwNodeRange aRg( pMyDoc->GetNodes(), rArr[ nFromPos ]->GetDocPos() );
    if( nToPos < rArr.size() )
        aRg.aEnd = rArr[ nToPos ]->GetDocPos();
    else
        aRg.aEnd = pMyDoc->GetNodes().GetEndOfContent();

    SwNodeIndex aInsPos( pMyDoc->GetNodes() );
    if( nInsPos < rArr.size() )
        aInsPos = rArr[ nInsPos ]->GetDocPos();
    else
        aInsPos  = pMyDoc->GetNodes().GetEndOfContent();

    bool bRet = pMyDoc->getIDocumentContentOperations().MoveNodeRange( aRg, aInsPos,
              SwMoveFlags::ALLFLYS | SwMoveFlags::CREATEUNDOOBJ );

    EndAllAction();
    return bRet;
}

void SwEditShell::GotoGlobalDocContent( const SwGlblDocContent& rPos )
{
    if( !getIDocumentSettingAccess().get(DocumentSettingId::GLOBAL_DOCUMENT) )
        return;

    SET_CURR_SHELL( this );
    SttCursorMove();

    SwPaM* pCursor = GetCursor();
    if( pCursor->GetNext() != pCursor || IsTableMode() )
        ClearMark();

    SwPosition& rCursorPos = *pCursor->GetPoint();
    rCursorPos.nNode = rPos.GetDocPos();

    SwDoc* pMyDoc = GetDoc();
    SwContentNode * pCNd = rCursorPos.nNode.GetNode().GetContentNode();
    if( !pCNd )
        pCNd = pMyDoc->GetNodes().GoNext( &rCursorPos.nNode );

    rCursorPos.nContent.Assign( pCNd, 0 );

    EndCursorMove();
}

SwGlblDocContent::SwGlblDocContent( sal_uLong nPos )
{
    eType = GLBLDOC_UNKNOWN;
    PTR.pTOX = nullptr;
    nDocPos = nPos;
}

SwGlblDocContent::SwGlblDocContent( const SwTOXBaseSection* pTOX )
{
    eType = GLBLDOC_TOXBASE;
    PTR.pTOX = pTOX;

    const SwSectionNode* pSectNd = pTOX->GetFormat()->GetSectionNode();
    nDocPos = pSectNd ? pSectNd->GetIndex() : 0;
}

SwGlblDocContent::SwGlblDocContent( const SwSection* pSect )
{
    eType = GLBLDOC_SECTION;
    PTR.pSect = pSect;

    const SwSectionNode* pSectNd = pSect->GetFormat()->GetSectionNode();
    nDocPos = pSectNd ? pSectNd->GetIndex() : 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
