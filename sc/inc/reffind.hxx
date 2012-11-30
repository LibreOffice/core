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
    ScAddress   maPos;
    xub_StrLen  nFound;
    xub_StrLen  nSelStart;
    xub_StrLen  nSelEnd;

public:
    ScRefFinder( const String& rFormula, const ScAddress& rPos,
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
