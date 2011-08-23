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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _TXTFTN_HXX
#include <txtftn.hxx>
#endif
#ifndef _FMTFTN_HXX
#include <fmtftn.hxx>
#endif
#ifndef _FTNINFO_HXX
#include <ftninfo.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _FTNIDX_HXX
#include <ftnidx.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _NDINDEX_HXX
#include <ndindex.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _FMTFTNTX_HXX
#include <fmtftntx.hxx>
#endif
namespace binfilter {


/*?*/ _SV_IMPL_SORTAR_ALG( _SwFtnIdxs, SwTxtFtnPtr )
/*?*/ BOOL _SwFtnIdxs::Seek_Entry( const SwTxtFtnPtr rSrch, USHORT* pFndPos ) const
/*?*/ {
/*?*/ 	ULONG nIdx = _SwTxtFtn_GetIndex( rSrch );
/*?*/ 	xub_StrLen nCntIdx = *rSrch->GetStart();
/*?*/ 
/*?*/ 	register USHORT nO = Count(), nM, nU = 0;
/*?*/ 	if( nO > 0 )
/*?*/ 	{
/*?*/ 		nO--;
/*?*/ 		while( nU <= nO )
/*?*/ 		{
/*?*/ 			nM = nU + ( nO - nU ) / 2;
/*?*/ 			ULONG nFndIdx = _SwTxtFtn_GetIndex( (*this)[ nM ] );
/*?*/ 			if( nFndIdx == nIdx && *(*this)[ nM ]->GetStart() == nCntIdx )
/*?*/ 			{
/*?*/ 				if( pFndPos )
/*?*/ 					*pFndPos = nM;
/*?*/ 				return TRUE;
/*?*/ 			}
/*?*/ 			else if( nFndIdx < nIdx ||
/*?*/ 				(nFndIdx == nIdx && *(*this)[ nM ]->GetStart() < nCntIdx ))
/*?*/ 				nU = nM + 1;
/*?*/ 			else if( nM == 0 )
/*?*/ 			{
/*?*/ 				if( pFndPos )
/*?*/ 					*pFndPos = nU;
/*?*/ 				return FALSE;
/*?*/ 			}
/*?*/ 			else
/*?*/ 				nO = nM - 1;
/*?*/ 		}
/*?*/ 	}
/*?*/ 	if( pFndPos )
/*?*/ 		*pFndPos = nU;
/*?*/ 	return FALSE;
/*?*/ }


/*N*/ void SwFtnIdxs::UpdateFtn( const SwNodeIndex& rStt )
/*N*/ {
/*N*/ 	if( !Count() )
/*N*/ 		return;
/*N*/ 
/*N*/ 	// besorge erstmal das Nodes-Array ueber den StartIndex der ersten Fussnote
/*N*/ 	SwDoc* pDoc = rStt.GetNode().GetDoc();
/*N*/ 	if( pDoc->IsInReading() )
/*N*/ 		return ;
/*N*/ 	SwTxtFtn* pTxtFtn;
/*N*/ 
/*N*/ 	const SwEndNoteInfo& rEndInfo = pDoc->GetEndNoteInfo();
/*N*/ 	const SwFtnInfo& rFtnInfo = pDoc->GetFtnInfo();
/*N*/ 
/*N*/ 	//Fuer normale Fussnoten werden Chapter- und Dokumentweise Nummerierung
/*N*/ 	//getrennt behandelt. Fuer Endnoten gibt es nur die Dokumentweise
/*N*/ 	//Nummerierung.
/*N*/ 	if( FTNNUM_CHAPTER == rFtnInfo.eNum )
/*N*/ 	{
/*?*/ 		const SwOutlineNodes& rOutlNds = pDoc->GetNodes().GetOutLineNds();
/*?*/ 		const SwNode* pCapStt = &pDoc->GetNodes().GetEndOfExtras();
/*?*/ 		ULONG nCapEnd = pDoc->GetNodes().GetEndOfContent().GetIndex();
/*?*/ 		if( rOutlNds.Count() )
/*?*/ 		{
/*?*/ 			// suche den Start des Kapitels, in den rStt steht.
                USHORT n=0;
/*?*/ 			for( n = 0; n < rOutlNds.Count(); ++n )
/*?*/ 				if( rOutlNds[ n ]->GetIndex() > rStt.GetIndex() )
/*?*/ 					break;		// gefunden
/*?*/ 				else if( !rOutlNds[ n ]->GetTxtNode()->GetTxtColl()->GetOutlineLevel() )
/*?*/ 					pCapStt = rOutlNds[ n ];	// Start eines neuen Kapitels
/*?*/ 			// dann suche jetzt noch das Ende vom Bereich
/*?*/ 			for( ; n < rOutlNds.Count(); ++n )
/*?*/ 				if( !rOutlNds[ n ]->GetTxtNode()->GetTxtColl()->GetOutlineLevel() )
/*?*/ 				{
/*?*/ 					nCapEnd = rOutlNds[ n ]->GetIndex();	// Ende des gefundenen Kapitels
/*?*/ 					break;
/*?*/ 				}
/*?*/ 		}
/*?*/ 
/*?*/ 		USHORT nPos, nFtnNo = 1;
/*?*/ 		if( SeekEntry( *pCapStt, &nPos ) && nPos )
/*?*/ 		{
/*?*/ 			// gehe nach vorne bis der Index nicht mehr gleich ist
/*?*/ 			const SwNode* pCmpNd = &rStt.GetNode();
/*?*/ 			while( nPos && pCmpNd == &((*this)[ --nPos ]->GetTxtNode()) )
/*?*/ 				;
/*?*/ 			++nPos;
/*?*/ 		}
/*?*/ 
/*?*/ 		if( nPos == Count() )		// nichts gefunden
/*?*/ 			return;
/*?*/ 
/*?*/ 		if( !rOutlNds.Count() )
/*?*/ 			nFtnNo = nPos+1;
/*?*/ 
/*?*/ 		for( ; nPos < Count(); ++nPos )
/*?*/ 		{
/*?*/ 			pTxtFtn = (*this)[ nPos ];
/*?*/ 			if( pTxtFtn->GetTxtNode().GetIndex() >= nCapEnd )
/*?*/ 				break;
/*?*/ 
/*?*/ 			const SwFmtFtn &rFtn = pTxtFtn->GetFtn();
/*?*/ 			if( !rFtn.GetNumStr().Len() && !rFtn.IsEndNote() &&
/*?*/ 				!SwUpdFtnEndNtAtEnd::FindSectNdWithEndAttr( *pTxtFtn ))
/*?*/ 				pTxtFtn->SetNumber( rFtnInfo.nFtnOffset + nFtnNo++,
/*?*/ 									&rFtn.GetNumStr() );
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	SwUpdFtnEndNtAtEnd aNumArr;
/*N*/ 
/*N*/ 	// BOOL, damit hier auch bei Chapter-Einstellung die Endnoten
/*N*/ 	// durchlaufen.
/*N*/ 	const FASTBOOL bEndNoteOnly = FTNNUM_DOC != rFtnInfo.eNum;
/*N*/ 
/*N*/ 	USHORT nPos, nFtnNo = 1, nEndNo = 1;
/*N*/ 	ULONG nUpdNdIdx = rStt.GetIndex();
/*N*/ 	for( nPos = 0; nPos < Count(); ++nPos )
/*N*/ 	{
/*N*/ 		pTxtFtn = (*this)[ nPos ];
/*N*/ 		if( nUpdNdIdx <= pTxtFtn->GetTxtNode().GetIndex() )
/*N*/ 			break;
/*N*/ 
/*?*/ 		const SwFmtFtn &rFtn = pTxtFtn->GetFtn();
/*?*/ 		if( !rFtn.GetNumStr().Len() )
/*?*/ 		{
/*?*/ 			if( !aNumArr.ChkNumber( *pTxtFtn ) )
/*?*/ 			{
/*?*/ 				if( pTxtFtn->GetFtn().IsEndNote() )
/*?*/ 					nEndNo++;
/*?*/ 				else
/*?*/ 					nFtnNo++;
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// ab nPos bei allen FootNotes die Array-Nummer setzen
/*N*/ 	for( ; nPos < Count(); ++nPos )
/*N*/ 	{
/*N*/ 		pTxtFtn = (*this)[ nPos ];
/*N*/ 		const SwFmtFtn &rFtn = pTxtFtn->GetFtn();
/*N*/ 		if( !rFtn.GetNumStr().Len() )
/*N*/ 		{
/*N*/ 			USHORT nSectNo = aNumArr.ChkNumber( *pTxtFtn );
/*N*/ 			if( !nSectNo && ( rFtn.IsEndNote() || !bEndNoteOnly ))
/*N*/ 				nSectNo = rFtn.IsEndNote()
/*N*/ 							? rEndInfo.nFtnOffset + nEndNo++
/*N*/ 							: rFtnInfo.nFtnOffset + nFtnNo++;
/*N*/ 
/*N*/ 			if( nSectNo )
/*N*/ 			{
/*N*/ 				if( rFtn.IsEndNote() )
/*?*/ 					pTxtFtn->SetNumber( nSectNo, &rFtn.GetNumStr() );
/*N*/ 				else
/*N*/ 					pTxtFtn->SetNumber( nSectNo, &rFtn.GetNumStr() );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	// Pageweise wird vom MA erfuellt !!
/*N*/ }


/*N*/ void SwFtnIdxs::UpdateAllFtn()
/*N*/ {
/*N*/ 	if( !Count() )
/*N*/ 		return;
/*N*/ 
/*N*/ 	// besorge erstmal das Nodes-Array ueber den StartIndex der
/*N*/ 	// ersten Fussnote
/*N*/ 	SwDoc* pDoc = (SwDoc*) (*this)[ 0 ]->GetTxtNode().GetDoc();
/*N*/ 	SwTxtFtn* pTxtFtn;
/*N*/ 	const SwEndNoteInfo& rEndInfo = pDoc->GetEndNoteInfo();
/*N*/ 	const SwFtnInfo& rFtnInfo = pDoc->GetFtnInfo();
/*N*/ 
/*N*/ 	SwUpdFtnEndNtAtEnd aNumArr;
/*N*/ 
/*N*/ 	//Fuer normale Fussnoten werden Chapter- und Dokumentweise Nummerierung
/*N*/ 	//getrennt behandelt. Fuer Endnoten gibt es nur die Dokumentweise
/*N*/ 	//Nummerierung.
/*N*/ 	if( FTNNUM_CHAPTER == rFtnInfo.eNum )
/*N*/ 	{
/*?*/ 		const SwOutlineNodes& rOutlNds = pDoc->GetNodes().GetOutLineNds();
/*?*/ 		USHORT nNo = 1,			// Nummer fuer die Fussnoten
/*?*/ 			   nFtnIdx = 0;		// Index in das FtnIdx-Array
/*?*/ 		for( USHORT n = 0; n < rOutlNds.Count(); ++n )
/*?*/ 		{
/*?*/ 			if( !rOutlNds[ n ]->GetTxtNode()->GetTxtColl()->GetOutlineLevel() )
/*?*/ 			{
/*?*/ 				ULONG nCapStt = rOutlNds[ n ]->GetIndex();	// Start eines neuen Kapitels
/*?*/ 				for( ; nFtnIdx < Count(); ++nFtnIdx )
/*?*/ 				{
/*?*/ 					pTxtFtn = (*this)[ nFtnIdx ];
/*?*/ 					if( pTxtFtn->GetTxtNode().GetIndex() >= nCapStt )
/*?*/ 						break;
/*?*/ 
/*?*/ 					// Endnoten nur Dokumentweise
/*?*/ 					const SwFmtFtn &rFtn = pTxtFtn->GetFtn();
/*?*/ 					if( !rFtn.IsEndNote() && !rFtn.GetNumStr().Len() &&
/*?*/ 						!SwUpdFtnEndNtAtEnd::FindSectNdWithEndAttr( *pTxtFtn ))
/*?*/ 						pTxtFtn->SetNumber( rFtnInfo.nFtnOffset + nNo++,
/*?*/ 											&rFtn.GetNumStr() );
/*?*/ 				}
/*?*/ 				if( nFtnIdx >= Count() )
/*?*/ 					break;			// ok alles geupdatet
/*?*/ 				nNo = 1;
/*?*/ 			}
/*?*/ 		}
/*?*/ 
/*?*/ 		for( nNo = 1; nFtnIdx < Count(); ++nFtnIdx )
/*?*/ 		{
/*?*/ 			//Endnoten nur Dokumentweise
/*?*/ 			pTxtFtn = (*this)[ nFtnIdx ];
/*?*/ 			const SwFmtFtn &rFtn = pTxtFtn->GetFtn();
/*?*/ 			if( !rFtn.IsEndNote() && !rFtn.GetNumStr().Len() &&
/*?*/ 				!SwUpdFtnEndNtAtEnd::FindSectNdWithEndAttr( *pTxtFtn ))
/*?*/ 				pTxtFtn->SetNumber( rFtnInfo.nFtnOffset + nNo++,
/*?*/ 									&rFtn.GetNumStr() );
/*?*/ 		}
/*?*/ 
/*N*/ 	}

    // BOOL, damit hier auch bei Chapter-Einstellung die Endnoten
    // durchlaufen.
/*N*/ 	const FASTBOOL bEndNoteOnly = FTNNUM_DOC != rFtnInfo.eNum;
/*N*/ 	USHORT nFtnNo = 0, nEndNo = 0;
/*N*/ 	for( USHORT nPos = 0; nPos < Count(); ++nPos )
/*N*/ 	{
/*N*/ 		pTxtFtn = (*this)[ nPos ];
/*N*/ 		const SwFmtFtn &rFtn = pTxtFtn->GetFtn();
/*N*/ 		if( !rFtn.GetNumStr().Len() )
/*N*/ 		{
/*?*/ 			USHORT nSectNo = aNumArr.ChkNumber( *pTxtFtn );
/*?*/ 			if( !nSectNo && ( rFtn.IsEndNote() || !bEndNoteOnly ))
/*?*/ 				nSectNo = rFtn.IsEndNote()
/*?*/ 								? rEndInfo.nFtnOffset + (++nEndNo)
/*?*/ 								: rFtnInfo.nFtnOffset + (++nFtnNo);
/*?*/ 
/*?*/ 			if( nSectNo )
/*?*/ 			{
/*?*/ 				if( rFtn.IsEndNote() )
/*?*/ 					pTxtFtn->SetNumber( nSectNo, &rFtn.GetNumStr() );
/*?*/ 				else
/*?*/ 					pTxtFtn->SetNumber( nSectNo, &rFtn.GetNumStr() );
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pDoc->GetRootFrm() && FTNNUM_PAGE == rFtnInfo.eNum )
/*?*/ 		{DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 pDoc->GetRootFrm()->UpdateFtnNums();
/*N*/ }


/*N*/ SwTxtFtn* SwFtnIdxs::SeekEntry( const SwNodeIndex& rPos, USHORT* pFndPos ) const
/*N*/ {
/*N*/ 	ULONG nIdx = rPos.GetIndex();
/*N*/ 
/*N*/ 	register USHORT nO = Count(), nM, nU = 0;
/*N*/ 	if( nO > 0 )
/*N*/ 	{
/*N*/ 		nO--;
/*N*/ 		while( nU <= nO )
/*N*/ 		{
/*N*/ 			nM = nU + ( nO - nU ) / 2;
/*N*/ 			ULONG nNdIdx = _SwTxtFtn_GetIndex( (*this)[ nM ] );
/*N*/ 			if( nNdIdx == nIdx )
/*N*/ 			{
/*N*/ 				if( pFndPos )
/*N*/ 					*pFndPos = nM;
/*N*/ 				return (*this)[ nM ];
/*N*/ 			}
/*N*/ 			else if( nNdIdx < nIdx )
/*N*/ 				nU = nM + 1;
/*N*/ 			else if( nM == 0 )
/*N*/ 			{
/*N*/ 				if( pFndPos )
/*N*/ 					*pFndPos = nU;
/*N*/ 				return 0;
/*N*/ 			}
/*N*/ 			else
/*N*/ 				nO = nM - 1;
/*N*/ 		}
/*N*/ 	}
/*?*/ 	if( pFndPos )
/*?*/ 		*pFndPos = nU;
/*?*/ 	return 0;
/*N*/ }

/*  */

/*N*/ const SwSectionNode* SwUpdFtnEndNtAtEnd::FindSectNdWithEndAttr(
/*N*/ 				const SwTxtFtn& rTxtFtn )
/*N*/ {
/*N*/ 	USHORT nWh = rTxtFtn.GetFtn().IsEndNote() ? RES_END_AT_TXTEND
/*N*/ 											  :	RES_FTN_AT_TXTEND;
/*N*/ 	USHORT nVal;
/*N*/ 	const SwSectionNode* pNd = rTxtFtn.GetTxtNode().FindSectionNode();
/*N*/ 	while( pNd && FTNEND_ATTXTEND_OWNNUMSEQ != ( nVal =
/*N*/ 			((const SwFmtFtnAtTxtEnd&)pNd->GetSection().GetFmt()->
/*N*/ 			GetAttr( nWh, TRUE )).GetValue() ) &&
/*N*/ 			FTNEND_ATTXTEND_OWNNUMANDFMT != nVal )
/*?*/ 		pNd = pNd->FindStartNode()->FindSectionNode();
/*N*/ 
/*N*/ 	return pNd;
/*N*/ }

/*N*/ USHORT SwUpdFtnEndNtAtEnd::GetNumber( const SwTxtFtn& rTxtFtn,
/*N*/ 									const SwSectionNode& rNd )
/*N*/ {
            USHORT nRet = 0, nWh;
            SvPtrarr* pArr;
            SvUShorts* pNum;
            if( rTxtFtn.GetFtn().IsEndNote() )
            {
                pArr = &aEndSects;
                pNum = &aEndNums;
                nWh = RES_END_AT_TXTEND;
            }
            else
            {
                pArr = &aFtnSects;
                pNum = &aFtnNums;
                nWh = RES_FTN_AT_TXTEND;
            }
            void* pNd = (void*)&rNd;

            for( USHORT n = pArr->Count(); n; )
                if( pArr->GetObject( --n ) == pNd )
                {
                    nRet = ++pNum->GetObject( n );
                    break;
                }

            if( !nRet )
            {
                pArr->Insert( pNd, pArr->Count() );
                nRet = ((SwFmtFtnEndAtTxtEnd&)rNd.GetSection().GetFmt()->
                                        GetAttr( nWh )).GetOffset();
                ++nRet;
                pNum->Insert( nRet, pNum->Count() );
            }
            return nRet;
/*N*/ }

/*N*/ USHORT SwUpdFtnEndNtAtEnd::ChkNumber( const SwTxtFtn& rTxtFtn )
/*N*/ {
/*N*/ 	const SwSectionNode* pSectNd = FindSectNdWithEndAttr( rTxtFtn );
/*N*/ 	return pSectNd ? GetNumber( rTxtFtn, *pSectNd ) : 0;
/*N*/ }




}
