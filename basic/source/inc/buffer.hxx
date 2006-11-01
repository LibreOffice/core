/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: buffer.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-01 16:14:47 $
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

#ifndef _BUFFER_HXX
#define _BUFFER_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

class SbiParser;

class SbiBuffer {                   // Code/Konstanten-Puffer:
    SbiParser* pParser;             // fuer Fehlermeldungen
    char*   pBuf;                   // Puffer-Pointer
    char*   pCur;                   // aktueller Puffer-Pointer
    UINT32  nOff;                   // aktuelles Offset
    UINT32  nSize;                  // aktuelle Groesse
    short   nInc;                   // Inkrement
    BOOL    Check( USHORT );        // Buffergroesse testen
public:
    SbiBuffer( SbiParser*, short ); // Inkrement
   ~SbiBuffer();
    void Patch( UINT32, UINT32 );   // Patchen
    void Chain( UINT32 );           // Back-Chain
    void Align( INT32 );            // Alignment
    BOOL Add( const void*, USHORT );// Element anfuegen
    BOOL operator += (const String&);// Basic-String speichern
    BOOL operator += (INT8);        // Zeichen speichern
    BOOL operator += (INT16);       // Integer speichern
    BOOL operator += (UINT8);       // Zeichen speichern
    BOOL operator += (UINT16);      // Integer speichern
    BOOL operator += (UINT32);      // Integer speichern
    BOOL operator += (INT32);       // Integer speichern
    char*  GetBuffer();             // Puffer rausgeben (selbst loeschen!)
    char*  GetBufferPtr(){ return pBuf; }
    UINT32 GetSize()                { return nOff; }
};

#endif
