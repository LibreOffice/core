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



#ifndef _SAMPLE_OBJECT_HXX
#define _SAMPLE_OBJECT_HXX

#include <basic/sbxfac.hxx>
#ifndef __SBX_SBXVARIABLE_HXX //autogen
#include <basic/sbxvar.hxx>
#endif
#include <basic/sbxobj.hxx>

// 1) Properties:
//    Name      der Name, R/O
//    Value     ein double-Wert, R/W
// 2) Methoden:
//    Display   Ausgabe eines Textes
//    Square    Argument * Argument
//    Event     Aufruf eines Basic-Programms
// 3) Unterobjekte:
//    eine Collection names "Elements". Der Zugriff ist sowohl als
//    Property (fuer das gesamte Objekt) als auch als Methode (fuer
//    einzelne Elemente, wird durchgereicht) implementiert.
// Diese Implementation ist ein Beispiel fuer eine tabellengesteuerte
// Version, die sehr viele Elemente enthalten kann.
// Die Collection findet sich in COLLECTN.*, die in der Collection
// enthaltenen Objekte in COLLELEM.*

class SampleObject : public SbxObject
{
using SbxVariable::GetInfo;
    // Definition eines Tabelleneintrags. Dies wird hier gemacht,
    // da dadurch die Methoden und Properties als private deklariert
    // werden koennen.
#if defined ( ICC ) || defined ( HPUX ) || defined ( C50 ) || defined ( C52 )
public:
#endif
    typedef void( SampleObject::*pMeth )
        ( SbxVariable* pThis, SbxArray* pArgs, sal_Bool bWrite );
#if defined ( ICC ) || defined ( HPUX )
private:
#endif

    struct Methods {
        const char* pName;      // Name des Eintrags
        SbxDataType eType;      // Datentyp
        pMeth pFunc;            // Function Pointer
        short nArgs;            // Argumente und Flags
    };
    static Methods aMethods[];  // Methodentabelle

    // Methoden
    void Display( SbxVariable*, SbxArray*, sal_Bool );
    void Event( SbxVariable*, SbxArray*, sal_Bool );
    void Square( SbxVariable*, SbxArray*, sal_Bool );
    void Create( SbxVariable*, SbxArray*, sal_Bool );
    // Infoblock auffuellen
    SbxInfo* GetInfo( short nIdx );
    // Broadcaster Notification
    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType );
public:
    SampleObject( const String& );
    // Suchen eines Elements
    virtual SbxVariable* Find( const String&, SbxClassType );
};

// Die dazugehoerige Factory:

class SampleObjectFac : public SbxFactory
{
public:
    virtual SbxObject* CreateObject( const String& );
};

#endif
