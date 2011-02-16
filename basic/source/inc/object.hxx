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

#ifndef _SAMPLE_OBJECT_HXX
#define _SAMPLE_OBJECT_HXX

#include <basic/sbxfac.hxx>
#include <basic/sbxvar.hxx>
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
#if defined ( ICC ) || defined ( C50 ) || defined ( C52 )
public:
#endif
    typedef void( SampleObject::*pMeth )
        ( SbxVariable* pThis, SbxArray* pArgs, BOOL bWrite );
#if defined ( ICC )
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
    void Display( SbxVariable*, SbxArray*, BOOL );
    void Event( SbxVariable*, SbxArray*, BOOL );
    void Square( SbxVariable*, SbxArray*, BOOL );
    void Create( SbxVariable*, SbxArray*, BOOL );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
