/*************************************************************************
 *
 *  $RCSfile: refupdat.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 10:31:47 $
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

