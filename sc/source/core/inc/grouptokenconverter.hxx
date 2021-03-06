/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <types.hxx>
#include <formulalogger.hxx>

class ScDocument;
class ScFormulaCell;
class ScTokenArray;
namespace sc { struct FormulaGroupContext; }

class ScGroupTokenConverter
{
    ScTokenArray& mrGroupTokens;
    ScDocument& mrDoc;
    std::shared_ptr<sc::FormulaGroupContext> mxFormulaGroupContext;
    const ScFormulaCell& mrCell;
    const ScAddress& mrPos;

    bool isSelfReferenceRelative(const ScAddress& rRefPos, SCROW nRelRow);
    bool isSelfReferenceAbsolute(const ScAddress& rRefPos);
    SCROW trimLength(SCTAB nTab, SCCOL nCol1, SCCOL nCol2, SCROW nRow, SCROW nRowLen);

public:
    ScGroupTokenConverter(ScTokenArray& rGroupTokens, ScDocument& rDoc, const ScFormulaCell& rCell, const ScAddress& rPos);

    bool convert( const ScTokenArray& rCode, sc::FormulaLogger::GroupScope& rScope );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
