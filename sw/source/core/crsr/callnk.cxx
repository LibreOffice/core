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

#include <hintids.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <fmtcntnt.hxx>
#include <txatbase.hxx>
#include <frmatr.hxx>
#include <viscrs.hxx>
#include <callnk.hxx>
#include <crsrsh.hxx>
#include <doc.hxx>
#include <frmfmt.hxx>
#include <txtfrm.hxx>
#include <tabfrm.hxx>
#include <rowfrm.hxx>
#include <fmtfsize.hxx>
#include <ndtxt.hxx>
#include <flyfrm.hxx>
#include <breakit.hxx>
#include <vcl/window.hxx>

SwCallLink::SwCallLink( SwCursorShell & rSh, sal_uLong nAktNode, sal_Int32 nAktContent,
                        sal_uInt8 nAktNdTyp, long nLRPos, bool bAktSelection )
    : rShell( rSh ), nNode( nAktNode ), nContent( nAktContent ),
      nNdTyp( nAktNdTyp ), nLeftFramePos( nLRPos ),
      bHasSelection( bAktSelection )
{
}

SwCallLink::SwCallLink( SwCursorShell & rSh )
    : rShell( rSh )
{
    // remember SPoint-values of current cursor
    SwPaM* pCursor = rShell.IsTableMode() ? rShell.GetTableCrs() : rShell.GetCursor();
    SwNode& rNd = pCursor->GetPoint()->nNode.GetNode();
    nNode = rNd.GetIndex();
    nContent = pCursor->GetPoint()->nContent.GetIndex();
    nNdTyp = rNd.GetNodeType();
    bHasSelection = ( *pCursor->GetPoint() != *pCursor->GetMark() );

    if( rNd.IsTextNode() )
        nLeftFramePos = SwCallLink::getLayoutFrame( rShell.GetLayout(), *rNd.GetTextNode(), nContent,
                                            !rShell.ActionPend() );
    else
    {
        nLeftFramePos = 0;

        // A special treatment for SwFeShell:
        // When deleting the header/footer, footnotes SwFeShell sets the
        // Cursor to NULL (Node + Content).
        // If the Cursor is not on a ContentNode (ContentNode) this fact gets
        // saved in NdType.
        if( ND_CONTENTNODE & nNdTyp )
            nNdTyp = 0;
    }
}

static void lcl_notifyRow(const SwContentNode* pNode, SwCursorShell& rShell)
{
    if ( !pNode )
        return;

    SwFrame *const pMyFrame = pNode->getLayoutFrame( rShell.GetLayout() );
    if ( !pMyFrame )
        return;

    // We need to emulated a change of the row height in order
    // to have the complete row redrawn
    SwRowFrame *const pRow = pMyFrame->FindRowFrame();
    if ( !pRow )
        return;

    const SwTableLine* pLine = pRow->GetTabLine( );
    // Avoid redrawing the complete row if there are no nested tables
    for (SwFrame *pCell = pRow->GetLower(); pCell; pCell = pCell->GetNext())
    {
        for (SwFrame *pContent = pCell->GetLower(); pContent; pContent = pContent->GetNext())
        {
            if (pContent->GetType() == FRM_TAB)
            {
                SwFormatFrameSize aSize = pLine->GetFrameFormat()->GetFrameSize();
                pRow->ModifyNotification(nullptr, &aSize);
                return;
            }
        }
    }
}

SwCallLink::~SwCallLink()
{
    if( !nNdTyp || !rShell.m_bCallChgLnk ) // see ctor
        return ;

    // If travelling over Nodes check formats and register them anew at the
    // new Node.
    SwPaM* pCurrentCursor = rShell.IsTableMode() ? rShell.GetTableCrs() : rShell.GetCursor();
    SwContentNode * pCNd = pCurrentCursor->GetContentNode();
    if( !pCNd )
        return;

    lcl_notifyRow(pCNd, rShell);

    const SwDoc *pDoc=rShell.GetDoc();
    const SwContentNode *pNode = nullptr;
    if ( ( pDoc != nullptr && nNode < pDoc->GetNodes( ).Count( ) ) )
    {
        pNode = pDoc->GetNodes()[nNode]->GetContentNode();
    }
    lcl_notifyRow(pNode, rShell);

    sal_Int32 nCmp, nAktContent = pCurrentCursor->GetPoint()->nContent.GetIndex();
    sal_uInt16 nNdWhich = pCNd->GetNodeType();
    sal_uLong nAktNode = pCurrentCursor->GetPoint()->nNode.GetIndex();

    // Register the Shell as dependent at the current Node. By doing this all
    // attribute changes can be signaled over the link.
    pCNd->Add( &rShell );

    if( nNdTyp != nNdWhich || nNode != nAktNode )
    {
        // Every time a switch between nodes occurs, there is a chance that
        // new attributes do apply - meaning text-attributes.
        // So the currently applying attributes would have to be determined.
        // That can be done in one go by the handler.
        rShell.CallChgLnk();
    }
    else if( !bHasSelection != !(*pCurrentCursor->GetPoint() != *pCurrentCursor->GetMark()) )
    {
        // always call change link when selection changes
        rShell.CallChgLnk();
    }
    else if( rShell.m_aChgLnk.IsSet() && ND_TEXTNODE == nNdWhich &&
             nContent != nAktContent )
    {
        // If travelling with left/right only and the frame is
        // unchanged (columns!) then check text hints.
        if( nLeftFramePos == SwCallLink::getLayoutFrame( rShell.GetLayout(), *pCNd->GetTextNode(), nAktContent,
                                                    !rShell.ActionPend() ) &&
            (( nCmp = nContent ) + 1 == nAktContent ||          // Right
            nContent -1 == ( nCmp = nAktContent )) )            // Left
        {
            if( nCmp == nAktContent && pCurrentCursor->HasMark() ) // left & select
                ++nCmp;

            if ( pCNd->GetTextNode()->HasHints() )
            {
                const SwpHints &rHts = pCNd->GetTextNode()->GetSwpHints();

                for( size_t n = 0; n < rHts.Count(); ++n )
                {
                    const SwTextAttr* pHt = rHts.Get( n );
                    const sal_Int32 *pEnd = pHt->End();
                    const sal_Int32 nStart = pHt->GetStart();

                    // If "only start" or "start and end equal" then call on
                    // every overflow of start.
                    if( ( !pEnd || ( nStart == *pEnd ) ) &&
                        ( nStart == nContent || nStart == nAktContent) )
                    {
                        rShell.CallChgLnk();
                        return;
                    }

                    // If the attribute has an area and that area is not empty ...
                    else if( pEnd && nStart < *pEnd &&
                        // ... then test if travelling occurred via start/end.
                        ( nStart == nCmp ||
                            ( pHt->DontExpand() ? nCmp == *pEnd-1
                                                : nCmp == *pEnd ) ))
                    {
                        rShell.CallChgLnk();
                        return;
                    }
                }
            }

            if( g_pBreakIt->GetBreakIter().is() )
            {
                const OUString rText = pCNd->GetTextNode()->GetText();
                if( !nCmp ||
                    g_pBreakIt->GetBreakIter()->getScriptType( rText, nContent )
                    != g_pBreakIt->GetBreakIter()->getScriptType(rText, nAktContent))
                {
                    rShell.CallChgLnk();
                    return;
                }
            }
        }
        else
            // If travelling more than one character with home/end/.. then
            // always call ChgLnk, because it can not be determined here what
            // has changed. Something may have changed.
            rShell.CallChgLnk();
    }

    const SwFrame* pFrame;
    const SwFlyFrame *pFlyFrame;
    if( !rShell.ActionPend() && nullptr != ( pFrame = pCNd->getLayoutFrame(rShell.GetLayout(), nullptr, nullptr, false) ) &&
        nullptr != ( pFlyFrame = pFrame->FindFlyFrame() ) && !rShell.IsTableMode() )
    {
        const SwNodeIndex* pIndex = pFlyFrame->GetFormat()->GetContent().GetContentIdx();
        OSL_ENSURE( pIndex, "Fly without Content" );

        if (!pIndex)
            return;

        const SwNode& rStNd = pIndex->GetNode();

        if( rStNd.EndOfSectionNode()->StartOfSectionIndex() > nNode ||
            nNode > rStNd.EndOfSectionIndex() )
            rShell.GetFlyMacroLnk().Call( pFlyFrame->GetFormat() );
    }
}

long SwCallLink::getLayoutFrame( const SwRootFrame* pRoot, SwTextNode& rNd, sal_Int32 nCntPos, bool bCalcFrame )
{
    SwTextFrame* pFrame = static_cast<SwTextFrame*>(rNd.getLayoutFrame(pRoot,nullptr,nullptr,bCalcFrame));
    SwTextFrame* pNext;
    if ( pFrame && !pFrame->IsHiddenNow() )
    {
        if( pFrame->HasFollow() )
            while( nullptr != ( pNext = pFrame->GetFollow() ) &&
                    nCntPos >= pNext->GetOfst() )
                pFrame = pNext;

        return pFrame->Frame().Left();
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
