/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
    virtual SbxObject* CreateObject( const rtl::OUString& );
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
    virtual SbxObject* CreateObject( const rtl::OUString& );

    SbModule* FindClass( const String& rClassName );
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
    bool            bCompiler;      // flag for compiler error
    bool            bGlobalInitErr;
    bool            bRunInit;       // true, if RunInit active from the Basic
    String          aErrMsg;        // buffer for GetErrorText()
    ::utl::TransliterationWrapper* pTransliterationWrapper;    // For StrComp
    bool            bBlockCompilerError;
    BasicManager*   pAppBasMgr;
    StarBASIC*      pMSOMacroRuntimLib; // Lib containing MSO Macro Runtime API entry symbols

    SbiGlobals();
    ~SbiGlobals();
};

// utility macros and routines

BASIC_DLLPUBLIC SbiGlobals* GetSbData();

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
