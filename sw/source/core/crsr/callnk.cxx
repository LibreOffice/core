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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <hintids.hxx>

#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif
#include <fmtcntnt.hxx>
#include <txatbase.hxx>
#include <frmatr.hxx>
#include <viscrs.hxx>
#include <callnk.hxx>
#include <crsrsh.hxx>
#include <doc.hxx>
#include <frmfmt.hxx>
#include <txtfrm.hxx>
#include <ndtxt.hxx>
#include <flyfrm.hxx>
#include <breakit.hxx>


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
    // SPoint-Werte vom aktuellen Cursor merken
    SwPaM* pCrsr = rShell.IsTableMode() ? rShell.GetTblCrs() : rShell.GetCrsr();
    SwNode& rNd = pCrsr->GetPoint()->nNode.GetNode();
    nNode = rNd.GetIndex();
    nCntnt = pCrsr->GetPoint()->nContent.GetIndex();
    nNdTyp = rNd.GetNodeType();
    bHasSelection = ( *pCrsr->GetPoint() != *pCrsr->GetMark() );

    if( ND_TEXTNODE & nNdTyp )
        nLeftFrmPos = SwCallLink::getLayoutFrm( rShell.GetLayout(), (SwTxtNode&)rNd, nCntnt,
                                            !rShell.ActionPend() );
    else
    {
        nLeftFrmPos = 0;

        // eine Sonderbehandlung fuer die SwFeShell: diese setzt beim Loeschen
        // der Kopf-/Fusszeile, Fussnoten den Cursor auf NULL (Node + Content)
        // steht der Cursor auf keinem CntntNode, wird sich das im NdType
        // gespeichert.
        if( ND_CONTENTNODE & nNdTyp )
            nNdTyp = 0;
    }
}


SwCallLink::~SwCallLink()
{
    if( !nNdTyp || !rShell.bCallChgLnk )        // siehe ctor
        return ;

    // wird ueber Nodes getravellt, Formate ueberpruefen und im neuen
    // Node wieder anmelden
    SwPaM* pCurCrsr = rShell.IsTableMode() ? rShell.GetTblCrs() : rShell.GetCrsr();
    SwCntntNode * pCNd = pCurCrsr->GetCntntNode();
    if( !pCNd )
        return;

    xub_StrLen nCmp, nAktCntnt = pCurCrsr->GetPoint()->nContent.GetIndex();
    sal_uInt16 nNdWhich = pCNd->GetNodeType();
    sal_uLong nAktNode = pCurCrsr->GetPoint()->nNode.GetIndex();

    // melde die Shell beim akt. Node als abhaengig an, dadurch koennen
    // alle Attribut-Aenderungen ueber den Link weiter gemeldet werden.
    pCNd->Add( &rShell );

    if( nNdTyp != nNdWhich || nNode != nAktNode )
    {
        /* immer, wenn zwischen Nodes gesprungen wird, kann es
         * vorkommen, das neue Attribute gelten; die Text-Attribute.
         * Es muesste also festgestellt werden, welche Attribute
         * jetzt gelten; das kann auch gleich der Handler machen
         */
        rShell.CallChgLnk();
    }
    else if( !bHasSelection != !(*pCurCrsr->GetPoint() != *pCurCrsr->GetMark()) )
    {
        // always call change link when selection changes
        rShell.CallChgLnk();
    }
    else if( rShell.aChgLnk.IsSet() && ND_TEXTNODE == nNdWhich &&
             nCntnt != nAktCntnt )
    {
        // nur wenn mit Left/right getravellt, dann Text-Hints pruefen
        // und sich nicht der Frame geaendert hat (Spalten!)
        if( nLeftFrmPos == SwCallLink::getLayoutFrm( rShell.GetLayout(), (SwTxtNode&)*pCNd, nAktCntnt,
                                                    !rShell.ActionPend() ) &&
            (( nCmp = nCntnt ) + 1 == nAktCntnt ||          // Right
            nCntnt -1 == ( nCmp = nAktCntnt )) )            // Left
        {
            if( nCmp == nAktCntnt && pCurCrsr->HasMark() ) // left & Sele
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

                    // nur Start oder Start und Ende gleich, dann immer
                    // beim Ueberlaufen von Start callen
                    if( ( !pEnd || ( nStart == *pEnd ) ) &&
                        ( nStart == nCntnt || nStart == nAktCntnt) )
                    {
                        rShell.CallChgLnk();
                        return;
                    }

                    // hat das Attribut einen Bereich und dieser nicht leer
                    else if( pEnd && nStart < *pEnd &&
                        // dann teste, ob ueber Start/Ende getravellt wurde
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

            if( pBreakIt->GetBreakIter().is() )
            {
                const String& rTxt = ((SwTxtNode*)pCNd)->GetTxt();
                if( !nCmp ||
                    pBreakIt->GetBreakIter()->getScriptType( rTxt, nCmp )
                     != pBreakIt->GetBreakIter()->getScriptType( rTxt, nCmp - 1 ))
                {
                    rShell.CallChgLnk();
                    return;
                }
            }
        }
        else
            /* wenn mit Home/End/.. mehr als 1 Zeichen getravellt, dann
             * immer den ChgLnk rufen, denn es kann hier nicht
             * festgestellt werden, was sich geaendert; etwas kann
             * veraendert sein.
             */
            rShell.CallChgLnk();
    }

    const SwFrm* pFrm;
    const SwFlyFrm *pFlyFrm;
    if( !rShell.ActionPend() && 0 != ( pFrm = pCNd->getLayoutFrm(rShell.GetLayout(),0,0,sal_False) ) &&
        0 != ( pFlyFrm = pFrm->FindFlyFrm() ) && !rShell.IsTableMode() )
    {
        const SwNodeIndex* pIndex = pFlyFrm->GetFmt()->GetCntnt().GetCntntIdx();
        ASSERT( pIndex, "Fly ohne Cntnt" );

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

