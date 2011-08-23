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

#include "svdoutl.hxx"
#include "svdotext.hxx"
#include <editstat.hxx>
namespace binfilter {


//TYPEINIT1( SdrOutliner, Outliner );

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/
/*N*/ SdrOutliner::SdrOutliner( SfxItemPool* pItemPool, USHORT nMode ):
/*N*/ Outliner( pItemPool, nMode ),
/*N*/ pTextObj( NULL )
/*N*/ {
/*N*/ }


/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/
/*N*/ SdrOutliner::~SdrOutliner()
/*N*/ {
/*N*/ }


/*************************************************************************
|*
|*
|*
\************************************************************************/
/*N*/ void SdrOutliner::SetTextObj( const SdrTextObj* pObj )
/*N*/ {
/*N*/ 	if( pObj && pObj != pTextObj )
/*N*/ 	{
/*N*/ 		SetUpdateMode(FALSE);
/*N*/ 		USHORT nOutlinerMode = OUTLINERMODE_OUTLINEOBJECT;
/*N*/ 		if ( !pObj->IsOutlText() )
/*N*/ 			nOutlinerMode = OUTLINERMODE_TEXTOBJECT;
/*N*/ 		Init( nOutlinerMode );
/*N*/ 
/*N*/ 		SetGlobalCharStretching(100,100);
/*N*/ 
/*N*/ 		ULONG nStat = GetControlWord();
/*N*/ 		nStat &= ~( EE_CNTRL_STRETCHING | EE_CNTRL_AUTOPAGESIZE );
/*N*/ 		SetControlWord(nStat);
/*N*/ 
/*N*/ 		Size aNullSize;
/*N*/ 		Size aMaxSize( 100000,100000 );
/*N*/ 		SetMinAutoPaperSize( aNullSize );
/*N*/ 		SetMaxAutoPaperSize( aMaxSize );
/*N*/ 		SetPaperSize( aMaxSize );
/*N*/ 		ClearPolygon();
/*N*/ 	}
/*N*/ 
/*N*/ 	pTextObj = pObj;
/*N*/ }

/*************************************************************************
|*
|*
|*
\************************************************************************/
/*N*/ void SdrOutliner::SetTextObjNoInit( const SdrTextObj* pObj )
/*N*/ {
/*N*/ 	pTextObj = pObj;
/*N*/ }

/*************************************************************************
|*
|*
|*
\************************************************************************/
/*N*/ XubString SdrOutliner::CalcFieldValue(const SvxFieldItem& rField, USHORT nPara, USHORT nPos,
/*N*/                                      Color*& rpTxtColor, Color*& rpFldColor)
/*N*/ {
/*N*/ 	FASTBOOL bOk = FALSE;
/*N*/ 	XubString aRet;
/*N*/ 
/*N*/ 	if (pTextObj)
/*N*/ 		bOk = pTextObj->CalcFieldValue(rField, nPara, nPos, FALSE, rpTxtColor, rpFldColor, aRet);
/*N*/ 
/*N*/ 	if (!bOk)
/*N*/ 		aRet = Outliner::CalcFieldValue(rField, nPara, nPos, rpTxtColor, rpFldColor);
/*N*/ 
/*N*/ 	return aRet;
/*N*/ }





}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
