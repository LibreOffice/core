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
// Definition of a table entry. This is done here because
// through this methods and property can declared as private.
#if defined ( ICC ) || defined ( HPUX ) || defined ( C50 ) || defined ( C52 )
public:
#endif
    typedef void( ProcessWrapper::*pMeth )
        ( SbxVariable* pThis, SbxArray* pArgs, sal_Bool bWrite );
#if defined ( ICC ) || defined ( HPUX )
private:
#endif

    struct Methods {
        const char* pName;      // Name of the entry
        SbxDataType eType;      // Data type
        pMeth pFunc;            // Function Pointer
        short nArgs;            // Arguments and flags
    };
    static Methods aProcessMethods[];   // Method table
    Methods *pMethods;  // Current method table

    void PSetImage( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void PStart( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void PGetExitCode( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void PIsRunning( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void PWasGPF( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );

    // Internal members and methods
    Process *pProcess;

    // Fill info block
    SbxInfo* GetInfo( short nIdx );

    // Broadcaster Notification
    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType );
public:
    ProcessWrapper();
    ~ProcessWrapper();
    // Search for an element
    virtual SbxVariable* Find( const String&, SbxClassType );
};

// Factory
class ProcessFactory : public SbxFactory
{
public:
    virtual SbxObject* CreateObject( const String& );
};

#endif
