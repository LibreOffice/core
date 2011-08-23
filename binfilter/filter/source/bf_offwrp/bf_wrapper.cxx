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

#ifndef _BF_WRAPPER_HXX
#include <bf_wrapper.hxx>
#endif

#include <bf_offmgr/app.hxx>
#include <bf_sw/swdll.hxx>
#include <bf_sc/scdll.hxx>
#include <bf_sd/sddll.hxx>
#include <bf_sch/schdll.hxx>
#include <bf_starmath/smdll.hxx>
#include <bf_svx/svdetc.hxx>
#include <bf_svx/itemdata.hxx> //STRIP002
#include <framework/imageproducer.hxx>

#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <bf_svtools/moduleoptions.hxx>
#endif

// #i30187#
#include <vos/mutex.hxx>

namespace binfilter {

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

static SwDLL*				pSwDLL = 0L;
static SdDLL*				pSdDLL = 0L;
static ScDLL*				pScDLL = 0L;
static SchDLL*				pSchDLL = 0L;
static SmDLL*				pSmDLL = 0L;
static bf_OfficeWrapper*	pOfficeWrapper = 0L;

SFX_IMPL_XSERVICEINFO( bf_OfficeWrapper, "com.sun.star.office.OfficeWrapper", "com.sun.star.comp.desktop.OfficeWrapper" )

Reference< XInterface >  SAL_CALL bf_OfficeWrapper_CreateInstance( const Reference< XMultiServiceFactory >  & rSMgr )
{
    static osl::Mutex aMutex;

    if ( 0L == pOfficeWrapper)
    {
        osl::MutexGuard guard( aMutex );

        if ( 0L == pOfficeWrapper )
        {
            // #i30187#
            ::vos::OGuard aGuard( Application::GetSolarMutex() );

            return (XComponent*) ( new bf_OfficeWrapper( rSMgr ) );
        }
    }

    return (XComponent*)0;
}

//added by jmeng for i31251 begin
extern "C"{
    void legcy_setBinfilterInitState(void);
}
//added by jmeng for i31251 end
bf_OfficeWrapper::bf_OfficeWrapper( const Reference < XMultiServiceFactory >& xFactory )
:	aListeners( aMutex ),
    pApp( new OfficeApplication )
{
    SvtModuleOptions aMOpt;

    //	if ( aMOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER ) )
    {
        pSwDLL  = new SwDLL;
        SwDLL::LibInit();
    }

    if ( aMOpt.IsModuleInstalled( SvtModuleOptions::E_SDRAW ) || aMOpt.IsModuleInstalled( SvtModuleOptions::E_SIMPRESS ) )
    {
        pSdDLL  = new SdDLL;
        SdDLL::LibInit();
    }

    if ( aMOpt.IsModuleInstalled( SvtModuleOptions::E_SCALC ) )
    {
        pScDLL  = new ScDLL;
        ScDLL::LibInit();
    }

    if ( aMOpt.IsModuleInstalled( SvtModuleOptions::E_SCHART ) )
    {
        pSchDLL = new SchDLL;
        SchDLL::LibInit();
    }

    if ( aMOpt.IsModuleInstalled( SvtModuleOptions::E_SMATH ) )
    {
        pSmDLL = new SmDLL;
        SmDLL::LibInit();
    }
    //added by jmeng for i31251 begin
    legcy_setBinfilterInitState();
    //added by jmeng for i31251 end
}

void SAL_CALL bf_OfficeWrapper::initialize( const Sequence< Any >& aArguments ) throw( Exception )
{
}

bf_OfficeWrapper::~bf_OfficeWrapper()
{
    {
        // all ConfigItems must be destroyed before destroying the SfxApp
        SvtModuleOptions aMOpt;

        if ( aMOpt.IsModuleInstalled( SvtModuleOptions::E_SCHART ) )
        {
            SchDLL::LibExit();
            DELETEZ( pSchDLL );
        }

        if ( aMOpt.IsModuleInstalled( SvtModuleOptions::E_SMATH ) )
        {
            SmDLL::LibExit();
            DELETEZ( pSmDLL );
        }

        //	if ( aMOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER ) )
        {
            SwDLL::LibExit();
            DELETEZ( pSwDLL );
        }

        if ( aMOpt.IsModuleInstalled( SvtModuleOptions::E_SDRAW ) || aMOpt.IsModuleInstalled( SvtModuleOptions::E_SIMPRESS ) )
        {
            SdDLL::LibExit();
            DELETEZ( pSdDLL );
        }

        if ( aMOpt.IsModuleInstalled( SvtModuleOptions::E_SCALC ) )
        {
            ScDLL::PreExit();           // der Teil vom Exit der noch Svx etc. braucht
            ScDLL::LibExit();
            DELETEZ( pScDLL );
        }
    }

    delete pApp;
    delete &GetSdrGlobalData(); // ??????????

    (*(SdrGlobalData**)GetAppData(BF_SHL_SVD))=0;
    (*(SvxGlobalItemData**)GetAppData(BF_SHL_ITEM))=0; 

    SotData_Impl * pSotData = SOTDATA();
    SotFactory * pFact = pSotData->pFactoryList->First();

    while( pFact )
    {
        pFact = pSotData->pFactoryList->Next();
    }
}

void SAL_CALL bf_OfficeWrapper::dispose() throw ( RuntimeException )
{
    EventObject aObject;
    aObject.Source = (XComponent*)this;
    aListeners.disposeAndClear( aObject );
}

void SAL_CALL bf_OfficeWrapper::addEventListener( const Reference< XEventListener > & aListener) throw ( RuntimeException )
{
    aListeners.addInterface( aListener );
}

void SAL_CALL bf_OfficeWrapper::removeEventListener( const Reference< XEventListener > & aListener ) throw ( RuntimeException )
{
    aListeners.removeInterface( aListener );
}

extern "C"
{

void SAL_CALL component_getImplementationEnvironment(	
    const sal_Char** ppEnvironmentTypeName,
    uno_Environment** ppEnvironment)
{
    *ppEnvironmentTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

sal_Bool SAL_CALL component_writeInfo( void* pServiceManager , void* pRegistryKey )
{
    Reference< XMultiServiceFactory >  xMan( reinterpret_cast< XMultiServiceFactory* >( pServiceManager ) ) ;
    //	::utl::setProcessServiceFactory( xMan );
    Reference< XRegistryKey > xKey( reinterpret_cast< XRegistryKey* >( pRegistryKey ) ) ;

    // Eigentliche Implementierung und ihre Services registrieren
    ::rtl::OUString aTempStr;

    ::rtl::OUString aImpl( RTL_CONSTASCII_USTRINGPARAM("/") );
    aImpl += bf_OfficeWrapper::impl_getStaticImplementationName();
    aImpl += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));
    Reference< XRegistryKey > xNewKey = xKey->createKey( aImpl );
    xNewKey->createKey( ::rtl::OUString::createFromAscii("com.sun.star.office.OfficeWrapper") );

    return sal_True;
}

void* SAL_CALL component_getFactory(	
    const sal_Char* pImplementationName,
    void* pServiceManager,
    void* pRegistryKey)
{
    // Set default return value for this operation - if it failed.
    void* pReturn = NULL;

    if ( pImplementationName && pServiceManager )
    {
        // Define variables which are used in following macros.
        Reference< XSingleServiceFactory > xFactory;
        Reference< XMultiServiceFactory > xServiceManager( reinterpret_cast< XMultiServiceFactory* >( pServiceManager ) );

        if ( bf_OfficeWrapper::impl_getStaticImplementationName().compareToAscii( pImplementationName ) == COMPARE_EQUAL )
        {
            xFactory = Reference< XSingleServiceFactory >( 
                cppu::createOneInstanceFactory( xServiceManager, bf_OfficeWrapper::impl_getStaticImplementationName(),
                bf_OfficeWrapper_CreateInstance, bf_OfficeWrapper::impl_getStaticSupportedServiceNames() ) );
        }

        // Factory is valid - service was found.
        if ( xFactory.is() )
        {
            xFactory->acquire();
            pReturn = xFactory.get();
        }
    }

    // Return with result of this operation.
    return pReturn;
}

} // extern "C"

// eof
}
