/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tabopparams.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2007-02-27 11:58:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_TABOPPARAMS_HXX
#define SC_TABOPPARAMS_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

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

