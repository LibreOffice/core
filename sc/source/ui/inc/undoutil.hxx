/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
                    //  Block markieren (unsichtbar, muss repainted werden)
    static void MarkSimpleBlock( ScDocShell* pDocShell,
                                SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                                SCCOL nEndX, SCROW nEndY, SCTAB nEndZ );
    static void MarkSimpleBlock( ScDocShell* pDocShell,
                                const ScAddress& rBlockStart,
                                const ScAddress& rBlockEnd );
    static void MarkSimpleBlock( ScDocShell* pDocShell,
                                const ScRange& rRange );

                    //  Bereich +1 painten
    static void PaintMore( ScDocShell* pDocShell,
                                const ScRange& rRange );

                    //  DB-Bereich im Dokument suchen ("unbenannt" oder nach Bereich)
                    //  legt neu an, wenn nicht gefunden
    static ScDBData* GetOldDBData( ScDBData* pUndoData, ScDocument* pDoc, SCTAB nTab,
                                    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
