/*************************************************************************
 *
 *  $RCSfile: uicommanddescription.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-02-25 17:52:43 $
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

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_UIELEMENT_UICOMMANDDESCRPTION_HXX_
#include "uielement/uicommanddescription.hxx"
#endif

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include "services.h"
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________
//

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace drafts::com::sun::star::frame;

//_________________________________________________________________________________________________________________
//  Namespace
//_________________________________________________________________________________________________________________
//

struct ModuleToCommands
{
    const char* pModuleId;
    const char* pCommands;
};

static const char GENERIC_UICOMMANDS[]                  = "generic";
static const char COMMANDS[]                            = "Commands";
static const char CONFIGURATION_ROOT_ACCESS[]           = "/org.openoffice.Office.UI.";
static const char CONFIGURATION_CMD_ELEMENT_ACCESS[]    = "/UserInterface/Commands";
static const char CONFIGURATION_POP_ELEMENT_ACCESS[]    = "/UserInterface/Popups";
static const char CONFIGURATION_PROPERTY_LABEL[]        = "Label";

// Property names of the resulting Property Set
static const char PROPSET_LABEL[]                       = "Label";
static const char PROPSET_NAME[]                        = "Name";
static const char PROPSET_POPUP[]                       = "Popup";

namespace framework
{

//*****************************************************************************************************************
//  Configuration access class for PopupMenuControllerFactory implementation
//*****************************************************************************************************************

class ConfigurationAccess_UICommand : // interfaces
                                        public  XTypeProvider                            ,
                                        public  XNameAccess                              ,
                                        public  XContainerListener                       ,
                                        // baseclasses
                                        // Order is neccessary for right initialization!
                                        private ThreadHelpBase                           ,
                                        public  ::cppu::OWeakObject
{
    public:
                                  ConfigurationAccess_UICommand( const ::rtl::OUString& aModuleName, const Reference< XNameAccess >& xGenericUICommands, const Reference< XMultiServiceFactory >& rServiceManager );
        virtual                   ~ConfigurationAccess_UICommand();

        //  XInterface, XTypeProvider
        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER

        // XNameAccess
        virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
            throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
            throw (::com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
            throw (::com::sun::star::uno::RuntimeException);

        // XElementAccess
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
            throw (::com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL hasElements()
            throw (::com::sun::star::uno::RuntimeException);

        // container.XContainerListener
        virtual void SAL_CALL     elementInserted( const ContainerEvent& aEvent ) throw(RuntimeException);
        virtual void SAL_CALL     elementRemoved ( const ContainerEvent& aEvent ) throw(RuntimeException);
        virtual void SAL_CALL     elementReplaced( const ContainerEvent& aEvent ) throw(RuntimeException);

        // lang.XEventListener
        virtual void SAL_CALL disposing( const EventObject& aEvent ) throw(RuntimeException);

    protected:
        struct CmdToInfoMap
        {
            rtl::OUString       aLabel;
            rtl::OUString       aCommandName;
            sal_Bool            bPopup;
        };

        Any                       insertCacheAndReturn( const rtl::OUString rCommandURL, const OUString& rLabel, sal_Bool bIsPopup );
        Any                       getInfoFromCommands( const rtl::OUString& rCommandURL );
        Any                       getInfoFromPopups( const rtl::OUString& rCommandURL );
        void                      fillInfoFromResult( CmdToInfoMap& rCmdInfo, const rtl::OUString& aLabel );
        Any                       getUILabelFromCommand( const rtl::OUString& rCommandURL );
        Sequence< rtl::OUString > getAllCommands();

    private:
        typedef ::std::hash_map< ::rtl::OUString,
                                 CmdToInfoMap,
                                 OUStringHashCode,
                                 ::std::equal_to< ::rtl::OUString > > CommandToInfoCache;

        sal_Bool initializeConfigAccess();

        rtl::OUString                     m_aConfigCmdAccess;
        rtl::OUString                     m_aConfigPopupAccess;
        rtl::OUString                     m_aPropUILabel;
        rtl::OUString                     m_aPropLabel;
        rtl::OUString                     m_aPropName;
        rtl::OUString                     m_aPropPopup;
        rtl::OUString                     m_aBrandName;
        rtl::OUString                     m_aXMLFileFormatVersion;
        rtl::OUString                     m_aVersion;
        rtl::OUString                     m_aExtension;
        Reference< XNameAccess >          m_xGenericUICommands;
        Reference< XMultiServiceFactory > m_xServiceManager;
        Reference< XMultiServiceFactory > m_xConfigProvider;
        Reference< XMultiServiceFactory > m_xConfigProviderPopups;
        Reference< XNameAccess >          m_xConfigAccess;
        Reference< XNameAccess >          m_xConfigAccessPopups;
        CommandToInfoCache                m_aCmdInfoCache;
        sal_Bool                          m_bConfigAccessInitialized;
};

//*****************************************************************************************************************
//  XInterface, XTypeProvider
//*****************************************************************************************************************
DEFINE_XINTERFACE_5     (   ConfigurationAccess_UICommand                                                   ,
                            OWeakObject                                                                     ,
                            DIRECT_INTERFACE ( css::container::XNameAccess                                  ),
                            DIRECT_INTERFACE ( css::container::XContainerListener                           ),
                            DIRECT_INTERFACE ( css::lang::XTypeProvider                                     ),
                            DERIVED_INTERFACE( css::container::XElementAccess, css::container::XNameAccess  ),
                            DERIVED_INTERFACE( css::lang::XEventListener, XContainerListener                )
                        )

DEFINE_XTYPEPROVIDER_5  (   ConfigurationAccess_UICommand       ,
                            css::container::XNameAccess         ,
                            css::container::XElementAccess      ,
                            css::container::XContainerListener  ,
                            css::lang::XTypeProvider            ,
                            css::lang::XEventListener
                        )

ConfigurationAccess_UICommand::ConfigurationAccess_UICommand( const rtl::OUString& aModuleName, const Reference< XNameAccess >& rGenericUICommands, const Reference< XMultiServiceFactory >& rServiceManager ) :
    ThreadHelpBase(),
    m_xServiceManager( rServiceManager ),
    m_aPropUILabel( RTL_CONSTASCII_USTRINGPARAM( CONFIGURATION_PROPERTY_LABEL )),
    m_bConfigAccessInitialized( sal_False ),
    m_aConfigCmdAccess( RTL_CONSTASCII_USTRINGPARAM( CONFIGURATION_ROOT_ACCESS )),
    m_aConfigPopupAccess( RTL_CONSTASCII_USTRINGPARAM( CONFIGURATION_ROOT_ACCESS )),
    m_aPropLabel( RTL_CONSTASCII_USTRINGPARAM( PROPSET_LABEL )),
    m_aPropName( RTL_CONSTASCII_USTRINGPARAM( PROPSET_NAME )),
    m_aPropPopup( RTL_CONSTASCII_USTRINGPARAM( PROPSET_POPUP )),
    m_xGenericUICommands( rGenericUICommands )
{
    // Create configuration hierachical access name
    m_aConfigCmdAccess += aModuleName;
    m_aConfigCmdAccess += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( CONFIGURATION_CMD_ELEMENT_ACCESS ));

    m_xConfigProvider = Reference< XMultiServiceFactory >( rServiceManager->createInstance(
                                                                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                                    "com.sun.star.configuration.ConfigurationProvider" ))),
                                                           UNO_QUERY );

    m_aConfigPopupAccess += aModuleName;
    m_aConfigPopupAccess += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( CONFIGURATION_POP_ELEMENT_ACCESS ));
    m_xConfigProviderPopups = Reference< XMultiServiceFactory >( rServiceManager->createInstance(
                                                                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                                    "com.sun.star.configuration.ConfigurationProvider" ))),
                                                           UNO_QUERY );

    Any aRet = ::utl::ConfigManager::GetDirectConfigProperty( ::utl::ConfigManager::PRODUCTNAME );
    rtl::OUString aTmp;
    aRet >>= aTmp;
    m_aBrandName = aTmp;
}

ConfigurationAccess_UICommand::~ConfigurationAccess_UICommand()
{
    // SAFE
    ResetableGuard aLock( m_aLock );
    Reference< XContainer > xContainer( m_xConfigAccess, UNO_QUERY );
    if ( xContainer.is() )
        xContainer->removeContainerListener( this );
    xContainer = Reference< XContainer >( m_xConfigAccessPopups, UNO_QUERY );
    if ( xContainer.is() )
        xContainer->removeContainerListener( this );
}

// XNameAccess
Any SAL_CALL ConfigurationAccess_UICommand::getByName( const ::rtl::OUString& rCommandURL )
throw ( NoSuchElementException, WrappedTargetException, RuntimeException)
{
    // SAFE
    ResetableGuard aLock( m_aLock );
    static sal_Int32 nRequests  = 0;
    static sal_Int32 nCacheHit  = 0;
    static sal_Int32 nCacheMiss = 0;

    ++nRequests;
    CommandToInfoCache::const_iterator pIter = m_aCmdInfoCache.find( rCommandURL );
    if ( pIter != m_aCmdInfoCache.end() )
    {
        ++nCacheHit;
        Sequence< PropertyValue > aPropSeq( 3 );
        aPropSeq[0].Name  = m_aPropLabel;
        aPropSeq[0].Value = makeAny( pIter->second.aLabel );
        aPropSeq[1].Name  = m_aPropName;
        aPropSeq[1].Value = makeAny( pIter->second.aCommandName );
        aPropSeq[2].Name  = m_aPropPopup;
        aPropSeq[2].Value = makeAny( pIter->second.bPopup );
        return makeAny( aPropSeq );
    }
    else
    {
        ++nCacheMiss;
        return getUILabelFromCommand( rCommandURL );
    }
}

Sequence< ::rtl::OUString > SAL_CALL ConfigurationAccess_UICommand::getElementNames()
throw ( RuntimeException )
{
    return getAllCommands();
}

sal_Bool SAL_CALL ConfigurationAccess_UICommand::hasByName( const ::rtl::OUString& rCommandURL )
throw (::com::sun::star::uno::RuntimeException)
{
    Any a = getByName( rCommandURL );
    if ( a != Any() )
        return sal_True;
    else
        return sal_False;
}

// XElementAccess
Type SAL_CALL ConfigurationAccess_UICommand::getElementType()
throw ( RuntimeException )
{
    return( ::getCppuType( (const Sequence< PropertyValue >*)NULL ) );
}

sal_Bool SAL_CALL ConfigurationAccess_UICommand::hasElements()
throw ( RuntimeException )
{
    // There must be global commands!
    return sal_True;
}

void ConfigurationAccess_UICommand::fillInfoFromResult( CmdToInfoMap& rCmdInfo, const rtl::OUString& aLabel )
{
    String rStr( aLabel );
    if ( rStr.SearchAscii( "%PRODUCT" ) != STRING_NOTFOUND )
        rStr.SearchAndReplaceAllAscii( "%PRODUCTNAME", m_aBrandName );
    rCmdInfo.aLabel       = OUString( rStr );
    rStr.EraseTrailingChars( '.' ); // Remove "..." from string
    rCmdInfo.aCommandName = OUString( rStr ); // TODO: After resync use SSA function to remove (~) or (..) from label
}

Any ConfigurationAccess_UICommand::insertCacheAndReturn( const rtl::OUString rCommandURL, const OUString& rLabel, sal_Bool bIsPopup )
{
    CmdToInfoMap aCmdToInfo;
    aCmdToInfo.bPopup = bIsPopup;

    fillInfoFromResult( aCmdToInfo, rLabel );
    m_aCmdInfoCache.insert( CommandToInfoCache::value_type( rCommandURL, aCmdToInfo ));

    Sequence< PropertyValue > aPropSeq( 3 );
    aPropSeq[0].Name  = m_aPropLabel;
    aPropSeq[0].Value = makeAny( aCmdToInfo.aLabel );
    aPropSeq[1].Name  = m_aPropName;
    aPropSeq[1].Value = makeAny( aCmdToInfo.aCommandName );
    aPropSeq[2].Name  = m_aPropPopup;
    aPropSeq[2].Value = makeAny( aCmdToInfo.bPopup );
    return makeAny( aPropSeq );
}

Any ConfigurationAccess_UICommand::getInfoFromCommands( const rtl::OUString& rCommandURL )
{
    Any a;

    try
    {
        // First try to ask our global commands configuration access. It also caches maybe
        // we find the entry in its cache first.
        if ( m_xGenericUICommands.is() )
        {
            try
            {
                return m_xGenericUICommands->getByName( rCommandURL );
            }
            catch ( com::sun::star::lang::WrappedTargetException& )
            {
            }
            catch ( com::sun::star::container::NoSuchElementException& )
            {
            }
        }

        // Try to ask our configuration access
        OUString                 aStr;
        Reference< XNameAccess > xNameAccess;
        a = m_xConfigAccess->getByName( rCommandURL );
        if ( a >>= xNameAccess )
            a = xNameAccess->getByName( m_aPropUILabel );
        a >>= aStr;

        return insertCacheAndReturn( rCommandURL, aStr, sal_False );
    }
    catch( com::sun::star::container::NoSuchElementException& )
    {
    }
    catch ( com::sun::star::lang::WrappedTargetException& )
    {
    }

    return a;
}

Any ConfigurationAccess_UICommand::getInfoFromPopups( const rtl::OUString& rCommandURL )
{
    Any a;

    try
    {
        // Try to ask our configuration access
        a = m_xConfigAccessPopups->getByName( rCommandURL );

        OUString                 aStr;
        Reference< XNameAccess > xNameAccess;
        if ( a >>= xNameAccess )
            a = xNameAccess->getByName( m_aPropUILabel );
        a >>= aStr;
        return insertCacheAndReturn( rCommandURL, aStr, sal_True );
    }
    catch( com::sun::star::container::NoSuchElementException& )
    {
    }
    catch ( com::sun::star::lang::WrappedTargetException& )
    {
    }

    return a;
}

Any ConfigurationAccess_UICommand::getUILabelFromCommand( const rtl::OUString& rCommandURL )
{
    Any a;

    if ( !m_bConfigAccessInitialized )
    {
        initializeConfigAccess();
        m_bConfigAccessInitialized = sal_True;
    }

    if ( m_xConfigAccess.is() && m_xConfigAccessPopups.is() )
    {
        a = getInfoFromCommands( rCommandURL );
        if ( a == Any() )
            a = getInfoFromPopups( rCommandURL );
        if ( a == Any() )
            throw NoSuchElementException();
    }

    return a;
}

Sequence< rtl::OUString > ConfigurationAccess_UICommand::getAllCommands()
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    if ( !m_bConfigAccessInitialized )
    {
        initializeConfigAccess();
        m_bConfigAccessInitialized = sal_True;
    }

    if ( m_xConfigAccess.is() )
    {
        Any                      a;
        Reference< XNameAccess > xNameAccess;

        try
        {
            Sequence< OUString > aNameSeq = m_xConfigAccess->getElementNames();

            if ( m_xGenericUICommands.is() )
            {
                // Create concat list of supported user interface commands of the module
                Sequence< OUString > aGenericNameSeq = m_xGenericUICommands->getElementNames();
                sal_uInt32 nCount1 = aNameSeq.getLength();
                sal_uInt32 nCount2 = aGenericNameSeq.getLength();

                aNameSeq.realloc( nCount1 + nCount2 );
                OUString* pNameSeq = aNameSeq.getArray();
                const OUString* pGenericSeq = aGenericNameSeq.getConstArray();
                for ( sal_uInt32 i = 0; i < nCount2; i++ )
                    pNameSeq[nCount1+i] = pGenericSeq[i];
            }

            return aNameSeq;
        }
        catch( com::sun::star::container::NoSuchElementException& )
        {
        }
        catch ( com::sun::star::lang::WrappedTargetException& )
        {
        }
    }

    return Sequence< rtl::OUString >();
}

sal_Bool ConfigurationAccess_UICommand::initializeConfigAccess()
{
    Sequence< Any > aArgs( 1 );
    PropertyValue   aPropValue;

    try
    {
        aPropValue.Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "nodepath" ));
        aPropValue.Value = makeAny( m_aConfigCmdAccess );
        aArgs[0] <<= aPropValue;

        m_xConfigAccess = Reference< XNameAccess >( m_xConfigProvider->createInstanceWithArguments(
                                                                            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                                                "com.sun.star.configuration.ConfigurationAccess" )),
                                                                            aArgs ),
                                                                        UNO_QUERY );
        if ( m_xConfigAccess.is() )
        {
            // Add as container listener
            Reference< XContainer > xContainer( m_xConfigAccess, UNO_QUERY );
            if ( xContainer.is() )
                xContainer->addContainerListener( this );
        }

        aPropValue.Value = makeAny( m_aConfigPopupAccess );
        aArgs[0] <<= aPropValue;
        m_xConfigAccessPopups = Reference< XNameAccess >( m_xConfigProvider->createInstanceWithArguments(
                                                                            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                                                "com.sun.star.configuration.ConfigurationAccess" )),
                                                                            aArgs ),
                                                                        UNO_QUERY );
        if ( m_xConfigAccessPopups.is() )
        {
            // Add as container listener
            Reference< XContainer > xContainer( m_xConfigAccessPopups, UNO_QUERY );
            if ( xContainer.is() )
                xContainer->addContainerListener( this );
        }

        return sal_True;
    }
    catch ( WrappedTargetException& e )
    {
    }
    catch ( Exception& e )
    {
    }

    return sal_False;
}

// container.XContainerListener
void SAL_CALL ConfigurationAccess_UICommand::elementInserted( const ContainerEvent& aEvent ) throw(RuntimeException)
{
}

void SAL_CALL ConfigurationAccess_UICommand::elementRemoved ( const ContainerEvent& aEvent ) throw(RuntimeException)
{
}

void SAL_CALL ConfigurationAccess_UICommand::elementReplaced( const ContainerEvent& aEvent ) throw(RuntimeException)
{
}

// lang.XEventListener
void SAL_CALL ConfigurationAccess_UICommand::disposing( const EventObject& aEvent ) throw(RuntimeException)
{
    // SAFE
    // remove our reference to the config access
    ResetableGuard aLock( m_aLock );

    Reference< XInterface > xIfac1( aEvent.Source, UNO_QUERY );
    Reference< XInterface > xIfac2( m_xConfigAccess, UNO_QUERY );
    if ( xIfac1 == xIfac2 )
        m_xConfigAccess.clear();
    else
    {
        xIfac2 = Reference< XInterface >( m_xConfigAccessPopups, UNO_QUERY );
        if ( xIfac1 == xIfac2 )
            m_xConfigAccessPopups.clear();
    }
}

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XINTERFACE_4                    (    UICommandDescription                                                            ,
                                            OWeakObject                                                                     ,
                                            DIRECT_INTERFACE( css::lang::XTypeProvider                                      ),
                                            DIRECT_INTERFACE( css::lang::XServiceInfo                                       ),
                                            DIRECT_INTERFACE( css::container::XNameAccess                                   ),
                                            DERIVED_INTERFACE( css::container::XElementAccess, css::container::XNameAccess  )
                                        )

DEFINE_XTYPEPROVIDER_4                  (   UICommandDescription            ,
                                            css::lang::XTypeProvider        ,
                                            css::lang::XServiceInfo         ,
                                            css::container::XNameAccess     ,
                                            css::container::XElementAccess
                                        )

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   UICommandDescription                    ,
                                            ::cppu::OWeakObject                     ,
                                            SERVICENAME_UICOMMANDDESCRIPTION        ,
                                            IMPLEMENTATIONNAME_UICOMMANDDESCRIPTION
                                        )

DEFINE_INIT_SERVICE                     (   UICommandDescription, {} )

UICommandDescription::UICommandDescription( const Reference< XMultiServiceFactory >& xServiceManager ) :
    ThreadHelpBase(),
    m_xServiceManager( xServiceManager )
{
    Reference< XNameAccess > xEmpty;
    rtl::OUString aGenericUICommand( OUString::createFromAscii( "GenericCommands" ));
    m_xGenericUICommands = new ConfigurationAccess_UICommand( aGenericUICommand, xEmpty, xServiceManager );

    m_xModuleManager = Reference< XModuleManager >( m_xServiceManager->createInstance( SERVICENAME_MODULEMANAGER ),
                                                    UNO_QUERY );
    Reference< XNameAccess > xNameAccess( m_xModuleManager, UNO_QUERY_THROW );
    Sequence< rtl::OUString > aElementNames = xNameAccess->getElementNames();
    Sequence< PropertyValue > aSeq;
    OUString                  aModuleIdentifier;

    for ( sal_Int32 i = 0; i < aElementNames.getLength(); i++ )
    {
        aModuleIdentifier = aElementNames[i];
        Any a = xNameAccess->getByName( aModuleIdentifier );
        if ( a >>= aSeq )
        {
            OUString aCommandStr;
            for ( sal_Int32 y = 0; y < aSeq.getLength(); y++ )
            {
                if ( aSeq[y].Name.equalsAscii("ooSetupFactoryCommandConfigRef") )
                {
                    aSeq[y].Value >>= aCommandStr;
                    break;
                }
            }

            // Create first mapping ModuleIdentifier ==> Command File
            m_aModuleToCommandFileMap.insert( ModuleToCommandFileMap::value_type( aModuleIdentifier, aCommandStr ));

            // Create second mapping Command File ==> commands instance
            UICommandsHashMap::iterator pIter = m_aUICommandsHashMap.find( aCommandStr );
            if ( pIter == m_aUICommandsHashMap.end() )
                m_aUICommandsHashMap.insert( UICommandsHashMap::value_type( aCommandStr, Reference< XNameAccess >() ));
        }
    }

    // insert generic commands
    UICommandsHashMap::iterator pIter = m_aUICommandsHashMap.find( aGenericUICommand );
    if ( pIter != m_aUICommandsHashMap.end() )
        pIter->second = m_xGenericUICommands;
}

UICommandDescription::~UICommandDescription()
{
    ResetableGuard aLock( m_aLock );
    m_aModuleToCommandFileMap.clear();
    m_aUICommandsHashMap.clear();
    m_xGenericUICommands.clear();
}

Any SAL_CALL UICommandDescription::getByName( const ::rtl::OUString& aName )
throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    Any a;

    ResetableGuard aLock( m_aLock );

    ModuleToCommandFileMap::const_iterator pIter = m_aModuleToCommandFileMap.find( aName );
    if ( pIter != m_aModuleToCommandFileMap.end() )
    {
        OUString aCommandFile( pIter->second );
        UICommandsHashMap::iterator pIter = m_aUICommandsHashMap.find( aCommandFile );
        if ( pIter != m_aUICommandsHashMap.end() )
        {
            if ( pIter->second.is() )
                a <<= pIter->second;
            else
            {
                Reference< XNameAccess > xUICommands;
                ConfigurationAccess_UICommand* pUICommands = new ConfigurationAccess_UICommand( aCommandFile,
                                                                                               m_xGenericUICommands,
                                                                                               m_xServiceManager );
                xUICommands = Reference< XNameAccess >( static_cast< cppu::OWeakObject* >( pUICommands ),UNO_QUERY );
                pIter->second = xUICommands;
                a <<= xUICommands;
            }
        }
    }
    else
    {
        throw NoSuchElementException();
    }

    return a;
}

Sequence< ::rtl::OUString > SAL_CALL UICommandDescription::getElementNames()
throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    Sequence< rtl::OUString > aSeq( m_aModuleToCommandFileMap.size() );

    sal_Int32 n = 0;
    ModuleToCommandFileMap::const_iterator pIter = m_aModuleToCommandFileMap.begin();
    while ( pIter != m_aModuleToCommandFileMap.end() )
    {
        aSeq[n] = pIter->first;
        ++pIter;
    }

    return aSeq;
}

sal_Bool SAL_CALL UICommandDescription::hasByName( const ::rtl::OUString& aName )
throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    ModuleToCommandFileMap::const_iterator pIter = m_aModuleToCommandFileMap.find( aName );
    return ( pIter != m_aModuleToCommandFileMap.end() );
}

// XElementAccess
Type SAL_CALL UICommandDescription::getElementType()
throw (::com::sun::star::uno::RuntimeException)
{
    return( ::getCppuType( (const Reference< XNameAccess >*)NULL ) );
}

sal_Bool SAL_CALL UICommandDescription::hasElements()
throw (::com::sun::star::uno::RuntimeException)
{
    // generic UI commands are always available!
    return sal_True;
}

} // namespace framework
