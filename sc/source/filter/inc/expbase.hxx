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



#ifndef SC_EXPBASE_HXX
#define SC_EXPBASE_HXX

#include <tools/solar.h>
#include "global.hxx"
#include "address.hxx"


class SvStream;
class ScFieldEditEngine;

class ScExportBase
{
public:
#if defined UNX
    static const sal_Char __FAR_DATA sNewLine;
#else
    static const sal_Char __FAR_DATA sNewLine[];
#endif

protected:

    SvStream&           rStrm;
    ScRange             aRange;
    ScDocument*         pDoc;
    SvNumberFormatter*  pFormatter;
    ScFieldEditEngine*  pEditEngine;

public:

                        ScExportBase( SvStream&, ScDocument*, const ScRange& );
    virtual             ~ScExportBase();

                        // Hidden Cols/Rows an den Raendern trimmen,
                        // return: sal_True wenn Bereich vorhanden
                        // Start/End/Col/Row muessen gueltige Ausgangswerte sein
    sal_Bool                TrimDataArea( SCTAB nTab, SCCOL& nStartCol,
                            SCROW& nStartRow, SCCOL& nEndCol, SCROW& nEndRow ) const;

                        // Ausgabebereich einer Tabelle ermitteln,
                        // Hidden Cols/Rows an den Raendern beruecksichtigt,
                        // return: sal_True wenn Bereich vorhanden
    sal_Bool                GetDataArea( SCTAB nTab, SCCOL& nStartCol,
                            SCROW& nStartRow, SCCOL& nEndCol, SCROW& nEndRow ) const;

                        // Tabelle nicht vorhanden oder leer
    sal_Bool                IsEmptyTable( SCTAB nTab ) const;

    ScFieldEditEngine&  GetEditEngine() const;

};


#endif  // SC_EXPBASE_HXX

