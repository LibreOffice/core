/*************************************************************************
 *
 *  $RCSfile: paminit.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _PAMTYP_HXX
#include <pamtyp.hxx>
#endif


static SwMoveFnCollection aFwrd = {
    /* fnNd         */  &GoNext,
    /* fnNds        */  &GoNextNds,
    /* fnDoc        */  &GoEndDoc,
    /* fnSections   */  &GoEndSection,
    /* fnCmpOp      */  &SwPosition::operator<,
    /* fnGetHint    */  &GetFrwrdTxtHint,
    /* fnSearch     */  &SearchText::SearchFrwrd,
    /* fnSection    */  &SwNodes::GoStartOfSection
};

static SwMoveFnCollection aBwrd = {
    /* fnNd         */  &GoPrevious,
    /* fnNds        */  &GoPreviousNds,
    /* fnDoc        */  &GoStartDoc,
    /* fnSections   */  &GoStartSection,
    /* fnCmpOp      */  &SwPosition::operator>,
    /* fnGetHint    */  &GetBkwrdTxtHint,
    /* fnSearch     */  &SearchText::SearchBkwrd,
    /* fnSection    */  &SwNodes::GoEndOfSection
};

SwGoInDoc fnGoDoc       = &GoInDoc;
SwGoInDoc fnGoSection   = &GoInSection;
SwGoInDoc fnGoNode      = &GoInNode;
SwGoInDoc fnGoCntnt     = &GoInCntnt;

SwWhichPara fnParaPrev  = &GoPrevPara;
SwWhichPara fnParaCurr  = &GoCurrPara;
SwWhichPara fnParaNext  = &GoNextPara;
SwPosPara fnParaStart   = &aFwrd;
SwPosPara fnParaEnd     = &aBwrd;

SwWhichSection fnSectionPrev    = &GoPrevSection;
SwWhichSection fnSectionCurr    = &GoCurrSection;
SwWhichSection fnSectionNext    = &GoNextSection;
SwPosSection fnSectionStart     = &aFwrd;
SwPosSection fnSectionEnd       = &aBwrd;

// Travelling in Tabellen
FASTBOOL GotoPrevTable( SwPaM&, SwPosTable, FASTBOOL bInReadOnly );
FASTBOOL GotoCurrTable( SwPaM&, SwPosTable, FASTBOOL bInReadOnly );
FASTBOOL GotoNextTable( SwPaM&, SwPosTable, FASTBOOL bInReadOnly );

SwWhichTable fnTablePrev = &GotoPrevTable;
SwWhichTable fnTableCurr = &GotoCurrTable;
SwWhichTable fnTableNext = &GotoNextTable;
SwPosTable fnTableStart = &aFwrd;
SwPosTable fnTableEnd = &aBwrd;

// Travelling in Bereichen
FASTBOOL GotoPrevRegion( SwPaM&, SwPosRegion, FASTBOOL bInReadOnly );
FASTBOOL GotoCurrRegion( SwPaM&, SwPosRegion, FASTBOOL bInReadOnly );
FASTBOOL GotoCurrRegionAndSkip( SwPaM&, SwPosRegion, FASTBOOL bInReadOnly );
FASTBOOL GotoNextRegion( SwPaM&, SwPosRegion, FASTBOOL bInReadOnly );

SwWhichRegion fnRegionPrev = &GotoPrevRegion;
SwWhichRegion fnRegionCurr = &GotoCurrRegion;
SwWhichRegion fnRegionCurrAndSkip = &GotoCurrRegionAndSkip;
SwWhichRegion fnRegionNext = &GotoNextRegion;
SwPosRegion fnRegionStart = &aFwrd;
SwPosRegion fnRegionEnd = &aBwrd;

SwMoveFn fnMoveBackward = &aBwrd;
SwMoveFn fnMoveForward  = &aFwrd;


#ifdef USED

// JP 30.11.95:
//  war fuer die CFRONT-Compiler noetig, jetzt sollte das nicht mehr
//  benoetigt werden.

void _InitPam()
{
    aBwrd.fnNd      =  &GoPrevious;
    aBwrd.fnNds     =  &GoPreviousNds;
    aBwrd.fnDoc     =  &GoStartDoc;
    aBwrd.fnSections=  &GoStartSection;
    aBwrd.fnCmpOp   =  &SwPosition::operator>;
    aBwrd.fnGetHint =  &GetBkwrdTxtHint;
    aBwrd.fnSearch  =  &SearchText::SearchBkwrd;
    aBwrd.fnSection =  &SwNodes::GoEndOfSection;

    aFwrd.fnNd      =  &GoNext;
    aFwrd.fnNds     =  &GoNextNds;
    aFwrd.fnDoc     =  &GoEndDoc;
    aFwrd.fnSections=  &GoEndSection;
    aFwrd.fnCmpOp   =  &SwPosition::operator<;
    aFwrd.fnGetHint =  &GetFrwrdTxtHint;
    aFwrd.fnSearch  =  &SearchText::SearchFrwrd;
    aFwrd.fnSection =  &SwNodes::GoStartOfSection;
}
#endif


