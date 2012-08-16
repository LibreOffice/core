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

#ifndef SC_UNDOUTIL_HXX
#define SC_UNDOUTIL_HXX

#include "address.hxx"
#include <tools/solar.h>

class ScRange;
class ScDocShell;
class ScDBData;
class ScDocument;

//----------------------------------------------------------------------------

class ScUndoUtil
{
public:
    /**  Mark Block (invisible - has to be repainted) */
    static void MarkSimpleBlock( ScDocShell* pDocShell,
                                SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                                SCCOL nEndX, SCROW nEndY, SCTAB nEndZ );
    static void MarkSimpleBlock( ScDocShell* pDocShell,
                                const ScAddress& rBlockStart,
                                const ScAddress& rBlockEnd );
    static void MarkSimpleBlock( ScDocShell* pDocShell,
                                const ScRange& rRange );

    static void PaintMore( ScDocShell* pDocShell,
                                const ScRange& rRange );

    /** Search for Data base range in Document ("untitled" or by region)
        create new if not found */
    static ScDBData* GetOldDBData( ScDBData* pUndoData, ScDocument* pDoc, SCTAB nTab,
                                    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
