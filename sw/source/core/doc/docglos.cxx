/*************************************************************************
 *
 *  $RCSfile: docglos.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:15 $
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
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _ACORRECT_HXX
#include <acorrect.hxx>
#endif
#ifndef _CRSRSH_HXX
#include <crsrsh.hxx>
#endif

/* -----------------22.07.99 11:47-------------------
    Description: inserts an AutoText block
 --------------------------------------------------*/
BOOL SwDoc::InsertGlossary( SwTextBlocks& rBlock, const String& rEntry,
                            SwPaM& rPaM, SwCrsrShell* pShell )
{
    BOOL bRet = FALSE;
    USHORT nIdx = rBlock.GetIndex( rEntry );
    if( (USHORT) -1 != nIdx )
    {
        // Bug #70238# ask the TextOnly-Flag before BeginGetDoc, because
        //              the method closed the Storage!
        BOOL bSav_IsInsGlossary = bInsOnlyTxtGlssry;
        bInsOnlyTxtGlssry = rBlock.IsOnlyTextBlock( nIdx );

        if( rBlock.BeginGetDoc( nIdx ) )
        {
            SwDoc* pGDoc = rBlock.GetDoc();

            // alle FixFelder aktualisieren. Dann aber auch mit der
            // richtigen DocInfo!
            pGDoc->SetInfo( *GetInfo() );
            pGDoc->SetFixFields();

            //StartAllAction();
            LockExpFlds();

            SwNodeIndex aStt( pGDoc->GetNodes().GetEndOfExtras(), 1 );
            SwCntntNode* pCntntNd = pGDoc->GetNodes().GoNext( &aStt );
            const SwTableNode* pTblNd = pCntntNd->FindTableNode();
            SwPaM aCpyPam( pTblNd ? *(SwNode*)pTblNd : *(SwNode*)pCntntNd );
            aCpyPam.SetMark();

            // dann bis zum Ende vom Nodes Array
            aCpyPam.GetPoint()->nNode = pGDoc->GetNodes().GetEndOfContent().GetIndex()-1;
            pCntntNd = aCpyPam.GetCntntNode();
            aCpyPam.GetPoint()->nContent.Assign( pCntntNd, pCntntNd->Len() );

                // steht nur eine Tabelle im Autotext? Dann kopiere ggfs.
                // Tabelle in Tabelle
            BOOL bChkTblInTbl = pTblNd &&
                                !aCpyPam.GetPoint()->nContent.GetIndex() &&
                                pTblNd->EndOfSectionIndex() + 1 ==
                                aCpyPam.GetPoint()->nNode.GetIndex();

            StartUndo( UNDO_INSGLOSSARY );
            SwPaM *_pStartCrsr = &rPaM, *__pStartCrsr = _pStartCrsr;
            do {

                const SwTxtNode* pTNd;
                SwPosition& rInsPos = *_pStartCrsr->GetPoint();
                SwStartNode* pBoxSttNd = (SwStartNode*)rInsPos.nNode.GetNode().
                                            FindTableBoxStartNode();

                // nur wenn die Box keinen Inhalt hat
                if( bChkTblInTbl && pBoxSttNd &&
                    2 == pBoxSttNd->EndOfSectionIndex() - pBoxSttNd->GetIndex()
                    && 0 != ( pTNd = GetNodes()[ pBoxSttNd->GetIndex() + 1]
                        ->GetTxtNode() ) && !pTNd->GetTxt().Len() )
                {
                    CopyTblInTbl( pTblNd->GetTable(),
                                        pBoxSttNd->FindTableNode()->GetTable(),
                                        SwNodeIndex( *pBoxSttNd ));
                }
                else
                {
                    if( pBoxSttNd && 2 == pBoxSttNd->EndOfSectionIndex() -
                                          pBoxSttNd->GetIndex() &&
                        aCpyPam.GetPoint()->nNode != aCpyPam.GetMark()->nNode )
                    {
                        // es wird mehr als 1 Node in die akt. Box kopiert.
                        // Dann muessen die BoxAttribute aber entfernt werden.
                        ClearBoxNumAttrs( rInsPos.nNode );
                    }

                    SwDontExpandItem aACD;
                    aACD.SaveDontExpandItems( rInsPos );

                    pGDoc->Copy( aCpyPam, rInsPos );

                    aACD.RestoreDontExpandItems( rInsPos );
                    if( pShell )
                        pShell->SaveTblBoxCntnt( &rInsPos );
                }
            } while( (_pStartCrsr=(SwPaM *)_pStartCrsr->GetNext()) !=
                        __pStartCrsr );
            EndUndo( UNDO_INSGLOSSARY );

            UnlockExpFlds();
            if( !IsExpFldsLocked() )
                UpdateExpFlds();
            bRet = TRUE;
        }
        bInsOnlyTxtGlssry = bSav_IsInsGlossary;
    }
    rBlock.EndGetDoc();
    return bRet;
}


