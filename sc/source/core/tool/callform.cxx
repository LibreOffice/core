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

#include <sal/config.h>

#include <boost/noncopyable.hpp>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <osl/module.hxx>
#include <osl/file.hxx>
#include <unotools/transliterationwrapper.hxx>

#include "callform.hxx"
#include "global.hxx"
#include "adiasync.hxx"

extern "C" {

typedef void (CALLTYPE* ExFuncPtr1)(void*);
typedef void (CALLTYPE* ExFuncPtr2)(void*, void*);
typedef void (CALLTYPE* ExFuncPtr3)(void*, void*, void*);
typedef void (CALLTYPE* ExFuncPtr4)(void*, void*, void*, void*);
typedef void (CALLTYPE* ExFuncPtr5)(void*, void*, void*, void*, void*);
typedef void (CALLTYPE* ExFuncPtr6)(void*, void*, void*, void*, void*, void*);
typedef void (CALLTYPE* ExFuncPtr7)(void*, void*, void*, void*, void*, void*, void*);
typedef void (CALLTYPE* ExFuncPtr8)(void*, void*, void*, void*, void*, void*, void*, void*);
typedef void (CALLTYPE* ExFuncPtr9)(void*, void*, void*, void*, void*, void*, void*, void*, void*);
typedef void (CALLTYPE* ExFuncPtr10)(void*, void*, void*, void*, void*, void*, void*, void*, void*, void*);
typedef void (CALLTYPE* ExFuncPtr11)(void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*);
typedef void (CALLTYPE* ExFuncPtr12)(void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*);
typedef void (CALLTYPE* ExFuncPtr13)(void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*);
typedef void (CALLTYPE* ExFuncPtr14)(void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*);
typedef void (CALLTYPE* ExFuncPtr15)(void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*);
typedef void (CALLTYPE* ExFuncPtr16)(void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*);

typedef void (CALLTYPE* GetFuncCountPtr)(sal_uInt16& nCount);
typedef void (CALLTYPE* GetFuncDataPtr)
    (sal_uInt16& nNo, sal_Char* pFuncName, sal_uInt16& nParamCount, ParamType* peType, sal_Char* pInternalName);

typedef void (CALLTYPE* SetLanguagePtr)( sal_uInt16& nLanguage );
typedef void (CALLTYPE* GetParamDesc)
    (sal_uInt16& nNo, sal_uInt16& nParam, sal_Char* pName, sal_Char* pDesc );

typedef void (CALLTYPE* IsAsync) ( sal_uInt16&      nNo,
                                   ParamType*   peType );
typedef void (CALLTYPE* Advice)  ( sal_uInt16&      nNo,
                                   AdvData&     pfCallback );
typedef void (CALLTYPE* Unadvice)( double&      nHandle );

typedef void (CALLTYPE* FARPROC) ( void );

}

#ifndef DISABLE_DYNLOADING
#define GETFUNCTIONCOUNT        "GetFunctionCount"
#define GETFUNCTIONDATA         "GetFunctionData"
#define SETLANGUAGE             "SetLanguage"
#define GETPARAMDESC            "GetParameterDescription"
#define ISASYNC                 "IsAsync"
#define ADVICE                  "Advice"
#define UNADVICE                "Unadvice"
#endif

class ModuleData: private boost::noncopyable
{
friend class ModuleCollection;
    OUString aName;
    osl::Module* pInstance;
public:
    ModuleData(const OUString& rStr, osl::Module* pInst) : aName(rStr), pInstance(pInst) {}
    ~ModuleData() { delete pInstance; }

    const OUString& GetName() const { return aName; }
    osl::Module*    GetInstance() const { return pInstance; }
};

FuncData::FuncData(const ModuleData*pModule,
                   const OUString& rIName,
                   const OUString& rFName,
                         sal_uInt16 nNo,
                    sal_uInt16  nCount,
                   const ParamType* peType,
                    ParamType  eType) :
    pModuleData     (pModule),
    aInternalName   (rIName),
    aFuncName       (rFName),
    nNumber         (nNo),
    nParamCount     (nCount),
    eAsyncType      (eType)
{
    for (sal_uInt16 i = 0; i < MAXFUNCPARAM; i++)
        eParamType[i] = peType[i];
}

FuncData::FuncData(const FuncData& rData) :
    pModuleData     (rData.pModuleData),
    aInternalName   (rData.aInternalName),
    aFuncName       (rData.aFuncName),
    nNumber         (rData.nNumber),
    nParamCount     (rData.nParamCount),
    eAsyncType      (rData.eAsyncType)
{
    for (sal_uInt16 i = 0; i < MAXFUNCPARAM; i++)
        eParamType[i] = rData.eParamType[i];
}

namespace {

class ModuleCollection
{
    typedef boost::ptr_map<OUString, ModuleData> MapType;
    MapType maData;
public:
    ModuleCollection() {}

    const ModuleData* findByName(const OUString& rName) const;
    void insert(ModuleData* pNew);
    void clear();
};

const ModuleData* ModuleCollection::findByName(const OUString& rName) const
{
    MapType::const_iterator it = maData.find(rName);
    return it == maData.end() ? NULL : it->second;
}

void ModuleCollection::insert(ModuleData* pNew)
{
    if (!pNew)
        return;

    OUString aName = pNew->GetName();
    maData.insert(aName, pNew);
}

void ModuleCollection::clear()
{
    maData.clear();
}

ModuleCollection aModuleCollection;

}

bool InitExternalFunc(const OUString& rModuleName)
{
#ifdef DISABLE_DYNLOADING
    (void) rModuleName;
    return false;
#else
    // Module already loaded?
    const ModuleData* pTemp = aModuleCollection.findByName(rModuleName);
    if (pTemp)
        return false;

    OUString aNP;
    aNP = rModuleName;

    bool bRet = false;
    osl::Module* pLib = new osl::Module( aNP );
    if (pLib->is())
    {
        FARPROC fpGetCount = (FARPROC)pLib->getFunctionSymbol(GETFUNCTIONCOUNT);
        FARPROC fpGetData = (FARPROC)pLib->getFunctionSymbol(GETFUNCTIONDATA);
        if ((fpGetCount != NULL) && (fpGetData != NULL))
        {
            FARPROC fpIsAsync = (FARPROC)pLib->getFunctionSymbol(ISASYNC);
            FARPROC fpAdvice = (FARPROC)pLib->getFunctionSymbol(ADVICE);
            FARPROC fpSetLanguage = (FARPROC)pLib->getFunctionSymbol(SETLANGUAGE);
            if ( fpSetLanguage )
            {
                LanguageType eLanguage = Application::GetSettings().GetUILanguageTag().getLanguageType();
                sal_uInt16 nLanguage = (sal_uInt16) eLanguage;
                (*((SetLanguagePtr)fpSetLanguage))( nLanguage );
            }

            // Module in die Collection aufnehmen
            ModuleData* pModuleData = new ModuleData(rModuleName, pLib);
            aModuleCollection.insert(pModuleData);

            // Schnittstelle initialisieren
            AdvData pfCallBack = &ScAddInAsyncCallBack;
            FuncData* pFuncData;
            FuncCollection* pFuncCol = ScGlobal::GetFuncCollection();
            sal_uInt16 nCount;
            (*((GetFuncCountPtr)fpGetCount))(nCount);
            for (sal_uInt16 i=0; i < nCount; i++)
            {
                sal_Char cFuncName[256];
                sal_Char cInternalName[256];
                sal_uInt16 nParamCount;
                ParamType eParamType[MAXFUNCPARAM];
                ParamType eAsyncType = ParamType::NONE;
                // initialize all,  in case the AddIn behaves bad
                cFuncName[0] = 0;
                cInternalName[0] = 0;
                nParamCount = 0;
                for ( sal_uInt16 j=0; j<MAXFUNCPARAM; j++ )
                {
                    eParamType[j] = ParamType::NONE;
                }
                (*((GetFuncDataPtr)fpGetData))(i, cFuncName, nParamCount,
                                               eParamType, cInternalName);
                if( fpIsAsync )
                {
                    (*((IsAsync)fpIsAsync))(i, &eAsyncType);
                    if ( fpAdvice && eAsyncType != ParamType::NONE )
                        (*((Advice)fpAdvice))( i, pfCallBack );
                }
                OUString aInternalName( cInternalName, strlen(cInternalName), osl_getThreadTextEncoding() );
                OUString aFuncName( cFuncName, strlen(cFuncName), osl_getThreadTextEncoding() );
                pFuncData = new FuncData( pModuleData,
                                          aInternalName,
                                          aFuncName,
                                          i,
                                          nParamCount,
                                          eParamType,
                                          eAsyncType );
                pFuncCol->insert(pFuncData);
            }
            bRet = true;
        }
        else
            delete pLib;
    }
    else
        delete pLib;
    return bRet;
#endif
}

void ExitExternalFunc()
{
    aModuleCollection.clear();
}

bool FuncData::Call(void** ppParam) const
{
#ifdef DISABLE_DYNLOADING
    (void) ppParam;
    return false;
#else
    bool bRet = false;
    osl::Module* pLib = pModuleData->GetInstance();
    FARPROC fProc = (FARPROC)pLib->getFunctionSymbol(aFuncName);
    if (fProc != NULL)
    {
        switch (nParamCount)
        {
            case 1 :
                (*((ExFuncPtr1)fProc))(ppParam[0]);
                bRet = true;
                break;
            case 2 :
                (*((ExFuncPtr2)fProc))(ppParam[0], ppParam[1]);
                bRet = true;
                break;
            case 3 :
                (*((ExFuncPtr3)fProc))(ppParam[0], ppParam[1], ppParam[2]);
                bRet = true;
                break;
            case 4 :
                (*((ExFuncPtr4)fProc))(ppParam[0], ppParam[1], ppParam[2], ppParam[3]);
                bRet = true;
                break;
            case 5 :
                (*((ExFuncPtr5)fProc))(ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4]);
                bRet = true;
                break;
            case 6 :
                (*((ExFuncPtr6)fProc))(ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5]);
                bRet = true;
                break;
            case 7 :
                (*((ExFuncPtr7)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
                                        ppParam[6]);
                bRet = true;
                break;
            case 8 :
                (*((ExFuncPtr8)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
                                        ppParam[6], ppParam[7]);
                bRet = true;
                break;
            case 9 :
                (*((ExFuncPtr9)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
                                        ppParam[6], ppParam[7], ppParam[8]);
                bRet = true;
                break;
            case 10 :
                (*((ExFuncPtr10)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
                                        ppParam[6], ppParam[7], ppParam[8], ppParam[9]);
                bRet = true;
                break;
            case 11 :
                (*((ExFuncPtr11)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
                                        ppParam[6], ppParam[7], ppParam[8], ppParam[9], ppParam[10]);
                bRet = true;
                break;
            case 12:
                (*((ExFuncPtr12)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
                                        ppParam[6], ppParam[7], ppParam[8], ppParam[9], ppParam[10], ppParam[11]);
                bRet = true;
                break;
            case 13:
                (*((ExFuncPtr13)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
                                        ppParam[6], ppParam[7], ppParam[8], ppParam[9], ppParam[10], ppParam[11],
                                        ppParam[12]);
                bRet = true;
                break;
            case 14 :
                (*((ExFuncPtr14)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
                                        ppParam[6], ppParam[7], ppParam[8], ppParam[9], ppParam[10], ppParam[11],
                                        ppParam[12], ppParam[13]);
                bRet = true;
                break;
            case 15 :
                (*((ExFuncPtr15)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
                                        ppParam[6], ppParam[7], ppParam[8], ppParam[9], ppParam[10], ppParam[11],
                                        ppParam[12], ppParam[13], ppParam[14]);
                bRet = true;
                break;
            case 16 :
                (*((ExFuncPtr16)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
                                        ppParam[6], ppParam[7], ppParam[8], ppParam[9], ppParam[10], ppParam[11],
                                        ppParam[12], ppParam[13], ppParam[14], ppParam[15]);
                bRet = true;
                break;
            default : break;
        }
    }
    return bRet;
#endif
}

bool FuncData::Unadvice( double nHandle )
{
#ifdef DISABLE_DYNLOADING
    (void) nHandle;
    return false;
#else
    bool bRet = false;
    osl::Module* pLib = pModuleData->GetInstance();
    FARPROC fProc = (FARPROC)pLib->getFunctionSymbol(UNADVICE);
    if (fProc != NULL)
    {
        ((::Unadvice)fProc)(nHandle);
        bRet = true;
    }
    return bRet;
#endif
}

const OUString& FuncData::GetModuleName() const
{
    return pModuleData->GetName();
}

bool FuncData::getParamDesc( OUString& aName, OUString& aDesc, sal_uInt16 nParam ) const
{
#ifdef DISABLE_DYNLOADING
    (void) aName;
    (void) aDesc;
    (void) nParam;
    return false;
#else
    bool bRet = false;
    if ( nParam <= nParamCount )
    {
        osl::Module* pLib = pModuleData->GetInstance();
        FARPROC fProc = (FARPROC) pLib->getFunctionSymbol(GETPARAMDESC);
        if ( fProc != NULL )
        {
            sal_Char pcName[256];
            sal_Char pcDesc[256];
            *pcName = *pcDesc = 0;
            sal_uInt16 nFuncNo = nNumber;   // nicht per Reference versauen lassen..
            ((::GetParamDesc)fProc)( nFuncNo, nParam, pcName, pcDesc );
            aName = OUString( pcName, 256, osl_getThreadTextEncoding() );
            aDesc = OUString( pcDesc, 256, osl_getThreadTextEncoding() );
            bRet = true;
        }
    }
    if ( !bRet )
    {
        aName.clear();
        aDesc.clear();
    }
    return bRet;
#endif
}

FuncCollection::FuncCollection() {}
FuncCollection::FuncCollection(const FuncCollection& r) : maData(r.maData) {}

const FuncData* FuncCollection::findByName(const OUString& rName) const
{
    MapType::const_iterator it = maData.find(rName);
    return it == maData.end() ? NULL : it->second;
}

FuncData* FuncCollection::findByName(const OUString& rName)
{
    MapType::iterator it = maData.find(rName);
    return it == maData.end() ? NULL : it->second;
}

void FuncCollection::insert(FuncData* pNew)
{
    OUString aName = pNew->GetInternalName();
    maData.insert(aName, pNew);
}

FuncCollection::const_iterator FuncCollection::begin() const
{
    return maData.begin();
}

FuncCollection::const_iterator FuncCollection::end() const
{
    return maData.end();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
