/*************************************************************************
 *
 *  $RCSfile: jumpmatrix.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $  $Date: 2004-06-04 10:31:30 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_JUMPMATRIX_HXX
#define SC_JUMPMATRIX_HXX

#ifndef SC_TOKEN_HXX
#include "token.hxx"
#endif
#ifndef SC_MATRIX_HXX
#include "scmatrix.hxx"
#endif
#ifndef SC_ERRORCODES_HXX
#include "errorcodes.hxx"
#endif
#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#include <vector>

typedef ::std::vector< ScToken*> ScTokenVec;

struct ScJumpMatrixEntry
{
    double  fBool;      // 0:= false  1:= true   also if no-path
                        // other values may contain error conditions like NAN and INF
    short   nStart;     // start of path (actually start-1, see ScTokenIterator)
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
                                        , bStarted( false )
                                    {
                                        // Initialize result matrix in case of
                                        // a premature end of the interpreter
                                        // due to errors.
                                        pMat->FillDouble( CreateDoubleError(
                                                    NOVALUE), 0, 0, nCols-1,
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
                                        pJump[ (ULONG)nCol * nRows + nRow ].
                                            SetJump( fBool, nStart, nNext, nStop);
                                    }
            void                GetJump( SCSIZE nCol, SCSIZE nRow, double& rBool,
                                            short& rStart, short& rNext,
                                            short& rStop ) const
                                    {
                                        pJump[ (ULONG)nCol * nRows + nRow ].
                                            GetJump( rBool, rStart, rNext, rStop);
                                    }
            void                SetAllJumps( double fBool,
                                            short nStart, short nNext,
                                            short nStop = SHRT_MAX )
                                    {
                                        ULONG n = (ULONG)nCols * nRows;
                                        for ( ULONG j=0; j<n; ++j )
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
                                            if ( ++nCurRow >= nRows )
                                            {
                                                nCurRow = 0;
                                                ++nCurCol;
                                            }
                                        }
                                        GetPos( rCol, rRow );
                                        return nCurCol < nCols;
                                    }
};

#endif // SC_JUMPMATRIX_HXX

