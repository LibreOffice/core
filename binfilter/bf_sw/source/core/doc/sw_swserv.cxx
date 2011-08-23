/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <bf_svx/linkmgr.hxx>
#include <com/sun/star/uno/Sequence.h>

#include <horiornt.hxx>
#include <errhdl.hxx>

#include <doc.hxx>
#include <swserv.hxx>
#include <swbaslnk.hxx>
#include <mvsave.hxx>
#include <bookmrk.hxx>
#include <pam.hxx>

#include <swerror.h>
namespace binfilter {

/*N*/ SV_IMPL_REF( SwServerObject )

/*N*/ SwServerObject::~SwServerObject()
/*N*/ {
/*N*/ }










/*N*/ BOOL SwServerObject::IsLinkInServer( const SwBaseLink* pChkLnk ) const
/*N*/ {
/*N*/ 	ULONG nSttNd = 0, nEndNd = 0;
/*N*/ 	xub_StrLen nStt, nEnd;
/*N*/ 	const SwNode* pNd = 0;
/*N*/ 	const SwNodes* pNds = 0;
/*N*/ 
/*N*/ 	switch( eType )
/*N*/ 	{
/*?*/ 	case BOOKMARK_SERVER:
/*?*/ 		if( CNTNT_TYPE.pBkmk->GetOtherPos() )
/*?*/ 		{
/*?*/ 			SwBookmark& rBkmk = *CNTNT_TYPE.pBkmk;
/*?*/ 			const SwPosition* pStt = &rBkmk.GetPos(),
/*?*/ 							* pEnd = rBkmk.GetOtherPos();
/*?*/ 			if( *pStt > *pEnd )
/*?*/ 			{
/*?*/ 				const SwPosition* pTmp = pStt;
/*?*/ 				pStt = pEnd;
/*?*/ 				pEnd = pTmp;
/*?*/ 			}
/*?*/ 
/*?*/ 			nSttNd = pStt->nNode.GetIndex();
/*?*/ 			nStt = pStt->nContent.GetIndex();
/*?*/ 			nEndNd = pEnd->nNode.GetIndex();
/*?*/ 			nEnd = pEnd->nContent.GetIndex();
/*?*/ 			pNds = &pStt->nNode.GetNodes();
/*?*/ 		}
/*?*/ 		break;
/*?*/ 
/*?*/ 	case TABLE_SERVER: 		pNd = CNTNT_TYPE.pTblNd;	break;
/*?*/ 	case SECTION_SERVER:	pNd = CNTNT_TYPE.pSectNd;	break;
/*?*/ 
/*N*/ 	case SECTION_SERVER+1:
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pNd )
/*N*/ 	{
/*N*/ 		nSttNd = pNd->GetIndex();
/*N*/ 		nEndNd = pNd->EndOfSectionIndex();
/*N*/ 		nStt = 0, nEnd = USHRT_MAX;
/*N*/ 		pNds = &pNd->GetNodes();
/*N*/ 	}
/*N*/ 
/*N*/ 	if( nSttNd && nEndNd )
/*N*/ 	{
/*N*/ 		// LinkManager besorgen:
/*N*/ 		const ::binfilter::SvBaseLinks& rLnks = pNds->GetDoc()->GetLinkManager().GetLinks();
/*N*/ 
/*N*/ // um Rekursionen zu Verhindern: ServerType umsetzen!
/*N*/ SwServerObject::ServerModes eSave = eType;
/*N*/ if( !pChkLnk )
/*N*/ // sowas sollte man nicht tun, wer weiss schon, wie gross ein enum ist
/*N*/ // ICC nimmt keinen int
/*N*/ // #41723#
/*N*/ //	*((int*)&eType) = SECTION_SERVER+1;
/*N*/ 	((SwServerObject*)this)->eType = NONE_SERVER;
/*N*/ 		for( USHORT n = rLnks.Count(); n; )
/*N*/ 		{
/*N*/ 			const ::binfilter::SvBaseLink* pLnk = &(*rLnks[ --n ]);
/*N*/ 			if( pLnk && OBJECT_CLIENT_GRF != pLnk->GetObjType() &&
/*N*/ 				pLnk->ISA( SwBaseLink ) &&
/*N*/ 				!((SwBaseLink*)pLnk)->IsNoDataFlag() &&
/*N*/ 				((SwBaseLink*)pLnk)->IsInRange( nSttNd, nEndNd, nStt, nEnd ))
/*N*/ 			{
/*N*/ 				if( pChkLnk )
/*N*/ 				{
/*N*/ 					if( pLnk == pChkLnk ||
/*N*/ 						((SwBaseLink*)pLnk)->IsRecursion( pChkLnk ) )
/*N*/ 						return TRUE;
/*N*/ 				}
/*N*/ 				else if( ((SwBaseLink*)pLnk)->IsRecursion( (SwBaseLink*)pLnk ) )
/*N*/ 					((SwBaseLink*)pLnk)->SetNoDataFlag();
/*N*/ 			}
/*N*/ 		}
/*N*/ if( !pChkLnk )
/*N*/ 	//	*((int*)&eType) = eSave;
/*?*/ 	((SwServerObject*)this)->eType = eSave;
/*N*/ 	}
/*N*/ 
/*N*/ 	return FALSE;
/*N*/ }

/*  */


/*N*/ SwDataChanged::SwDataChanged( const SwPaM& rPam, USHORT nTyp )
/*N*/ 	: pPam( &rPam ), nType( nTyp ), pDoc( rPam.GetDoc() ), pPos( 0 )
/*N*/ {
/*N*/ 	nNode = rPam.GetPoint()->nNode.GetIndex();
/*N*/ 	nCntnt = rPam.GetPoint()->nContent.GetIndex();
/*N*/ }
 

/*N*/ SwDataChanged::SwDataChanged( SwDoc* pDoc, const SwPosition& rPos, USHORT nTyp )
/*N*/ 	: pPam( 0 ), nType( nTyp ), pDoc( pDoc ), pPos( &rPos )
/*N*/ {
/*N*/ 	nNode = rPos.nNode.GetIndex();
/*N*/ 	nCntnt = rPos.nContent.GetIndex();
/*N*/ }
/*N*/ 

/*N*/ SwDataChanged::~SwDataChanged()
/*N*/ {
/*N*/ 	// JP 09.04.96: nur wenn das Layout vorhanden ist ( also waehrend der
/*N*/ 	//				Eingabe)
/*N*/ 	if( pDoc->GetRootFrm() )
/*N*/ 	{
/*N*/ 		const ::binfilter::SvLinkSources& rServers = pDoc->GetLinkManager().GetServers();
/*N*/ 
/*N*/ 		for( USHORT nCnt = rServers.Count(); nCnt; )
/*N*/ 		{
/*N*/ 			::binfilter::SvLinkSourceRef refObj( rServers[ --nCnt ] );
/*N*/ 			// noch jemand am Object interessiert ?
/*?*/ 			if( refObj->HasDataLinks() && refObj->ISA( SwServerObject ))
/*?*/ 			{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	SwServerObject& rObj = *(SwServerObject*)&refObj;
/*?*/ 			}
/*?*/ 
/*?*/ 			// sollte jetzt gar keine Verbindung mehr bestehen
/*?*/ 			if( !refObj->HasDataLinks() )
/*?*/ 			{
/*?*/ 				// dann raus aus der Liste (Object bleibt aber bestehen!)
/*?*/ 				// falls es noch da ist !!
/*?*/ 				if( nCnt < rServers.Count() && &refObj == rServers[ nCnt ] )
/*?*/ 					pDoc->GetLinkManager().RemoveServer( nCnt, 1 );
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ }




}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
