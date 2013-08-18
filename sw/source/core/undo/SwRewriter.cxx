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


#include <algorithm>
#include <SwRewriter.hxx>

using namespace std;

bool operator == (const SwRewriteRule & a, const SwRewriteRule & b)
{
    return a.first == b.first;
}

SwRewriter::SwRewriter()
{
}

SwRewriter::SwRewriter(const SwRewriter & rSrc)
    : mRules(rSrc.mRules)
{
}

SwRewriter::~SwRewriter()
{
}

void SwRewriter::AddRule(SwUndoArg eWhat, const OUString & rWith)
{
    SwRewriteRule aRule(eWhat, rWith);

    vector<SwRewriteRule>::iterator aIt;

    aIt = find(mRules.begin(), mRules.end(), aRule);

    if (aIt != mRules.end())
        *aIt = aRule;
    else
        mRules.push_back(aRule);
}

namespace
{

const char UNDO_ARG1[] = "$1";
const char UNDO_ARG2[] = "$2";
const char UNDO_ARG3[] = "$3";

}

OUString SwRewriter::Apply(const OUString & rStr) const
{
    OUString aResult = rStr;
    std::vector<SwRewriteRule>::const_iterator aIt;

    for (aIt = mRules.begin(); aIt != mRules.end(); ++aIt)
    {
        switch (aIt->first)
        {
            case UndoArg1:
            default:
                aResult = aResult.replaceAll(UNDO_ARG1, aIt->second);
                break;
            case UndoArg2:
                aResult = aResult.replaceAll(UNDO_ARG2, aIt->second);
                break;
            case UndoArg3:
                aResult = aResult.replaceAll(UNDO_ARG3, aIt->second);
                break;
        }
    }

    return aResult;
}

OUString SwRewriter::GetPlaceHolder(SwUndoArg eId)
{
    switch (eId)
    {
        case UndoArg1:
            return OUString(UNDO_ARG1);
        case UndoArg2:
            return OUString(UNDO_ARG2);
        case UndoArg3:
            return OUString(UNDO_ARG3);
        default:
            break;
    }
    return OUString(UNDO_ARG1);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
