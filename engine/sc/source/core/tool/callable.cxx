/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <config_features.h>

#include <sal/config.h>

#include <forward_list>
#include <queue>
#include <vector>

#include <basic/sbmeth.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbxvar.hxx>
#include <formula/FormulaCompiler.hxx>
#include <formula/opcode.hxx>
#include <formula/token.hxx>
#include <formula/tokenarray.hxx>
#include <osl/diagnose.h>
#include <sfx2/app.hxx>
#include <docsh.hxx>
#include <interpre.hxx>
#include <tokenarray.hxx>

#include <callable.hxx>

ScMacroFunction::ScMacroFunction(const ScInterpreter& rInterpreter, const OUString& aMacro)
    : formula::FormulaCallable()
    , mnError(FormulaError::NONE)
    , mpDocShell(rInterpreter.mrDoc.GetDocumentShell())
    , mpMethod(nullptr)
    , mpModule(nullptr)
    , mbInvalid(false)
{
#if !HAVE_FEATURE_SCRIPTING
    (void)rInterpreter;
    (void)aMacro;
    mbInvalid = true; // without DocShell no CallBasic
#else
    SbxBase::ResetError();

    if (!mpDocShell)
    {
        mbInvalid = true; // without DocShell no CallBasic
        return;
    }

    //  no security queue beforehand (just CheckMacroWarn), moved to  CallBasic

    //  If the  Dok was loaded during a Basic-Call,
    //  is the  Sbx-object created(?)
    //  pDocSh->GetSbxObject();

    //  search function with the name,
    //  then assemble  SfxObjectShell::CallBasic from aBasicStr, aMacroStr

    StarBASIC* pRoot;

    try
    {
        pRoot = mpDocShell->GetBasic();
    }
    catch (...)
    {
        pRoot = nullptr;
    }

    SbxVariable* pVar = pRoot ? pRoot->Find(aMacro, SbxClassType::Method) : nullptr;
    if (!pVar || pVar->GetType() == SbxVOID)
    {
        mbInvalid = true;
        mnError = FormulaError::NoMacro;
        return;
    }
    mpMethod = dynamic_cast<SbMethod*>(pVar);
    if (!mpMethod)
    {
        mbInvalid = true;
        mnError = FormulaError::NoMacro;
        return;
    }
    mpModule = mpMethod->GetModule();
    if (!mpModule)
    {
        mbInvalid = true;
        mnError = FormulaError::NoMacro;
        return;
    }

    bool bUseVBAObjects = mpModule->IsVBASupport();
    SbxObject* pObject = mpModule->GetParent();
    assert(pObject);
    OSL_ENSURE(dynamic_cast<const StarBASIC*>(pObject) != nullptr, "No Basic found!");
    maMacroStr = pObject->GetName() + "." + mpModule->GetName() + "." + mpMethod->GetName();
    if (pRoot && bUseVBAObjects)
    {
        // just here to make sure the VBA objects when we run the macro during ODF import
        pRoot->getVBAGlobals();
    }
    if (pObject->GetParent())
        maBasicStr = pObject->GetParent()->GetName(); // document BASIC
    else
        maBasicStr = SfxGetpApp()->GetName(); // application BASIC
#endif
}

/// clone a contiguous subset of the (RPN) tokens into a new array
static void lcl_CloneSubarray(const ScTokenArray& rTokensIn, short nStartPos, short nEndPos,
                              ScTokenArray& rTokensOut)
{
    const short nOffset = nStartPos + 1;
    const short nLen = nEndPos - nOffset;
    formula::FormulaToken** pResult = new formula::FormulaToken*[nLen];
    formula::FormulaToken** p = pResult;
    formula::FormulaTokenArrayPlainIterator aIter(rTokensIn);
    aIter.Jump(nOffset);
    for (const formula::FormulaToken* pToken = aIter.NextRPN();
         aIter.GetIndex() <= nEndPos && pToken; pToken = aIter.NextRPN())
    {
        formula::FormulaToken* pNewToken = pToken->Clone();
        pNewToken->IncRef();
        *(p++) = pNewToken;
        auto pJumpToken = dynamic_cast<formula::FormulaJumpToken*>(pNewToken);
        if (pJumpToken)
        {
            // adjust jumps to match new locations
            short* pJump = pJumpToken->GetJump();
            short nJumpCount = pJump[0];
            for (short nJump = 1; nJump <= nJumpCount; ++nJump)
                pJump[nJump] -= nOffset;
        }
    }
    rTokensOut.CreateNewRPNArrayFromData(pResult, nLen);
    delete[] pResult;
}

/// find all the locations where the specified variable would have to be substituted
static std::forward_list<short> lcl_FindReplacementPositions(std::u16string_view aStrName,
                                                             const ScTokenArray& rTokens,
                                                             short nEntryPoint)
{
    std::forward_list<short> aPositions;
    formula::FormulaTokenIterator aIter(rTokens);
    std::queue<short> aStartPoints;
    aStartPoints.push(nEntryPoint);

    while (!aStartPoints.empty())
    {
        {
            short nStartPoint = aStartPoints.front();
            aStartPoints.pop();
            aIter.Jump(nStartPoint, SHRT_MAX, SHRT_MAX);
        }
        while (!aIter.IsEndOfPath())
        {
            const formula::FormulaToken* pToken = aIter.Next();
            auto pStringNameToken = dynamic_cast<const formula::FormulaStringNameToken*>(pToken);
            if (pStringNameToken)
            {
                short nNextOp = aIter.GetPC() + 1;
                if (!(nNextOp < rTokens.GetCodeLen()
                      && (rTokens.GetCode()[nNextOp]->GetOpCode() == ocLet
                          || rTokens.GetCode()[nNextOp]->GetOpCode() == ocLambda)))
                {
                    if (pStringNameToken->GetString().getString().equalsIgnoreAsciiCase(aStrName))
                        aPositions.push_front(aIter.GetPC());
                }
            }
            else
            {
                auto pJumpToken = dynamic_cast<const formula::FormulaJumpToken*>(pToken);
                if (pJumpToken)
                {
                    const short* pJump = pJumpToken->GetJump();
                    short nJumpCount = pJump[0];
                    switch (pJumpToken->GetOpCode())
                    {
                        case ocLet:
                        {
                            aStartPoints.push(pJump[1]);
                            auto pSubToken = dynamic_cast<const formula::FormulaStringNameToken*>(
                                rTokens.GetCode()[aIter.GetPC() - 1]);
                            if (!(pSubToken
                                  && pSubToken->GetString().getString().equalsIgnoreAsciiCase(
                                         aStrName)))
                            {
                                bool bFound = false;
                                for (short nJump = 2; nJump < nJumpCount - 1; nJump += 2)
                                {
                                    aStartPoints.push(pJump[nJump + 1]);
                                    pSubToken
                                        = dynamic_cast<const formula::FormulaStringNameToken*>(
                                            rTokens.GetCode()[pJump[nJump]]);
                                    if (pSubToken
                                        && pSubToken->GetString().getString().equalsIgnoreAsciiCase(
                                               aStrName))
                                    {
                                        // this name shadows the one we're looking for, after this point
                                        bFound = true;
                                        break;
                                    }
                                }
                                if (!bFound)
                                    aStartPoints.push(pJump[nJumpCount - 1]);
                            }
                        }
                        break;
                        case ocLambda:
                        {
                            auto pSubToken = dynamic_cast<const formula::FormulaStringNameToken*>(
                                rTokens.GetCode()[aIter.GetPC() - 1]);
                            if (!(pSubToken && pSubToken->GetString().getString() == aStrName))
                            {
                                bool bFound = false;
                                for (short nJump = 1; nJump < nJumpCount - 1; ++nJump)
                                {
                                    pSubToken
                                        = dynamic_cast<const formula::FormulaStringNameToken*>(
                                            rTokens.GetCode()[pJump[nJump]]);
                                    if (pSubToken && pSubToken->GetString().getString() == aStrName)
                                    {
                                        // this name shadows the one we're looking for, in the body
                                        bFound = true;
                                        break;
                                    }
                                }
                                if (!bFound)
                                    aStartPoints.push(pJump[nJumpCount - 1]);
                            }
                        }
                        break;
                        default: // nothing else affects local variables
                        {
                            for (short nJump = 1; nJump < nJumpCount; ++nJump)
                                aStartPoints.push(pJump[nJump]);
                        }
                        break;
                    }
                    aIter.Jump(pJump[nJumpCount], SHRT_MAX, SHRT_MAX);
                } // pJumpToken
            } // !pStringNameToken
        } // !aIter.IsEndOfPath()
    } // !aStartPoints.empty()

    return aPositions;
}

ScFormulaFunction::ScFormulaFunction(const ScInterpreter& rInterpreter,
                                     const std::vector<OUString>& rParams,
                                     const ScTokenArray& rTokens, short nBodyStart, short nBodyEnd)
    : formula::FormulaCallable()
    , mpDoc(&rInterpreter.mrDoc)
    , maPos(rInterpreter.aPos)
    , mpCell(rInterpreter.mrDoc.GetFormulaCell(rInterpreter.aPos))
    , mpContext(&rInterpreter.mrContext)
    , maLambdaBody(rInterpreter.mrDoc)
    , maReplacementPositions(rParams.size())
{
    lcl_CloneSubarray(rTokens, nBodyStart, nBodyEnd, maLambdaBody);

    // figure out right now where the names will need to be replaced, as an optimization
    for (std::vector<OUString>::size_type nParam = 0; nParam < rParams.size(); ++nParam)
        maReplacementPositions[nParam]
            = lcl_FindReplacementPositions(rParams[nParam], maLambdaBody, -1);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
