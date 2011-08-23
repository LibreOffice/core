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

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#include "doc.hxx"

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif


#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#include "flypos.hxx"
#include "frmfmt.hxx"
#include "dcontact.hxx"
#include "flyfrm.hxx"
#include "dflyobj.hxx"
#include "ndindex.hxx"
namespace binfilter {



/*N*/ SV_IMPL_OP_PTRARR_SORT( SwPosFlyFrms, SwPosFlyFrmPtr )

/*N*/ SwPosFlyFrm::SwPosFlyFrm( const SwNodeIndex& rIdx, const SwFrmFmt* pFmt,
/*N*/ 							USHORT nArrPos )
/*N*/ 	: pNdIdx( (SwNodeIndex*) &rIdx ), pFrmFmt( pFmt )
/*N*/ {
/*N*/ 	BOOL bFnd = FALSE;
/*N*/ 	const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
/*N*/ 	if( FLY_PAGE == rAnchor.GetAnchorId() )
/*?*/ 		pNdIdx = new SwNodeIndex( rIdx );
/*N*/ 	else if( pFmt->GetDoc()->GetRootFrm() )
/*N*/ 	{
/*N*/ 		SwClientIter aIter( (SwFmt&)*pFmt );
/*N*/ 		if( RES_FLYFRMFMT == pFmt->Which() )
/*N*/ 		{
/*N*/ 			// Schauen, ob es ein SdrObject dafuer gibt
/*N*/ 			if( aIter.First( TYPE( SwFlyFrm) ) )
/*N*/ 				nOrdNum = ((SwFlyFrm*)aIter())->GetVirtDrawObj()->GetOrdNum(),
/*N*/ 				bFnd = TRUE;
/*N*/ 		}
/*N*/ 		else if( RES_DRAWFRMFMT == pFmt->Which() )
/*N*/ 		{
/*N*/ 			// Schauen, ob es ein SdrObject dafuer gibt
/*N*/ 			if( aIter.First( TYPE(SwDrawContact) ) )
/*N*/ 				nOrdNum = ((SwDrawContact*)aIter())->GetMaster()->GetOrdNum(),
/*N*/ 				bFnd = TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( !bFnd )
/*N*/ 	{
/*N*/ 		nOrdNum = pFmt->GetDoc()->GetSpzFrmFmts()->Count();
/*N*/ 		nOrdNum += nArrPos;
/*N*/ 	}
/*N*/ }

/*N*/ SwPosFlyFrm::~SwPosFlyFrm()
/*N*/ {
/*N*/ 	const SwFmtAnchor& rAnchor = pFrmFmt->GetAnchor();
/*N*/ 	if( FLY_PAGE == rAnchor.GetAnchorId() )
/*?*/ 		delete pNdIdx;
/*N*/ }

/*N*/ BOOL SwPosFlyFrm::operator==( const SwPosFlyFrm& rPosFly )
/*N*/ {
/*N*/ 	return FALSE;	// FlyFrames koennen auf der gleichen Position stehen
/*N*/ }

/*N*/ BOOL SwPosFlyFrm::operator<( const SwPosFlyFrm& rPosFly )
/*N*/ {
/*N*/ 	if( pNdIdx->GetIndex() == rPosFly.pNdIdx->GetIndex() )
/*N*/ 	{
/*N*/ 		// dann entscheidet die Ordnungsnummer!
/*N*/ 		return nOrdNum < rPosFly.nOrdNum;
/*N*/ 	}
/*N*/ 	return pNdIdx->GetIndex() < rPosFly.pNdIdx->GetIndex();
/*N*/ }



}
