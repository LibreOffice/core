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

#ifndef SC_TABOPPARAMS_HXX
#define SC_TABOPPARAMS_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
#include <vector>
namespace binfilter {

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
    BOOL        bValid;
    BOOL        bRefresh;
    BOOL        bCollectNotifications;

    ScInterpreterTableOpParams()
            : bValid( FALSE )
            , bRefresh( FALSE )
            , bCollectNotifications( TRUE )
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

    BOOL operator ==( const ScInterpreterTableOpParams& r )
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

DECLARE_LIST( ScTabOpList, ScInterpreterTableOpParams* )//STRIP008 DECLARE_LIST( ScTabOpList, ScInterpreterTableOpParams* );

} //namespace binfilter
#endif // SC_TABOPPARAMS_HXX

