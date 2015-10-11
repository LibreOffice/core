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

#ifndef INCLUDED_SC_SOURCE_CORE_INC_JUMPMATRIX_HXX
#define INCLUDED_SC_SOURCE_CORE_INC_JUMPMATRIX_HXX

#include <formula/token.hxx>
#include <formula/errorcodes.hxx>
#include <limits.h>
#include <vector>
#include "types.hxx"
#include "address.hxx"

typedef ::std::vector< formula::FormulaToken*> ScTokenVec;

struct ScJumpMatrixEntry
{
    double  fBool;      // 0:= false  1:= true   also if no-path
                        // other values may contain error conditions like NAN and INF
    short   nStart;     // start of path (actually start-1, see formula::FormulaTokenIterator)
    short   nNext;      // next after path
                        // jump path exists if nStart != nNext, else no path
    short   nStop;      // optional stop of path (nPC < nStop)

    void    SetJump( double fBoolP, short nStartP, short nNextP, short nStopP )
                {
                    fBool = fBoolP;
                    nStart = nStartP;
                    nNext = nNextP;
                    nStop = nStopP;
                }
    void    GetJump( double& rBool, short& rStart, short& rNext, short& rStop )
                {
                    rBool = fBool;
                    rStart = nStart;
                    rNext = nNext;
                    rStop = nStop;
                }
};

class ScJumpMatrix
{
    ScJumpMatrixEntry*  pJump;      // the jumps
    ScMatrixRef         pMat;       // the results
    ScTokenVec*         pParams;    // parameter stack
    SCSIZE              nCols;
    SCSIZE              nRows;
    SCSIZE              nCurCol;
    SCSIZE              nCurRow;
    SCSIZE              nResMatCols;
    SCSIZE              nResMatRows;
    bool                bStarted;

    // Buffer result ranges to be able to set a range of identically typed
    // values at the result matrix in order to avoid multiple shrinks and
    // growths of multi_type_vector segments, which is a major performance
    // bottleneck, see fdo#72929
    ::std::vector< svl::SharedString >  mvBufferStrings;
    ::std::vector< double >             mvBufferDoubles;
    SCSIZE                              mnBufferCol;
    SCSIZE                              mnBufferRowStart;
    SCSIZE                              mnBufferEmptyCount;
    SCSIZE                              mnBufferEmptyPathCount;

    enum BufferType
    {
        BUFFER_NONE,
        BUFFER_DOUBLE,
        BUFFER_STRING,
        BUFFER_EMPTY,
        BUFFER_EMPTYPATH
    };

    /** Flush different types or non-consecutive buffers. */
    void FlushBufferOtherThan( BufferType eType, SCSIZE nC, SCSIZE nR );

    ScJumpMatrix( const ScJumpMatrix& ) = delete;
    ScJumpMatrix& operator=( const ScJumpMatrix& ) = delete;

public:
    ScJumpMatrix( SCSIZE nColsP, SCSIZE nRowsP );
    ~ScJumpMatrix();
    void GetDimensions( SCSIZE& rCols, SCSIZE& rRows ) const;
    void SetJump( SCSIZE nCol, SCSIZE nRow, double fBool, short nStart, short nNext, short nStop = SHRT_MAX );
    void GetJump( SCSIZE nCol, SCSIZE nRow, double& rBool, short& rStart, short& rNext, short& rStop ) const;
    void SetAllJumps( double fBool, short nStart, short nNext, short nStop = SHRT_MAX );
    void SetJumpParameters( ScTokenVec* p );
    const ScTokenVec* GetJumpParameters() const { return pParams;}
    bool HasResultMatrix() const;
    ScMatrix* GetResultMatrix();        ///< also applies pending buffered values
    void GetPos( SCSIZE& rCol, SCSIZE& rRow ) const;
    bool Next( SCSIZE& rCol, SCSIZE& rRow );
    void GetResMatDimensions( SCSIZE& rCols, SCSIZE& rRows );
    void SetNewResMat( SCSIZE nNewCols, SCSIZE nNewRows );

    void PutResultDouble( double fVal, SCSIZE nC, SCSIZE nR );
    void PutResultString( const svl::SharedString& rStr, SCSIZE nC, SCSIZE nR );
    void PutResultEmpty( SCSIZE nC, SCSIZE nR );
    void PutResultEmptyPath( SCSIZE nC, SCSIZE nR );
};

#endif // INCLUDED_SC_SOURCE_CORE_INC_JUMPMATRIX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
