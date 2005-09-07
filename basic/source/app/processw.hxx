/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: processw.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:19:11 $
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

#ifndef _PROCESSW_HXX
#define _PROCESSW_HXX

#ifndef __SBX_SBX_FACTORY_HXX //autogen
#include <sbxfac.hxx>
#endif
#ifndef __SBX_SBXVARIABLE_HXX //autogen
#include <sbxvar.hxx>
#endif
#ifndef _SBX_SBXOBJECT_HXX //autogen
#include <sbxobj.hxx>
#endif
#include "process.hxx"

class ProcessWrapper : public SbxObject
{
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
        short nArgs;            // Argumente und Flags
        pMeth pFunc;            // Function Pointer
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
