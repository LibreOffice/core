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



#ifndef SC_REFFIND_HXX
#define SC_REFFIND_HXX

#include <tools/string.hxx>
#include "address.hxx"

class ScDocument;

// -----------------------------------------------------------------------

class ScRefFinder
{
private:
    String      aFormula;
    formula::FormulaGrammar::AddressConvention eConv;
    ScDocument* pDoc;
    xub_StrLen  nFound;
    xub_StrLen  nSelStart;
    xub_StrLen  nSelEnd;

public:
    static const sal_Unicode __FAR_DATA pDelimiters[];

    ScRefFinder( const String& rFormula,
             ScDocument* pDocument = NULL,
             formula::FormulaGrammar::AddressConvention eConvP = formula::FormulaGrammar::CONV_OOO );
    ~ScRefFinder();

    const String&   GetText() const     { return aFormula; }
    xub_StrLen      GetFound() const    { return nFound; }
    xub_StrLen      GetSelStart() const { return nSelStart; }
    xub_StrLen      GetSelEnd() const   { return nSelEnd; }

    void            ToggleRel( xub_StrLen nStartPos, xub_StrLen nEndPos );
};



#endif


