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
