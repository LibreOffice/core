/*************************************************************************
 *
 *  $RCSfile: callnk.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-15 13:42:41 $
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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _TXATBASE_HXX //autogen
#include <txatbase.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _VISCRS_HXX
#include <viscrs.hxx>
#endif
#ifndef _CALLNK_HXX
#include <callnk.hxx>
#endif
#ifndef _CRSRSH_HXX
#include <crsrsh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif


SwCallLink::SwCallLink( SwCrsrShell & rSh, ULONG nAktNode, xub_StrLen nAktCntnt,
                        BYTE nAktNdTyp, long nLRPos )
    : rShell( rSh ), nNode( nAktNode ), nCntnt( nAktCntnt ),
    nNdTyp( nAktNdTyp ), nLeftFrmPos( nLRPos )
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

    if( ND_TEXTNODE & nNdTyp )
        nLeftFrmPos = SwCallLink::GetFrm( (SwTxtNode&)rNd, nCntnt,
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
    USHORT nNdWhich = pCNd->GetNodeType();
    ULONG nAktNode = pCurCrsr->GetPoint()->nNode.GetIndex();

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
    else if( rShell.aChgLnk.IsSet() && ND_TEXTNODE == nNdWhich &&
             nCntnt != nAktCntnt )
    {
        // nur wenn mit Left/right getravellt, dann Text-Hints pruefen
        // und sich nicht der Frame geaendert hat (Spalten!)
        if( nLeftFrmPos == SwCallLink::GetFrm( (SwTxtNode&)*pCNd, nAktCntnt,
                                                    !rShell.ActionPend() ) &&
            (( nCmp = nCntnt ) + 1 == nAktCntnt ||          // Right
            nCntnt -1 == ( nCmp = nAktCntnt )) )            // Left
        {
            if ( ((SwTxtNode*)pCNd)->HasHints() )
            {
                if( nCmp == nAktCntnt && pCurCrsr->HasMark() ) // left & Sele
                    ++nCmp;

                const SwpHints &rHts = ((SwTxtNode*)pCNd)->GetSwpHints();
                USHORT n;
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

            if( pBreakIt->xBreak.is() )
            {
                const String& rTxt = ((SwTxtNode*)pCNd)->GetTxt();
                if( pBreakIt->xBreak->getScriptType( rTxt, nCntnt )
                     != pBreakIt->xBreak->getScriptType( rTxt, nAktCntnt ))
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
    if( !rShell.ActionPend() && 0 != ( pFrm = pCNd->GetFrm(0,0,FALSE) ) &&
        0 != ( pFlyFrm = pFrm->FindFlyFrm() ) && !rShell.IsTableMode() )
    {
        const SwNodeIndex* pIndex = pFlyFrm->GetFmt()->GetCntnt().GetCntntIdx();
        ASSERT( pIndex, "Fly ohne Cntnt" );
        const SwNode& rStNd = pIndex->GetNode();

        if( rStNd.EndOfSectionNode()->StartOfSectionIndex() > nNode ||
            nNode > rStNd.EndOfSectionIndex() )
            rShell.GetFlyMacroLnk().Call( (void*)pFlyFrm->GetFmt() );
    }
}

long SwCallLink::GetFrm( SwTxtNode& rNd, xub_StrLen nCntPos, BOOL bCalcFrm )
{
    SwTxtFrm* pFrm = (SwTxtFrm*)rNd.GetFrm(0,0,bCalcFrm), *pNext = pFrm;
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

/*---------------------------------------------------------------------*/


SwChgLinkFlag::SwChgLinkFlag( SwCrsrShell& rShell )
    : rCrsrShell( rShell ), bOldFlag( rShell.bCallChgLnk ), nLeftFrmPos( 0 )
{
    rCrsrShell.bCallChgLnk = FALSE;
    if( bOldFlag && !rCrsrShell.pTblCrsr )
    {
        SwNode* pNd = rCrsrShell.pCurCrsr->GetNode();
        if( ND_TEXTNODE & pNd->GetNodeType() )
            nLeftFrmPos = SwCallLink::GetFrm( (SwTxtNode&)*pNd,
                    rCrsrShell.pCurCrsr->GetPoint()->nContent.GetIndex(),
                    !rCrsrShell.ActionPend() );
    }
}


SwChgLinkFlag::~SwChgLinkFlag()
{
    rCrsrShell.bCallChgLnk = bOldFlag;
    if( bOldFlag && !rCrsrShell.pTblCrsr )
    {
        // die Spalten Ueberwachung brauchen wir immer!!!
        SwNode* pNd = rCrsrShell.pCurCrsr->GetNode();
        if( ND_TEXTNODE & pNd->GetNodeType() &&
            nLeftFrmPos != SwCallLink::GetFrm( (SwTxtNode&)*pNd,
                    rCrsrShell.pCurCrsr->GetPoint()->nContent.GetIndex(),
                    !rCrsrShell.ActionPend() ))
        {
            /* immer, wenn zwischen Frames gesprungen wird, gelten
             * neue Attribute. Es muesste also festgestellt werden, welche
             * Attribute jetzt gelten; das kann gleich der Handler machen.
             * Diesen direkt rufen !!!
             */
            rCrsrShell.aChgLnk.Call( &rCrsrShell );
            rCrsrShell.bChgCallFlag = FALSE;        // Flag zuruecksetzen
        }
    }
}




