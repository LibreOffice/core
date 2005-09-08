/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: refupdat.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 18:35:13 $
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

#ifndef SC_REFUPDAT_HXX
#define SC_REFUPDAT_HXX

#include "global.hxx"

class ScDocument;
class ScBigRange;
struct ComplRefData;
class ScAddress;
class ScRange;

enum ScRefUpdateRes {
    UR_NOTHING  = 0,        // keine Anpassungen
    UR_UPDATED  = 1,        // Anpassungen erfolgt
    UR_INVALID  = 2         // Referenz wurde ungueltig
};

class ScRefUpdate
{
public:
    static ScRefUpdateRes Update
        ( ScDocument* pDoc, UpdateRefMode eUpdateRefMode,
                            SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                            SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                            SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                            SCCOL& theCol1, SCROW& theRow1, SCTAB& theTab1,
                            SCCOL& theCol2, SCROW& theRow2, SCTAB& theTab2 );

    static ScRefUpdateRes Update( UpdateRefMode eUpdateRefMode,
                                const ScBigRange& rWhere,
                                INT32 nDx, INT32 nDy, INT32 nDz,
                                ScBigRange& rWhat );

    static ScRefUpdateRes Update( ScDocument* pDoc,
                                  UpdateRefMode eUpdateRefMode,
                                  const ScAddress& rPos, const ScRange& rRange,
                                  SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                                  ComplRefData& rRef );
    static ScRefUpdateRes Move( ScDocument* pDoc, const ScAddress& rPos,
                                SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                                ComplRefData& rRef, BOOL bWrap, BOOL bAbsolute );
    static void MoveRelWrap( ScDocument* pDoc, const ScAddress& rPos,
                                ComplRefData& rRef );

    static ScRefUpdateRes UpdateTranspose( ScDocument* pDoc,
                                const ScRange& rSource, const ScAddress& rDest,
                                ComplRefData& rRef );
    static void DoTranspose( SCsCOL& rCol, SCsROW& rRow, SCsTAB& rTab, ScDocument* pDoc,
                                const ScRange& rSource, const ScAddress& rDest );

    static ScRefUpdateRes UpdateGrow(
                                const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY,
                                ComplRefData& rRef );

    static ScRefUpdateRes DoGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY,
                                ScRange& rRef );
};


#endif

