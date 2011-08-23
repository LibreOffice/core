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

#ifndef _SB_INTERN_HXX
#define _SB_INTERN_HXX

#include <sbxfac.hxx>
#include <unotools/transliterationwrapper.hxx>
#include "sb.hxx"

namespace 
{
    class TransliterationWrapper;
}

namespace binfilter {

class SbUnoFactory;
class SbTypeFactory;
class SbOLEFactory;
class SbiInstance;
class SbModule;

class SbiFactory : public SbxFactory
{
public:
    virtual SbxBase* Create( UINT16 nSbxId, UINT32 = SBXCR_SBX );
    virtual SbxObject* CreateObject( const String& );
};

struct SbClassData
{
    SbxArrayRef		mxIfaces;

    ~SbClassData( void )
        { clear(); }
    void clear( void );
};

// #115824: Factory class to create class objects (type command)
// Implementation: sb.cxx
class SbClassFactory : public SbxFactory
{
    SbxObjectRef	xClassModules;

public:
    SbClassFactory( void );
    virtual ~SbClassFactory();

    virtual SbxBase* Create( UINT16 nSbxId, UINT32 = SBXCR_SBX );
    virtual SbxObject* CreateObject( const String& );
};

// Stack fuer die im Fehlerfall abgebaute SbiRuntime Kette
class SbErrorStackEntry
{
public:
    SbErrorStackEntry(SbMethodRef aM, xub_StrLen nL, xub_StrLen nC1, xub_StrLen nC2)
        : aMethod(aM), nLine(nL), nCol1(nC1), nCol2(nC2) {}
    SbMethodRef aMethod;
    xub_StrLen nLine;
    xub_StrLen nCol1, nCol2;
};

SV_DECL_PTRARR_DEL(SbErrorStack, SbErrorStackEntry*, 1, 1)



struct SbiGlobals
{
    SbiInstance*	pInst; 			// alle aktiven Runtime-Instanzen
    SbiFactory*		pSbFac;			// StarBASIC-Factory
    SbUnoFactory*	pUnoFac;		// Factory fuer Uno-Structs bei DIM AS NEW
    SbTypeFactory*	pTypeFac;		// Factory for user defined types
    SbClassFactory*	pClassFac;		// Factory for user defined classes (based on class modules)
    SbOLEFactory*	pOLEFac;		// Factory for OLE types
    SbModule*		pMod;			// aktuell aktives Modul
    SbModule*		pCompMod;		// aktuell compiliertes Modul
    short			nInst;			// Anzahl BASICs
    Link			aErrHdl;		// globaler Error-Handler
    Link			aBreakHdl;		// globaler Break-Handler
    SbError			nCode;			// aktueller Fehlercode
    xub_StrLen		nLine;			// aktuelle Zeile
    xub_StrLen		nCol1,nCol2;	// aktuelle Spalten (von,bis)
    BOOL			bCompiler;		// Flag fuer Compiler-Error
    BOOL			bGlobalInitErr;	// Beim GlobalInit trat ein Compiler-Fehler auf
    BOOL			bRunInit;		// TRUE, wenn RunInit vom Basic aktiv ist
    String			aErrMsg;		// Puffer fuer GetErrorText()
    SbLanguageMode	eLanguageMode;	// Flag fuer Visual-Basic-Script-Modus
    SbErrorStack*	pErrStack;		// Stack fuer die im Fehlerfall abgebaute SbiRuntime Kette
    ::utl::TransliterationWrapper* pTransliterationWrapper;    // For StrComp
    BOOL			bBlockCompilerError;
    BasicManager*   pAppBasMgr;

    SbiGlobals();
    ~SbiGlobals();
};

// Utility-Makros und -Routinen

SbiGlobals* GetSbData();

#define	pINST		GetSbData()->pInst
#define pMOD		GetSbData()->pMod
#define pCMOD		GetSbData()->pCompMod
#define pSBFAC		GetSbData()->pSbFac
#define pUNOFAC		GetSbData()->pUnoFac
#define pTYPEFAC	GetSbData()->pTypeFac
#define pCLASSFAC	GetSbData()->pClassFac
#define pOLEFAC		GetSbData()->pOLEFac

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
