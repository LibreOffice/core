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
#include <i18npool/mslangid.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/objsh.hxx>
#include <unotools/charclass.hxx>

#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/reflection/XIdlClass.hpp>
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#include <com/sun/star/beans/XIntrospection.hpp>
#include <com/sun/star/beans/MethodConcept.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/sheet/XCompatibilityNames.hpp>
#include <com/sun/star/sheet/NoConvergenceException.hpp>

#include "addincol.hxx"
#include "addinhelpid.hxx"
#include "compiler.hxx"
#include "scmatrix.hxx"
#include "addinlis.hxx"
#include "formula/errorcodes.hxx"
#include "scfuncs.hrc"
#include "optutil.hxx"
#include "addincfg.hxx"
#include "scmod.hxx"
#include "rangeseq.hxx"
#include "funcdesc.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

#define SC_CALLERPOS_NONE   (-1)

#define SCADDINSUPPLIER_SERVICE "com.sun.star.sheet.AddIn"

//------------------------------------------------------------------------




//------------------------------------------------------------------------

ScUnoAddInFuncData::ScUnoAddInFuncData( const ::rtl::OUString& rNam, const ::rtl::OUString& rLoc,
                                        const ::rtl::OUString& rDesc,
                                        sal_uInt16 nCat, const rtl::OString& sHelp,
                                        const uno::Reference<reflection::XIdlMethod>& rFunc,
                                        const uno::Any& rO,
                                        long nAC, const ScAddInArgDesc* pAD,
                                        long nCP ) :
    aOriginalName( rNam ),
    aLocalName( rLoc ),
    aUpperName( rNam ),
    aUpperLocal( rLoc ),
    aDescription( rDesc ),
    xFunction( rFunc ),
    aObject( rO ),
    nArgCount( nAC ),
    nCallerPos( nCP ),
    nCategory( nCat ),
    sHelpId( sHelp ),
    bCompInitialized( false )
{
    if ( nArgCount )
    {
        pArgDescs = new ScAddInArgDesc[nArgCount];
        for (long i=0; i<nArgCount; i++)
            pArgDescs[i] = pAD[i];
    }
    else
        pArgDescs = NULL;

    aUpperName = ScGlobal::pCharClass->uppercase(aUpperName);
    aUpperLocal = ScGlobal::pCharClass->uppercase(aUpperLocal);
}

ScUnoAddInFuncData::~ScUnoAddInFuncData()
{
    delete[] pArgDescs;
}

const uno::Sequence<sheet::LocalizedName>& ScUnoAddInFuncData::GetCompNames() const
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
                        rLocale.Language = rLocale.Language.toAsciiLowerCase();
                        rLocale.Country  = rLocale.Country.toAsciiUpperCase();
                    }
                }
            }
        }

        bCompInitialized = sal_True;        // also if not successful
    }
    return aCompNames;
}

void ScUnoAddInFuncData::SetCompNames( const uno::Sequence< sheet::LocalizedName>& rNew )
{
    OSL_ENSURE( !bCompInitialized, "SetCompNames after initializing" );

    aCompNames = rNew;

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
            rLocale.Language = rLocale.Language.toAsciiLowerCase();
            rLocale.Country  = rLocale.Country.toAsciiUpperCase();
        }
    }

    bCompInitialized = sal_True;
}

sal_Bool ScUnoAddInFuncData::GetExcelName( LanguageType eDestLang, ::rtl::OUString& rRetExcelName ) const
{
    const uno::Sequence<sheet::LocalizedName>& rSequence = GetCompNames();
    long nSeqLen = rSequence.getLength();
    if ( nSeqLen )
    {
        const sheet::LocalizedName* pArray = rSequence.getConstArray();
        long i;

        rtl::OUString aLangStr, aCountryStr;
        MsLangId::convertLanguageToIsoNames( eDestLang, aLangStr, aCountryStr );
        rtl::OUString aUserLang = aLangStr.toAsciiLowerCase();
        rtl::OUString aUserCountry = aCountryStr.toAsciiUpperCase();

        //  first check for match of both language and country

        for ( i=0; i<nSeqLen; i++)
            if ( pArray[i].Locale.Language == aUserLang &&
                    pArray[i].Locale.Country  == aUserCountry )
            {
                rRetExcelName = pArray[i].Name;
                return sal_True;
            }

        //  second: check only language

        for ( i=0; i<nSeqLen; i++)
            if ( pArray[i].Locale.Language == aUserLang )
            {
                rRetExcelName = pArray[i].Name;
                return sal_True;
            }

        // third: #i57772# fall-back to en-US

        if ( eDestLang != LANGUAGE_ENGLISH_US )
            return GetExcelName( LANGUAGE_ENGLISH_US, rRetExcelName );

        //  forth: use first (default) entry

        rRetExcelName = pArray[0].Name;
        return sal_True;
    }
    return false;
}

void ScUnoAddInFuncData::SetFunction( const uno::Reference< reflection::XIdlMethod>& rNewFunc, const uno::Any& rNewObj )
{
    xFunction = rNewFunc;
    aObject = rNewObj;
}

void ScUnoAddInFuncData::SetArguments( long nNewCount, const ScAddInArgDesc* pNewDescs )
{
    delete[] pArgDescs;

    nArgCount = nNewCount;
    if ( nArgCount )
    {
        pArgDescs = new ScAddInArgDesc[nArgCount];
        for (long i=0; i<nArgCount; i++)
            pArgDescs[i] = pNewDescs[i];
    }
    else
        pArgDescs = NULL;
}

void ScUnoAddInFuncData::SetCallerPos( long nNewPos )
{
    nCallerPos = nNewPos;
}

//------------------------------------------------------------------------

ScUnoAddInCollection::ScUnoAddInCollection() :
    nFuncCount( 0 ),
    ppFuncData( NULL ),
    pExactHashMap( NULL ),
    pNameHashMap( NULL ),
    pLocalHashMap( NULL ),
    bInitialized( false )
{
}

ScUnoAddInCollection::~ScUnoAddInCollection()
{
    Clear();
}

void ScUnoAddInCollection::Clear()
{
    DELETEZ( pExactHashMap );
    DELETEZ( pNameHashMap );
    DELETEZ( pLocalHashMap );
    if ( ppFuncData )
    {
        for ( long i=0; i<nFuncCount; i++ )
            delete ppFuncData[i];
        delete[] ppFuncData;
    }
    ppFuncData = NULL;
    nFuncCount = 0;

    bInitialized = false;
}

void ScUnoAddInCollection::Initialize()
{
    OSL_ENSURE( !bInitialized, "Initialize twice?" );

    uno::Reference<lang::XMultiServiceFactory> xManager = comphelper::getProcessServiceFactory();
    uno::Reference<container::XContentEnumerationAccess> xEnAc( xManager, uno::UNO_QUERY );
    if ( xEnAc.is() )
    {
        uno::Reference<container::XEnumeration> xEnum =
                        xEnAc->createContentEnumeration(
                            rtl::OUString(SCADDINSUPPLIER_SERVICE) );
        if ( xEnum.is() )
        {
            //  loop through all AddIns
            while ( xEnum->hasMoreElements() )
            {
                uno::Any aAddInAny = xEnum->nextElement();

                {
                    uno::Reference<uno::XInterface> xIntFac;
                    aAddInAny >>= xIntFac;
                    if ( xIntFac.is() )
                    {
                        // #i59984# try XSingleComponentFactory in addition to (old) XSingleServiceFactory,
                        // passing the context to the component

                        uno::Reference<uno::XInterface> xInterface;
                        uno::Reference<uno::XComponentContext> xCtx(
                            comphelper::getComponentContext(xManager));
                        uno::Reference<lang::XSingleComponentFactory> xCFac( xIntFac, uno::UNO_QUERY );
                        if (xCFac.is())
                        {
                            xInterface = xCFac->createInstanceWithContext(xCtx);
                            if (xInterface.is())
                                ReadFromAddIn( xInterface );
                        }

                        if (!xInterface.is())
                        {
                            uno::Reference<lang::XSingleServiceFactory> xFac( xIntFac, uno::UNO_QUERY );
                            if ( xFac.is() )
                            {
                                xInterface = xFac->createInstance();
                                if (xInterface.is())
                                    ReadFromAddIn( xInterface );
                            }
                        }
                    }
                }
            }
        }
    }

    // ReadConfiguration is called after looking at the AddIn implementations.
    // Duplicated are skipped (by using the service information, they don't have to be updated again
    // when argument information is needed).
    ReadConfiguration();

    bInitialized = sal_True;        // with or without functions
}
// -----------------------------------------------------------------------------

sal_uInt16 lcl_GetCategory( const ::rtl::OUString& rName )
{
    static const sal_Char* aFuncNames[SC_FUNCGROUP_COUNT] =
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
    for (sal_uInt16 i=0; i<SC_FUNCGROUP_COUNT; i++)
        if ( rName.equalsAscii( aFuncNames[i] ) )
            return i+1;                             // IDs start at 1

    return ID_FUNCTION_GRP_ADDINS;  // if not found, use Add-In group
}


#define CFGPATH_ADDINS              "Office.CalcAddIns/AddInInfo"
#define CFGSTR_ADDINFUNCTIONS       "AddInFunctions"

#define CFG_FUNCPROP_DISPLAYNAME    0
#define CFG_FUNCPROP_DESCRIPTION    1
#define CFG_FUNCPROP_CATEGORY       2
#define CFG_FUNCPROP_COUNT          3
#define CFGSTR_DISPLAYNAME          "DisplayName"
#define CFGSTR_DESCRIPTION          "Description"
#define CFGSTR_CATEGORY             "Category"
// CategoryDisplayName is ignored for now

#define CFGSTR_COMPATIBILITYNAME    "CompatibilityName"
#define CFGSTR_PARAMETERS           "Parameters"


void ScUnoAddInCollection::ReadConfiguration()
{
    // called only from Initialize

    ScAddInCfg& rAddInConfig = SC_MOD()->GetAddInCfg();

    // additional, temporary config item for the compatibility names
    ScLinkConfigItem aAllLocalesConfig( rtl::OUString(CFGPATH_ADDINS), CONFIG_MODE_ALL_LOCALES );
    // CommitLink is not used (only reading values)

    const rtl::OUString sSlash('/');

    // get the list of add-ins (services)
    rtl::OUString aEmptyString;
    uno::Sequence<rtl::OUString> aServiceNames = rAddInConfig.GetNodeNames( aEmptyString );

    sal_Int32 nServiceCount = aServiceNames.getLength();
    for ( sal_Int32 nService = 0; nService < nServiceCount; nService++ )
    {
        rtl::OUString aServiceName = aServiceNames[nService];
        ScUnoAddInHelpIdGenerator aHelpIdGenerator( aServiceName );

        rtl::OUString aFunctionsPath = aServiceName;
        aFunctionsPath += sSlash;
        aFunctionsPath += rtl::OUString(CFGSTR_ADDINFUNCTIONS);

        uno::Sequence<rtl::OUString> aFunctionNames = rAddInConfig.GetNodeNames( aFunctionsPath );
        sal_Int32 nNewCount = aFunctionNames.getLength();

        // allocate pointers

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

        //! TODO: adjust bucket count?
        if ( !pExactHashMap )
            pExactHashMap = new ScAddInHashMap;
        if ( !pNameHashMap )
            pNameHashMap = new ScAddInHashMap;
        if ( !pLocalHashMap )
            pLocalHashMap = new ScAddInHashMap;

        //! get the function information in a single call for all functions?

        const rtl::OUString* pFuncNameArray = aFunctionNames.getConstArray();
        for ( sal_Int32 nFuncPos = 0; nFuncPos < nNewCount; nFuncPos++ )
        {
            ppFuncData[nFuncPos+nOld] = NULL;

            // stored function name: (service name).(function)
            rtl::OUStringBuffer aFuncNameBuffer( aServiceName.getLength()+1+pFuncNameArray[nFuncPos].getLength());
            aFuncNameBuffer.append(aServiceName);
            aFuncNameBuffer.append('.');
            aFuncNameBuffer.append(pFuncNameArray[nFuncPos]);
            rtl::OUString aFuncName = aFuncNameBuffer.makeStringAndClear();

            // skip the function if already known (read from old AddIn service)

            if ( pExactHashMap->find( aFuncName ) == pExactHashMap->end() )
            {
                rtl::OUString aLocalName;
                rtl::OUString aDescription;
                sal_uInt16 nCategory = ID_FUNCTION_GRP_ADDINS;

                // get direct information on the function

                rtl::OUString aFuncPropPath = aFunctionsPath;
                aFuncPropPath += sSlash;
                aFuncPropPath += pFuncNameArray[nFuncPos];
                aFuncPropPath += sSlash;

                uno::Sequence<rtl::OUString> aFuncPropNames(CFG_FUNCPROP_COUNT);
                rtl::OUString* pNameArray = aFuncPropNames.getArray();
                pNameArray[CFG_FUNCPROP_DISPLAYNAME] = aFuncPropPath;
                pNameArray[CFG_FUNCPROP_DISPLAYNAME] += rtl::OUString(CFGSTR_DISPLAYNAME);
                pNameArray[CFG_FUNCPROP_DESCRIPTION] = aFuncPropPath;
                pNameArray[CFG_FUNCPROP_DESCRIPTION] += rtl::OUString(CFGSTR_DESCRIPTION);
                pNameArray[CFG_FUNCPROP_CATEGORY] = aFuncPropPath;
                pNameArray[CFG_FUNCPROP_CATEGORY] += rtl::OUString(CFGSTR_CATEGORY);

                uno::Sequence<uno::Any> aFuncProperties = rAddInConfig.GetProperties( aFuncPropNames );
                if ( aFuncProperties.getLength() == CFG_FUNCPROP_COUNT )
                {
                    aFuncProperties[CFG_FUNCPROP_DISPLAYNAME] >>= aLocalName;
                    aFuncProperties[CFG_FUNCPROP_DESCRIPTION] >>= aDescription;

                    rtl::OUString aCategoryName;
                    aFuncProperties[CFG_FUNCPROP_CATEGORY] >>= aCategoryName;
                    nCategory = lcl_GetCategory( aCategoryName );
                }

                // get compatibility names

                uno::Sequence<sheet::LocalizedName> aCompNames;

                rtl::OUString aCompPath = aFuncPropPath;
                aCompPath += rtl::OUString(CFGSTR_COMPATIBILITYNAME);
                uno::Sequence<rtl::OUString> aCompPropNames( &aCompPath, 1 );

                uno::Sequence<uno::Any> aCompProperties = aAllLocalesConfig.GetProperties( aCompPropNames );
                if ( aCompProperties.getLength() == 1 )
                {
                    uno::Sequence<beans::PropertyValue> aLocalEntries;
                    if ( aCompProperties[0] >>= aLocalEntries )
                    {
                        sal_Int32 nLocaleCount = aLocalEntries.getLength();
                        aCompNames.realloc( nLocaleCount );
                        const beans::PropertyValue* pConfigArray = aLocalEntries.getConstArray();
                        sheet::LocalizedName* pCompArray = aCompNames.getArray();

                        for ( sal_Int32 nLocale = 0; nLocale < nLocaleCount; nLocale++ )
                        {
                            const sal_Unicode cLocaleSep = '-';     // separator in configuration locale strings

                            // PropertyValue name is the locale (convert from string to Locale struct)

                            const rtl::OUString& rLocaleStr = pConfigArray[nLocale].Name;
                            lang::Locale& rLocale = pCompArray[nLocale].Locale;
                            sal_Int32 nSepPos = rLocaleStr.indexOf( cLocaleSep );
                            if ( nSepPos >= 0 )
                            {
                                rLocale.Language = rLocaleStr.copy( 0, nSepPos );
                                rLocale.Country = rLocaleStr.copy( nSepPos+1 );
                            }
                            else
                                rLocale.Language = rLocaleStr;      // leave country empty (default ctor from sequence)

                            // PropertyValue value is the localized value (string in this case)

                            pConfigArray[nLocale].Value >>= pCompArray[nLocale].Name;
                        }
                    }
                }

                // get argument info

                ScAddInArgDesc* pVisibleArgs = NULL;
                long nVisibleCount = 0;
                long nCallerPos = SC_CALLERPOS_NONE;

                rtl::OUString aArgumentsPath = aFuncPropPath;
                aArgumentsPath += rtl::OUString(CFGSTR_PARAMETERS);

                uno::Sequence<rtl::OUString> aArgumentNames = rAddInConfig.GetNodeNames( aArgumentsPath );
                sal_Int32 nArgumentCount = aArgumentNames.getLength();
                if ( nArgumentCount )
                {
                    // get DisplayName and Description for each argument
                    uno::Sequence<rtl::OUString> aArgPropNames( nArgumentCount * 2 );
                    rtl::OUString* pPropNameArray = aArgPropNames.getArray();

                    sal_Int32 nArgument;
                    sal_Int32 nIndex = 0;
                    const rtl::OUString* pArgNameArray = aArgumentNames.getConstArray();
                    for ( nArgument = 0; nArgument < nArgumentCount; nArgument++ )
                    {
                        rtl::OUString aOneArgPath = aArgumentsPath;
                        aOneArgPath += sSlash;
                        aOneArgPath += pArgNameArray[nArgument];
                        aOneArgPath += sSlash;

                        pPropNameArray[nIndex] = aOneArgPath;
                        pPropNameArray[nIndex++] += rtl::OUString(CFGSTR_DISPLAYNAME);
                        pPropNameArray[nIndex] = aOneArgPath;
                        pPropNameArray[nIndex++] += rtl::OUString(CFGSTR_DESCRIPTION);
                    }

                    uno::Sequence<uno::Any> aArgProperties = rAddInConfig.GetProperties( aArgPropNames );
                    if ( aArgProperties.getLength() == aArgPropNames.getLength() )
                    {
                        const uno::Any* pPropArray = aArgProperties.getConstArray();
                        rtl::OUString sDisplayName;
                        rtl::OUString sDescription;

                        ScAddInArgDesc aDesc;
                        aDesc.eType = SC_ADDINARG_NONE;     // arg type is not in configuration
                        aDesc.bOptional = false;

                        nVisibleCount = nArgumentCount;
                        pVisibleArgs = new ScAddInArgDesc[nVisibleCount];

                        nIndex = 0;
                        for ( nArgument = 0; nArgument < nArgumentCount; nArgument++ )
                        {
                            pPropArray[nIndex++] >>= sDisplayName;
                            pPropArray[nIndex++] >>= sDescription;

                            aDesc.aInternalName = pArgNameArray[nArgument];
                            aDesc.aName         = sDisplayName;
                            aDesc.aDescription  = sDescription;

                            pVisibleArgs[nArgument] = aDesc;
                        }
                    }
                }

                rtl::OString sHelpId = aHelpIdGenerator.GetHelpId( pFuncNameArray[nFuncPos] );

                uno::Reference<reflection::XIdlMethod> xFunc;       // remains empty
                uno::Any aObject;                                   // also empty

                // create and insert into the array

                ScUnoAddInFuncData* pData = new ScUnoAddInFuncData(
                    aFuncName, aLocalName, aDescription,
                    nCategory, sHelpId,
                    xFunc, aObject,
                    nVisibleCount, pVisibleArgs, nCallerPos );

                pData->SetCompNames( aCompNames );

                ppFuncData[nFuncPos+nOld] = pData;

                pExactHashMap->insert(
                        ScAddInHashMap::value_type(
                            pData->GetOriginalName(),
                            pData ) );
                pNameHashMap->insert(
                        ScAddInHashMap::value_type(
                            pData->GetUpperName(),
                            pData ) );
                pLocalHashMap->insert(
                        ScAddInHashMap::value_type(
                            pData->GetUpperLocal(),
                            pData ) );

                delete[] pVisibleArgs;
            }
        }
    }
}

void ScUnoAddInCollection::LoadComponent( const ScUnoAddInFuncData& rFuncData )
{
    ::rtl::OUString aFullName = rFuncData.GetOriginalName();
    sal_Int32 nPos = aFullName.lastIndexOf( (sal_Unicode) '.' );
    if ( nPos > 0 )
    {
        ::rtl::OUString aServiceName = aFullName.copy( 0, nPos );

        uno::Reference<lang::XMultiServiceFactory> xServiceFactory = comphelper::getProcessServiceFactory();
        uno::Reference<uno::XInterface> xInterface( xServiceFactory->createInstance( aServiceName ) );

        if (xInterface.is())
            UpdateFromAddIn( xInterface, aServiceName );
    }
}

sal_Bool ScUnoAddInCollection::GetExcelName( const ::rtl::OUString& rCalcName,
                                        LanguageType eDestLang, ::rtl::OUString& rRetExcelName )
{
    const ScUnoAddInFuncData* pFuncData = GetFuncData( rCalcName );
    if ( pFuncData )
        return pFuncData->GetExcelName( eDestLang, rRetExcelName);
    return false;
}

sal_Bool ScUnoAddInCollection::GetCalcName( const ::rtl::OUString& rExcelName, ::rtl::OUString& rRetCalcName )
{
    if (!bInitialized)
        Initialize();

    rtl::OUString aUpperCmp = ScGlobal::pCharClass->uppercase(rExcelName);

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
                for ( long nName=0; nName<nSeqLen; nName++)
                    if ( ScGlobal::pCharClass->uppercase( pArray[nName].Name ) == aUpperCmp )
                    {
                        //! store upper case for comparing?

                        //  use the first function that has this name for any language
                        rRetCalcName = pFuncData->GetOriginalName();
                        return sal_True;
                    }
            }
        }
    }
    return false;
}

inline sal_Bool IsTypeName( const rtl::OUString& rName, const uno::Type& rType )
{
    return rName == rType.getTypeName();
}

sal_Bool lcl_ValidReturnType( const uno::Reference<reflection::XIdlClass>& xClass )
{
    //  this must match with ScUnoAddInCall::SetResult

    if ( !xClass.is() ) return false;

    switch (xClass->getTypeClass())
    {
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
            return sal_True;                        // values or string

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
                    IsTypeName( sName, getCppuType((uno::Sequence< uno::Sequence<sal_Int32> >*)0) ) ||
                    IsTypeName( sName, getCppuType((uno::Sequence< uno::Sequence<double> >*)0) ) ||
                    IsTypeName( sName, getCppuType((uno::Sequence< uno::Sequence<rtl::OUString> >*)0) ) ||
                    IsTypeName( sName, getCppuType((uno::Sequence< uno::Sequence<uno::Any> >*)0) ) );
            }
    }
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

    if (IsTypeName( sName, getCppuType((uno::Sequence< uno::Sequence<sal_Int32> >*)0) ))
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
        LanguageType eOfficeLang = Application::GetSettings().GetUILanguage();

        lang::Locale aLocale( MsLangId::convertLanguageToLocale( eOfficeLang ));
        xAddIn->setLocale( aLocale );

        ::rtl::OUString aServiceName( xName->getServiceName() );
        ScUnoAddInHelpIdGenerator aHelpIdGenerator( xName->getServiceName() );

        //! pass XIntrospection to ReadFromAddIn

        uno::Reference<lang::XMultiServiceFactory> xManager = comphelper::getProcessServiceFactory();
        if ( xManager.is() )
        {
            uno::Reference<beans::XIntrospection> xIntro(
                                    xManager->createInstance(rtl::OUString("com.sun.star.beans.Introspection")),
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

                        //! TODO: adjust bucket count?
                        if ( !pExactHashMap )
                            pExactHashMap = new ScAddInHashMap;
                        if ( !pNameHashMap )
                            pNameHashMap = new ScAddInHashMap;
                        if ( !pLocalHashMap )
                            pLocalHashMap = new ScAddInHashMap;

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
                                sal_Bool bSkip = sal_True;
                                if ( xClass.is() )
                                {
                                    //! XIdlClass needs getType() method!
                                    rtl::OUString sName = xClass->getName();
                                    bSkip = (
                                        IsTypeName( sName,
                                            getCppuType((uno::Reference<uno::XInterface>*)0) ) ||
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
                                        bSkip = sal_True;
                                }
                                if (!bSkip)
                                {
                                    rtl::OUString aFuncU = xFunc->getName();

                                    // stored function name: (service name).(function)
                                    rtl::OUStringBuffer aFuncNameBuffer( aServiceName.getLength()+1+aFuncU.getLength());
                                    aFuncNameBuffer.append(aServiceName);
                                    aFuncNameBuffer.append('.');
                                    aFuncNameBuffer.append(aFuncU);
                                    rtl::OUString aFuncName = aFuncNameBuffer.makeStringAndClear();

                                    sal_Bool bValid = sal_True;
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
                                            bValid = false;
                                        uno::Reference<reflection::XIdlClass> xParClass =
                                                    pParArr[nParamPos].aType;
                                        ScAddInArgumentType eArgType = lcl_GetArgType( xParClass );
                                        if ( eArgType == SC_ADDINARG_NONE )
                                            bValid = false;
                                        else if ( eArgType == SC_ADDINARG_CALLER )
                                            nCallerPos = nParamPos;
                                        else
                                            ++nVisibleCount;
                                    }
                                    if (bValid)
                                    {
                                        sal_uInt16 nCategory = lcl_GetCategory(
                                                xAddIn->getProgrammaticCategoryName( aFuncU ) );

                                        rtl::OString sHelpId = aHelpIdGenerator.GetHelpId( aFuncU );

                                        ::rtl::OUString aLocalName;
                                        try
                                        {
                                            aLocalName = xAddIn->
                                                getDisplayFunctionName( aFuncU );
                                        }
                                        catch(uno::Exception&)
                                        {
                                            aLocalName = "###";
                                        }

                                        ::rtl::OUString aDescription;
                                        try
                                        {
                                            aDescription = xAddIn->
                                                getFunctionDescription( aFuncU );
                                        }
                                        catch(uno::Exception&)
                                        {
                                            aDescription = "###";
                                        }

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
                                                        aArgName = "###";
                                                    }
                                                    rtl::OUString aArgDesc;
                                                    try
                                                    {
                                                        aArgDesc = xAddIn->
                                                            getArgumentDescription( aFuncU, nParamPos );
                                                    }
                                                    catch(uno::Exception&)
                                                    {
                                                        aArgName = "###";
                                                    }

                                                    sal_Bool bOptional =
                                                        ( eArgType == SC_ADDINARG_VALUE_OR_ARRAY ||
                                                          eArgType == SC_ADDINARG_VARARGS );

                                                    aDesc.eType = eArgType;
                                                    aDesc.aName = aArgName;
                                                    aDesc.aDescription = aArgDesc;
                                                    aDesc.bOptional = bOptional;
                                                    //! initialize aInternalName only from config?
                                                    aDesc.aInternalName = pParArr[nParamPos].aName;

                                                    pVisibleArgs[nDestPos++] = aDesc;
                                                }
                                            }
                                            OSL_ENSURE( nDestPos==nVisibleCount, "wrong count" );
                                        }

                                        ppFuncData[nFuncPos+nOld] = new ScUnoAddInFuncData(
                                            aFuncName, aLocalName, aDescription,
                                            nCategory, sHelpId,
                                            xFunc, aObject,
                                            nVisibleCount, pVisibleArgs, nCallerPos );

                                        const ScUnoAddInFuncData* pData =
                                            ppFuncData[nFuncPos+nOld];
                                        pExactHashMap->insert(
                                                ScAddInHashMap::value_type(
                                                    pData->GetOriginalName(),
                                                    pData ) );
                                        pNameHashMap->insert(
                                                ScAddInHashMap::value_type(
                                                    pData->GetUpperName(),
                                                    pData ) );
                                        pLocalHashMap->insert(
                                                ScAddInHashMap::value_type(
                                                    pData->GetUpperLocal(),
                                                    pData ) );

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

void lcl_UpdateFunctionList( ScFunctionList& rFunctionList, const ScUnoAddInFuncData& rFuncData )
{
    ::rtl::OUString aCompare = rFuncData.GetUpperLocal();    // as used in FillFunctionDescFromData

    sal_uLong nCount = rFunctionList.GetCount();
    for (sal_uLong nPos=0; nPos<nCount; nPos++)
    {
        const ScFuncDesc* pDesc = rFunctionList.GetFunction( nPos );
        if ( pDesc && pDesc->pFuncName && *pDesc->pFuncName == aCompare )
        {
            ScUnoAddInCollection::FillFunctionDescFromData( rFuncData, *const_cast<ScFuncDesc*>(pDesc) );
            break;
        }
    }
}

const ScAddInArgDesc* lcl_FindArgDesc( const ScUnoAddInFuncData& rFuncData, const ::rtl::OUString& rArgIntName )
{
    long nArgCount = rFuncData.GetArgumentCount();
    const ScAddInArgDesc* pArguments = rFuncData.GetArguments();
    for (long nPos=0; nPos<nArgCount; nPos++)
    {
        if ( pArguments[nPos].aInternalName == rArgIntName )
            return &pArguments[nPos];
    }
    return NULL;
}

void ScUnoAddInCollection::UpdateFromAddIn( const uno::Reference<uno::XInterface>& xInterface,
                                            const ::rtl::OUString& rServiceName )
{
    uno::Reference<lang::XLocalizable> xLoc( xInterface, uno::UNO_QUERY );
    if ( xLoc.is() )        // optional in new add-ins
    {
        LanguageType eOfficeLang = Application::GetSettings().GetUILanguage();
        lang::Locale aLocale( MsLangId::convertLanguageToLocale( eOfficeLang ));
        xLoc->setLocale( aLocale );
    }

    // if function list was already initialized, it must be updated

    ScFunctionList* pFunctionList = NULL;
    if ( ScGlobal::HasStarCalcFunctionList() )
        pFunctionList = ScGlobal::GetStarCalcFunctionList();

    // only get the function information from Introspection

    uno::Reference<lang::XMultiServiceFactory> xManager = comphelper::getProcessServiceFactory();
    if ( xManager.is() )
    {
        uno::Reference<beans::XIntrospection> xIntro(
                                xManager->createInstance(rtl::OUString("com.sun.star.beans.Introspection")),
                                uno::UNO_QUERY );
        if ( xIntro.is() )
        {
            uno::Any aObject;
            aObject <<= xInterface;
            uno::Reference<beans::XIntrospectionAccess> xAcc = xIntro->inspect(aObject);
            if (xAcc.is())
            {
                uno::Sequence< uno::Reference<reflection::XIdlMethod> > aMethods =
                        xAcc->getMethods( beans::MethodConcept::ALL );
                long nMethodCount = aMethods.getLength();
                const uno::Reference<reflection::XIdlMethod>* pArray = aMethods.getConstArray();
                for (long nFuncPos=0; nFuncPos<nMethodCount; nFuncPos++)
                {
                    uno::Reference<reflection::XIdlMethod> xFunc = pArray[nFuncPos];
                    if (xFunc.is())
                    {
                        rtl::OUString aFuncU = xFunc->getName();

                        // stored function name: (service name).(function)
                        rtl::OUStringBuffer aFuncNameBuffer( rServiceName.getLength()+1+aFuncU.getLength());
                        aFuncNameBuffer.append(rServiceName);
                        aFuncNameBuffer.append('.');
                        aFuncNameBuffer.append(aFuncU);
                        rtl::OUString aFuncName = aFuncNameBuffer.makeStringAndClear();

                        // internal names are skipped because no FuncData exists
                        ScUnoAddInFuncData* pOldData = const_cast<ScUnoAddInFuncData*>( GetFuncData( aFuncName ) );
                        if ( pOldData )
                        {
                            // Create new (complete) argument info.
                            // As in ReadFromAddIn, the reflection information is authoritative.
                            // Local names and descriptions from pOldData are looked up using the
                            // internal argument name.

                            sal_Bool bValid = sal_True;
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
                                    bValid = false;
                                uno::Reference<reflection::XIdlClass> xParClass =
                                            pParArr[nParamPos].aType;
                                ScAddInArgumentType eArgType = lcl_GetArgType( xParClass );
                                if ( eArgType == SC_ADDINARG_NONE )
                                    bValid = false;
                                else if ( eArgType == SC_ADDINARG_CALLER )
                                    nCallerPos = nParamPos;
                                else
                                    ++nVisibleCount;
                            }
                            if (bValid)
                            {
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
                                            const ScAddInArgDesc* pOldArgDesc =
                                                lcl_FindArgDesc( *pOldData, pParArr[nParamPos].aName );
                                            if ( pOldArgDesc )
                                            {
                                                aDesc.aName = pOldArgDesc->aName;
                                                aDesc.aDescription = pOldArgDesc->aDescription;
                                            }
                                            else
                                                aDesc.aName = aDesc.aDescription = "###";

                                            sal_Bool bOptional =
                                                ( eArgType == SC_ADDINARG_VALUE_OR_ARRAY ||
                                                  eArgType == SC_ADDINARG_VARARGS );

                                            aDesc.eType = eArgType;
                                            aDesc.bOptional = bOptional;
                                            //! initialize aInternalName only from config?
                                            aDesc.aInternalName = pParArr[nParamPos].aName;

                                            pVisibleArgs[nDestPos++] = aDesc;
                                        }
                                    }
                                    OSL_ENSURE( nDestPos==nVisibleCount, "wrong count" );
                                }

                                pOldData->SetFunction( xFunc, aObject );
                                pOldData->SetArguments( nVisibleCount, pVisibleArgs );
                                pOldData->SetCallerPos( nCallerPos );

                                if ( pFunctionList )
                                    lcl_UpdateFunctionList( *pFunctionList, *pOldData );

                                delete[] pVisibleArgs;
                            }
                        }
                    }
                }
            }
        }
    }
}

::rtl::OUString ScUnoAddInCollection::FindFunction( const ::rtl::OUString& rUpperName, sal_Bool bLocalFirst )
{
    if (!bInitialized)
        Initialize();

    if (nFuncCount == 0)
        return EMPTY_OUSTRING;

    if ( bLocalFirst )
    {
        //  first scan all local names (used for entering formulas)

        ScAddInHashMap::const_iterator iLook( pLocalHashMap->find( rUpperName ) );
        if ( iLook != pLocalHashMap->end() )
            return iLook->second->GetOriginalName();
    }
    else
    {
        //  first scan international names (used when calling a function)
        //! before that, check for exact match???

        ScAddInHashMap::const_iterator iLook( pNameHashMap->find( rUpperName ) );
        if ( iLook != pNameHashMap->end() )
            return iLook->second->GetOriginalName();

        //  after that, scan all local names (to allow replacing old AddIns with Uno)

        iLook = pLocalHashMap->find( rUpperName );
        if ( iLook != pLocalHashMap->end() )
            return iLook->second->GetOriginalName();
    }

    return EMPTY_OUSTRING;
}

const ScUnoAddInFuncData* ScUnoAddInCollection::GetFuncData( const ::rtl::OUString& rName, bool bComplete )
{
    if (!bInitialized)
        Initialize();

    //  rName must be the exact internal name

    ScAddInHashMap::const_iterator iLook( pExactHashMap->find( rName ) );
    if ( iLook != pExactHashMap->end() )
    {
        const ScUnoAddInFuncData* pFuncData = iLook->second;

        if ( bComplete && !pFuncData->GetFunction().is() )           //! extra flag?
            LoadComponent( *pFuncData );

        return pFuncData;
    }

    return NULL;
}

const ScUnoAddInFuncData* ScUnoAddInCollection::GetFuncData( long nIndex )
{
    if (!bInitialized)
        Initialize();

    if (nIndex < nFuncCount)
        return ppFuncData[nIndex];
    return NULL;
}

void ScUnoAddInCollection::LocalizeString( ::rtl::OUString& rName )
{
    if (!bInitialized)
        Initialize();

    //  modify rName - input: exact name

    ScAddInHashMap::const_iterator iLook( pExactHashMap->find( rName ) );
    if ( iLook != pExactHashMap->end() )
        rName = iLook->second->GetUpperLocal();         //! upper?
}


long ScUnoAddInCollection::GetFuncCount()
{
    if (!bInitialized)
        Initialize();

    return nFuncCount;
}

sal_Bool ScUnoAddInCollection::FillFunctionDesc( long nFunc, ScFuncDesc& rDesc )
{
    if (!bInitialized)
        Initialize();

    if (nFunc >= nFuncCount || !ppFuncData[nFunc])
        return false;

    const ScUnoAddInFuncData& rFuncData = *ppFuncData[nFunc];

    return FillFunctionDescFromData( rFuncData, rDesc );
}

sal_Bool ScUnoAddInCollection::FillFunctionDescFromData( const ScUnoAddInFuncData& rFuncData, ScFuncDesc& rDesc )
{
    rDesc.Clear();

    sal_Bool bIncomplete = !rFuncData.GetFunction().is();       //! extra flag?

    long nArgCount = rFuncData.GetArgumentCount();
    if ( nArgCount > USHRT_MAX )
        return false;

    if ( bIncomplete )
        nArgCount = 0;      // if incomplete, fill without argument info (no wrong order)

    // nFIndex is set from outside

    rDesc.pFuncName = new ::rtl::OUString( rFuncData.GetUpperLocal() );     //! upper?
    rDesc.nCategory = rFuncData.GetCategory();
    rDesc.sHelpId = rFuncData.GetHelpId();

    ::rtl::OUString aDesc = rFuncData.GetDescription();
    if (aDesc.isEmpty())
        aDesc = rFuncData.GetLocalName();      // use name if no description is available
    rDesc.pFuncDesc = new ::rtl::OUString( aDesc );

    // AddInArgumentType_CALLER is already left out in FuncData

    rDesc.nArgCount = (sal_uInt16)nArgCount;
    if ( nArgCount )
    {
        sal_Bool bMultiple = false;
        const ScAddInArgDesc* pArgs = rFuncData.GetArguments();

        rDesc.ppDefArgNames = new ::rtl::OUString*[nArgCount];
        rDesc.ppDefArgDescs = new ::rtl::OUString*[nArgCount];
        rDesc.pDefArgFlags   = new ScFuncDesc::ParameterFlags[nArgCount];
        for ( long nArg=0; nArg<nArgCount; nArg++ )
        {
            rDesc.ppDefArgNames[nArg] = new ::rtl::OUString( pArgs[nArg].aName );
            rDesc.ppDefArgDescs[nArg] = new ::rtl::OUString( pArgs[nArg].aDescription );
            rDesc.pDefArgFlags[nArg].bOptional = pArgs[nArg].bOptional;
            rDesc.pDefArgFlags[nArg].bSuppress = false;

            // no empty names...
            if ( rDesc.ppDefArgNames[nArg]->isEmpty() )
            {
                ::rtl::OUString aDefName("arg");
                aDefName += ::rtl::OUString::valueOf( nArg+1 );
                *rDesc.ppDefArgNames[nArg] = aDefName;
            }

            //  last argument repeated?
            if ( nArg+1 == nArgCount && ( pArgs[nArg].eType == SC_ADDINARG_VARARGS ) )
                bMultiple = sal_True;
        }

        if ( bMultiple )
            rDesc.nArgCount += VAR_ARGS - 1;    // VAR_ARGS means just one repeated arg
    }

    rDesc.bIncomplete = bIncomplete;

    return sal_True;
}


//------------------------------------------------------------------------

ScUnoAddInCall::ScUnoAddInCall( ScUnoAddInCollection& rColl, const ::rtl::OUString& rName,
                                long nParamCount ) :
    bValidCount( false ),
    nErrCode( errNoCode ),      // before function was called
    bHasString( sal_True ),
    fValue( 0.0 ),
    xMatrix( NULL )
{
    pFuncData = rColl.GetFuncData( rName, true );           // need fully initialized data
    OSL_ENSURE( pFuncData, "Function Data missing" );
    if ( pFuncData )
    {
        long nDescCount = pFuncData->GetArgumentCount();
        const ScAddInArgDesc* pArgs = pFuncData->GetArguments();

        //  is aVarArg sequence needed?
        if ( nParamCount >= nDescCount && nDescCount > 0 &&
             pArgs[nDescCount-1].eType == SC_ADDINARG_VARARGS )
        {
            long nVarCount = nParamCount - ( nDescCount - 1 );  // size of last argument
            aVarArg.realloc( nVarCount );
            bValidCount = sal_True;
        }
        else if ( nParamCount <= nDescCount )
        {
            //  all args behind nParamCount must be optional
            bValidCount = sal_True;
            for (long i=nParamCount; i<nDescCount; i++)
                if ( !pArgs[i].bOptional )
                    bValidCount = false;
        }
        // else invalid (too many arguments)

        if ( bValidCount )
            aArgs.realloc( nDescCount );    // sequence must always match function signature
    }
}

ScUnoAddInCall::~ScUnoAddInCall()
{
    // pFuncData is deleted with ScUnoAddInCollection
}

sal_Bool ScUnoAddInCall::ValidParamCount()
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

sal_Bool ScUnoAddInCall::NeedsCaller() const
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
            {
                OSL_FAIL("wrong argument number");
            }
        }
        else if ( nPos < aArgs.getLength() )
            aArgs.getArray()[nPos] = rValue;
        else
        {
            OSL_FAIL("wrong argument number");
        }
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

        OSL_ENSURE( aArgs.getLength() == nCount, "wrong argument count" );
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
            OSL_FAIL("wrong CallPos");
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

        catch(const reflection::InvocationTargetException& rWrapped)
        {
            if ( rWrapped.TargetException.getValueType().equals(
                    getCppuType( (lang::IllegalArgumentException*)0 ) ) )
                nErrCode = errIllegalArgument;
            else if ( rWrapped.TargetException.getValueType().equals(
                    getCppuType( (sheet::NoConvergenceException*)0 ) ) )
                nErrCode = errNoConvergence;
            else
                nErrCode = errNoValue;
        }

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
            nErrCode = NOTAVAILABLE;         // #NA
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
            {
                uno::TypeClass eMyClass;
                ScApiTypeConversion::ConvertAnyToDouble( fValue, eMyClass, rNewRes);
                bHasString = false;
            }
            break;

        case uno::TypeClass_STRING:
            {
                rNewRes >>= aString;
                bHasString = sal_True;
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
            if ( aType.equals( getCppuType( (uno::Sequence< uno::Sequence<sal_Int32> > *)0 ) ) )
            {
                const uno::Sequence< uno::Sequence<sal_Int32> >* pRowSeq = NULL;

                //! use pointer from any!
                uno::Sequence< uno::Sequence<sal_Int32> > aSequence;
                if ( rNewRes >>= aSequence )
                    pRowSeq = &aSequence;

                if ( pRowSeq )
                {
                    long nRowCount = pRowSeq->getLength();
                    const uno::Sequence<sal_Int32>* pRowArr = pRowSeq->getConstArray();
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
                        xMatrix = new ScMatrix(
                                static_cast<SCSIZE>(nMaxColCount),
                                static_cast<SCSIZE>(nRowCount), 0.0);
                        for (nRow=0; nRow<nRowCount; nRow++)
                        {
                            long nColCount = pRowArr[nRow].getLength();
                            const sal_Int32* pColArr = pRowArr[nRow].getConstArray();
                            for (nCol=0; nCol<nColCount; nCol++)
                                xMatrix->PutDouble( pColArr[nCol],
                                        static_cast<SCSIZE>(nCol),
                                        static_cast<SCSIZE>(nRow) );
                            for (nCol=nColCount; nCol<nMaxColCount; nCol++)
                                xMatrix->PutDouble( 0.0,
                                        static_cast<SCSIZE>(nCol),
                                        static_cast<SCSIZE>(nRow) );
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
                        xMatrix = new ScMatrix(
                                static_cast<SCSIZE>(nMaxColCount),
                                static_cast<SCSIZE>(nRowCount), 0.0);
                        for (nRow=0; nRow<nRowCount; nRow++)
                        {
                            long nColCount = pRowArr[nRow].getLength();
                            const double* pColArr = pRowArr[nRow].getConstArray();
                            for (nCol=0; nCol<nColCount; nCol++)
                                xMatrix->PutDouble( pColArr[nCol],
                                        static_cast<SCSIZE>(nCol),
                                        static_cast<SCSIZE>(nRow) );
                            for (nCol=nColCount; nCol<nMaxColCount; nCol++)
                                xMatrix->PutDouble( 0.0,
                                        static_cast<SCSIZE>(nCol),
                                        static_cast<SCSIZE>(nRow) );
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
                        xMatrix = new ScMatrix(
                                static_cast<SCSIZE>(nMaxColCount),
                                static_cast<SCSIZE>(nRowCount), 0.0);
                        for (nRow=0; nRow<nRowCount; nRow++)
                        {
                            long nColCount = pRowArr[nRow].getLength();
                            const rtl::OUString* pColArr = pRowArr[nRow].getConstArray();
                            for (nCol=0; nCol<nColCount; nCol++)
                                xMatrix->PutString( pColArr[nCol],
                                    static_cast<SCSIZE>(nCol),
                                    static_cast<SCSIZE>(nRow) );
                            for (nCol=nColCount; nCol<nMaxColCount; nCol++)
                                xMatrix->PutString( EMPTY_OUSTRING,
                                        static_cast<SCSIZE>(nCol),
                                        static_cast<SCSIZE>(nRow) );
                        }
                    }
                }
            }
            else if ( aType.equals( getCppuType( (uno::Sequence< uno::Sequence<uno::Any> > *)0 ) ) )
            {
                xMatrix = ScSequenceToMatrix::CreateMixedMatrix( rNewRes );
            }

            if (!xMatrix)                       // no array found
                nErrCode = errNoValue;          //! code for error in return type???
    }
}



//------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
