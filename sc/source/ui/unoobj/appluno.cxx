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
#include "sc.hrc"
#include "unonames.hxx"
#include "funcdesc.hxx"
#include <com/sun/star/sheet/FunctionArgument.hpp>
#include "ScPanelFactory.hxx"

using namespace com::sun::star;



// Calc document
extern uno::Sequence< OUString > SAL_CALL ScDocument_getSupportedServiceNames() throw();
extern OUString SAL_CALL ScDocument_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScDocument_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr, const sal_uInt64 _nCreationFlags ) throw( uno::Exception );

// Calc XML import
extern uno::Sequence< OUString > SAL_CALL ScXMLImport_getSupportedServiceNames() throw();
extern OUString SAL_CALL ScXMLImport_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLImport_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );
extern uno::Sequence< OUString > SAL_CALL ScXMLImport_Meta_getSupportedServiceNames() throw();
extern OUString SAL_CALL ScXMLImport_Meta_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLImport_Meta_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );
extern uno::Sequence< OUString > SAL_CALL ScXMLImport_Styles_getSupportedServiceNames() throw();
extern OUString SAL_CALL ScXMLImport_Styles_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLImport_Styles_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );
extern uno::Sequence< OUString > SAL_CALL ScXMLImport_Content_getSupportedServiceNames() throw();
extern OUString SAL_CALL ScXMLImport_Content_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLImport_Content_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );
extern uno::Sequence< OUString > SAL_CALL ScXMLImport_Settings_getSupportedServiceNames() throw();
extern OUString SAL_CALL ScXMLImport_Settings_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLImport_Settings_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );

// Calc XML export
extern uno::Sequence< OUString > SAL_CALL ScXMLOOoExport_getSupportedServiceNames() throw();
extern OUString SAL_CALL ScXMLOOoExport_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLOOoExport_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr )
                throw (uno::Exception, std::exception);
extern uno::Sequence< OUString > SAL_CALL ScXMLOOoExport_Meta_getSupportedServiceNames() throw();
extern OUString SAL_CALL ScXMLOOoExport_Meta_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLOOoExport_Meta_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );
extern uno::Sequence< OUString > SAL_CALL ScXMLOOoExport_Styles_getSupportedServiceNames() throw();
extern OUString SAL_CALL ScXMLOOoExport_Styles_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLOOoExport_Styles_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );
extern uno::Sequence< OUString > SAL_CALL ScXMLOOoExport_Content_getSupportedServiceNames() throw();
extern OUString SAL_CALL ScXMLOOoExport_Content_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLOOoExport_Content_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );
extern uno::Sequence< OUString > SAL_CALL ScXMLOOoExport_Settings_getSupportedServiceNames() throw();
extern OUString SAL_CALL ScXMLOOoExport_Settings_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLOOoExport_Settings_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr )
                throw( uno::Exception, std::exception );

// Calc XML Oasis export
extern uno::Sequence< OUString > SAL_CALL ScXMLOasisExport_getSupportedServiceNames() throw();
extern OUString SAL_CALL ScXMLOasisExport_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLOasisExport_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr )
                throw (uno::Exception, std::exception);
extern uno::Sequence< OUString > SAL_CALL ScXMLOasisExport_Meta_getSupportedServiceNames() throw();
extern OUString SAL_CALL ScXMLOasisExport_Meta_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLOasisExport_Meta_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );
extern uno::Sequence< OUString > SAL_CALL ScXMLOasisExport_Styles_getSupportedServiceNames() throw();
extern OUString SAL_CALL ScXMLOasisExport_Styles_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLOasisExport_Styles_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > & rSMgr )
        throw( uno::Exception, std::exception );
extern uno::Sequence< OUString > SAL_CALL ScXMLOasisExport_Content_getSupportedServiceNames() throw();
extern OUString SAL_CALL ScXMLOasisExport_Content_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLOasisExport_Content_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );
extern uno::Sequence< OUString > SAL_CALL ScXMLOasisExport_Settings_getSupportedServiceNames() throw();
extern OUString SAL_CALL ScXMLOasisExport_Settings_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLOasisExport_Settings_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );



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



//  alles ohne Which-ID, Map nur fuer PropertySetInfo

static const SfxItemPropertyMapEntry* lcl_GetSettingsPropertyMap()
{
    static const SfxItemPropertyMapEntry aSettingsPropertyMap_Impl[] =
    {
        {OUString(SC_UNONAME_DOAUTOCP), 0,  getBooleanCppuType(),              0, 0},
        {OUString(SC_UNONAME_ENTERED),  0,  getBooleanCppuType(),              0, 0},
        {OUString(SC_UNONAME_EXPREF),   0,  getBooleanCppuType(),              0, 0},
        {OUString(SC_UNONAME_EXTFMT),   0,  getBooleanCppuType(),              0, 0},
        {OUString(SC_UNONAME_LINKUPD),  0,  getCppuType((sal_Int16*)0),        0, 0},
        {OUString(SC_UNONAME_MARKHDR),  0,  getBooleanCppuType(),              0, 0},
        {OUString(SC_UNONAME_METRIC),   0,  getCppuType((sal_Int16*)0),        0, 0},
        {OUString(SC_UNONAME_MOVEDIR),  0,  getCppuType((sal_Int16*)0),        0, 0},
        {OUString(SC_UNONAME_MOVESEL),  0,  getBooleanCppuType(),              0, 0},
        {OUString(SC_UNONAME_PRALLSH),  0,  getBooleanCppuType(),              0, 0},
        {OUString(SC_UNONAME_PREMPTY),  0,  getBooleanCppuType(),              0, 0},
        {OUString(SC_UNONAME_RANGEFIN), 0,  getBooleanCppuType(),              0, 0},
        {OUString(SC_UNONAME_SCALE),    0,  getCppuType((sal_Int16*)0),        0, 0},
        {OUString(SC_UNONAME_STBFUNC),  0,  getCppuType((sal_Int16*)0),        0, 0},
        {OUString(SC_UNONAME_ULISTS),   0,  getCppuType((uno::Sequence<OUString>*)0), 0, 0},
        {OUString(SC_UNONAME_PRMETRICS),0,  getBooleanCppuType(),              0, 0},
        {OUString(SC_UNONAME_USETABCOL),0,  getBooleanCppuType(),              0, 0},
        {OUString(SC_UNONAME_REPLWARN), 0,  getBooleanCppuType(),              0, 0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aSettingsPropertyMap_Impl;
}



#define SCFUNCTIONLISTOBJ_SERVICE       "com.sun.star.sheet.FunctionDescriptions"
#define SCRECENTFUNCTIONSOBJ_SERVICE    "com.sun.star.sheet.RecentFunctions"
#define SCSPREADSHEETSETTINGS_SERVICE   "com.sun.star.sheet.GlobalSheetSettings"

SC_SIMPLE_SERVICE_INFO( ScFunctionListObj, "ScFunctionListObj", SCFUNCTIONLISTOBJ_SERVICE )
SC_SIMPLE_SERVICE_INFO( ScRecentFunctionsObj, "ScRecentFunctionsObj", SCRECENTFUNCTIONSOBJ_SERVICE )
SC_SIMPLE_SERVICE_INFO( ScSpreadsheetSettings, "ScSpreadsheetSettings", SCSPREADSHEETSETTINGS_SERVICE )



extern "C" {

SAL_DLLPUBLIC_EXPORT void * SAL_CALL sc_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * /* pRegistryKey */ )
{
    if (!pServiceManager)
        return NULL;

    uno::Reference<lang::XSingleServiceFactory> xFactory;
    OUString aImpl(OUString::createFromAscii(pImplName));

    if ( aImpl == ScSpreadsheetSettings::getImplementationName_Static() )
    {
        xFactory.set(cppu::createOneInstanceFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScSpreadsheetSettings::getImplementationName_Static(),
                ScSpreadsheetSettings_CreateInstance,
                ScSpreadsheetSettings::getSupportedServiceNames_Static() ));
    }
    else if ( aImpl == ScRecentFunctionsObj::getImplementationName_Static() )
    {
        xFactory.set(cppu::createOneInstanceFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScRecentFunctionsObj::getImplementationName_Static(),
                ScRecentFunctionsObj_CreateInstance,
                ScRecentFunctionsObj::getSupportedServiceNames_Static() ));
    }
    else if ( aImpl == ScFunctionListObj::getImplementationName_Static() )
    {
        xFactory.set(cppu::createOneInstanceFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScFunctionListObj::getImplementationName_Static(),
                ScFunctionListObj_CreateInstance,
                ScFunctionListObj::getSupportedServiceNames_Static() ));
    }
    else if ( aImpl == ScAutoFormatsObj::getImplementationName_Static() )
    {
        xFactory.set(cppu::createOneInstanceFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScAutoFormatsObj::getImplementationName_Static(),
                ScAutoFormatsObj_CreateInstance,
                ScAutoFormatsObj::getSupportedServiceNames_Static() ));
    }
    else if ( aImpl == ScFunctionAccess::getImplementationName_Static() )
    {
        xFactory.set(cppu::createOneInstanceFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScFunctionAccess::getImplementationName_Static(),
                ScFunctionAccess_CreateInstance,
                ScFunctionAccess::getSupportedServiceNames_Static() ));
    }
    else if ( aImpl == ScFilterOptionsObj::getImplementationName_Static() )
    {
        xFactory.set(cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScFilterOptionsObj::getImplementationName_Static(),
                ScFilterOptionsObj_CreateInstance,
                ScFilterOptionsObj::getSupportedServiceNames_Static() ));
    }
    else if ( aImpl == ScXMLImport_getImplementationName() )
    {
        xFactory.set(cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLImport_getImplementationName(),
                ScXMLImport_createInstance,
                ScXMLImport_getSupportedServiceNames() ));
    }
    else if ( aImpl == ScXMLImport_Meta_getImplementationName() )
    {
        xFactory.set(cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLImport_Meta_getImplementationName(),
                ScXMLImport_Meta_createInstance,
                ScXMLImport_Meta_getSupportedServiceNames() ));
    }
    else if ( aImpl == ScXMLImport_Styles_getImplementationName() )
    {
        xFactory.set(cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLImport_Styles_getImplementationName(),
                ScXMLImport_Styles_createInstance,
                ScXMLImport_Styles_getSupportedServiceNames() ));
    }
    else if ( aImpl == ScXMLImport_Content_getImplementationName() )
    {
        xFactory.set(cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLImport_Content_getImplementationName(),
                ScXMLImport_Content_createInstance,
                ScXMLImport_Content_getSupportedServiceNames() ));
    }
    else if ( aImpl == ScXMLImport_Settings_getImplementationName() )
    {
        xFactory.set(cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLImport_Settings_getImplementationName(),
                ScXMLImport_Settings_createInstance,
                ScXMLImport_Settings_getSupportedServiceNames() ));
    }
    else if ( aImpl == ScXMLOOoExport_getImplementationName() )
    {
        xFactory = cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLOOoExport_getImplementationName(),
                ScXMLOOoExport_createInstance,
                ScXMLOOoExport_getSupportedServiceNames() );
    }
    else if ( aImpl == ScXMLOOoExport_Meta_getImplementationName() )
    {
        xFactory = cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLOOoExport_Meta_getImplementationName(),
                ScXMLOOoExport_Meta_createInstance,
                ScXMLOOoExport_Meta_getSupportedServiceNames() );
    }
    else if ( aImpl == ScXMLOOoExport_Styles_getImplementationName() )
    {
        xFactory = cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLOOoExport_Styles_getImplementationName(),
                ScXMLOOoExport_Styles_createInstance,
                ScXMLOOoExport_Styles_getSupportedServiceNames() );
    }
    else if ( aImpl == ScXMLOOoExport_Content_getImplementationName() )
    {
        xFactory = cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLOOoExport_Content_getImplementationName(),
                ScXMLOOoExport_Content_createInstance,
                ScXMLOOoExport_Content_getSupportedServiceNames() );
    }
    else if ( aImpl == ScXMLOOoExport_Settings_getImplementationName() )
    {
        xFactory = cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLOOoExport_Settings_getImplementationName(),
                ScXMLOOoExport_Settings_createInstance,
                ScXMLOOoExport_Settings_getSupportedServiceNames() );
    }
    else if ( aImpl == ScXMLOasisExport_getImplementationName() )
    {
        xFactory = cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLOasisExport_getImplementationName(),
                ScXMLOasisExport_createInstance,
                ScXMLOasisExport_getSupportedServiceNames() );
    }
    else if ( aImpl == ScXMLOasisExport_Meta_getImplementationName() )
    {
        xFactory = cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLOasisExport_Meta_getImplementationName(),
                ScXMLOasisExport_Meta_createInstance,
                ScXMLOasisExport_Meta_getSupportedServiceNames() );
    }
    else if ( aImpl == ScXMLOasisExport_Styles_getImplementationName() )
    {
        xFactory = cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLOasisExport_Styles_getImplementationName(),
                ScXMLOasisExport_Styles_createInstance,
                ScXMLOasisExport_Styles_getSupportedServiceNames() );
    }
    else if ( aImpl == ScXMLOasisExport_Content_getImplementationName() )
    {
        xFactory = cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLOasisExport_Content_getImplementationName(),
                ScXMLOasisExport_Content_createInstance,
                ScXMLOasisExport_Content_getSupportedServiceNames() );
    }
    else if ( aImpl == ScXMLOasisExport_Settings_getImplementationName() )
    {
        xFactory = cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLOasisExport_Settings_getImplementationName(),
                ScXMLOasisExport_Settings_createInstance,
                ScXMLOasisExport_Settings_getSupportedServiceNames() );
    }
    else if ( aImpl == ScDocument_getImplementationName() )
    {
        xFactory.set(sfx2::createSfxModelFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScDocument_getImplementationName(),
                ScDocument_createInstance,
                ScDocument_getSupportedServiceNames() ));
    }
    else if ( aImpl == ::sc::sidebar::ScPanelFactory::getImplementationName() )
    {
        xFactory = ::cppu::createSingleFactory(
            reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
            ::sc::sidebar::ScPanelFactory::getImplementationName(),
            ::sc::sidebar::ScPanelFactory::createInstance,
            ::sc::sidebar::ScPanelFactory::getSupportedServiceNames() );
    }

    void* pRet = NULL;
    if (xFactory.is())
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}

}   // extern C



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

OUString ScSpreadsheetSettings::getImplementationName_Static()
{
    return OUString( "stardiv.StarCalc.ScSpreadsheetSettings" );
}

uno::Sequence<OUString> ScSpreadsheetSettings::getSupportedServiceNames_Static()
{
    uno::Sequence<OUString> aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( SCSPREADSHEETSETTINGS_SERVICE );
    return aRet;
}

sal_Bool ScSpreadsheetSettings::getPropertyBool(const OUString& aPropertyName) throw (css::uno::RuntimeException)
{
   uno::Any any = getPropertyValue(aPropertyName);
   sal_Bool b = sal_False;
   any >>= b;
   return b;
}

sal_Int16 ScSpreadsheetSettings::getPropertyInt16(const OUString& aPropertyName) throw (css::uno::RuntimeException)
{
   uno::Any any = getPropertyValue(aPropertyName);
   sal_Int16 b = 0;
   any >>= b;
   return b;
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
                        const OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    OUString aString(aPropertyName);

    ScModule* pScMod = SC_MOD();
    ScAppOptions   aAppOpt(pScMod->GetAppOptions());
    ScInputOptions aInpOpt(pScMod->GetInputOptions());
    sal_Bool bSaveApp = false;
    sal_Bool bSaveInp = false;
    // print options aren't loaded until needed

    if (aString.equalsAscii( SC_UNONAME_DOAUTOCP ))
    {
        aAppOpt.SetAutoComplete( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveApp = sal_True;
    }
    else if (aString.equalsAscii( SC_UNONAME_ENTERED ))
    {
        aInpOpt.SetEnterEdit( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = sal_True;
    }
    else if (aString.equalsAscii( SC_UNONAME_EXPREF ))
    {
        aInpOpt.SetExpandRefs( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = sal_True;
    }
    else if (aString.equalsAscii( SC_UNONAME_EXTFMT ))
    {
        aInpOpt.SetExtendFormat( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = sal_True;
    }
    else if (aString.equalsAscii( SC_UNONAME_LINKUPD ))
    {
        aAppOpt.SetLinkMode( (ScLkUpdMode) ScUnoHelpFunctions::GetInt16FromAny( aValue ) );
        bSaveApp = sal_True;
    }
    else if (aString.equalsAscii( SC_UNONAME_MARKHDR ))
    {
        aInpOpt.SetMarkHeader( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = sal_True;
    }
    else if (aString.equalsAscii( SC_UNONAME_MOVESEL ))
    {
        aInpOpt.SetMoveSelection( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = sal_True;
    }
    else if (aString.equalsAscii( SC_UNONAME_RANGEFIN ))
    {
        aInpOpt.SetRangeFinder( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = sal_True;
    }
    else if (aString.equalsAscii( SC_UNONAME_USETABCOL ))
    {
        aInpOpt.SetUseTabCol( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = sal_True;
    }
    else if (aString.equalsAscii( SC_UNONAME_PRMETRICS ))
    {
        aInpOpt.SetTextWysiwyg( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = sal_True;
    }
    else if (aString.equalsAscii( SC_UNONAME_REPLWARN ))
    {
        aInpOpt.SetReplaceCellsWarn( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = sal_True;
    }
    else if (aString.equalsAscii( SC_UNONAME_METRIC ))
    {
        aAppOpt.SetAppMetric( (FieldUnit) ScUnoHelpFunctions::GetInt16FromAny( aValue ) );
        bSaveApp = sal_True;
    }
    else if (aString.equalsAscii( SC_UNONAME_MOVEDIR ))
    {
        aInpOpt.SetMoveDir( ScUnoHelpFunctions::GetInt16FromAny( aValue ) );
        bSaveInp = sal_True;
    }
    else if (aString.equalsAscii( SC_UNONAME_SCALE ))
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
    else if (aString.equalsAscii( SC_UNONAME_STBFUNC ))
    {
        aAppOpt.SetStatusFunc( ScUnoHelpFunctions::GetInt16FromAny( aValue ) );
        bSaveApp = sal_True;
    }
    else if (aString.equalsAscii( SC_UNONAME_ULISTS ))
    {
        ScUserList* pUserList = ScGlobal::GetUserList();
        uno::Sequence<OUString> aSeq;
        if ( pUserList && ( aValue >>= aSeq ) )
        {
            //  es wird direkt die "lebende" Liste veraendert,
            //  mehr tut ScGlobal::SetUserList auch nicht

            pUserList->clear();                 // alle Eintraege raus
            sal_uInt16 nCount = (sal_uInt16)aSeq.getLength();
            const OUString* pAry = aSeq.getConstArray();
            for (sal_uInt16 i=0; i<nCount; i++)
            {
                OUString aEntry = pAry[i];
                ScUserListData* pData = new ScUserListData(aEntry);
                pUserList->push_back(pData);
            }
            bSaveApp = sal_True;    // Liste wird mit den App-Optionen gespeichert
        }
    }
    else if (aString.equalsAscii( SC_UNONAME_PRALLSH ))
    {
        ScPrintOptions aPrintOpt(pScMod->GetPrintOptions());
        aPrintOpt.SetAllSheets( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        pScMod->SetPrintOptions( aPrintOpt );
    }
    else if (aString.equalsAscii( SC_UNONAME_PREMPTY ))
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

uno::Any SAL_CALL ScSpreadsheetSettings::getPropertyValue( const OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    OUString aString = aPropertyName;
    uno::Any aRet;

    ScModule* pScMod = SC_MOD();
    ScAppOptions   aAppOpt = pScMod->GetAppOptions();
    ScInputOptions aInpOpt = pScMod->GetInputOptions();
    // print options aren't loaded until needed

    if (aString.equalsAscii( SC_UNONAME_DOAUTOCP ))     ScUnoHelpFunctions::SetBoolInAny( aRet, aAppOpt.GetAutoComplete() );
    else if (aString.equalsAscii( SC_UNONAME_ENTERED )) ScUnoHelpFunctions::SetBoolInAny( aRet, aInpOpt.GetEnterEdit() );
    else if (aString.equalsAscii( SC_UNONAME_EXPREF ))  ScUnoHelpFunctions::SetBoolInAny( aRet, aInpOpt.GetExpandRefs() );
    else if (aString.equalsAscii( SC_UNONAME_EXTFMT ))  ScUnoHelpFunctions::SetBoolInAny( aRet, aInpOpt.GetExtendFormat() );
    else if (aString.equalsAscii( SC_UNONAME_LINKUPD )) aRet <<= (sal_Int16) aAppOpt.GetLinkMode();
    else if (aString.equalsAscii( SC_UNONAME_MARKHDR )) ScUnoHelpFunctions::SetBoolInAny( aRet, aInpOpt.GetMarkHeader() );
    else if (aString.equalsAscii( SC_UNONAME_MOVESEL )) ScUnoHelpFunctions::SetBoolInAny( aRet, aInpOpt.GetMoveSelection() );
    else if (aString.equalsAscii( SC_UNONAME_RANGEFIN )) ScUnoHelpFunctions::SetBoolInAny( aRet, aInpOpt.GetRangeFinder() );
    else if (aString.equalsAscii( SC_UNONAME_USETABCOL )) ScUnoHelpFunctions::SetBoolInAny( aRet, aInpOpt.GetUseTabCol() );
    else if (aString.equalsAscii( SC_UNONAME_PRMETRICS )) ScUnoHelpFunctions::SetBoolInAny( aRet, aInpOpt.GetTextWysiwyg() );
    else if (aString.equalsAscii( SC_UNONAME_REPLWARN )) ScUnoHelpFunctions::SetBoolInAny( aRet, aInpOpt.GetReplaceCellsWarn() );
    else if (aString.equalsAscii( SC_UNONAME_METRIC ))  aRet <<= (sal_Int16) aAppOpt.GetAppMetric();
    else if (aString.equalsAscii( SC_UNONAME_MOVEDIR )) aRet <<= (sal_Int16) aInpOpt.GetMoveDir();
    else if (aString.equalsAscii( SC_UNONAME_STBFUNC )) aRet <<= (sal_Int16) aAppOpt.GetStatusFunc();
    else if (aString.equalsAscii( SC_UNONAME_SCALE ))
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
    else if (aString.equalsAscii( SC_UNONAME_ULISTS ))
    {
        ScUserList* pUserList = ScGlobal::GetUserList();
        if (pUserList)
        {
            size_t nCount = pUserList->size();
            uno::Sequence<OUString> aSeq(nCount);
            OUString* pAry = aSeq.getArray();
            for (size_t i=0; i<nCount; ++i)
            {
                OUString aEntry((*pUserList)[i]->GetString());
                pAry[i] = aEntry;
            }
            aRet <<= aSeq;
        }
    }
    else if (aString.equalsAscii( SC_UNONAME_PRALLSH ))
        ScUnoHelpFunctions::SetBoolInAny( aRet, pScMod->GetPrintOptions().GetAllSheets() );
    else if (aString.equalsAscii( SC_UNONAME_PREMPTY ))
        ScUnoHelpFunctions::SetBoolInAny( aRet, !pScMod->GetPrintOptions().GetSkipEmpty() );    // reversed

    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScSpreadsheetSettings )



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

OUString ScRecentFunctionsObj::getImplementationName_Static()
{
    return OUString( "stardiv.StarCalc.ScRecentFunctionsObj" );
}

uno::Sequence<OUString> ScRecentFunctionsObj::getSupportedServiceNames_Static()
{
    uno::Sequence<OUString> aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( SCRECENTFUNCTIONSOBJ_SERVICE );
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
    sal_uInt16 nCount = (sal_uInt16) std::min( aRecentFunctionIds.getLength(), (sal_Int32) LRU_MAX );
    const sal_Int32* pAry = aRecentFunctionIds.getConstArray();

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

OUString ScFunctionListObj::getImplementationName_Static()
{
    return OUString( "stardiv.StarCalc.ScFunctionListObj" );
}

uno::Sequence<OUString> ScFunctionListObj::getSupportedServiceNames_Static()
{
    uno::Sequence<OUString> aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( SCFUNCTIONLISTOBJ_SERVICE );
    return aRet;
}


static void lcl_FillSequence( uno::Sequence<beans::PropertyValue>& rSequence, const ScFuncDesc& rDesc )
{
    rDesc.initArgumentInfo();   // full argument info is needed

    OSL_ENSURE( rSequence.getLength() == SC_FUNCDESC_PROPCOUNT, "Falscher Count" );

    beans::PropertyValue* pArray = rSequence.getArray();

    pArray[0].Name = OUString( SC_UNONAME_ID );
    pArray[0].Value <<= (sal_Int32) rDesc.nFIndex;

    pArray[1].Name = OUString( SC_UNONAME_CATEGORY );
    pArray[1].Value <<= (sal_Int32) rDesc.nCategory;

    pArray[2].Name = OUString( SC_UNONAME_NAME );
    if (rDesc.pFuncName)
        pArray[2].Value <<= OUString( *rDesc.pFuncName );

    pArray[3].Name = OUString( SC_UNONAME_DESCRIPTION );
    if (rDesc.pFuncDesc)
        pArray[3].Value <<= OUString( *rDesc.pFuncDesc );

    pArray[4].Name = OUString( SC_UNONAME_ARGUMENTS );
    if (!rDesc.maDefArgNames.empty() && !rDesc.maDefArgDescs.empty() && rDesc.pDefArgFlags )
    {
        sal_uInt16 nCount = rDesc.nArgCount;
		if (nCount >= PAIRED_VAR_ARGS)
		    nCount -= PAIRED_VAR_ARGS - 2;
        else if (nCount >= VAR_ARGS)
            nCount -= VAR_ARGS - 1;
        sal_uInt16 nSeqCount = rDesc.GetSuppressedArgCount();
        if (nSeqCount >= PAIRED_VAR_ARGS)
            nSeqCount -= PAIRED_VAR_ARGS - 2;
        else if (nSeqCount >= VAR_ARGS)
            nSeqCount -= VAR_ARGS - 1;

        if (nSeqCount)
        {
            uno::Sequence<sheet::FunctionArgument> aArgSeq(nSeqCount);
            sheet::FunctionArgument* pArgAry = aArgSeq.getArray();
            for (sal_uInt16 i=0, j=0; i<nCount; i++)
            {
                if (!rDesc.pDefArgFlags[i].bSuppress)
                {
                    sheet::FunctionArgument aArgument;
                    aArgument.Name        = rDesc.maDefArgNames[i];
                    aArgument.Description = rDesc.maDefArgDescs[i];
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

uno::Any SAL_CALL ScFunctionListObj::getByName( const OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    OUString aNameStr(aName);
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
    return new ScIndexEnumeration(this, OUString("com.sun.star.sheet.FunctionDescriptionEnumeration"));
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

uno::Sequence<OUString> SAL_CALL ScFunctionListObj::getElementNames() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    const ScFunctionList* pFuncList = ScGlobal::GetStarCalcFunctionList();
    if ( pFuncList )
    {
        sal_uInt32 nCount = pFuncList->GetCount();
        uno::Sequence<OUString> aSeq(nCount);
        OUString* pAry = aSeq.getArray();
        for (sal_uInt32 nIndex=0; nIndex<nCount; ++nIndex)
        {
            const ScFuncDesc* pDesc = pFuncList->GetFunction(nIndex);
            if ( pDesc && pDesc->pFuncName )
                pAry[nIndex] = *pDesc->pFuncName;
        }
        return aSeq;
    }
    return uno::Sequence<OUString>(0);
}

sal_Bool SAL_CALL ScFunctionListObj::hasByName( const OUString& aName )
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






/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
