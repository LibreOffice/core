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

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _TXTTXMRK_HXX //autogen
#include <txttxmrk.hxx>
#endif
#ifndef _TOX_HXX
#include <tox.hxx>
#endif
namespace binfilter {

/*N*/ SwTxtTOXMark::SwTxtTOXMark( const SwTOXMark& rAttr,
/*N*/ 					xub_StrLen nStart, xub_StrLen* pEnde )
/*N*/ 	: SwTxtAttrEnd( rAttr, nStart, nStart ),
/*N*/ 	pEnd( 0 ),
/*N*/ 	pMyTxtNd( 0 )
/*N*/ {
/*N*/ 	((SwTOXMark&)rAttr).pTxtAttr = this;
/*N*/ 	if( !rAttr.GetAlternativeText().Len() )
/*N*/ 	{
/*N*/ 		nEnd = *pEnde;
/*N*/ 		pEnd = &nEnd;
/*N*/ 	}
/*N*/ 	SetDontMergeAttr( TRUE );
/*N*/ 	SetDontMoveAttr( TRUE );
/*N*/ 	SetOverlapAllowedAttr( TRUE );
/*N*/ }

/*N*/ SwTxtTOXMark::~SwTxtTOXMark()
/*N*/ {
/*N*/ }

/*N*/ xub_StrLen* SwTxtTOXMark::GetEnd()
/*N*/ {
/*N*/ 	return pEnd;
/*N*/ }

/*N*/ void SwTxtTOXMark::CopyTOXMark( SwDoc* pDoc )
/*N*/ {
/*N*/ 	SwTOXMark& rTOX = (SwTOXMark&)GetTOXMark();
/*N*/ 	TOXTypes    eType   = rTOX.GetTOXType()->GetType();
/*N*/ 	USHORT      nCount  = pDoc->GetTOXTypeCount( eType );
/*N*/ 	const SwTOXType* pType = 0;
/*N*/ 	const XubString& rNm = rTOX.GetTOXType()->GetTypeName();
/*N*/ 
/*N*/ 	// kein entsprechender Verzeichnistyp vorhanden -> anlegen
/*N*/ 	// sonst verwenden
/*N*/ 	for(USHORT i=0; i < nCount; ++i)
/*N*/ 	{
/*N*/ 		const SwTOXType* pSrcType = pDoc->GetTOXType(eType, i);
/*N*/ 		if(pSrcType->GetTypeName() == rNm )
/*N*/ 		{
/*N*/ 			pType = pSrcType;
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	// kein entsprechender Typ vorhanden -> neu erzeugen
/*N*/ 	//
/*N*/ 	if(!pType)
/*N*/ 	{
/*N*/ 		pDoc->InsertTOXType( SwTOXType( eType, rNm ) );
/*N*/ 		pType = pDoc->GetTOXType(eType, 0);
/*N*/ 	}
/*N*/ 	// Verzeichnistyp umhaengen
/*N*/ 	//
/*N*/ 	((SwTOXType*)pType)->Add( &rTOX );
/*N*/ }
}
