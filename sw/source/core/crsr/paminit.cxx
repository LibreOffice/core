/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <pam.hxx>
#include <pamtyp.hxx>
#include <trvlreg.hxx>
#include <trvltbl.hxx>

static SwMoveFnCollection aFwrd = {
    /* fnNd         */  &GoNext,
    /* fnNds        */  &GoNextNds,
    /* fnDoc        */  &GoEndDoc,
    /* fnSections   */  &GoEndSection,
    /* fnCmpOp      */  &SwPosition::operator<,
    /* fnGetHint    */  &GetFrwrdTextHint,
    /* fnSearch     */  &utl::TextSearch::SearchForward,
    /* fnSection    */  &SwNodes::GoStartOfSection
};

static SwMoveFnCollection aBwrd = {
    /* fnNd         */  &GoPrevious,
    /* fnNds        */  &GoPreviousNds,
    /* fnDoc        */  &GoStartDoc,
    /* fnSections   */  &GoStartSection,
    /* fnCmpOp      */  &SwPosition::operator>,
    /* fnGetHint    */  &GetBkwrdTextHint,
    /* fnSearch     */  &utl::TextSearch::SearchBackward,
    /* fnSection    */  &SwNodes::GoEndOfSection
};

SwGoInDoc fnGoDoc       = &GoInDoc;
SwGoInDoc fnGoSection   = &GoInSection;
SwGoInDoc fnGoNode      = &GoInNode;
SwGoInDoc fnGoContent     = &GoInContent;
SwGoInDoc fnGoContentCells = &GoInContentCells;
SwGoInDoc fnGoContentSkipHidden      = &GoInContentSkipHidden;
SwGoInDoc fnGoContentCellsSkipHidden = &GoInContentCellsSkipHidden;

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

SwWhichTable fnTablePrev = &GotoPrevTable;
SwWhichTable fnTableCurr = &GotoCurrTable;
SwWhichTable fnTableNext = &GotoNextTable;
SwPosTable fnTableStart = &aFwrd;
SwPosTable fnTableEnd = &aBwrd;

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
