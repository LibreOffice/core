/*************************************************************************
 *
 *  $RCSfile: addincol.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: er $ $Date: 2001-03-14 18:10:21 $
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

#include <comphelper/processfactory.hxx>
#include <tools/debug.hxx>
#include <tools/isolang.hxx>
#include <vcl/svapp.hxx>
#include <vcl/system.hxx>
#include <vos/xception.hxx>
#include <sfx2/objsh.hxx>
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
#include "compiler.hxx"
#include "scmatrix.hxx"
#include "addinlis.hxx"
#include "scfuncs.hrc"

using namespace com::sun::star;

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
    uno::Sequence<sheet::LocalizedName> aCompNames;
    BOOL                bCompInitialized;

public:
                ScUnoAddInFuncData( const String& rNam, const String& rLoc,
                                    const String& rDesc, USHORT nCat,
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

    const uno::Sequence<sheet::LocalizedName>& GetCompNames();
};

//------------------------------------------------------------------------

ScUnoAddInFuncData::ScUnoAddInFuncData( const String& rNam, const String& rLoc,
                                        const String& rDesc, USHORT nCat,
                                        const uno::Reference<reflection::XIdlMethod>& rFunc,
                                        const uno::Any& rO,
                                        long nAC, const ScAddInArgDesc* pAD,
                                        long nCP ) :
    aOriginalName( rNam ),
    aUpperName( rNam ),
    aLocalName( rLoc ),
    aUpperLocal( rLoc ),
    aDescription( rDesc ),
    nCategory( nCat ),
    xFunction( rFunc ),
    aObject( rO ),
    nArgCount( nAC ),
    nCallerPos( nCP ),
    bCompInitialized( FALSE )
{
    if ( nArgCount )
    {
        pArgDescs = new ScAddInArgDesc[nArgCount];
        for (long i=0; i<nArgCount; i++)
            pArgDescs[i] = pAD[i];
    }
    else
        pArgDescs = NULL;

    ScGlobal::pCharClass->toUpper(aUpperName);
    ScGlobal::pCharClass->toUpper(aUpperLocal);
}

ScUnoAddInFuncData::~ScUnoAddInFuncData()
{
    delete[] pArgDescs;
}

const uno::Sequence<sheet::LocalizedName>& ScUnoAddInFuncData::GetCompNames()
{
    if ( !bCompInitialized )
    {
        //  read sequence of compatibility names on demand

        uno::Reference<sheet::XAddIn> xAddIn;
        if ( aObject >>= xAddIn )
        {
            uno::Reference<sheet::XCompatibilityNames> xComp( xAddIn, uno::UNO_QUERY );
            if ( xComp.is() && xFunction.is() )
            {
                rtl::OUString aMethodName = xFunction->getName();
                aCompNames = xComp->getCompatibilityNames( aMethodName );

                //  change all locale entries to default case
                //  (language in lower case, country in upper case)
                //  for easier searching

                long nSeqLen = aCompNames.getLength();
                if ( nSeqLen )
                {
                    sheet::LocalizedName* pArray = aCompNames.getArray();
                    for (long i=0; i<nSeqLen; i++)
                    {
                        lang::Locale& rLocale = pArray[i].Locale;
                        rLocale.Language = rLocale.Language.toLowerCase();
                        rLocale.Country  = rLocale.Country.toUpperCase();
                    }
                }
            }
        }

        bCompInitialized = TRUE;        // also if not successful
    }
    return aCompNames;
}

//------------------------------------------------------------------------

BOOL lcl_ConvertToDouble( const uno::Any& rAny, double& rOut )
{
    BOOL bRet = FALSE;
    uno::TypeClass eClass = rAny.getValueTypeClass();
    switch (eClass)
    {
        //! extract integer values
        case uno::TypeClass_ENUM:
        case uno::TypeClass_BOOLEAN:
        case uno::TypeClass_CHAR:
        case uno::TypeClass_BYTE:
        case uno::TypeClass_SHORT:
        case uno::TypeClass_UNSIGNED_SHORT:
        case uno::TypeClass_LONG:
        case uno::TypeClass_UNSIGNED_LONG:
        case uno::TypeClass_FLOAT:
        case uno::TypeClass_DOUBLE:
            rAny >>= rOut;
            bRet = TRUE;
            break;
    }
    if (!bRet)
        rOut = 0.0;
    return bRet;
}

//------------------------------------------------------------------------

ScUnoAddInCollection::ScUnoAddInCollection() :
    nFuncCount( 0 ),
    ppFuncData( NULL ),
    bInitialized( FALSE )
{
}

ScUnoAddInCollection::~ScUnoAddInCollection()
{
    if ( ppFuncData )
    {
        for ( long i=0; i<nFuncCount; i++ )
            delete ppFuncData[i];
        delete[] ppFuncData;
    }
}

void ScUnoAddInCollection::Initialize()
{
    DBG_ASSERT( !bInitialized, "Initialize twice?" )

    uno::Reference<lang::XMultiServiceFactory> xManager = comphelper::getProcessServiceFactory();
    uno::Reference<container::XContentEnumerationAccess> xEnAc( xManager, uno::UNO_QUERY );
    if ( xEnAc.is() )
    {
        uno::Reference<container::XEnumeration> xEnum =
                        xEnAc->createContentEnumeration(
                            rtl::OUString::createFromAscii(SCADDINSUPPLIER_SERVICE) );
        if ( xEnum.is() )
        {
            //  loop through all AddIns
            while ( xEnum->hasMoreElements() )
            {
                uno::Any aAddInAny = xEnum->nextElement();
//?             if ( aAddInAny.getReflection()->getTypeClass() == uno::TypeClass_INTERFACE )
                {
                    uno::Reference<uno::XInterface> xIntFac;
                    aAddInAny >>= xIntFac;
                    if ( xIntFac.is() )
                    {
                        uno::Reference<lang::XSingleServiceFactory> xFac( xIntFac, uno::UNO_QUERY );
                        if ( xFac.is() )
                        {
                            uno::Reference<uno::XInterface> xInterface = xFac->createInstance();
                            ReadFromAddIn( xInterface );
                        }
                    }
                }
            }
        }
    }

    bInitialized = TRUE;        // with or without functions
}

BOOL ScUnoAddInCollection::GetExcelName( const String& rCalcName,
                                        LanguageType eDestLang, String& rRetExcelName )
{
    ScUnoAddInFuncData* pFuncData = GetFuncData( rCalcName );
    if ( pFuncData )
    {
        const uno::Sequence<sheet::LocalizedName>& rSequence = pFuncData->GetCompNames();
        long nSeqLen = rSequence.getLength();
        if ( nSeqLen )
        {
            const sheet::LocalizedName* pArray = rSequence.getConstArray();
            long i;

            String aLangStr, aCountryStr;
            ConvertLanguageToIsoNames( eDestLang, aLangStr, aCountryStr );
            rtl::OUString aUserLang = aLangStr.ToLowerAscii();
            rtl::OUString aUserCountry = aCountryStr.ToUpperAscii();

            //  first check for match of both language and country

            for ( i=0; i<nSeqLen; i++)
                if ( pArray[i].Locale.Language == aUserLang &&
                     pArray[i].Locale.Country  == aUserCountry )
                {
                    rRetExcelName = pArray[i].Name;
                    return TRUE;
                }

            //  second: check only language

            for ( i=0; i<nSeqLen; i++)
                if ( pArray[i].Locale.Language == aUserLang )
                {
                    rRetExcelName = pArray[i].Name;
                    return TRUE;
                }

            //  third: use first (default) entry

            rRetExcelName = pArray[0].Name;
            return TRUE;
        }
    }
    return FALSE;
}

BOOL ScUnoAddInCollection::GetCalcName( const String& rExcelName, String& rRetCalcName )
{
    if (!bInitialized)
        Initialize();

    String aUpperCmp = rExcelName;
    ScGlobal::pCharClass->toUpper(aUpperCmp);

    for (long i=0; i<nFuncCount; i++)
    {
        ScUnoAddInFuncData* pFuncData = ppFuncData[i];
        if ( pFuncData )
        {
            const uno::Sequence<sheet::LocalizedName>& rSequence = pFuncData->GetCompNames();
            long nSeqLen = rSequence.getLength();
            if ( nSeqLen )
            {
                const sheet::LocalizedName* pArray = rSequence.getConstArray();
                for ( long i=0; i<nSeqLen; i++)
                    if ( ScGlobal::pCharClass->upper( pArray[i].Name ) == aUpperCmp )
                    {
                        //! store upper case for comparing?

                        //  use the first function that has this name for any language
                        rRetCalcName = pFuncData->GetOriginalName();
                        return TRUE;
                    }
            }
        }
    }
    return FALSE;
}

USHORT lcl_GetCategory( const String& rName )
{
    for (USHORT i=0; i<SC_FUNCGROUP_COUNT; i++)
        if ( rName.EqualsAscii( aFuncNames[i] ) )
            return i+1;                             // IDs start at 1

    return ID_FUNCTION_GRP_ADDINS;  // if not found, use Add-In group
}

inline BOOL IsTypeName( const rtl::OUString& rName, const uno::Type& rType )
{
    return rName == rType.getTypeName();
}

BOOL lcl_ValidReturnType( const uno::Reference<reflection::XIdlClass>& xClass )
{
    //  this must match with ScUnoAddInCall::SetResult

    if ( !xClass.is() ) return FALSE;

    switch (xClass->getTypeClass())
    {
        // case uno::TypeClass_VOID:
        //  ???

        case uno::TypeClass_ANY:                // variable type
        case uno::TypeClass_ENUM:               //! ???
        case uno::TypeClass_BOOLEAN:
        case uno::TypeClass_CHAR:
        case uno::TypeClass_BYTE:
        case uno::TypeClass_SHORT:
        case uno::TypeClass_UNSIGNED_SHORT:
        case uno::TypeClass_LONG:
        case uno::TypeClass_UNSIGNED_LONG:
        case uno::TypeClass_FLOAT:
        case uno::TypeClass_DOUBLE:
        case uno::TypeClass_STRING:
            return TRUE;                        // values or string

        case uno::TypeClass_INTERFACE:
            {
                //  return type XInterface may contain a XVolatileResult
                //! XIdlClass needs getType() method!

                rtl::OUString sName = xClass->getName();
                return (
                    IsTypeName( sName, getCppuType((uno::Reference<sheet::XVolatileResult>*)0) ) ||
                    IsTypeName( sName, getCppuType((uno::Reference<uno::XInterface>*)0) ) );
            }

        default:
            {
                //  nested sequences for arrays
                //! XIdlClass needs getType() method!

                rtl::OUString sName = xClass->getName();
                return (
                    IsTypeName( sName, getCppuType((uno::Sequence< uno::Sequence<INT32> >*)0) ) ||
                    IsTypeName( sName, getCppuType((uno::Sequence< uno::Sequence<double> >*)0) ) ||
                    IsTypeName( sName, getCppuType((uno::Sequence< uno::Sequence<rtl::OUString> >*)0) ) ||
                    IsTypeName( sName, getCppuType((uno::Sequence< uno::Sequence<uno::Any> >*)0) ) );
            }
    }
    return FALSE;
}

ScAddInArgumentType lcl_GetArgType( const uno::Reference<reflection::XIdlClass>& xClass )
{
    if (!xClass.is())
        return SC_ADDINARG_NONE;

    uno::TypeClass eType = xClass->getTypeClass();

    if ( eType == uno::TypeClass_LONG )             //! other integer types?
        return SC_ADDINARG_INTEGER;

    if ( eType == uno::TypeClass_DOUBLE )
        return SC_ADDINARG_DOUBLE;

    if ( eType == uno::TypeClass_STRING )
        return SC_ADDINARG_STRING;

    //! XIdlClass needs getType() method!
    rtl::OUString sName = xClass->getName();

    if (IsTypeName( sName, getCppuType((uno::Sequence< uno::Sequence<INT32> >*)0) ))
        return SC_ADDINARG_INTEGER_ARRAY;

    if (IsTypeName( sName, getCppuType((uno::Sequence< uno::Sequence<double> >*)0) ))
        return SC_ADDINARG_DOUBLE_ARRAY;

    if (IsTypeName( sName, getCppuType((uno::Sequence< uno::Sequence<rtl::OUString> >*)0) ))
        return SC_ADDINARG_STRING_ARRAY;

    if (IsTypeName( sName, getCppuType((uno::Sequence< uno::Sequence<uno::Any> >*)0) ))
        return SC_ADDINARG_MIXED_ARRAY;

    if (IsTypeName( sName, getCppuType((uno::Any*)0) ))
        return SC_ADDINARG_VALUE_OR_ARRAY;

    if (IsTypeName( sName, getCppuType((uno::Reference<table::XCellRange>*)0) ))
        return SC_ADDINARG_CELLRANGE;

    if (IsTypeName( sName, getCppuType((uno::Reference<beans::XPropertySet>*)0) ))
        return SC_ADDINARG_CALLER;

    if (IsTypeName( sName, getCppuType((uno::Sequence<uno::Any>*)0) ))
        return SC_ADDINARG_VARARGS;

    return SC_ADDINARG_NONE;
}

void ScUnoAddInCollection::ReadFromAddIn( const uno::Reference<uno::XInterface>& xInterface )
{
    uno::Reference<sheet::XAddIn> xAddIn( xInterface, uno::UNO_QUERY );
    uno::Reference<lang::XServiceName> xName( xInterface, uno::UNO_QUERY );
    if ( xAddIn.is() && xName.is() )
    {
        //  AddIns must use the language for which the office is installed
        LanguageType eOfficeLang = Application::GetAppInternational().GetLanguage();
        if ( eOfficeLang == LANGUAGE_SYSTEM )
            eOfficeLang = System::GetLanguage();

        String aLanguage, aCountry;
        ConvertLanguageToIsoNames( eOfficeLang, aLanguage, aCountry );

        rtl::OUString aEmpty;
        lang::Locale aLocale(  aLanguage, aCountry, aEmpty );
        xAddIn->setLocale( aLocale );

        String aServiceName = String( xName->getServiceName() );

        //! pass XIntrospection to ReadFromAddIn

        uno::Reference<lang::XMultiServiceFactory> xManager = comphelper::getProcessServiceFactory();
        if ( xManager.is() )
        {
            uno::Reference<beans::XIntrospection> xIntro(
                                    xManager->createInstance(rtl::OUString::createFromAscii(
                                        "com.sun.star.beans.Introspection" )),
                                    uno::UNO_QUERY );
            if ( xIntro.is() )
            {
                uno::Any aObject;
                aObject <<= xAddIn;
                uno::Reference<beans::XIntrospectionAccess> xAcc = xIntro->inspect(aObject);
                if (xAcc.is())
                {
                    uno::Sequence< uno::Reference<reflection::XIdlMethod> > aMethods =
                            xAcc->getMethods( beans::MethodConcept::ALL );
                    long nNewCount = aMethods.getLength();
                    if ( nNewCount )
                    {
                        long nOld = nFuncCount;
                        nFuncCount = nNewCount+nOld;
                        if ( nOld )
                        {
                            ScUnoAddInFuncData** ppNew = new ScUnoAddInFuncData*[nFuncCount];
                            for (long i=0; i<nOld; i++)
                                ppNew[i] = ppFuncData[i];
                            delete[] ppFuncData;
                            ppFuncData = ppNew;
                        }
                        else
                            ppFuncData = new ScUnoAddInFuncData*[nFuncCount];

                        const uno::Reference<reflection::XIdlMethod>* pArray = aMethods.getConstArray();
                        for (long nFuncPos=0; nFuncPos<nNewCount; nFuncPos++)
                        {
                            ppFuncData[nFuncPos+nOld] = NULL;

                            uno::Reference<reflection::XIdlMethod> xFunc = pArray[nFuncPos];
                            if (xFunc.is())
                            {
                                //  leave out internal functions
                                uno::Reference<reflection::XIdlClass> xClass =
                                                xFunc->getDeclaringClass();
                                BOOL bSkip = TRUE;
                                if ( xClass.is() )
                                {
                                    //! XIdlClass needs getType() method!
                                    rtl::OUString sName = xClass->getName();
                                    bSkip = (
                                        IsTypeName( sName,
                                            getCppuType((uno::Reference<uno::XInterface>*)0) ) ||
                                        IsTypeName( sName,
                                            getCppuType((uno::Reference<reflection::XIdlClassProvider>*)0) ) ||
                                        IsTypeName( sName,
                                            getCppuType((uno::Reference<lang::XServiceName>*)0) ) ||
                                        IsTypeName( sName,
                                            getCppuType((uno::Reference<lang::XServiceInfo>*)0) ) ||
                                        IsTypeName( sName,
                                            getCppuType((uno::Reference<sheet::XAddIn>*)0) ) );
                                }
                                if (!bSkip)
                                {
                                    uno::Reference<reflection::XIdlClass> xReturn =
                                                xFunc->getReturnType();
                                    if ( !lcl_ValidReturnType( xReturn ) )
                                        bSkip = TRUE;
                                }
                                if (!bSkip)
                                {
                                    rtl::OUString aFuncU = xFunc->getName();

                                    // stored function name: (service name).(function)
                                    String aFuncName = aServiceName;
                                    aFuncName += '.';
                                    aFuncName += String( aFuncU );

                                    BOOL bValid = TRUE;
                                    long nVisibleCount = 0;
                                    long nCallerPos = SC_CALLERPOS_NONE;

                                    uno::Sequence<reflection::ParamInfo> aParams =
                                            xFunc->getParameterInfos();
                                    long nParamCount = aParams.getLength();
                                    const reflection::ParamInfo* pParArr = aParams.getConstArray();
                                    long nParamPos;
                                    for (nParamPos=0; nParamPos<nParamCount; nParamPos++)
                                    {
                                        if ( pParArr[nParamPos].aMode != reflection::ParamMode_IN )
                                            bValid = FALSE;
                                        uno::Reference<reflection::XIdlClass> xParClass =
                                                    pParArr[nParamPos].aType;
                                        ScAddInArgumentType eArgType = lcl_GetArgType( xParClass );
                                        if ( eArgType == SC_ADDINARG_NONE )
                                            bValid = FALSE;
                                        else if ( eArgType == SC_ADDINARG_CALLER )
                                            nCallerPos = nParamPos;
                                        else
                                            ++nVisibleCount;
                                    }
                                    if (bValid)
                                    {
                                        USHORT nCategory = lcl_GetCategory(
                                            String(
                                            xAddIn->getProgrammaticCategoryName(
                                            aFuncU ) ) );

                                        rtl::OUString aLocalU;
                                        try
                                        {
                                            aLocalU = xAddIn->
                                                getDisplayFunctionName( aFuncU );
                                        }
                                        catch(uno::Exception&)
                                        {
                                            aLocalU = rtl::OUString::createFromAscii( "###" );
                                        }
                                        String aLocalName = String( aLocalU );

                                        rtl::OUString aDescU;
                                        try
                                        {
                                            aDescU = xAddIn->
                                                getFunctionDescription( aFuncU );
                                        }
                                        catch(uno::Exception&)
                                        {
                                            aDescU = rtl::OUString::createFromAscii( "###" );
                                        }
                                        String aDescription = String( aDescU );

                                        ScAddInArgDesc* pVisibleArgs = NULL;
                                        if ( nVisibleCount > 0 )
                                        {
                                            ScAddInArgDesc aDesc;
                                            pVisibleArgs = new ScAddInArgDesc[nVisibleCount];
                                            long nDestPos = 0;
                                            for (nParamPos=0; nParamPos<nParamCount; nParamPos++)
                                            {
                                                uno::Reference<reflection::XIdlClass> xParClass =
                                                    pParArr[nParamPos].aType;
                                                ScAddInArgumentType eArgType = lcl_GetArgType( xParClass );
                                                if ( eArgType != SC_ADDINARG_CALLER )
                                                {
                                                    rtl::OUString aArgName;
                                                    try
                                                    {
                                                        aArgName = xAddIn->
                                                            getDisplayArgumentName( aFuncU, nParamPos );
                                                    }
                                                    catch(uno::Exception&)
                                                    {
                                                        aArgName = rtl::OUString::createFromAscii( "###" );
                                                    }
                                                    rtl::OUString aArgDesc;
                                                    try
                                                    {
                                                        aArgDesc = xAddIn->
                                                            getArgumentDescription( aFuncU, nParamPos );
                                                    }
                                                    catch(uno::Exception&)
                                                    {
                                                        aArgName = rtl::OUString::createFromAscii( "###" );
                                                    }

                                                    BOOL bOptional =
                                                        ( eArgType == SC_ADDINARG_VALUE_OR_ARRAY ||
                                                          eArgType == SC_ADDINARG_VARARGS );

                                                    aDesc.eType = eArgType;
                                                    aDesc.aName = String( aArgName );
                                                    aDesc.aDescription = String( aArgDesc );
                                                    aDesc.bOptional = bOptional;

                                                    pVisibleArgs[nDestPos++] = aDesc;
                                                }
                                            }
                                            DBG_ASSERT( nDestPos==nVisibleCount, "wrong count" );
                                        }

                                        ppFuncData[nFuncPos+nOld] = new ScUnoAddInFuncData(
                                            aFuncName, aLocalName, aDescription, nCategory,
                                            xFunc, aObject,
                                            nVisibleCount, pVisibleArgs, nCallerPos );

                                        delete[] pVisibleArgs;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

String ScUnoAddInCollection::FindFunction( const String& rName, BOOL bLocalFirst )
{
    if (!bInitialized)
        Initialize();

    if (nFuncCount == 0)
        return EMPTY_STRING;

    String aUpperCmp = rName;
    ScGlobal::pCharClass->toUpper(aUpperCmp);
    long i;

    if ( bLocalFirst )
    {
        //  first scan all local names (used for entering formulas)

        for (i=0; i<nFuncCount; i++)
            if ( ppFuncData[i] && ppFuncData[i]->GetUpperLocal() == aUpperCmp )
                return ppFuncData[i]->GetOriginalName();

#if 0
        //  after that, scan international names (really?)

        for (i=0; i<nFuncCount; i++)
            if ( ppFuncData[i] && ppFuncData[i]->GetUpperName() == aUpperCmp )
                return ppFuncData[i]->GetOriginalName();
#endif
    }
    else
    {
        //  first scan international names (used when calling a function)
        //! before that, check for exact match???

        for (i=0; i<nFuncCount; i++)
            if ( ppFuncData[i] && ppFuncData[i]->GetUpperName() == aUpperCmp )
                return ppFuncData[i]->GetOriginalName();

        //  after that, scan all local names (to allow replacing old AddIns with Uno)

        for (i=0; i<nFuncCount; i++)
            if ( ppFuncData[i] && ppFuncData[i]->GetUpperLocal() == aUpperCmp )
                return ppFuncData[i]->GetOriginalName();
    }

    return EMPTY_STRING;
}

ScUnoAddInFuncData* ScUnoAddInCollection::GetFuncData( const String& rName )
{
    if (!bInitialized)
        Initialize();

    //  rName must be the exact internal name

    for (long i=0; i<nFuncCount; i++)
        if ( ppFuncData[i] && ppFuncData[i]->GetOriginalName() == rName )
            return ppFuncData[i];

    return NULL;
}

void ScUnoAddInCollection::LocalizeString( String& rName )
{
    if (!bInitialized)
        Initialize();

    //  modify rName - input: exact name

    for (long i=0; i<nFuncCount; i++)
        if ( ppFuncData[i] && ppFuncData[i]->GetOriginalName() == rName )
        {
            rName = ppFuncData[i]->GetUpperLocal();         //! upper?
            return;
        }
}


long ScUnoAddInCollection::GetFuncCount()
{
    if (!bInitialized)
        Initialize();

    return nFuncCount;
}

BOOL ScUnoAddInCollection::FillFunctionDesc( long nFunc, ScFuncDesc& rDesc )
{
    //  rDesc must me freshly allocated (no arguments set)
    DBG_ASSERT( !rDesc.aDefArgNames, "FuncDesc is not empty" );

    if (!bInitialized)
        Initialize();

    if (nFunc >= nFuncCount || !ppFuncData[nFunc])
        return FALSE;

    long nArgCount = ppFuncData[nFunc]->GetArgumentCount();
    if ( nArgCount > USHRT_MAX )
        return FALSE;

    // nFIndex is set from outside

    rDesc.pFuncName = new String( ppFuncData[nFunc]->GetUpperLocal() );     //! upper?
    rDesc.nCategory = ppFuncData[nFunc]->GetCategory();

    String aDesc = ppFuncData[nFunc]->GetDescription();
    if (!aDesc.Len())
        aDesc = ppFuncData[nFunc]->GetLocalName();      // use name if no description is available
    rDesc.pFuncDesc = new String( aDesc );

    // AddInArgumentType_CALLER is already left out in FuncData

    rDesc.nArgCount = (USHORT)nArgCount;
    if ( nArgCount )
    {
        BOOL bMultiple = FALSE;
        const ScAddInArgDesc* pArgs = ppFuncData[nFunc]->GetArguments();

        rDesc.aDefArgNames = new String*[nArgCount];
        rDesc.aDefArgDescs = new String*[nArgCount];
        rDesc.aDefArgOpt   = new BOOL[nArgCount];
        for ( long nArg=0; nArg<nArgCount; nArg++ )
        {
            rDesc.aDefArgNames[nArg] = new String( pArgs[nArg].aName );
            rDesc.aDefArgDescs[nArg] = new String( pArgs[nArg].aDescription );
            rDesc.aDefArgOpt[nArg] = pArgs[nArg].bOptional;

            // no empty names...
            if ( rDesc.aDefArgNames[nArg]->Len() == 0 )
            {
                String aDefName( RTL_CONSTASCII_USTRINGPARAM("arg") );
                aDefName += String::CreateFromInt32( nArg+1 );
                *rDesc.aDefArgNames[nArg] = aDefName;
            }

            //  last argument repeated?
            if ( nArg+1 == nArgCount && ( pArgs[nArg].eType == SC_ADDINARG_VARARGS ) )
                bMultiple = TRUE;
        }

        if ( bMultiple )
            rDesc.nArgCount += VAR_ARGS - 1;    // VAR_ARGS means just one repeated arg
    }

    return TRUE;
}


//------------------------------------------------------------------------

ScUnoAddInCall::ScUnoAddInCall( ScUnoAddInCollection& rColl, const String& rName,
                                long nParamCount ) :
    nErrCode( errNoCode ),      // before function was called
    bHasString( TRUE ),
    fValue( 0.0 ),
    pMatrix( NULL ),
    bValidCount( FALSE )
{
    pFuncData = rColl.GetFuncData( rName );
    DBG_ASSERT( pFuncData, "Function Data missing" );
    if ( pFuncData )
    {
        long nDescCount = pFuncData->GetArgumentCount();
        const ScAddInArgDesc* pArgs = pFuncData->GetArguments();
        long nVarCount = 0;

        //  is aVarArg sequence needed?
        if ( nParamCount >= nDescCount && nDescCount > 0 &&
             pArgs[nDescCount-1].eType == SC_ADDINARG_VARARGS )
        {
            long nVarCount = nParamCount - ( nDescCount - 1 );  // size of last argument
            aVarArg.realloc( nVarCount );
            bValidCount = TRUE;
        }
        else if ( nParamCount <= nDescCount )
        {
            //  all args behind nParamCount must be optional
            bValidCount = TRUE;
            for (long i=nParamCount; i<nDescCount; i++)
                if ( !pArgs[i].bOptional )
                    bValidCount = FALSE;
        }
        // else invalid (too many arguments)

        if ( bValidCount )
            aArgs.realloc( nDescCount );    // sequence must always match function signature
    }
}

ScUnoAddInCall::~ScUnoAddInCall()
{
    // pFuncData is deleted with ScUnoAddInCollection

    delete pMatrix;
}

BOOL ScUnoAddInCall::ValidParamCount()
{
    return bValidCount;
}

ScAddInArgumentType ScUnoAddInCall::GetArgType( long nPos )
{
    if ( pFuncData )
    {
        long nCount = pFuncData->GetArgumentCount();
        const ScAddInArgDesc* pArgs = pFuncData->GetArguments();

        // if last arg is sequence, use "any" type
        if ( nCount > 0 && nPos >= nCount-1 && pArgs[nCount-1].eType == SC_ADDINARG_VARARGS )
            return SC_ADDINARG_VALUE_OR_ARRAY;

        if ( nPos < nCount )
            return pArgs[nPos].eType;
    }
    return SC_ADDINARG_VALUE_OR_ARRAY;      //! error code !!!!
}

BOOL ScUnoAddInCall::NeedsCaller() const
{
    return pFuncData && pFuncData->GetCallerPos() != SC_CALLERPOS_NONE;
}

void ScUnoAddInCall::SetCaller( const uno::Reference<uno::XInterface>& rInterface )
{
    xCaller = rInterface;
}

void ScUnoAddInCall::SetCallerFromObjectShell( SfxObjectShell* pObjSh )
{
    if (pObjSh)
    {
        uno::Reference<uno::XInterface> xInt( pObjSh->GetBaseModel(), uno::UNO_QUERY );
        SetCaller( xInt );
    }
}

void ScUnoAddInCall::SetParam( long nPos, const uno::Any& rValue )
{
    if ( pFuncData )
    {
        long nCount = pFuncData->GetArgumentCount();
        const ScAddInArgDesc* pArgs = pFuncData->GetArguments();
        if ( nCount > 0 && nPos >= nCount-1 && pArgs[nCount-1].eType == SC_ADDINARG_VARARGS )
        {
            long nVarPos = nPos-(nCount-1);
            if ( nVarPos < aVarArg.getLength() )
                aVarArg.getArray()[nVarPos] = rValue;
            else
                DBG_ERROR("wrong argument number");
        }
        else if ( nPos < aArgs.getLength() )
            aArgs.getArray()[nPos] = rValue;
        else
            DBG_ERROR("wrong argument number");
    }
}

void ScUnoAddInCall::ExecuteCall()
{
    if ( !pFuncData )
        return;

    long nCount = pFuncData->GetArgumentCount();
    const ScAddInArgDesc* pArgs = pFuncData->GetArguments();
    if ( nCount > 0 && pArgs[nCount-1].eType == SC_ADDINARG_VARARGS )
    {
        //  insert aVarArg as last argument
        //! after inserting caller (to prevent copying twice)?

        DBG_ASSERT( aArgs.getLength() == nCount, "wrong argument count" );
        aArgs.getArray()[nCount-1] <<= aVarArg;
    }

    if ( pFuncData->GetCallerPos() != SC_CALLERPOS_NONE )
    {
        uno::Any aCallerAny;
        aCallerAny <<= xCaller;

        long nUserLen = aArgs.getLength();
        long nCallPos = pFuncData->GetCallerPos();
        if (nCallPos>nUserLen)                          // should not happen
        {
            DBG_ERROR("wrong CallPos");
            nCallPos = nUserLen;
        }

        long nDestLen = nUserLen + 1;
        uno::Sequence<uno::Any> aRealArgs( nDestLen );
        uno::Any* pDest = aRealArgs.getArray();

        const uno::Any* pSource = aArgs.getConstArray();
        long nSrcPos = 0;

        for ( long nDestPos = 0; nDestPos < nDestLen; nDestPos++ )
        {
            if ( nDestPos == nCallPos )
                pDest[nDestPos] = aCallerAny;
            else
                pDest[nDestPos] = pSource[nSrcPos++];
        }

        ExecuteCallWithArgs( aRealArgs );
    }
    else
        ExecuteCallWithArgs( aArgs );
}

void ScUnoAddInCall::ExecuteCallWithArgs(uno::Sequence<uno::Any>& rCallArgs)
{
    //  rCallArgs may not match argument descriptions (because of caller)

    uno::Reference<reflection::XIdlMethod> xFunction;
    uno::Any aObject;
    if ( pFuncData )
    {
        xFunction = pFuncData->GetFunction();
        aObject = pFuncData->GetObject();
    }

    if ( xFunction.is() )
    {
        uno::Any aAny;
        nErrCode = 0;

        try
        {
            aAny = xFunction->invoke( aObject, rCallArgs );
        }
        catch(lang::IllegalArgumentException&)
        {
            nErrCode = errIllegalArgument;
        }
#if 0
        catch(FloatingPointException&)
        {
            nErrCode = errIllegalFPOperation;
        }
#endif
        catch(uno::Exception&)
        {
            nErrCode = errNoValue;
        }

        if (!nErrCode)
            SetResult( aAny );      // convert result to Calc types
    }
}

void ScUnoAddInCall::SetResult( const uno::Any& rNewRes )
{
    nErrCode = 0;
    xVarRes = NULL;

    // Reflection* pRefl = rNewRes.getReflection();

    uno::TypeClass eClass = rNewRes.getValueTypeClass();
    uno::Type aType = rNewRes.getValueType();
    switch (eClass)
    {
        case uno::TypeClass_VOID:
            nErrCode = NOVALUE;         // #nv
            break;

        case uno::TypeClass_ENUM:
        case uno::TypeClass_BOOLEAN:
        case uno::TypeClass_CHAR:
        case uno::TypeClass_BYTE:
        case uno::TypeClass_SHORT:
        case uno::TypeClass_UNSIGNED_SHORT:
        case uno::TypeClass_LONG:
        case uno::TypeClass_UNSIGNED_LONG:
        case uno::TypeClass_FLOAT:
        case uno::TypeClass_DOUBLE:
            lcl_ConvertToDouble( rNewRes, fValue );
            bHasString = FALSE;
            break;

        case uno::TypeClass_STRING:
            {
                rtl::OUString aUStr;
                rNewRes >>= aUStr;
                aString = String( aUStr );
                bHasString = TRUE;
            }
            break;

        case uno::TypeClass_INTERFACE:
            {
                //! directly extract XVolatileResult from any?
                uno::Reference<uno::XInterface> xInterface;
                rNewRes >>= xInterface;
                if ( xInterface.is() )
                    xVarRes = uno::Reference<sheet::XVolatileResult>( xInterface, uno::UNO_QUERY );

                if (!xVarRes.is())
                    nErrCode = errNoValue;          // unknown interface
            }
            break;

        default:
            if ( aType.equals( getCppuType( (uno::Sequence< uno::Sequence<INT32> > *)0 ) ) )
            {
                const uno::Sequence< uno::Sequence<INT32> >* pRowSeq = NULL;

                //! use pointer from any!
                uno::Sequence< uno::Sequence<INT32> > aSequence;
                if ( rNewRes >>= aSequence )
                    pRowSeq = &aSequence;

                if ( pRowSeq )
                {
                    long nRowCount = pRowSeq->getLength();
                    const uno::Sequence<INT32>* pRowArr = pRowSeq->getConstArray();
                    long nMaxColCount = 0;
                    long nCol, nRow;
                    for (nRow=0; nRow<nRowCount; nRow++)
                    {
                        long nTmp = pRowArr[nRow].getLength();
                        if ( nTmp > nMaxColCount )
                            nMaxColCount = nTmp;
                    }
                    if ( nMaxColCount && nRowCount )
                    {
                        pMatrix = new ScMatrix( (USHORT)nMaxColCount, (USHORT)nRowCount );
                        for (nRow=0; nRow<nRowCount; nRow++)
                        {
                            long nColCount = pRowArr[nRow].getLength();
                            const INT32* pColArr = pRowArr[nRow].getConstArray();
                            for (nCol=0; nCol<nColCount; nCol++)
                                pMatrix->PutDouble( pColArr[nCol], (USHORT)nCol, (USHORT)nRow );
                            for (nCol=nColCount; nCol<nMaxColCount; nCol++)
                                pMatrix->PutDouble( 0.0, (USHORT)nCol, (USHORT)nRow );
                        }
                    }
                }
            }
            else if ( aType.equals( getCppuType( (uno::Sequence< uno::Sequence<double> > *)0 ) ) )
            {
                const uno::Sequence< uno::Sequence<double> >* pRowSeq = NULL;

                //! use pointer from any!
                uno::Sequence< uno::Sequence<double> > aSequence;
                if ( rNewRes >>= aSequence )
                    pRowSeq = &aSequence;

                if ( pRowSeq )
                {
                    long nRowCount = pRowSeq->getLength();
                    const uno::Sequence<double>* pRowArr = pRowSeq->getConstArray();
                    long nMaxColCount = 0;
                    long nCol, nRow;
                    for (nRow=0; nRow<nRowCount; nRow++)
                    {
                        long nTmp = pRowArr[nRow].getLength();
                        if ( nTmp > nMaxColCount )
                            nMaxColCount = nTmp;
                    }
                    if ( nMaxColCount && nRowCount )
                    {
                        pMatrix = new ScMatrix( (USHORT)nMaxColCount, (USHORT)nRowCount );
                        for (nRow=0; nRow<nRowCount; nRow++)
                        {
                            long nColCount = pRowArr[nRow].getLength();
                            const double* pColArr = pRowArr[nRow].getConstArray();
                            for (nCol=0; nCol<nColCount; nCol++)
                                pMatrix->PutDouble( pColArr[nCol], (USHORT)nCol, (USHORT)nRow );
                            for (nCol=nColCount; nCol<nMaxColCount; nCol++)
                                pMatrix->PutDouble( 0.0, (USHORT)nCol, (USHORT)nRow );
                        }
                    }
                }
            }
            else if ( aType.equals( getCppuType( (uno::Sequence< uno::Sequence<rtl::OUString> > *)0 ) ) )
            {
                const uno::Sequence< uno::Sequence<rtl::OUString> >* pRowSeq = NULL;

                //! use pointer from any!
                uno::Sequence< uno::Sequence<rtl::OUString> > aSequence;
                if ( rNewRes >>= aSequence )
                    pRowSeq = &aSequence;

                if ( pRowSeq )
                {
                    long nRowCount = pRowSeq->getLength();
                    const uno::Sequence<rtl::OUString>* pRowArr = pRowSeq->getConstArray();
                    long nMaxColCount = 0;
                    long nCol, nRow;
                    for (nRow=0; nRow<nRowCount; nRow++)
                    {
                        long nTmp = pRowArr[nRow].getLength();
                        if ( nTmp > nMaxColCount )
                            nMaxColCount = nTmp;
                    }
                    if ( nMaxColCount && nRowCount )
                    {
                        pMatrix = new ScMatrix( (USHORT)nMaxColCount, (USHORT)nRowCount );
                        for (nRow=0; nRow<nRowCount; nRow++)
                        {
                            long nColCount = pRowArr[nRow].getLength();
                            const rtl::OUString* pColArr = pRowArr[nRow].getConstArray();
                            for (nCol=0; nCol<nColCount; nCol++)
                                pMatrix->PutString( String( pColArr[nCol] ),
                                    (USHORT)nCol, (USHORT)nRow );
                            for (nCol=nColCount; nCol<nMaxColCount; nCol++)
                                pMatrix->PutString( EMPTY_STRING, (USHORT)nCol, (USHORT)nRow );
                        }
                    }
                }
            }
            else if ( aType.equals( getCppuType( (uno::Sequence< uno::Sequence<uno::Any> > *)0 ) ) )
            {
                const uno::Sequence< uno::Sequence<uno::Any> >* pRowSeq = NULL;

                //! use pointer from any!
                uno::Sequence< uno::Sequence<uno::Any> > aSequence;
                if ( rNewRes >>= aSequence )
                    pRowSeq = &aSequence;

                if ( pRowSeq )
                {
                    long nRowCount = pRowSeq->getLength();
                    const uno::Sequence<uno::Any>* pRowArr = pRowSeq->getConstArray();
                    long nMaxColCount = 0;
                    long nCol, nRow;
                    for (nRow=0; nRow<nRowCount; nRow++)
                    {
                        long nTmp = pRowArr[nRow].getLength();
                        if ( nTmp > nMaxColCount )
                            nMaxColCount = nTmp;
                    }
                    if ( nMaxColCount && nRowCount )
                    {
                        rtl::OUString aUStr;
                        pMatrix = new ScMatrix( (USHORT)nMaxColCount, (USHORT)nRowCount );
                        for (nRow=0; nRow<nRowCount; nRow++)
                        {
                            long nColCount = pRowArr[nRow].getLength();
                            const uno::Any* pColArr = pRowArr[nRow].getConstArray();
                            for (nCol=0; nCol<nColCount; nCol++)
                            {
                                //Reflection* pRefl = pColArr[nCol].getReflection();
                                //if ( pRefl->equals( *OUString_getReflection() ) )
                                if ( pColArr[nCol] >>= aUStr )
                                    pMatrix->PutString( String( aUStr ),
                                        (USHORT)nCol, (USHORT)nRow );
                                else
                                {
                                    // try to convert to double, empty if not possible

                                    double fCellVal;
                                    if ( lcl_ConvertToDouble( pColArr[nCol], fCellVal ) )
                                        pMatrix->PutDouble( fCellVal, (USHORT)nCol, (USHORT)nRow );
                                    else
                                        pMatrix->PutEmpty( (USHORT)nCol, (USHORT)nRow );
                                }
                            }
                            for (nCol=nColCount; nCol<nMaxColCount; nCol++)
                                pMatrix->PutString( EMPTY_STRING, (USHORT)nCol, (USHORT)nRow );
                        }
                    }
                }
            }

            if (!pMatrix)                       // no array found
                nErrCode = errNoValue;          //! code for error in return type???
    }
}



//------------------------------------------------------------------------



