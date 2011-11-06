/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SC_REFTOKENHELPER_HXX
#define SC_REFTOKENHELPER_HXX

#include "token.hxx"

#include <vector>

namespace rtl {
    class OUString;
}

class ScDocument;
class ScRange;
class ScRangeList;

class ScRefTokenHelper
{
private:
    ScRefTokenHelper();
    ScRefTokenHelper(const ScRefTokenHelper&);
    ~ScRefTokenHelper();

public:
    /**
     * Compile an array of reference tokens from a data source range string.
     * The source range may consist of multiple ranges separated by ';'s.
     */
    static void compileRangeRepresentation(
        ::std::vector<ScSharedTokenRef>& rRefTokens, const ::rtl::OUString& rRangeStr, ScDocument* pDoc,
        ::formula::FormulaGrammar::Grammar eGrammar = ::formula::FormulaGrammar::GRAM_ENGLISH);

    static bool getRangeFromToken(ScRange& rRange, const ScSharedTokenRef& pToken, bool bExternal = false);

    static void getRangeListFromTokens(ScRangeList& rRangeList, const ::std::vector<ScSharedTokenRef>& pTokens);

    /**
     * Create a double reference token from a range object.
     */
    static void getTokenFromRange(ScSharedTokenRef& pToken, const ScRange& rRange);

    static void getTokensFromRangeList(::std::vector<ScSharedTokenRef>& pTokens, const ScRangeList& rRanges);

    static bool SC_DLLPUBLIC isRef(const ScSharedTokenRef& pToken);
    static bool SC_DLLPUBLIC isExternalRef(const ScSharedTokenRef& pToken);

    static bool SC_DLLPUBLIC intersects(const ::std::vector<ScSharedTokenRef>& rTokens, const ScSharedTokenRef& pToken);

    static void SC_DLLPUBLIC join(::std::vector<ScSharedTokenRef>& rTokens, const ScSharedTokenRef& pToken);

    static bool getDoubleRefDataFromToken(ScComplexRefData& rData, const ScSharedTokenRef& pToken);
};

#endif
