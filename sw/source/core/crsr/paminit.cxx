/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: paminit.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:30:30 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


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
    /* fnSearch     */  &utl::TextSearch::SearchFrwrd,
    /* fnSection    */  &SwNodes::GoStartOfSection
};

static SwMoveFnCollection aBwrd = {
    /* fnNd         */  &GoPrevious,
    /* fnNds        */  &GoPreviousNds,
    /* fnDoc        */  &GoStartDoc,
    /* fnSections   */  &GoStartSection,
    /* fnCmpOp      */  &SwPosition::operator>,
    /* fnGetHint    */  &GetBkwrdTxtHint,
    /* fnSearch     */  &utl::TextSearch::SearchBkwrd,
    /* fnSection    */  &SwNodes::GoEndOfSection
};

SwGoInDoc fnGoDoc       = &GoInDoc;
SwGoInDoc fnGoSection   = &GoInSection;
SwGoInDoc fnGoNode      = &GoInNode;
SwGoInDoc fnGoCntnt     = &GoInCntnt;
SwGoInDoc fnGoCntntCells = &GoInCntntCells;
SwGoInDoc fnGoCntntSkipHidden      = &GoInCntntSkipHidden;
SwGoInDoc fnGoCntntCellsSkipHidden = &GoInCntntCellsSkipHidden;

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
BOOL GotoPrevTable( SwPaM&, SwPosTable, BOOL bInReadOnly );
BOOL GotoCurrTable( SwPaM&, SwPosTable, BOOL bInReadOnly );
BOOL GotoNextTable( SwPaM&, SwPosTable, BOOL bInReadOnly );

SwWhichTable fnTablePrev = &GotoPrevTable;
SwWhichTable fnTableCurr = &GotoCurrTable;
SwWhichTable fnTableNext = &GotoNextTable;
SwPosTable fnTableStart = &aFwrd;
SwPosTable fnTableEnd = &aBwrd;

// Travelling in Bereichen
BOOL GotoPrevRegion( SwPaM&, SwPosRegion, BOOL bInReadOnly );
BOOL GotoCurrRegion( SwPaM&, SwPosRegion, BOOL bInReadOnly );
BOOL GotoCurrRegionAndSkip( SwPaM&, SwPosRegion, BOOL bInReadOnly );
BOOL GotoNextRegion( SwPaM&, SwPosRegion, BOOL bInReadOnly );

SwWhichRegion fnRegionPrev = &GotoPrevRegion;
SwWhichRegion fnRegionCurr = &GotoCurrRegion;
SwWhichRegion fnRegionCurrAndSkip = &GotoCurrRegionAndSkip;
SwWhichRegion fnRegionNext = &GotoNextRegion;
SwPosRegion fnRegionStart = &aFwrd;
SwPosRegion fnRegionEnd = &aBwrd;

SwMoveFn fnMoveBackward = &aBwrd;
SwMoveFn fnMoveForward  = &aFwrd;

SwWhichPara GetfnParaCurr()
{
    return fnParaCurr;
}
SwPosPara GetfnParaStart()
{
     return  fnParaStart;
}
SwWhichTable GetfnTablePrev()
{
    return fnTablePrev;
}
SwPosPara GetfnParaEnd()
{
     return  fnParaEnd;
}
SwPosTable GetfnTableStart()
{
    return fnTableStart;
}
SwWhichTable GetfnTableCurr()
{
    return fnTableCurr;
}
SwPosTable GetfnTableEnd()
{
    return fnTableEnd;
}
