/*************************************************************************
 *
 *  $RCSfile: toolsdll.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
