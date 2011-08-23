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
#include "lineinfo.hxx"
#include "charfmt.hxx"
#include "poolfmt.hxx"
#include "rootfrm.hxx"
#include "viewsh.hxx"
namespace binfilter {

/*N*/ void SwDoc::SetLineNumberInfo( const SwLineNumberInfo &rNew )
/*N*/ {
/*N*/ 	if ( GetRootFrm() &&
/*N*/ 		 (rNew.IsCountBlankLines() != pLineNumberInfo->IsCountBlankLines() ||
/*N*/ 		  rNew.IsRestartEachPage() != pLineNumberInfo->IsRestartEachPage()) )
/*N*/ 	{
/*?*/ 		GetRootFrm()->StartAllAction();
/*?*/ 		GetRootFrm()->InvalidateAllCntnt( INV_LINENUM );
/*?*/ 		GetRootFrm()->EndAllAction();
/*N*/ 	}
/*N*/ 	*pLineNumberInfo = rNew;
/*N*/ 	SetModified();
/*N*/ }

/*N*/ SwLineNumberInfo::SwLineNumberInfo() :
/*N*/ 	nPosFromLeft( MM50 ),
/*N*/ 	nCountBy( 5 ),
/*N*/ 	nDividerCountBy( 3 ),
/*N*/ 	ePos( LINENUMBER_POS_LEFT ),
/*N*/ 	bPaintLineNumbers( FALSE ),
/*N*/ 	bCountBlankLines( TRUE ),
/*N*/ 	bCountInFlys( FALSE ),
/*N*/ 	bRestartEachPage( FALSE )
/*N*/ {
/*N*/ }

/*N*/ SwLineNumberInfo::SwLineNumberInfo(const SwLineNumberInfo &rCpy ) :
/*N*/ 	aType( rCpy.GetNumType() ),
/*N*/ 	aDivider( rCpy.GetDivider() ),
/*N*/ 	nPosFromLeft( rCpy.GetPosFromLeft() ),
/*N*/ 	nCountBy( rCpy.GetCountBy() ),
/*N*/ 	nDividerCountBy( rCpy.GetDividerCountBy() ),
/*N*/ 	ePos( rCpy.GetPos() ),
/*N*/ 	bPaintLineNumbers( rCpy.IsPaintLineNumbers() ),
/*N*/ 	bCountBlankLines( rCpy.IsCountBlankLines() ),
/*N*/ 	bCountInFlys( rCpy.IsCountInFlys() ),
/*N*/ 	bRestartEachPage( rCpy.IsRestartEachPage() )
/*N*/ {
/*N*/ 	if ( rCpy.GetRegisteredIn() )
/*?*/ 		((SwModify*)rCpy.GetRegisteredIn())->Add( this );
/*N*/ }

/*N*/ SwLineNumberInfo& SwLineNumberInfo::operator=(const SwLineNumberInfo &rCpy)
/*N*/ {
/*N*/ 	if ( rCpy.GetRegisteredIn() )
/*N*/ 		((SwModify*)rCpy.GetRegisteredIn())->Add( this );
/*N*/ 	else if ( GetRegisteredIn() )
/*?*/ 		pRegisteredIn->Remove( this );
/*N*/ 
/*N*/ 	aType = rCpy.GetNumType();
/*N*/ 	aDivider = rCpy.GetDivider();
/*N*/ 	nPosFromLeft = rCpy.GetPosFromLeft();
/*N*/ 	nCountBy = rCpy.GetCountBy();
/*N*/ 	nDividerCountBy = rCpy.GetDividerCountBy();
/*N*/ 	ePos = rCpy.GetPos();
/*N*/ 	bPaintLineNumbers = rCpy.IsPaintLineNumbers();
/*N*/ 	bCountBlankLines = rCpy.IsCountBlankLines();
/*N*/ 	bCountInFlys = rCpy.IsCountInFlys();
/*N*/ 	bRestartEachPage = rCpy.IsRestartEachPage();
/*N*/ 
/*N*/ 	return *this;
/*N*/ }



/*N*/ SwCharFmt* SwLineNumberInfo::GetCharFmt(SwDoc &rDoc) const
/*N*/ {
/*N*/ 	if ( !GetRegisteredIn() )
/*?*/ 	{
/*?*/ 		SwCharFmt* pFmt = rDoc.GetCharFmtFromPool( RES_POOLCHR_LINENUM );
/*?*/ 		pFmt->Add( (SwClient*)this );
/*?*/ 	}
/*N*/ 	return (SwCharFmt*)GetRegisteredIn();
/*N*/ }

/*N*/ void SwLineNumberInfo::SetCharFmt( SwCharFmt *pChFmt )
/*N*/ {
/*N*/ 	ASSERT( pChFmt, "SetCharFmt, 0 is not a valid pointer" );
/*N*/ 	pChFmt->Add( this );
/*N*/ }

/*N*/ void SwLineNumberInfo::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
/*N*/ {
/*N*/ 	SwClient::Modify( pOld, pNew );
/*N*/ 	SwDoc *pDoc = ((SwCharFmt*)GetRegisteredIn())->GetDoc();
/*N*/ 	SwRootFrm* pRoot = pDoc->GetRootFrm();
/*N*/ 	if( pRoot && pRoot->GetCurrShell() )
/*N*/ 	{
/*?*/ 		pRoot->StartAllAction();
/*?*/ 		pRoot->GetCurrShell()->AddPaintRect( pRoot->Frm() );
/*?*/ 		pRoot->EndAllAction();
/*N*/ 	}
/*N*/ }

}
