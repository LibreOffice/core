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
