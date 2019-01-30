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

#include <comphelper/processfactory.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <sfx2/objsh.hxx>
#include <unotools/charclass.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/reflection/XIdlClass.hpp>
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#include <com/sun/star/beans/theIntrospection.hpp>
#include <com/sun/star/beans/MethodConcept.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/sheet/XCompatibilityNames.hpp>
#include <com/sun/star/sheet/NoConvergenceException.hpp>
#include <com/sun/star/sheet/XAddIn.hpp>
#include <com/sun/star/sheet/XVolatileResult.hpp>

#include <addincol.hxx>
#include <addinhelpid.hxx>
#include <compiler.hxx>
#include <scmatrix.hxx>
#include <addinlis.hxx>
#include <formula/errorcodes.hxx>
#include <formula/funcvarargs.h>
#include <sc.hrc>
#include <optutil.hxx>
#include <addincfg.hxx>
#include <scmod.hxx>
#include <rangeseq.hxx>
#include <funcdesc.hxx>
#include <svl/sharedstring.hxx>
#include <formulaopt.hxx>
#include <memory>

using namespace com::sun::star;

#define SC_CALLERPOS_NONE   (-1)

ScUnoAddInFuncData::ScUnoAddInFuncData( const OUString& rNam, const OUString& rLoc,
                                        const OUString& rDesc,
                                        sal_uInt16 nCat, const OString& sHelp,
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
        pArgDescs.reset( new ScAddInArgDesc[nArgCount] );
        for (long i=0; i<nArgCount; i++)
            pArgDescs[i] = pAD[i];
    }

    aUpperName = ScGlobal::pCharClass->uppercase(aUpperName);
    aUpperLocal = ScGlobal::pCharClass->uppercase(aUpperLocal);
}

ScUnoAddInFuncData::~ScUnoAddInFuncData()
{
}

const ::std::vector<ScUnoAddInFuncData::LocalizedName>& ScUnoAddInFuncData::GetCompNames() const
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
                OUString aMethodName = xFunction->getName();
                uno::Sequence< sheet::LocalizedName> aCompNames( xComp->getCompatibilityNames( aMethodName ));
                maCompNames.clear();
                sal_Int32 nSeqLen = aCompNames.getLength();
                if ( nSeqLen )
                {
                    const sheet::LocalizedName* pArray = aCompNames.getArray();
                    for (sal_Int32 i=0; i<nSeqLen; i++)
                    {
                        maCompNames.emplace_back(
                                    LanguageTag::convertToBcp47( pArray[i].Locale, false),
                                    pArray[i].Name);
                    }
                }
            }
        }

        bCompInitialized = true;        // also if not successful
    }
    return maCompNames;
}

void ScUnoAddInFuncData::SetCompNames( const ::std::vector< ScUnoAddInFuncData::LocalizedName >& rNew )
{
    OSL_ENSURE( !bCompInitialized, "SetCompNames after initializing" );

    maCompNames = rNew;

    bCompInitialized = true;
}

bool ScUnoAddInFuncData::GetExcelName( LanguageType eDestLang, OUString& rRetExcelName ) const
{
    const ::std::vector<LocalizedName>& rCompNames = GetCompNames();
    if ( !rCompNames.empty() )
    {
        LanguageTag aLanguageTag( eDestLang);
        const OUString& aSearch( aLanguageTag.getBcp47());

        // First, check exact match without fallback overhead.
        ::std::vector<LocalizedName>::const_iterator itNames = std::find_if(rCompNames.begin(), rCompNames.end(),
            [&aSearch](const LocalizedName& rName) { return rName.maLocale == aSearch; });
        if (itNames != rCompNames.end())
        {
            rRetExcelName = (*itNames).maName;
            return true;
        }

        // Second, try match of fallback search with fallback locales,
        // appending also 'en-US' and 'en' to search if not queried.
        ::std::vector< OUString > aFallbackSearch( aLanguageTag.getFallbackStrings( true));
        if (aSearch != "en-US")
        {
            aFallbackSearch.emplace_back("en-US");
            if (aSearch != "en")
            {
                aFallbackSearch.emplace_back("en");
            }
        }
        for (const auto& rSearch : aFallbackSearch)
        {
            for (const auto& rCompName : rCompNames)
            {
                // We checked already the full tag, start with second.
                ::std::vector< OUString > aFallbackLocales( LanguageTag( rCompName.maLocale).getFallbackStrings( false));
                if (std::find(aFallbackLocales.begin(), aFallbackLocales.end(), rSearch) != aFallbackLocales.end())
                {
                    rRetExcelName = rCompName.maName;
                    return true;
                }
            }
        }

        // Third, last resort, use first (default) entry.
        rRetExcelName = rCompNames[0].maName;
        return true;
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
    nArgCount = nNewCount;
    if ( nArgCount )
    {
        pArgDescs.reset( new ScAddInArgDesc[nArgCount] );
        for (long i=0; i<nArgCount; i++)
            pArgDescs[i] = pNewDescs[i];
    }
    else
        pArgDescs.reset();
}

void ScUnoAddInFuncData::SetCallerPos( long nNewPos )
{
    nCallerPos = nNewPos;
}

ScUnoAddInCollection::ScUnoAddInCollection() :
    nFuncCount( 0 ),
    bInitialized( false )
{
}

ScUnoAddInCollection::~ScUnoAddInCollection()
{
}

void ScUnoAddInCollection::Clear()
{
    pExactHashMap.reset();
    pNameHashMap.reset();
    pLocalHashMap.reset();
    ppFuncData.reset();
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
                        xEnAc->createContentEnumeration( "com.sun.star.sheet.AddIn" );
        if ( xEnum.is() )
        {
            //  loop through all AddIns
            while ( xEnum->hasMoreElements() )
            {
                uno::Any aAddInAny = xEnum->nextElement();

                try
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
                } catch ( const uno::Exception& ) {
                    SAL_WARN ( "sc", "Failed to initialize create instance of sheet.AddIn" );
                }
            }
        }
    }

    // ReadConfiguration is called after looking at the AddIn implementations.
    // Duplicated are skipped (by using the service information, they don't have to be updated again
    // when argument information is needed).
    ReadConfiguration();

    bInitialized = true;        // with or without functions
}

static sal_uInt16 lcl_GetCategory( const OUString& rName )
{
    static const sal_Char* aFuncNames[SC_FUNCGROUP_COUNT] =
    {
        //  array index = ID - 1 (ID starts at 1)
        //  all upper case
        "Database",         // ID_FUNCTION_GRP_DATABASE
        "Date&Time",        // ID_FUNCTION_GRP_DATETIME
        "Financial",        // ID_FUNCTION_GRP_FINANCIAL
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
    ScLinkConfigItem aAllLocalesConfig( CFGPATH_ADDINS, ConfigItemMode::AllLocales );
    // CommitLink is not used (only reading values)

    const OUString sSlash('/');

    // get the list of add-ins (services)
    uno::Sequence<OUString> aServiceNames = rAddInConfig.GetNodeNames( "" );

    sal_Int32 nServiceCount = aServiceNames.getLength();
    for ( sal_Int32 nService = 0; nService < nServiceCount; nService++ )
    {
        OUString aServiceName = aServiceNames[nService];
        ScUnoAddInHelpIdGenerator aHelpIdGenerator( aServiceName );

        OUString aFunctionsPath(aServiceName + sSlash + CFGSTR_ADDINFUNCTIONS);

        uno::Sequence<OUString> aFunctionNames = rAddInConfig.GetNodeNames( aFunctionsPath );
        sal_Int32 nNewCount = aFunctionNames.getLength();

        // allocate pointers

        long nOld = nFuncCount;
        nFuncCount = nNewCount+nOld;
        if ( nOld )
        {
            std::unique_ptr<std::unique_ptr<ScUnoAddInFuncData>[]> ppNew(new std::unique_ptr<ScUnoAddInFuncData>[nFuncCount]);
            for (long i=0; i<nOld; i++)
                ppNew[i] = std::move(ppFuncData[i]);
            ppFuncData = std::move(ppNew);
        }
        else
            ppFuncData.reset( new std::unique_ptr<ScUnoAddInFuncData>[nFuncCount] );

        //TODO: adjust bucket count?
        if ( !pExactHashMap )
            pExactHashMap.reset( new ScAddInHashMap );
        if ( !pNameHashMap )
            pNameHashMap.reset( new ScAddInHashMap );
        if ( !pLocalHashMap )
            pLocalHashMap.reset( new ScAddInHashMap );

        //TODO: get the function information in a single call for all functions?

        const OUString* pFuncNameArray = aFunctionNames.getConstArray();
        for ( sal_Int32 nFuncPos = 0; nFuncPos < nNewCount; nFuncPos++ )
        {
            ppFuncData[nFuncPos+nOld] = nullptr;

            // stored function name: (service name).(function)
            OUStringBuffer aFuncNameBuffer( aServiceName.getLength()+1+pFuncNameArray[nFuncPos].getLength());
            aFuncNameBuffer.append(aServiceName);
            aFuncNameBuffer.append('.');
            aFuncNameBuffer.append(pFuncNameArray[nFuncPos]);
            OUString aFuncName = aFuncNameBuffer.makeStringAndClear();

            // skip the function if already known (read from old AddIn service)

            if ( pExactHashMap->find( aFuncName ) == pExactHashMap->end() )
            {
                OUString aLocalName;
                OUString aDescription;
                sal_uInt16 nCategory = ID_FUNCTION_GRP_ADDINS;

                // get direct information on the function

                OUString aFuncPropPath = aFunctionsPath;
                aFuncPropPath += sSlash;
                aFuncPropPath += pFuncNameArray[nFuncPos];
                aFuncPropPath += sSlash;

                uno::Sequence<OUString> aFuncPropNames{
                    (aFuncPropPath + CFGSTR_DISPLAYNAME), // CFG_FUNCPROP_DISPLAYNAME
                    (aFuncPropPath + CFGSTR_DESCRIPTION), // CFG_FUNCPROP_DESCRIPTION
                    (aFuncPropPath + CFGSTR_CATEGORY)};   // CFG_FUNCPROP_CATEGORY

                uno::Sequence<uno::Any> aFuncProperties = rAddInConfig.GetProperties( aFuncPropNames );
                if ( aFuncProperties.getLength() == CFG_FUNCPROP_COUNT )
                {
                    aFuncProperties[CFG_FUNCPROP_DISPLAYNAME] >>= aLocalName;
                    aFuncProperties[CFG_FUNCPROP_DESCRIPTION] >>= aDescription;

                    OUString aCategoryName;
                    aFuncProperties[CFG_FUNCPROP_CATEGORY] >>= aCategoryName;
                    nCategory = lcl_GetCategory( aCategoryName );
                }

                // get compatibility names

                ::std::vector<ScUnoAddInFuncData::LocalizedName> aCompNames;

                OUString aCompPath(aFuncPropPath + CFGSTR_COMPATIBILITYNAME);
                uno::Sequence<OUString> aCompPropNames( &aCompPath, 1 );

                uno::Sequence<uno::Any> aCompProperties = aAllLocalesConfig.GetProperties( aCompPropNames );
                if ( aCompProperties.getLength() == 1 )
                {
                    uno::Sequence<beans::PropertyValue> aLocalEntries;
                    if ( aCompProperties[0] >>= aLocalEntries )
                    {
                        sal_Int32 nLocaleCount = aLocalEntries.getLength();
                        const beans::PropertyValue* pConfigArray = aLocalEntries.getConstArray();

                        for ( sal_Int32 nLocale = 0; nLocale < nLocaleCount; nLocale++ )
                        {
                            // PropertyValue name is the locale ("convert" from
                            // string to canonicalize)
                            OUString aLocale( LanguageTag( pConfigArray[nLocale].Name, true).getBcp47( false));
                            // PropertyValue value is the localized value (string in this case)
                            OUString aName;
                            pConfigArray[nLocale].Value >>= aName;
                            aCompNames.emplace_back( aLocale, aName);
                        }
                    }
                }

                // get argument info

                std::unique_ptr<ScAddInArgDesc[]> pVisibleArgs;
                long nVisibleCount = 0;

                OUString aArgumentsPath(aFuncPropPath + CFGSTR_PARAMETERS);

                uno::Sequence<OUString> aArgumentNames = rAddInConfig.GetNodeNames( aArgumentsPath );
                sal_Int32 nArgumentCount = aArgumentNames.getLength();
                if ( nArgumentCount )
                {
                    // get DisplayName and Description for each argument
                    uno::Sequence<OUString> aArgPropNames( nArgumentCount * 2 );
                    OUString* pPropNameArray = aArgPropNames.getArray();

                    sal_Int32 nArgument;
                    sal_Int32 nIndex = 0;
                    const OUString* pArgNameArray = aArgumentNames.getConstArray();
                    for ( nArgument = 0; nArgument < nArgumentCount; nArgument++ )
                    {
                        OUString aOneArgPath = aArgumentsPath;
                        aOneArgPath += sSlash;
                        aOneArgPath += pArgNameArray[nArgument];
                        aOneArgPath += sSlash;

                        pPropNameArray[nIndex++] = aOneArgPath
                            + CFGSTR_DISPLAYNAME;
                        pPropNameArray[nIndex++] = aOneArgPath
                            + CFGSTR_DESCRIPTION;
                    }

                    uno::Sequence<uno::Any> aArgProperties = rAddInConfig.GetProperties( aArgPropNames );
                    if ( aArgProperties.getLength() == aArgPropNames.getLength() )
                    {
                        const uno::Any* pPropArray = aArgProperties.getConstArray();
                        OUString sDisplayName;
                        OUString sDescription;

                        ScAddInArgDesc aDesc;
                        aDesc.eType = SC_ADDINARG_NONE;     // arg type is not in configuration
                        aDesc.bOptional = false;

                        nVisibleCount = nArgumentCount;
                        pVisibleArgs.reset(new ScAddInArgDesc[nVisibleCount]);

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

                OString sHelpId = aHelpIdGenerator.GetHelpId( pFuncNameArray[nFuncPos] );

                uno::Reference<reflection::XIdlMethod> xFunc;       // remains empty
                uno::Any aObject;                                   // also empty

                // create and insert into the array

                ScUnoAddInFuncData* pData = new ScUnoAddInFuncData(
                    aFuncName, aLocalName, aDescription,
                    nCategory, sHelpId,
                    xFunc, aObject,
                    nVisibleCount, pVisibleArgs.get(), SC_CALLERPOS_NONE );

                pData->SetCompNames( aCompNames );

                ppFuncData[nFuncPos+nOld].reset(pData);

                pExactHashMap->emplace(
                            pData->GetOriginalName(),
                            pData );
                pNameHashMap->emplace(
                            pData->GetUpperName(),
                            pData );
                pLocalHashMap->emplace(
                            pData->GetUpperLocal(),
                            pData );
            }
        }
    }
}

void ScUnoAddInCollection::LoadComponent( const ScUnoAddInFuncData& rFuncData )
{
    const OUString& aFullName = rFuncData.GetOriginalName();
    sal_Int32 nPos = aFullName.lastIndexOf( '.' );
    if ( nPos > 0 )
    {
        OUString aServiceName = aFullName.copy( 0, nPos );

        try
        {
            uno::Reference<lang::XMultiServiceFactory> xServiceFactory = comphelper::getProcessServiceFactory();
            uno::Reference<uno::XInterface> xInterface( xServiceFactory->createInstance( aServiceName ) );

            if (xInterface.is())
                UpdateFromAddIn( xInterface, aServiceName );
        }
        catch (const uno::Exception &)
        {
            SAL_WARN ("sc", "Failed to create addin component '"
                      << aServiceName << "'");
        }
    }
}

bool ScUnoAddInCollection::GetExcelName( const OUString& rCalcName,
                                        LanguageType eDestLang, OUString& rRetExcelName )
{
    const ScUnoAddInFuncData* pFuncData = GetFuncData( rCalcName );
    if ( pFuncData )
        return pFuncData->GetExcelName( eDestLang, rRetExcelName);
    return false;
}

bool ScUnoAddInCollection::GetCalcName( const OUString& rExcelName, OUString& rRetCalcName )
{
    if (!bInitialized)
        Initialize();

    OUString aUpperCmp = ScGlobal::pCharClass->uppercase(rExcelName);

    for (long i=0; i<nFuncCount; i++)
    {
        ScUnoAddInFuncData* pFuncData = ppFuncData[i].get();
        if ( pFuncData )
        {
            const ::std::vector<ScUnoAddInFuncData::LocalizedName>& rNames = pFuncData->GetCompNames();
            auto bFound = std::any_of(rNames.begin(), rNames.end(),
                [&aUpperCmp](const ScUnoAddInFuncData::LocalizedName& rName) {
                    return ScGlobal::pCharClass->uppercase( rName.maName ) == aUpperCmp; });
            if (bFound)
            {
                //TODO: store upper case for comparing?

                //  use the first function that has this name for any language
                rRetCalcName = pFuncData->GetOriginalName();
                return true;
            }
        }
    }
    return false;
}

static bool IsTypeName( const OUString& rName, const uno::Type& rType )
{
    return rName == rType.getTypeName();
}

static bool lcl_ValidReturnType( const uno::Reference<reflection::XIdlClass>& xClass )
{
    //  this must match with ScUnoAddInCall::SetResult

    if ( !xClass.is() ) return false;

    switch (xClass->getTypeClass())
    {
        case uno::TypeClass_ANY:                // variable type
        case uno::TypeClass_ENUM:               //TODO: ???
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
            return true;                        // values or string

        case uno::TypeClass_INTERFACE:
            {
                //  return type XInterface may contain a XVolatileResult
                //TODO: XIdlClass needs getType() method!

                OUString sName = xClass->getName();
                return (
                    IsTypeName( sName, cppu::UnoType<sheet::XVolatileResult>::get()) ||
                    IsTypeName( sName, cppu::UnoType<uno::XInterface>::get()) );
            }

        default:
            {
                //  nested sequences for arrays
                //TODO: XIdlClass needs getType() method!

                OUString sName = xClass->getName();
                return (
                    IsTypeName( sName, cppu::UnoType<uno::Sequence< uno::Sequence<sal_Int32> >>::get() ) ||
                    IsTypeName( sName, cppu::UnoType<uno::Sequence< uno::Sequence<double> >>::get() ) ||
                    IsTypeName( sName, cppu::UnoType<uno::Sequence< uno::Sequence<OUString> >>::get() ) ||
                    IsTypeName( sName, cppu::UnoType<uno::Sequence< uno::Sequence<uno::Any> >>::get() ) );
            }
    }
}

static ScAddInArgumentType lcl_GetArgType( const uno::Reference<reflection::XIdlClass>& xClass )
{
    if (!xClass.is())
        return SC_ADDINARG_NONE;

    uno::TypeClass eType = xClass->getTypeClass();

    if ( eType == uno::TypeClass_LONG )             //TODO: other integer types?
        return SC_ADDINARG_INTEGER;

    if ( eType == uno::TypeClass_DOUBLE )
        return SC_ADDINARG_DOUBLE;

    if ( eType == uno::TypeClass_STRING )
        return SC_ADDINARG_STRING;

    //TODO: XIdlClass needs getType() method!
    OUString sName = xClass->getName();

    if (IsTypeName( sName, cppu::UnoType<uno::Sequence< uno::Sequence<sal_Int32> >>::get() ))
        return SC_ADDINARG_INTEGER_ARRAY;

    if (IsTypeName( sName, cppu::UnoType<uno::Sequence< uno::Sequence<double> >>::get() ))
        return SC_ADDINARG_DOUBLE_ARRAY;

    if (IsTypeName( sName, cppu::UnoType<uno::Sequence< uno::Sequence<OUString> >>::get() ))
        return SC_ADDINARG_STRING_ARRAY;

    if (IsTypeName( sName, cppu::UnoType<uno::Sequence< uno::Sequence<uno::Any> >>::get() ))
        return SC_ADDINARG_MIXED_ARRAY;

    if (IsTypeName( sName, cppu::UnoType<uno::Any>::get()))
        return SC_ADDINARG_VALUE_OR_ARRAY;

    if (IsTypeName( sName, cppu::UnoType<table::XCellRange>::get()))
        return SC_ADDINARG_CELLRANGE;

    if (IsTypeName( sName, cppu::UnoType<beans::XPropertySet>::get()))
        return SC_ADDINARG_CALLER;

    if (IsTypeName( sName, cppu::UnoType<uno::Sequence<uno::Any>>::get() ))
        return SC_ADDINARG_VARARGS;

    return SC_ADDINARG_NONE;
}

void ScUnoAddInCollection::ReadFromAddIn( const uno::Reference<uno::XInterface>& xInterface )
{
    uno::Reference<sheet::XAddIn> xAddIn( xInterface, uno::UNO_QUERY );
    uno::Reference<lang::XServiceName> xName( xInterface, uno::UNO_QUERY );
    if ( xAddIn.is() && xName.is() )
    {
        // fdo50118 when GetUseEnglishFunctionName() returns true, set the
        // locale to en-US to get English function names
        if ( SC_MOD()->GetFormulaOptions().GetUseEnglishFuncName() )
            xAddIn->setLocale( lang::Locale( "en", "US", ""));
        else
            xAddIn->setLocale( Application::GetSettings().GetUILanguageTag().getLocale());

        OUString aServiceName( xName->getServiceName() );
        ScUnoAddInHelpIdGenerator aHelpIdGenerator( aServiceName );

        //TODO: pass XIntrospection to ReadFromAddIn

        uno::Reference<uno::XComponentContext> xContext = comphelper::getProcessComponentContext();

        uno::Reference<beans::XIntrospection> xIntro = beans::theIntrospection::get( xContext );
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
                    std::unique_ptr<std::unique_ptr<ScUnoAddInFuncData>[]> ppNew(new std::unique_ptr<ScUnoAddInFuncData>[nFuncCount]);
                    for (long i=0; i<nOld; i++)
                        ppNew[i] = std::move(ppFuncData[i]);
                    ppFuncData = std::move(ppNew);
                }
                else
                    ppFuncData.reset(new std::unique_ptr<ScUnoAddInFuncData>[nFuncCount]);

                //TODO: adjust bucket count?
                if ( !pExactHashMap )
                    pExactHashMap.reset( new ScAddInHashMap );
                if ( !pNameHashMap )
                    pNameHashMap.reset( new ScAddInHashMap );
                if ( !pLocalHashMap )
                    pLocalHashMap.reset( new ScAddInHashMap );

                const uno::Reference<reflection::XIdlMethod>* pArray = aMethods.getConstArray();
                for (long nFuncPos=0; nFuncPos<nNewCount; nFuncPos++)
                {
                    ppFuncData[nFuncPos+nOld] = nullptr;

                    uno::Reference<reflection::XIdlMethod> xFunc = pArray[nFuncPos];
                    if (xFunc.is())
                    {
                        //  leave out internal functions
                        uno::Reference<reflection::XIdlClass> xClass =
                                        xFunc->getDeclaringClass();
                        bool bSkip = true;
                        if ( xClass.is() )
                        {
                            //TODO: XIdlClass needs getType() method!
                            OUString sName = xClass->getName();
                            bSkip = (
                                IsTypeName( sName,
                                    cppu::UnoType<uno::XInterface>::get()) ||
                                IsTypeName( sName,
                                    cppu::UnoType<lang::XServiceName>::get()) ||
                                IsTypeName( sName,
                                    cppu::UnoType<lang::XServiceInfo>::get()) ||
                                IsTypeName( sName,
                                    cppu::UnoType<sheet::XAddIn>::get()) );
                        }
                        if (!bSkip)
                        {
                            uno::Reference<reflection::XIdlClass> xReturn =
                                        xFunc->getReturnType();
                            if ( !lcl_ValidReturnType( xReturn ) )
                                bSkip = true;
                        }
                        if (!bSkip)
                        {
                            OUString aFuncU = xFunc->getName();

                            // stored function name: (service name).(function)
                            OUStringBuffer aFuncNameBuffer( aServiceName.getLength()+1+aFuncU.getLength());
                            aFuncNameBuffer.append(aServiceName);
                            aFuncNameBuffer.append('.');
                            aFuncNameBuffer.append(aFuncU);
                            OUString aFuncName = aFuncNameBuffer.makeStringAndClear();

                            bool bValid = true;
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

                                OString sHelpId = aHelpIdGenerator.GetHelpId( aFuncU );

                                OUString aLocalName;
                                try
                                {
                                    aLocalName = xAddIn->
                                        getDisplayFunctionName( aFuncU );
                                }
                                catch(uno::Exception&)
                                {
                                    aLocalName = "###";
                                }

                                OUString aDescription;
                                try
                                {
                                    aDescription = xAddIn->
                                        getFunctionDescription( aFuncU );
                                }
                                catch(uno::Exception&)
                                {
                                    aDescription = "###";
                                }

                                std::unique_ptr<ScAddInArgDesc[]> pVisibleArgs;
                                if ( nVisibleCount > 0 )
                                {
                                    ScAddInArgDesc aDesc;
                                    pVisibleArgs.reset(new ScAddInArgDesc[nVisibleCount]);
                                    long nDestPos = 0;
                                    for (nParamPos=0; nParamPos<nParamCount; nParamPos++)
                                    {
                                        uno::Reference<reflection::XIdlClass> xParClass =
                                            pParArr[nParamPos].aType;
                                        ScAddInArgumentType eArgType = lcl_GetArgType( xParClass );
                                        if ( eArgType != SC_ADDINARG_CALLER )
                                        {
                                            OUString aArgName;
                                            try
                                            {
                                                aArgName = xAddIn->
                                                    getDisplayArgumentName( aFuncU, nParamPos );
                                            }
                                            catch(uno::Exception&)
                                            {
                                                aArgName = "###";
                                            }
                                            OUString aArgDesc;
                                            try
                                            {
                                                aArgDesc = xAddIn->
                                                    getArgumentDescription( aFuncU, nParamPos );
                                            }
                                            catch(uno::Exception&)
                                            {
                                                aArgDesc = "###";
                                            }

                                            bool bOptional =
                                                ( eArgType == SC_ADDINARG_VALUE_OR_ARRAY ||
                                                  eArgType == SC_ADDINARG_VARARGS );

                                            aDesc.eType = eArgType;
                                            aDesc.aName = aArgName;
                                            aDesc.aDescription = aArgDesc;
                                            aDesc.bOptional = bOptional;
                                            //TODO: initialize aInternalName only from config?
                                            aDesc.aInternalName = pParArr[nParamPos].aName;

                                            pVisibleArgs[nDestPos++] = aDesc;
                                        }
                                    }
                                    OSL_ENSURE( nDestPos==nVisibleCount, "wrong count" );
                                }

                                ppFuncData[nFuncPos+nOld].reset( new ScUnoAddInFuncData(
                                    aFuncName, aLocalName, aDescription,
                                    nCategory, sHelpId,
                                    xFunc, aObject,
                                    nVisibleCount, pVisibleArgs.get(), nCallerPos ) );

                                const ScUnoAddInFuncData* pData =
                                    ppFuncData[nFuncPos+nOld].get();
                                pExactHashMap->emplace(
                                            pData->GetOriginalName(),
                                            pData );
                                pNameHashMap->emplace(
                                            pData->GetUpperName(),
                                            pData );
                                pLocalHashMap->emplace(
                                            pData->GetUpperLocal(),
                                            pData );
                            }
                        }
                    }
                }
            }
        }
    }
}

static void lcl_UpdateFunctionList( const ScFunctionList& rFunctionList, const ScUnoAddInFuncData& rFuncData )
{
    const OUString& aCompare = rFuncData.GetUpperLocal();    // as used in FillFunctionDescFromData

    sal_uLong nCount = rFunctionList.GetCount();
    for (sal_uLong nPos=0; nPos<nCount; nPos++)
    {
        const ScFuncDesc* pDesc = rFunctionList.GetFunction( nPos );
        if ( pDesc && pDesc->mxFuncName && *pDesc->mxFuncName == aCompare )
        {
            ScUnoAddInCollection::FillFunctionDescFromData( rFuncData, *const_cast<ScFuncDesc*>(pDesc) );
            break;
        }
    }
}

static const ScAddInArgDesc* lcl_FindArgDesc( const ScUnoAddInFuncData& rFuncData, const OUString& rArgIntName )
{
    long nArgCount = rFuncData.GetArgumentCount();
    const ScAddInArgDesc* pArguments = rFuncData.GetArguments();
    for (long nPos=0; nPos<nArgCount; nPos++)
    {
        if ( pArguments[nPos].aInternalName == rArgIntName )
            return &pArguments[nPos];
    }
    return nullptr;
}

void ScUnoAddInCollection::UpdateFromAddIn( const uno::Reference<uno::XInterface>& xInterface,
                                            const OUString& rServiceName )
{
    uno::Reference<lang::XLocalizable> xLoc( xInterface, uno::UNO_QUERY );
    if ( xLoc.is() )        // optional in new add-ins
    {
        // fdo50118 when GetUseEnglishFunctionName() returns true, set the
        // locale to en-US to get English function names
        if ( SC_MOD()->GetFormulaOptions().GetUseEnglishFuncName() )
            xLoc->setLocale( lang::Locale( "en", "US", ""));
        else
            xLoc->setLocale( Application::GetSettings().GetUILanguageTag().getLocale());
    }

    // if function list was already initialized, it must be updated

    ScFunctionList* pFunctionList = nullptr;
    if ( ScGlobal::HasStarCalcFunctionList() )
        pFunctionList = ScGlobal::GetStarCalcFunctionList();

    // only get the function information from Introspection

    uno::Reference<uno::XComponentContext> xContext = comphelper::getProcessComponentContext();

    uno::Reference<beans::XIntrospection> xIntro = beans::theIntrospection::get(xContext);
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
                OUString aFuncU = xFunc->getName();

                // stored function name: (service name).(function)
                OUStringBuffer aFuncNameBuffer( rServiceName.getLength()+1+aFuncU.getLength());
                aFuncNameBuffer.append(rServiceName);
                aFuncNameBuffer.append('.');
                aFuncNameBuffer.append(aFuncU);
                OUString aFuncName = aFuncNameBuffer.makeStringAndClear();

                // internal names are skipped because no FuncData exists
                ScUnoAddInFuncData* pOldData = const_cast<ScUnoAddInFuncData*>( GetFuncData( aFuncName ) );
                if ( pOldData )
                {
                    // Create new (complete) argument info.
                    // As in ReadFromAddIn, the reflection information is authoritative.
                    // Local names and descriptions from pOldData are looked up using the
                    // internal argument name.

                    bool bValid = true;
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
                        std::unique_ptr<ScAddInArgDesc[]> pVisibleArgs;
                        if ( nVisibleCount > 0 )
                        {
                            ScAddInArgDesc aDesc;
                            pVisibleArgs.reset(new ScAddInArgDesc[nVisibleCount]);
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

                                    bool bOptional =
                                        ( eArgType == SC_ADDINARG_VALUE_OR_ARRAY ||
                                          eArgType == SC_ADDINARG_VARARGS );

                                    aDesc.eType = eArgType;
                                    aDesc.bOptional = bOptional;
                                    //TODO: initialize aInternalName only from config?
                                    aDesc.aInternalName = pParArr[nParamPos].aName;

                                    pVisibleArgs[nDestPos++] = aDesc;
                                }
                            }
                            OSL_ENSURE( nDestPos==nVisibleCount, "wrong count" );
                        }

                        pOldData->SetFunction( xFunc, aObject );
                        pOldData->SetArguments( nVisibleCount, pVisibleArgs.get() );
                        pOldData->SetCallerPos( nCallerPos );

                        if ( pFunctionList )
                            lcl_UpdateFunctionList( *pFunctionList, *pOldData );
                    }
                }
            }
        }
    }
}

const OUString & ScUnoAddInCollection::FindFunction( const OUString& rUpperName, bool bLocalFirst )
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
        //TODO: before that, check for exact match???

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

const ScUnoAddInFuncData* ScUnoAddInCollection::GetFuncData( const OUString& rName, bool bComplete )
{
    if (!bInitialized)
        Initialize();

    //  rName must be the exact internal name

    ScAddInHashMap::const_iterator iLook( pExactHashMap->find( rName ) );
    if ( iLook != pExactHashMap->end() )
    {
        const ScUnoAddInFuncData* pFuncData = iLook->second;

        if ( bComplete && !pFuncData->GetFunction().is() )           //TODO: extra flag?
            LoadComponent( *pFuncData );

        return pFuncData;
    }

    return nullptr;
}

const ScUnoAddInFuncData* ScUnoAddInCollection::GetFuncData( long nIndex )
{
    if (!bInitialized)
        Initialize();

    if (nIndex < nFuncCount)
        return ppFuncData[nIndex].get();
    return nullptr;
}

void ScUnoAddInCollection::LocalizeString( OUString& rName )
{
    if (!bInitialized)
        Initialize();

    //  modify rName - input: exact name

    ScAddInHashMap::const_iterator iLook( pExactHashMap->find( rName ) );
    if ( iLook != pExactHashMap->end() )
        rName = iLook->second->GetUpperLocal();         //TODO: upper?
}

long ScUnoAddInCollection::GetFuncCount()
{
    if (!bInitialized)
        Initialize();

    return nFuncCount;
}

bool ScUnoAddInCollection::FillFunctionDesc( long nFunc, ScFuncDesc& rDesc )
{
    if (!bInitialized)
        Initialize();

    if (nFunc >= nFuncCount || !ppFuncData[nFunc])
        return false;

    const ScUnoAddInFuncData& rFuncData = *ppFuncData[nFunc];

    return FillFunctionDescFromData( rFuncData, rDesc );
}

bool ScUnoAddInCollection::FillFunctionDescFromData( const ScUnoAddInFuncData& rFuncData, ScFuncDesc& rDesc )
{
    rDesc.Clear();

    bool bIncomplete = !rFuncData.GetFunction().is();       //TODO: extra flag?

    long nArgCount = rFuncData.GetArgumentCount();
    if ( nArgCount > SAL_MAX_UINT16 )
        return false;

    if ( bIncomplete )
        nArgCount = 0;      // if incomplete, fill without argument info (no wrong order)

    // nFIndex is set from outside

    rDesc.mxFuncName = rFuncData.GetUpperLocal();     //TODO: upper?
    rDesc.nCategory = rFuncData.GetCategory();
    rDesc.sHelpId = rFuncData.GetHelpId();

    OUString aDesc = rFuncData.GetDescription();
    if (aDesc.isEmpty())
        aDesc = rFuncData.GetLocalName();      // use name if no description is available
    rDesc.mxFuncDesc = aDesc ;

    // AddInArgumentType_CALLER is already left out in FuncData

    rDesc.nArgCount = static_cast<sal_uInt16>(nArgCount);
    if ( nArgCount )
    {
        bool bMultiple = false;
        const ScAddInArgDesc* pArgs = rFuncData.GetArguments();

        rDesc.maDefArgNames.clear();
        rDesc.maDefArgNames.resize(nArgCount);
        rDesc.maDefArgDescs.clear();
        rDesc.maDefArgDescs.resize(nArgCount);
        rDesc.pDefArgFlags   = new ScFuncDesc::ParameterFlags[nArgCount];
        for ( long nArg=0; nArg<nArgCount; nArg++ )
        {
            rDesc.maDefArgNames[nArg] = pArgs[nArg].aName;
            rDesc.maDefArgDescs[nArg] = pArgs[nArg].aDescription;
            rDesc.pDefArgFlags[nArg].bOptional = pArgs[nArg].bOptional;

            // no empty names...
            if ( rDesc.maDefArgNames[nArg].isEmpty() )
            {
                OUString aDefName("arg");
                aDefName += OUString::number( nArg+1 );
                rDesc.maDefArgNames[nArg] = aDefName;
            }

            //  last argument repeated?
            if ( nArg+1 == nArgCount && ( pArgs[nArg].eType == SC_ADDINARG_VARARGS ) )
                bMultiple = true;
        }

        if ( bMultiple )
            rDesc.nArgCount += VAR_ARGS - 1;    // VAR_ARGS means just one repeated arg
    }

    rDesc.bIncomplete = bIncomplete;

    return true;
}

ScUnoAddInCall::ScUnoAddInCall( ScUnoAddInCollection& rColl, const OUString& rName,
                                long nParamCount ) :
    bValidCount( false ),
    nErrCode( FormulaError::NoCode ),      // before function was called
    bHasString( true ),
    fValue( 0.0 ),
    xMatrix( nullptr )
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
            bValidCount = true;
        }
        else if ( nParamCount <= nDescCount )
        {
            //  all args behind nParamCount must be optional
            bValidCount = true;
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
    return SC_ADDINARG_VALUE_OR_ARRAY;      //TODO: error code !!!!
}

bool ScUnoAddInCall::NeedsCaller() const
{
    return pFuncData && pFuncData->GetCallerPos() != SC_CALLERPOS_NONE;
}

void ScUnoAddInCall::SetCaller( const uno::Reference<uno::XInterface>& rInterface )
{
    xCaller = rInterface;
}

void ScUnoAddInCall::SetCallerFromObjectShell( const SfxObjectShell* pObjSh )
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
        //TODO: after inserting caller (to prevent copying twice)?

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
        nErrCode = FormulaError::NONE;

        try
        {
            aAny = xFunction->invoke( aObject, rCallArgs );
        }
        catch(lang::IllegalArgumentException&)
        {
            nErrCode = FormulaError::IllegalArgument;
        }
        catch(const reflection::InvocationTargetException& rWrapped)
        {
            if ( rWrapped.TargetException.getValueType().equals(
                    cppu::UnoType<lang::IllegalArgumentException>::get()) )
                nErrCode = FormulaError::IllegalArgument;
            else if ( rWrapped.TargetException.getValueType().equals(
                    cppu::UnoType<sheet::NoConvergenceException>::get()) )
                nErrCode = FormulaError::NoConvergence;
            else
                nErrCode = FormulaError::NoValue;
        }
        catch(uno::Exception&)
        {
            nErrCode = FormulaError::NoValue;
        }

        if (nErrCode == FormulaError::NONE)
            SetResult( aAny );      // convert result to Calc types
    }
}

void ScUnoAddInCall::SetResult( const uno::Any& rNewRes )
{
    nErrCode = FormulaError::NONE;
    xVarRes = nullptr;

    // Reflection* pRefl = rNewRes.getReflection();

    uno::TypeClass eClass = rNewRes.getValueTypeClass();
    const uno::Type& aType = rNewRes.getValueType();
    switch (eClass)
    {
        case uno::TypeClass_VOID:
            nErrCode = FormulaError::NotAvailable;         // #NA
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
                bHasString = true;
            }
            break;

        case uno::TypeClass_INTERFACE:
            {
                //TODO: directly extract XVolatileResult from any?
                uno::Reference<uno::XInterface> xInterface;
                rNewRes >>= xInterface;
                if ( xInterface.is() )
                    xVarRes.set( xInterface, uno::UNO_QUERY );

                if (!xVarRes.is())
                    nErrCode = FormulaError::NoValue;          // unknown interface
            }
            break;

        default:
            if ( aType.equals( cppu::UnoType<uno::Sequence< uno::Sequence<sal_Int32> >>::get() ) )
            {
                const uno::Sequence< uno::Sequence<sal_Int32> >* pRowSeq = nullptr;

                //TODO: use pointer from any!
                uno::Sequence< uno::Sequence<sal_Int32> > aSequence;
                if ( rNewRes >>= aSequence )
                    pRowSeq = &aSequence;

                if ( pRowSeq )
                {
                    long nRowCount = pRowSeq->getLength();
                    const uno::Sequence<sal_Int32>* pRowArr = pRowSeq->getConstArray();
                    long nMaxColCount = 0;
                    for (long nRow=0; nRow<nRowCount; nRow++)
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
                        for (long nRow=0; nRow<nRowCount; nRow++)
                        {
                            long nColCount = pRowArr[nRow].getLength();
                            const sal_Int32* pColArr = pRowArr[nRow].getConstArray();
                            for (long nCol=0; nCol<nColCount; nCol++)
                                xMatrix->PutDouble( pColArr[nCol],
                                        static_cast<SCSIZE>(nCol),
                                        static_cast<SCSIZE>(nRow) );
                            for (long nCol=nColCount; nCol<nMaxColCount; nCol++)
                                xMatrix->PutDouble( 0.0,
                                        static_cast<SCSIZE>(nCol),
                                        static_cast<SCSIZE>(nRow) );
                        }
                    }
                }
            }
            else if ( aType.equals( cppu::UnoType<uno::Sequence< uno::Sequence<double> >>::get() ) )
            {
                const uno::Sequence< uno::Sequence<double> >* pRowSeq = nullptr;

                //TODO: use pointer from any!
                uno::Sequence< uno::Sequence<double> > aSequence;
                if ( rNewRes >>= aSequence )
                    pRowSeq = &aSequence;

                if ( pRowSeq )
                {
                    long nRowCount = pRowSeq->getLength();
                    const uno::Sequence<double>* pRowArr = pRowSeq->getConstArray();
                    long nMaxColCount = 0;
                    for (long nRow=0; nRow<nRowCount; nRow++)
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
                        for (long nRow=0; nRow<nRowCount; nRow++)
                        {
                            long nColCount = pRowArr[nRow].getLength();
                            const double* pColArr = pRowArr[nRow].getConstArray();
                            for (long nCol=0; nCol<nColCount; nCol++)
                                xMatrix->PutDouble( pColArr[nCol],
                                        static_cast<SCSIZE>(nCol),
                                        static_cast<SCSIZE>(nRow) );
                            for (long nCol=nColCount; nCol<nMaxColCount; nCol++)
                                xMatrix->PutDouble( 0.0,
                                        static_cast<SCSIZE>(nCol),
                                        static_cast<SCSIZE>(nRow) );
                        }
                    }
                }
            }
            else if ( aType.equals( cppu::UnoType<uno::Sequence< uno::Sequence<OUString> >>::get() ) )
            {
                const uno::Sequence< uno::Sequence<OUString> >* pRowSeq = nullptr;

                //TODO: use pointer from any!
                uno::Sequence< uno::Sequence<OUString> > aSequence;
                if ( rNewRes >>= aSequence )
                    pRowSeq = &aSequence;

                if ( pRowSeq )
                {
                    long nRowCount = pRowSeq->getLength();
                    const uno::Sequence<OUString>* pRowArr = pRowSeq->getConstArray();
                    long nMaxColCount = 0;
                    for (long nRow=0; nRow<nRowCount; nRow++)
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
                        for (long nRow=0; nRow<nRowCount; nRow++)
                        {
                            long nColCount = pRowArr[nRow].getLength();
                            const OUString* pColArr = pRowArr[nRow].getConstArray();
                            for (long nCol=0; nCol<nColCount; nCol++)
                            {
                                xMatrix->PutString(
                                    svl::SharedString(pColArr[nCol]),
                                    static_cast<SCSIZE>(nCol), static_cast<SCSIZE>(nRow));
                            }
                            for (long nCol=nColCount; nCol<nMaxColCount; nCol++)
                            {
                                xMatrix->PutString(
                                    svl::SharedString(EMPTY_OUSTRING),
                                    static_cast<SCSIZE>(nCol), static_cast<SCSIZE>(nRow));
                            }
                        }
                    }
                }
            }
            else if ( aType.equals( cppu::UnoType<uno::Sequence< uno::Sequence<uno::Any> >>::get() ) )
            {
                xMatrix = ScSequenceToMatrix::CreateMixedMatrix( rNewRes );
            }

            if (!xMatrix)                       // no array found
                nErrCode = FormulaError::NoValue;          //TODO: code for error in return type???
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
