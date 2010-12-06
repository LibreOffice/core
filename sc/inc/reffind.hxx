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
    static const sal_Unicode pDelimiters[];

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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
