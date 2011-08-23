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

#include <comphelper/processfactory.hxx>
#include <tools/debug.hxx>
#include <i18npool/mslangid.hxx>
#include <vcl/svapp.hxx>
#include <bf_sfx2/objsh.hxx>
#include <unotools/charclass.hxx>

#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/reflection/XIdlClass.hpp>
#include <com/sun/star/reflection/XIdlClassProvider.hpp>
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#include <com/sun/star/beans/XIntrospection.hpp>
#include <com/sun/star/beans/MethodConcept.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/sheet/XCompatibilityNames.hpp>

#include "addincol.hxx"
#include "addinhelpid.hxx"
#include "compiler.hxx"
#include "scmatrix.hxx"
#include "addinlis.hxx"
#include "scfuncs.hrc"
#include <legacysmgr/legacy_binfilters_smgr.hxx>
namespace binfilter {
using namespace ::com::sun::star;

//------------------------------------------------------------------------

#define SC_CALLERPOS_NONE   (-1)

#define SCADDINSUPPLIER_SERVICE "com.sun.star.sheet.AddIn"

//------------------------------------------------------------------------


#define SC_FUNCGROUP_COUNT  ID_FUNCTION_GRP_ADDINS

static const sal_Char* __FAR_DATA aFuncNames[SC_FUNCGROUP_COUNT] =
    {
        //  array index = ID - 1 (ID starts at 1)
        //  all upper case
        "Database",         // ID_FUNCTION_GRP_DATABASE
        "Date&Time",        // ID_FUNCTION_GRP_DATETIME
        "Financial",        // ID_FUNCTION_GRP_FINANZ
        "Information",      // ID_FUNCTION_GRP_INFO
        "Logical",          // ID_FUNCTION_GRP_LOGIC
        "Mathematical",     // ID_FUNCTION_GRP_MATH
        "Matrix",           // ID_FUNCTION_GRP_MATRIX
        "Statistical",      // ID_FUNCTION_GRP_STATISTIC
        "Spreadsheet",      // ID_FUNCTION_GRP_TABLE
        "Text",             // ID_FUNCTION_GRP_TEXT
        "Add-In"            // ID_FUNCTION_GRP_ADDINS
    };


//------------------------------------------------------------------------

struct ScAddInArgDesc
{
    String              aName;
    String              aDescription;
    ScAddInArgumentType eType;
    BOOL                bOptional;
};

class ScUnoAddInFuncData
{
private:
    String              aOriginalName;      // kept in formula
    String              aLocalName;         // for display
    String              aUpperName;         // for entering formulas
    String              aUpperLocal;        // for entering formulas
    String              aDescription;
    uno::Reference<reflection::XIdlMethod> xFunction;
    uno::Any            aObject;
    long                nArgCount;
    ScAddInArgDesc*     pArgDescs;
    long                nCallerPos;
    USHORT              nCategory;
    USHORT              nHelpId;
    mutable uno::Sequence<sheet::LocalizedName> aCompNames;
    mutable BOOL        bCompInitialized;

public:
                ScUnoAddInFuncData( const String& rNam, const String& rLoc,
                                    const String& rDesc,
                                    USHORT nCat, USHORT nHelp,
                                    const uno::Reference<reflection::XIdlMethod>& rFunc,
                                    const uno::Any& rO,
                                    long nAC, const ScAddInArgDesc* pAD,
                                    long nCP );
                ~ScUnoAddInFuncData();

    const String&           GetOriginalName() const     { return aOriginalName; }
    const String&           GetLocalName() const        { return aLocalName; }
    const String&           GetUpperName() const        { return aUpperName; }
    const String&           GetUpperLocal() const       { return aUpperLocal; }
    const uno::Reference<reflection::XIdlMethod>& GetFunction() const
                                                        { return xFunction; }
    const uno::Any&         GetObject() const           { return aObject; }
    long                    GetArgumentCount() const    { return nArgCount; }
    const ScAddInArgDesc*   GetArguments() const        { return pArgDescs; }
    long                    GetCallerPos() const        { return nCallerPos; }
    const String&           GetDescription() const      { return aDescription; }
    USHORT                  GetCategory() const         { return nCategory; }
    USHORT                  GetHelpId() const           { return nHelpId; }

    const uno::Sequence<sheet::LocalizedName>& GetCompNames() const;
};

//------------------------------------------------------------------------

/*N*/ ScUnoAddInFuncData::ScUnoAddInFuncData( const String& rNam, const String& rLoc,
/*N*/                                         const String& rDesc,
/*N*/                                         USHORT nCat, USHORT nHelp,
/*N*/                                         const uno::Reference<reflection::XIdlMethod>& rFunc,
/*N*/                                         const uno::Any& rO,
/*N*/                                         long nAC, const ScAddInArgDesc* pAD,
/*N*/                                         long nCP ) :
/*N*/     aOriginalName( rNam ),
/*N*/     aUpperName( rNam ),
/*N*/     aLocalName( rLoc ),
/*N*/     aUpperLocal( rLoc ),
/*N*/     aDescription( rDesc ),
/*N*/     nCategory( nCat ),
/*N*/     nHelpId( nHelp ),
/*N*/     xFunction( rFunc ),
/*N*/     aObject( rO ),
/*N*/     nArgCount( nAC ),
/*N*/     nCallerPos( nCP ),
/*N*/     bCompInitialized( FALSE )
/*N*/ {
/*N*/     if ( nArgCount )
/*N*/     {
/*N*/         pArgDescs = new ScAddInArgDesc[nArgCount];
/*N*/         for (long i=0; i<nArgCount; i++)
/*N*/             pArgDescs[i] = pAD[i];
/*N*/     }
/*N*/     else
/*N*/         pArgDescs = NULL;
/*N*/ 
/*N*/     ScGlobal::pCharClass->toUpper(aUpperName);
/*N*/     ScGlobal::pCharClass->toUpper(aUpperLocal);
/*N*/ }

/*N*/ ScUnoAddInFuncData::~ScUnoAddInFuncData()
/*N*/ {
/*N*/     delete[] pArgDescs;
/*N*/ }


//------------------------------------------------------------------------

/*N*/ BOOL lcl_ConvertToDouble( const uno::Any& rAny, double& rOut )
/*N*/ {
/*N*/     BOOL bRet = FALSE;
/*N*/     uno::TypeClass eClass = rAny.getValueTypeClass();
/*N*/     switch (eClass)
/*N*/     {
/*N*/         //! extract integer values
/*N*/         case uno::TypeClass_ENUM:
/*N*/         case uno::TypeClass_BOOLEAN:
/*N*/         case uno::TypeClass_CHAR:
/*N*/         case uno::TypeClass_BYTE:
/*N*/         case uno::TypeClass_SHORT:
/*N*/         case uno::TypeClass_UNSIGNED_SHORT:
/*N*/         case uno::TypeClass_LONG:
/*N*/         case uno::TypeClass_UNSIGNED_LONG:
/*N*/         case uno::TypeClass_FLOAT:
/*N*/         case uno::TypeClass_DOUBLE:
/*N*/             rAny >>= rOut;
/*N*/             bRet = TRUE;
/*N*/             break;
/*N*/     }
/*N*/     if (!bRet)
/*N*/         rOut = 0.0;
/*N*/     return bRet;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScUnoAddInCollection::ScUnoAddInCollection() :
/*N*/     nFuncCount( 0 ),
/*N*/     ppFuncData( NULL ),
/*N*/     pExactHashMap( NULL ),
/*N*/     pNameHashMap( NULL ),
/*N*/     pLocalHashMap( NULL ),
/*N*/     bInitialized( FALSE )
/*N*/ {
/*N*/ }

/*N*/ ScUnoAddInCollection::~ScUnoAddInCollection()
/*N*/ {
/*N*/     delete pExactHashMap;
/*N*/     delete pNameHashMap;
/*N*/     delete pLocalHashMap;
/*N*/     if ( ppFuncData )
/*N*/     {
/*N*/         for ( long i=0; i<nFuncCount; i++ )
/*N*/             delete ppFuncData[i];
/*N*/         delete[] ppFuncData;
/*N*/     }
/*N*/ }

/*N*/ void ScUnoAddInCollection::Initialize()
/*N*/ {
/*N*/     DBG_ASSERT( !bInitialized, "Initialize twice?" );
/*N*/ 
/*N*/     uno::Reference<lang::XMultiServiceFactory> xManager = ::legacy_binfilters::getLegacyProcessServiceFactory();
/*N*/     uno::Reference<container::XContentEnumerationAccess> xEnAc( xManager, uno::UNO_QUERY );
/*N*/     if ( xEnAc.is() )
/*N*/     {
/*N*/         uno::Reference<container::XEnumeration> xEnum =
/*N*/                         xEnAc->createContentEnumeration(
/*N*/                             ::rtl::OUString::createFromAscii(SCADDINSUPPLIER_SERVICE) );
/*N*/         if ( xEnum.is() )
/*N*/         {
/*N*/             //  loop through all AddIns
/*N*/             while ( xEnum->hasMoreElements() )
/*N*/             {
/*N*/                 uno::Any aAddInAny = xEnum->nextElement();
/*N*/ //?             if ( aAddInAny.getReflection()->getTypeClass() == uno::TypeClass_INTERFACE )
/*N*/                 {
/*N*/                     uno::Reference<uno::XInterface> xIntFac;
/*N*/                     aAddInAny >>= xIntFac;
/*N*/                     if ( xIntFac.is() )
/*N*/                     {
/*N*/                         uno::Reference<lang::XSingleServiceFactory> xFac( xIntFac, uno::UNO_QUERY );
/*N*/                         if ( xFac.is() )
/*N*/                         {
/*N*/                             uno::Reference<uno::XInterface> xInterface = xFac->createInstance();
/*N*/                             ReadFromAddIn( xInterface );
/*N*/                         }
/*N*/                     }
/*N*/                 }
/*N*/             }
/*N*/         }
/*N*/     }
/*N*/ 
/*N*/     bInitialized = TRUE;        // with or without functions
/*N*/ }



/*N*/ USHORT lcl_GetCategory( const String& rName )
/*N*/ {
/*N*/     for (USHORT i=0; i<SC_FUNCGROUP_COUNT; i++)
/*N*/         if ( rName.EqualsAscii( aFuncNames[i] ) )
/*N*/             return i+1;                             // IDs start at 1
/*N*/ 
/*N*/     return ID_FUNCTION_GRP_ADDINS;  // if not found, use Add-In group
/*N*/ }

/*N*/ inline BOOL IsTypeName( const ::rtl::OUString& rName, const uno::Type& rType )
/*N*/ {
/*N*/     return rName == rType.getTypeName();
/*N*/ }

/*N*/ BOOL lcl_ValidReturnType( const uno::Reference<reflection::XIdlClass>& xClass )
/*N*/ {
/*N*/     //  this must match with ScUnoAddInCall::SetResult
/*N*/ 
/*N*/     if ( !xClass.is() ) return FALSE;
/*N*/ 
/*N*/     switch (xClass->getTypeClass())
/*N*/     {
/*N*/         // case uno::TypeClass_VOID:
/*N*/         //  ???
/*N*/ 
/*N*/         case uno::TypeClass_ANY:                // variable type
/*N*/         case uno::TypeClass_ENUM:               //! ???
/*N*/         case uno::TypeClass_BOOLEAN:
/*N*/         case uno::TypeClass_CHAR:
/*N*/         case uno::TypeClass_BYTE:
/*N*/         case uno::TypeClass_SHORT:
/*N*/         case uno::TypeClass_UNSIGNED_SHORT:
/*N*/         case uno::TypeClass_LONG:
/*N*/         case uno::TypeClass_UNSIGNED_LONG:
/*N*/         case uno::TypeClass_FLOAT:
/*N*/         case uno::TypeClass_DOUBLE:
/*N*/         case uno::TypeClass_STRING:
/*N*/             return TRUE;                        // values or string
/*N*/ 
/*N*/         case uno::TypeClass_INTERFACE:
/*N*/             {
/*N*/                 //  return type XInterface may contain a XVolatileResult
/*N*/                 //! XIdlClass needs getType() method!
/*N*/ 
/*N*/                 ::rtl::OUString sName = xClass->getName();
/*N*/                 return (
/*N*/                     IsTypeName( sName, getCppuType((uno::Reference<sheet::XVolatileResult>*)0) ) ||
/*N*/                     IsTypeName( sName, getCppuType((uno::Reference<uno::XInterface>*)0) ) );
/*N*/             }
/*N*/ 
/*N*/         default:
/*N*/             {
/*N*/                 //  nested sequences for arrays
/*N*/                 //! XIdlClass needs getType() method!
/*N*/ 
/*N*/                 ::rtl::OUString sName = xClass->getName();
/*N*/                 return (
/*N*/                     IsTypeName( sName, getCppuType((uno::Sequence< uno::Sequence<INT32> >*)0) ) ||
/*N*/                     IsTypeName( sName, getCppuType((uno::Sequence< uno::Sequence<double> >*)0) ) ||
/*N*/                     IsTypeName( sName, getCppuType((uno::Sequence< uno::Sequence<rtl::OUString> >*)0) ) ||
/*N*/                     IsTypeName( sName, getCppuType((uno::Sequence< uno::Sequence<uno::Any> >*)0) ) );
/*N*/             }
/*N*/     }
/*N*/     return FALSE;
/*N*/ }

/*N*/ ScAddInArgumentType lcl_GetArgType( const uno::Reference<reflection::XIdlClass>& xClass )
/*N*/ {
/*N*/     if (!xClass.is())
/*N*/         return SC_ADDINARG_NONE;
/*N*/ 
/*N*/     uno::TypeClass eType = xClass->getTypeClass();
/*N*/ 
/*N*/     if ( eType == uno::TypeClass_LONG )             //! other integer types?
/*N*/         return SC_ADDINARG_INTEGER;
/*N*/ 
/*N*/     if ( eType == uno::TypeClass_DOUBLE )
/*N*/         return SC_ADDINARG_DOUBLE;
/*N*/ 
/*N*/     if ( eType == uno::TypeClass_STRING )
/*N*/         return SC_ADDINARG_STRING;
/*N*/ 
/*N*/     //! XIdlClass needs getType() method!
/*N*/     ::rtl::OUString sName = xClass->getName();
/*N*/ 
/*N*/     if (IsTypeName( sName, getCppuType((uno::Sequence< uno::Sequence<INT32> >*)0) ))
/*N*/         return SC_ADDINARG_INTEGER_ARRAY;
/*N*/ 
/*N*/     if (IsTypeName( sName, getCppuType((uno::Sequence< uno::Sequence<double> >*)0) ))
/*N*/         return SC_ADDINARG_DOUBLE_ARRAY;
/*N*/ 
/*N*/     if (IsTypeName( sName, getCppuType((uno::Sequence< uno::Sequence<rtl::OUString> >*)0) ))
/*N*/         return SC_ADDINARG_STRING_ARRAY;
/*N*/ 
/*N*/     if (IsTypeName( sName, getCppuType((uno::Sequence< uno::Sequence<uno::Any> >*)0) ))
/*N*/         return SC_ADDINARG_MIXED_ARRAY;
/*N*/ 
/*N*/     if (IsTypeName( sName, getCppuType((uno::Any*)0) ))
/*N*/         return SC_ADDINARG_VALUE_OR_ARRAY;
/*N*/ 
/*N*/     if (IsTypeName( sName, getCppuType((uno::Reference<table::XCellRange>*)0) ))
/*N*/         return SC_ADDINARG_CELLRANGE;
/*N*/ 
/*N*/     if (IsTypeName( sName, getCppuType((uno::Reference<beans::XPropertySet>*)0) ))
/*N*/         return SC_ADDINARG_CALLER;
/*N*/ 
/*N*/     if (IsTypeName( sName, getCppuType((uno::Sequence<uno::Any>*)0) ))
/*N*/         return SC_ADDINARG_VARARGS;
/*N*/ 
/*N*/     return SC_ADDINARG_NONE;
/*N*/ }

/*N*/ void ScUnoAddInCollection::ReadFromAddIn( const uno::Reference<uno::XInterface>& xInterface )
/*N*/ {
/*N*/     uno::Reference<sheet::XAddIn> xAddIn( xInterface, uno::UNO_QUERY );
/*N*/     uno::Reference<lang::XServiceName> xName( xInterface, uno::UNO_QUERY );
/*N*/     if ( xAddIn.is() && xName.is() )
/*N*/     {
/*N*/         //  AddIns must use the language for which the office is installed
/*N*/         LanguageType eOfficeLang = Application::GetSettings().GetUILanguage();
/*N*/ 
/*N*/         lang::Locale aLocale( MsLangId::convertLanguageToLocale( eOfficeLang ));
/*N*/         xAddIn->setLocale( aLocale );
/*N*/ 
/*N*/         String aServiceName = String( xName->getServiceName() );
/*N*/         ScUnoAddInHelpIdGenerator aHelpIdGenerator( xName->getServiceName() );
/*N*/ 
/*N*/         //! pass XIntrospection to ReadFromAddIn
/*N*/ 
/*N*/         uno::Reference<lang::XMultiServiceFactory> xManager = ::legacy_binfilters::getLegacyProcessServiceFactory();
/*N*/         if ( xManager.is() )
/*N*/         {
/*N*/             uno::Reference<beans::XIntrospection> xIntro(
/*N*/                                     xManager->createInstance(::rtl::OUString::createFromAscii(
/*N*/                                         "com.sun.star.beans.Introspection" )),
/*N*/                                     uno::UNO_QUERY );
/*N*/             if ( xIntro.is() )
/*N*/             {
/*N*/                 uno::Any aObject;
/*N*/                 aObject <<= xAddIn;
/*N*/                 uno::Reference<beans::XIntrospectionAccess> xAcc = xIntro->inspect(aObject);
/*N*/                 if (xAcc.is())
/*N*/                 {
/*N*/                     uno::Sequence< uno::Reference<reflection::XIdlMethod> > aMethods =
/*N*/                             xAcc->getMethods( beans::MethodConcept::ALL );
/*N*/                     long nNewCount = aMethods.getLength();
/*N*/                     if ( nNewCount )
/*N*/                     {
/*N*/                         long nOld = nFuncCount;
/*N*/                         nFuncCount = nNewCount+nOld;
/*N*/                         if ( nOld )
/*N*/                         {
/*N*/                             ScUnoAddInFuncData** ppNew = new ScUnoAddInFuncData*[nFuncCount];
/*N*/                             for (long i=0; i<nOld; i++)
/*N*/                                 ppNew[i] = ppFuncData[i];
/*N*/                             delete[] ppFuncData;
/*N*/                             ppFuncData = ppNew;
/*N*/                         }
/*N*/                         else
/*N*/                             ppFuncData = new ScUnoAddInFuncData*[nFuncCount];
/*N*/ 
/*N*/                         //! TODO: adjust bucket count?
/*N*/                         if ( !pExactHashMap )
/*N*/                             pExactHashMap = new ScAddInHashMap;
/*N*/                         if ( !pNameHashMap )
/*N*/                             pNameHashMap = new ScAddInHashMap;
/*N*/                         if ( !pLocalHashMap )
/*N*/                             pLocalHashMap = new ScAddInHashMap;
/*N*/ 
/*N*/                         const uno::Reference<reflection::XIdlMethod>* pArray = aMethods.getConstArray();
/*N*/                         for (long nFuncPos=0; nFuncPos<nNewCount; nFuncPos++)
/*N*/                         {
/*N*/                             ppFuncData[nFuncPos+nOld] = NULL;
/*N*/ 
/*N*/                             uno::Reference<reflection::XIdlMethod> xFunc = pArray[nFuncPos];
/*N*/                             if (xFunc.is())
/*N*/                             {
/*N*/                                 //  leave out internal functions
/*N*/                                 uno::Reference<reflection::XIdlClass> xClass =
/*N*/                                                 xFunc->getDeclaringClass();
/*N*/                                 BOOL bSkip = TRUE;
/*N*/                                 if ( xClass.is() )
/*N*/                                 {
/*N*/                                     //! XIdlClass needs getType() method!
/*N*/                                     ::rtl::OUString sName = xClass->getName();
/*N*/                                     bSkip = (
/*N*/                                         IsTypeName( sName,
/*N*/                                             getCppuType((uno::Reference<uno::XInterface>*)0) ) ||
/*N*/                                         IsTypeName( sName,
/*N*/                                             getCppuType((uno::Reference<reflection::XIdlClassProvider>*)0) ) ||
/*N*/                                         IsTypeName( sName,
/*N*/                                             getCppuType((uno::Reference<lang::XServiceName>*)0) ) ||
/*N*/                                         IsTypeName( sName,
/*N*/                                             getCppuType((uno::Reference<lang::XServiceInfo>*)0) ) ||
/*N*/                                         IsTypeName( sName,
/*N*/                                             getCppuType((uno::Reference<sheet::XAddIn>*)0) ) );
/*N*/                                 }
/*N*/                                 if (!bSkip)
/*N*/                                 {
/*N*/                                     uno::Reference<reflection::XIdlClass> xReturn =
/*N*/                                                 xFunc->getReturnType();
/*N*/                                     if ( !lcl_ValidReturnType( xReturn ) )
/*N*/                                         bSkip = TRUE;
/*N*/                                 }
/*N*/                                 if (!bSkip)
/*N*/                                 {
/*N*/                                     ::rtl::OUString aFuncU = xFunc->getName();
/*N*/ 
/*N*/                                     // stored function name: (service name).(function)
/*N*/                                     String aFuncName = aServiceName;
/*N*/                                     aFuncName += '.';
/*N*/                                     aFuncName += String( aFuncU );
/*N*/ 
/*N*/                                     BOOL bValid = TRUE;
/*N*/                                     long nVisibleCount = 0;
/*N*/                                     long nCallerPos = SC_CALLERPOS_NONE;
/*N*/ 
/*N*/                                     uno::Sequence<reflection::ParamInfo> aParams =
/*N*/                                             xFunc->getParameterInfos();
/*N*/                                     long nParamCount = aParams.getLength();
/*N*/                                     const reflection::ParamInfo* pParArr = aParams.getConstArray();
/*N*/                                     long nParamPos;
/*N*/                                     for (nParamPos=0; nParamPos<nParamCount; nParamPos++)
/*N*/                                     {
/*N*/                                         if ( pParArr[nParamPos].aMode != reflection::ParamMode_IN )
/*N*/                                             bValid = FALSE;
/*N*/                                         uno::Reference<reflection::XIdlClass> xParClass =
/*N*/                                                     pParArr[nParamPos].aType;
/*N*/                                         ScAddInArgumentType eArgType = lcl_GetArgType( xParClass );
/*N*/                                         if ( eArgType == SC_ADDINARG_NONE )
/*N*/                                             bValid = FALSE;
/*N*/                                         else if ( eArgType == SC_ADDINARG_CALLER )
/*N*/                                             nCallerPos = nParamPos;
/*N*/                                         else
/*N*/                                             ++nVisibleCount;
/*N*/                                     }
/*N*/                                     if (bValid)
/*N*/                                     {
/*N*/                                         USHORT nCategory = lcl_GetCategory(
/*N*/                                             String(
/*N*/                                             xAddIn->getProgrammaticCategoryName(
/*N*/                                             aFuncU ) ) );
/*N*/ 
/*N*/                                         USHORT nHelpId = aHelpIdGenerator.GetHelpId( aFuncU );
/*N*/ 
/*N*/                                         ::rtl::OUString aLocalU;
/*N*/                                         try
/*N*/                                         {
/*N*/                                             aLocalU = xAddIn->
/*N*/                                                 getDisplayFunctionName( aFuncU );
/*N*/                                         }
/*N*/                                         catch(uno::Exception&)
/*N*/                                         {
/*N*/                                             aLocalU = ::rtl::OUString::createFromAscii( "###" );
/*N*/                                         }
/*N*/                                         String aLocalName = String( aLocalU );
/*N*/ 
/*N*/                                         ::rtl::OUString aDescU;
/*N*/                                         try
/*N*/                                         {
/*N*/                                             aDescU = xAddIn->
/*N*/                                                 getFunctionDescription( aFuncU );
/*N*/                                         }
/*N*/                                         catch(uno::Exception&)
/*N*/                                         {
/*N*/                                             aDescU = ::rtl::OUString::createFromAscii( "###" );
/*N*/                                         }
/*N*/                                         String aDescription = String( aDescU );
/*N*/ 
/*N*/                                         ScAddInArgDesc* pVisibleArgs = NULL;
/*N*/                                         if ( nVisibleCount > 0 )
/*N*/                                         {
/*N*/                                             ScAddInArgDesc aDesc;
/*N*/                                             pVisibleArgs = new ScAddInArgDesc[nVisibleCount];
/*N*/                                             long nDestPos = 0;
/*N*/                                             for (nParamPos=0; nParamPos<nParamCount; nParamPos++)
/*N*/                                             {
/*N*/                                                 uno::Reference<reflection::XIdlClass> xParClass =
/*N*/                                                     pParArr[nParamPos].aType;
/*N*/                                                 ScAddInArgumentType eArgType = lcl_GetArgType( xParClass );
/*N*/                                                 if ( eArgType != SC_ADDINARG_CALLER )
/*N*/                                                 {
/*N*/                                                     ::rtl::OUString aArgName;
/*N*/                                                     try
/*N*/                                                     {
/*N*/                                                         aArgName = xAddIn->
/*N*/                                                             getDisplayArgumentName( aFuncU, nParamPos );
/*N*/                                                     }
/*N*/                                                     catch(uno::Exception&)
/*N*/                                                     {
/*N*/                                                         aArgName = ::rtl::OUString::createFromAscii( "###" );
/*N*/                                                     }
/*N*/                                                     ::rtl::OUString aArgDesc;
/*N*/                                                     try
/*N*/                                                     {
/*N*/                                                         aArgDesc = xAddIn->
/*N*/                                                             getArgumentDescription( aFuncU, nParamPos );
/*N*/                                                     }
/*N*/                                                     catch(uno::Exception&)
/*N*/                                                     {
/*N*/                                                         aArgName = ::rtl::OUString::createFromAscii( "###" );
/*N*/                                                     }
/*N*/ 
/*N*/                                                     BOOL bOptional =
/*N*/                                                         ( eArgType == SC_ADDINARG_VALUE_OR_ARRAY ||
/*N*/                                                           eArgType == SC_ADDINARG_VARARGS );
/*N*/ 
/*N*/                                                     aDesc.eType = eArgType;
/*N*/                                                     aDesc.aName = String( aArgName );
/*N*/                                                     aDesc.aDescription = String( aArgDesc );
/*N*/                                                     aDesc.bOptional = bOptional;
/*N*/ 
/*N*/                                                     pVisibleArgs[nDestPos++] = aDesc;
/*N*/                                                 }
/*N*/                                             }
/*N*/                                             DBG_ASSERT( nDestPos==nVisibleCount, "wrong count" );
/*N*/                                         }
/*N*/ 
/*N*/                                         ppFuncData[nFuncPos+nOld] = new ScUnoAddInFuncData(
/*N*/                                             aFuncName, aLocalName, aDescription,
/*N*/                                             nCategory, nHelpId,
/*N*/                                             xFunc, aObject,
/*N*/                                             nVisibleCount, pVisibleArgs, nCallerPos );
/*N*/ 
/*N*/                                         const ScUnoAddInFuncData* pData =
/*N*/                                             ppFuncData[nFuncPos+nOld];
/*N*/                                         pExactHashMap->insert(
/*N*/                                                 ScAddInHashMap::value_type(
/*N*/                                                     pData->GetOriginalName(),
/*N*/                                                     pData ) );
/*N*/                                         pNameHashMap->insert(
/*N*/                                                 ScAddInHashMap::value_type(
/*N*/                                                     pData->GetUpperName(),
/*N*/                                                     pData ) );
/*N*/                                         pLocalHashMap->insert(
/*N*/                                                 ScAddInHashMap::value_type(
/*N*/                                                     pData->GetUpperLocal(),
/*N*/                                                     pData ) );
/*N*/ 
/*N*/                                         delete[] pVisibleArgs;
/*N*/                                     }
/*N*/                                 }
/*N*/                             }
/*N*/                         }
/*N*/                     }
/*N*/                 }
/*N*/             }
/*N*/         }
/*N*/     }
/*N*/ }

/*N*/ String ScUnoAddInCollection::FindFunction( const String& rUpperName, BOOL bLocalFirst )
/*N*/ {
/*N*/     if (!bInitialized)
/*N*/         Initialize();
/*N*/ 
/*N*/     if (nFuncCount == 0)
/*N*/         return EMPTY_STRING;
/*N*/ 
/*N*/     if ( bLocalFirst )
/*N*/     {
/*N*/         //  first scan all local names (used for entering formulas)
/*N*/ 
/*N*/         ScAddInHashMap::const_iterator iLook( pLocalHashMap->find( rUpperName ) );
/*N*/         if ( iLook != pLocalHashMap->end() )
/*?*/             return iLook->second->GetOriginalName();
/*N*/ 
/*N*/ #if 0
/*N*/         //  after that, scan international names (really?)
/*N*/ 
/*N*/         iLook = pNameHashMap->find( rUpperName );
/*N*/         if ( iLook != pNameHashMap->end() )
/*N*/             return iLook->second->GetOriginalName();
/*N*/ #endif
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         //  first scan international names (used when calling a function)
/*N*/         //! before that, check for exact match???
/*N*/ 
/*N*/         ScAddInHashMap::const_iterator iLook( pNameHashMap->find( rUpperName ) );
/*N*/         if ( iLook != pNameHashMap->end() )
/*N*/             return iLook->second->GetOriginalName();
/*N*/ 
/*N*/         //  after that, scan all local names (to allow replacing old AddIns with Uno)
/*N*/ 
/*N*/         iLook = pLocalHashMap->find( rUpperName );
/*N*/         if ( iLook != pLocalHashMap->end() )
/*N*/             return iLook->second->GetOriginalName();
/*N*/     }
/*N*/ 
/*N*/     return EMPTY_STRING;
/*N*/ }

/*N*/ const ScUnoAddInFuncData* ScUnoAddInCollection::GetFuncData( const String& rName )
/*N*/ {
/*N*/     if (!bInitialized)
/*?*/         Initialize();
/*N*/ 
/*N*/     //  rName must be the exact internal name
/*N*/ 
/*N*/     ScAddInHashMap::const_iterator iLook( pExactHashMap->find( rName ) );
/*N*/     if ( iLook != pExactHashMap->end() )
/*N*/         return iLook->second;
/*N*/ 
/*N*/     return NULL;
/*N*/ }






//------------------------------------------------------------------------

/*N*/ ScUnoAddInCall::ScUnoAddInCall( ScUnoAddInCollection& rColl, const String& rName,
/*N*/                                 long nParamCount ) :
/*N*/     nErrCode( errNoCode ),      // before function was called
/*N*/     bHasString( TRUE ),
/*N*/     fValue( 0.0 ),
/*N*/     pMatrix( NULL ),
/*N*/     bValidCount( FALSE )
/*N*/ {
/*N*/	pFuncData = rColl.GetFuncData( rName );
/*N*/     DBG_ASSERT( pFuncData, "Function Data missing" );
/*N*/     if ( pFuncData )
/*N*/     {
/*N*/         long nDescCount = pFuncData->GetArgumentCount();
/*N*/         const ScAddInArgDesc* pArgs = pFuncData->GetArguments();
/*N*/         long nVarCount = 0;
/*N*/ 
/*N*/         //  is aVarArg sequence needed?
/*N*/         if ( nParamCount >= nDescCount && nDescCount > 0 &&
/*N*/              pArgs[nDescCount-1].eType == SC_ADDINARG_VARARGS )
/*N*/         {
/*?*/ 					{DBG_BF_ASSERT(0, "STRIP");} //STRIP001             long nVarCount = nParamCount - ( nDescCount - 1 );  // size of last argument
/*N*/         }
/*N*/         else if ( nParamCount <= nDescCount )
/*N*/         {
/*N*/             //  all args behind nParamCount must be optional
/*N*/             bValidCount = TRUE;
/*N*/             for (long i=nParamCount; i<nDescCount; i++)
/*?*/                 if ( !pArgs[i].bOptional )
/*?*/                     bValidCount = FALSE;
/*N*/         }
/*N*/         // else invalid (too many arguments)
/*N*/ 
/*N*/         if ( bValidCount )
/*N*/             aArgs.realloc( nDescCount );    // sequence must always match function signature
/*N*/     }
/*N*/ }

/*N*/ ScUnoAddInCall::~ScUnoAddInCall()
/*N*/ {
/*N*/     // pFuncData is deleted with ScUnoAddInCollection
/*N*/ 
/*N*/     delete pMatrix;
/*N*/ }

/*N*/ BOOL ScUnoAddInCall::ValidParamCount()
/*N*/ {
/*N*/     return bValidCount;
/*N*/ }

/*N*/ ScAddInArgumentType ScUnoAddInCall::GetArgType( long nPos )
/*N*/ {
/*N*/     if ( pFuncData )
/*N*/     {
/*N*/         long nCount = pFuncData->GetArgumentCount();
/*N*/         const ScAddInArgDesc* pArgs = pFuncData->GetArguments();
/*N*/ 
/*N*/         // if last arg is sequence, use "any" type
/*N*/         if ( nCount > 0 && nPos >= nCount-1 && pArgs[nCount-1].eType == SC_ADDINARG_VARARGS )
/*N*/             return SC_ADDINARG_VALUE_OR_ARRAY;
/*N*/ 
/*N*/         if ( nPos < nCount )
/*N*/             return pArgs[nPos].eType;
/*N*/     }
/*N*/     return SC_ADDINARG_VALUE_OR_ARRAY;      //! error code !!!!
/*N*/ }

/*N*/ BOOL ScUnoAddInCall::NeedsCaller() const
/*N*/ {
/*N*/     return pFuncData && pFuncData->GetCallerPos() != SC_CALLERPOS_NONE;
/*N*/ }

/*N*/ void ScUnoAddInCall::SetCaller( const uno::Reference<uno::XInterface>& rInterface )
/*N*/ {
/*N*/     xCaller = rInterface;
/*N*/ }

/*N*/ void ScUnoAddInCall::SetCallerFromObjectShell( SfxObjectShell* pObjSh )
/*N*/ {
/*N*/     if (pObjSh)
/*N*/     {
/*N*/         uno::Reference<uno::XInterface> xInt( pObjSh->GetBaseModel(), uno::UNO_QUERY );
/*N*/         SetCaller( xInt );
/*N*/     }
/*N*/ }

/*N*/ void ScUnoAddInCall::SetParam( long nPos, const uno::Any& rValue )
/*N*/ {
/*N*/     if ( pFuncData )
/*N*/     {
/*N*/         long nCount = pFuncData->GetArgumentCount();
/*N*/         const ScAddInArgDesc* pArgs = pFuncData->GetArguments();
/*N*/         if ( nCount > 0 && nPos >= nCount-1 && pArgs[nCount-1].eType == SC_ADDINARG_VARARGS )
/*N*/         {
/*N*/             long nVarPos = nPos-(nCount-1);
/*N*/             if ( nVarPos < aVarArg.getLength() )
/*N*/                 aVarArg.getArray()[nVarPos] = rValue;
/*N*/             else
/*N*/                 DBG_ERROR("wrong argument number");
/*N*/         }
/*N*/         else if ( nPos < aArgs.getLength() )
/*N*/             aArgs.getArray()[nPos] = rValue;
/*N*/         else
/*N*/             DBG_ERROR("wrong argument number");
/*N*/     }
/*N*/ }

/*N*/ void ScUnoAddInCall::ExecuteCall()
/*N*/ {
/*N*/     if ( !pFuncData )
/*N*/         return;
/*N*/ 
/*N*/     long nCount = pFuncData->GetArgumentCount();
/*N*/     const ScAddInArgDesc* pArgs = pFuncData->GetArguments();
/*N*/     if ( nCount > 0 && pArgs[nCount-1].eType == SC_ADDINARG_VARARGS )
/*N*/     {
/*N*/         //  insert aVarArg as last argument
/*N*/         //! after inserting caller (to prevent copying twice)?
/*N*/ 
/*N*/         DBG_ASSERT( aArgs.getLength() == nCount, "wrong argument count" );
/*N*/         aArgs.getArray()[nCount-1] <<= aVarArg;
/*N*/     }
/*N*/ 
/*N*/     if ( pFuncData->GetCallerPos() != SC_CALLERPOS_NONE )
/*N*/     {
/*N*/         uno::Any aCallerAny;
/*N*/         aCallerAny <<= xCaller;
/*N*/ 
/*N*/         long nUserLen = aArgs.getLength();
/*N*/         long nCallPos = pFuncData->GetCallerPos();
/*N*/         if (nCallPos>nUserLen)                          // should not happen
/*N*/         {
/*N*/             DBG_ERROR("wrong CallPos");
/*N*/             nCallPos = nUserLen;
/*N*/         }
/*N*/ 
/*N*/         long nDestLen = nUserLen + 1;
/*N*/         uno::Sequence<uno::Any> aRealArgs( nDestLen );
/*N*/         uno::Any* pDest = aRealArgs.getArray();
/*N*/ 
/*N*/         const uno::Any* pSource = aArgs.getConstArray();
/*N*/         long nSrcPos = 0;
/*N*/ 
/*N*/         for ( long nDestPos = 0; nDestPos < nDestLen; nDestPos++ )
/*N*/         {
/*N*/             if ( nDestPos == nCallPos )
/*N*/                 pDest[nDestPos] = aCallerAny;
/*N*/             else
/*N*/                 pDest[nDestPos] = pSource[nSrcPos++];
/*N*/         }
/*N*/ 
/*N*/         ExecuteCallWithArgs( aRealArgs );
/*N*/     }
/*N*/     else
/*N*/         ExecuteCallWithArgs( aArgs );
/*N*/ }

/*N*/ void ScUnoAddInCall::ExecuteCallWithArgs(uno::Sequence<uno::Any>& rCallArgs)
/*N*/ {
/*N*/     //  rCallArgs may not match argument descriptions (because of caller)
/*N*/ 
/*N*/     uno::Reference<reflection::XIdlMethod> xFunction;
/*N*/     uno::Any aObject;
/*N*/     if ( pFuncData )
/*N*/     {
/*N*/         xFunction = pFuncData->GetFunction();
/*N*/         aObject = pFuncData->GetObject();
/*N*/     }
/*N*/ 
/*N*/     if ( xFunction.is() )
/*N*/     {
/*N*/         uno::Any aAny;
/*N*/         nErrCode = 0;
/*N*/ 
/*N*/         try
/*N*/         {
/*N*/             aAny = xFunction->invoke( aObject, rCallArgs );
/*N*/         }
/*N*/         catch(lang::IllegalArgumentException&)
/*N*/         {
/*N*/             nErrCode = errIllegalArgument;
/*N*/         }
/*N*/ #if 0
/*N*/         catch(FloatingPointException&)
/*N*/         {
/*N*/             nErrCode = errIllegalFPOperation;
/*N*/         }
/*N*/ #endif
/*N*/ 		catch(reflection::InvocationTargetException& rWrapped)
/*N*/ 		{
/*N*/ 			if ( rWrapped.TargetException.getValueType().equals(
/*N*/ 					getCppuType( (lang::IllegalArgumentException*)0 ) ) )
/*N*/ 				nErrCode = errIllegalArgument;
/*N*/ 			else
/*N*/ 	            nErrCode = errNoValue;
/*N*/ 		}
/*N*/         catch(uno::Exception&)
/*N*/         {
/*N*/             nErrCode = errNoValue;
/*N*/         }
/*N*/ 
/*N*/         if (!nErrCode)
/*N*/             SetResult( aAny );      // convert result to Calc types
/*N*/     }
/*N*/ }

/*N*/ void ScUnoAddInCall::SetResult( const uno::Any& rNewRes )
/*N*/ {
/*N*/     nErrCode = 0;
/*N*/     xVarRes = NULL;
/*N*/ 
/*N*/     // Reflection* pRefl = rNewRes.getReflection();
/*N*/ 
/*N*/     uno::TypeClass eClass = rNewRes.getValueTypeClass();
/*N*/     uno::Type aType = rNewRes.getValueType();
/*N*/     switch (eClass)
/*N*/     {
/*N*/         case uno::TypeClass_VOID:
/*N*/             nErrCode = NOVALUE;         // #nv
/*N*/             break;
/*N*/ 
/*N*/         case uno::TypeClass_ENUM:
/*N*/         case uno::TypeClass_BOOLEAN:
/*N*/         case uno::TypeClass_CHAR:
/*N*/         case uno::TypeClass_BYTE:
/*N*/         case uno::TypeClass_SHORT:
/*N*/         case uno::TypeClass_UNSIGNED_SHORT:
/*N*/         case uno::TypeClass_LONG:
/*N*/         case uno::TypeClass_UNSIGNED_LONG:
/*N*/         case uno::TypeClass_FLOAT:
/*N*/         case uno::TypeClass_DOUBLE:
/*N*/             lcl_ConvertToDouble( rNewRes, fValue );
/*N*/             bHasString = FALSE;
/*N*/             break;
/*N*/ 
/*N*/         case uno::TypeClass_STRING:
/*N*/             {
/*N*/                 ::rtl::OUString aUStr;
/*N*/                 rNewRes >>= aUStr;
/*N*/                 aString = String( aUStr );
/*N*/                 bHasString = TRUE;
/*N*/             }
/*N*/             break;
/*N*/ 
/*N*/         case uno::TypeClass_INTERFACE:
/*N*/             {
/*N*/                 //! directly extract XVolatileResult from any?
/*N*/                 uno::Reference<uno::XInterface> xInterface;
/*N*/                 rNewRes >>= xInterface;
/*N*/                 if ( xInterface.is() )
/*N*/                     xVarRes = uno::Reference<sheet::XVolatileResult>( xInterface, uno::UNO_QUERY );
/*N*/ 
/*N*/                 if (!xVarRes.is())
/*N*/                     nErrCode = errNoValue;          // unknown interface
/*N*/             }
/*N*/             break;
/*N*/ 
/*N*/         default:
/*N*/             if ( aType.equals( getCppuType( (uno::Sequence< uno::Sequence<INT32> > *)0 ) ) )
/*N*/             {
/*N*/                 const uno::Sequence< uno::Sequence<INT32> >* pRowSeq = NULL;
/*N*/ 
/*N*/                 //! use pointer from any!
/*N*/                 uno::Sequence< uno::Sequence<INT32> > aSequence;
/*N*/                 if ( rNewRes >>= aSequence )
/*N*/                     pRowSeq = &aSequence;
/*N*/ 
/*N*/                 if ( pRowSeq )
/*N*/                 {
/*N*/                     long nRowCount = pRowSeq->getLength();
/*N*/                     const uno::Sequence<INT32>* pRowArr = pRowSeq->getConstArray();
/*N*/                     long nMaxColCount = 0;
/*N*/                     long nCol, nRow;
/*N*/                     for (nRow=0; nRow<nRowCount; nRow++)
/*N*/                     {
/*N*/                         long nTmp = pRowArr[nRow].getLength();
/*N*/                         if ( nTmp > nMaxColCount )
/*N*/                             nMaxColCount = nTmp;
/*N*/                     }
/*N*/                     if ( nMaxColCount && nRowCount )
/*N*/                     {
/*N*/                         pMatrix = new ScMatrix( (USHORT)nMaxColCount, (USHORT)nRowCount );
/*N*/                         for (nRow=0; nRow<nRowCount; nRow++)
/*N*/                         {
/*N*/                             long nColCount = pRowArr[nRow].getLength();
/*N*/                             const INT32* pColArr = pRowArr[nRow].getConstArray();
/*N*/                             for (nCol=0; nCol<nColCount; nCol++)
/*N*/                                 pMatrix->PutDouble( pColArr[nCol], (USHORT)nCol, (USHORT)nRow );
/*N*/                             for (nCol=nColCount; nCol<nMaxColCount; nCol++)
/*N*/                                 pMatrix->PutDouble( 0.0, (USHORT)nCol, (USHORT)nRow );
/*N*/                         }
/*N*/                     }
/*N*/                 }
/*N*/             }
/*N*/             else if ( aType.equals( getCppuType( (uno::Sequence< uno::Sequence<double> > *)0 ) ) )
/*N*/             {
/*N*/                 const uno::Sequence< uno::Sequence<double> >* pRowSeq = NULL;
/*N*/ 
/*N*/                 //! use pointer from any!
/*N*/                 uno::Sequence< uno::Sequence<double> > aSequence;
/*N*/                 if ( rNewRes >>= aSequence )
/*N*/                     pRowSeq = &aSequence;
/*N*/ 
/*N*/                 if ( pRowSeq )
/*N*/                 {
/*N*/                     long nRowCount = pRowSeq->getLength();
/*N*/                     const uno::Sequence<double>* pRowArr = pRowSeq->getConstArray();
/*N*/                     long nMaxColCount = 0;
/*N*/                     long nCol, nRow;
/*N*/                     for (nRow=0; nRow<nRowCount; nRow++)
/*N*/                     {
/*N*/                         long nTmp = pRowArr[nRow].getLength();
/*N*/                         if ( nTmp > nMaxColCount )
/*N*/                             nMaxColCount = nTmp;
/*N*/                     }
/*N*/                     if ( nMaxColCount && nRowCount )
/*N*/                     {
/*N*/                         pMatrix = new ScMatrix( (USHORT)nMaxColCount, (USHORT)nRowCount );
/*N*/                         for (nRow=0; nRow<nRowCount; nRow++)
/*N*/                         {
/*N*/                             long nColCount = pRowArr[nRow].getLength();
/*N*/                             const double* pColArr = pRowArr[nRow].getConstArray();
/*N*/                             for (nCol=0; nCol<nColCount; nCol++)
/*N*/                                 pMatrix->PutDouble( pColArr[nCol], (USHORT)nCol, (USHORT)nRow );
/*N*/                             for (nCol=nColCount; nCol<nMaxColCount; nCol++)
/*N*/                                 pMatrix->PutDouble( 0.0, (USHORT)nCol, (USHORT)nRow );
/*N*/                         }
/*N*/                     }
/*N*/                 }
/*N*/             }
/*N*/             else if ( aType.equals( getCppuType( (uno::Sequence< uno::Sequence<rtl::OUString> > *)0 ) ) )
/*N*/             {
/*N*/                 const uno::Sequence< uno::Sequence<rtl::OUString> >* pRowSeq = NULL;
/*N*/ 
/*N*/                 //! use pointer from any!
/*N*/                 uno::Sequence< uno::Sequence<rtl::OUString> > aSequence;
/*N*/                 if ( rNewRes >>= aSequence )
/*N*/                     pRowSeq = &aSequence;
/*N*/ 
/*N*/                 if ( pRowSeq )
/*N*/                 {
/*N*/                     long nRowCount = pRowSeq->getLength();
/*N*/                     const uno::Sequence<rtl::OUString>* pRowArr = pRowSeq->getConstArray();
/*N*/                     long nMaxColCount = 0;
/*N*/                     long nCol, nRow;
/*N*/                     for (nRow=0; nRow<nRowCount; nRow++)
/*N*/                     {
/*N*/                         long nTmp = pRowArr[nRow].getLength();
/*N*/                         if ( nTmp > nMaxColCount )
/*N*/                             nMaxColCount = nTmp;
/*N*/                     }
/*N*/                     if ( nMaxColCount && nRowCount )
/*N*/                     {
/*N*/                         pMatrix = new ScMatrix( (USHORT)nMaxColCount, (USHORT)nRowCount );
/*N*/                         for (nRow=0; nRow<nRowCount; nRow++)
/*N*/                         {
/*N*/                             long nColCount = pRowArr[nRow].getLength();
/*N*/                             const ::rtl::OUString* pColArr = pRowArr[nRow].getConstArray();
/*N*/                             for (nCol=0; nCol<nColCount; nCol++)
/*N*/                                 pMatrix->PutString( String( pColArr[nCol] ),
/*N*/                                     (USHORT)nCol, (USHORT)nRow );
/*N*/                             for (nCol=nColCount; nCol<nMaxColCount; nCol++)
/*N*/                                 pMatrix->PutString( EMPTY_STRING, (USHORT)nCol, (USHORT)nRow );
/*N*/                         }
/*N*/                     }
/*N*/                 }
/*N*/             }
/*N*/             else if ( aType.equals( getCppuType( (uno::Sequence< uno::Sequence<uno::Any> > *)0 ) ) )
/*N*/             {
/*N*/                 const uno::Sequence< uno::Sequence<uno::Any> >* pRowSeq = NULL;
/*N*/ 
/*N*/                 //! use pointer from any!
/*N*/                 uno::Sequence< uno::Sequence<uno::Any> > aSequence;
/*N*/                 if ( rNewRes >>= aSequence )
/*N*/                     pRowSeq = &aSequence;
/*N*/ 
/*N*/                 if ( pRowSeq )
/*N*/                 {
/*N*/                     long nRowCount = pRowSeq->getLength();
/*N*/                     const uno::Sequence<uno::Any>* pRowArr = pRowSeq->getConstArray();
/*N*/                     long nMaxColCount = 0;
/*N*/                     long nCol, nRow;
/*N*/                     for (nRow=0; nRow<nRowCount; nRow++)
/*N*/                     {
/*N*/                         long nTmp = pRowArr[nRow].getLength();
/*N*/                         if ( nTmp > nMaxColCount )
/*N*/                             nMaxColCount = nTmp;
/*N*/                     }
/*N*/                     if ( nMaxColCount && nRowCount )
/*N*/                     {
/*N*/                         ::rtl::OUString aUStr;
/*N*/                         pMatrix = new ScMatrix( (USHORT)nMaxColCount, (USHORT)nRowCount );
/*N*/                         for (nRow=0; nRow<nRowCount; nRow++)
/*N*/                         {
/*N*/                             long nColCount = pRowArr[nRow].getLength();
/*N*/                             const uno::Any* pColArr = pRowArr[nRow].getConstArray();
/*N*/                             for (nCol=0; nCol<nColCount; nCol++)
/*N*/                             {
/*N*/                                 //Reflection* pRefl = pColArr[nCol].getReflection();
/*N*/                                 //if ( pRefl->equals( *OUString_getReflection() ) )
/*N*/                                 if ( pColArr[nCol] >>= aUStr )
/*N*/                                     pMatrix->PutString( String( aUStr ),
/*N*/                                         (USHORT)nCol, (USHORT)nRow );
/*N*/                                 else
/*N*/                                 {
/*N*/                                     // try to convert to double, empty if not possible
/*N*/ 
/*N*/                                     double fCellVal;
/*N*/                                     if ( lcl_ConvertToDouble( pColArr[nCol], fCellVal ) )
/*N*/                                         pMatrix->PutDouble( fCellVal, (USHORT)nCol, (USHORT)nRow );
/*N*/                                     else
/*N*/                                         pMatrix->PutEmpty( (USHORT)nCol, (USHORT)nRow );
/*N*/                                 }
/*N*/                             }
/*N*/                             for (nCol=nColCount; nCol<nMaxColCount; nCol++)
/*N*/                                 pMatrix->PutString( EMPTY_STRING, (USHORT)nCol, (USHORT)nRow );
/*N*/                         }
/*N*/                     }
/*N*/                 }
/*N*/             }
/*N*/ 
/*N*/             if (!pMatrix)                       // no array found
/*N*/                 nErrCode = errNoValue;          //! code for error in return type???
/*N*/     }
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
