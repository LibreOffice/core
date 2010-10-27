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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
