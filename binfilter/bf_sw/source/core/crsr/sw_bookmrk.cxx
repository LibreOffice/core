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


#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>
#endif
#ifndef _SWSERV_HXX
#include <swserv.hxx>
#endif

#ifndef _ERRHDL_HXX //autogen
#include <errhdl.hxx>
#endif
namespace binfilter {

/*N*/ SV_IMPL_REF( SwServerObject )

/*N*/ SwBookmark::SwBookmark(const SwPosition& aPos, const KeyCode& rCode,
/*N*/ 						const String& rName, const String& rShortName )
/*N*/ 	: SwModify( 0 ),
/*N*/ 	aStartMacro( aEmptyStr, aEmptyStr ),
/*N*/ 	aEndMacro  ( aEmptyStr, aEmptyStr ),
/*N*/ 	aCode(rCode),
/*N*/ 	aName(rName),
/*N*/ 	aShortName(rShortName),
/*N*/ 	pPos2( 0 ),
/*N*/ 	eMarkType( BOOKMARK )
/*N*/ {
/*N*/ 	pPos1 = new SwPosition(aPos);
/*N*/ }

// Beim Loeschen von Text werden Bookmarks mitgeloescht!


/*N*/ SwBookmark::~SwBookmark()
/*N*/ {
/*N*/ 	// falls wir noch der DDE-Bookmark sind, dann muss der aus dem
/*N*/ 	// Clipboard ausgetragen werden. Wird automatisch ueber DataChanged
/*N*/ 	// ausgeloest.
/*N*/ 	if( refObj.Is() )
/*N*/ 	{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	}
/*N*/ 
/*N*/ 	delete pPos1;
/*N*/ 	if( pPos2 )
/*N*/ 		delete pPos2;
/*N*/ }

// Vergleiche auf Basis der Dokumentposition

/*N*/ BOOL SwBookmark::operator<(const SwBookmark &rBM) const
/*N*/ {
/*N*/ 	const SwPosition* pThisPos = ( !pPos2 || *pPos1 <= *pPos2 ) ? pPos1 : pPos2;
/*N*/ 	const SwPosition* pBMPos = ( !rBM.pPos2 || *rBM.pPos1 <= *rBM.pPos2 )
/*N*/ 										? rBM.pPos1 : rBM.pPos2;
/*N*/ 
/*N*/ 	return *pThisPos < *pBMPos;
/*N*/ }

/*N*/ BOOL SwBookmark::operator==(const SwBookmark &rBM) const
/*N*/ {
/*N*/ 	return (this == &rBM);
/*N*/ }

/*N*/ SwUNOMark::SwUNOMark( const SwPosition& aPos,
/*N*/ 				const KeyCode& rCode,
/*N*/ 				const String& rName,
/*N*/ 				const String& rShortName )
/*N*/ 	: SwBookmark( aPos, rCode, rName, rShortName )
/*N*/ {
/*N*/ 	eMarkType = UNO_BOOKMARK;
/*N*/ }

}
