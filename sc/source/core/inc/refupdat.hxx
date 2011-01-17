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

#ifndef SC_REFUPDAT_HXX
#define SC_REFUPDAT_HXX

#include "global.hxx"

class ScDocument;
class ScBigRange;
struct ScComplexRefData;
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

    /// What type of reference is to be updated.
    enum WhatType
    {
        ALL,        /// all references
        ABSOLUTE    /// only absolute references
    };

    static ScRefUpdateRes Update
        ( ScDocument* pDoc, UpdateRefMode eUpdateRefMode,
                            SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                            SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                            SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                            SCCOL& theCol1, SCROW& theRow1, SCTAB& theTab1,
                            SCCOL& theCol2, SCROW& theRow2, SCTAB& theTab2 );

    static ScRefUpdateRes Update( UpdateRefMode eUpdateRefMode,
                                const ScBigRange& rWhere,
                                sal_Int32 nDx, sal_Int32 nDy, sal_Int32 nDz,
                                ScBigRange& rWhat );

    /// Before calling, the absolute references must be up-to-date!
    static ScRefUpdateRes Update( ScDocument* pDoc,
                                  UpdateRefMode eUpdateRefMode,
                                  const ScAddress& rPos, const ScRange& rRange,
                                  SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                                  ScComplexRefData& rRef, WhatType eWhat = ALL );

    /// Before calling, the absolute references must be up-to-date!
    static ScRefUpdateRes Move( ScDocument* pDoc, const ScAddress& rPos,
                                SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                                ScComplexRefData& rRef, sal_Bool bWrap, sal_Bool bAbsolute );

    static void MoveRelWrap( ScDocument* pDoc, const ScAddress& rPos,
                             SCCOL nMaxCol, SCROW nMaxRow, ScComplexRefData& rRef );

    /// Before calling, the absolute references must be up-to-date!
    static ScRefUpdateRes UpdateTranspose( ScDocument* pDoc,
                                const ScRange& rSource, const ScAddress& rDest,
                                ScComplexRefData& rRef );

    static void DoTranspose( SCsCOL& rCol, SCsROW& rRow, SCsTAB& rTab, ScDocument* pDoc,
                                const ScRange& rSource, const ScAddress& rDest );

    /// Before calling, the absolute references must be up-to-date!
    static ScRefUpdateRes UpdateGrow(
                                const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY,
                                ScComplexRefData& rRef );
};


#endif

