/*************************************************************************
 *
 *  $RCSfile: moduleuicfgsupplier.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 16:59:18 $
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

#ifndef __FRAMEWORK_UICONFIGURATION_MODULEUICFGSUPPLIER_HXX_
#include <uiconfiguration/moduleuicfgsupplier.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_XPACKAGESTRUCTURECREATOR_HPP_
#include <com/sun/star/embed/XPackageStructureCreator.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

#include <vcl/svapp.hxx>

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::embed;
using namespace drafts::com::sun::star::ui;
using namespace drafts::com::sun::star::frame;

namespace framework
{

class RootStorageWrapper :  public com::sun::star::lang::XTypeProvider      ,
                            public com::sun::star::embed::XTransactedObject ,
                            public ::cppu::OWeakObject
{
    public:
        //  XInterface, XTypeProvider
        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER

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
                                            DIRECT_INTERFACE( drafts::com::sun::star::ui::XModuleUIConfigurationManagerSupplier )
                                        )

DEFINE_XTYPEPROVIDER_4                  (   ModuleUIConfigurationManagerSupplier                                ,
                                            css::lang::XTypeProvider                                            ,
                                            css::lang::XServiceInfo                                             ,
                                            css::lang::XComponent                                               ,
                                            drafts::com::sun::star::ui::XModuleUIConfigurationManagerSupplier
                                        )

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   ModuleUIConfigurationManagerSupplier                    ,
                                            ::cppu::OWeakObject                                     ,
                                            SERVICENAME_MODULEUICONFIGURATIONMANAGERSUPPLIER        ,
                                            IMPLEMENTATIONNAME_MODULEUICONFIGURATIONMANAGERSUPPLIER
                                        )

DEFINE_INIT_SERVICE                     (   ModuleUIConfigurationManagerSupplier, {} )


void ModuleUIConfigurationManagerSupplier::impl_initStorages()
{
    if ( !m_bInit )
    {
        RTL_LOGFILE_CONTEXT( aLog, "framework (cd100003) ::ModuleUIConfigurationManagerSupplier::impl_initStorages" );

        rtl::OUString aFinalSlash( RTL_CONSTASCII_USTRINGPARAM( "/" ));
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
/*
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
*/

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


ModuleUIConfigurationManagerSupplier::ModuleUIConfigurationManagerSupplier( const Reference< XMultiServiceFactory >& xServiceManager ) :
    ThreadHelpBase( &Application::GetSolarMutex() )
    , m_aListenerContainer( m_aLock.getShareableOslMutex() )
    , m_xServiceManager( xServiceManager )
    , m_bDisposed( false )
    , m_bInit( false )
    , m_xModuleMgr( Reference< XModuleManager >( xServiceManager->createInstance( SERVICENAME_MODULEMANAGER ), UNO_QUERY ))
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

    impl_initStorages();

    // Create instance on demand
    if ( !pIter->second.is() )
    {
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
        aArg.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "ModuleShortName" ));
        aArg.Value <<= sShort;
        aArgs[4] <<= aArg;

        pIter->second = Reference< XUIConfigurationManager >( m_xServiceManager->createInstanceWithArguments(
                                                                SERVICENAME_MODULEUICONFIGURATIONMANAGER, aArgs ),
                                                              UNO_QUERY );
    }

    return pIter->second;
}

} // namespace framework
