/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: uicommanddescription.cxx,v $
 * $Revision: 1.16 $
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

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include "uielement/uicommanddescription.hxx"
#include <threadhelp/resetableguard.hxx>
#include "services.h"

#include "properties.h"

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________
#include <rtl/ustrbuf.hxx>
#include <cppuhelper/weak.hxx>
#include <unotools/configmgr.hxx>
#include <tools/string.hxx>

#ifndef _VCL_MNEMONIC_HXX_
#include <vcl/mnemonic.hxx>
#endif
#include <comphelper/sequence.hxx>
#include <rtl/logfile.hxx>

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________
//

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace ::com::sun::star::frame;

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
static const char CONFIGURATION_PROPERTY_CONTEXT_LABEL[] = "ContextLabel";

// Property names of the resulting Property Set
static const char PROPSET_LABEL[]                       = "Label";
static const char PROPSET_NAME[]                        = "Name";
static const char PROPSET_POPUP[]                       = "Popup";
static const char PROPSET_PROPERTIES[]                  = "Properties";

// Special resource URLs to retrieve additional information
static const char PRIVATE_RESOURCE_URL[]                = "private:";

const sal_Int32   COMMAND_PROPERTY_IMAGE                = 1;
const sal_Int32   COMMAND_PROPERTY_ROTATE               = 2;
const sal_Int32   COMMAND_PROPERTY_MIRROR               = 4;

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
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER

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
            CmdToInfoMap() : bPopup( false ),
                             bCommandNameCreated( false ),
                             nProperties( 0 ) {}

            rtl::OUString       aLabel;
            rtl::OUString       aContextLabel;
            rtl::OUString       aCommandName;
            bool                bPopup : 1,
                                bCommandNameCreated : 1;
            sal_Int32           nProperties;
        };

        Any                       getSequenceFromCache( const rtl::OUString& aCommandURL );
        Any                       getInfoFromCommand( const rtl::OUString& rCommandURL );
        void                      fillInfoFromResult( CmdToInfoMap& rCmdInfo, const rtl::OUString& aLabel );
        Any                       getUILabelFromCommand( const rtl::OUString& rCommandURL );
        Sequence< rtl::OUString > getAllCommands();
        void                      resetCache();
        sal_Bool                  fillCache();
        sal_Bool                  addGenericInfoToCache();

    private:
        typedef ::std::hash_map< ::rtl::OUString,
                                 CmdToInfoMap,
                                 OUStringHashCode,
                                 ::std::equal_to< ::rtl::OUString > > CommandToInfoCache;

        sal_Bool initializeConfigAccess();

        rtl::OUString                     m_aConfigCmdAccess;
        rtl::OUString                     m_aConfigPopupAccess;
        rtl::OUString                     m_aPropUILabel;
        rtl::OUString                     m_aPropUIContextLabel;
        rtl::OUString                     m_aPropLabel;
        rtl::OUString                     m_aPropName;
        rtl::OUString                     m_aPropPopup;
        rtl::OUString                     m_aPropProperties;
        rtl::OUString                     m_aBrandName;
        rtl::OUString                     m_aXMLFileFormatVersion;
        rtl::OUString                     m_aVersion;
        rtl::OUString                     m_aExtension;
        rtl::OUString                     m_aPrivateResourceURL;
        Reference< XNameAccess >          m_xGenericUICommands;
        Reference< XMultiServiceFactory > m_xServiceManager;
        Reference< XMultiServiceFactory > m_xConfigProvider;
        Reference< XMultiServiceFactory > m_xConfigProviderPopups;
        Reference< XNameAccess >          m_xConfigAccess;
        Reference< XNameAccess >          m_xConfigAccessPopups;
        Sequence< rtl::OUString >         m_aCommandImageList;
        Sequence< rtl::OUString >         m_aCommandRotateImageList;
        Sequence< rtl::OUString >         m_aCommandMirrorImageList;
        CommandToInfoCache                m_aCmdInfoCache;
        sal_Bool                          m_bConfigAccessInitialized;
        sal_Bool                          m_bCacheFilled;
        sal_Bool                          m_bGenericDataRetrieved;
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
    m_aConfigCmdAccess( RTL_CONSTASCII_USTRINGPARAM( CONFIGURATION_ROOT_ACCESS )),
    m_aConfigPopupAccess( RTL_CONSTASCII_USTRINGPARAM( CONFIGURATION_ROOT_ACCESS )),
    m_aPropUILabel( RTL_CONSTASCII_USTRINGPARAM( CONFIGURATION_PROPERTY_LABEL )),
    m_aPropUIContextLabel( RTL_CONSTASCII_USTRINGPARAM( CONFIGURATION_PROPERTY_CONTEXT_LABEL )),
    m_aPropLabel( RTL_CONSTASCII_USTRINGPARAM( PROPSET_LABEL )),
    m_aPropName( RTL_CONSTASCII_USTRINGPARAM( PROPSET_NAME )),
    m_aPropPopup( RTL_CONSTASCII_USTRINGPARAM( PROPSET_POPUP )),
    m_aPropProperties( RTL_CONSTASCII_USTRINGPARAM( PROPSET_PROPERTIES )),
    m_aPrivateResourceURL( RTL_CONSTASCII_USTRINGPARAM( PRIVATE_RESOURCE_URL )),
    m_xGenericUICommands( rGenericUICommands ),
    m_xServiceManager( rServiceManager ),
    m_bConfigAccessInitialized( sal_False ),
    m_bCacheFilled( sal_False ),
    m_bGenericDataRetrieved( sal_False )
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
    static sal_Int32 nRequests  = 0;

    ResetableGuard aLock( m_aLock );
    if ( !m_bConfigAccessInitialized )
    {
        initializeConfigAccess();
        m_bConfigAccessInitialized = sal_True;
        fillCache();
    }

    if ( rCommandURL.indexOf( m_aPrivateResourceURL ) == 0 )
    {
        // special keys to retrieve information about a set of commands
        // SAFE
        addGenericInfoToCache();

        if ( rCommandURL.equalsIgnoreAsciiCaseAscii( UICOMMANDDESCRIPTION_NAMEACCESS_COMMANDIMAGELIST ))
            return makeAny( m_aCommandImageList );
        else if ( rCommandURL.equalsIgnoreAsciiCaseAscii( UICOMMANDDESCRIPTION_NAMEACCESS_COMMANDROTATEIMAGELIST ))
            return makeAny( m_aCommandRotateImageList );
        else if ( rCommandURL.equalsIgnoreAsciiCaseAscii( UICOMMANDDESCRIPTION_NAMEACCESS_COMMANDMIRRORIMAGELIST ))
            return makeAny( m_aCommandMirrorImageList );
        else
            throw NoSuchElementException();
    }
    else
    {
        // SAFE
        ++nRequests;
        Any a = getInfoFromCommand( rCommandURL );

        if ( !a.hasValue() )
            throw NoSuchElementException();

        return a;
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
    // There must are global commands!
    return sal_True;
}

void ConfigurationAccess_UICommand::fillInfoFromResult( CmdToInfoMap& rCmdInfo, const rtl::OUString& aLabel )
{
    String rStr( aLabel );
    if ( rStr.SearchAscii( "%PRODUCT" ) != STRING_NOTFOUND )
        rStr.SearchAndReplaceAllAscii( "%PRODUCTNAME", m_aBrandName );
    rCmdInfo.aLabel       = OUString( rStr );
    rStr.EraseTrailingChars( '.' ); // Remove "..." from string
    rCmdInfo.aCommandName = OUString( MnemonicGenerator::EraseAllMnemonicChars( rStr ));
    rCmdInfo.bCommandNameCreated = sal_True;
}

Any ConfigurationAccess_UICommand::getSequenceFromCache( const OUString& aCommandURL )
{
    CommandToInfoCache::iterator pIter = m_aCmdInfoCache.find( aCommandURL );
    if ( pIter != m_aCmdInfoCache.end() )
    {
        if ( !pIter->second.bCommandNameCreated )
            fillInfoFromResult( pIter->second, pIter->second.aLabel );

        Sequence< PropertyValue > aPropSeq( 3 );
        aPropSeq[0].Name  = m_aPropLabel;
        aPropSeq[0].Value = pIter->second.aContextLabel.getLength() ?
                makeAny( pIter->second.aContextLabel ): makeAny( pIter->second.aLabel );
        aPropSeq[1].Name  = m_aPropName;
        aPropSeq[1].Value = makeAny( pIter->second.aCommandName );
        aPropSeq[2].Name  = m_aPropPopup;
        aPropSeq[2].Value = makeAny( pIter->second.bPopup );
        return makeAny( aPropSeq );
    }

    return Any();
}

void ConfigurationAccess_UICommand::resetCache()
{
    m_aCmdInfoCache.clear();
    m_bCacheFilled = sal_False;
    m_bGenericDataRetrieved = sal_False;
}

sal_Bool ConfigurationAccess_UICommand::fillCache()
{
    RTL_LOGFILE_CONTEXT( aLog, "framework (cd100003) ::ConfigurationAccess_UICommand::fillCache" );

    if ( m_bCacheFilled )
        return sal_True;

    sal_Int32               i( 0 );
    Any                     a;
    std::vector< OUString > aImageCommandVector;
    std::vector< OUString > aImageRotateVector;
    std::vector< OUString > aImageMirrorVector;
    Sequence< OUString >    aNameSeq;

    if ( m_xConfigAccess.is() )
    {
        aNameSeq = m_xConfigAccess->getElementNames();
        for ( i = 0; i < aNameSeq.getLength(); i++ )
        {
            try
            {
                Reference< XNameAccess > xNameAccess;
                a = m_xConfigAccess->getByName( aNameSeq[i] );
                if ( a >>= xNameAccess )
                {
                    CmdToInfoMap aCmdToInfo;

                    a = xNameAccess->getByName( m_aPropUILabel );
                    a >>= aCmdToInfo.aLabel;
                    a = xNameAccess->getByName( m_aPropUIContextLabel );
                    a >>= aCmdToInfo.aContextLabel;
                    a = xNameAccess->getByName( m_aPropProperties );
                    a >>= aCmdToInfo.nProperties;

                    m_aCmdInfoCache.insert( CommandToInfoCache::value_type( aNameSeq[i], aCmdToInfo ));

                    if ( aCmdToInfo.nProperties & COMMAND_PROPERTY_IMAGE )
                        aImageCommandVector.push_back( aNameSeq[i] );
                    if ( aCmdToInfo.nProperties & COMMAND_PROPERTY_ROTATE )
                        aImageRotateVector.push_back( aNameSeq[i] );
                    if ( aCmdToInfo.nProperties & COMMAND_PROPERTY_MIRROR )
                        aImageMirrorVector.push_back( aNameSeq[i] );
                }
            }
            catch ( com::sun::star::lang::WrappedTargetException& )
            {
            }
            catch ( com::sun::star::container::NoSuchElementException& )
            {
            }
        }
    }

    if ( m_xConfigAccessPopups.is() )
    {
        aNameSeq = m_xConfigAccessPopups->getElementNames();
        for ( i = 0; i < aNameSeq.getLength(); i++ )
        {
            try
            {
                Reference< XNameAccess > xNameAccess;
                a = m_xConfigAccessPopups->getByName( aNameSeq[i] );
                if ( a >>= xNameAccess )
                {
                    CmdToInfoMap aCmdToInfo;

                    aCmdToInfo.bPopup = sal_True;
                    a = xNameAccess->getByName( m_aPropUILabel );
                    a >>= aCmdToInfo.aLabel;
                    a = xNameAccess->getByName( m_aPropUIContextLabel );
                    a >>= aCmdToInfo.aContextLabel;
                    a = xNameAccess->getByName( m_aPropProperties );
                    a >>= aCmdToInfo.nProperties;

                    m_aCmdInfoCache.insert( CommandToInfoCache::value_type( aNameSeq[i], aCmdToInfo ));

                    if ( aCmdToInfo.nProperties & COMMAND_PROPERTY_IMAGE )
                        aImageCommandVector.push_back( aNameSeq[i] );
                    if ( aCmdToInfo.nProperties & COMMAND_PROPERTY_ROTATE )
                        aImageRotateVector.push_back( aNameSeq[i] );
                    if ( aCmdToInfo.nProperties & COMMAND_PROPERTY_MIRROR )
                        aImageMirrorVector.push_back( aNameSeq[i] );
                }
            }
            catch ( com::sun::star::lang::WrappedTargetException& )
            {
            }
            catch ( com::sun::star::container::NoSuchElementException& )
            {
            }
        }
    }

    // Create cached sequences for fast retrieving
    m_aCommandImageList       = comphelper::containerToSequence( aImageCommandVector );
    m_aCommandRotateImageList = comphelper::containerToSequence( aImageRotateVector );
    m_aCommandMirrorImageList = comphelper::containerToSequence( aImageMirrorVector );

    m_bCacheFilled = sal_True;

    return sal_True;
}

sal_Bool ConfigurationAccess_UICommand::addGenericInfoToCache()
{
    if ( m_xGenericUICommands.is() && !m_bGenericDataRetrieved )
    {
        Sequence< rtl::OUString > aCommandNameSeq;
        try
        {
            if ( m_xGenericUICommands->getByName(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( UICOMMANDDESCRIPTION_NAMEACCESS_COMMANDROTATEIMAGELIST ))) >>= aCommandNameSeq )
                m_aCommandRotateImageList = comphelper::concatSequences< rtl::OUString >( m_aCommandRotateImageList, aCommandNameSeq );
        }
        catch ( RuntimeException& e )
        {
            throw e;
        }
        catch ( Exception& )
        {
        }

        try
        {
            if ( m_xGenericUICommands->getByName(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( UICOMMANDDESCRIPTION_NAMEACCESS_COMMANDMIRRORIMAGELIST ))) >>= aCommandNameSeq )
                m_aCommandMirrorImageList = comphelper::concatSequences< rtl::OUString >( m_aCommandMirrorImageList, aCommandNameSeq );
        }
        catch ( RuntimeException& e )
        {
            throw e;
        }
        catch ( Exception& )
        {
        }

        m_bGenericDataRetrieved = sal_True;
    }

    return sal_True;
}

Any ConfigurationAccess_UICommand::getInfoFromCommand( const rtl::OUString& rCommandURL )
{
    Any a;

    try
    {
        a = getSequenceFromCache( rCommandURL );
        if ( !a.hasValue() )
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
        }
    }
    catch( com::sun::star::container::NoSuchElementException& )
    {
    }
    catch ( com::sun::star::lang::WrappedTargetException& )
    {
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
        fillCache();
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
    catch ( WrappedTargetException& )
    {
    }
    catch ( Exception& )
    {
    }

    return sal_False;
}

// container.XContainerListener
void SAL_CALL ConfigurationAccess_UICommand::elementInserted( const ContainerEvent& ) throw(RuntimeException)
{
    ResetableGuard aLock( m_aLock );
    m_bCacheFilled = sal_False;
    fillCache();
}

void SAL_CALL ConfigurationAccess_UICommand::elementRemoved( const ContainerEvent& ) throw(RuntimeException)
{
    ResetableGuard aLock( m_aLock );
    m_bCacheFilled = sal_False;
    fillCache();
}

void SAL_CALL ConfigurationAccess_UICommand::elementReplaced( const ContainerEvent& ) throw(RuntimeException)
{
    ResetableGuard aLock( m_aLock );
    m_bCacheFilled = sal_False;
    fillCache();
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
    m_aPrivateResourceURL( RTL_CONSTASCII_USTRINGPARAM( PRIVATE_RESOURCE_URL )),
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

    ModuleToCommandFileMap::const_iterator pM2CIter = m_aModuleToCommandFileMap.find( aName );
    if ( pM2CIter != m_aModuleToCommandFileMap.end() )
    {
        OUString aCommandFile( pM2CIter->second );
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
    else if ( aName.indexOf( m_aPrivateResourceURL ) == 0 )
    {
        // special keys to retrieve information about a set of commands
        return m_xGenericUICommands->getByName( aName );
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
