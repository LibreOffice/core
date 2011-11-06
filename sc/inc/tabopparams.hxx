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



#ifndef SC_TABOPPARAMS_HXX
#define SC_TABOPPARAMS_HXX

#include "global.hxx"
#include "address.hxx"

#include <vector>

class ScFormulaCell;

struct ScInterpreterTableOpParams
{
    ScAddress   aOld1;
    ScAddress   aNew1;
    ScAddress   aOld2;
    ScAddress   aNew2;
    ScAddress   aFormulaPos;
    ::std::vector< ScFormulaCell* > aNotifiedFormulaCells;
    ::std::vector< ScAddress >      aNotifiedFormulaPos;
    sal_Bool        bValid;
    sal_Bool        bRefresh;
    sal_Bool        bCollectNotifications;

    ScInterpreterTableOpParams()
            : bValid( sal_False )
            , bRefresh( sal_False )
            , bCollectNotifications( sal_True )
    {
    }

    ScInterpreterTableOpParams( const ScInterpreterTableOpParams& r )
            : aOld1(                 r.aOld1 )
            , aNew1(                 r.aNew1 )
            , aOld2(                 r.aOld2 )
            , aNew2(                 r.aNew2 )
            , aFormulaPos(           r.aFormulaPos )
            //! never copied!   , aNotifiedFormulaCells( r.aNotifiedFormulaCells )
            , aNotifiedFormulaPos(   r.aNotifiedFormulaPos )
            , bValid(                r.bValid )
            , bRefresh(              r.bRefresh )
            , bCollectNotifications( r.bCollectNotifications )
    {
    }

    ScInterpreterTableOpParams& operator =( const ScInterpreterTableOpParams& r )
    {
        aOld1                 = r.aOld1;
        aNew1                 = r.aNew1;
        aOld2                 = r.aOld2;
        aNew2                 = r.aNew2;
        aFormulaPos           = r.aFormulaPos;
        //! never copied!   aNotifiedFormulaCells = r.aNotifiedFormulaCells;
        //! instead, empty anything eventually present
        ::std::vector< ScFormulaCell* >().swap( aNotifiedFormulaCells );
        aNotifiedFormulaPos   = r.aNotifiedFormulaPos;
        bValid                = r.bValid;
        bRefresh              = r.bRefresh;
        bCollectNotifications = r.bCollectNotifications;
        return *this;
    }

    sal_Bool operator ==( const ScInterpreterTableOpParams& r )
    {
        return
            bValid && r.bValid &&
            aOld1       == r.aOld1 &&
            aOld2       == r.aOld2 &&
            aFormulaPos == r.aFormulaPos ;
            // aNotifiedFormula(Cells|Pos), aNew1, aNew2, bRefresh,
            // bCollectNotifications are not compared
    }
};

DECLARE_LIST( ScTabOpList, ScInterpreterTableOpParams* )

#endif // SC_TABOPPARAMS_HXX

