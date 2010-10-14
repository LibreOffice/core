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

#ifndef _TOOLSDLL_HXX
#define _TOOLSDLL_HXX

#ifdef WIN

#include <string.h>
#include <tools/svwin.h>
#include <tools/solar.h>
#include <tools/shl.hxx>

// ----------------
// - MemMgr-Daten -
// ----------------

// Anzahl Freispeicherlisten
#define MEM_FREELIST_COUNT  15


// --------------
// - Tools-Data -
// --------------

struct SVDATA;
struct FreeBlock;

struct MemData
{
    FreeBlock*  pFirstBlocks[MEM_FREELIST_COUNT];   // Erste Bloecke in Freispeicherlisten
    void*       pNewHandler;                        // New-Handler
    int         nMultiThread;                       // MultiThread an/aus
};

struct ToolsData
{
    // !!! SVDATA muss in jedem Fall ganz vorne in der Struktur stehen !!!
    SVDATA*     pSVData;                // Pointer auf StarView-Daten
    HANDLE      hMem;                   // Handler der Tools-Daten
    MemData     aMemD;                  // Daten der Speicherverwaltung
    void*       aAppData[SHL_COUNT];    // Daten fuer andere Shared Libs
};


// ----------------------
// - Zugriffsfunktionen -
// ----------------------

// IN APPDATA.ASM
extern "C"
{
ToolsData*  FAR PASCAL ImpGetAppData();
void        FAR PASCAL ImpSetAppData( ToolsData* pData );
}

// IN TOOLSDLL.CXX
ToolsData* ImpGetToolsData();

#endif

#endif // _DLL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
