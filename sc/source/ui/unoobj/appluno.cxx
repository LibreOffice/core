/*************************************************************************
 *
 *  $RCSfile: appluno.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: sab $ $Date: 2001-03-02 17:28:46 $
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
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <tools/shl.hxx>
#include <cppuhelper/factory.hxx>
#include <osl/diagnose.h>

#include "appluno.hxx"
#include "afmtuno.hxx"
#include "funcuno.hxx"
#include "miscuno.hxx"
#include "scmod.hxx"
#include "appoptio.hxx"
#include "inputopt.hxx"
#include "userlist.hxx"
#include "sc.hrc"           // VAR_ARGS
#include "unoguard.hxx"
#include "unonames.hxx"

#include <com/sun/star/sheet/FunctionArgument.hpp>

using namespace com::sun::star;

//------------------------------------------------------------------------

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

// Calc XML export
extern uno::Sequence< rtl::OUString > SAL_CALL ScXMLExport_getSupportedServiceNames() throw();
extern rtl::OUString SAL_CALL ScXMLExport_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLExport_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );
extern uno::Sequence< rtl::OUString > SAL_CALL ScXMLExport_Meta_getSupportedServiceNames() throw();
extern rtl::OUString SAL_CALL ScXMLExport_Meta_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLExport_Meta_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );
extern uno::Sequence< rtl::OUString > SAL_CALL ScXMLExport_Styles_getSupportedServiceNames() throw();
extern rtl::OUString SAL_CALL ScXMLExport_Styles_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLExport_Styles_createInstance(
            const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );
extern uno::Sequence< rtl::OUString > SAL_CALL ScXMLExport_Content_getSupportedServiceNames() throw();
extern rtl::OUString SAL_CALL ScXMLExport_Content_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL ScXMLExport_Content_createInstance(
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

const SfxItemPropertyMap* lcl_GetSettingsPropertyMap()
{
    static SfxItemPropertyMap aSettingsPropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_DOAUTOCP), 0,  &getBooleanCppuType(),              0},
        {MAP_CHAR_LEN(SC_UNONAME_ENTERED),  0,  &getBooleanCppuType(),              0},
        {MAP_CHAR_LEN(SC_UNONAME_EXPREF),   0,  &getBooleanCppuType(),              0},
        {MAP_CHAR_LEN(SC_UNONAME_EXTFMT),   0,  &getBooleanCppuType(),              0},
        {MAP_CHAR_LEN(SC_UNONAME_LINKUPD),  0,  &getCppuType((sal_Int16*)0),        0},
        {MAP_CHAR_LEN(SC_UNONAME_MARKHDR),  0,  &getBooleanCppuType(),              0},
        {MAP_CHAR_LEN(SC_UNONAME_METRIC),   0,  &getCppuType((sal_Int16*)0),        0},
        {MAP_CHAR_LEN(SC_UNONAME_MOVEDIR),  0,  &getCppuType((sal_Int16*)0),        0},
        {MAP_CHAR_LEN(SC_UNONAME_MOVESEL),  0,  &getBooleanCppuType(),              0},
        {MAP_CHAR_LEN(SC_UNONAME_RANGEFIN), 0,  &getBooleanCppuType(),              0},
        {MAP_CHAR_LEN(SC_UNONAME_SCALE),    0,  &getCppuType((sal_Int16*)0),        0},
        {MAP_CHAR_LEN(SC_UNONAME_STBFUNC),  0,  &getCppuType((sal_Int16*)0),        0},
        {MAP_CHAR_LEN(SC_UNONAME_ULISTS),   0,  &getCppuType((uno::Sequence<rtl::OUString>*)0), 0},
        {MAP_CHAR_LEN(SC_UNONAME_USETABCOL),0,  &getBooleanCppuType(),              0},
        {0,0,0,0}
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

void lcl_WriteInfo( registry::XRegistryKey* pRegistryKey,
                        const rtl::OUString& rImplementationName,
                        const uno::Sequence< rtl::OUString >& rServices )
                    throw( registry::InvalidRegistryException )
{
    rtl::OUString aImpl = rtl::OUString::createFromAscii( "/" );
    aImpl += rImplementationName;
    aImpl += rtl::OUString::createFromAscii( "/UNO/SERVICES" );
    uno::Reference<registry::XRegistryKey> xNewKey = pRegistryKey->createKey(aImpl);

    const rtl::OUString* pArray = rServices.getConstArray();
    for( sal_Int32 i = 0; i < rServices.getLength(); i++ )
        xNewKey->createKey( pArray[i]);
}

extern "C" {

void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, registry::XRegistryKey * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            lcl_WriteInfo( pRegistryKey,
                            ScSpreadsheetSettings::getImplementationName_Static(),
                            ScSpreadsheetSettings::getSupportedServiceNames_Static() );

            lcl_WriteInfo( pRegistryKey,
                            ScRecentFunctionsObj::getImplementationName_Static(),
                            ScRecentFunctionsObj::getSupportedServiceNames_Static() );

            lcl_WriteInfo( pRegistryKey,
                            ScFunctionListObj::getImplementationName_Static(),
                            ScFunctionListObj::getSupportedServiceNames_Static() );

            lcl_WriteInfo( pRegistryKey,
                            ScAutoFormatsObj::getImplementationName_Static(),
                            ScAutoFormatsObj::getSupportedServiceNames_Static() );

            lcl_WriteInfo( pRegistryKey,
                            ScFunctionAccess::getImplementationName_Static(),
                            ScFunctionAccess::getSupportedServiceNames_Static() );

            lcl_WriteInfo( pRegistryKey,
                            ScXMLImport_getImplementationName(),
                            ScXMLImport_getSupportedServiceNames() );

            lcl_WriteInfo( pRegistryKey,
                            ScXMLImport_Meta_getImplementationName(),
                            ScXMLImport_Meta_getSupportedServiceNames() );

            lcl_WriteInfo( pRegistryKey,
                            ScXMLImport_Styles_getImplementationName(),
                            ScXMLImport_Styles_getSupportedServiceNames() );

            lcl_WriteInfo( pRegistryKey,
                            ScXMLImport_Content_getImplementationName(),
                            ScXMLImport_Content_getSupportedServiceNames() );

            lcl_WriteInfo( pRegistryKey,
                            ScXMLExport_getImplementationName(),
                            ScXMLExport_getSupportedServiceNames() );

            lcl_WriteInfo( pRegistryKey,
                            ScXMLExport_Meta_getImplementationName(),
                            ScXMLExport_Meta_getSupportedServiceNames() );

            lcl_WriteInfo( pRegistryKey,
                            ScXMLExport_Styles_getImplementationName(),
                            ScXMLExport_Styles_getSupportedServiceNames() );

            lcl_WriteInfo( pRegistryKey,
                            ScXMLExport_Content_getImplementationName(),
                            ScXMLExport_Content_getSupportedServiceNames() );

            return sal_True;
        }
        catch (registry::InvalidRegistryException&)
        {
            OSL_ENSHURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}

void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    if (!pServiceManager)
        return NULL;

    uno::Reference<lang::XSingleServiceFactory> xFactory;
    rtl::OUString aImpl = rtl::OUString::createFromAscii(pImplName);

    if ( aImpl == ScSpreadsheetSettings::getImplementationName_Static() )
        xFactory = cppu::createOneInstanceFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScSpreadsheetSettings::getImplementationName_Static(),
                ScSpreadsheetSettings_CreateInstance,
                ScSpreadsheetSettings::getSupportedServiceNames_Static() );

    if ( aImpl == ScRecentFunctionsObj::getImplementationName_Static() )
        xFactory = cppu::createOneInstanceFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScRecentFunctionsObj::getImplementationName_Static(),
                ScRecentFunctionsObj_CreateInstance,
                ScRecentFunctionsObj::getSupportedServiceNames_Static() );

    if ( aImpl == ScFunctionListObj::getImplementationName_Static() )
        xFactory = cppu::createOneInstanceFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScFunctionListObj::getImplementationName_Static(),
                ScFunctionListObj_CreateInstance,
                ScFunctionListObj::getSupportedServiceNames_Static() );

    if ( aImpl == ScAutoFormatsObj::getImplementationName_Static() )
        xFactory = cppu::createOneInstanceFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScAutoFormatsObj::getImplementationName_Static(),
                ScAutoFormatsObj_CreateInstance,
                ScAutoFormatsObj::getSupportedServiceNames_Static() );

    if ( aImpl == ScFunctionAccess::getImplementationName_Static() )
        xFactory = cppu::createOneInstanceFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScFunctionAccess::getImplementationName_Static(),
                ScFunctionAccess_CreateInstance,
                ScFunctionAccess::getSupportedServiceNames_Static() );

    if ( aImpl == ScXMLImport_getImplementationName() )
        xFactory = cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLImport_getImplementationName(),
                ScXMLImport_createInstance,
                ScXMLImport_getSupportedServiceNames() );

    if ( aImpl == ScXMLImport_Meta_getImplementationName() )
        xFactory = cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLImport_Meta_getImplementationName(),
                ScXMLImport_Meta_createInstance,
                ScXMLImport_Meta_getSupportedServiceNames() );

    if ( aImpl == ScXMLImport_Styles_getImplementationName() )
        xFactory = cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLImport_Styles_getImplementationName(),
                ScXMLImport_Styles_createInstance,
                ScXMLImport_Styles_getSupportedServiceNames() );

    if ( aImpl == ScXMLImport_Content_getImplementationName() )
        xFactory = cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLImport_Content_getImplementationName(),
                ScXMLImport_Content_createInstance,
                ScXMLImport_Content_getSupportedServiceNames() );

    if ( aImpl == ScXMLExport_getImplementationName() )
        xFactory = cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLExport_getImplementationName(),
                ScXMLExport_createInstance,
                ScXMLExport_getSupportedServiceNames() );

    if ( aImpl == ScXMLExport_Meta_getImplementationName() )
        xFactory = cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLExport_Meta_getImplementationName(),
                ScXMLExport_Meta_createInstance,
                ScXMLExport_Meta_getSupportedServiceNames() );

    if ( aImpl == ScXMLExport_Styles_getImplementationName() )
        xFactory = cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLExport_Styles_getImplementationName(),
                ScXMLExport_Styles_createInstance,
                ScXMLExport_Styles_getSupportedServiceNames() );

    if ( aImpl == ScXMLExport_Content_getImplementationName() )
        xFactory = cppu::createSingleFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                ScXMLExport_Content_getImplementationName(),
                ScXMLExport_Content_createInstance,
                ScXMLExport_Content_getSupportedServiceNames() );

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
                        const uno::Reference<lang::XMultiServiceFactory>& rSMgr )
{
    ScUnoGuard aGuard;
    SC_DLL()->Load();       // load module
    static uno::Reference<uno::XInterface> xInst = (cppu::OWeakObject*)new ScSpreadsheetSettings();
    return xInst;
}

rtl::OUString ScSpreadsheetSettings::getImplementationName_Static()
{
    return rtl::OUString::createFromAscii( "stardiv.StarCalc.ScSpreadsheetSettings" );
}

uno::Sequence<rtl::OUString> ScSpreadsheetSettings::getSupportedServiceNames_Static()
{
    uno::Sequence<rtl::OUString> aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = rtl::OUString::createFromAscii( SCSPREADSHEETSETTINGS_SERVICE );
    return aRet;
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScSpreadsheetSettings::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScSpreadsheetSettings::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aString = aPropertyName;

    ScModule* pScMod = SC_MOD();
    ScAppOptions   aAppOpt = pScMod->GetAppOptions();
    ScInputOptions aInpOpt = pScMod->GetInputOptions();
    BOOL bSaveApp = FALSE;
    BOOL bSaveInp = FALSE;

    if (aString.EqualsAscii( SC_UNONAME_DOAUTOCP ))
    {
        aAppOpt.SetAutoComplete( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveApp = TRUE;
    }
    else if (aString.EqualsAscii( SC_UNONAME_ENTERED ))
    {
        aInpOpt.SetEnterEdit( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = TRUE;
    }
    else if (aString.EqualsAscii( SC_UNONAME_EXPREF ))
    {
        aInpOpt.SetExpandRefs( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = TRUE;
    }
    else if (aString.EqualsAscii( SC_UNONAME_EXTFMT ))
    {
        aInpOpt.SetExtendFormat( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = TRUE;
    }
    else if (aString.EqualsAscii( SC_UNONAME_LINKUPD ))
    {
        aAppOpt.SetLinkMode( (ScLkUpdMode) ScUnoHelpFunctions::GetInt16FromAny( aValue ) );
        bSaveApp = TRUE;
    }
    else if (aString.EqualsAscii( SC_UNONAME_MARKHDR ))
    {
        aInpOpt.SetMarkHeader( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = TRUE;
    }
    else if (aString.EqualsAscii( SC_UNONAME_MOVESEL ))
    {
        aInpOpt.SetMoveSelection( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = TRUE;
    }
    else if (aString.EqualsAscii( SC_UNONAME_RANGEFIN ))
    {
        aInpOpt.SetRangeFinder( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = TRUE;
    }
    else if (aString.EqualsAscii( SC_UNONAME_USETABCOL ))
    {
        aInpOpt.SetUseTabCol( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = TRUE;
    }
    else if (aString.EqualsAscii( SC_UNONAME_METRIC ))
    {
        aAppOpt.SetAppMetric( (FieldUnit) ScUnoHelpFunctions::GetInt16FromAny( aValue ) );
        bSaveApp = TRUE;
    }
    else if (aString.EqualsAscii( SC_UNONAME_MOVEDIR ))
    {
        aInpOpt.SetMoveDir( ScUnoHelpFunctions::GetInt16FromAny( aValue ) );
        bSaveInp = TRUE;
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
        bSaveApp = TRUE;
    }
    else if (aString.EqualsAscii( SC_UNONAME_STBFUNC ))
    {
        aAppOpt.SetStatusFunc( ScUnoHelpFunctions::GetInt16FromAny( aValue ) );
        bSaveApp = TRUE;
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
            USHORT nCount = (USHORT)aSeq.getLength();
            const rtl::OUString* pAry = aSeq.getConstArray();
            for (USHORT i=0; i<nCount; i++)
            {
                String aEntry = pAry[i];
                ScUserListData* pData = new ScUserListData(aEntry);
                if (!pUserList->Insert(pData))      // hinten anhaengen
                    delete pData;                   // sollte nicht vorkommen
            }
            bSaveApp = TRUE;    // Liste wird mit den App-Optionen gespeichert
        }
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
    ScUnoGuard aGuard;
    String aString = aPropertyName;
    uno::Any aRet;

    ScModule* pScMod = SC_MOD();
    ScAppOptions   aAppOpt = pScMod->GetAppOptions();
    ScInputOptions aInpOpt = pScMod->GetInputOptions();

    if (aString.EqualsAscii( SC_UNONAME_DOAUTOCP ))     ScUnoHelpFunctions::SetBoolInAny( aRet, aAppOpt.GetAutoComplete() );
    else if (aString.EqualsAscii( SC_UNONAME_ENTERED )) ScUnoHelpFunctions::SetBoolInAny( aRet, aInpOpt.GetEnterEdit() );
    else if (aString.EqualsAscii( SC_UNONAME_EXPREF ))  ScUnoHelpFunctions::SetBoolInAny( aRet, aInpOpt.GetExpandRefs() );
    else if (aString.EqualsAscii( SC_UNONAME_EXTFMT ))  ScUnoHelpFunctions::SetBoolInAny( aRet, aInpOpt.GetExtendFormat() );
    else if (aString.EqualsAscii( SC_UNONAME_LINKUPD )) aRet <<= (sal_Int16) aAppOpt.GetLinkMode();
    else if (aString.EqualsAscii( SC_UNONAME_MARKHDR )) ScUnoHelpFunctions::SetBoolInAny( aRet, aInpOpt.GetMarkHeader() );
    else if (aString.EqualsAscii( SC_UNONAME_MOVESEL )) ScUnoHelpFunctions::SetBoolInAny( aRet, aInpOpt.GetMoveSelection() );
    else if (aString.EqualsAscii( SC_UNONAME_RANGEFIN )) ScUnoHelpFunctions::SetBoolInAny( aRet, aInpOpt.GetRangeFinder() );
    else if (aString.EqualsAscii( SC_UNONAME_USETABCOL )) ScUnoHelpFunctions::SetBoolInAny( aRet, aInpOpt.GetUseTabCol() );
    else if (aString.EqualsAscii( SC_UNONAME_METRIC ))  aRet <<= (sal_Int16) aAppOpt.GetAppMetric();
    else if (aString.EqualsAscii( SC_UNONAME_MOVEDIR )) aRet <<= (sal_Int16) aInpOpt.GetMoveDir();
    else if (aString.EqualsAscii( SC_UNONAME_STBFUNC )) aRet <<= (sal_Int16) aAppOpt.GetStatusFunc();
    else if (aString.EqualsAscii( SC_UNONAME_SCALE ))
    {
        INT16 nZoomVal = 0;
        switch ( aAppOpt.GetZoomType() )
        {
            case SVX_ZOOM_PERCENT:   nZoomVal = aAppOpt.GetZoom();    break;
            case SVX_ZOOM_OPTIMAL:   nZoomVal = SC_ZOOMVAL_OPTIMAL;   break;
            case SVX_ZOOM_WHOLEPAGE: nZoomVal = SC_ZOOMVAL_WHOLEPAGE; break;
            case SVX_ZOOM_PAGEWIDTH: nZoomVal = SC_ZOOMVAL_PAGEWIDTH; break;
        }
        aRet <<= (sal_Int16) nZoomVal;
    }
    else if (aString.EqualsAscii( SC_UNONAME_ULISTS ))
    {
        ScUserList* pUserList = ScGlobal::GetUserList();
        if (pUserList)
        {
            USHORT nCount = pUserList->GetCount();
            uno::Sequence<rtl::OUString> aSeq(nCount);
            rtl::OUString* pAry = aSeq.getArray();
            for (USHORT i=0; i<nCount; i++)
            {
                String aEntry = (*pUserList)[i]->GetString();
                pAry[i] = aEntry;
            }
            aRet <<= aSeq;
        }
    }

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
                        const uno::Reference<lang::XMultiServiceFactory>& rSMgr )
{
    ScUnoGuard aGuard;
    SC_DLL()->Load();       // load module
    static uno::Reference<uno::XInterface> xInst = (cppu::OWeakObject*)new ScRecentFunctionsObj();
    return xInst;
}

rtl::OUString ScRecentFunctionsObj::getImplementationName_Static()
{
    return rtl::OUString::createFromAscii( "stardiv.StarCalc.ScRecentFunctionsObj" );
}

uno::Sequence<rtl::OUString> ScRecentFunctionsObj::getSupportedServiceNames_Static()
{
    uno::Sequence<rtl::OUString> aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = rtl::OUString::createFromAscii( SCRECENTFUNCTIONSOBJ_SERVICE );
    return aRet;
}

// XRecentFunctions

uno::Sequence<sal_Int32> SAL_CALL ScRecentFunctionsObj::getRecentFunctionIds()
                                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    const ScAppOptions& rOpt = SC_MOD()->GetAppOptions();
    USHORT nCount = rOpt.GetLRUFuncListCount();
    const USHORT* pFuncs = rOpt.GetLRUFuncList();
    if (pFuncs)
    {
        uno::Sequence<sal_Int32> aSeq(nCount);
        sal_Int32* pAry = aSeq.getArray();
        for (USHORT i=0; i<nCount; i++)
            pAry[i] = pFuncs[i];
        return aSeq;
    }
    return uno::Sequence<sal_Int32>(0);
}

void SAL_CALL ScRecentFunctionsObj::setRecentFunctionIds(
                    const uno::Sequence<sal_Int32>& aRecentFunctionIds )
                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    USHORT nCount = (USHORT) Min( aRecentFunctionIds.getLength(), (INT32) LRU_MAX );
    const INT32* pAry = aRecentFunctionIds.getConstArray();

    USHORT* pFuncs = nCount ? new USHORT[nCount] : NULL;
    for (USHORT i=0; i<nCount; i++)
        pFuncs[i] = (USHORT)pAry[i];        //! auf gueltige Werte testen?

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
                        const uno::Reference<lang::XMultiServiceFactory>& rSMgr )
{
    ScUnoGuard aGuard;
    SC_DLL()->Load();       // load module
    static uno::Reference<uno::XInterface> xInst = (cppu::OWeakObject*)new ScFunctionListObj();
    return xInst;
}

rtl::OUString ScFunctionListObj::getImplementationName_Static()
{
    return rtl::OUString::createFromAscii( "stardiv.StarCalc.ScFunctionListObj" );
}

uno::Sequence<rtl::OUString> ScFunctionListObj::getSupportedServiceNames_Static()
{
    uno::Sequence<rtl::OUString> aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = rtl::OUString::createFromAscii( SCFUNCTIONLISTOBJ_SERVICE );
    return aRet;
}


void lcl_FillSequence( uno::Sequence<beans::PropertyValue>& rSequence, const ScFuncDesc& rDesc )
{
    DBG_ASSERT( rSequence.getLength() == SC_FUNCDESC_PROPCOUNT, "Falscher Count" );

    beans::PropertyValue* pArray = rSequence.getArray();

    pArray[0].Name = rtl::OUString::createFromAscii( SC_UNONAME_ID );
    pArray[0].Value <<= (sal_Int32) rDesc.nFIndex;

    pArray[1].Name = rtl::OUString::createFromAscii( SC_UNONAME_CATEGORY );
    pArray[1].Value <<= (sal_Int32) rDesc.nCategory;

    pArray[2].Name = rtl::OUString::createFromAscii( SC_UNONAME_NAME );
    if (rDesc.pFuncName)
        pArray[2].Value <<= rtl::OUString( *rDesc.pFuncName );

    pArray[3].Name = rtl::OUString::createFromAscii( SC_UNONAME_DESCRIPTION );
    if (rDesc.pFuncDesc)
        pArray[3].Value <<= rtl::OUString( *rDesc.pFuncDesc );

    pArray[4].Name = rtl::OUString::createFromAscii( SC_UNONAME_ARGUMENTS );
    if (rDesc.aDefArgNames && rDesc.aDefArgDescs && rDesc.aDefArgOpt )
    {
        USHORT nCount = rDesc.nArgCount;
        if (nCount >= VAR_ARGS) nCount = nCount-VAR_ARGS+1;

        uno::Sequence<sheet::FunctionArgument> aArgSeq(nCount);
        sheet::FunctionArgument* pArgAry = aArgSeq.getArray();
        for (USHORT i=0; i<nCount; i++)
        {
            String aArgName;
            if (rDesc.aDefArgNames[i]) aArgName = *rDesc.aDefArgNames[i];
            String aArgDesc;
            if (rDesc.aDefArgDescs[i]) aArgDesc = *rDesc.aDefArgDescs[i];
            sheet::FunctionArgument aArgument;
            aArgument.Name        = aArgName;
            aArgument.Description = aArgDesc;
            aArgument.IsOptional  = rDesc.aDefArgOpt[i];
            pArgAry[i] = aArgument;
        }
        pArray[4].Value <<= aArgSeq;
    }
}

// XFunctionDescriptions

uno::Sequence<beans::PropertyValue> SAL_CALL ScFunctionListObj::getById( sal_Int32 nId )
                                throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    const ScFunctionList* pFuncList = ScGlobal::GetStarCalcFunctionList();
    if ( pFuncList )
    {
        USHORT nCount = (USHORT)pFuncList->GetCount();
        for (USHORT nIndex=0; nIndex<nCount; nIndex++)
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

    return uno::Sequence<beans::PropertyValue>(0);
}

// XNameAccess

uno::Any SAL_CALL ScFunctionListObj::getByName( const rtl::OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameStr = aName;
    const ScFunctionList* pFuncList = ScGlobal::GetStarCalcFunctionList();
    if ( pFuncList )
    {
        USHORT nCount = (USHORT)pFuncList->GetCount();
        for (USHORT nIndex=0; nIndex<nCount; nIndex++)
        {
            const ScFuncDesc* pDesc = pFuncList->GetFunction(nIndex);
            //! Case-insensitiv ???
            if ( pDesc && pDesc->pFuncName && aNameStr == *pDesc->pFuncName )
            {
                uno::Sequence<beans::PropertyValue> aSeq( SC_FUNCDESC_PROPCOUNT );
                lcl_FillSequence( aSeq, *pDesc );
                uno::Any aAny;
                aAny <<= aSeq;
                return aAny;
            }
        }

        throw container::NoSuchElementException();      // not found
    }
    else
        throw uno::RuntimeException();                  // should not happen

    return uno::Any();
}

// XIndexAccess

sal_Int32 SAL_CALL ScFunctionListObj::getCount() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    USHORT nCount = 0;
    const ScFunctionList* pFuncList = ScGlobal::GetStarCalcFunctionList();
    if ( pFuncList )
        nCount = (USHORT)pFuncList->GetCount();
    return nCount;
}

uno::Any SAL_CALL ScFunctionListObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
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
                uno::Any aAny;
                aAny <<= aSeq;
                return aAny;
            }
        }

        throw lang::IndexOutOfBoundsException();        // illegal index
    }
    else
        throw uno::RuntimeException();                  // should not happen

    return uno::Any();
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScFunctionListObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScIndexEnumeration(this);
}

// XElementAccess

uno::Type SAL_CALL ScFunctionListObj::getElementType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return getCppuType((uno::Sequence<beans::PropertyValue>*)0);
}

sal_Bool SAL_CALL ScFunctionListObj::hasElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ( getCount() > 0 );
}

uno::Sequence<rtl::OUString> SAL_CALL ScFunctionListObj::getElementNames() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    const ScFunctionList* pFuncList = ScGlobal::GetStarCalcFunctionList();
    if ( pFuncList )
    {
        USHORT nCount = (USHORT)pFuncList->GetCount();
        uno::Sequence<rtl::OUString> aSeq(nCount);
        rtl::OUString* pAry = aSeq.getArray();
        for (USHORT nIndex=0; nIndex<nCount; nIndex++)
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
    ScUnoGuard aGuard;
    String aNameStr = aName;
    const ScFunctionList* pFuncList = ScGlobal::GetStarCalcFunctionList();
    if ( pFuncList )
    {
        USHORT nCount = (USHORT)pFuncList->GetCount();
        for (USHORT nIndex=0; nIndex<nCount; nIndex++)
        {
            const ScFuncDesc* pDesc = pFuncList->GetFunction(nIndex);
            //! Case-insensitiv ???
            if ( pDesc && pDesc->pFuncName && aNameStr == *pDesc->pFuncName )
                return TRUE;
        }
    }
    return FALSE;
}

//------------------------------------------------------------------------




