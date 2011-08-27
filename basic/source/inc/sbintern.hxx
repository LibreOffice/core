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

#include <basic/basicdllapi.h>
#include <basic/sbxfac.hxx>
#include <unotools/transliterationwrapper.hxx>
#include "sb.hxx"

namespace utl
{
    class TransliterationWrapper;
}
class SbUnoFactory;
class SbTypeFactory;
class SbOLEFactory;
class SbFormFactory;
class SbiInstance;
class SbModule;

class SbiFactory : public SbxFactory
{
public:
    virtual SbxBase* Create( sal_uInt16 nSbxId, sal_uInt32 = SBXCR_SBX );
    virtual SbxObject* CreateObject( const String& );
};

typedef ::std::vector< String > StringVector;

struct SbClassData
{
    SbxArrayRef     mxIfaces;

    // types this module depends on because of use in Dim As New <type>
    // needed for initialization order of class modules
    StringVector    maRequiredTypes;

    SbClassData( void );
    ~SbClassData( void )
        { clear(); }
    void clear( void );
};

// #115824: Factory class to create class objects (type command)
// Implementation: sb.cxx
class BASIC_DLLPUBLIC SbClassFactory : public SbxFactory
{
    SbxObjectRef    xClassModules;

public:
    SbClassFactory( void );
    virtual ~SbClassFactory();

    void AddClassModule( SbModule* pClassModule );
    void RemoveClassModule( SbModule* pClassModule );

    virtual SbxBase* Create( sal_uInt16 nSbxId, sal_uInt32 = SBXCR_SBX );
    virtual SbxObject* CreateObject( const String& );

    SbModule* FindClass( const String& rClassName );
};

// stack for the SbiRuntime chain which is removed in the case of an error
class BASIC_DLLPUBLIC SbErrorStackEntry
{
public:
    SbErrorStackEntry(SbMethodRef aM, xub_StrLen nL, xub_StrLen nC1, xub_StrLen nC2)
        : aMethod(aM), nLine(nL), nCol1(nC1), nCol2(nC2) {}
    SbMethodRef aMethod;
    xub_StrLen nLine;
    xub_StrLen nCol1, nCol2;
};

typedef sal_Bool (*FnForEach_SbErrorStack)( const SbErrorStackEntry* &, void* );
class BASIC_DLLPUBLIC SbErrorStack: public SvPtrarr
{
public:
    SbErrorStack( sal_uInt16 nIni=1, sal_uInt8 nG=1 )
        : SvPtrarr(nIni,nG) {}
    ~SbErrorStack() { DeleteAndDestroy( 0, Count() ); }
    void Insert( const SbErrorStack *pI, sal_uInt16 nP,
            sal_uInt16 nS = 0, sal_uInt16 nE = USHRT_MAX ) {
        SvPtrarr::Insert((const SvPtrarr*)pI, nP, nS, nE);
    }
    void Insert( const SbErrorStackEntry* & aE, sal_uInt16 nP ) {
        SvPtrarr::Insert((const VoidPtr &)aE, nP );
    }
    void Insert( const SbErrorStackEntry* *pE, sal_uInt16 nL, sal_uInt16 nP ) {
        SvPtrarr::Insert( (const VoidPtr *)pE, nL, nP );
    }
    void Replace( const SbErrorStackEntry* & aE, sal_uInt16 nP ) {
        SvPtrarr::Replace( (const VoidPtr &)aE, nP );
    }
    void Replace( const SbErrorStackEntry* *pE, sal_uInt16 nL, sal_uInt16 nP ) {
        SvPtrarr::Replace( (const VoidPtr*)pE, nL, nP );
    }
    void Remove( sal_uInt16 nP, sal_uInt16 nL = 1) {
        SvPtrarr::Remove(nP,nL);
    }
    const SbErrorStackEntry** GetData() const {
        return (const SbErrorStackEntry**)SvPtrarr::GetData();
    }
    void ForEach( CONCAT( FnForEach_, SbErrorStack ) fnForEach, void* pArgs = 0 )
    {
        _ForEach( 0, nA, (FnForEach_SvPtrarr)fnForEach, pArgs );
    }
    void ForEach( sal_uInt16 nS, sal_uInt16 nE,
                    CONCAT( FnForEach_, SbErrorStack ) fnForEach, void* pArgs = 0 )
    {
        _ForEach( nS, nE, (FnForEach_SvPtrarr)fnForEach, pArgs );
    }
    SbErrorStackEntry* operator[]( sal_uInt16 nP )const  {
        return (SbErrorStackEntry*)SvPtrarr::operator[](nP); }
    SbErrorStackEntry* GetObject( sal_uInt16 nP )const  {
        return (SbErrorStackEntry*)SvPtrarr::GetObject(nP); }

    sal_uInt16 GetPos( const SbErrorStackEntry* & aE ) const {
        return SvPtrarr::GetPos((const VoidPtr &)aE);
    }
    void DeleteAndDestroy( sal_uInt16 nP, sal_uInt16 nL=1 );
private:
    BASIC_DLLPRIVATE SbErrorStack( const SbErrorStack& );
    BASIC_DLLPRIVATE SbErrorStack& operator=( const SbErrorStack& );
};


struct SbiGlobals
{
    SbiInstance*    pInst;          // all active runtime instances
    SbiFactory*     pSbFac;         // StarBASIC-Factory
    SbUnoFactory*   pUnoFac;        // Factory for Uno-Structs at DIM AS NEW
    SbTypeFactory*  pTypeFac;       // Factory for user defined types
    SbClassFactory* pClassFac;      // Factory for user defined classes (based on class modules)
    SbOLEFactory*   pOLEFac;        // Factory for OLE types
    SbFormFactory*  pFormFac;       // Factory for user forms
    SbModule*       pMod;           // currently active module
    SbModule*       pCompMod;       // currently compiled module
    short           nInst;          // number of BASICs
    Link            aErrHdl;        // global error handler
    Link            aBreakHdl;      // global break handler
    SbError         nCode;
    xub_StrLen      nLine;
    xub_StrLen      nCol1,nCol2;    // from... to...
    sal_Bool            bCompiler;      // flag for compiler error
    sal_Bool            bGlobalInitErr;
    sal_Bool            bRunInit;       // sal_True, if RunInit active from the Basic
    String          aErrMsg;        // buffer for GetErrorText()
    SbLanguageMode  eLanguageMode;  // flag for Visual-Basic-Script-Modus
    SbErrorStack*   pErrStack;      // for the SbiRuntime chain
    ::utl::TransliterationWrapper* pTransliterationWrapper;    // For StrComp
    sal_Bool            bBlockCompilerError;
    BasicManager*   pAppBasMgr;
    StarBASIC*      pMSOMacroRuntimLib; // Lib containing MSO Macro Runtime API entry symbols

    SbiGlobals();
    ~SbiGlobals();
};

// utility macros and routines

BASIC_DLLPUBLIC SbiGlobals* GetSbData();

#define pINST       GetSbData()->pInst
#define pMOD        GetSbData()->pMod
#define pCMOD       GetSbData()->pCompMod
#define pSBFAC      GetSbData()->pSbFac
#define pUNOFAC     GetSbData()->pUnoFac
#define pTYPEFAC    GetSbData()->pTypeFac
#define pCLASSFAC   GetSbData()->pClassFac
#define pOLEFAC     GetSbData()->pOLEFac
#define pFORMFAC    GetSbData()->pFormFac

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
