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

#include <vcl/svapp.hxx>
#include <osl/module.hxx>
#include <osl/file.hxx>
#include <unotools/transliterationwrapper.hxx>

#include "callform.hxx"
#include "global.hxx"
#include "adiasync.hxx"

//------------------------------------------------------------------------

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

#define GETFUNCTIONCOUNT        "GetFunctionCount"
#define GETFUNCTIONDATA         "GetFunctionData"
#define SETLANGUAGE             "SetLanguage"
#define GETPARAMDESC            "GetParameterDescription"
#define ISASYNC                 "IsAsync"
#define ADVICE                  "Advice"
#define UNADVICE                "Unadvice"

class ModuleData
{
friend class ModuleCollection;
    rtl::OUString aName;
    osl::Module* pInstance;
public:
    ModuleData(const rtl::OUString& rStr, osl::Module* pInst) : aName(rStr), pInstance(pInst) {}
    ModuleData(const ModuleData& rData) : aName(rData.aName)
    {
#ifndef DISABLE_DYNLOADING
        pInstance = new osl::Module(aName);
#else
        pInstance = NULL;
#endif
    }
    ~ModuleData() { delete pInstance; }

    const rtl::OUString& GetName() const { return aName; }
    osl::Module*    GetInstance() const { return pInstance; }
    void            FreeInstance() { delete pInstance; pInstance = 0; }
};

//------------------------------------------------------------------------

FuncData::FuncData(const ModuleData*pModule,
                   const rtl::OUString& rIName,
                   const rtl::OUString& rFName,
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

//------------------------------------------------------------------------

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
    typedef boost::ptr_map<rtl::OUString, ModuleData> MapType;
    MapType maData;
public:
    ModuleCollection() {}
    ModuleCollection(const ModuleCollection& r) : maData(r.maData) {}

    const ModuleData* findByName(const rtl::OUString& rName) const;
    void insert(ModuleData* pNew);
    void clear();
};

const ModuleData* ModuleCollection::findByName(const rtl::OUString& rName) const
{
    MapType::const_iterator it = maData.find(rName);
    return it == maData.end() ? NULL : it->second;
}

void ModuleCollection::insert(ModuleData* pNew)
{
    if (!pNew)
        return;

    rtl::OUString aName = pNew->GetName();
    maData.insert(aName, pNew);
}

void ModuleCollection::clear()
{
    maData.clear();
}

ModuleCollection aModuleCollection;

}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

bool InitExternalFunc(const rtl::OUString& rModuleName)
{
#ifdef DISABLE_DYNLOADING
    (void) rModuleName;
    return false;
#else
    // Module already loaded?
    const ModuleData* pTemp = aModuleCollection.findByName(rModuleName);
    if (pTemp)
        return false;

    rtl::OUString aNP;
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
                LanguageType eLanguage = Application::GetSettings().GetUILanguage();
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
                ParamType eAsyncType = NONE;
                // initialize all,  in case the AddIn behaves bad
                cFuncName[0] = 0;
                cInternalName[0] = 0;
                nParamCount = 0;
                for ( sal_uInt16 j=0; j<MAXFUNCPARAM; j++ )
                {
                    eParamType[j] = NONE;
                }
                (*((GetFuncDataPtr)fpGetData))(i, cFuncName, nParamCount,
                                               eParamType, cInternalName);
                if( fpIsAsync )
                {
                    (*((IsAsync)fpIsAsync))(i, &eAsyncType);
                    if ( fpAdvice && eAsyncType != NONE )
                        (*((Advice)fpAdvice))( i, pfCallBack );
                }
                String aInternalName( cInternalName, osl_getThreadTextEncoding() );
                String aFuncName( cFuncName, osl_getThreadTextEncoding() );
                pFuncData = new FuncData( pModuleData,
                                          aInternalName,
                                          aFuncName,
                                          i,
                                          nParamCount,
                                          eParamType,
                                          eAsyncType );
                pFuncCol->insert(pFuncData);
            }
            bRet = sal_True;
        }
        else
            delete pLib;
    }
    else
        delete pLib;
    return bRet;
#endif
}

//------------------------------------------------------------------------

void ExitExternalFunc()
{
    aModuleCollection.clear();
}

//------------------------------------------------------------------------

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

//------------------------------------------------------------------------

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

//------------------------------------------------------------------------

const rtl::OUString& FuncData::GetModuleName() const
{
    return pModuleData->GetName();
}

bool FuncData::getParamDesc( ::rtl::OUString& aName, ::rtl::OUString& aDesc, sal_uInt16 nParam ) const
{
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
            aName = ::rtl::OUString( pcName, 256, osl_getThreadTextEncoding() );
            aDesc = ::rtl::OUString( pcDesc, 256, osl_getThreadTextEncoding() );
            bRet = true;
        }
    }
    if ( !bRet )
    {
        aName = ::rtl::OUString();
        aDesc = ::rtl::OUString();
    }
    return bRet;
}

FuncCollection::FuncCollection() {}
FuncCollection::FuncCollection(const FuncCollection& r) : maData(r.maData) {}

const FuncData* FuncCollection::findByName(const rtl::OUString& rName) const
{
    MapType::const_iterator it = maData.find(rName);
    return it == maData.end() ? NULL : it->second;
}

FuncData* FuncCollection::findByName(const rtl::OUString& rName)
{
    MapType::iterator it = maData.find(rName);
    return it == maData.end() ? NULL : it->second;
}

void FuncCollection::insert(FuncData* pNew)
{
    rtl::OUString aName = pNew->GetInternalName();
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
