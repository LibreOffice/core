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



#include <errhdl.hxx>
#include <horiornt.hxx>

#include <extinput.hxx>
#include <doc.hxx>
namespace binfilter {








// die Doc Schnittstellen:



/*N*/ SwExtTextInput* SwDoc::GetExtTextInput( const SwNode& rNd,
/*N*/ 										xub_StrLen nCntntPos ) const
/*N*/ {
/*N*/ 	SwExtTextInput* pRet = 0;
/*N*/ 	if( pExtInputRing )
/*N*/ 	{
/*?*/ 		ULONG nNdIdx = rNd.GetIndex();
/*?*/ 		SwExtTextInput* pTmp = (SwExtTextInput*)pExtInputRing;
/*?*/ 		do {
/*?*/ 			ULONG nPt = pTmp->GetPoint()->nNode.GetIndex(),
/*?*/ 				  nMk = pTmp->GetMark()->nNode.GetIndex();
/*?*/ 			xub_StrLen nPtCnt = pTmp->GetPoint()->nContent.GetIndex(),
/*?*/ 				  	   nMkCnt = pTmp->GetMark()->nContent.GetIndex();
/*?*/ 
/*?*/ 			if( nPt < nMk || ( nPt == nMk && nPtCnt < nMkCnt ))
/*?*/ 			{
/*?*/ 				ULONG nTmp = nMk; nMk = nPt; nPt = nTmp;
/*?*/ 				nTmp = nMkCnt; nMkCnt = nPtCnt; nPtCnt = (xub_StrLen)nTmp;
/*?*/ 			}
/*?*/ 
/*?*/ 			if( nMk <= nNdIdx && nNdIdx <= nPt &&
/*?*/ 				( STRING_NOTFOUND == nCntntPos ||
/*?*/ 					( nMkCnt <= nCntntPos && nCntntPos <= nPtCnt )))
/*?*/ 			{
/*?*/ 				pRet = pTmp;
/*?*/ 				break;
/*?*/ 			}
/*?*/ 		} while( pExtInputRing != (pTmp = (SwExtTextInput*)pExtInputRing ) );
/*N*/ 	}
/*N*/ 	return pRet;
/*N*/ }



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
