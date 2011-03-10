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

#include "appluno.hxx"
#include "sal/types.h"
#include <osl/diagnose.h>
#include <cppuhelper/factory.hxx>

#include <sfx2/app.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include "afmtuno.hxx"
#include "funcuno.hxx"
#include "filtuno.hxx"
#include "miscuno.hxx"
#include "scmod.hxx"
#include "appoptio.hxx"
#include "inputopt.hxx"
#include "printopt.hxx"
#include "userlist.hxx"
#include "sc.hrc"           // VAR_ARGS
#include "unonames.hxx"
#include "funcdesc.hxx"
#include <com/sun/star/sheet/FunctionArgument.hpp>

using namespace com::sun::star;

//------------------------------------------------------------------------

// Calc document
extern uno::Sequence< rtl::OUString > SAL_CALL ScDocument_getSupportedServiceNames() throw();
extern rtl::OUString SAL_CALL ScDocument_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScDocument_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr, const sal_uInt64 _nCreationFlags ) throw( uno::Exception );

// Calc XML import
extern uno::Sequence< rtl::OUString > SAL_CALL ScXMLImport_getSupportedServiceNames() throw();
extern rtl::OUString SAL_CALL ScXMLImport_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLImport_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );
extern uno::Sequence< rtl::OUString > SAL_CALL ScXMLImport_Meta_getSupportedServiceNames() throw();
extern rtl::OUString SAL_CALL ScXMLImport_Meta_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLImport_Meta_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );
extern uno::Sequence< rtl::OUString > SAL_CALL ScXMLImport_Styles_getSupportedServiceNames() throw();
extern rtl::OUString SAL_CALL ScXMLImport_Styles_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLImport_Styles_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );
extern uno::Sequence< rtl::OUString > SAL_CALL ScXMLImport_Content_getSupportedServiceNames() throw();
extern rtl::OUString SAL_CALL ScXMLImport_Content_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLImport_Content_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );
extern uno::Sequence< rtl::OUString > SAL_CALL ScXMLImport_Settings_getSupportedServiceNames() throw();
extern rtl::OUString SAL_CALL ScXMLImport_Settings_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLImport_Settings_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );

// Calc XML export
extern uno::Sequence< rtl::OUString > SAL_CALL ScXMLOOoExport_getSupportedServiceNames() throw();
extern rtl::OUString SAL_CALL ScXMLOOoExport_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLOOoExport_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );
extern uno::Sequence< rtl::OUString > SAL_CALL ScXMLOOoExport_Meta_getSupportedServiceNames() throw();
extern rtl::OUString SAL_CALL ScXMLOOoExport_Meta_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLOOoExport_Meta_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );
extern uno::Sequence< rtl::OUString > SAL_CALL ScXMLOOoExport_Styles_getSupportedServiceNames() throw();
extern rtl::OUString SAL_CALL ScXMLOOoExport_Styles_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLOOoExport_Styles_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );
extern uno::Sequence< rtl::OUString > SAL_CALL ScXMLOOoExport_Content_getSupportedServiceNames() throw();
extern rtl::OUString SAL_CALL ScXMLOOoExport_Content_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLOOoExport_Content_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );
extern uno::Sequence< rtl::OUString > SAL_CALL ScXMLOOoExport_Settings_getSupportedServiceNames() throw();
extern rtl::OUString SAL_CALL ScXMLOOoExport_Settings_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLOOoExport_Settings_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );

// Calc XML Oasis export
extern uno::Sequence< rtl::OUString > SAL_CALL ScXMLOasisExport_getSupportedServiceNames() throw();
extern rtl::OUString SAL_CALL ScXMLOasisExport_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLOasisExport_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );
extern uno::Sequence< rtl::OUString > SAL_CALL ScXMLOasisExport_Meta_getSupportedServiceNames() throw();
extern rtl::OUString SAL_CALL ScXMLOasisExport_Meta_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLOasisExport_Meta_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );
extern uno::Sequence< rtl::OUString > SAL_CALL ScXMLOasisExport_Styles_getSupportedServiceNames() throw();
extern rtl::OUString SAL_CALL ScXMLOasisExport_Styles_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLOasisExport_Styles_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );
extern uno::Sequence< rtl::OUString > SAL_CALL ScXMLOasisExport_Content_getSupportedServiceNames() throw();
extern rtl::OUString SAL_CALL ScXMLOasisExport_Content_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLOasisExport_Content_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );
extern uno::Sequence< rtl::OUString > SAL_CALL ScXMLOasisExport_Settings_getSupportedServiceNames() throw();
extern rtl::OUString SAL_CALL ScXMLOasisExport_Settings_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLOasisExport_Settings_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );

//------------------------------------------------------------------------

//  Anzahl der Funktionen, die als zuletzt benutzt gespeichert werden
//! Define mit funcpage.hxx und dwfunctr.hxx zusammenfassen !!!
#define LRU_MAX 10

//  Spezial-Werte fuer Zoom
//! irgendwo zentral
#define SC_ZOOMVAL_OPTIMAL      (-1)
#define SC_ZOOMVAL_WHOLEPAGE    (-2)
#define SC_ZOOMVAL_PAGEWIDTH    (-3)

//  Anzahl der PropertyValues in einer Function-Description
#define SC_FUNCDESC_PROPCOUNT   5

//------------------------------------------------------------------------

//  alles ohne Which-ID, Map nur fuer PropertySetInfo

static const SfxItemPropertyMapEntry* lcl_GetSettingsPropertyMap()
{
    static SfxItemPropertyMapEntry aSettingsPropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_DOAUTOCP), 0,  &getBooleanCppuType(),              0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_ENTERED),  0,  &getBooleanCppuType(),              0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_EXPREF),   0,  &getBooleanCppuType(),              0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_EXTFMT),   0,  &getBooleanCppuType(),              0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_LINKUPD),  0,  &getCppuType((sal_Int16*)0),        0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_MARKHDR),  0,  &getBooleanCppuType(),              0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_METRIC),   0,  &getCppuType((sal_Int16*)0),        0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_MOVEDIR),  0,  &getCppuType((sal_Int16*)0),        0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_MOVESEL),  0,  &getBooleanCppuType(),              0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_PRALLSH),  0,  &getBooleanCppuType(),              0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_PREMPTY),  0,  &getBooleanCppuType(),              0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_RANGEFIN), 0,  &getBooleanCppuType(),              0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_SCALE),    0,  &getCppuType((sal_Int16*)0),        0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_STBFUNC),  0,  &getCppuType((sal_Int16*)0),        0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_ULISTS),   0,  &getCppuType((uno::Sequence<rtl::OUString>*)0), 0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_PRMETRICS),0,  &getBooleanCppuType(),              0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_USETABCOL),0,  &getBooleanCppuType(),              0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_REPLWARN), 0,  &getBooleanCppuType(),              0, 0},
        {0,0,0,0,0,0}
    };
    return aSettingsPropertyMap_Impl;
}

//------------------------------------------------------------------------

#define SCFUNCTIONLISTOBJ_SERVICE       "com.sun.star.sheet.FunctionDescriptions"
#define SCRECENTFUNCTIONSOBJ_SERVICE    "com.sun.star.sheet.RecentFunctions"
#define SCSPREADSHEETSETTINGS_SERVICE   "com.sun.star.sheet.GlobalSheetSettings"

SC_SIMPLE_SERVICE_INFO( ScFunctionListObj, "ScFunctionListObj", SCFUNCTIONLISTOBJ_SERVICE )
SC_SIMPLE_SERVICE_INFO( ScRecentFunctionsObj, "ScRecentFunctionsObj", SCRECENTFUNCTIONSOBJ_SERVICE )
SC_SIMPLE_SERVICE_INFO( ScSpreadsheetSettings, "ScSpreadsheetSettings", SCSPREADSHEETSETTINGS_SERVICE )

//------------------------------------------------------------------------

static void lcl_WriteInfo( registry::XRegistryKey* pRegistryKey,
                        const rtl::OUString& rImplementationName,
                        const uno::Sequence< rtl::OUString >& rServices )
                    throw( registry::InvalidRegistryException )
{
    rtl::OUString aImpl(RTL_CONSTASCII_USTRINGPARAM( "/" ));
    aImpl += rImplementationName;
    aImpl += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES" ));
    uno::Reference<registry::XRegistryKey> xNewKey(pRegistryKey->createKey(aImpl));

    const rtl::OUString* pArray = rServices.getConstArray();
    for( sal_Int32 i = 0; i < rServices.getLength(); i++ )
        xNewKey->createKey( pArray[i]);
}

extern "C" {

SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** /* ppEnv */ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * /* pRegistryKey */ )
{
    if (!pServiceManager)
        return NULL;

    uno::Reference<lang::XSingleServiceFactory> xFactory;
    rtl::OUString aImpl(rtl::OUString::createFromAscii(pImplName));

    if ( aImpl == ScSpreadsheetSettings::getImplementationName_Static() )
        xFactory.set(cppu::createOneInstanceFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScSpreadsheetSettings::getImplementationName_Static(),
                ScSpreadsheetSettings_CreateInstance,
                ScSpreadsheetSettings::getSupportedServiceNames_Static() ));

    if ( aImpl == ScRecentFunctionsObj::getImplementationName_Static() )
        xFactory.set(cppu::createOneInstanceFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScRecentFunctionsObj::getImplementationName_Static(),
                ScRecentFunctionsObj_CreateInstance,
                ScRecentFunctionsObj::getSupportedServiceNames_Static() ));

    if ( aImpl == ScFunctionListObj::getImplementationName_Static() )
        xFactory.set(cppu::createOneInstanceFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScFunctionListObj::getImplementationName_Static(),
                ScFunctionListObj_CreateInstance,
                ScFunctionListObj::getSupportedServiceNames_Static() ));

    if ( aImpl == ScAutoFormatsObj::getImplementationName_Static() )
        xFactory.set(cppu::createOneInstanceFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScAutoFormatsObj::getImplementationName_Static(),
                ScAutoFormatsObj_CreateInstance,
                ScAutoFormatsObj::getSupportedServiceNames_Static() ));

    if ( aImpl == ScFunctionAccess::getImplementationName_Static() )
        xFactory.set(cppu::createOneInstanceFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScFunctionAccess::getImplementationName_Static(),
                ScFunctionAccess_CreateInstance,
                ScFunctionAccess::getSupportedServiceNames_Static() ));

    if ( aImpl == ScFilterOptionsObj::getImplementationName_Static() )
        xFactory.set(cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScFilterOptionsObj::getImplementationName_Static(),
                ScFilterOptionsObj_CreateInstance,
                ScFilterOptionsObj::getSupportedServiceNames_Static() ));

    if ( aImpl == ScXMLImport_getImplementationName() )
        xFactory.set(cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLImport_getImplementationName(),
                ScXMLImport_createInstance,
                ScXMLImport_getSupportedServiceNames() ));

    if ( aImpl == ScXMLImport_Meta_getImplementationName() )
        xFactory.set(cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLImport_Meta_getImplementationName(),
                ScXMLImport_Meta_createInstance,
                ScXMLImport_Meta_getSupportedServiceNames() ));

    if ( aImpl == ScXMLImport_Styles_getImplementationName() )
        xFactory.set(cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLImport_Styles_getImplementationName(),
                ScXMLImport_Styles_createInstance,
                ScXMLImport_Styles_getSupportedServiceNames() ));

    if ( aImpl == ScXMLImport_Content_getImplementationName() )
        xFactory.set(cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLImport_Content_getImplementationName(),
                ScXMLImport_Content_createInstance,
                ScXMLImport_Content_getSupportedServiceNames() ));

    if ( aImpl == ScXMLImport_Settings_getImplementationName() )
        xFactory.set(cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLImport_Settings_getImplementationName(),
                ScXMLImport_Settings_createInstance,
                ScXMLImport_Settings_getSupportedServiceNames() ));

    if ( aImpl == ScXMLOOoExport_getImplementationName() )
        xFactory = cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLOOoExport_getImplementationName(),
                ScXMLOOoExport_createInstance,
                ScXMLOOoExport_getSupportedServiceNames() );

    if ( aImpl == ScXMLOOoExport_Meta_getImplementationName() )
        xFactory = cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLOOoExport_Meta_getImplementationName(),
                ScXMLOOoExport_Meta_createInstance,
                ScXMLOOoExport_Meta_getSupportedServiceNames() );

    if ( aImpl == ScXMLOOoExport_Styles_getImplementationName() )
        xFactory = cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLOOoExport_Styles_getImplementationName(),
                ScXMLOOoExport_Styles_createInstance,
                ScXMLOOoExport_Styles_getSupportedServiceNames() );

    if ( aImpl == ScXMLOOoExport_Content_getImplementationName() )
        xFactory = cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLOOoExport_Content_getImplementationName(),
                ScXMLOOoExport_Content_createInstance,
                ScXMLOOoExport_Content_getSupportedServiceNames() );

    if ( aImpl == ScXMLOOoExport_Settings_getImplementationName() )
        xFactory = cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLOOoExport_Settings_getImplementationName(),
                ScXMLOOoExport_Settings_createInstance,
                ScXMLOOoExport_Settings_getSupportedServiceNames() );

    if ( aImpl == ScXMLOasisExport_getImplementationName() )
        xFactory = cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLOasisExport_getImplementationName(),
                ScXMLOasisExport_createInstance,
                ScXMLOasisExport_getSupportedServiceNames() );
    if ( aImpl == ScXMLOasisExport_Meta_getImplementationName() )
        xFactory = cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLOasisExport_Meta_getImplementationName(),
                ScXMLOasisExport_Meta_createInstance,
                ScXMLOasisExport_Meta_getSupportedServiceNames() );
    if ( aImpl == ScXMLOasisExport_Styles_getImplementationName() )
        xFactory = cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLOasisExport_Styles_getImplementationName(),
                ScXMLOasisExport_Styles_createInstance,
                ScXMLOasisExport_Styles_getSupportedServiceNames() );
    if ( aImpl == ScXMLOasisExport_Content_getImplementationName() )
        xFactory = cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLOasisExport_Content_getImplementationName(),
                ScXMLOasisExport_Content_createInstance,
                ScXMLOasisExport_Content_getSupportedServiceNames() );
    if ( aImpl == ScXMLOasisExport_Settings_getImplementationName() )
        xFactory = cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLOasisExport_Settings_getImplementationName(),
                ScXMLOasisExport_Settings_createInstance,
                ScXMLOasisExport_Settings_getSupportedServiceNames() );

    if ( aImpl == ScDocument_getImplementationName() )
        xFactory.set(sfx2::createSfxModelFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScDocument_getImplementationName(),
                ScDocument_createInstance,
                ScDocument_getSupportedServiceNames() ));

    void* pRet = NULL;
    if (xFactory.is())
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}

}   // extern C

//------------------------------------------------------------------------

ScSpreadsheetSettings::ScSpreadsheetSettings() :
    aPropSet( lcl_GetSettingsPropertyMap() )
{
}

ScSpreadsheetSettings::~ScSpreadsheetSettings()
{
}

uno::Reference<uno::XInterface> SAL_CALL ScSpreadsheetSettings_CreateInstance(
                        const uno::Reference<lang::XMultiServiceFactory>& /* rSMgr */ )
{
    SolarMutexGuard aGuard;
    ScDLL::Init();
    static uno::Reference<uno::XInterface> xInst((cppu::OWeakObject*)new ScSpreadsheetSettings());
    return xInst;
}

rtl::OUString ScSpreadsheetSettings::getImplementationName_Static()
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "stardiv.StarCalc.ScSpreadsheetSettings" ));
}

uno::Sequence<rtl::OUString> ScSpreadsheetSettings::getSupportedServiceNames_Static()
{
    uno::Sequence<rtl::OUString> aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SCSPREADSHEETSETTINGS_SERVICE ));
    return aRet;
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScSpreadsheetSettings::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() ));
    return aRef;
}

void SAL_CALL ScSpreadsheetSettings::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    String aString(aPropertyName);

    ScModule* pScMod = SC_MOD();
    ScAppOptions   aAppOpt(pScMod->GetAppOptions());
    ScInputOptions aInpOpt(pScMod->GetInputOptions());
    sal_Bool bSaveApp = false;
    sal_Bool bSaveInp = false;
    // print options aren't loaded until needed

    if (aString.EqualsAscii( SC_UNONAME_DOAUTOCP ))
    {
        aAppOpt.SetAutoComplete( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveApp = sal_True;
    }
    else if (aString.EqualsAscii( SC_UNONAME_ENTERED ))
    {
        aInpOpt.SetEnterEdit( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = sal_True;
    }
    else if (aString.EqualsAscii( SC_UNONAME_EXPREF ))
    {
        aInpOpt.SetExpandRefs( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = sal_True;
    }
    else if (aString.EqualsAscii( SC_UNONAME_EXTFMT ))
    {
        aInpOpt.SetExtendFormat( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = sal_True;
    }
    else if (aString.EqualsAscii( SC_UNONAME_LINKUPD ))
    {
        aAppOpt.SetLinkMode( (ScLkUpdMode) ScUnoHelpFunctions::GetInt16FromAny( aValue ) );
        bSaveApp = sal_True;
    }
    else if (aString.EqualsAscii( SC_UNONAME_MARKHDR ))
    {
        aInpOpt.SetMarkHeader( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = sal_True;
    }
    else if (aString.EqualsAscii( SC_UNONAME_MOVESEL ))
    {
        aInpOpt.SetMoveSelection( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = sal_True;
    }
    else if (aString.EqualsAscii( SC_UNONAME_RANGEFIN ))
    {
        aInpOpt.SetRangeFinder( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = sal_True;
    }
    else if (aString.EqualsAscii( SC_UNONAME_USETABCOL ))
    {
        aInpOpt.SetUseTabCol( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = sal_True;
    }
    else if (aString.EqualsAscii( SC_UNONAME_PRMETRICS ))
    {
        aInpOpt.SetTextWysiwyg( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = sal_True;
    }
    else if (aString.EqualsAscii( SC_UNONAME_REPLWARN ))
    {
        aInpOpt.SetReplaceCellsWarn( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = sal_True;
    }
    else if (aString.EqualsAscii( SC_UNONAME_METRIC ))
    {
        aAppOpt.SetAppMetric( (FieldUnit) ScUnoHelpFunctions::GetInt16FromAny( aValue ) );
        bSaveApp = sal_True;
    }
    else if (aString.EqualsAscii( SC_UNONAME_MOVEDIR ))
    {
        aInpOpt.SetMoveDir( ScUnoHelpFunctions::GetInt16FromAny( aValue ) );
        bSaveInp = sal_True;
    }
    else if (aString.EqualsAscii( SC_UNONAME_SCALE ))
    {
        short nVal = ScUnoHelpFunctions::GetInt16FromAny( aValue );
        if ( nVal < 0 )
        {
            SvxZoomType eType = SVX_ZOOM_PERCENT;
            switch (nVal)
            {
                case SC_ZOOMVAL_OPTIMAL:    eType = SVX_ZOOM_OPTIMAL;   break;
                case SC_ZOOMVAL_WHOLEPAGE:  eType = SVX_ZOOM_WHOLEPAGE; break;
                case SC_ZOOMVAL_PAGEWIDTH:  eType = SVX_ZOOM_PAGEWIDTH; break;
            }
            aAppOpt.SetZoomType( eType );
        }
        else if ( nVal >= MINZOOM && nVal <= MAXZOOM )
        {
            aAppOpt.SetZoom( nVal );
            aAppOpt.SetZoomType( SVX_ZOOM_PERCENT );
        }
        bSaveApp = sal_True;
    }
    else if (aString.EqualsAscii( SC_UNONAME_STBFUNC ))
    {
        aAppOpt.SetStatusFunc( ScUnoHelpFunctions::GetInt16FromAny( aValue ) );
        bSaveApp = sal_True;
    }
    else if (aString.EqualsAscii( SC_UNONAME_ULISTS ))
    {
        ScUserList* pUserList = ScGlobal::GetUserList();
        uno::Sequence<rtl::OUString> aSeq;
        if ( pUserList && ( aValue >>= aSeq ) )
        {
            //  es wird direkt die "lebende" Liste veraendert,
            //  mehr tut ScGlobal::SetUserList auch nicht

            pUserList->FreeAll();                   // alle Eintraege raus
            sal_uInt16 nCount = (sal_uInt16)aSeq.getLength();
            const rtl::OUString* pAry = aSeq.getConstArray();
            for (sal_uInt16 i=0; i<nCount; i++)
            {
                String aEntry = pAry[i];
                ScUserListData* pData = new ScUserListData(aEntry);
                if (!pUserList->Insert(pData))      // hinten anhaengen
                    delete pData;                   // sollte nicht vorkommen
            }
            bSaveApp = sal_True;    // Liste wird mit den App-Optionen gespeichert
        }
    }
    else if (aString.EqualsAscii( SC_UNONAME_PRALLSH ))
    {
        ScPrintOptions aPrintOpt(pScMod->GetPrintOptions());
        aPrintOpt.SetAllSheets( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        pScMod->SetPrintOptions( aPrintOpt );
    }
    else if (aString.EqualsAscii( SC_UNONAME_PREMPTY ))
    {
        ScPrintOptions aPrintOpt(pScMod->GetPrintOptions());
        aPrintOpt.SetSkipEmpty( !ScUnoHelpFunctions::GetBoolFromAny( aValue ) );    // reversed
        pScMod->SetPrintOptions( aPrintOpt );
        SFX_APP()->Broadcast( SfxSimpleHint( SID_SCPRINTOPTIONS ) );    // update previews
    }

    if ( bSaveApp )
        pScMod->SetAppOptions( aAppOpt );
    if ( bSaveInp )
        pScMod->SetInputOptions( aInpOpt );
}

uno::Any SAL_CALL ScSpreadsheetSettings::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    String aString = aPropertyName;
    uno::Any aRet;

    ScModule* pScMod = SC_MOD();
    ScAppOptions   aAppOpt = pScMod->GetAppOptions();
    ScInputOptions aInpOpt = pScMod->GetInputOptions();
    // print options aren't loaded until needed

    if (aString.EqualsAscii( SC_UNONAME_DOAUTOCP ))     ScUnoHelpFunctions::SetBoolInAny( aRet, aAppOpt.GetAutoComplete() );
    else if (aString.EqualsAscii( SC_UNONAME_ENTERED )) ScUnoHelpFunctions::SetBoolInAny( aRet, aInpOpt.GetEnterEdit() );
    else if (aString.EqualsAscii( SC_UNONAME_EXPREF ))  ScUnoHelpFunctions::SetBoolInAny( aRet, aInpOpt.GetExpandRefs() );
    else if (aString.EqualsAscii( SC_UNONAME_EXTFMT ))  ScUnoHelpFunctions::SetBoolInAny( aRet, aInpOpt.GetExtendFormat() );
    else if (aString.EqualsAscii( SC_UNONAME_LINKUPD )) aRet <<= (sal_Int16) aAppOpt.GetLinkMode();
    else if (aString.EqualsAscii( SC_UNONAME_MARKHDR )) ScUnoHelpFunctions::SetBoolInAny( aRet, aInpOpt.GetMarkHeader() );
    else if (aString.EqualsAscii( SC_UNONAME_MOVESEL )) ScUnoHelpFunctions::SetBoolInAny( aRet, aInpOpt.GetMoveSelection() );
    else if (aString.EqualsAscii( SC_UNONAME_RANGEFIN )) ScUnoHelpFunctions::SetBoolInAny( aRet, aInpOpt.GetRangeFinder() );
    else if (aString.EqualsAscii( SC_UNONAME_USETABCOL )) ScUnoHelpFunctions::SetBoolInAny( aRet, aInpOpt.GetUseTabCol() );
    else if (aString.EqualsAscii( SC_UNONAME_PRMETRICS )) ScUnoHelpFunctions::SetBoolInAny( aRet, aInpOpt.GetTextWysiwyg() );
    else if (aString.EqualsAscii( SC_UNONAME_REPLWARN )) ScUnoHelpFunctions::SetBoolInAny( aRet, aInpOpt.GetReplaceCellsWarn() );
    else if (aString.EqualsAscii( SC_UNONAME_METRIC ))  aRet <<= (sal_Int16) aAppOpt.GetAppMetric();
    else if (aString.EqualsAscii( SC_UNONAME_MOVEDIR )) aRet <<= (sal_Int16) aInpOpt.GetMoveDir();
    else if (aString.EqualsAscii( SC_UNONAME_STBFUNC )) aRet <<= (sal_Int16) aAppOpt.GetStatusFunc();
    else if (aString.EqualsAscii( SC_UNONAME_SCALE ))
    {
        sal_Int16 nZoomVal = 0;
        switch ( aAppOpt.GetZoomType() )
        {
            case SVX_ZOOM_PERCENT:   nZoomVal = aAppOpt.GetZoom();    break;
            case SVX_ZOOM_OPTIMAL:   nZoomVal = SC_ZOOMVAL_OPTIMAL;   break;
            case SVX_ZOOM_WHOLEPAGE: nZoomVal = SC_ZOOMVAL_WHOLEPAGE; break;
            case SVX_ZOOM_PAGEWIDTH: nZoomVal = SC_ZOOMVAL_PAGEWIDTH; break;
            default:
            {
                // added to avoid warnings
            }
        }
        aRet <<= (sal_Int16) nZoomVal;
    }
    else if (aString.EqualsAscii( SC_UNONAME_ULISTS ))
    {
        ScUserList* pUserList = ScGlobal::GetUserList();
        if (pUserList)
        {
            sal_uInt16 nCount = pUserList->GetCount();
            uno::Sequence<rtl::OUString> aSeq(nCount);
            rtl::OUString* pAry = aSeq.getArray();
            for (sal_uInt16 i=0; i<nCount; i++)
            {
                String aEntry((*pUserList)[i]->GetString());
                pAry[i] = aEntry;
            }
            aRet <<= aSeq;
        }
    }
    else if (aString.EqualsAscii( SC_UNONAME_PRALLSH ))
        ScUnoHelpFunctions::SetBoolInAny( aRet, pScMod->GetPrintOptions().GetAllSheets() );
    else if (aString.EqualsAscii( SC_UNONAME_PREMPTY ))
        ScUnoHelpFunctions::SetBoolInAny( aRet, !pScMod->GetPrintOptions().GetSkipEmpty() );    // reversed

    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScSpreadsheetSettings )

//------------------------------------------------------------------------

ScRecentFunctionsObj::ScRecentFunctionsObj()
{
}

ScRecentFunctionsObj::~ScRecentFunctionsObj()
{
}

// stuff for exService_...

uno::Reference<uno::XInterface> SAL_CALL ScRecentFunctionsObj_CreateInstance(
                        const uno::Reference<lang::XMultiServiceFactory>& /* rSMgr */ )
{
    SolarMutexGuard aGuard;
    ScDLL::Init();
    static uno::Reference<uno::XInterface> xInst((cppu::OWeakObject*)new ScRecentFunctionsObj());
    return xInst;
}

rtl::OUString ScRecentFunctionsObj::getImplementationName_Static()
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "stardiv.StarCalc.ScRecentFunctionsObj" ));
}

uno::Sequence<rtl::OUString> ScRecentFunctionsObj::getSupportedServiceNames_Static()
{
    uno::Sequence<rtl::OUString> aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SCRECENTFUNCTIONSOBJ_SERVICE ));
    return aRet;
}

// XRecentFunctions

uno::Sequence<sal_Int32> SAL_CALL ScRecentFunctionsObj::getRecentFunctionIds()
                                                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    const ScAppOptions& rOpt = SC_MOD()->GetAppOptions();
    sal_uInt16 nCount = rOpt.GetLRUFuncListCount();
    const sal_uInt16* pFuncs = rOpt.GetLRUFuncList();
    if (pFuncs)
    {
        uno::Sequence<sal_Int32> aSeq(nCount);
        sal_Int32* pAry = aSeq.getArray();
        for (sal_uInt16 i=0; i<nCount; i++)
            pAry[i] = pFuncs[i];
        return aSeq;
    }
    return uno::Sequence<sal_Int32>(0);
}

void SAL_CALL ScRecentFunctionsObj::setRecentFunctionIds(
                    const uno::Sequence<sal_Int32>& aRecentFunctionIds )
                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    sal_uInt16 nCount = (sal_uInt16) Min( aRecentFunctionIds.getLength(), (INT32) LRU_MAX );
    const INT32* pAry = aRecentFunctionIds.getConstArray();

    sal_uInt16* pFuncs = nCount ? new sal_uInt16[nCount] : NULL;
    for (sal_uInt16 i=0; i<nCount; i++)
        pFuncs[i] = (sal_uInt16)pAry[i];        //! auf gueltige Werte testen?

    ScModule* pScMod = SC_MOD();
    ScAppOptions aNewOpts(pScMod->GetAppOptions());
    aNewOpts.SetLRUFuncList(pFuncs, nCount);
    pScMod->SetAppOptions(aNewOpts);

    pScMod->RecentFunctionsChanged();       // update function list child window

    delete[] pFuncs;
}

sal_Int32 SAL_CALL ScRecentFunctionsObj::getMaxRecentFunctions() throw(uno::RuntimeException)
{
    return LRU_MAX;
}

//------------------------------------------------------------------------

ScFunctionListObj::ScFunctionListObj()
{
}

ScFunctionListObj::~ScFunctionListObj()
{
}

// stuff for exService_...

uno::Reference<uno::XInterface> SAL_CALL ScFunctionListObj_CreateInstance(
                        const uno::Reference<lang::XMultiServiceFactory>& /* rSMgr */ )
{
    SolarMutexGuard aGuard;
    ScDLL::Init();
    static uno::Reference<uno::XInterface> xInst((cppu::OWeakObject*)new ScFunctionListObj());
    return xInst;
}

rtl::OUString ScFunctionListObj::getImplementationName_Static()
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "stardiv.StarCalc.ScFunctionListObj" ));
}

uno::Sequence<rtl::OUString> ScFunctionListObj::getSupportedServiceNames_Static()
{
    uno::Sequence<rtl::OUString> aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SCFUNCTIONLISTOBJ_SERVICE ));
    return aRet;
}


static void lcl_FillSequence( uno::Sequence<beans::PropertyValue>& rSequence, const ScFuncDesc& rDesc )
{
    rDesc.initArgumentInfo();   // full argument info is needed

    DBG_ASSERT( rSequence.getLength() == SC_FUNCDESC_PROPCOUNT, "Falscher Count" );

    beans::PropertyValue* pArray = rSequence.getArray();

    pArray[0].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_ID ));
    pArray[0].Value <<= (sal_Int32) rDesc.nFIndex;

    pArray[1].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_CATEGORY ));
    pArray[1].Value <<= (sal_Int32) rDesc.nCategory;

    pArray[2].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_NAME ));
    if (rDesc.pFuncName)
        pArray[2].Value <<= rtl::OUString( *rDesc.pFuncName );

    pArray[3].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_DESCRIPTION ));
    if (rDesc.pFuncDesc)
        pArray[3].Value <<= rtl::OUString( *rDesc.pFuncDesc );

    pArray[4].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_ARGUMENTS ));
    if (rDesc.ppDefArgNames && rDesc.ppDefArgDescs && rDesc.pDefArgFlags )
    {
        sal_uInt16 nCount = rDesc.nArgCount;
        if (nCount >= VAR_ARGS)
            nCount -= VAR_ARGS - 1;
        sal_uInt16 nSeqCount = rDesc.GetSuppressedArgCount();
        if (nSeqCount >= VAR_ARGS)
            nSeqCount -= VAR_ARGS - 1;

        if (nSeqCount)
        {
            uno::Sequence<sheet::FunctionArgument> aArgSeq(nSeqCount);
            sheet::FunctionArgument* pArgAry = aArgSeq.getArray();
            for (sal_uInt16 i=0, j=0; i<nCount; i++)
            {
                if (!rDesc.pDefArgFlags[i].bSuppress)
                {
                    String aArgName;
                    if (rDesc.ppDefArgNames[i]) aArgName = *rDesc.ppDefArgNames[i];
                    String aArgDesc;
                    if (rDesc.ppDefArgDescs[i]) aArgDesc = *rDesc.ppDefArgDescs[i];
                    sheet::FunctionArgument aArgument;
                    aArgument.Name        = aArgName;
                    aArgument.Description = aArgDesc;
                    aArgument.IsOptional  = rDesc.pDefArgFlags[i].bOptional;
                    pArgAry[j++] = aArgument;
                }
            }
            pArray[4].Value <<= aArgSeq;
        }
    }
}

// XFunctionDescriptions

uno::Sequence<beans::PropertyValue> SAL_CALL ScFunctionListObj::getById( sal_Int32 nId )
                                throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    const ScFunctionList* pFuncList = ScGlobal::GetStarCalcFunctionList();
    if ( pFuncList )
    {
        sal_uInt16 nCount = (sal_uInt16)pFuncList->GetCount();
        for (sal_uInt16 nIndex=0; nIndex<nCount; nIndex++)
        {
            const ScFuncDesc* pDesc = pFuncList->GetFunction(nIndex);
            if ( pDesc && pDesc->nFIndex == nId )
            {
                uno::Sequence<beans::PropertyValue> aSeq( SC_FUNCDESC_PROPCOUNT );
                lcl_FillSequence( aSeq, *pDesc );
                return aSeq;
            }
        }

        throw lang::IllegalArgumentException();         // not found
    }
    else
        throw uno::RuntimeException();                  // should not happen
}

// XNameAccess

uno::Any SAL_CALL ScFunctionListObj::getByName( const rtl::OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ::rtl::OUString aNameStr(aName);
    const ScFunctionList* pFuncList = ScGlobal::GetStarCalcFunctionList();
    if ( pFuncList )
    {
        sal_uInt16 nCount = (sal_uInt16)pFuncList->GetCount();
        for (sal_uInt16 nIndex=0; nIndex<nCount; nIndex++)
        {
            const ScFuncDesc* pDesc = pFuncList->GetFunction(nIndex);
            //! Case-insensitiv ???
            if ( pDesc && pDesc->pFuncName && aNameStr == *pDesc->pFuncName )
            {
                uno::Sequence<beans::PropertyValue> aSeq( SC_FUNCDESC_PROPCOUNT );
                lcl_FillSequence( aSeq, *pDesc );
                return uno::makeAny(aSeq);
            }
        }

        throw container::NoSuchElementException();      // not found
    }
    else
        throw uno::RuntimeException();                  // should not happen
}

// XIndexAccess

sal_Int32 SAL_CALL ScFunctionListObj::getCount() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    sal_Int32 nCount = 0;
    const ScFunctionList* pFuncList = ScGlobal::GetStarCalcFunctionList();
    if ( pFuncList )
        nCount = static_cast<sal_Int32>(pFuncList->GetCount());
    return nCount;
}

uno::Any SAL_CALL ScFunctionListObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    const ScFunctionList* pFuncList = ScGlobal::GetStarCalcFunctionList();
    if ( pFuncList )
    {
        if ( nIndex >= 0 && nIndex < (sal_Int32)pFuncList->GetCount() )
        {
            const ScFuncDesc* pDesc = pFuncList->GetFunction(nIndex);
            if ( pDesc )
            {
                uno::Sequence<beans::PropertyValue> aSeq( SC_FUNCDESC_PROPCOUNT );
                lcl_FillSequence( aSeq, *pDesc );
                return uno::makeAny(aSeq);
            }
        }

        throw lang::IndexOutOfBoundsException();        // illegal index
    }
    else
        throw uno::RuntimeException();                  // should not happen
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScFunctionListObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.FunctionDescriptionEnumeration")));
}

// XElementAccess

uno::Type SAL_CALL ScFunctionListObj::getElementType() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return getCppuType((uno::Sequence<beans::PropertyValue>*)0);
}

sal_Bool SAL_CALL ScFunctionListObj::hasElements() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return ( getCount() > 0 );
}

uno::Sequence<rtl::OUString> SAL_CALL ScFunctionListObj::getElementNames() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    const ScFunctionList* pFuncList = ScGlobal::GetStarCalcFunctionList();
    if ( pFuncList )
    {
        sal_uInt32 nCount = pFuncList->GetCount();
        uno::Sequence<rtl::OUString> aSeq(nCount);
        rtl::OUString* pAry = aSeq.getArray();
        for (sal_uInt32 nIndex=0; nIndex<nCount; ++nIndex)
        {
            const ScFuncDesc* pDesc = pFuncList->GetFunction(nIndex);
            if ( pDesc && pDesc->pFuncName )
                pAry[nIndex] = *pDesc->pFuncName;
        }
        return aSeq;
    }
    return uno::Sequence<rtl::OUString>(0);
}

sal_Bool SAL_CALL ScFunctionListObj::hasByName( const rtl::OUString& aName )
                                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    const ScFunctionList* pFuncList = ScGlobal::GetStarCalcFunctionList();
    if ( pFuncList )
    {
        sal_uInt32 nCount = pFuncList->GetCount();
        for (sal_uInt32 nIndex=0; nIndex<nCount; ++nIndex)
        {
            const ScFuncDesc* pDesc = pFuncList->GetFunction(nIndex);
            //! Case-insensitiv ???
            if ( pDesc && pDesc->pFuncName && aName == *pDesc->pFuncName )
                return sal_True;
        }
    }
    return false;
}

//------------------------------------------------------------------------




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
