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

