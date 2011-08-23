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

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif


#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>			// UpdateFlds der KapitelNummerierung
#endif
namespace binfilter {


//-------------------------------------------------------
// Gliederung

/*N*/ struct _OutlinePara
/*N*/ {
/*N*/ 	SwNodeNum aNum;
/*N*/ 	const SwNodes& rNds;
/*N*/ 	BYTE nMin, nNewLevel;
/*N*/     // OD 21.11.2002 #100043# - array to remember, which level numbering
/*N*/     // has to be started.
/*N*/     bool aStartLevel[ MAXLEVEL ];
/*N*/ 
/*N*/ 	_OutlinePara( const SwNodes& rNodes, USHORT nSttPos, BYTE nOld, BYTE nNew );
/*N*/ 	BOOL UpdateOutline( SwTxtNode& rTxtNd );
/*N*/ };

/*N*/ _SV_IMPL_SORTAR_ALG( SwOutlineNodes, SwNodePtr )
/*N*/ BOOL SwOutlineNodes::Seek_Entry( const SwNodePtr rSrch, USHORT* pFndPos ) const
/*N*/ {
/*N*/ 	ULONG nIdx = rSrch->GetIndex();
/*N*/ 
/*N*/ 	register USHORT nO = Count(), nM, nU = 0;
/*N*/ 	if( nO > 0 )
/*N*/ 	{
/*N*/ //JP 17.03.98: aufgrund des Bug 48592 - wo unter anderem nach Undo/Redo
/*N*/ //				Nodes aus dem falschen NodesArray im OutlineArray standen,
/*N*/ //				jetzt mal einen Check eingebaut.
/*N*/ #ifdef DBG_UTIL
/*N*/ 		{
/*N*/ 			for( register USHORT n = 1; n < nO; ++n )
/*N*/ 				if( &(*this)[ n-1 ]->GetNodes() !=
/*N*/ 					&(*this)[ n ]->GetNodes() )
/*N*/ 				{
/*?*/ 					ASSERT( !this, "Node im falschen Outline-Array" );
/*N*/ 				}
/*N*/ 		}
/*N*/ #endif
/*N*/ 
/*N*/ 		nO--;
/*N*/ 		while( nU <= nO )
/*N*/ 		{
/*N*/ 			nM = nU + ( nO - nU ) / 2;
/*N*/ 			if( (*this)[ nM ] == rSrch )
/*N*/ 			{
/*N*/ 				if( pFndPos )
/*N*/ 					*pFndPos = nM;
/*N*/ 				return TRUE;
/*N*/ 			}
/*N*/ 			else if( (*this)[ nM ]->GetIndex() < nIdx )
/*N*/ 				nU = nM + 1;
/*N*/ 			else if( nM == 0 )
/*N*/ 			{
/*N*/ 				if( pFndPos )
/*N*/ 					*pFndPos = nU;
/*N*/ 				return FALSE;
/*N*/ 			}
/*N*/ 			else
/*N*/ 				nO = nM - 1;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if( pFndPos )
/*N*/ 		*pFndPos = nU;
/*N*/ 	return FALSE;
/*N*/ }


/*N*/ _OutlinePara::_OutlinePara( const SwNodes& rNodes, USHORT nSttPos,
/*N*/ 							BYTE nOld, BYTE nNew )
/*N*/ 	: rNds( rNodes ),
/*N*/     aNum( NO_NUM > nNew ? nNew : 0 ),
/*N*/ 	nMin( Min( nOld, nNew )),
/*N*/     nNewLevel( nNew )
/*N*/ {
/*N*/     // OD 25.11.2002 #100043# - init <aStartLevel[]> with defaults, only valid
/*N*/     // if update of outline numbering started at first outline numbering node.
/*N*/     for ( int i = 0; i < MAXLEVEL; ++i)
/*N*/         aStartLevel[i] = true;
/*N*/ 
/*N*/     // hole vom Vorgaenger die aktuelle Nummerierung
/*N*/ 	SwNode* pNd;
/*N*/ 	ULONG nEndOfExtras = rNds.GetEndOfExtras().GetIndex();
/*N*/     if ( nSttPos &&
/*N*/          (pNd = rNds.GetOutLineNds()[ --nSttPos ])->GetIndex() > nEndOfExtras &&
/*N*/          static_cast<SwTxtNode*>(pNd)->GetOutlineNum()
/*N*/        )
/*N*/ 	{
/*N*/ 		const SwNodeNum* pNum = ((SwTxtNode*)pNd)->GetOutlineNum();
/*N*/ #ifdef TASK_59308
/*N*/ 		if( pNum->GetLevel() & NO_NUMLEVEL )
/*N*/ 		{
/*N*/ 			// dann suche den mit richtigem Level:
/*N*/ 			BYTE nSrchLvl = aNum.GetLevel();
/*N*/ 			pNum = 0;
/*N*/ 			while( nSttPos-- )
/*N*/ 			{
/*N*/ 				if( ( pNd = rNds.GetOutLineNds()[ nSttPos ])->
/*N*/ 					GetIndex() < nEndOfExtras )
/*N*/ 					break;
/*N*/ 
/*N*/ 				if( 0 != ( pNum = ((SwTxtNode*)pNd)->GetOutlineNum() ))
/*N*/ 				{
/*N*/ 					// uebergeordnete Ebene
/*N*/ 					if( nSrchLvl > (pNum->GetLevel() &~ NO_NUMLEVEL ))
/*N*/ 					{
/*N*/ 						pNum = 0;
/*N*/ 						break;
/*N*/ 					}
/*N*/ 					// gleiche Ebene und kein NO_NUMLEVEL
/*N*/ 					if( nSrchLvl == (pNum->GetLevel() &~ NO_NUMLEVEL)
/*N*/ 						&& !( pNum->GetLevel() & NO_NUMLEVEL ))
/*N*/ 						break;
/*N*/ 
/*N*/ 					pNum = 0;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ #endif
/*N*/ 		if( pNum )
/*N*/ 		{
/*N*/ 			aNum = *pNum;
/*N*/ 			aNum.SetStart( FALSE );
/*N*/ 			aNum.SetSetValue( USHRT_MAX );
/*N*/ 		}
/*N*/ 
/*N*/ 		if( aNum.GetLevel()+1 < MAXLEVEL )
/*N*/         {
/*N*/ 			memset( aNum.GetLevelVal() + (aNum.GetLevel()+1), 0,
/*N*/ 					(MAXLEVEL - (aNum.GetLevel()+1)) * sizeof(aNum.GetLevelVal()[0]) );
/*N*/         }
/*N*/         // OD 22.11.2002 #100043# - init array <aStartLevel[]>, not starting at
/*N*/         // first outline numbering node.
/*N*/         aStartLevel[ pNum->GetLevel() ] = false;
/*N*/         USHORT nHighestLevelFound = pNum->GetLevel();
/*N*/         while ( pNum->GetLevel() > 0 && nSttPos-- )
/*N*/         {
/*N*/             pNd = rNds.GetOutLineNds()[ nSttPos ];
/*N*/             if ( pNd->GetIndex() < nEndOfExtras )
/*N*/                 break;
/*N*/             pNum = static_cast<SwTxtNode*>(pNd)->GetOutlineNum();
/*N*/             if ( pNum && pNum->GetLevel() < nHighestLevelFound )
/*N*/             {
/*N*/                 aStartLevel[ pNum->GetLevel() ] = false;
/*N*/                 nHighestLevelFound = pNum->GetLevel();
/*N*/             }
/*N*/         }
/*N*/ 	}
/*N*/ }

/*N*/ BOOL _OutlinePara::UpdateOutline( SwTxtNode& rTxtNd )
/*N*/ {
/*N*/     // alle die ausserhalb des Fliesstextes liegen, NO_NUM zuweisen.
/*N*/ 	if( rTxtNd.GetIndex() < rNds.GetEndOfExtras().GetIndex() )
/*N*/ 	{
/*N*/ 		BYTE nTmpLevel = aNum.GetLevel();
/*N*/ 		aNum.SetLevel( NO_NUM );
/*N*/ 		rTxtNd.UpdateOutlineNum( aNum );
/*N*/ 		aNum.SetLevel( nTmpLevel );
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	BYTE nLevel = rTxtNd.GetTxtColl()->GetOutlineLevel();
/*N*/ 	BOOL bRet = !(nMin > nLevel);
/*N*/ 	if( bRet )
/*N*/ 	{
/*N*/ 		// existierte am Node schon eine Nummerierung ??
/*N*/ 		// dann erfrage den "User definierten Wert"
/*N*/ 		USHORT nSetValue;
/*N*/ 		const SwNumRule* pOutlRule = rTxtNd.GetDoc()->GetOutlineNumRule();
/*N*/ 		const SwNodeNum* pOutlNum = rTxtNd.GetOutlineNum();
/*N*/ 
/*N*/ #ifdef TASK_59308
/*N*/ 		if( pOutlNum && ( pOutlNum->GetLevel() & NO_NUMLEVEL ) &&
/*N*/ 			GetRealLevel( pOutlNum->GetLevel() ) == nLevel )
/*N*/ 		{
/*N*/ 			// diesen nicht mit numerieren
/*N*/ 			BYTE nTmpLevel = aNum.GetLevel();
/*N*/ 			aNum.SetLevel( pOutlNum->GetLevel() );
/*N*/ 			rTxtNd.UpdateOutlineNum( aNum );
/*N*/ 			aNum.SetLevel( nTmpLevel );
/*N*/ 			return TRUE;
/*N*/ 		}
/*N*/ #endif
/*N*/ 
/*N*/         // OD 21.11.2002 #100043# - determine, if level numbering has to be started.
/*N*/         // OD 09.12.2002 #106070# - correct outline numbering, even for the
/*N*/         // first heading. Thus, state of <aStartLevel[]> always has to be
/*N*/         // consulted, not only on level change.
/*N*/         if( aStartLevel[ nLevel ] )
/*N*/ 		{
/*N*/ 			nSetValue= pOutlRule->Get( nLevel ).GetStart();
/*N*/             // OD 21.11.2002 #100043# - reset <aStartLevel[nLevel]>
/*N*/             aStartLevel[ nLevel ] = false;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			nSetValue = aNum.GetLevelVal()[ nLevel ] + 1;
/*N*/ 
/*N*/ 		 // alle unter dem neuen Level liegenden auf 0 setzen
/*N*/ 		if( aNum.GetLevel() > nLevel && nLevel+1 < MAXLEVEL
/*N*/ 			/* ??? && NO_NUM > nNewLevel */ )
/*N*/         {
/*N*/ 			memset( aNum.GetLevelVal() + (nLevel+1), 0,
/*N*/ 					(MAXLEVEL - ( nLevel+1 )) * sizeof(aNum.GetLevelVal()[0]) );
/*N*/             // OD 22.11.2002 #100043# - all next level numberings have to be started.
/*N*/             for ( int i = nLevel+1; i < MAXLEVEL; ++i)
/*N*/                 aStartLevel[i] = true;
/*N*/         }
/*N*/ 
/*N*/ 		if( pOutlNum && USHRT_MAX != pOutlNum->GetSetValue() )
/*N*/ 			aNum.SetSetValue( nSetValue = pOutlNum->GetSetValue() );
/*N*/ 
/*N*/ 		aNum.GetLevelVal()[ nLevel ] = nSetValue;
/*N*/ 		aNum.SetLevel( nLevel );
/*N*/ 		rTxtNd.UpdateOutlineNum( aNum );
/*N*/ 		aNum.SetSetValue( USHRT_MAX );
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }




/*N*/ BOOL lcl_UpdateOutline( const SwNodePtr& rpNd, void* pPara )
/*N*/ {
/*N*/ 	_OutlinePara* pOutlPara = (_OutlinePara*)pPara;
/*N*/ 	SwTxtNode* pTxtNd = rpNd->GetTxtNode();
/*N*/ 	ASSERT( pTxtNd, "kein TextNode als OutlineNode !" );
/*N*/ 
/*N*/ 	return pOutlPara->UpdateOutline( *pTxtNd );
/*N*/ }




/*N*/ void SwNodes::UpdateOutlineNode( const SwNode& rNd, BYTE nOldLevel,
/*N*/ 								BYTE nNewLevel )
/*N*/ {
/*N*/ 	const SwNodePtr pSrch = (SwNodePtr)&rNd;
/*N*/ 	USHORT nSttPos;
/*N*/ 	BOOL bSeekIdx = pOutlineNds->Seek_Entry( pSrch, &nSttPos );
/*N*/ 
/*N*/ 	if( NO_NUMBERING == nOldLevel )			// neuen Level einfuegen
/*N*/ 	{
/*N*/ 		// nicht vorhanden, also einfuegen
/*N*/ 		ASSERT( !bSeekIdx, "Der Node ist schon als OutlineNode vorhanden" );
/*N*/ 
/*N*/ 		//JP 12.03.99: 63293 - Nodes vom RedlineBereich NIE aufnehmen
/*N*/ 		ULONG nNd = rNd.GetIndex();
/*N*/ 		if( nNd < GetEndOfRedlines().GetIndex() &&
/*N*/ 			nNd > GetEndOfRedlines().FindStartNode()->GetIndex() )
/*?*/ 			return ;
/*N*/ 
/*N*/ 		// jetzt noch alle nachfolgende Outline-Nodes updaten
/*N*/ 		pOutlineNds->Insert( pSrch );
/*N*/ 		if( NO_NUM <= nNewLevel )
/*?*/ 			return;		// keine Nummerierung dann kein Update
/*N*/ 	}
/*N*/ 	else if( NO_NUMBERING == nNewLevel )	// Level entfernen
/*N*/ 	{
/*?*/ 		if( !bSeekIdx )
/*?*/ 			return;
/*?*/ 
/*?*/ 		// jetzt noch alle nachfolgende Outline-Nodes updaten
/*?*/ 		pOutlineNds->Remove( nSttPos );
/*?*/ 		if( NO_NUM <= nOldLevel )
/*?*/ 			return;		// keine Nummerierung dann kein Update
/*N*/ 	}
/*N*/ 	else if( !bSeekIdx )		// Update und Index nicht gefunden ??
/*?*/ 		return ;
/*N*/ 
/*N*/ 	_OutlinePara aPara( *this, nSttPos, nOldLevel, nNewLevel );
/*N*/ 	pOutlineNds->ForEach( nSttPos, pOutlineNds->Count(),
/*N*/ 						lcl_UpdateOutline, &aPara );
/*N*/ 
/*N*/ //FEATURE::CONDCOLL
/*N*/ 	{
/*N*/ 		SwCntntNode* pCNd;
/*N*/ 		ULONG nSttNd = rNd.GetIndex();
/*N*/ 		if( NO_NUMBERING != nNewLevel )
/*N*/ 			++nSttPos;
/*N*/ 
/*N*/ 		ULONG nChkCount = ( nSttPos < pOutlineNds->Count()
/*N*/ 								? (*pOutlineNds)[ nSttPos ]->GetIndex()
/*N*/ 								: GetEndOfContent().GetIndex()  )
/*N*/ 							- nSttNd;
/*N*/ 		for( ; nChkCount--; ++nSttNd )
/*N*/ 			if( 0 != (pCNd = (*this)[ nSttNd ]->GetCntntNode() ) &&
/*N*/ 				RES_CONDTXTFMTCOLL == pCNd->GetFmtColl()->Which() )
/*N*/ 				pCNd->ChkCondColl();
/*N*/ 	}
/*N*/ //FEATURE::CONDCOLL
/*N*/ 
/*N*/ 	// die Gliederungs-Felder Updaten
/*N*/ 	GetDoc()->GetSysFldType( RES_CHAPTERFLD )->UpdateFlds();
/*N*/ }



/*N*/ void SwNodes::UpdtOutlineIdx( const SwNode& rNd )
/*N*/ {
/*N*/ 	if( !pOutlineNds->Count() )		// keine OutlineNodes vorhanden ?
/*N*/ 		return;
/*N*/ 
/*N*/ 	const SwNodePtr pSrch = (SwNodePtr)&rNd;
/*N*/ 	USHORT nPos;
/*N*/ 	pOutlineNds->Seek_Entry( pSrch, &nPos );
/*N*/ 	if( nPos == pOutlineNds->Count() )		// keine zum Updaten vorhanden ?
/*N*/ 		return;
/*N*/ 
/*N*/ 	if( nPos )
/*N*/ 		--nPos;
/*N*/ 
/*N*/ 	if( !GetDoc()->IsInDtor() && IsDocNodes() )
/*N*/ 		UpdateOutlineNode( *(*pOutlineNds)[ nPos ], 0, 0 );
/*N*/ }



/*N*/ void SwNodes::UpdateOutlineNodes()
/*N*/ {
/*N*/ 	if( pOutlineNds->Count() )		// OutlineNodes vorhanden ?
/*N*/ 		UpdateOutlineNode( *(*pOutlineNds)[ 0 ], 0, 0 );
/*N*/ }

}
