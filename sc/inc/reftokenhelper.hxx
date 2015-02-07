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

#ifndef INCLUDED_SC_INC_REFTOKENHELPER_HXX
#define INCLUDED_SC_INC_REFTOKENHELPER_HXX

#include "token.hxx"

#include <vector>

class ScDocument;
class ScRange;
class ScRangeList;

namespace ScRefTokenHelper
{
    /**
     * Compile an array of reference tokens from a data source range string.
     * The source range may consist of multiple ranges separated by ';'s.
     */
    void compileRangeRepresentation(
        ::std::vector<ScTokenRef>& rRefTokens, const OUString& rRangeStr, ScDocument* pDoc,
        const sal_Unicode cSep, ::formula::FormulaGrammar::Grammar eGrammar, bool bOnly3DRef = false);

    bool getRangeFromToken(ScRange& rRange, const ScTokenRef& pToken, const ScAddress& rPos, bool bExternal = false);

    void getRangeListFromTokens(ScRangeList& rRangeList, const ::std::vector<ScTokenRef>& pTokens, const ScAddress& rPos);

    /**
     * Create a double reference token from a range object.
     */
    void getTokenFromRange(ScTokenRef& pToken, const ScRange& rRange);

    void getTokensFromRangeList(::std::vector<ScTokenRef>& pTokens, const ScRangeList& rRanges);

    bool SC_DLLPUBLIC isRef(const ScTokenRef& pToken);
    bool SC_DLLPUBLIC isExternalRef(const ScTokenRef& pToken);

    bool SC_DLLPUBLIC intersects(
        const ::std::vector<ScTokenRef>& rTokens, const ScTokenRef& pToken, const ScAddress& rPos);

    void SC_DLLPUBLIC join(::std::vector<ScTokenRef>& rTokens, const ScTokenRef& pToken, const ScAddress& rPos);

    bool getDoubleRefDataFromToken(ScComplexRefData& rData, const ScTokenRef& pToken);

    ScTokenRef createRefToken(const ScAddress& rAddr);
    ScTokenRef createRefToken(const ScRange& rRange);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
