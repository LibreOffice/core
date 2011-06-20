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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <pamtyp.hxx>


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
sal_Bool GotoPrevTable( SwPaM&, SwPosTable, sal_Bool bInReadOnly );
sal_Bool GotoCurrTable( SwPaM&, SwPosTable, sal_Bool bInReadOnly );
sal_Bool GotoNextTable( SwPaM&, SwPosTable, sal_Bool bInReadOnly );

SwWhichTable fnTablePrev = &GotoPrevTable;
SwWhichTable fnTableCurr = &GotoCurrTable;
SwWhichTable fnTableNext = &GotoNextTable;
SwPosTable fnTableStart = &aFwrd;
SwPosTable fnTableEnd = &aBwrd;

// Travelling in Bereichen
sal_Bool GotoPrevRegion( SwPaM&, SwPosRegion, sal_Bool bInReadOnly );
sal_Bool GotoCurrRegion( SwPaM&, SwPosRegion, sal_Bool bInReadOnly );
sal_Bool GotoCurrRegionAndSkip( SwPaM&, SwPosRegion, sal_Bool bInReadOnly );
sal_Bool GotoNextRegion( SwPaM&, SwPosRegion, sal_Bool bInReadOnly );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
