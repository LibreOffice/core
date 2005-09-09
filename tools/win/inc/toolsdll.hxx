/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: toolsdll.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:46:28 $
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

#ifndef _TOOLSDLL_HXX
#define _TOOLSDLL_HXX

#ifdef WIN

#ifndef _STRING_H
#include <string.h>
#endif

#ifndef _SVWIN_H
#include <svwin.h>
#endif

#ifndef _SOLAR_H
#include <solar.h>
#endif

#ifndef _SHL_HXX
#include <shl.hxx>
#endif

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
