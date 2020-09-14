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

#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <fmtcntnt.hxx>
#include <txatbase.hxx>
#include "callnk.hxx"
#include <crsrsh.hxx>
#include <doc.hxx>
#include <frmfmt.hxx>
#include <txtfrm.hxx>
#include <rowfrm.hxx>
#include <fmtfsize.hxx>
#include <ndtxt.hxx>
#include <flyfrm.hxx>
#include <breakit.hxx>

SwCallLink::SwCallLink( SwCursorShell & rSh )
    : m_rShell( rSh )
{
    // remember SPoint-values of current cursor
    SwPaM* pCursor = m_rShell.IsTableMode() ? m_rShell.GetTableCrs() : m_rShell.GetCursor();
    SwNode& rNd = pCursor->GetPoint()->nNode.GetNode();
    m_nNode = rNd.GetIndex();
    m_nContent = pCursor->GetPoint()->nContent.GetIndex();
    m_nNodeType = rNd.GetNodeType();
    m_bHasSelection = ( *pCursor->GetPoint() != *pCursor->GetMark() );

    if( rNd.IsTextNode() )
        m_nLeftFramePos = SwCallLink::getLayoutFrame( m_rShell.GetLayout(), *rNd.GetTextNode(), m_nContent,
                                            !m_rShell.ActionPend() );
    else
    {
        m_nLeftFramePos = 0;

        // A special treatment for SwFeShell:
        // When deleting the header/footer, footnotes SwFeShell sets the
        // Cursor to NULL (Node + Content).
        // If the Cursor is not on a ContentNode (ContentNode) this fact gets
        // saved in nNdType.
        if( SwNodeType::ContentMask & m_nNodeType )
            m_nNodeType = SwNodeType::NONE;
    }
}

static void lcl_notifyRow(const SwContentNode* pNode, SwCursorShell & rShell)
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

    if (rShell.IsTableMode() || (rShell.StartsWithTable() && rShell.ExtendedSelectedAll()))
    {
        // If we have a table selection, then avoid the notification: it's not necessary (the text
        // cursor needs no updating) and the notification may kill the selection overlay, leading to
        // flicker.
        // Same for whole-document selection when it starts with a table.
        return;
    }

    SwFormatFrameSize aSize = pLine->GetFrameFormat()->GetFrameSize();
    pRow->ModifyNotification(nullptr, &aSize);
}

SwCallLink::~SwCallLink() COVERITY_NOEXCEPT_FALSE
{
    if( m_nNodeType == SwNodeType::NONE || !m_rShell.m_bCallChgLnk ) // see ctor
        return ;

    // If travelling over Nodes check formats and register them anew at the
    // new Node.
    SwPaM* pCurrentCursor = m_rShell.IsTableMode() ? m_rShell.GetTableCrs() : m_rShell.GetCursor();
    SwContentNode * pCNd = pCurrentCursor->GetContentNode();
    if( !pCNd )
        return;

    lcl_notifyRow(pCNd, m_rShell);

    const SwDoc *pDoc=m_rShell.GetDoc();
    const SwContentNode *pNode = nullptr;
    if ( pDoc && m_nNode < pDoc->GetNodes( ).Count( ) )
    {
        pNode = pDoc->GetNodes()[m_nNode]->GetContentNode();
    }
    lcl_notifyRow(pNode, m_rShell);

    sal_Int32 nCmp, nCurrentContent = pCurrentCursor->GetPoint()->nContent.GetIndex();
    SwNodeType nNdWhich = pCNd->GetNodeType();
    sal_uLong nCurrentNode = pCurrentCursor->GetPoint()->nNode.GetIndex();

    // Register the Shell as dependent at the current Node. By doing this all
    // attribute changes can be signaled over the link.
    pCNd->Add( &m_rShell );

    const bool bCurrentHasSelection = (*pCurrentCursor->GetPoint() != *pCurrentCursor->GetMark());

    if( m_nNodeType != nNdWhich || m_nNode != nCurrentNode )
    {
        // Every time a switch between nodes occurs, there is a chance that
        // new attributes do apply - meaning text-attributes.
        // So the currently applying attributes would have to be determined.
        // That can be done in one go by the handler.
        m_rShell.CallChgLnk();
    }
    else if (m_bHasSelection != bCurrentHasSelection)
    {
        // always call change link when selection changes
        m_rShell.CallChgLnk();
    }
    else if( m_rShell.m_aChgLnk.IsSet() && SwNodeType::Text == nNdWhich &&
             m_nContent != nCurrentContent )
    {
        // If travelling with left/right only and the frame is
        // unchanged (columns!) then check text hints.
        if( m_nLeftFramePos == SwCallLink::getLayoutFrame( m_rShell.GetLayout(), *pCNd->GetTextNode(), nCurrentContent,
                                                    !m_rShell.ActionPend() ) &&
            (( nCmp = m_nContent ) + 1 == nCurrentContent ||          // Right
            m_nContent -1 == ( nCmp = nCurrentContent )) )            // Left
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
                        ( nStart == m_nContent || nStart == nCurrentContent) )
                    {
                        m_rShell.CallChgLnk();
                        return;
                    }

                    // If the attribute has an area and that area is not empty ...
                    else if( pEnd && nStart < *pEnd &&
                        // ... then test if travelling occurred via start/end.
                        ( nStart == nCmp ||
                            ( pHt->DontExpand() ? nCmp == *pEnd-1
                                                : nCmp == *pEnd ) ))
                    {
                        m_rShell.CallChgLnk();
                        return;
                    }
                }
            }

            assert(g_pBreakIt && g_pBreakIt->GetBreakIter().is());
            const OUString rText = pCNd->GetTextNode()->GetText();
            if( !nCmp ||
                g_pBreakIt->GetBreakIter()->getScriptType( rText, m_nContent )
                != g_pBreakIt->GetBreakIter()->getScriptType(rText, nCurrentContent))
            {
                m_rShell.CallChgLnk();
                return;
            }
        }
        else
            // If travelling more than one character with home/end/.. then
            // always call ChgLnk, because it can not be determined here what
            // has changed. Something may have changed.
            m_rShell.CallChgLnk();
    }

    const SwFrame* pFrame;
    const SwFlyFrame *pFlyFrame;
    if (m_rShell.ActionPend())
        return;
    pFrame = pCNd->getLayoutFrame(m_rShell.GetLayout(), nullptr, nullptr);
    if (!pFrame)
        return;
    pFlyFrame = pFrame->FindFlyFrame();
    if ( !pFlyFrame || m_rShell.IsTableMode() )
        return;

    const SwNodeIndex* pIndex = pFlyFrame->GetFormat()->GetContent().GetContentIdx();
    OSL_ENSURE( pIndex, "Fly without Content" );

    if (!pIndex)
        return;

    const SwNode& rStNd = pIndex->GetNode();

    if( rStNd.EndOfSectionNode()->StartOfSectionIndex() > m_nNode ||
        m_nNode > rStNd.EndOfSectionIndex() )
        m_rShell.GetFlyMacroLnk().Call( pFlyFrame->GetFormat() );
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
            for (;;)
            {
                pNext = pFrame->GetFollow();
                if(!pNext || nPos < pNext->GetOffset())
                    break;
                pFrame = pNext;
            }
        }

        return pFrame->getFrameArea().Left();
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
