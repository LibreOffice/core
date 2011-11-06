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



#include "hashtbl.hxx"

#define MAXFILT 200

struct LibExport
{
    char            *cExportName;       // zu exportierende Fkt.
    unsigned long   nOrdinal;           // Nummer der zu export. Fkt.
    bool            bByName;            // NONAME anhaengen
    bool            bExport;            // exportieren oder nicht ?
};

class ExportSet;
class LibDump
{
    ExportSet       *pBaseTab;          // Zugriff auf gemangelte Namen
    ExportSet       *pIndexTab;         // Zugriff auf die Ordinals
    char            *cBName;            // Name der Datenbasis
    char            *cAPrefix;          // Prefix fuer C-Fkts.
    char            *cLibName;          // Name der zu untersuchenden Lib
    char            *cFilterName;       // Name der Filterdatei
    char            *cModName;          // Modulname
    unsigned short  nBegin;             // Nummer des ersten Exports
    unsigned long   nBaseLines;         // Line in Datenbasis
    unsigned long   nFilterLines;       // Line in FilterTabelle
    char            **pFilterLines;     // Filtertabelle
    unsigned long   nDefStart;
    bool            bBase;              // Existenz der DatenBasis;
    bool            bAll;               // Alle Fkts exportieren
    bool            bDef;               // DefFile schreiben ( bei -E )
    int             bExportName;        // 0 - export by ordinal; 1 - export by name

    bool            CheckDataBase();
    bool            CheckLibrary(char * cName);
    bool            ReadDataBase();
    bool            ReadFilter(char *);
    bool            PrintSym(char *, bool bName = true );
public:
                    LibDump( char *cFileName, int bExportByName );
                    ~LibDump();
    bool            Dump();
    bool            SetFilter(char *cFilterName);
    void            SetBeginExport(unsigned short nVal){nBegin = nVal;}
    void            SetCExport( char* pName );
    bool            Filter(char *pName);
    bool            IsFromAnonymousNamespace(char *pName);
    bool            PrintDefFile();
    bool            PrintDataBase();
    static void     DumpError(unsigned long nError);
};

