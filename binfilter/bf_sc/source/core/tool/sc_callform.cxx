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

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------

#include <osl/module.hxx>

#include "adiasync.hxx"
#include <tools/debug.hxx>
namespace binfilter {


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

typedef void (CALLTYPE* IsAsync) ( USHORT&		nNo,
                                   ParamType*	peType );
typedef void (CALLTYPE* Advice)  ( USHORT&		nNo,
                                   AdvData&		pfCallback );
typedef void (CALLTYPE* Unadvice)( double&		nHandle );

typedef void (CALLTYPE* FARPROC) ( void );

}

#if defined(OS2) && defined(BLC)
#define GETFUNCTIONCOUNT		"_GetFunctionCount"
#define GETFUNCTIONDATA			"_GetFunctionData"
#define SETLANGUAGE				"_SetLanguage"
#define GETPARAMDESC			"_GetParameterDescription"
#define ISASYNC					"_IsAsync"
#define ADVICE					"_Advice"
#define UNADVICE				"_Unadvice"
#else // Pascal oder extern "C"
#define GETFUNCTIONCOUNT		"GetFunctionCount"
#define GETFUNCTIONDATA			"GetFunctionData"
#define SETLANGUAGE				"SetLanguage"
#define GETPARAMDESC			"GetParameterDescription"
#define ISASYNC					"IsAsync"
#define ADVICE					"Advice"
#define UNADVICE				"Unadvice"
#endif

#define LIBFUNCNAME( name ) \
    (String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( name ) ))

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/*N*/ FuncData::FuncData(const String& rIName) :
/*N*/ 	pModuleData		(NULL),
/*N*/ 	aInternalName   (rIName),
/*N*/ //  aFuncName		(""),
/*N*/ 	nNumber			(0),
/*N*/ 	nParamCount		(0),
/*N*/ 	eAsyncType		(NONE)
/*N*/ {
/*N*/ 	for (USHORT i = 0; i < MAXFUNCPARAM; i++)
/*N*/ 		eParamType[i] = PTR_DOUBLE;
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------

/*N*/ FuncData::FuncData(const FuncData& rData) :
/*N*/ 	pModuleData		(rData.pModuleData),
/*N*/ 	aInternalName   (rData.aInternalName),
/*N*/ 	aFuncName		(rData.aFuncName),
/*N*/ 	nNumber			(rData.nNumber),
/*N*/ 	nParamCount		(rData.nParamCount),
/*N*/ 	eAsyncType		(rData.eAsyncType)
/*N*/ {
/*N*/ 	for (USHORT i = 0; i < MAXFUNCPARAM; i++)
/*N*/ 		eParamType[i] = rData.eParamType[i];
/*N*/ }

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/*N*/ short FuncCollection::Compare(DataObject* pKey1, DataObject* pKey2) const
/*N*/ {
            DBG_BF_ASSERT(0, "STRIP"); return 0; //STRIP001 return (short) ScGlobal::pTransliteration->compareString(
/*N*/ }

//------------------------------------------------------------------------

/*N*/ BOOL FuncCollection::SearchFunc( const String& rName, USHORT& rIndex ) const
/*N*/ {
/*N*/ 	FuncData aDataObj(rName);
/*N*/ 	return Search( &aDataObj, rIndex );
/*N*/ }

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class ModuleData : public DataObject
{
    friend class ModuleCollection;
    String      aName;
    osl::Module* pInstance;
public:
    ModuleData(const String& rStr, osl::Module* pInst) : aName (rStr), pInstance (pInst) {}
    ModuleData(const ModuleData& rData) : aName (rData.aName) {pInstance = new osl::Module(aName);}
    ~ModuleData() { delete pInstance; }
    virtual DataObject* Clone() const { return new ModuleData(*this); }

    const   String&         GetName() const { return aName; }
    osl::Module*            GetInstance() const { return pInstance; }
    void            FreeInstance() { delete pInstance; pInstance = 0; }
};

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
/*N*/ class ModuleCollection : public SortedCollection
/*N*/ {
/*N*/ public:
/*N*/ 	ModuleCollection(USHORT nLim = 4, USHORT nDel = 4, BOOL bDup = FALSE) : SortedCollection ( nLim, nDel, bDup ) {}
/*N*/ 	ModuleCollection(const ModuleCollection& rModuleCollection) : SortedCollection ( rModuleCollection ) {}
/*N*/ 
/*N*/ 	virtual DataObject*		Clone() const { return new ModuleCollection(*this); }
/*N*/ 			ModuleData*		operator[]( const USHORT nIndex) const {return (ModuleData*)At(nIndex);}
    virtual short			Compare(DataObject* pKey1, DataObject* pKey2) const{DBG_BF_ASSERT(0, "STRIP"); return 0;} //STRIP001 virtual short			Compare(DataObject* pKey1, DataObject* pKey2) const;
/*N*/ };

/*N*/ #ifdef _MSC_VER
/*N*/ #pragma code_seg("SCSTATICS")
/*N*/ #endif
/*N*/ 
/*N*/ static ModuleCollection aModuleCollection;
/*N*/ 
/*N*/ #ifdef _MSC_VER
/*N*/ #pragma code_seg()
/*N*/ #endif

//------------------------------------------------------------------------


//------------------------------------------------------------------------


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


//------------------------------------------------------------------------

/*N*/ void ExitExternalFunc()
/*N*/ {
/*N*/ 	USHORT nCount = aModuleCollection.GetCount();
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 	{
/*?*/ 		ModuleData* pData = aModuleCollection[i];
/*?*/ 		pData->FreeInstance();
/*N*/ 	}
/*N*/ }

//------------------------------------------------------------------------

/*N*/ BOOL FuncData::Call(void** ppParam)
/*N*/ {
/*N*/ 	BOOL bRet = FALSE;
        osl::Module* pLib = pModuleData->GetInstance();
/*N*/ 	FARPROC fProc = (FARPROC)pLib->getSymbol(aFuncName);
/*N*/ 	if (fProc != NULL)
/*N*/ 	{
/*N*/ 		switch (nParamCount)
/*N*/ 		{
/*N*/ 			case 1 :
/*N*/ 				(*((ExFuncPtr1)fProc))(ppParam[0]);
/*N*/ 				bRet = TRUE;
/*N*/ 				break;
/*N*/ 			case 2 :
/*N*/ 				(*((ExFuncPtr2)fProc))(ppParam[0], ppParam[1]);
/*N*/ 				bRet = TRUE;
/*N*/ 				break;
/*N*/ 			case 3 :
/*N*/ 				(*((ExFuncPtr3)fProc))(ppParam[0], ppParam[1], ppParam[2]);
/*N*/ 				bRet = TRUE;
/*N*/ 				break;
/*N*/ 			case 4 :
/*N*/ 				(*((ExFuncPtr4)fProc))(ppParam[0], ppParam[1], ppParam[2], ppParam[3]);
/*N*/ 				bRet = TRUE;
/*N*/ 				break;
/*N*/ 			case 5 :
/*N*/ 				(*((ExFuncPtr5)fProc))(ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4]);
/*N*/ 				bRet = TRUE;
/*N*/ 				break;
/*N*/ 			case 6 :
/*N*/ 				(*((ExFuncPtr6)fProc))(ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5]);
/*N*/ 				bRet = TRUE;
/*N*/ 				break;
/*N*/ 			case 7 :
/*N*/ 				(*((ExFuncPtr7)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
/*N*/ 										ppParam[6]);
/*N*/ 				bRet = TRUE;
/*N*/ 				break;
/*N*/ 			case 8 :
/*N*/ 				(*((ExFuncPtr8)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
/*N*/ 										ppParam[6], ppParam[7]);
/*N*/ 				bRet = TRUE;
/*N*/ 				break;
/*N*/ 			case 9 :
/*N*/ 				(*((ExFuncPtr9)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
/*N*/ 										ppParam[6], ppParam[7], ppParam[8]);
/*N*/ 				bRet = TRUE;
/*N*/ 				break;
/*N*/ 			case 10 :
/*N*/ 				(*((ExFuncPtr10)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
/*N*/ 										ppParam[6], ppParam[7], ppParam[8], ppParam[9]);
/*N*/ 				bRet = TRUE;
/*N*/ 				break;
/*N*/ 			case 11 :
/*N*/ 				(*((ExFuncPtr11)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
/*N*/ 										ppParam[6], ppParam[7], ppParam[8], ppParam[9], ppParam[10]);
/*N*/ 				bRet = TRUE;
/*N*/ 				break;
/*N*/ 			case 12:
/*N*/ 				(*((ExFuncPtr12)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
/*N*/ 										ppParam[6], ppParam[7], ppParam[8], ppParam[9], ppParam[10], ppParam[11]);
/*N*/ 				bRet = TRUE;
/*N*/ 				break;
/*N*/ 			case 13:
/*N*/ 				(*((ExFuncPtr13)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
/*N*/ 										ppParam[6], ppParam[7], ppParam[8], ppParam[9], ppParam[10], ppParam[11],
/*N*/ 										ppParam[12]);
/*N*/ 				bRet = TRUE;
/*N*/ 				break;
/*N*/ 			case 14 :
/*N*/ 				(*((ExFuncPtr14)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
/*N*/ 										ppParam[6], ppParam[7], ppParam[8], ppParam[9], ppParam[10], ppParam[11],
/*N*/ 										ppParam[12], ppParam[13]);
/*N*/ 				bRet = TRUE;
/*N*/ 				break;
/*N*/ 			case 15 :
/*N*/ 				(*((ExFuncPtr15)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
/*N*/ 										ppParam[6], ppParam[7], ppParam[8], ppParam[9], ppParam[10], ppParam[11],
/*N*/ 										ppParam[12], ppParam[13], ppParam[14]);
/*N*/ 				bRet = TRUE;
/*N*/ 				break;
/*N*/ 			case 16 :
/*N*/ 				(*((ExFuncPtr16)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
/*N*/ 										ppParam[6], ppParam[7], ppParam[8], ppParam[9], ppParam[10], ppParam[11],
/*N*/ 										ppParam[12], ppParam[13], ppParam[14], ppParam[15]);
/*N*/ 				bRet = TRUE;
/*N*/ 				break;
/*N*/ 			default : break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
