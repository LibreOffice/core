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

#include "errhdl.hxx"			// fuers ASSERT
#include "error.h"				// fuers ASSERT
#include "ndindex.hxx"
namespace binfilter {

#ifdef DBG_UTIL
int SwNodeIndex::nSerial = 0;
#endif


/*N*/ SwNodeRange::SwNodeRange( const SwNodeIndex &rS, const SwNodeIndex &rE )
/*N*/ 	: aStart( rS ), aEnd( rE )
/*N*/ {}

/*N*/ SwNodeRange::SwNodeRange( const SwNodeRange &rRange )
/*N*/ 	: aStart( rRange.aStart ), aEnd( rRange.aEnd )
/*N*/ {}

/*N*/ SwNodeRange::SwNodeRange( const SwNodeIndex& rS, long nSttDiff,
/*N*/ 						  const SwNodeIndex& rE, long nEndDiff )
/*N*/ 	: aStart( rS, nSttDiff ), aEnd( rE, nEndDiff )
/*N*/ {}

/*N*/ SwNodeRange::SwNodeRange( const SwNode& rS, long nSttDiff,
/*N*/ 						  const SwNode& rE, long nEndDiff )
/*N*/ 	: aStart( rS, nSttDiff ), aEnd( rE, nEndDiff )
/*N*/ {}


/*N*/ SwNodeIndex::SwNodeIndex( SwNodes& rNds, ULONG nIdx )
/*N*/ 	: pNd( rNds[ nIdx ] ), pNext( 0 ), pPrev( 0 )
/*N*/ {
/*N*/ 	rNds.RegisterIndex( *this );
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 	MySerial = ++nSerial;		// nur in der nicht PRODUCT-Version
/*N*/ #endif
/*N*/ }


/*N*/ SwNodeIndex::SwNodeIndex( const SwNodeIndex& rIdx, long nDiff )
/*N*/ 	: pNext( 0 ), pPrev( 0 )
/*N*/ {
/*N*/ 	if( nDiff )
/*N*/ 		pNd = rIdx.GetNodes()[ rIdx.GetIndex() + nDiff ];
/*N*/ 	else
/*N*/ 		pNd = rIdx.pNd;
/*N*/ 
/*N*/ 	pNd->GetNodes().RegisterIndex( *this );
/*N*/ #ifdef DBG_UTIL
/*N*/ 	MySerial = ++nSerial;		// nur in der nicht PRODUCT-Version
/*N*/ #endif
/*N*/ }


/*N*/ SwNodeIndex::SwNodeIndex( const SwNode& rNd, long nDiff )
/*N*/ 	: pNext( 0 ), pPrev( 0 )
/*N*/ {
/*N*/ 	if( nDiff )
/*N*/ 		pNd = rNd.GetNodes()[ rNd.GetIndex() + nDiff ];
/*N*/ 	else
/*N*/ 		pNd = (SwNode*)&rNd;
/*N*/ 
/*N*/ 	pNd->GetNodes().RegisterIndex( *this );
/*N*/ #ifdef DBG_UTIL
/*N*/ 	MySerial = ++nSerial;		// nur in der nicht PRODUCT-Version
/*N*/ #endif
/*N*/ }


/*N*/ void SwNodeIndex::Remove()
/*N*/ {
/*N*/ 	pNd->GetNodes().DeRegisterIndex( *this );
/*N*/ }

/*N*/ SwNodeIndex& SwNodeIndex::operator=( const SwNodeIndex& rIdx )
/*N*/ {
/*N*/ 	if( &pNd->GetNodes() != &rIdx.pNd->GetNodes() )
/*N*/ 	{
/*N*/ 		pNd->GetNodes().DeRegisterIndex( *this );
/*N*/ 		pNd = rIdx.pNd;
/*N*/ 		pNd->GetNodes().RegisterIndex( *this );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pNd = rIdx.pNd;
/*N*/ 	return *this;
/*N*/ }

/*N*/ SwNodeIndex& SwNodeIndex::operator=( const SwNode& rNd )
/*N*/ {
/*N*/ 	if( &pNd->GetNodes() != &rNd.GetNodes() )
/*N*/ 	{
/*?*/ 		pNd->GetNodes().DeRegisterIndex( *this );
/*?*/ 		pNd = (SwNode*)&rNd;
/*?*/ 		pNd->GetNodes().RegisterIndex( *this );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pNd = (SwNode*)&rNd;
/*N*/ 	return *this;
/*N*/ }


/*N*/ SwNodeIndex& SwNodeIndex::Assign( const SwNode& rNd, long nOffset )
/*N*/ {
/*N*/ 	if( &pNd->GetNodes() != &rNd.GetNodes() )
/*N*/ 	{
/*?*/ 		pNd->GetNodes().DeRegisterIndex( *this );
/*?*/ 		pNd = (SwNode*)&rNd;
/*?*/ 		pNd->GetNodes().RegisterIndex( *this );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pNd = (SwNode*)&rNd;
/*N*/ 
/*N*/ 	if( nOffset )
/*N*/ 		pNd = pNd->GetNodes()[ pNd->GetIndex() + nOffset ];
/*N*/ 
/*N*/ 	return *this;
/*N*/ }


}
