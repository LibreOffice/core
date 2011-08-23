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

#ifdef WIN

#ifndef _SVWIN_H
#include <svwin.h>
#endif

#ifndef _SYSDEP_HXX
#include <sysdep.hxx>
#endif

#endif

#ifdef WIN
// Statische DLL-Verwaltungs-Variablen
static HINSTANCE hDLLInst = 0;      // HANDLE der DLL

/***************************************************************************
|*    LibMain()
|*
|*    Beschreibung       Initialisierungsfunktion der DLL
***************************************************************************/
extern "C" int CALLBACK LibMain( HINSTANCE hDLL, WORD, WORD nHeap, LPSTR )
{
#ifndef WNT
    if ( nHeap )
        UnlockData( 0 );
#endif

    hDLLInst = hDLL;

    return TRUE;
}

/***************************************************************************
|*    WEP()
|*
|*    Beschreibung      DLL-Deinitialisierung
***************************************************************************/
extern "C" int CALLBACK WEP( int )
{
    return 1;
}

#endif

#ifdef WNT
void ResourceDummy (void )
{

}
#endif

#ifdef OS2

#ifndef _SVWIN_H
#include <svpm.h>
#endif

// Statische DLL-Verwaltungs-Variablen
static ULONG hDLLInst = 0;      // HANDLE der DLL


/***************************************************************************
|*    LibMain()
|*
|*    Beschreibung       Initialisierungsfunktion der DLL
***************************************************************************/
extern "C" int LibDummy()
{
    return TRUE;
}

/***************************************************************************
|*    WEP()
|*
|*    Beschreibung      DLL-Deinitialisierung
***************************************************************************/
extern "C" int WEPDummy( int )
{
    return 1;
}

#endif

