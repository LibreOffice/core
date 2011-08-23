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

#include "asciiopt.hxx"
#include "asciiopt.hrc"

#include <rtl/tencinfo.h>
namespace binfilter {


// ============================================================================

//! TODO make dynamic
#ifdef WIN
const sal_Int32 ASCIIDLG_MAXROWS                = 10000;
#else
const sal_Int32 ASCIIDLG_MAXROWS                = 32000;
#endif

static const sal_Char __FAR_DATA pStrFix[] = "FIX";
static const sal_Char __FAR_DATA pStrMrg[] = "MRG";


// ============================================================================

/*N*/ ScAsciiOptions::ScAsciiOptions() :
/*N*/ 	bFixedLen		( FALSE ),
/*N*/ 	aFieldSeps		( ';' ),
/*N*/ 	bMergeFieldSeps	( FALSE ),
/*N*/ 	cTextSep		( 34 ),
/*N*/ 	eCharSet		( gsl_getSystemTextEncoding() ),
/*N*/ 	bCharSetSystem	( FALSE ),
/*N*/ 	nStartRow		( 1 ),
/*N*/ 	nInfoCount		( 0 ),
/*N*/     pColStart       ( NULL ),
/*N*/ 	pColFormat		( NULL )
/*N*/ {
/*N*/ }


/*N*/ ScAsciiOptions::ScAsciiOptions(const ScAsciiOptions& rOpt) :
/*N*/ 	bFixedLen		( rOpt.bFixedLen ),
/*N*/ 	aFieldSeps		( rOpt.aFieldSeps ),
/*N*/ 	bMergeFieldSeps	( rOpt.bMergeFieldSeps ),
/*N*/ 	cTextSep		( rOpt.cTextSep ),
/*N*/ 	eCharSet		( rOpt.eCharSet ),
/*N*/ 	bCharSetSystem	( rOpt.bCharSetSystem ),
/*N*/ 	nStartRow		( rOpt.nStartRow ),
/*N*/ 	nInfoCount		( rOpt.nInfoCount )
/*N*/ {
/*N*/ 	if (nInfoCount)
/*N*/ 	{
/*N*/ 		pColStart = new xub_StrLen[nInfoCount];
/*N*/ 		pColFormat = new BYTE[nInfoCount];
/*N*/ 		for (USHORT i=0; i<nInfoCount; i++)
/*N*/ 		{
/*N*/ 			pColStart[i] = rOpt.pColStart[i];
/*N*/ 			pColFormat[i] = rOpt.pColFormat[i];
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		pColStart = NULL;
/*N*/ 		pColFormat = NULL;
/*N*/ 	}
/*N*/ }


/*N*/ ScAsciiOptions::~ScAsciiOptions()
/*N*/ {
/*N*/ 	delete[] pColStart;
/*N*/ 	delete[] pColFormat;
/*N*/ }









//
//	Der Options-String darf kein Semikolon mehr enthalten (wegen Pickliste)
//	darum ab Version 336 Komma stattdessen
//


/*N*/ void ScAsciiOptions::ReadFromString( const String& rString )
/*N*/ {
/*N*/ 	xub_StrLen nCount = rString.GetTokenCount(',');
/*N*/ 	String aToken;
/*N*/ 	xub_StrLen nSub;
/*N*/ 	xub_StrLen i;
/*N*/ 
/*N*/ 		//
/*N*/ 		//	Feld-Trenner
/*N*/ 		//
/*N*/ 
/*N*/ 	if ( nCount >= 1 )
/*N*/ 	{
/*N*/ 		bFixedLen = bMergeFieldSeps = FALSE;
/*N*/ 		aFieldSeps.Erase();
/*N*/ 
/*N*/ 		aToken = rString.GetToken(0,',');
/*N*/ 		if ( aToken.EqualsAscii(pStrFix) )
/*N*/ 			bFixedLen = TRUE;
/*N*/ 		nSub = aToken.GetTokenCount('/');
/*N*/ 		for ( i=0; i<nSub; i++ )
/*N*/ 		{
/*N*/ 			String aCode = aToken.GetToken( i, '/' );
/*N*/ 			if ( aCode.EqualsAscii(pStrMrg) )
/*N*/ 				bMergeFieldSeps = TRUE;
/*N*/ 			else
/*N*/ 			{
/*N*/ 				sal_Int32 nVal = aCode.ToInt32();
/*N*/ 				if ( nVal )
/*N*/ 					aFieldSeps += (sal_Unicode) nVal;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 		//
/*N*/ 		//	Text-Trenner
/*N*/ 		//
/*N*/ 
/*N*/ 	if ( nCount >= 2 )
/*N*/ 	{
/*N*/ 		aToken = rString.GetToken(1,',');
/*N*/ 		sal_Int32 nVal = aToken.ToInt32();
/*N*/ 		cTextSep = (sal_Unicode) nVal;
/*N*/ 	}
/*N*/ 
/*N*/ 		//
/*N*/ 		//	Zeichensatz
/*N*/ 		//
/*N*/ 
/*N*/ 	if ( nCount >= 3 )
/*N*/ 	{
/*N*/ 		aToken = rString.GetToken(2,',');
/*N*/ 		eCharSet = ScGlobal::GetCharsetValue( aToken );
/*N*/ 	}
/*N*/ 
/*N*/ 		//
/*N*/ 		//	Startzeile
/*N*/ 		//
/*N*/ 
/*N*/ 	if ( nCount >= 4 )
/*N*/ 	{
/*N*/ 		aToken = rString.GetToken(3,',');
/*N*/ 		nStartRow = aToken.ToInt32();
/*N*/ 	}
/*N*/ 
/*N*/ 		//
/*N*/ 		//	Spalten-Infos
/*N*/ 		//
/*N*/ 
/*N*/ 	if ( nCount >= 5 )
/*N*/ 	{
/*N*/ 		delete[] pColStart;
/*N*/ 		delete[] pColFormat;
/*N*/ 
/*N*/ 		aToken = rString.GetToken(4,',');
/*N*/ 		nSub = aToken.GetTokenCount('/');
/*N*/ 		nInfoCount = nSub / 2;
/*N*/ 		if (nInfoCount)
/*N*/ 		{
/*N*/ 			pColStart = new xub_StrLen[nInfoCount];
/*N*/ 			pColFormat = new BYTE[nInfoCount];
/*N*/ 			for (USHORT nInfo=0; nInfo<nInfoCount; nInfo++)
/*N*/ 			{
/*N*/ 				pColStart[nInfo]  = (xub_StrLen) aToken.GetToken( 2*nInfo, '/' ).ToInt32();
/*N*/ 				pColFormat[nInfo] = (BYTE) aToken.GetToken( 2*nInfo+1, '/' ).ToInt32();
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			pColStart = NULL;
/*N*/ 			pColFormat = NULL;
/*N*/ 		}
/*N*/ 	}
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
