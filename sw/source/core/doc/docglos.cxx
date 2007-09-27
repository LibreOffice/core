/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docglos.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:35:50 $
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
        BOOL bSav_IsInsGlossary = mbInsOnlyTxtGlssry;
        mbInsOnlyTxtGlssry = rBlock.IsOnlyTextBlock( nIdx );

        if( rBlock.BeginGetDoc( nIdx ) )
        {
            SwDoc* pGDoc = rBlock.GetDoc();

            // alle FixFelder aktualisieren. Dann aber auch mit der
            // richtigen DocInfo!
            pGDoc->SetDocumentInfo( *GetDocumentInfo() );
            pGDoc->SetFixFields(false, NULL);

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

            StartUndo( UNDO_INSGLOSSARY, NULL );
            SwPaM *_pStartCrsr = &rPaM, *__pStartCrsr = _pStartCrsr;
            do {

                SwPosition& rInsPos = *_pStartCrsr->GetPoint();
                SwStartNode* pBoxSttNd = (SwStartNode*)rInsPos.nNode.GetNode().
                                            FindTableBoxStartNode();

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
            } while( (_pStartCrsr=(SwPaM *)_pStartCrsr->GetNext()) !=
                        __pStartCrsr );
            EndUndo( UNDO_INSGLOSSARY, NULL );

            UnlockExpFlds();
            if( !IsExpFldsLocked() )
                UpdateExpFlds(NULL, true);
            bRet = TRUE;
        }
        mbInsOnlyTxtGlssry = bSav_IsInsGlossary;
    }
    rBlock.EndGetDoc();
    return bRet;
}


