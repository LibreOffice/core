/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ldump.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 14:07:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

