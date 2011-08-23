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

#ifdef BIDI
#endif


#include "txtcfg.hxx"
#include "pormulti.hxx"
#include "inftxt.hxx"
namespace binfilter {

#ifdef DBG_UTIL

/*N*/ sal_Bool ChkChain( SwLinePortion *pStart )
/*N*/ {
/*N*/ 	SwLinePortion *pPor = pStart->GetPortion();
/*N*/ 	MSHORT nCount = 0;
/*N*/ 	while( pPor )
/*N*/ 	{
/*N*/ 		++nCount;
/*N*/ 		ASSERT( nCount < 200 && pPor != pStart,
/*N*/ 				"ChkChain(): lost in chains" );
/*N*/ 		if( nCount >= 200 || pPor == pStart )
/*N*/ 		{
/*N*/ 			// der Lebensretter
/*?*/ 			pPor = pStart->GetPortion();
/*?*/ 			pStart->SetPortion(0);
/*?*/ 			pPor->Truncate();
/*?*/ 			pStart->SetPortion( pPor );
/*?*/ 			return sal_False;
/*N*/ 		}
/*N*/ 		pPor = pPor->GetPortion();
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }
#endif

#if OSL_DEBUG_LEVEL > 1
const sal_Char *GetPortionName( const MSHORT nType );
#endif

/*N*/ SwLinePortion::~SwLinePortion()
/*N*/ {
/*N*/ }

/*N*/ SwLinePortion *SwLinePortion::Compress()
/*N*/ {
/*N*/ 	return GetLen() || Width() ? this : 0;
/*N*/ }


/*************************************************************************
 *				 SwLinePortion::SwLinePortion( )
 *************************************************************************/

/*N*/ SwLinePortion::SwLinePortion( ) :
/*N*/ 	nLineLength( 0 ),
/*N*/ 	nAscent( 0 ),
/*N*/ 	pPortion( NULL )
/*N*/ {
/*N*/ }

/*************************************************************************
 *               SwLinePortion::PrePaint()
 *************************************************************************/


/*************************************************************************
 *					SwLinePortion::CalcTxtSize()
 *************************************************************************/

/*N*/ void SwLinePortion::CalcTxtSize( const SwTxtSizeInfo &rInf )
/*N*/ {
/*N*/ 	if( GetLen() == rInf.GetLen()  )
/*N*/ 		*((SwPosSize*)this) = GetTxtSize( rInf );
/*N*/ 	else
/*N*/ 	{
/*?*/ 		SwTxtSizeInfo aInf( rInf );
/*?*/ 		aInf.SetLen( GetLen() );
/*?*/ 		*((SwPosSize*)this) = GetTxtSize( aInf );
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *					SwLinePortion::Truncate()
 *
 * Es werden alle nachfolgenden Portions geloescht.
 *************************************************************************/

/*N*/ void SwLinePortion::_Truncate()
/*N*/ {
/*N*/ 	SwLinePortion *pPos = pPortion;
/*N*/ 	do
/*N*/ 	{	ASSERT( pPos != this, "SwLinePortion::Truncate: loop" );
/*N*/ 		SwLinePortion *pLast = pPos;
/*N*/ 		pPos = pPos->GetPortion();
/*N*/ 		pLast->SetPortion( 0 );
/*N*/ 		delete pLast;
/*N*/ 
/*N*/ 	} while( pPos );
/*N*/ 
/*N*/ 	pPortion = 0;
/*N*/ }

/*************************************************************************
 *				  virtual SwLinePortion::Insert()
 *
 * Es wird immer hinter uns eingefuegt.
 *************************************************************************/

/*N*/ SwLinePortion *SwLinePortion::Insert( SwLinePortion *pIns )
/*N*/ {
/*N*/ 	pIns->FindLastPortion()->SetPortion( pPortion );
/*N*/ 	SetPortion( pIns );
/*N*/ #ifdef DBG_UTIL
/*N*/ 	ChkChain( this );
/*N*/ #endif
/*N*/ 	return pIns;
/*N*/ }

/*************************************************************************
 *					SwLinePortion::FindLastPortion()
 *************************************************************************/

/*N*/ SwLinePortion *SwLinePortion::FindLastPortion()
/*N*/ {
/*N*/ 	register SwLinePortion *pPos = this;
/*N*/ 	// An das Ende wandern und pLinPortion an den letzten haengen ...
/*N*/ 	while( pPos->GetPortion() )
/*N*/ 	{
///*N*/ 		DBG_LOOP;
/*N*/ 		pPos = pPos->GetPortion();
/*N*/ 	}
/*N*/ 	return pPos;
/*N*/ }

/*************************************************************************
 *				  virtual SwLinePortion::Append()
 *************************************************************************/

/*N*/ SwLinePortion *SwLinePortion::Append( SwLinePortion *pIns )
/*N*/ {
/*N*/ 	SwLinePortion *pPos = FindLastPortion();
/*N*/ 	pPos->SetPortion( pIns );
/*N*/ 	pIns->SetPortion( 0 );
/*N*/ #ifdef DBG_UTIL
/*N*/ 	ChkChain( this );
/*N*/ #endif
/*N*/ 	return pIns;
/*N*/ }

/*************************************************************************
 *				  virtual SwLinePortion::Cut()
 *************************************************************************/

/*N*/ SwLinePortion *SwLinePortion::Cut( SwLinePortion *pVictim )
/*N*/ {
/*N*/ 	SwLinePortion *pPrev = pVictim->FindPrevPortion( this );
/*N*/ 	ASSERT( pPrev, "SwLinePortion::Cut(): can't cut" );
/*N*/ 	pPrev->SetPortion( pVictim->GetPortion() );
/*N*/ 	pVictim->SetPortion(0);
/*N*/ 	return pVictim;
/*N*/ }

/*************************************************************************
 *				  SwLinePortion::FindPrevPortion()
 *************************************************************************/

/*N*/ SwLinePortion *SwLinePortion::FindPrevPortion( const SwLinePortion *pRoot )
/*N*/ {
/*N*/ 	ASSERT( pRoot != this, "SwLinePortion::FindPrevPortion(): invalid root" );
/*N*/ 	SwLinePortion *pPos = (SwLinePortion*)pRoot;
/*N*/ 	while( pPos->GetPortion() && pPos->GetPortion() != this )
/*N*/ 	{
///*N*/ 		DBG_LOOP;
/*N*/ 		pPos = pPos->GetPortion();
/*N*/ 	}
/*N*/ 	ASSERT( pPos->GetPortion(),
/*N*/ 			"SwLinePortion::FindPrevPortion: blowing in the wind");
/*N*/ 	return pPos;
/*N*/ }

/*************************************************************************
 *				  virtual SwLinePortion::GetCrsrOfst()
 *************************************************************************/


/*************************************************************************
 *				  virtual SwLinePortion::GetTxtSize()
 *************************************************************************/
/*N*/ 
/*N*/ SwPosSize SwLinePortion::GetTxtSize( const SwTxtSizeInfo & ) const
/*N*/ {
/*N*/ 	ASSERT( !this, "SwLinePortion::GetTxtSize: don't ask me about sizes, "
/*N*/ 				   "I'm only a stupid SwLinePortion" );
/*N*/ 	return SwPosSize();
/*N*/ }

#ifdef DBG_UTIL

/*************************************************************************
 *				  virtual SwLinePortion::Check()
 *************************************************************************/

#endif

/*************************************************************************
 *				   virtual SwLinePortion::Format()
 *************************************************************************/

/*N*/ sal_Bool SwLinePortion::Format( SwTxtFormatInfo &rInf )
/*N*/ {
/*N*/ 	if( rInf.X() > rInf.Width() )
/*N*/ 	{
/*?*/ 		Truncate();
/*?*/ 		rInf.SetUnderFlow( this );
/*?*/ 		return sal_True;
/*N*/ 	}
/*N*/ 
/*N*/ 	register const SwLinePortion *pLast = rInf.GetLast();
/*N*/ 	Height( pLast->Height() );
/*N*/ 	SetAscent( pLast->GetAscent() );
/*N*/ 	const KSHORT nNewWidth = rInf.X() + PrtWidth();
/*N*/ 	// Nur Portions mit echter Breite koennen ein sal_True zurueckliefern
/*N*/ 	// Notizen beispielsweise setzen niemals bFull==sal_True
/*N*/     if( rInf.Width() <= nNewWidth && PrtWidth() && ! IsKernPortion() )
/*N*/ 	{
/*?*/ 		Truncate();
/*?*/ 		if( nNewWidth > rInf.Width() )
/*?*/ 			PrtWidth( nNewWidth - rInf.Width() );
/*?*/ 		rInf.GetLast()->FormatEOL( rInf );
/*?*/ 		return sal_True;
/*N*/ 	}
/*N*/ 	return sal_False;
/*N*/ }

/*************************************************************************
 *				   virtual SwLinePortion::FormatEOL()
 *************************************************************************/

// Format end of line

/*N*/ void SwLinePortion::FormatEOL( SwTxtFormatInfo &rInf )
/*N*/ { }

/*************************************************************************
 *						SwLinePortion::Move()
 *************************************************************************/

/*N*/ void SwLinePortion::Move( SwTxtPaintInfo &rInf )
/*N*/ {
/*N*/ 	BOOL bB2T = rInf.GetDirection() == DIR_BOTTOM2TOP;
/*N*/ #ifdef BIDI
/*N*/     const BOOL bFrmDir = rInf.GetTxtFrm()->IsRightToLeft();
/*N*/     BOOL bCounterDir = ( ! bFrmDir && DIR_RIGHT2LEFT == rInf.GetDirection() ) ||
/*N*/                        (   bFrmDir && DIR_LEFT2RIGHT == rInf.GetDirection() );
/*N*/ #endif
/*N*/ 
/*N*/     if ( InSpaceGrp() && rInf.GetSpaceAdd() )
/*N*/ 	{
/*?*/ 		SwTwips nTmp = PrtWidth() +	CalcSpacing( rInf.GetSpaceAdd(), rInf );
/*?*/ 		if( rInf.IsRotated() )
/*?*/ 			rInf.Y( rInf.Y() + ( bB2T ? -nTmp : nTmp ) );
/*?*/ #ifdef BIDI
/*?*/         else if ( bCounterDir )
/*?*/             rInf.X( rInf.X() - nTmp );
/*?*/ #endif
/*?*/         else
/*?*/             rInf.X( rInf.X() + nTmp );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/         if( InFixMargGrp() && !IsMarginPortion() )
/*N*/ 		{
/*N*/ 			rInf.IncSpaceIdx();
/*N*/             rInf.IncKanaIdx();
/*N*/ 		}
/*N*/ 		if( rInf.IsRotated() )
/*?*/ 			rInf.Y( rInf.Y() + ( bB2T ? -PrtWidth() : PrtWidth() ) );
/*N*/ #ifdef BIDI
/*N*/         else if ( bCounterDir )
/*?*/             rInf.X( rInf.X() - PrtWidth() );
/*N*/ #endif
/*N*/         else
/*N*/ 			rInf.X( rInf.X() + PrtWidth() );
/*N*/ 	}
/*N*/ 	if( IsMultiPortion() && ((SwMultiPortion*)this)->HasTabulator() )
/*?*/ 		rInf.IncSpaceIdx();
/*N*/ 
/*N*/ 	rInf.SetIdx( rInf.GetIdx() + GetLen() );
/*N*/ }

/*************************************************************************
 *				virtual SwLinePortion::CalcSpacing()
 *************************************************************************/

/*N*/ long SwLinePortion::CalcSpacing( short nSpaceAdd, const SwTxtSizeInfo &rInf ) const
/*N*/ {
/*N*/ 	return 0;
/*N*/ }

/*************************************************************************
 *				virtual SwLinePortion::GetExpTxt()
 *************************************************************************/

/*N*/ sal_Bool SwLinePortion::GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const
/*N*/ {
/*N*/ 	return sal_False;
/*N*/ }

/*************************************************************************
 *              virtual SwLinePortion::HandlePortion()
 *************************************************************************/


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
