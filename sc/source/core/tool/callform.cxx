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

#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <osl/module.hxx>
#include <osl/file.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <memory>

#include <callform.hxx>
#include <global.hxx>
#include <adiasync.hxx>

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

class ModuleData
{
friend class ModuleCollection;
    OUString const aName;
    std::unique_ptr<osl::Module> pInstance;
public:
    ModuleData(const ModuleData&) = delete;
    const ModuleData& operator=(const ModuleData&) = delete;

    ModuleData(const OUString& rStr, std::unique_ptr<osl::Module> pInst) : aName(rStr), pInstance(std::move(pInst)) {}

    const OUString& GetName() const { return aName; }
    osl::Module*    GetInstance() const { return pInstance.get(); }
};

LegacyFuncData::LegacyFuncData(const ModuleData*pModule,
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

LegacyFuncData::LegacyFuncData(const LegacyFuncData& rData) :
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
    typedef std::map<OUString, std::unique_ptr<ModuleData>> MapType;
    MapType m_Data;
public:
    ModuleCollection() {}

    const ModuleData* findByName(const OUString& rName) const;
    void insert(ModuleData* pNew);
    void clear();
};

const ModuleData* ModuleCollection::findByName(const OUString& rName) const
{
    MapType::const_iterator it = m_Data.find(rName);
    return it == m_Data.end() ? nullptr : it->second.get();
}

void ModuleCollection::insert(ModuleData* pNew)
{
    if (!pNew)
        return;

    OUString aName = pNew->GetName();
    m_Data.insert(std::make_pair(aName, std::unique_ptr<ModuleData>(pNew)));
}

void ModuleCollection::clear()
{
    m_Data.clear();
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

    std::unique_ptr<osl::Module> pLib(new osl::Module( aNP ));
    if (!pLib->is())
       return false;

    oslGenericFunction fpGetCount = pLib->getFunctionSymbol(GETFUNCTIONCOUNT);
    oslGenericFunction fpGetData = pLib->getFunctionSymbol(GETFUNCTIONDATA);
    if ((fpGetCount == nullptr) || (fpGetData == nullptr))
       return false;

    oslGenericFunction fpIsAsync = pLib->getFunctionSymbol(ISASYNC);
    oslGenericFunction fpAdvice = pLib->getFunctionSymbol(ADVICE);
    oslGenericFunction fpSetLanguage = pLib->getFunctionSymbol(SETLANGUAGE);
    if ( fpSetLanguage )
    {
        LanguageType eLanguage = Application::GetSettings().GetUILanguageTag().getLanguageType();
        sal_uInt16 nLanguage = static_cast<sal_uInt16>(eLanguage);
        (*reinterpret_cast<SetLanguagePtr>(fpSetLanguage))( nLanguage );
    }

    // include module into the collection
    ModuleData* pModuleData = new ModuleData(rModuleName, std::move(pLib));
    aModuleCollection.insert(pModuleData);

    // initialize interface
    AdvData pfCallBack = &ScAddInAsyncCallBack;
    LegacyFuncCollection* pLegacyFuncCol = ScGlobal::GetLegacyFuncCollection();
    sal_uInt16 nCount;
    (*reinterpret_cast<GetFuncCountPtr>(fpGetCount))(nCount);
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
        for (ParamType & rParamType : eParamType)
        {
            rParamType = ParamType::NONE;
        }
        (*reinterpret_cast<GetFuncDataPtr>(fpGetData))(i, cFuncName, nParamCount,
                                       eParamType, cInternalName);
        if( fpIsAsync )
        {
            (*reinterpret_cast<IsAsync>(fpIsAsync))(i, &eAsyncType);
            if ( fpAdvice && eAsyncType != ParamType::NONE )
                (*reinterpret_cast<Advice>(fpAdvice))( i, pfCallBack );
        }
        OUString aInternalName( cInternalName, strlen(cInternalName), osl_getThreadTextEncoding() );
        OUString aFuncName( cFuncName, strlen(cFuncName), osl_getThreadTextEncoding() );
        LegacyFuncData* pLegacyFuncData = new LegacyFuncData( pModuleData,
                                  aInternalName,
                                  aFuncName,
                                  i,
                                  nParamCount,
                                  eParamType,
                                  eAsyncType );
        pLegacyFuncCol->insert(pLegacyFuncData);
    }
    return true;
#endif
}

void ExitExternalFunc()
{
    aModuleCollection.clear();
}

void LegacyFuncData::Call(void** ppParam) const
{
#ifdef DISABLE_DYNLOADING
    (void) ppParam;
#else
    osl::Module* pLib = pModuleData->GetInstance();
    oslGenericFunction fProc = pLib->getFunctionSymbol(aFuncName);
    if (fProc != nullptr)
    {
        switch (nParamCount)
        {
            case 1 :
                (*reinterpret_cast<ExFuncPtr1>(fProc))(ppParam[0]);
                break;
            case 2 :
                (*reinterpret_cast<ExFuncPtr2>(fProc))(ppParam[0], ppParam[1]);
                break;
            case 3 :
                (*reinterpret_cast<ExFuncPtr3>(fProc))(ppParam[0], ppParam[1], ppParam[2]);
                break;
            case 4 :
                (*reinterpret_cast<ExFuncPtr4>(fProc))(ppParam[0], ppParam[1], ppParam[2], ppParam[3]);
                break;
            case 5 :
                (*reinterpret_cast<ExFuncPtr5>(fProc))(ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4]);
                break;
            case 6 :
                (*reinterpret_cast<ExFuncPtr6>(fProc))(ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5]);
                break;
            case 7 :
                (*reinterpret_cast<ExFuncPtr7>(fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
                                        ppParam[6]);
                break;
            case 8 :
                (*reinterpret_cast<ExFuncPtr8>(fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
                                        ppParam[6], ppParam[7]);
                break;
            case 9 :
                (*reinterpret_cast<ExFuncPtr9>(fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
                                        ppParam[6], ppParam[7], ppParam[8]);
                break;
            case 10 :
                (*reinterpret_cast<ExFuncPtr10>(fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
                                        ppParam[6], ppParam[7], ppParam[8], ppParam[9]);
                break;
            case 11 :
                (*reinterpret_cast<ExFuncPtr11>(fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
                                        ppParam[6], ppParam[7], ppParam[8], ppParam[9], ppParam[10]);
                break;
            case 12:
                (*reinterpret_cast<ExFuncPtr12>(fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
                                        ppParam[6], ppParam[7], ppParam[8], ppParam[9], ppParam[10], ppParam[11]);
                break;
            case 13:
                (*reinterpret_cast<ExFuncPtr13>(fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
                                        ppParam[6], ppParam[7], ppParam[8], ppParam[9], ppParam[10], ppParam[11],
                                        ppParam[12]);
                break;
            case 14 :
                (*reinterpret_cast<ExFuncPtr14>(fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
                                        ppParam[6], ppParam[7], ppParam[8], ppParam[9], ppParam[10], ppParam[11],
                                        ppParam[12], ppParam[13]);
                break;
            case 15 :
                (*reinterpret_cast<ExFuncPtr15>(fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
                                        ppParam[6], ppParam[7], ppParam[8], ppParam[9], ppParam[10], ppParam[11],
                                        ppParam[12], ppParam[13], ppParam[14]);
                break;
            case 16 :
                (*reinterpret_cast<ExFuncPtr16>(fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
                                        ppParam[6], ppParam[7], ppParam[8], ppParam[9], ppParam[10], ppParam[11],
                                        ppParam[12], ppParam[13], ppParam[14], ppParam[15]);
                break;
            default : break;
        }
    }
#endif
}

void LegacyFuncData::Unadvice( double nHandle )
{
#ifdef DISABLE_DYNLOADING
    (void) nHandle;
#else
    osl::Module* pLib = pModuleData->GetInstance();
    oslGenericFunction fProc = pLib->getFunctionSymbol(UNADVICE);
    if (fProc != nullptr)
    {
        reinterpret_cast< ::Unadvice>(fProc)(nHandle);
    }
#endif
}

const OUString& LegacyFuncData::GetModuleName() const
{
    return pModuleData->GetName();
}

void LegacyFuncData::getParamDesc( OUString& aName, OUString& aDesc, sal_uInt16 nParam ) const
{
#ifdef DISABLE_DYNLOADING
    (void) aName;
    (void) aDesc;
    (void) nParam;
#else
    bool bRet = false;
    if ( nParam <= nParamCount )
    {
        osl::Module* pLib = pModuleData->GetInstance();
        oslGenericFunction fProc = pLib->getFunctionSymbol(GETPARAMDESC);
        if ( fProc != nullptr )
        {
            sal_Char pcName[256];
            sal_Char pcDesc[256];
            *pcName = *pcDesc = 0;
            sal_uInt16 nFuncNo = nNumber;   // don't let it mess up via reference...
            reinterpret_cast< ::GetParamDesc>(fProc)( nFuncNo, nParam, pcName, pcDesc );
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
#endif
}

LegacyFuncCollection::LegacyFuncCollection() {}
LegacyFuncCollection::LegacyFuncCollection(const LegacyFuncCollection& r)
{
    for (auto const& it : r.m_Data)
    {
        m_Data.insert(std::make_pair(it.first, std::make_unique<LegacyFuncData>(*it.second)));
    }
}

const LegacyFuncData* LegacyFuncCollection::findByName(const OUString& rName) const
{
    MapType::const_iterator it = m_Data.find(rName);
    return it == m_Data.end() ? nullptr : it->second.get();
}

LegacyFuncData* LegacyFuncCollection::findByName(const OUString& rName)
{
    MapType::iterator it = m_Data.find(rName);
    return it == m_Data.end() ? nullptr : it->second.get();
}

void LegacyFuncCollection::insert(LegacyFuncData* pNew)
{
    OUString aName = pNew->GetInternalName();
    m_Data.insert(std::make_pair(aName, std::unique_ptr<LegacyFuncData>(pNew)));
}

LegacyFuncCollection::const_iterator LegacyFuncCollection::begin() const
{
    return m_Data.begin();
}

LegacyFuncCollection::const_iterator LegacyFuncCollection::end() const
{
    return m_Data.end();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
