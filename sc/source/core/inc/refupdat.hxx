/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <global.hxx>

class ScDocument;
class ScBigRange;
struct ScComplexRefData;
class ScAddress;
class ScRange;

enum ScRefUpdateRes {
    UR_NOTHING  = 0,        ///< Reference not affected, no change at all.
    UR_UPDATED  = 1,        ///< Reference was adjusted/updated.
    UR_INVALID  = 2,        ///< Some part of the reference became invalid.
    UR_STICKY   = 3         /**< Not updated because the reference is sticky,
                              but would had been updated if it wasn't. For
                              entire columns/rows. Essentially the same as
                              not UR_NOTHING for the caller but allows
                              differentiation. */
};

class ScRefUpdate
{
public:

    static ScRefUpdateRes Update
        ( const ScDocument* pDoc, UpdateRefMode eUpdateRefMode,
                            SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                            SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                            SCCOL nDx, SCROW nDy, SCTAB nDz,
                            SCCOL& theCol1, SCROW& theRow1, SCTAB& theTab1,
                            SCCOL& theCol2, SCROW& theRow2, SCTAB& theTab2 );

    static ScRefUpdateRes Update( UpdateRefMode eUpdateRefMode,
                                const ScBigRange& rWhere,
                                sal_Int32 nDx, sal_Int32 nDy, sal_Int32 nDz,
                                ScBigRange& rWhat );

    static void MoveRelWrap( const ScDocument& rDoc, const ScAddress& rPos,
                             SCCOL nMaxCol, SCROW nMaxRow, ScComplexRefData& rRef );

    static ScRefUpdateRes UpdateTranspose(
        const ScDocument& rDoc, const ScRange& rSource, const ScAddress& rDest, ScRange& rRef );

    static void DoTranspose( SCCOL& rCol, SCROW& rRow, SCTAB& rTab, const ScDocument& rDoc,
                                const ScRange& rSource, const ScAddress& rDest );

    static ScRefUpdateRes UpdateGrow(
        const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY, ScRange& rRef );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
