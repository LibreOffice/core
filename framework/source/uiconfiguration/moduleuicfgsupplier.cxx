/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: moduleuicfgsupplier.cxx,v $
 * $Revision: 1.11 $
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
#include "precompiled_framework.hxx"
#include <uiconfiguration/moduleuicfgsupplier.hxx>
#include <threadhelp/resetableguard.hxx>
#include <services.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/embed/XPackageStructureCreator.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <rtl/logfile.hxx>

#include <vcl/svapp.hxx>

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::embed;
using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::frame;

namespace framework
{

class RootStorageWrapper :  public com::sun::star::lang::XTypeProvider      ,
                            public com::sun::star::embed::XTransactedObject ,
                            public ::cppu::OWeakObject
{
    public:
        //  XInterface, XTypeProvider
        FWK_DECLARE_XINTERFACE
      FWK_DECLARE_XTYPEPROVIDER

        RootStorageWrapper( const Reference< XTransactedObject >& xRootCommit ) : m_xRootCommit( xRootCommit ) {}
        virtual ~RootStorageWrapper() {}

        // XTransactedObject
        virtual void SAL_CALL commit() throw ( com::sun::star::io::IOException, com::sun::star::lang::WrappedTargetException )
        {
            m_xRootCommit->commit();
        }

        virtual void SAL_CALL revert() throw ( com::sun::star::io::IOException, com::sun::star::lang::WrappedTargetException )
        {
            m_xRootCommit->revert();
        }

    private:
        Reference< XTransactedObject > m_xRootCommit;
};

DEFINE_XINTERFACE_2                    (    RootStorageWrapper                                          ,
                                            OWeakObject                                                 ,
                                            DIRECT_INTERFACE( css::lang::XTypeProvider                 ),
                                            DIRECT_INTERFACE( com::sun::star::embed::XTransactedObject )
                                        )

DEFINE_XTYPEPROVIDER_2                  (   RootStorageWrapper              ,
                                            css::lang::XTypeProvider        ,
                                            css::embed::XTransactedObject
                                        )

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XINTERFACE_4                    (    ModuleUIConfigurationManagerSupplier                    ,
                                            OWeakObject                                             ,
                                            DIRECT_INTERFACE( css::lang::XTypeProvider                                          ),
                                            DIRECT_INTERFACE( css::lang::XServiceInfo                                           ),
                                            DIRECT_INTERFACE( css::lang::XComponent                                             ),
                                            DIRECT_INTERFACE( ::com::sun::star::ui::XModuleUIConfigurationManagerSupplier )
                                        )

DEFINE_XTYPEPROVIDER_4                  (   ModuleUIConfigurationManagerSupplier                                ,
                                            css::lang::XTypeProvider                                            ,
                                            css::lang::XServiceInfo                                             ,
                                            css::lang::XComponent                                               ,
                                            ::com::sun::star::ui::XModuleUIConfigurationManagerSupplier
                                        )

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   ModuleUIConfigurationManagerSupplier                    ,
                                            ::cppu::OWeakObject                                     ,
                                            SERVICENAME_MODULEUICONFIGURATIONMANAGERSUPPLIER        ,
                                            IMPLEMENTATIONNAME_MODULEUICONFIGURATIONMANAGERSUPPLIER
                                        )

DEFINE_INIT_SERVICE                     (   ModuleUIConfigurationManagerSupplier, {} )


/*TODO_AS
void ModuleUIConfigurationManagerSupplier::impl_initStorages()
{
    if ( !m_bInit )
    {
        RTL_LOGFILE_CONTEXT( aLog, "framework (cd100003) ::ModuleUIConfigurationManagerSupplier::impl_initStorages" );

        rtl::OUString aFinalSlash( RTL_CONSTASCII_USTRINGPARAM( "/" ));
        rtl::OUString aConfigRootFolder( RTL_CONSTASCII_USTRINGPARAM( "soffice.cfg/modules" ));
        rtl::OUString aConfigSubFolder( RTL_CONSTASCII_USTRINGPARAM( "soffice.cfg/modules/soffice.cfg" ));
        rtl::OUString aConfigRootFolder( RTL_CONSTASCII_USTRINGPARAM( "soffice.cfg" ));
        rtl::OUString aConfigSubFolder( RTL_CONSTASCII_USTRINGPARAM( "soffice.cfg/soffice.cfg" ));
        rtl::OUString aConfigFileName( RTL_CONSTASCII_USTRINGPARAM( "soffice.cfg/uiconfig.zip" ));

        Reference< XPropertySet > xPathSettings( m_xServiceManager->createInstance(
                                                        OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.PathSettings" ))),
                                                  UNO_QUERY_THROW );

        Any a = xPathSettings->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "UIConfig" )));
        a >>= m_aDefaultConfigURL;
        a = xPathSettings->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "UserConfig" )));
        a >>= m_aUserConfigURL;

        // Use only the first entry from "UIConfig"
        sal_Int32 nIndex = m_aDefaultConfigURL.indexOf( ';' );
        if ( nIndex > 0 )
            m_aDefaultConfigURL = m_aDefaultConfigURL.copy( 0, nIndex );

        rtl::OUString aDefaultConfigFolderURL( m_aDefaultConfigURL );

        nIndex = m_aDefaultConfigURL.lastIndexOf( '/' );
        if (( nIndex > 0 ) &&  ( nIndex != ( m_aDefaultConfigURL.getLength()-1 )))
        {
            m_aDefaultConfigURL += aFinalSlash;
            aDefaultConfigFolderURL += aFinalSlash;
        }

        nIndex = m_aUserConfigURL.lastIndexOf( '/' );
        if (( nIndex > 0 ) &&  ( nIndex != ( m_aUserConfigURL.getLength()-1 )))
            m_aUserConfigURL += aFinalSlash;

//        aDefaultConfigFolderURL += aConfigRootFolder;

        // Create root storages for user interface configuration data (default and customizable)
        Reference< XSingleServiceFactory > xStorageFactory( m_xServiceManager->createInstance(
                                                                OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.embed.StorageFactory" ))),
                                                            UNO_QUERY_THROW );

        Sequence< Any > aArgs( 2 );

        // Default root storage (READ-ACCESS)
        aArgs[0] <<= m_aDefaultConfigURL + aConfigFileName; //aConfigSubFolder;
        aArgs[1] <<= ElementModes::READ;
        m_xDefaultCfgRootStorage = Reference< XStorage >( xStorageFactory->createInstanceWithArguments( aArgs ), UNO_QUERY_THROW );

        Reference < XOutputStream > xTempOut( m_xServiceManager->createInstance (
                                                ::rtl::OUString::createFromAscii( "com.sun.star.io.TempFile" ) ),
                                              UNO_QUERY );

        Reference< XPackageStructureCreator > xPackageStructCreator( m_xServiceManager->createInstance(
                                                                        OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.embed.PackageStructureCreator" ))),
                                                                     UNO_QUERY_THROW );

        RTL_LOGFILE_CONTEXT_TRACE( aLog, "{ convertToPackage" );
        xPackageStructCreator->convertToPackage( aDefaultConfigFolderURL, xTempOut );
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "} convertToPackage" );

        xTempOut->closeOutput();
        Reference< XInputStream > xTempIn( xTempOut, UNO_QUERY );
        Reference< XSeekable > xTempSeek( xTempOut, UNO_QUERY );

        // Default root storage (READ-ACCESS)
        xTempSeek->seek( 0 );

        aArgs[0] <<= xTempIn;
        aArgs[1] <<= ElementModes::READ;
        m_xDefaultCfgRootStorage = Reference< XStorage >( xStorageFactory->createInstanceWithArguments( aArgs ), UNO_QUERY_THROW );

        // Customizable root storage (READWRITE-ACCESS)
        aArgs[0] <<= m_aUserConfigURL + aConfigSubFolder;
        aArgs[1] <<= ElementModes::READWRITE;
        m_xUserCfgRootStorage = Reference< XStorage >( xStorageFactory->createInstanceWithArguments( aArgs ), UNO_QUERY );

        // Create wrapper object for module user interface configuration managers, so they are able to call commit/revert on
        // root storage and nothing more (saftey)!
        RootStorageWrapper* pUserRootStorageWrapper = new RootStorageWrapper( Reference< XTransactedObject >( m_xUserCfgRootStorage, UNO_QUERY ));
        m_xUserRootCommit = Reference< XTransactedObject>( static_cast< OWeakObject *>( pUserRootStorageWrapper ), UNO_QUERY );
    }

    m_bInit = true;
}
*/


ModuleUIConfigurationManagerSupplier::ModuleUIConfigurationManagerSupplier( const Reference< XMultiServiceFactory >& xServiceManager ) :
    ThreadHelpBase( &Application::GetSolarMutex() )
    , m_bDisposed( false )
//TODO_AS    , m_bInit( false )
    , m_xModuleMgr( Reference< XModuleManager >( xServiceManager->createInstance( SERVICENAME_MODULEMANAGER ), UNO_QUERY ))
    , m_xServiceManager( xServiceManager )
    , m_aListenerContainer( m_aLock.getShareableOslMutex() )
{
    // Retrieve known modules and insert them into our hash_map to speed-up access time.
    Reference< XNameAccess > xNameAccess( m_xModuleMgr, UNO_QUERY );
    const Sequence< OUString >     aNameSeq   = xNameAccess->getElementNames();
    const OUString*                pNameSeq   = aNameSeq.getConstArray();
    for ( sal_Int32 n = 0; n < aNameSeq.getLength(); n++ )
        m_aModuleToModuleUICfgMgrMap.insert( ModuleToModuleCfgMgr::value_type(  pNameSeq[n], Reference< XUIConfigurationManager >() ));
}

ModuleUIConfigurationManagerSupplier::~ModuleUIConfigurationManagerSupplier()
{
    m_xUserRootCommit.clear();

    // dispose all our module user interface configuration managers
    ModuleToModuleCfgMgr::iterator pIter = m_aModuleToModuleUICfgMgrMap.begin();
    while ( pIter != m_aModuleToModuleUICfgMgrMap.end() )
    {
        Reference< XComponent > xComponent( pIter->second, UNO_QUERY );
        if ( xComponent.is() )
            xComponent->dispose();
        ++pIter;
    }
/*TODO_AS
    // Dispose our root configuration storages
    if ( m_xDefaultCfgRootStorage.is() )
    {
        Reference< XComponent > xComponent( m_xDefaultCfgRootStorage, UNO_QUERY );
        xComponent->dispose();
    }

    if ( m_xUserCfgRootStorage.is() )
    {
        Reference< XComponent > xComponent( m_xUserCfgRootStorage, UNO_QUERY );
        xComponent->dispose();
    }
*/
}

// XComponent
void SAL_CALL ModuleUIConfigurationManagerSupplier::dispose()
throw ( RuntimeException )
{
    Reference< XComponent > xThis( static_cast< OWeakObject* >(this), UNO_QUERY );

    css::lang::EventObject aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    {
        ResetableGuard aGuard( m_aLock );
        m_bDisposed = true;
    }
}

void SAL_CALL ModuleUIConfigurationManagerSupplier::addEventListener( const Reference< XEventListener >& xListener )
throw ( RuntimeException )
{
    {
        ResetableGuard aGuard( m_aLock );

        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        if ( m_bDisposed )
            throw DisposedException();
    }

    m_aListenerContainer.addInterface( ::getCppuType( ( const Reference< XEventListener >* ) NULL ), xListener );
}

void SAL_CALL ModuleUIConfigurationManagerSupplier::removeEventListener( const Reference< XEventListener >& xListener )
throw ( RuntimeException )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_aListenerContainer.removeInterface( ::getCppuType( ( const Reference< XEventListener >* ) NULL ), xListener );
}

// XModuleUIConfigurationManagerSupplier
Reference< XUIConfigurationManager > SAL_CALL ModuleUIConfigurationManagerSupplier::getUIConfigurationManager( const ::rtl::OUString& ModuleIdentifier )
throw ( NoSuchElementException, RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        throw DisposedException();

    ModuleToModuleCfgMgr::iterator pIter = m_aModuleToModuleUICfgMgrMap.find( ModuleIdentifier );
    if ( pIter == m_aModuleToModuleUICfgMgrMap.end() )
        throw NoSuchElementException();

    OUString sShort;
    try
    {
        Sequence< PropertyValue > lProps;
        Reference< XNameAccess > xCont(m_xModuleMgr, UNO_QUERY);
        xCont->getByName(ModuleIdentifier) >>= lProps;
        for (sal_Int32 i=0; i<lProps.getLength(); ++i)
        {
            if (lProps[i].Name.equalsAscii("ooSetupFactoryShortName"))
            {
                lProps[i].Value >>= sShort;
                break;
            }
        }
    }
    catch( Exception& )
    {
        sShort = OUString();
    }

    if (!sShort.getLength())
        throw NoSuchElementException();

//TODO_AS    impl_initStorages();

    // Create instance on demand
    if ( !pIter->second.is() )
    {
        /*TODO_AS
        Reference< XStorage > xDefaultConfigModuleStorage;
        Reference< XStorage > xUserConfigModuleStorage;

        try
        {
            xDefaultConfigModuleStorage = Reference< XStorage >( m_xDefaultCfgRootStorage->openStorageElement(
                                                                    sShort, ElementModes::READ ), UNO_QUERY_THROW );

            if ( m_xUserCfgRootStorage.is() )
            {
                try
                {
                    xUserConfigModuleStorage = Reference< XStorage >( m_xUserCfgRootStorage->openStorageElement(
                                                                        sShort, ElementModes::READWRITE ), UNO_QUERY );
                }
                catch( ::com::sun::star::io::IOException& )
                {
                    try
                    {
                        xUserConfigModuleStorage = Reference< XStorage >( m_xUserCfgRootStorage->openStorageElement(
                                                                            sShort, ElementModes::READ ), UNO_QUERY );
                    }
                    catch( com::sun::star::uno::Exception& )
                    {
                    }
                }
            }
        }
        catch ( com::sun::star::uno::Exception& )
        {
        }
        PropertyValue   aArg;
        Sequence< Any > aArgs( 5 );
        aArg.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "ModuleIdentifier" ));
        aArg.Value <<= ModuleIdentifier;
        aArgs[0] <<= aArg;
        aArg.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultConfigStorage" ));
        aArg.Value <<= xDefaultConfigModuleStorage;
        aArgs[1] <<= aArg;
        aArg.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "UserConfigStorage" ));
        aArg.Value <<= xUserConfigModuleStorage;
        aArgs[2] <<= aArg;
        aArg.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "UserRootCommit" ));
        aArg.Value <<= m_xUserRootCommit;
        aArgs[3] <<= aArg;
        */
        PropertyValue   aArg;
        Sequence< Any > aArgs( 2 );
        aArg.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "ModuleShortName" ));
        aArg.Value <<= sShort;
        aArgs[0] <<= aArg;
        aArg.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "ModuleIdentifier" ));
        aArg.Value <<= ModuleIdentifier;
        aArgs[1] <<= aArg;

        pIter->second = Reference< XUIConfigurationManager >( m_xServiceManager->createInstanceWithArguments(
                                                                SERVICENAME_MODULEUICONFIGURATIONMANAGER, aArgs ),
                                                              UNO_QUERY );
    }

    return pIter->second;
}

} // namespace framework
