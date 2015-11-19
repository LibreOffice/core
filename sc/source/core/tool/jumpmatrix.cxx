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

#include "jumpmatrix.hxx"
#include "scmatrix.hxx"

#include <osl/diagnose.h>

namespace {
// Don't bother with buffer overhead for less than y rows.
const SCSIZE kBufferThreshhold = 128;
}

ScJumpMatrix::ScJumpMatrix(SCSIZE nColsP, SCSIZE nRowsP)
    : pJump(new ScJumpMatrixEntry[nColsP * nRowsP])
    , pMat(new ScFullMatrix(nColsP, nRowsP))
    , pParams(nullptr)
    , nCols(nColsP)
    , nRows(nRowsP)
    , nCurCol(0)
    , nCurRow(0)
    , nResMatCols(nColsP)
    , nResMatRows(nRowsP)
    , bStarted(false)
    , mnBufferCol(0)
    , mnBufferRowStart(0)
    , mnBufferEmptyCount(0)
    , mnBufferEmptyPathCount(0)
{
    // Initialize result matrix in case of
    // a premature end of the interpreter
    // due to errors.
    pMat->FillDouble(CreateDoubleError(NOTAVAILABLE), 0, 0, nCols - 1, nRows - 1);
    /*! pJump not initialized */
}

ScJumpMatrix::~ScJumpMatrix()
{
    if (pParams)
    {
        for (ScTokenVec::iterator i =
             pParams->begin(); i !=
                 pParams->end(); ++i)
        {
            (*i)->DecRef();
        }
        delete pParams;
    }
    delete[] pJump;
}

void ScJumpMatrix::GetDimensions(SCSIZE& rCols, SCSIZE& rRows) const
{
    rCols = nCols;
    rRows = nRows;
}

void ScJumpMatrix::SetJump(SCSIZE nCol, SCSIZE nRow, double fBool,
                           short nStart, short nNext, short nStop)
{
    pJump[(sal_uLong)nCol * nRows + nRow].SetJump(fBool, nStart, nNext, nStop);
}

void ScJumpMatrix::GetJump(
    SCSIZE nCol, SCSIZE nRow, double& rBool, short& rStart, short& rNext, short& rStop) const
{
    if (nCols == 1 && nRows == 1)
    {
        nCol = 0;
        nRow = 0;
    }
    else if (nCols == 1 && nRow < nRows) nCol = 0;
    else if (nRows == 1 && nCol < nCols) nRow = 0;
    else if (nCols <= nCol || nRows <= nRow)
    {
        OSL_FAIL("ScJumpMatrix::GetJump: dimension error");
        nCol = 0;
        nRow = 0;
    }
    pJump[(sal_uLong)nCol * nRows + nRow].
        GetJump(rBool, rStart, rNext, rStop);
}

void ScJumpMatrix::SetAllJumps(double fBool, short nStart, short nNext, short nStop)
{
    sal_uLong n = (sal_uLong)nCols * nRows;
    for (sal_uLong j = 0; j < n; ++j)
    {
        pJump[j].SetJump(fBool, nStart,
                         nNext, nStop);
    }
}

void ScJumpMatrix::SetJumpParameters(ScTokenVec* p)
{
    pParams = p;
}

void ScJumpMatrix::GetPos(SCSIZE& rCol, SCSIZE& rRow) const
{
    rCol = nCurCol;
    rRow = nCurRow;
}

bool ScJumpMatrix::Next(SCSIZE& rCol, SCSIZE& rRow)
{
    if (!bStarted)
    {
        bStarted = true;
        nCurCol = nCurRow = 0;
    }
    else
    {
        if (++nCurRow >= nResMatRows)
        {
            nCurRow = 0;
            ++nCurCol;
        }
    }
    GetPos(rCol, rRow);
    return nCurCol < nResMatCols;
}

void ScJumpMatrix::GetResMatDimensions(SCSIZE& rCols, SCSIZE& rRows)
{
    rCols = nResMatCols;
    rRows = nResMatRows;
}

void ScJumpMatrix::SetNewResMat(SCSIZE nNewCols, SCSIZE nNewRows)
{
    if (nNewCols > nResMatCols || nNewRows > nResMatRows)
    {
        FlushBufferOtherThan( BUFFER_NONE, 0, 0);
        pMat = pMat->CloneAndExtend(nNewCols, nNewRows);
        if (nResMatCols < nNewCols)
        {
            pMat->FillDouble(CreateDoubleError(
                                 NOTAVAILABLE), nResMatCols, 0, nNewCols - 1,
                             nResMatRows - 1);
        }
        if (nResMatRows < nNewRows)
        {
            pMat->FillDouble(CreateDoubleError(
                                 NOTAVAILABLE), 0, nResMatRows, nNewCols - 1,
                             nNewRows - 1);
        }
        if (nRows == 1 && nCurCol != 0)
        {
            nCurCol = 0;
            nCurRow = nResMatRows - 1;
        }
        nResMatCols = nNewCols;
        nResMatRows = nNewRows;
    }
}

bool ScJumpMatrix::HasResultMatrix() const
{
    // We now always have a matrix but caller logic may still want to check it.
    return pMat.get() != nullptr;
}

void ScJumpMatrix::FlushBufferOtherThan( ScJumpMatrix::BufferType eType, SCSIZE nC, SCSIZE nR )
{
    if (!mvBufferDoubles.empty() &&
            (eType != BUFFER_DOUBLE || nC != mnBufferCol || nR != mnBufferRowStart + mvBufferDoubles.size()))
    {
        pMat->PutDoubleVector( mvBufferDoubles, mnBufferCol, mnBufferRowStart);
        mvBufferDoubles.clear();
    }
    if (!mvBufferStrings.empty() &&
            (eType != BUFFER_STRING || nC != mnBufferCol || nR != mnBufferRowStart + mvBufferStrings.size()))
    {
        pMat->PutStringVector( mvBufferStrings, mnBufferCol, mnBufferRowStart);
        mvBufferStrings.clear();
    }
    if (mnBufferEmptyCount &&
            (eType != BUFFER_EMPTY || nC != mnBufferCol || nR != mnBufferRowStart + mnBufferEmptyCount))
    {
        pMat->PutEmptyVector( mnBufferEmptyCount, mnBufferCol, mnBufferRowStart);
        mnBufferEmptyCount = 0;
    }
    if (mnBufferEmptyPathCount &&
            (eType != BUFFER_EMPTYPATH || nC != mnBufferCol || nR != mnBufferRowStart + mnBufferEmptyPathCount))
    {
        pMat->PutEmptyPathVector( mnBufferEmptyPathCount, mnBufferCol, mnBufferRowStart);
        mnBufferEmptyPathCount = 0;
    }
}

ScMatrix* ScJumpMatrix::GetResultMatrix()
{
    if (nResMatRows >= kBufferThreshhold)
        FlushBufferOtherThan( BUFFER_NONE, 0, 0);
    return pMat.get();
}

void ScJumpMatrix::PutResultDouble( double fVal, SCSIZE nC, SCSIZE nR )
{
    if (nResMatRows < kBufferThreshhold)
        pMat->PutDouble( fVal, nC, nR);
    else
    {
        FlushBufferOtherThan( BUFFER_DOUBLE, nC, nR);
        if (mvBufferDoubles.empty())
        {
            mnBufferCol = nC;
            mnBufferRowStart = nR;
        }
        mvBufferDoubles.push_back( fVal);
    }
}

void ScJumpMatrix::PutResultString( const svl::SharedString& rStr, SCSIZE nC, SCSIZE nR )
{
    if (nResMatRows < kBufferThreshhold)
        pMat->PutString( rStr, nC, nR);
    else
    {
        FlushBufferOtherThan( BUFFER_STRING, nC, nR);
        if (mvBufferStrings.empty())
        {
            mnBufferCol = nC;
            mnBufferRowStart = nR;
        }
        mvBufferStrings.push_back( rStr);
    }
}

void ScJumpMatrix::PutResultEmpty( SCSIZE nC, SCSIZE nR )
{
    if (nResMatRows < kBufferThreshhold)
        pMat->PutEmpty( nC, nR);
    else
    {
        FlushBufferOtherThan( BUFFER_EMPTY, nC, nR);
        if (!mnBufferEmptyCount)
        {
            mnBufferCol = nC;
            mnBufferRowStart = nR;
        }
        ++mnBufferEmptyCount;
    }
}

void ScJumpMatrix::PutResultEmptyPath( SCSIZE nC, SCSIZE nR )
{
    if (nResMatRows < kBufferThreshhold)
        pMat->PutEmptyPath( nC, nR);
    else
    {
        FlushBufferOtherThan( BUFFER_EMPTYPATH, nC, nR);
        if (!mnBufferEmptyPathCount)
        {
            mnBufferCol = nC;
            mnBufferRowStart = nR;
        }
        ++mnBufferEmptyPathCount;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
