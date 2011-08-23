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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "hints.hxx"
namespace binfilter {

// -----------------------------------------------------------------------

/*N*/ TYPEINIT1(ScPaintHint, SfxHint);
/*N*/ TYPEINIT1(ScUpdateRefHint, SfxHint);
/*N*/ TYPEINIT1(ScPointerChangedHint, SfxHint);
/*N*/ TYPEINIT1(ScLinkRefreshedHint, SfxHint);
/*N*/ TYPEINIT1(ScAutoStyleHint, SfxHint);

// -----------------------------------------------------------------------
//      ScPaintHint - Angabe, was neu gezeichnet werden muss
// -----------------------------------------------------------------------

/*N*/ ScPaintHint::ScPaintHint( const ScRange& rRng, USHORT nPaint ) :
/*N*/ 	aRange( rRng ),
/*N*/ 	nParts( nPaint ),
/*N*/ 	bPrint( TRUE )
/*N*/ {
/*N*/ }

/*N*/ ScPaintHint::~ScPaintHint()
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------
//      ScUpdateRefHint - Referenz-Updaterei
// -----------------------------------------------------------------------

/*N*/ ScUpdateRefHint::ScUpdateRefHint( UpdateRefMode eMode, const ScRange& rR,
/*N*/ 									short nX, short nY, short nZ ) :
/*N*/ 	eUpdateRefMode( eMode ),
/*N*/ 	aRange( rR ),
/*N*/ 	nDx( nX ),
/*N*/ 	nDy( nY ),
/*N*/ 	nDz( nZ )
/*N*/ {
/*N*/ }

/*N*/ ScUpdateRefHint::~ScUpdateRefHint()
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------
//      ScPointerChangedHint - Pointer ist ungueltig geworden
// -----------------------------------------------------------------------

/*N*/ ScPointerChangedHint::ScPointerChangedHint( USHORT nF ) :
/*N*/ 	nFlags( nF )
/*N*/ {
/*N*/ }

/*N*/ ScPointerChangedHint::~ScPointerChangedHint()
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------
//      ScLinkRefreshedHint - a link has been refreshed
// -----------------------------------------------------------------------

/*N*/ ScLinkRefreshedHint::ScLinkRefreshedHint() :
/*N*/ 	nLinkType( SC_LINKREFTYPE_NONE ),
/*N*/ 	nDdeMode( 0 )
/*N*/ {
/*N*/ }

/*N*/ ScLinkRefreshedHint::~ScLinkRefreshedHint()
/*N*/ {
/*N*/ }

/*N*/ void ScLinkRefreshedHint::SetSheetLink( const String& rSourceUrl )
/*N*/ {
/*N*/ 	nLinkType = SC_LINKREFTYPE_SHEET;
/*N*/ 	aUrl = rSourceUrl;
/*N*/ }


/*N*/ void ScLinkRefreshedHint::SetAreaLink( const ScAddress& rPos )
/*N*/ {
/*N*/ 	nLinkType = SC_LINKREFTYPE_AREA;
/*N*/ 	aDestPos = rPos;
/*N*/ }

// -----------------------------------------------------------------------
//      ScAutoStyleHint - STYLE() function has been called
// -----------------------------------------------------------------------






}
