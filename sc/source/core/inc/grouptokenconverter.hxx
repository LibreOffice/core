/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_CORE_INC_GROUPTOKENCONVERTER_HXX
#define INCLUDED_SC_SOURCE_CORE_INC_GROUPTOKENCONVERTER_HXX

#include "document.hxx"
#include "formulacell.hxx"
#include "scdllapi.h"
#include "tokenarray.hxx"
#include "types.hxx"

class SC_DLLPUBLIC ScGroupTokenConverter
{
    ScTokenArray& mrGroupTokens;
    ScDocument& mrDoc;
    std::shared_ptr<sc::FormulaGroupContext> mxFormulaGroupContext;
    ScFormulaCell& mrCell;
    const ScAddress& mrPos;

    bool isSelfReferenceRelative(const ScAddress& rRefPos, SCROW nRelRow);
    bool isSelfReferenceAbsolute(const ScAddress& rRefPos);
    SCROW trimLength(SCTAB nTab, SCCOL nCol1, SCCOL nCol2, SCROW nRow, SCROW nRowLen);

public:
    ScGroupTokenConverter(ScTokenArray& rGroupTokens, ScDocument& rDoc, ScFormulaCell& rCell, const ScAddress& rPos);

    bool convert(ScTokenArray& rCode);
};

#endif // INCLUDED_SC_SOURCE_CORE_INC_GROUPTOKENCONVERTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
