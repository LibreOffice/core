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

SwCallLink::SwCallLink( SwCrsrShell & rSh, sal_uLong nAktNode, sal_Int32 nAktContent,
                        sal_uInt8 nAktNdTyp, long nLRPos, bool bAktSelection )
    : rShell( rSh ), nNode( nAktNode ), nContent( nAktContent ),
      nNdTyp( nAktNdTyp ), nLeftFrmPos( nLRPos ),
      bHasSelection( bAktSelection )
{
}

SwCallLink::SwCallLink( SwCrsrShell & rSh )
    : rShell( rSh )
{
    // remember SPoint-values of current cursor
    SwPaM* pCrsr = rShell.IsTableMode() ? rShell.GetTableCrs() : rShell.GetCrsr();
    SwNode& rNd = pCrsr->GetPoint()->nNode.GetNode();
    nNode = rNd.GetIndex();
    nContent = pCrsr->GetPoint()->nContent.GetIndex();
    nNdTyp = rNd.GetNodeType();
    bHasSelection = ( *pCrsr->GetPoint() != *pCrsr->GetMark() );

    if( rNd.IsTextNode() )
        nLeftFrmPos = SwCallLink::getLayoutFrm( rShell.GetLayout(), *rNd.GetTextNode(), nContent,
                                            !rShell.ActionPend() );
    else
    {
        nLeftFrmPos = 0;

        // A special treatment for SwFeShell:
        // When deleting the header/footer, footnotes SwFeShell sets the
        // Cursor to NULL (Node + Content).
        // If the Cursor is not on a ContentNode (ContentNode) this fact gets
        // saved in NdType.
        if( ND_CONTENTNODE & nNdTyp )
            nNdTyp = 0;
    }
}

static void lcl_notifyRow(const SwContentNode* pNode, SwCrsrShell& rShell)
{
    if ( !pNode )
        return;

    SwFrm *const pMyFrm = pNode->getLayoutFrm( rShell.GetLayout() );
    if ( !pMyFrm )
        return;

    // We need to emulated a change of the row height in order
    // to have the complete row redrawn
    SwRowFrm *const pRow = pMyFrm->FindRowFrm();
    if ( !pRow )
        return;

    const SwTableLine* pLine = pRow->GetTabLine( );
    // Avoid redrawing the complete row if there are no nested tables
    for (SwFrm *pCell = pRow->GetLower(); pCell; pCell = pCell->GetNext())
    {
        for (SwFrm *pContent = pCell->GetLower(); pContent; pContent = pContent->GetNext())
        {
            if (pContent->GetType() == FRM_TAB)
            {
                SwFormatFrmSize pSize = pLine->GetFrameFormat()->GetFrmSize();
                pRow->ModifyNotification(nullptr, &pSize);
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
    SwPaM* pCurCrsr = rShell.IsTableMode() ? rShell.GetTableCrs() : rShell.GetCrsr();
    SwContentNode * pCNd = pCurCrsr->GetContentNode();
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

    sal_Int32 nCmp, nAktContent = pCurCrsr->GetPoint()->nContent.GetIndex();
    sal_uInt16 nNdWhich = pCNd->GetNodeType();
    sal_uLong nAktNode = pCurCrsr->GetPoint()->nNode.GetIndex();

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
    else if( !bHasSelection != !(*pCurCrsr->GetPoint() != *pCurCrsr->GetMark()) )
    {
        // always call change link when selection changes
        rShell.CallChgLnk();
    }
    else if( rShell.m_aChgLnk.IsSet() && ND_TEXTNODE == nNdWhich &&
             nContent != nAktContent )
    {
        // If travelling with left/right only and the frame is
        // unchanged (columns!) then check text hints.
        if( nLeftFrmPos == SwCallLink::getLayoutFrm( rShell.GetLayout(), *pCNd->GetTextNode(), nAktContent,
                                                    !rShell.ActionPend() ) &&
            (( nCmp = nContent ) + 1 == nAktContent ||          // Right
            nContent -1 == ( nCmp = nAktContent )) )            // Left
        {
            if( nCmp == nAktContent && pCurCrsr->HasMark() ) // left & select
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
                    g_pBreakIt->GetBreakIter()->getScriptType( rText, nCmp )
                     != g_pBreakIt->GetBreakIter()->getScriptType( rText, nCmp - 1 ))
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

    const SwFrm* pFrm;
    const SwFlyFrm *pFlyFrm;
    if( !rShell.ActionPend() && nullptr != ( pFrm = pCNd->getLayoutFrm(rShell.GetLayout(), nullptr, nullptr, false) ) &&
        nullptr != ( pFlyFrm = pFrm->FindFlyFrm() ) && !rShell.IsTableMode() )
    {
        const SwNodeIndex* pIndex = pFlyFrm->GetFormat()->GetContent().GetContentIdx();
        OSL_ENSURE( pIndex, "Fly without Content" );

        if (!pIndex)
            return;

        const SwNode& rStNd = pIndex->GetNode();

        if( rStNd.EndOfSectionNode()->StartOfSectionIndex() > nNode ||
            nNode > rStNd.EndOfSectionIndex() )
            rShell.GetFlyMacroLnk().Call( pFlyFrm->GetFormat() );
    }
}

long SwCallLink::getLayoutFrm( const SwRootFrm* pRoot, SwTextNode& rNd, sal_Int32 nCntPos, bool bCalcFrm )
{
    SwTextFrm* pFrm = static_cast<SwTextFrm*>(rNd.getLayoutFrm(pRoot,nullptr,nullptr,bCalcFrm));
    SwTextFrm* pNext;
    if ( pFrm && !pFrm->IsHiddenNow() )
    {
        if( pFrm->HasFollow() )
            while( nullptr != ( pNext = pFrm->GetFollow() ) &&
                    nCntPos >= pNext->GetOfst() )
                pFrm = pNext;

        return pFrm->Frm().Left();
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
