/*************************************************************************
 *
 *  $RCSfile: tabopparams.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $ $
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

#ifndef SC_TABOPPARAMS_HXX
#define SC_TABOPPARAMS_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
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
            , aNotifiedFormulaCells( r.aNotifiedFormulaCells )
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
        aNotifiedFormulaCells = r.aNotifiedFormulaCells;
        aNotifiedFormulaPos   = r.aNotifiedFormulaPos;
        bValid                = r.bValid;
        bRefresh              = r.bRefresh;
        bCollectNotifications = r.bCollectNotifications;
        return *this;
    }

    operator ==( const ScInterpreterTableOpParams& r )
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

DECLARE_LIST( ScTabOpList, ScInterpreterTableOpParams* );

#endif // SC_TABOPPARAMS_HXX

