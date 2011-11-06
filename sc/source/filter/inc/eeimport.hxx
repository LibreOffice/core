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


#ifndef SC_EEIMPORT_HXX
#define SC_EEIMPORT_HXX

#include "global.hxx"
#include "address.hxx"
#include "filter.hxx"
#include "scdllapi.h"

class ScDocument;
class ScEEParser;
class ScTabEditEngine;
class SvStream;
class Table;

struct ScEEParseEntry;

class ScEEImport : public ScEEAbsImport
{
protected:
    ScRange             maRange;
    ScDocument*         mpDoc;
    ScEEParser*         mpParser;
    ScTabEditEngine*    mpEngine;
    Table*              mpRowHeights;

    sal_Bool                GraphicSize( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                     ScEEParseEntry* );
    void                InsertGraphic( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                       ScEEParseEntry* );
public:
    ScEEImport( ScDocument* pDoc, const ScRange& rRange );
    virtual ~ScEEImport();

    virtual sal_uLong    Read( SvStream& rStream, const String& rBaseURL );
    virtual ScRange  GetRange() { return maRange; }
    virtual void     WriteToDocument( sal_Bool bSizeColsRows = sal_False,
                                      double nOutputFactor = 1.0,
                                      SvNumberFormatter* pFormatter = NULL,
                                      bool bConvertDate = true );
};

#endif
