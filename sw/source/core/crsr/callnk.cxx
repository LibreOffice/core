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
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <fmtcntnt.hxx>
#include <txatbase.hxx>
#include <frmatr.hxx>
#include <viscrs.hxx>
#include "callnk.hxx"
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
        // saved in nNdType.
        if( SwNodeType::ContentMask & nNdTyp )
            nNdTyp = SwNodeType::NONE;
    }
}

static void lcl_notifyRow(const SwContentNode* pNode, SwCursorShell const & rShell)
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
            if (pContent->GetType() == SwFrameType::Tab)
            {
                SwFormatFrameSize aSize = pLine->GetFrameFormat()->GetFrameSize();
                pRow->ModifyNotification(nullptr, &aSize);
                return;
            }
        }
    }
}

SwCallLink::~SwCallLink() COVERITY_NOEXCEPT_FALSE
{
    if( nNdTyp == SwNodeType::NONE || !rShell.m_bCallChgLnk ) // see ctor
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
    if ( pDoc && nNode < pDoc->GetNodes( ).Count( ) )
    {
        pNode = pDoc->GetNodes()[nNode]->GetContentNode();
    }
    lcl_notifyRow(pNode, rShell);

    sal_Int32 nCmp, nCurrentContent = pCurrentCursor->GetPoint()->nContent.GetIndex();
    SwNodeType nNdWhich = pCNd->GetNodeType();
    sal_uLong nCurrentNode = pCurrentCursor->GetPoint()->nNode.GetIndex();

    // Register the Shell as dependent at the current Node. By doing this all
    // attribute changes can be signaled over the link.
    pCNd->Add( &rShell );

    if( nNdTyp != nNdWhich || nNode != nCurrentNode )
    {
        // Every time a switch between nodes occurs, there is a chance that
        // new attributes do apply - meaning text-attributes.
        // So the currently applying attributes would have to be determined.
        // That can be done in one go by the handler.
        rShell.CallChgLnk();
    }
    else if( !bHasSelection != (*pCurrentCursor->GetPoint() == *pCurrentCursor->GetMark()) )
    {
        // always call change link when selection changes
        rShell.CallChgLnk();
    }
    else if( rShell.m_aChgLnk.IsSet() && SwNodeType::Text == nNdWhich &&
             nContent != nCurrentContent )
    {
        // If travelling with left/right only and the frame is
        // unchanged (columns!) then check text hints.
        if( nLeftFramePos == SwCallLink::getLayoutFrame( rShell.GetLayout(), *pCNd->GetTextNode(), nCurrentContent,
                                                    !rShell.ActionPend() ) &&
            (( nCmp = nContent ) + 1 == nCurrentContent ||          // Right
            nContent -1 == ( nCmp = nCurrentContent )) )            // Left
        {
            if( nCmp == nCurrentContent && pCurrentCursor->HasMark() ) // left & select
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
                        ( nStart == nContent || nStart == nCurrentContent) )
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

            assert(g_pBreakIt && g_pBreakIt->GetBreakIter().is());
            const OUString rText = pCNd->GetTextNode()->GetText();
            if( !nCmp ||
                g_pBreakIt->GetBreakIter()->getScriptType( rText, nContent )
                != g_pBreakIt->GetBreakIter()->getScriptType(rText, nCurrentContent))
            {
                rShell.CallChgLnk();
                return;
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
    if (!rShell.ActionPend() && nullptr != (pFrame = pCNd->getLayoutFrame(rShell.GetLayout(), nullptr, nullptr)) &&
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

long SwCallLink::getLayoutFrame(const SwRootFrame* pRoot,
        SwTextNode const & rNd, sal_Int32 nCntPos, bool /*bCalcFrame*/)
{
    SwTextFrame* pFrame = static_cast<SwTextFrame*>(rNd.getLayoutFrame(pRoot, nullptr, nullptr));
    SwTextFrame* pNext;
    if ( pFrame && !pFrame->IsHiddenNow() )
    {
        if( pFrame->HasFollow() )
        {
            TextFrameIndex const nPos(pFrame->MapModelToView(&rNd, nCntPos));
            while( nullptr != ( pNext = pFrame->GetFollow() ) &&
                    nPos >= pNext->GetOfst())
                pFrame = pNext;
        }

        return pFrame->getFrameArea().Left();
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
