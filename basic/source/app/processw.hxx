/*************************************************************************
 *
 *  $RCSfile: processw.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:09 $
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

#ifndef _PROCESSW_HXX
#define _PROCESSW_HXX

#ifndef __SBX_SBX_FACTORY_HXX //autogen
#include <svtools/sbxfac.hxx>
#endif
#ifndef __SBX_SBXVARIABLE_HXX //autogen
#include <svtools/sbxvar.hxx>
#endif
#ifndef _SBX_SBXOBJECT_HXX //autogen
#include <svtools/sbxobj.hxx>
#endif
#include "process.hxx"

class ProcessWrapper : public SbxObject
{
    // Definition eines Tabelleneintrags. Dies wird hier gemacht,
    // da dadurch die Methoden und Properties als private deklariert
    // werden koennen.
#if defined ( ICC ) || defined ( HPUX ) || defined ( C50 )
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
