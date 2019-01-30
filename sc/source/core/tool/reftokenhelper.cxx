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

#include <reftokenhelper.hxx>
#include <document.hxx>
#include <rangeutl.hxx>
#include <compiler.hxx>
#include <tokenarray.hxx>

#include <rtl/ustring.hxx>
#include <formula/grammar.hxx>
#include <formula/token.hxx>

#include <memory>

using namespace formula;

using ::std::vector;

void ScRefTokenHelper::compileRangeRepresentation(
    vector<ScTokenRef>& rRefTokens, const OUString& rRangeStr, ScDocument* pDoc,
    const sal_Unicode cSep, FormulaGrammar::Grammar eGrammar, bool bOnly3DRef)
{
    // #i107275# ignore parentheses
    OUString aRangeStr = rRangeStr;
    while( (aRangeStr.getLength() >= 2) && (aRangeStr[ 0 ] == '(') && (aRangeStr[ aRangeStr.getLength() - 1 ] == ')') )
        aRangeStr = aRangeStr.copy( 1, aRangeStr.getLength() - 2 );

    bool bFailure = false;
    sal_Int32 nOffset = 0;
    while (nOffset >= 0 && !bFailure)
    {
        OUString aToken;
        ScRangeStringConverter::GetTokenByOffset(aToken, aRangeStr, nOffset, cSep);
        if (nOffset < 0)
            break;

        ScCompiler aCompiler(pDoc, ScAddress(0,0,0), eGrammar);
        std::unique_ptr<ScTokenArray> pArray(aCompiler.CompileString(aToken));

        // There MUST be exactly one reference per range token and nothing
        // else, and it MUST be a valid reference, not some #REF!
        sal_uInt16 nLen = pArray->GetLen();
        if (!nLen)
            continue;   // Should a missing range really be allowed?
        if (nLen != 1)
        {
            bFailure = true;
            break;
        }

        const FormulaToken* p = pArray->FirstToken();
        if (!p)
        {
            bFailure = true;
            break;
        }

        switch (p->GetType())
        {
            case svSingleRef:
                {
                    const ScSingleRefData& rRef = *p->GetSingleRef();
                    if (!rRef.Valid())
                        bFailure = true;
                    else if (bOnly3DRef && !rRef.IsFlag3D())
                        bFailure = true;
                }
                break;
            case svDoubleRef:
                {
                    const ScComplexRefData& rRef = *p->GetDoubleRef();
                    if (!rRef.Valid())
                        bFailure = true;
                    else if (bOnly3DRef && !rRef.Ref1.IsFlag3D())
                        bFailure = true;
                }
                break;
            case svExternalSingleRef:
                {
                    if (!p->GetSingleRef()->ValidExternal())
                        bFailure = true;
                }
                break;
            case svExternalDoubleRef:
                {
                    if (!p->GetDoubleRef()->ValidExternal())
                        bFailure = true;
                }
                break;
            case svString:
                if (p->GetString().isEmpty())
                    bFailure = true;
                break;
            default:
                bFailure = true;
                break;
        }
        if (!bFailure)
            rRefTokens.emplace_back(p->Clone());

    }
    if (bFailure)
        rRefTokens.clear();
}

bool ScRefTokenHelper::getRangeFromToken(
    ScRange& rRange, const ScTokenRef& pToken, const ScAddress& rPos, bool bExternal)
{
    StackVar eType = pToken->GetType();
    switch (pToken->GetType())
    {
        case svSingleRef:
        case svExternalSingleRef:
        {
            if ((eType == svExternalSingleRef && !bExternal) ||
                (eType == svSingleRef && bExternal))
                return false;

            const ScSingleRefData& rRefData = *pToken->GetSingleRef();
            rRange.aStart = rRefData.toAbs(rPos);
            rRange.aEnd = rRange.aStart;
            return true;
        }
        case svDoubleRef:
        case svExternalDoubleRef:
        {
            if ((eType == svExternalDoubleRef && !bExternal) ||
                (eType == svDoubleRef && bExternal))
                return false;

            const ScComplexRefData& rRefData = *pToken->GetDoubleRef();
            rRange = rRefData.toAbs(rPos);
            return true;
        }
        default:
            ; // do nothing
    }
    return false;
}

void ScRefTokenHelper::getRangeListFromTokens(
    ScRangeList& rRangeList, const vector<ScTokenRef>& rTokens, const ScAddress& rPos)
{
    for (const auto& rToken : rTokens)
    {
        ScRange aRange;
        getRangeFromToken(aRange, rToken, rPos);
        rRangeList.push_back(aRange);
    }
}

void ScRefTokenHelper::getTokenFromRange(ScTokenRef& pToken, const ScRange& rRange)
{
    ScComplexRefData aData;
    aData.InitRange(rRange);
    aData.Ref1.SetFlag3D(true);

    // Display sheet name on 2nd reference only when the 1st and 2nd refs are on
    // different sheets.
    aData.Ref2.SetFlag3D(rRange.aStart.Tab() != rRange.aEnd.Tab());

    pToken.reset(new ScDoubleRefToken(aData));
}

void ScRefTokenHelper::getTokensFromRangeList(vector<ScTokenRef>& pTokens, const ScRangeList& rRanges)
{
    vector<ScTokenRef> aTokens;
    size_t nCount = rRanges.size();
    aTokens.reserve(nCount);
    for (size_t i = 0; i < nCount; ++i)
    {
        const ScRange & rRange = rRanges[i];
        ScTokenRef pToken;
        ScRefTokenHelper::getTokenFromRange(pToken, rRange);
        aTokens.push_back(pToken);
    }
    pTokens.swap(aTokens);
}

bool ScRefTokenHelper::isRef(const ScTokenRef& pToken)
{
    switch (pToken->GetType())
    {
        case svSingleRef:
        case svDoubleRef:
        case svExternalSingleRef:
        case svExternalDoubleRef:
            return true;
        default:
            ;
    }
    return false;
}

bool ScRefTokenHelper::isExternalRef(const ScTokenRef& pToken)
{
    switch (pToken->GetType())
    {
        case svExternalSingleRef:
        case svExternalDoubleRef:
            return true;
        default:
            ;
    }
    return false;
}

bool ScRefTokenHelper::intersects(
    const vector<ScTokenRef>& rTokens, const ScTokenRef& pToken, const ScAddress& rPos)
{
    if (!isRef(pToken))
        return false;

    bool bExternal = isExternalRef(pToken);
    sal_uInt16 nFileId = bExternal ? pToken->GetIndex() : 0;

    ScRange aRange;
    getRangeFromToken(aRange, pToken, rPos, bExternal);

    for (const ScTokenRef& p : rTokens)
    {
        if (!isRef(p))
            continue;

        if (bExternal != isExternalRef(p))
            continue;

        ScRange aRange2;
        getRangeFromToken(aRange2, p, rPos, bExternal);

        if (bExternal && nFileId != p->GetIndex())
            // different external file
            continue;

        if (aRange.Intersects(aRange2))
            return true;
    }
    return false;
}

namespace {

class JoinRefTokenRanges
{
public:
    /**
     * Insert a new reference token into the existing list of reference tokens,
     * but in that process, try to join as many adjacent ranges as possible.
     *
     * @param rTokens existing list of reference tokens
     * @param rToken new token
     */
    void operator() (vector<ScTokenRef>& rTokens, const ScTokenRef& pToken, const ScAddress& rPos)
    {
        join(rTokens, pToken, rPos);
    }

private:

    /**
     * Check two 1-dimensional ranges to see if they overlap each other.
     *
     * @param nMin1 min value of range 1
     * @param nMax1 max value of range 1
     * @param nMin2 min value of range 2
     * @param nMax2 max value of range 2
     * @param rNewMin min value of new range in case they overlap
     * @param rNewMax max value of new range in case they overlap
     */
    template<typename T>
    static bool overlaps(T nMin1, T nMax1, T nMin2, T nMax2, T& rNewMin, T& rNewMax)
    {
        bool bDisjoint1 = (nMin1 > nMax2) && (nMin1 - nMax2 > 1);
        bool bDisjoint2  = (nMin2 > nMax1) && (nMin2 - nMax1 > 1);
        if (bDisjoint1 || bDisjoint2)
            // These two ranges cannot be joined.  Move on.
            return false;

        T nMin = std::min(nMin1, nMin2);
        T nMax = std::max(nMax1, nMax2);

        rNewMin = nMin;
        rNewMax = nMax;

        return true;
    }

    void join(vector<ScTokenRef>& rTokens, const ScTokenRef& pToken, const ScAddress& rPos)
    {
        // Normalize the token to a double reference.
        ScComplexRefData aData;
        if (!ScRefTokenHelper::getDoubleRefDataFromToken(aData, pToken))
            return;

        // Get the information of the new token.
        bool bExternal = ScRefTokenHelper::isExternalRef(pToken);
        sal_uInt16 nFileId = bExternal ? pToken->GetIndex() : 0;
        svl::SharedString aTabName = bExternal ? pToken->GetString() : svl::SharedString::getEmptyString();

        bool bJoined = false;
        for (ScTokenRef& pOldToken : rTokens)
        {
            if (!ScRefTokenHelper::isRef(pOldToken))
                // A non-ref token should not have been added here in the first
                // place!
                continue;

            if (bExternal != ScRefTokenHelper::isExternalRef(pOldToken))
                // External and internal refs don't mix.
                continue;

            if (bExternal)
            {
                if (nFileId != pOldToken->GetIndex())
                    // Different external files.
                    continue;

                if (aTabName != pOldToken->GetString())
                    // Different table names.
                    continue;
            }

            ScComplexRefData aOldData;
            if (!ScRefTokenHelper::getDoubleRefDataFromToken(aOldData, pOldToken))
                continue;

            ScRange aOld = aOldData.toAbs(rPos), aNew = aData.toAbs(rPos);

            if (aNew.aStart.Tab() != aOld.aStart.Tab() || aNew.aEnd.Tab() != aOld.aEnd.Tab())
                // Sheet ranges differ.
                continue;

            if (aOld.In(aNew))
                // This new range is part of an existing range.  Skip it.
                return;

            bool bSameRows = (aNew.aStart.Row() == aOld.aStart.Row()) && (aNew.aEnd.Row() == aOld.aEnd.Row());
            bool bSameCols = (aNew.aStart.Col() == aOld.aStart.Col()) && (aNew.aEnd.Col() == aOld.aEnd.Col());
            ScComplexRefData aNewData = aOldData;
            bool bJoinRanges = false;
            if (bSameRows)
            {
                SCCOL nNewMin, nNewMax;
                bJoinRanges = overlaps(
                    aNew.aStart.Col(), aNew.aEnd.Col(), aOld.aStart.Col(), aOld.aEnd.Col(),
                    nNewMin, nNewMax);

                if (bJoinRanges)
                {
                    aNew.aStart.SetCol(nNewMin);
                    aNew.aEnd.SetCol(nNewMax);
                    aNewData.SetRange(aNew, rPos);
                }
            }
            else if (bSameCols)
            {
                SCROW nNewMin, nNewMax;
                bJoinRanges = overlaps(
                    aNew.aStart.Row(), aNew.aEnd.Row(), aOld.aStart.Row(), aOld.aEnd.Row(),
                    nNewMin, nNewMax);

                if (bJoinRanges)
                {
                    aNew.aStart.SetRow(nNewMin);
                    aNew.aEnd.SetRow(nNewMax);
                    aNewData.SetRange(aNew, rPos);
                }
            }

            if (bJoinRanges)
            {
                if (bExternal)
                    pOldToken.reset(new ScExternalDoubleRefToken(nFileId, aTabName, aNewData));
                else
                    pOldToken.reset(new ScDoubleRefToken(aNewData));

                bJoined = true;
                break;
            }
        }

        if (bJoined)
        {
            if (rTokens.size() == 1)
                // There is only one left.  No need to do more joining.
                return;

            // Pop the last token from the list, and keep joining recursively.
            ScTokenRef p = rTokens.back();
            rTokens.pop_back();
            join(rTokens, p, rPos);
        }
        else
            rTokens.push_back(pToken);
    }
};

}

void ScRefTokenHelper::join(vector<ScTokenRef>& rTokens, const ScTokenRef& pToken, const ScAddress& rPos)
{
    JoinRefTokenRanges join;
    join(rTokens, pToken, rPos);
}

bool ScRefTokenHelper::getDoubleRefDataFromToken(ScComplexRefData& rData, const ScTokenRef& pToken)
{
    switch (pToken->GetType())
    {
        case svSingleRef:
        case svExternalSingleRef:
        {
            const ScSingleRefData& r = *pToken->GetSingleRef();
            rData.Ref1 = r;
            rData.Ref1.SetFlag3D(true);
            rData.Ref2 = r;
            rData.Ref2.SetFlag3D(false); // Don't display sheet name on second reference.
        }
        break;
        case svDoubleRef:
        case svExternalDoubleRef:
            rData = *pToken->GetDoubleRef();
        break;
        default:
            // Not a reference token.  Bail out.
            return false;
    }
    return true;
}

ScTokenRef ScRefTokenHelper::createRefToken(const ScAddress& rAddr)
{
    ScSingleRefData aRefData;
    aRefData.InitAddress(rAddr);
    ScTokenRef pRef(new ScSingleRefToken(aRefData));
    return pRef;
}

ScTokenRef ScRefTokenHelper::createRefToken(const ScRange& rRange)
{
    ScComplexRefData aRefData;
    aRefData.InitRange(rRange);
    ScTokenRef pRef(new ScDoubleRefToken(aRefData));
    return pRef;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
