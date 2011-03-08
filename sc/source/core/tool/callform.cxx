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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------
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

typedef void (CALLTYPE* GetFuncCountPtr)(USHORT& nCount);
typedef void (CALLTYPE* GetFuncDataPtr)
    (USHORT& nNo, sal_Char* pFuncName, USHORT& nParamCount, ParamType* peType, sal_Char* pInternalName);

typedef void (CALLTYPE* SetLanguagePtr)( USHORT& nLanguage );
typedef void (CALLTYPE* GetParamDesc)
    (USHORT& nNo, USHORT& nParam, sal_Char* pName, sal_Char* pDesc );

typedef void (CALLTYPE* IsAsync) ( USHORT&      nNo,
                                   ParamType*   peType );
typedef void (CALLTYPE* Advice)  ( USHORT&      nNo,
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

#define LIBFUNCNAME( name ) \
    (String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( name ) ))

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

FuncData::FuncData(const String& rIName) :
    pModuleData     (NULL),
    aInternalName   (rIName),
    nNumber         (0),
    nParamCount     (0),
    eAsyncType      (NONE)
{
    for (USHORT i = 0; i < MAXFUNCPARAM; i++)
        eParamType[i] = PTR_DOUBLE;
}

//------------------------------------------------------------------------

FuncData::FuncData(const ModuleData*pModule,
                   const String&    rIName,
                   const String&    rFName,
                         USHORT nNo,
                    USHORT  nCount,
                   const ParamType* peType,
                    ParamType  eType) :
    pModuleData     (pModule),
    aInternalName   (rIName),
    aFuncName       (rFName),
    nNumber         (nNo),
    nParamCount     (nCount),
    eAsyncType      (eType)
{
    for (USHORT i = 0; i < MAXFUNCPARAM; i++)
        eParamType[i] = peType[i];
}

//------------------------------------------------------------------------

FuncData::FuncData(const FuncData& rData) :
    ScDataObject(),
    pModuleData     (rData.pModuleData),
    aInternalName   (rData.aInternalName),
    aFuncName       (rData.aFuncName),
    nNumber         (rData.nNumber),
    nParamCount     (rData.nParamCount),
    eAsyncType      (rData.eAsyncType)
{
    for (USHORT i = 0; i < MAXFUNCPARAM; i++)
        eParamType[i] = rData.eParamType[i];
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

short FuncCollection::Compare(ScDataObject* pKey1, ScDataObject* pKey2) const
{
    return (short) ScGlobal::GetpTransliteration()->compareString(
        ((FuncData*)pKey1)->aInternalName, ((FuncData*)pKey2)->aInternalName );
}

//------------------------------------------------------------------------

BOOL FuncCollection::SearchFunc( const String& rName, USHORT& rIndex ) const
{
    FuncData aDataObj(rName);
    return Search( &aDataObj, rIndex );
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class ModuleData : public ScDataObject
{
friend class ModuleCollection;
    String      aName;
    osl::Module* pInstance;
public:
    ModuleData(const String& rStr, osl::Module* pInst) : aName (rStr), pInstance (pInst) {}
    ModuleData(const ModuleData& rData) : ScDataObject(), aName (rData.aName) {pInstance = new osl::Module(aName);}
    ~ModuleData() { delete pInstance; }
    virtual ScDataObject*   Clone() const { return new ModuleData(*this); }

    const   String&         GetName() const { return aName; }
            osl::Module*    GetInstance() const { return pInstance; }
            void            FreeInstance() { delete pInstance; pInstance = 0; }
};

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class ModuleCollection : public ScSortedCollection
{
public:
    ModuleCollection(USHORT nLim = 4, USHORT nDel = 4, BOOL bDup = FALSE) : ScSortedCollection ( nLim, nDel, bDup ) {}
    ModuleCollection(const ModuleCollection& rModuleCollection) : ScSortedCollection ( rModuleCollection ) {}

    virtual ScDataObject*       Clone() const { return new ModuleCollection(*this); }
            ModuleData*     operator[]( const USHORT nIndex) const {return (ModuleData*)At(nIndex);}
    virtual short           Compare(ScDataObject* pKey1, ScDataObject* pKey2) const;
            BOOL            SearchModule( const String& rName,
                                          const ModuleData*& rpModule ) const;
};

static ModuleCollection aModuleCollection;

//------------------------------------------------------------------------

short ModuleCollection::Compare(ScDataObject* pKey1, ScDataObject* pKey2) const
{
    return (short) ScGlobal::GetpTransliteration()->compareString(
        ((ModuleData*)pKey1)->aName, ((ModuleData*)pKey2)->aName );
}

//------------------------------------------------------------------------

BOOL ModuleCollection::SearchModule( const String& rName,
                                     const ModuleData*& rpModule ) const
{
    USHORT nIndex;
    ModuleData aSearchModule(rName, 0);
    BOOL bFound = Search( &aSearchModule, nIndex );
    if (bFound)
        rpModule = (ModuleData*)At(nIndex);
    else
        rpModule = 0;
    return bFound;
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

BOOL InitExternalFunc(const rtl::OUString& rModuleName)
{
    String aModuleName( rModuleName );

    // Module schon geladen?
    const ModuleData* pTemp;
    if (aModuleCollection.SearchModule(aModuleName, pTemp))
        return FALSE;

    rtl::OUString aNP;
    aNP = rModuleName;

    BOOL bRet = FALSE;
    osl::Module* pLib = new osl::Module( aNP );
    if (pLib->is())
    {
        FARPROC fpGetCount = (FARPROC)pLib->getFunctionSymbol(LIBFUNCNAME(GETFUNCTIONCOUNT));
        FARPROC fpGetData = (FARPROC)pLib->getFunctionSymbol(LIBFUNCNAME(GETFUNCTIONDATA));
        if ((fpGetCount != NULL) && (fpGetData != NULL))
        {
            FARPROC fpIsAsync = (FARPROC)pLib->getFunctionSymbol(LIBFUNCNAME(ISASYNC));
            FARPROC fpAdvice = (FARPROC)pLib->getFunctionSymbol(LIBFUNCNAME(ADVICE));
            FARPROC fpSetLanguage = (FARPROC)pLib->getFunctionSymbol(LIBFUNCNAME(SETLANGUAGE));
            if ( fpSetLanguage )
            {
                LanguageType eLanguage = Application::GetSettings().GetUILanguage();
                USHORT nLanguage = (USHORT) eLanguage;
                (*((SetLanguagePtr)fpSetLanguage))( nLanguage );
            }

            // Module in die Collection aufnehmen
            ModuleData* pModuleData = new ModuleData(aModuleName, pLib);
            aModuleCollection.Insert(pModuleData);

            // Schnittstelle initialisieren
            AdvData pfCallBack = &ScAddInAsyncCallBack;
            FuncData* pFuncData;
            FuncCollection* pFuncCol = ScGlobal::GetFuncCollection();
            USHORT nCount;
            (*((GetFuncCountPtr)fpGetCount))(nCount);
            for (USHORT i=0; i < nCount; i++)
            {
                sal_Char cFuncName[256];
                sal_Char cInternalName[256];
                USHORT nParamCount;
                ParamType eParamType[MAXFUNCPARAM];
                ParamType eAsyncType = NONE;
                // alles initialisieren, falls das AddIn sich schlecht verhaelt
                cFuncName[0] = 0;
                cInternalName[0] = 0;
                nParamCount = 0;
                for ( USHORT j=0; j<MAXFUNCPARAM; j++ )
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
                pFuncCol->Insert(pFuncData);
            }
            bRet = TRUE;
        }
        else
            delete pLib;
    }
    else
        delete pLib;
    return bRet;
}

//------------------------------------------------------------------------

void ExitExternalFunc()
{
    USHORT nCount = aModuleCollection.GetCount();
    for (USHORT i=0; i<nCount; i++)
    {
        ModuleData* pData = aModuleCollection[i];
        pData->FreeInstance();
    }
}

//------------------------------------------------------------------------

BOOL FuncData::Call(void** ppParam)
{
    BOOL bRet = FALSE;
    osl::Module* pLib = pModuleData->GetInstance();
    FARPROC fProc = (FARPROC)pLib->getFunctionSymbol(aFuncName);
    if (fProc != NULL)
    {
        switch (nParamCount)
        {
            case 1 :
                (*((ExFuncPtr1)fProc))(ppParam[0]);
                bRet = TRUE;
                break;
            case 2 :
                (*((ExFuncPtr2)fProc))(ppParam[0], ppParam[1]);
                bRet = TRUE;
                break;
            case 3 :
                (*((ExFuncPtr3)fProc))(ppParam[0], ppParam[1], ppParam[2]);
                bRet = TRUE;
                break;
            case 4 :
                (*((ExFuncPtr4)fProc))(ppParam[0], ppParam[1], ppParam[2], ppParam[3]);
                bRet = TRUE;
                break;
            case 5 :
                (*((ExFuncPtr5)fProc))(ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4]);
                bRet = TRUE;
                break;
            case 6 :
                (*((ExFuncPtr6)fProc))(ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5]);
                bRet = TRUE;
                break;
            case 7 :
                (*((ExFuncPtr7)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
                                        ppParam[6]);
                bRet = TRUE;
                break;
            case 8 :
                (*((ExFuncPtr8)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
                                        ppParam[6], ppParam[7]);
                bRet = TRUE;
                break;
            case 9 :
                (*((ExFuncPtr9)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
                                        ppParam[6], ppParam[7], ppParam[8]);
                bRet = TRUE;
                break;
            case 10 :
                (*((ExFuncPtr10)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
                                        ppParam[6], ppParam[7], ppParam[8], ppParam[9]);
                bRet = TRUE;
                break;
            case 11 :
                (*((ExFuncPtr11)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
                                        ppParam[6], ppParam[7], ppParam[8], ppParam[9], ppParam[10]);
                bRet = TRUE;
                break;
            case 12:
                (*((ExFuncPtr12)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
                                        ppParam[6], ppParam[7], ppParam[8], ppParam[9], ppParam[10], ppParam[11]);
                bRet = TRUE;
                break;
            case 13:
                (*((ExFuncPtr13)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
                                        ppParam[6], ppParam[7], ppParam[8], ppParam[9], ppParam[10], ppParam[11],
                                        ppParam[12]);
                bRet = TRUE;
                break;
            case 14 :
                (*((ExFuncPtr14)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
                                        ppParam[6], ppParam[7], ppParam[8], ppParam[9], ppParam[10], ppParam[11],
                                        ppParam[12], ppParam[13]);
                bRet = TRUE;
                break;
            case 15 :
                (*((ExFuncPtr15)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
                                        ppParam[6], ppParam[7], ppParam[8], ppParam[9], ppParam[10], ppParam[11],
                                        ppParam[12], ppParam[13], ppParam[14]);
                bRet = TRUE;
                break;
            case 16 :
                (*((ExFuncPtr16)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
                                        ppParam[6], ppParam[7], ppParam[8], ppParam[9], ppParam[10], ppParam[11],
                                        ppParam[12], ppParam[13], ppParam[14], ppParam[15]);
                bRet = TRUE;
                break;
            default : break;
        }
    }
    return bRet;
}

//------------------------------------------------------------------------

BOOL FuncData::Unadvice( double nHandle )
{
    BOOL bRet = FALSE;
    osl::Module* pLib = pModuleData->GetInstance();
    FARPROC fProc = (FARPROC)pLib->getFunctionSymbol(LIBFUNCNAME(UNADVICE));
    if (fProc != NULL)
    {
        ((::Unadvice)fProc)(nHandle);
        bRet = TRUE;
    }
    return bRet;
}

//------------------------------------------------------------------------

const String& FuncData::GetModuleName() const
{
    return pModuleData->GetName();
}

bool FuncData::getParamDesc( ::rtl::OUString& aName, ::rtl::OUString& aDesc, sal_uInt16 nParam )
{
    bool bRet = false;
    if ( nParam <= nParamCount )
    {
        osl::Module* pLib = pModuleData->GetInstance();
        FARPROC fProc = (FARPROC) pLib->getFunctionSymbol( LIBFUNCNAME(GETPARAMDESC) );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
