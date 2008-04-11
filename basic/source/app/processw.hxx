/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: processw.hxx,v $
 * $Revision: 1.8 $
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

#ifndef _PROCESSW_HXX
#define _PROCESSW_HXX

#include <basic/sbxfac.hxx>
#ifndef __SBX_SBXVARIABLE_HXX //autogen
#include <basic/sbxvar.hxx>
#endif
#include <basic/sbxobj.hxx>
#include <basic/process.hxx>

class ProcessWrapper : public SbxObject
{
using SbxVariable::GetInfo;
    // Definition eines Tabelleneintrags. Dies wird hier gemacht,
    // da dadurch die Methoden und Properties als private deklariert
    // werden koennen.
#if defined ( ICC ) || defined ( HPUX ) || defined ( C50 ) || defined ( C52 )
public:
#endif
    typedef void( ProcessWrapper::*pMeth )
        ( SbxVariable* pThis, SbxArray* pArgs, BOOL bWrite );
#if defined ( ICC ) || defined ( HPUX )
private:
#endif

    struct Methods {
        const char* pName;      // Name des Eintrags
        SbxDataType eType;      // Datentyp
        pMeth pFunc;            // Function Pointer
        short nArgs;            // Argumente und Flags
    };
    static Methods aProcessMethods[];   // Methodentabelle
    Methods *pMethods;  // Aktuelle Methodentabelle

    // Methoden
    void PSetImage( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite );
    void PStart( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite );
    void PGetExitCode( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite );
    void PIsRunning( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite );
    void PWasGPF( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite );

    // Interne Member und Methoden
    Process *pProcess;

    // Infoblock auffuellen
    SbxInfo* GetInfo( short nIdx );

    // Broadcaster Notification
    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType );
public:
    ProcessWrapper();
    ~ProcessWrapper();
    // Suchen eines Elements
    virtual SbxVariable* Find( const String&, SbxClassType );
};


// Die dazugehoerige Factory:

class ProcessFactory : public SbxFactory
{
public:
    virtual SbxObject* CreateObject( const String& );
};

#endif
