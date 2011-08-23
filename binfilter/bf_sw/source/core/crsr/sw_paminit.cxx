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

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#ifndef _PAMTYP_HXX
#include <pamtyp.hxx>
#endif
namespace binfilter {


/*N*/ static SwMoveFnCollection aFwrd = {
/*N*/ 	/* fnNd			*/  &GoNext,
/*N*/ 	/* fnNds 		*/  &GoNextNds,
/*N*/ 	/* fnDoc 		*/  &GoEndDoc,
/*N*/ 	/* fnSections	*/  &GoEndSection,
/*N*/ 	/* fnCmpOp		*/  &SwPosition::operator<,
/*N*/ 	/* fnGetHint 	*/  &GetFrwrdTxtHint,
/*N*/ 	/* fnSearch		*/  &::utl::TextSearch::SearchFrwrd,
/*N*/ 	/* fnSection 	*/  &SwNodes::GoStartOfSection
/*N*/ };

/*N*/ static SwMoveFnCollection aBwrd = {
/*N*/ 	/* fnNd			*/  &GoPrevious,
/*N*/ 	/* fnNds 		*/  &GoPreviousNds,
/*N*/ 	/* fnDoc 		*/  &GoStartDoc,
/*N*/ 	/* fnSections	*/  &GoStartSection,
/*N*/ 	/* fnCmpOp		*/  &SwPosition::operator>,
/*N*/ 	/* fnGetHint 	*/  &GetBkwrdTxtHint,
/*N*/ 	/* fnSearch		*/  &::utl::TextSearch::SearchBkwrd,
/*N*/ 	/* fnSection 	*/  &SwNodes::GoEndOfSection
/*N*/ };

/*N*/ SwGoInDoc fnGoDoc 		= &GoInDoc;
/*N*/ SwGoInDoc fnGoSection	= &GoInSection;
/*N*/ SwGoInDoc fnGoNode		= &GoInNode;
/*N*/ SwGoInDoc fnGoCntnt		= &GoInCntnt;
/*N*/ SwGoInDoc fnGoCntntCells= &GoInCntntCells;

/*N*/ SwWhichPara fnParaPrev	= &GoPrevPara;
/*N*/ SwWhichPara fnParaCurr	= &GoCurrPara;
/*N*/ SwWhichPara fnParaNext	= &GoNextPara;
/*N*/ SwPosPara fnParaStart	= &aFwrd;
/*N*/ SwPosPara fnParaEnd		= &aBwrd;

/*N*/ SwWhichSection fnSectionPrev	= &GoPrevSection;
/*N*/ SwWhichSection fnSectionCurr	= &GoCurrSection;
/*N*/ SwWhichSection fnSectionNext	= &GoNextSection;
/*N*/ SwPosSection fnSectionStart		= &aFwrd;
/*N*/ SwPosSection fnSectionEnd		= &aBwrd;

// Travelling in Tabellen
/*N*/ FASTBOOL GotoPrevTable( SwPaM&, SwPosTable, FASTBOOL bInReadOnly );
/*N*/ FASTBOOL GotoCurrTable( SwPaM&, SwPosTable, FASTBOOL bInReadOnly );
/*N*/ FASTBOOL GotoNextTable( SwPaM&, SwPosTable, FASTBOOL bInReadOnly );

/*N*/ SwWhichTable fnTablePrev = &GotoPrevTable;
/*N*/ SwWhichTable fnTableCurr = &GotoCurrTable;
/*N*/ SwWhichTable fnTableNext = &GotoNextTable;
/*N*/ SwPosTable fnTableStart = &aFwrd;
/*N*/ SwPosTable fnTableEnd = &aBwrd;

// Travelling in Bereichen
/*N*/ FASTBOOL GotoPrevRegion( SwPaM&, SwPosRegion, FASTBOOL bInReadOnly );
/*N*/ FASTBOOL GotoCurrRegion( SwPaM&, SwPosRegion, FASTBOOL bInReadOnly );
/*N*/ FASTBOOL GotoCurrRegionAndSkip( SwPaM&, SwPosRegion, FASTBOOL bInReadOnly );
/*N*/ FASTBOOL GotoNextRegion( SwPaM&, SwPosRegion, FASTBOOL bInReadOnly );

/*N*/ SwWhichRegion fnRegionPrev = &GotoPrevRegion;
/*N*/ SwWhichRegion fnRegionCurr = &GotoCurrRegion;
/*N*/ SwWhichRegion fnRegionCurrAndSkip = &GotoCurrRegionAndSkip;
/*N*/ SwWhichRegion fnRegionNext = &GotoNextRegion;
/*N*/ SwPosRegion fnRegionStart = &aFwrd;
/*N*/ SwPosRegion fnRegionEnd = &aBwrd;
/*N*/ 
/*N*/ SwMoveFn fnMoveBackward	= &aBwrd;
/*N*/ SwMoveFn fnMoveForward	= &aFwrd;


#ifdef USED

// JP 30.11.95:
//  war fuer die CFRONT-Compiler noetig, jetzt sollte das nicht mehr
//	benoetigt werden.

/*N*/ void _InitPam()
/*N*/ {
/*N*/  	aBwrd.fnNd		=  &GoPrevious;
/*N*/ 	aBwrd.fnNds 	=  &GoPreviousNds;
/*N*/ 	aBwrd.fnDoc 	=  &GoStartDoc;
/*N*/ 	aBwrd.fnSections=  &GoStartSection;
/*N*/ 	aBwrd.fnCmpOp	=  &SwPosition::operator>;
/*N*/ 	aBwrd.fnGetHint =  &GetBkwrdTxtHint;
/*N*/ 	aBwrd.fnSearch	=  &SearchText::SearchBkwrd;
/*N*/ 	aBwrd.fnSection =  &SwNodes::GoEndOfSection;
/*N*/ 
/*N*/ 	aFwrd.fnNd		=  &GoNext;
/*N*/ 	aFwrd.fnNds 	=  &GoNextNds;
/*N*/ 	aFwrd.fnDoc 	=  &GoEndDoc;
/*N*/ 	aFwrd.fnSections=  &GoEndSection;
/*N*/ 	aFwrd.fnCmpOp	=  &SwPosition::operator<;
/*N*/ 	aFwrd.fnGetHint =  &GetFrwrdTxtHint;
/*N*/ 	aFwrd.fnSearch	=  &SearchText::SearchFrwrd;
/*N*/ 	aFwrd.fnSection =  &SwNodes::GoStartOfSection;
/*N*/ }
#endif


}
