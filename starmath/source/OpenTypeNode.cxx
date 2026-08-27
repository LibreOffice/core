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

#include <node.hxx>
#include "tmpdevice.hxx"

void SmNode::ArrangeOpenType(OutputDevice& rDev, const SmFormat& rFormat)
{
    // Fallback: Default to legacy arrange if OpenType is not implemented
    Arrange(rDev, rFormat);
}

void SmTableNode::ArrangeOpenType(OutputDevice& rDev, const SmFormat& rFormat)
// arranges all subnodes in one column
{
    SmNode* pNode;
    size_t nSize = GetNumSubNodes();

    // make distance depend on font size
    tools::Long nDist
        = +(rFormat.GetDistance(DIS_VERTICAL) * GetFont().GetFontSize().Height()) / 100;

    if (nSize < 1)
        return;

    // arrange subnodes and get maximum width of them
    tools::Long nMaxWidth = 0, nTmp;
    for (size_t i = 0; i < nSize; ++i)
    {
        if (nullptr != (pNode = GetSubNode(i)))
        {
            pNode->ArrangeOpenType(rDev, rFormat);
            if ((nTmp = pNode->GetItalicWidth()) > nMaxWidth)
                nMaxWidth = nTmp;
        }
    }

    Point aPos;
    SmRect::operator=(SmRect(nMaxWidth, 1));
    for (size_t i = 0; i < nSize; ++i)
    {
        if (nullptr != (pNode = GetSubNode(i)))
        {
            const SmRect& rNodeRect = pNode->GetRect();
            const SmNode* pCoNode = pNode->GetLeftMost();
            RectHorAlign eHorAlign = pCoNode->GetRectHorAlign();

            aPos = rNodeRect.AlignTo(*this, RectPos::Bottom, eHorAlign, RectVerAlign::Baseline);
            if (i)
                aPos.AdjustY(nDist);
            pNode->MoveTo(aPos);
            ExtendBy(rNodeRect, nSize > 1 ? RectCopyMBL::None : RectCopyMBL::Arg);
        }
    }
    // #i972#
    if (HasBaseline())
        mnFormulaBaseline = GetBaseline();
    else
    {
        SmTmpDevice aTmpDev(rDev, true);
        aTmpDev.SetFont(GetFont());

        SmRect aRect(aTmpDev, &rFormat, u"a"_ustr, GetFont().GetBorderWidth());
        mnFormulaBaseline = GetAlignM();
        // move from middle position by constant - distance
        // between middle and baseline for single letter
        mnFormulaBaseline += aRect.GetBaseline() - aRect.GetAlignM();
    }
}

void SmLineNode::ArrangeOpenType(OutputDevice& rDev, const SmFormat& rFormat)
// arranges all subnodes in one row with some extra space between
{
    SmNode* pNode;
    size_t nSize = GetNumSubNodes();
    for (size_t i = 0; i < nSize; ++i)
    {
        if (nullptr != (pNode = GetSubNode(i)))
            pNode->ArrangeOpenType(rDev, rFormat);
    }

    SmTmpDevice aTmpDev(rDev, true);
    aTmpDev.SetFont(GetFont());

    if (nSize < 1)
    {
        // provide an empty rectangle with alignment parameters for the "current"
        // font (in order to make "a^1 {}_2^3 a_4" work correctly, that is, have the
        // same sub-/supscript positions.)
        //! be sure to use a character that has explicitly defined HiAttribute
        //! line in rect.cxx such as 'a' in order to make 'vec a' look the same as
        //! 'vec {a}'.
        SmRect::operator=(SmRect(aTmpDev, &rFormat, u"a"_ustr, GetFont().GetBorderWidth()));
        // make sure that the rectangle occupies (almost) no space
        SetWidth(1);
        SetItalicSpaces(0, 0);
        return;
    }

    // make distance depend on font size
    tools::Long nDist
        = (rFormat.GetDistance(DIS_HORIZONTAL) * GetFont().GetFontSize().Height()) / 100;
    if (!IsUseExtraSpaces())
        nDist = 0;

    Point aPos;
    // copy the first node into LineNode and extend by the others
    if (nullptr != (pNode = GetSubNode(0)))
        SmRect::operator=(pNode->GetRect());

    for (size_t i = 1; i < nSize; ++i)
    {
        if (nullptr != (pNode = GetSubNode(i)))
        {
            aPos = pNode->AlignTo(*this, RectPos::Right, RectHorAlign::Center,
                                  RectVerAlign::Baseline);

            // add horizontal space to the left for each but the first sub node
            aPos.AdjustX(nDist);

            pNode->MoveTo(aPos);
            ExtendBy(*pNode, RectCopyMBL::Xor);
        }
    }
}

void SmBinHorNode::ArrangeOpenType(OutputDevice& rDev, const SmFormat& rFormat)
{
    SmNode *pLeft = LeftOperand(), *pOper = Symbol(), *pRight = RightOperand();
    assert(pLeft);
    assert(pOper);
    assert(pRight);

    pOper->SetSize(double(rFormat.GetRelSize(SIZ_OPERATOR)) / 100);

    pLeft->ArrangeOpenType(rDev, rFormat);
    pOper->ArrangeOpenType(rDev, rFormat);
    pRight->ArrangeOpenType(rDev, rFormat);

    const SmRect& rOpRect = pOper->GetRect();

    tools::Long nMul;
    if (o3tl::checked_multiply<tools::Long>(rOpRect.GetWidth(), rFormat.GetDistance(DIS_HORIZONTAL),
                                            nMul))
    {
        SAL_WARN("starmath", "integer overflow");
        return;
    }

    tools::Long nDist = nMul / 100;

    SmRect::operator=(*pLeft);

    Point aPos;
    aPos = pOper->AlignTo(*this, RectPos::Right, RectHorAlign::Center, RectVerAlign::Baseline);
    aPos.AdjustX(nDist);
    pOper->MoveTo(aPos);
    ExtendBy(*pOper, RectCopyMBL::Xor);

    aPos = pRight->AlignTo(*this, RectPos::Right, RectHorAlign::Center, RectVerAlign::Baseline);
    aPos.AdjustX(nDist);

    pRight->MoveTo(aPos);
    ExtendBy(*pRight, RectCopyMBL::Xor);
}

void SmBinVerNode::ArrangeOpenType(OutputDevice& rDev, const SmFormat& rFormat)
{
    SmNode *pNum = GetSubNode(0), *pLine = GetSubNode(1), *pDenom = GetSubNode(2);
    assert(pNum);
    assert(pLine);
    assert(pDenom);

    bool bIsTextmode = rFormat.IsTextmode();
    if (bIsTextmode)
    {
        double fFraction = double(rFormat.GetRelSize(SIZ_INDEX)) / 100;
        pNum->SetSize(fFraction);
        pLine->SetSize(fFraction);
        pDenom->SetSize(fFraction);
    }

    pNum->ArrangeOpenType(rDev, rFormat);
    pDenom->ArrangeOpenType(rDev, rFormat);

    // Describes the amount to shift the numerator/denominator of the fraction when
    // a fraction bar is present. Data is populated from the OpenType MATH table.
    // If the OpenType MATH table is not present fallback values are used.
    // https://w3c.github.io/mathml-core/#fraction-with-nonzero-line-thickness

    tools::Long nFontHeight, nExtLen, nWidth, nAxisHeight, nNumBaseline, nDenomBaseline,
        nNumDescent, nDenomAscent;

    nFontHeight = GetFont().GetFontSize().Height();
    nExtLen = nFontHeight * rFormat.GetDistance(DIS_FRACTION) / 100;
    nWidth = std::max(pNum->GetItalicWidth(), pDenom->GetItalicWidth());
    nAxisHeight = rDev.GetOpenTypeMathConstant(vcl::OpenTypeMathConstant::AxisHeight);
    nNumBaseline = pNum->HasBaseline() ? pNum->GetBaseline() : pNum->GetCenterY();
    nDenomBaseline = pDenom->HasBaseline() ? pDenom->GetBaseline() : pDenom->GetCenterY();

    nNumDescent = pNum->GetBottom() - nNumBaseline;
    nDenomAscent = nDenomBaseline - pDenom->GetTop();

    // We try and read constants to draw the fraction from the OpenType MATH and
    // use fallback values suggested in the MathML Core specification otherwise.

    tools::Long nThick, nNumShiftUp, nDenomShiftDown, nNumGapMin, nDenomGapMin, nNumDist,
        nDenomDist;

    nThick = rDev.GetOpenTypeMathConstant(vcl::OpenTypeMathConstant::FractionRuleThickness);

    if (!bIsTextmode)
    {
        nNumShiftUp = rDev.GetOpenTypeMathConstant(
            vcl::OpenTypeMathConstant::FractionNumeratorDisplayStyleShiftUp);
        nDenomShiftDown = rDev.GetOpenTypeMathConstant(
            vcl::OpenTypeMathConstant::FractionDenominatorDisplayStyleShiftDown);
        nNumGapMin = rDev.GetOpenTypeMathConstant(
            vcl::OpenTypeMathConstant::FractionNumDisplayStyleGapMin);
        nDenomGapMin = rDev.GetOpenTypeMathConstant(
            vcl::OpenTypeMathConstant::FractionDenomDisplayStyleGapMin);
    }
    else
    {
        nNumShiftUp
            = rDev.GetOpenTypeMathConstant(vcl::OpenTypeMathConstant::FractionNumeratorShiftUp);
        nDenomShiftDown
            = rDev.GetOpenTypeMathConstant(vcl::OpenTypeMathConstant::FractionDenominatorShiftDown);
        nNumGapMin
            = rDev.GetOpenTypeMathConstant(vcl::OpenTypeMathConstant::FractionNumeratorGapMin);
        nDenomGapMin
            = rDev.GetOpenTypeMathConstant(vcl::OpenTypeMathConstant::FractionDenominatorGapMin);
    }

    nNumDist
        = std::max<tools::Long>(nNumShiftUp, nAxisHeight + nThick / 2 + nNumGapMin + nNumDescent);
    nDenomDist = std::max<tools::Long>(nDenomShiftDown,
                                       nThick / 2 + nDenomGapMin + nDenomAscent - nAxisHeight);

    // font specialist advised to change the width first
    pLine->AdaptToY(rDev, nThick);
    pLine->AdaptToX(rDev, nWidth + 2 * nExtLen);
    pLine->ArrangeOpenType(rDev, rFormat);

    Point aLinePos(pLine->GetLeft(), pLine->GetTop());
    aLinePos.AdjustY(nAxisHeight - nThick / 2);
    pLine->MoveTo(aLinePos);

    // get horizontal alignment for numerator
    const SmNode* pLM = pNum->GetLeftMost();
    RectHorAlign eHorAlign = pLM->GetRectHorAlign();

    // move numerator to its position
    Point aPos = pNum->AlignTo(*pLine, RectPos::Top, eHorAlign, RectVerAlign::Top);
    tools::Long nMathAxisY = pLine->GetCenterY();

    tools::Long nNumBaselineOffset = nNumBaseline - pNum->GetTop();
    aPos.setY(nMathAxisY - (nNumDist - nAxisHeight) - nNumBaselineOffset);
    pNum->MoveTo(aPos);

    // get horizontal alignment for denominator
    pLM = pDenom->GetLeftMost();
    eHorAlign = pLM->GetRectHorAlign();

    // move denominator to its position
    aPos = pDenom->AlignTo(*pLine, RectPos::Bottom, eHorAlign, RectVerAlign::Top);

    tools::Long nDenomBaselineOffset = nDenomBaseline - pDenom->GetTop();
    aPos.setY(nMathAxisY + (nDenomDist + nAxisHeight) - nDenomBaselineOffset);
    pDenom->MoveTo(aPos);

    SmRect::operator=(*pNum);
    ExtendBy(*pDenom, RectCopyMBL::None).ExtendBy(*pLine, RectCopyMBL::None, pLine->GetCenterY());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
