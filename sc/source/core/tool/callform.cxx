/*************************************************************************
 *
 *  $RCSfile: callform.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: er $ $Date: 2001-03-14 16:02:54 $
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

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include <tools/intn.hxx>
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_SYSTEM_HXX //autogen
#include <vcl/system.hxx>
#endif
#ifndef _VOS_MODULE_HXX_
#include <vos/module.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#include <unotools/collatorwrapper.hxx>

#include "callform.hxx"
#include "global.hxx"
#include "adiasync.hxx"

using namespace ::vos;

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

#if defined(OS2) && defined(BLC)
#define GETFUNCTIONCOUNT        "_GetFunctionCount"
#define GETFUNCTIONDATA         "_GetFunctionData"
#define SETLANGUAGE             "_SetLanguage"
#define GETPARAMDESC            "_GetParameterDescription"
#define ISASYNC                 "_IsAsync"
#define ADVICE                  "_Advice"
#define UNADVICE                "_Unadvice"
#else // Pascal oder extern "C"
#define GETFUNCTIONCOUNT        "GetFunctionCount"
#define GETFUNCTIONDATA         "GetFunctionData"
#define SETLANGUAGE             "SetLanguage"
#define GETPARAMDESC            "GetParameterDescription"
#define ISASYNC                 "IsAsync"
#define ADVICE                  "Advice"
#define UNADVICE                "Unadvice"
#endif

#define LIBFUNCNAME( name ) \
    (String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( name ) ))

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

FuncData::FuncData(const String& rIName) :
    pModuleData     (NULL),
    aInternalName   (rIName),
//  aFuncName       (""),
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

short FuncCollection::Compare(DataObject* pKey1, DataObject* pKey2) const
{
    return (short) ScGlobal::pCollator->compareString(
        ((FuncData*)pKey1)->aInternalName, ((FuncData*)pKey2)->aInternalName );
}

//------------------------------------------------------------------------

BOOL FuncCollection::SearchFunc( const String& rName, USHORT& rIndex ) const
{
    FuncData aDataObj(rName);
    return Search( &aDataObj, rIndex );
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class ModuleData : public DataObject
{
friend class ModuleCollection;
    String      aName;
    OModule*    pInstance;
public:
    ModuleData(const String& rStr, OModule* pInst) : aName (rStr), pInstance (pInst) {}
    ModuleData(const ModuleData& rData) : aName (rData.aName) {pInstance = new OModule(aName);}
    ~ModuleData() { delete pInstance; }
    virtual DataObject* Clone() const { return new ModuleData(*this); }

    const   String&         GetName() const { return aName; }
            OModule*        GetInstance() const { return pInstance; }
            void            FreeInstance() { delete pInstance; pInstance = 0; }
};

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class ModuleCollection : public SortedCollection
{
public:
    ModuleCollection(USHORT nLim = 4, USHORT nDel = 4, BOOL bDup = FALSE) : SortedCollection ( nLim, nDel, bDup ) {}
    ModuleCollection(const ModuleCollection& rModuleCollection) : SortedCollection ( rModuleCollection ) {}

    virtual DataObject*     Clone() const { return new ModuleCollection(*this); }
            ModuleData*     operator[]( const USHORT nIndex) const {return (ModuleData*)At(nIndex);}
    virtual short           Compare(DataObject* pKey1, DataObject* pKey2) const;
            BOOL            SearchModule( const String& rName,
                                          const ModuleData*& rpModule ) const;
};

#pragma code_seg("SCSTATICS")

static ModuleCollection aModuleCollection;

#pragma code_seg()

//------------------------------------------------------------------------

short ModuleCollection::Compare(DataObject* pKey1, DataObject* pKey2) const
{
    return (short) ScGlobal::pCollator->compareString(
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
    if ( osl::FileBase::getNormalizedPathFromFileURL( rModuleName, aNP ) != osl::FileBase::E_None )
        return FALSE;

    BOOL bRet = FALSE;
    OModule* pLib = new OModule( aNP );
    if (pLib->isLoaded())
    {
        FARPROC fpGetCount = (FARPROC)pLib->getSymbol(LIBFUNCNAME(GETFUNCTIONCOUNT));
        FARPROC fpGetData = (FARPROC)pLib->getSymbol(LIBFUNCNAME(GETFUNCTIONDATA));
        if ((fpGetCount != NULL) && (fpGetData != NULL))
        {
            FARPROC fpIsAsync = (FARPROC)pLib->getSymbol(LIBFUNCNAME(ISASYNC));
            FARPROC fpAdvice = (FARPROC)pLib->getSymbol(LIBFUNCNAME(ADVICE));
            FARPROC fpSetLanguage = (FARPROC)pLib->getSymbol(LIBFUNCNAME(SETLANGUAGE));
            if ( fpSetLanguage )
            {
                LanguageType eLanguage =
                    Application::GetAppInternational().GetLanguage();
                if ( (eLanguage == LANGUAGE_SYSTEM) ||
                        (eLanguage == LANGUAGE_DONTKNOW) )
                    eLanguage = System::GetLanguage();
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
                // #62113# alles initialisieren, falls das AddIn sich schlecht verhaelt
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
    OModule* pLib = pModuleData->GetInstance();
    FARPROC fProc = (FARPROC)pLib->getSymbol(aFuncName);
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

BOOL FuncData::Advice( AdvData pfCallback )
{
    BOOL bRet = FALSE;
    OModule* pLib = pModuleData->GetInstance();
    FARPROC fProc = (FARPROC)pLib->getSymbol(LIBFUNCNAME(ADVICE));
    if (fProc != NULL)
    {
        ((::Advice)fProc)(nNumber, pfCallback);
        bRet = TRUE;
    }
    return bRet;
}

//------------------------------------------------------------------------

BOOL FuncData::Unadvice( double nHandle )
{
    BOOL bRet = FALSE;
    OModule* pLib = pModuleData->GetInstance();
    FARPROC fProc = (FARPROC)pLib->getSymbol(LIBFUNCNAME(UNADVICE));
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
    // DBG_ASSERT( pModuleData, "Keine Arme, keine Kekse" ):
    return pModuleData->GetName();
}

//------------------------------------------------------------------------

BOOL FuncData::GetParamDesc( String& aName, String& aDesc, USHORT nParam )
{
    BOOL bRet = FALSE;
    if ( nParam <= nParamCount )
    {
        OModule* pLib = pModuleData->GetInstance();
        FARPROC fProc = (FARPROC) pLib->getSymbol( LIBFUNCNAME(GETPARAMDESC) );
        if ( fProc != NULL )
        {
            sal_Char pcName[256];
            sal_Char pcDesc[256];
            *pcName = *pcDesc = 0;
            USHORT nFuncNo = nNumber;   // nicht per Reference versauen lassen..
            ((::GetParamDesc)fProc)( nFuncNo, nParam, pcName, pcDesc );
            aName = String( pcName, osl_getThreadTextEncoding() );
            aDesc = String( pcDesc, osl_getThreadTextEncoding() );
            bRet = TRUE;
        }
    }
    if ( !bRet )
    {
        aName.Erase();
        aDesc.Erase();
    }
    return bRet;
}


