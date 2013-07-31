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

#include<vcl/window.hxx>


SwCallLink::SwCallLink( SwCrsrShell & rSh, sal_uLong nAktNode, xub_StrLen nAktCntnt,
                        sal_uInt8 nAktNdTyp, long nLRPos, bool bAktSelection )
    : rShell( rSh ), nNode( nAktNode ), nCntnt( nAktCntnt ),
      nNdTyp( nAktNdTyp ), nLeftFrmPos( nLRPos ),
      bHasSelection( bAktSelection )
{
}


SwCallLink::SwCallLink( SwCrsrShell & rSh )
    : rShell( rSh )
{
    // remember SPoint-values of current cursor
    SwPaM* pCrsr = rShell.IsTableMode() ? rShell.GetTblCrs() : rShell.GetCrsr();
    SwNode& rNd = pCrsr->GetPoint()->nNode.GetNode();
    nNode = rNd.GetIndex();
    nCntnt = pCrsr->GetPoint()->nContent.GetIndex();
    nNdTyp = rNd.GetNodeType();
    bHasSelection = ( *pCrsr->GetPoint() != *pCrsr->GetMark() );

    if( rNd.IsTxtNode() )
        nLeftFrmPos = SwCallLink::getLayoutFrm( rShell.GetLayout(), (SwTxtNode&)rNd, nCntnt,
                                            !rShell.ActionPend() );
    else
    {
        nLeftFrmPos = 0;

        // A special treatment for SwFeShell:
        // When deleting the header/footer, footnotes SwFeShell sets the
        // Cursor to NULL (Node + Content).
        // If the Cursor is not on a CntntNode (ContentNode) this fact gets
        // saved in NdType.
        if( ND_CONTENTNODE & nNdTyp )
            nNdTyp = 0;
    }
}

static void lcl_notifyRow(const SwCntntNode* pNode, SwCrsrShell& rShell)
{
    if ( pNode != NULL )
    {
        SwFrm *myFrm = pNode->getLayoutFrm( rShell.GetLayout() );
        if (myFrm!=NULL)
        {
            // We need to emulated a change of the row height in order
            // to have the complete row redrawn
            SwRowFrm* pRow = myFrm->FindRowFrm();
            if ( pRow )
            {
                const SwTableLine* pLine = pRow->GetTabLine( );
                // Avoid redrawing the complete row if there are no nested tables
                bool bHasTable = false;
                SwFrm *pCell = pRow->GetLower();
                for (; pCell && !bHasTable; pCell = pCell->GetNext())
                {
                    SwFrm *pContent = pCell->GetLower();
                    for (; pContent && !bHasTable; pContent = pContent->GetNext())
                        if (pContent->GetType() == FRM_TAB)
                            bHasTable = true;
                }
                if (bHasTable)
                {
                    SwFmtFrmSize pSize = pLine->GetFrmFmt()->GetFrmSize();
                    pRow->ModifyNotification(NULL, &pSize);
                }
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
    SwPaM* pCurCrsr = rShell.IsTableMode() ? rShell.GetTblCrs() : rShell.GetCrsr();
    SwCntntNode * pCNd = pCurCrsr->GetCntntNode();
    if( !pCNd )
        return;

    lcl_notifyRow(pCNd, rShell);

    const SwDoc *pDoc=rShell.GetDoc();
    const SwCntntNode *pNode = NULL;
    if ( ( pDoc != NULL && nNode < pDoc->GetNodes( ).Count( ) ) )
    {
        pNode = pDoc->GetNodes()[nNode]->GetCntntNode();
    }
    lcl_notifyRow(pNode, rShell);

    xub_StrLen nCmp, nAktCntnt = pCurCrsr->GetPoint()->nContent.GetIndex();
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
             nCntnt != nAktCntnt )
    {
        // If travelling with left/right only and the frame is
        // unchanged (columns!) then check text hints.
        if( nLeftFrmPos == SwCallLink::getLayoutFrm( rShell.GetLayout(), (SwTxtNode&)*pCNd, nAktCntnt,
                                                    !rShell.ActionPend() ) &&
            (( nCmp = nCntnt ) + 1 == nAktCntnt ||          // Right
            nCntnt -1 == ( nCmp = nAktCntnt )) )            // Left
        {
            if( nCmp == nAktCntnt && pCurCrsr->HasMark() ) // left & select
                ++nCmp;
            if ( ((SwTxtNode*)pCNd)->HasHints() )
            {

                const SwpHints &rHts = ((SwTxtNode*)pCNd)->GetSwpHints();
                sal_uInt16 n;
                xub_StrLen nStart;
                const xub_StrLen *pEnd;

                for( n = 0; n < rHts.Count(); n++ )
                {
                    const SwTxtAttr* pHt = rHts[ n ];
                    pEnd = pHt->GetEnd();
                    nStart = *pHt->GetStart();

                    // If "only start" or "start and end equal" then call on
                    // every overflow of start.
                    if( ( !pEnd || ( nStart == *pEnd ) ) &&
                        ( nStart == nCntnt || nStart == nAktCntnt) )
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
                    nStart = 0;
                }
            }

            if( g_pBreakIt->GetBreakIter().is() )
            {
                const OUString rTxt = ((SwTxtNode*)pCNd)->GetTxt();
                if( !nCmp ||
                    g_pBreakIt->GetBreakIter()->getScriptType( rTxt, nCmp )
                     != g_pBreakIt->GetBreakIter()->getScriptType( rTxt, nCmp - 1 ))
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
    if( !rShell.ActionPend() && 0 != ( pFrm = pCNd->getLayoutFrm(rShell.GetLayout(),0,0,sal_False) ) &&
        0 != ( pFlyFrm = pFrm->FindFlyFrm() ) && !rShell.IsTableMode() )
    {
        const SwNodeIndex* pIndex = pFlyFrm->GetFmt()->GetCntnt().GetCntntIdx();
        OSL_ENSURE( pIndex, "Fly without Cntnt" );

        if (!pIndex)
            return;

        const SwNode& rStNd = pIndex->GetNode();

        if( rStNd.EndOfSectionNode()->StartOfSectionIndex() > nNode ||
            nNode > rStNd.EndOfSectionIndex() )
            rShell.GetFlyMacroLnk().Call( (void*)pFlyFrm->GetFmt() );
    }
}

long SwCallLink::getLayoutFrm( const SwRootFrm* pRoot, SwTxtNode& rNd, xub_StrLen nCntPos, sal_Bool bCalcFrm )
{
    SwTxtFrm* pFrm = (SwTxtFrm*)rNd.getLayoutFrm(pRoot,0,0,bCalcFrm), *pNext = pFrm;
    if ( pFrm && !pFrm->IsHiddenNow() )
    {
        if( pFrm->HasFollow() )
            while( 0 != ( pNext = (SwTxtFrm*)pFrm->GetFollow() ) &&
                    nCntPos >= pNext->GetOfst() )
                pFrm = pNext;

        return pFrm->Frm().Left();
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
