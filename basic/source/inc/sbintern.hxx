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

#pragma once

#include <basic/basicdllapi.h>
#include <basic/sbstar.hxx>
#include <sbxfac.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <vcl/errcode.hxx>

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
class BasicManager;

class SbiFactory : public SbxFactory
{
public:
    virtual SbxBase* Create( sal_uInt16 nSbxId, sal_uInt32 ) override;
    virtual SbxObject* CreateObject( const OUString& ) override;
};

struct SbClassData
{
    SbxArrayRef     mxIfaces;

    // types this module depends on because of use in Dim As New <type>
    // needed for initialization order of class modules
    std::vector< OUString >    maRequiredTypes;

    SbClassData();
    ~SbClassData()
        { clear(); }
    void clear();
};

// #115824: Factory class to create class objects (type command)
// Implementation: sb.cxx
class SbClassFactory : public SbxFactory
{
    SbxObjectRef    xClassModules;

public:
    SbClassFactory();
    virtual ~SbClassFactory() override;

    void AddClassModule( SbModule* pClassModule );
    void RemoveClassModule( SbModule* pClassModule );

    virtual SbxBase* Create( sal_uInt16 nSbxId, sal_uInt32 ) override;
    virtual SbxObject* CreateObject( const OUString& ) override;

    SbModule* FindClass( const OUString& rClassName );
};

// Factory class to create user defined objects (type command)
class SbTypeFactory : public SbxFactory
{
public:
    virtual SbxBase* Create( sal_uInt16 nSbxId, sal_uInt32 ) override;
    virtual SbxObject* CreateObject( const OUString& ) override;
};

class SbFormFactory : public SbxFactory
{
public:
    virtual SbxBase* Create( sal_uInt16 nSbxId, sal_uInt32 ) override;
    virtual SbxObject* CreateObject( const OUString& ) override;
};

// Factory class to create OLE objects
class SbOLEFactory : public SbxFactory
{
public:
    virtual SbxBase* Create( sal_uInt16 nSbxId, sal_uInt32 ) override;
    virtual SbxObject* CreateObject( const OUString& ) override;
};



struct SbiGlobals
{
    static SbiGlobals* pGlobals;
    SbiInstance*    pInst;          // all active runtime instances
    std::unique_ptr<SbiFactory>   pSbFac;    // StarBASIC-Factory
    std::unique_ptr<SbUnoFactory> pUnoFac;   // Factory for Uno-Structs at DIM AS NEW
    std::unique_ptr<SbTypeFactory>
                    pTypeFac;       // Factory for user defined types
    std::unique_ptr<SbClassFactory>
                    pClassFac;      // Factory for user defined classes (based on class modules)
    std::unique_ptr<SbOLEFactory>
                    pOLEFac;        // Factory for OLE types
    std::unique_ptr<SbFormFactory>
                    pFormFac;       // Factory for user forms
    SbModule*       pMod;           // currently active module
    SbModule*       pCompMod;       // currently compiled module
    short           nInst;          // number of BASICs
    Link<StarBASIC*,bool>            aErrHdl;        // global error handler
    Link<StarBASIC*,BasicDebugFlags> aBreakHdl;      // global break handler
    ErrCode         nCode;
    sal_Int32       nLine;
    sal_Int32       nCol1,nCol2;    // from... to...
    bool            bCompilerError; // flag for compiler error
    bool            bGlobalInitErr;
    bool            bRunInit;       // true, if RunInit active from the Basic
    OUString        aErrMsg;        // buffer for GetErrorText()
    std::unique_ptr<::utl::TransliterationWrapper> pTransliterationWrapper;    // For StrComp
    bool            bBlockCompilerError;
    std::unique_ptr<BasicManager>   pAppBasMgr;
    StarBASIC*      pMSOMacroRuntimLib; // Lib containing MSO Macro Runtime API entry symbols

    SbiGlobals();
    ~SbiGlobals();
};

// utility macros and routines

SbiGlobals* GetSbData();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
