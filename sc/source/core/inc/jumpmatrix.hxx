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

#ifndef SC_JUMPMATRIX_HXX
#define SC_JUMPMATRIX_HXX

#include "formula/token.hxx"
#include "formula/errorcodes.hxx"
#include <tools/solar.h>
#include <vector>
#include "scmatrix.hxx"

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

                                // not implemented, prevent usage
                                ScJumpMatrix( const ScJumpMatrix& );
            ScJumpMatrix&       operator=( const ScJumpMatrix& );

public:
                                ScJumpMatrix( SCSIZE nColsP, SCSIZE nRowsP )
                                        : pJump( new ScJumpMatrixEntry[ nColsP * nRowsP ] )
                                        , pMat( new ScMatrix( nColsP, nRowsP) )
                                        , pParams( NULL )
                                        , nCols( nColsP )
                                        , nRows( nRowsP )
                                        , nCurCol( 0 )
                                        , nCurRow( 0 )
                                        , nResMatCols( nColsP )
                                        , nResMatRows( nRowsP )
                                        , bStarted( false )
                                    {
                                        // Initialize result matrix in case of
                                        // a premature end of the interpreter
                                        // due to errors.
                                        pMat->FillDouble( CreateDoubleError(
                                                NOTAVAILABLE), 0, 0, nCols-1,
                                                nRows-1);
                                        /*! pJump not initialized */
                                    }
                                ~ScJumpMatrix()
                                    {
                                        if ( pParams )
                                        {
                                            for ( ScTokenVec::iterator i =
                                                    pParams->begin(); i !=
                                                    pParams->end(); ++i )
                                            {
                                                (*i)->DecRef();
                                            }
                                            delete pParams;
                                        }
                                        delete [] pJump;
                                    }
            void                GetDimensions( SCSIZE& rCols, SCSIZE& rRows ) const
                                    {
                                        rCols = nCols;
                                        rRows = nRows;
                                    }
            void                SetJump( SCSIZE nCol, SCSIZE nRow, double fBool,
                                            short nStart, short nNext,
                                            short nStop = SHRT_MAX )
                                    {
                                        pJump[ (sal_uLong)nCol * nRows + nRow ].
                                            SetJump( fBool, nStart, nNext, nStop);
                                    }
            void                GetJump( SCSIZE nCol, SCSIZE nRow, double& rBool,
                                            short& rStart, short& rNext,
                                            short& rStop ) const
                                    {
                                        if (nCols == 1 && nRows == 1)
                                        {
                                            nCol = 0;
                                            nRow = 0;
                                        }
                                        else if (nCols == 1 && nRow < nRows)
                                            nCol = 0;
                                        else if (nRows == 1 && nCol < nCols)
                                            nRow = 0;
                                        else if (nCols <= nCol || nRows <= nRow)
                                        {
                                            DBG_ERROR("ScJumpMatrix::GetJump: dimension error");
                                            nCol = 0;
                                            nRow = 0;
                                        }
                                        pJump[ (sal_uLong)nCol * nRows + nRow ].
                                            GetJump( rBool, rStart, rNext, rStop);
                                    }
            void                SetAllJumps( double fBool,
                                            short nStart, short nNext,
                                            short nStop = SHRT_MAX )
                                    {
                                        sal_uLong n = (sal_uLong)nCols * nRows;
                                        for ( sal_uLong j=0; j<n; ++j )
                                        {
                                            pJump[ j ].SetJump( fBool, nStart,
                                                    nNext, nStop);
                                        }
                                    }
            void                SetJumpParameters( ScTokenVec* p )
                                    { pParams = p; }
            const ScTokenVec*   GetJumpParameters() const { return pParams; }
            ScMatrix*           GetResultMatrix() const { return pMat; }
            void                GetPos( SCSIZE& rCol, SCSIZE& rRow ) const
                                    {
                                        rCol = nCurCol;
                                        rRow = nCurRow;
                                    }
            bool                Next( SCSIZE& rCol, SCSIZE& rRow )
                                    {
                                        if ( !bStarted )
                                        {
                                            bStarted = true;
                                            nCurCol = nCurRow = 0;
                                        }
                                        else
                                        {
                                            if ( ++nCurRow >= nResMatRows )
                                            {
                                                nCurRow = 0;
                                                ++nCurCol;
                                            }
                                        }
                                        GetPos( rCol, rRow );
                                        return nCurCol < nResMatCols;
                                    }
            void                GetResMatDimensions( SCSIZE& rCols, SCSIZE& rRows )
                                    {
                                        rCols = nResMatCols;
                                        rRows = nResMatRows;
                                    }
            void                SetNewResMat( SCSIZE nNewCols, SCSIZE nNewRows )
                                    {
                                        if ( nNewCols > nResMatCols || nNewRows > nResMatRows )
                                        {
                                            pMat = pMat->CloneAndExtend( nNewCols, nNewRows );
                                            if ( nResMatCols < nNewCols )
                                            {
                                                pMat->FillDouble( CreateDoubleError(
                                                    NOTAVAILABLE), nResMatCols, 0, nNewCols-1,
                                                    nResMatRows-1);
                                            }
                                            if ( nResMatRows < nNewRows )
                                            {
                                                pMat->FillDouble( CreateDoubleError(
                                                    NOTAVAILABLE), 0, nResMatRows, nNewCols-1,
                                                    nNewRows-1);
                                            }
                                            if ( nRows == 1 && nCurCol != 0 )
                                            {
                                                nCurCol = 0;
                                                nCurRow = nResMatRows - 1;
                                            }
                                            nResMatCols = nNewCols;
                                            nResMatRows = nNewRows;
                                        }
                                    }
};

#endif // SC_JUMPMATRIX_HXX

