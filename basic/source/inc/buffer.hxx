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

#ifndef _BUFFER_HXX
#define _BUFFER_HXX

#include <tools/solar.h>
#include <tools/string.hxx>

class SbiParser;

class SbiBuffer {                   // Code/Konstanten-Puffer:
    SbiParser* pParser;             // fuer Fehlermeldungen
    char*   pBuf;                   // Puffer-Pointer
    char*   pCur;                   // aktueller Puffer-Pointer
    sal_uInt32  nOff;                   // aktuelles Offset
    sal_uInt32  nSize;                  // aktuelle Groesse
    short   nInc;                   // Inkrement
    sal_Bool    Check( sal_uInt16 );        // Buffergroesse testen
public:
    SbiBuffer( SbiParser*, short ); // Inkrement
   ~SbiBuffer();
    void Patch( sal_uInt32, sal_uInt32 );   // Patchen
    void Chain( sal_uInt32 );           // Back-Chain
    void Align( sal_Int32 );            // Alignment
    sal_Bool Add( const void*, sal_uInt16 );// Element anfuegen
    sal_Bool operator += (const String&);// Basic-String speichern
    sal_Bool operator += (sal_Int8);        // Zeichen speichern
    sal_Bool operator += (sal_Int16);       // Integer speichern
    sal_Bool operator += (sal_uInt8);       // Zeichen speichern
    sal_Bool operator += (sal_uInt16);      // Integer speichern
    sal_Bool operator += (sal_uInt32);      // Integer speichern
    sal_Bool operator += (sal_Int32);       // Integer speichern
    char*  GetBuffer();             // Puffer rausgeben (selbst loeschen!)
    char*  GetBufferPtr(){ return pBuf; }
    sal_uInt32 GetSize()                { return nOff; }
};

#endif
