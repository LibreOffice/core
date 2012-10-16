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

#include "uiconfiguration/windowstateconfiguration.hxx"
#include <threadhelp/resetableguard.hxx>
#include "services.h"

#include "helper/mischelper.hxx"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/ui/DockingArea.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>

#include <rtl/ustrbuf.hxx>
#include <cppuhelper/weak.hxx>
#include <tools/debug.hxx>

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace com::sun::star::container;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::ui;

//_________________________________________________________________________________________________________________
//  Namespace
//_________________________________________________________________________________________________________________

static const char CONFIGURATION_ROOT_ACCESS[]               = "/org.openoffice.Office.UI.";
static const char CONFIGURATION_WINDOWSTATE_ACCESS[]        = "/UIElements/States";

static const char CONFIGURATION_PROPERTY_LOCKED[]           = WINDOWSTATE_PROPERTY_LOCKED;
static const char CONFIGURATION_PROPERTY_DOCKED[]           = WINDOWSTATE_PROPERTY_DOCKED;
static const char CONFIGURATION_PROPERTY_VISIBLE[]          = WINDOWSTATE_PROPERTY_VISIBLE;
static const char CONFIGURATION_PROPERTY_DOCKINGAREA[]      = WINDOWSTATE_PROPERTY_DOCKINGAREA;
static const char CONFIGURATION_PROPERTY_DOCKPOS[]          = WINDOWSTATE_PROPERTY_DOCKPOS;
static const char CONFIGURATION_PROPERTY_DOCKSIZE[]         = WINDOWSTATE_PROPERTY_DOCKSIZE;
static const char CONFIGURATION_PROPERTY_POS[]              = WINDOWSTATE_PROPERTY_POS;
static const char CONFIGURATION_PROPERTY_SIZE[]             = WINDOWSTATE_PROPERTY_SIZE;
static const char CONFIGURATION_PROPERTY_UINAME[]           = WINDOWSTATE_PROPERTY_UINAME;
static const char CONFIGURATION_PROPERTY_INTERNALSTATE[]    = WINDOWSTATE_PROPERTY_INTERNALSTATE;
static const char CONFIGURATION_PROPERTY_STYLE[]            = WINDOWSTATE_PROPERTY_STYLE;
static const char CONFIGURATION_PROPERTY_CONTEXT[]          = WINDOWSTATE_PROPERTY_CONTEXT;
static const char CONFIGURATION_PROPERTY_HIDEFROMMENU[]     = WINDOWSTATE_PROPERTY_HIDEFROMENU;
static const char CONFIGURATION_PROPERTY_NOCLOSE[]          = WINDOWSTATE_PROPERTY_NOCLOSE;
static const char CONFIGURATION_PROPERTY_SOFTCLOSE[]        = WINDOWSTATE_PROPERTY_SOFTCLOSE;
static const char CONFIGURATION_PROPERTY_CONTEXTACTIVE[]    = WINDOWSTATE_PROPERTY_CONTEXTACTIVE;

// Zero based indexes, order must be the same as WindowStateMask && CONFIGURATION_PROPERTIES!
static const sal_Int16 PROPERTY_LOCKED                  = 0;
static const sal_Int16 PROPERTY_DOCKED                  = 1;
static const sal_Int16 PROPERTY_VISIBLE                 = 2;
static const sal_Int16 PROPERTY_CONTEXT                 = 3;
static const sal_Int16 PROPERTY_HIDEFROMMENU            = 4;
static const sal_Int16 PROPERTY_NOCLOSE                 = 5;
static const sal_Int16 PROPERTY_SOFTCLOSE               = 6;
static const sal_Int16 PROPERTY_CONTEXTACTIVE           = 7;
static const sal_Int16 PROPERTY_DOCKINGAREA             = 8;
static const sal_Int16 PROPERTY_POS                     = 9;
static const sal_Int16 PROPERTY_SIZE                    = 10;
static const sal_Int16 PROPERTY_UINAME                  = 11;
static const sal_Int16 PROPERTY_INTERNALSTATE           = 12;
static const sal_Int16 PROPERTY_STYLE                   = 13;
static const sal_Int16 PROPERTY_DOCKPOS                 = 14;
static const sal_Int16 PROPERTY_DOCKSIZE                = 15;

// Order must be the same as WindowStateMask!!
static const char* CONFIGURATION_PROPERTIES[]           =
{
    CONFIGURATION_PROPERTY_LOCKED,
    CONFIGURATION_PROPERTY_DOCKED,
    CONFIGURATION_PROPERTY_VISIBLE,
    CONFIGURATION_PROPERTY_CONTEXT,
    CONFIGURATION_PROPERTY_HIDEFROMMENU,
    CONFIGURATION_PROPERTY_NOCLOSE,
    CONFIGURATION_PROPERTY_SOFTCLOSE,
    CONFIGURATION_PROPERTY_CONTEXTACTIVE,
    CONFIGURATION_PROPERTY_DOCKINGAREA,
    CONFIGURATION_PROPERTY_POS,
    CONFIGURATION_PROPERTY_SIZE,
    CONFIGURATION_PROPERTY_UINAME,
    CONFIGURATION_PROPERTY_INTERNALSTATE,
    CONFIGURATION_PROPERTY_STYLE,
    CONFIGURATION_PROPERTY_DOCKPOS,
    CONFIGURATION_PROPERTY_DOCKSIZE,
    0
};

namespace framework
{

//*****************************************************************************************************************
//  Configuration access class for WindowState supplier implementation
//*****************************************************************************************************************

class ConfigurationAccess_WindowState : // interfaces
                                        public  XTypeProvider                            ,
                                        public  XNameContainer                           ,
                                        public  XContainerListener                       ,
                                        // baseclasses
                                        // Order is neccessary for right initialization!
                                        private ThreadHelpBase                           ,
                                        public  ::cppu::OWeakObject
{
    public:
                                  ConfigurationAccess_WindowState( const ::rtl::OUString& aWindowStateConfigFile, const Reference< XMultiServiceFactory >& rServiceManager );
        virtual                   ~ConfigurationAccess_WindowState();

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

        // XNameContainer
        virtual void SAL_CALL removeByName( const ::rtl::OUString& sName )
            throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException );

        virtual void SAL_CALL insertByName( const ::rtl::OUString& sName, const css::uno::Any&   aPropertySet )
            throw(css::lang::IllegalArgumentException, css::container::ElementExistException, css::lang::WrappedTargetException, css::uno::RuntimeException );

        // XNameReplace
        virtual void SAL_CALL replaceByName( const ::rtl::OUString& sName, const css::uno::Any& aPropertySet )
            throw(css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException );

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
        enum WindowStateMask
        {
            WINDOWSTATE_MASK_LOCKED         = 1,
            WINDOWSTATE_MASK_DOCKED         = 2,
            WINDOWSTATE_MASK_VISIBLE        = 4,
            WINDOWSTATE_MASK_CONTEXT        = 8,
            WINDOWSTATE_MASK_HIDEFROMMENU   = 16,
            WINDOWSTATE_MASK_NOCLOSE        = 32,
            WINDOWSTATE_MASK_SOFTCLOSE      = 64,
            WINDOWSTATE_MASK_CONTEXTACTIVE  = 128,
            WINDOWSTATE_MASK_DOCKINGAREA    = 256,
            WINDOWSTATE_MASK_POS            = 512,
            WINDOWSTATE_MASK_SIZE           = 1024,
            WINDOWSTATE_MASK_UINAME         = 2048,
            WINDOWSTATE_MASK_INTERNALSTATE  = 4096,
            WINDOWSTATE_MASK_STYLE          = 8192,
            WINDOWSTATE_MASK_DOCKPOS        = 16384,
            WINDOWSTATE_MASK_DOCKSIZE       = 32768
        };

        // Cache structure. Valid values are described by tje eMask member. All other values should not be
        // provided to outside code!
        struct WindowStateInfo
        {
            WindowStateInfo() : aDockingArea( ::com::sun::star::ui::DockingArea_DOCKINGAREA_TOP ),
                                aDockPos( 0, 0 ),
                                aPos( 0, 0 ),
                                aSize( 0, 0 ),
                                nInternalState( 0 ),
                                nStyle( 0 ),
                                nMask( 0 ) {}

            bool                                    bLocked : 1,
                                                    bDocked : 1,
                                                    bVisible : 1,
                                                    bContext : 1,
                                                    bHideFromMenu : 1,
                                                    bNoClose : 1,
                                                    bSoftClose : 1,
                                                    bContextActive : 1;
            ::com::sun::star::ui::DockingArea       aDockingArea;
            com::sun::star::awt::Point              aDockPos;
            com::sun::star::awt::Size               aDockSize;
            com::sun::star::awt::Point              aPos;
            com::sun::star::awt::Size               aSize;
            rtl::OUString                           aUIName;
            sal_uInt32                              nInternalState;
            sal_uInt16                              nStyle;
            sal_uInt32                              nMask; // see WindowStateMask
        };

        void                      impl_putPropertiesFromStruct( const WindowStateInfo& rWinStateInfo, Reference< XPropertySet >& xPropSet );
        Any                       impl_insertCacheAndReturnSequence( const rtl::OUString& rResourceURL, Reference< XNameAccess >& rNameAccess );
        WindowStateInfo&          impl_insertCacheAndReturnWinState( const rtl::OUString& rResourceURL, Reference< XNameAccess >& rNameAccess );
        Any                       impl_getSequenceFromStruct( const WindowStateInfo& rWinStateInfo );
        void                      impl_fillStructFromSequence( WindowStateInfo& rWinStateInfo, const Sequence< PropertyValue >& rSeq );
        Any                       impl_getWindowStateFromResourceURL( const rtl::OUString& rResourceURL );
        sal_Bool                  impl_initializeConfigAccess();

    private:
        typedef ::boost::unordered_map< ::rtl::OUString,
                                 WindowStateInfo,
                                 OUStringHashCode,
                                 ::std::equal_to< ::rtl::OUString > > ResourceURLToInfoCache;

        rtl::OUString                     m_aConfigWindowAccess;
        Reference< XMultiServiceFactory > m_xServiceManager;
        Reference< XMultiServiceFactory > m_xConfigProvider;
        Reference< XNameAccess >          m_xConfigAccess;
        Reference< XContainerListener >   m_xConfigListener;
        ResourceURLToInfoCache            m_aResourceURLToInfoCache;
        sal_Bool                          m_bConfigAccessInitialized : 1,
                                          m_bModified : 1;
        std::vector< ::rtl::OUString >           m_aPropArray;
};

//*****************************************************************************************************************
//  XInterface, XTypeProvider
//*****************************************************************************************************************
DEFINE_XINTERFACE_7     (   ConfigurationAccess_WindowState                                                   ,
                            OWeakObject                                                                     ,
                            DIRECT_INTERFACE ( css::container::XNameContainer                               ),
                            DIRECT_INTERFACE ( css::container::XContainerListener                           ),
                            DIRECT_INTERFACE ( css::lang::XTypeProvider                                     ),
                            DERIVED_INTERFACE( css::container::XElementAccess, css::container::XNameAccess  ),
                            DERIVED_INTERFACE( css::container::XNameAccess, css::container::XNameReplace    ),
                            DERIVED_INTERFACE( css::container::XNameReplace, css::container::XNameContainer ),
                            DERIVED_INTERFACE( css::lang::XEventListener, XContainerListener                )
                        )

DEFINE_XTYPEPROVIDER_7  (   ConfigurationAccess_WindowState         ,
                            css::container::XNameContainer          ,
                            css::container::XNameReplace            ,
                            css::container::XNameAccess             ,
                            css::container::XElementAccess          ,
                            css::container::XContainerListener      ,
                            css::lang::XEventListener               ,
                            css::lang::XTypeProvider
                        )

ConfigurationAccess_WindowState::ConfigurationAccess_WindowState( const rtl::OUString& aModuleName, const Reference< XMultiServiceFactory >& rServiceManager ) :
    ThreadHelpBase(),
    m_aConfigWindowAccess( CONFIGURATION_ROOT_ACCESS ),
    m_xServiceManager( rServiceManager ),
    m_bConfigAccessInitialized( sal_False ),
    m_bModified( sal_False )
{
    // Create configuration hierachical access name
    m_aConfigWindowAccess += aModuleName;
    m_aConfigWindowAccess += rtl::OUString( CONFIGURATION_WINDOWSTATE_ACCESS );
    m_xConfigProvider = Reference< XMultiServiceFactory >( rServiceManager->createInstance( SERVICENAME_CFGPROVIDER ), UNO_QUERY );

    // Initialize access array with property names.
    sal_Int32 n = 0;
    while ( CONFIGURATION_PROPERTIES[n] )
    {
        m_aPropArray.push_back( ::rtl::OUString::createFromAscii( CONFIGURATION_PROPERTIES[n] ));
        ++n;
    }
}

ConfigurationAccess_WindowState::~ConfigurationAccess_WindowState()
{
    // SAFE
    ResetableGuard aLock( m_aLock );
    Reference< XContainer > xContainer( m_xConfigAccess, UNO_QUERY );
    if ( xContainer.is() )
        xContainer->removeContainerListener(m_xConfigListener);
}

// XNameAccess
Any SAL_CALL ConfigurationAccess_WindowState::getByName( const ::rtl::OUString& rResourceURL )
throw ( NoSuchElementException, WrappedTargetException, RuntimeException)
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    ResourceURLToInfoCache::const_iterator pIter = m_aResourceURLToInfoCache.find( rResourceURL );
    if ( pIter != m_aResourceURLToInfoCache.end() )
        return impl_getSequenceFromStruct( pIter->second );
    else
    {
        Any a( impl_getWindowStateFromResourceURL( rResourceURL ) );
        if ( a == Any() )
            throw NoSuchElementException();
        else
            return a;
    }
}

Sequence< ::rtl::OUString > SAL_CALL ConfigurationAccess_WindowState::getElementNames()
throw ( RuntimeException )
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    if ( !m_bConfigAccessInitialized )
    {
        impl_initializeConfigAccess();
        m_bConfigAccessInitialized = sal_True;
    }

    if ( m_xConfigAccess.is() )
        return m_xConfigAccess->getElementNames();
    else
        return Sequence< ::rtl::OUString > ();
}

sal_Bool SAL_CALL ConfigurationAccess_WindowState::hasByName( const ::rtl::OUString& rResourceURL )
throw (::com::sun::star::uno::RuntimeException)
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    ResourceURLToInfoCache::const_iterator pIter = m_aResourceURLToInfoCache.find( rResourceURL );
    if ( pIter != m_aResourceURLToInfoCache.end() )
        return sal_True;
    else
    {
        Any a( impl_getWindowStateFromResourceURL( rResourceURL ) );
        if ( a == Any() )
            return sal_False;
        else
            return sal_True;
    }
}

// XElementAccess
Type SAL_CALL ConfigurationAccess_WindowState::getElementType()
throw ( RuntimeException )
{
    return( ::getCppuType( (const Sequence< PropertyValue >*)NULL ) );
}

sal_Bool SAL_CALL ConfigurationAccess_WindowState::hasElements()
throw ( RuntimeException )
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    if ( !m_bConfigAccessInitialized )
    {
        impl_initializeConfigAccess();
        m_bConfigAccessInitialized = sal_True;
    }

    if ( m_xConfigAccess.is() )
        return m_xConfigAccess->hasElements();
    else
        return sal_False;
}

// XNameContainer
void SAL_CALL ConfigurationAccess_WindowState::removeByName( const ::rtl::OUString& rResourceURL )
throw( NoSuchElementException, WrappedTargetException, RuntimeException )
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    ResourceURLToInfoCache::iterator pIter = m_aResourceURLToInfoCache.find( rResourceURL );
    if ( pIter != m_aResourceURLToInfoCache.end() )
        m_aResourceURLToInfoCache.erase( pIter );

    if ( !m_bConfigAccessInitialized )
    {
        impl_initializeConfigAccess();
        m_bConfigAccessInitialized = sal_True;
    }

    try
    {
        // Remove must be write-through => remove element from configuration
        Reference< XNameContainer > xNameContainer( m_xConfigAccess, UNO_QUERY );
        if ( xNameContainer.is() )
        {
            aLock.unlock();

            xNameContainer->removeByName( rResourceURL );
            Reference< XChangesBatch > xFlush( m_xConfigAccess, UNO_QUERY );
            if ( xFlush.is() )
                xFlush->commitChanges();
        }
    }
    catch ( const com::sun::star::lang::WrappedTargetException& )
    {
    }
}

void SAL_CALL ConfigurationAccess_WindowState::insertByName( const ::rtl::OUString& rResourceURL, const css::uno::Any& aPropertySet )
throw( IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException )
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    Sequence< PropertyValue > aPropSet;
    if ( aPropertySet >>= aPropSet )
    {
        ResourceURLToInfoCache::const_iterator pIter = m_aResourceURLToInfoCache.find( rResourceURL );
        if ( pIter != m_aResourceURLToInfoCache.end() )
            throw ElementExistException();
        else
        {
            if ( !m_bConfigAccessInitialized )
            {
                impl_initializeConfigAccess();
                m_bConfigAccessInitialized = sal_True;
            }

            // Try to ask our configuration access
            if ( m_xConfigAccess.is() )
            {
                if ( m_xConfigAccess->hasByName( rResourceURL ) )
                    throw ElementExistException();
                else
                {
                    WindowStateInfo aWinStateInfo;
                    impl_fillStructFromSequence( aWinStateInfo, aPropSet );
                    m_aResourceURLToInfoCache.insert( ResourceURLToInfoCache::value_type( rResourceURL, aWinStateInfo ));

                    // insert must be write-through => insert element into configuration
                    Reference< XNameContainer > xNameContainer( m_xConfigAccess, UNO_QUERY );
                    if ( xNameContainer.is() )
                    {
                        Reference< XSingleServiceFactory > xFactory( m_xConfigAccess, UNO_QUERY );
                        aLock.unlock();

                        try
                        {
                            Reference< XPropertySet > xPropSet( xFactory->createInstance(), UNO_QUERY );
                            if ( xPropSet.is() )
                            {
                                Any a;
                                impl_putPropertiesFromStruct( aWinStateInfo, xPropSet );
                                a <<= xPropSet;
                                xNameContainer->insertByName( rResourceURL, a );
                                Reference< XChangesBatch > xFlush( xFactory, UNO_QUERY );
                                if ( xFlush.is() )
                                    xFlush->commitChanges();
                            }
                        }
                        catch ( const Exception& )
                        {
                        }
                    }
                }
            }
        }
    }
    else
        throw IllegalArgumentException();
}

// XNameReplace
void SAL_CALL ConfigurationAccess_WindowState::replaceByName( const ::rtl::OUString& rResourceURL, const css::uno::Any& aPropertySet )
throw( IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException )
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    Sequence< PropertyValue > aPropSet;
    if ( aPropertySet >>= aPropSet )
    {
        ResourceURLToInfoCache::iterator pIter = m_aResourceURLToInfoCache.find( rResourceURL );
        if ( pIter != m_aResourceURLToInfoCache.end() )
        {
            WindowStateInfo& rWinStateInfo = pIter->second;
            impl_fillStructFromSequence( rWinStateInfo, aPropSet );
            m_bModified = sal_True;
        }
        else
        {
            if ( !m_bConfigAccessInitialized )
            {
                impl_initializeConfigAccess();
                m_bConfigAccessInitialized = sal_True;
            }

            // Try to ask our configuration access
            Reference< XNameAccess > xNameAccess;
            Any a( m_xConfigAccess->getByName( rResourceURL ));

            if ( a >>= xNameAccess )
            {
                WindowStateInfo& rWinStateInfo( impl_insertCacheAndReturnWinState( rResourceURL, xNameAccess ));
                impl_fillStructFromSequence( rWinStateInfo, aPropSet );
                m_bModified = sal_True;
                pIter = m_aResourceURLToInfoCache.find( rResourceURL );
            }
            else
                throw NoSuchElementException();
        }

        if ( m_bModified && pIter != m_aResourceURLToInfoCache.end() )
        {
            Reference< XNameContainer > xNameContainer( m_xConfigAccess, UNO_QUERY );
            if ( xNameContainer.is() )
            {
                WindowStateInfo aWinStateInfo( pIter->second );
                ::rtl::OUString        aResourceURL( pIter->first );
                m_bModified = sal_False;
                aLock.unlock();

                try
                {
                    Reference< XPropertySet > xPropSet;
                    if ( xNameContainer->getByName( aResourceURL ) >>= xPropSet )
                    {
                        impl_putPropertiesFromStruct( aWinStateInfo, xPropSet );

                        Reference< XChangesBatch > xFlush( m_xConfigAccess, UNO_QUERY );
                        if ( xFlush.is() )
                            xFlush->commitChanges();
                    }
                }
                catch ( const Exception& )
                {
                }
            }
        }
    }
    else
        throw IllegalArgumentException();
}

// container.XContainerListener
void SAL_CALL ConfigurationAccess_WindowState::elementInserted( const ContainerEvent& ) throw(RuntimeException)
{
    // do nothing - next time someone wants to retrieve this node we will find it in the configuration
}

void SAL_CALL ConfigurationAccess_WindowState::elementRemoved ( const ContainerEvent& ) throw(RuntimeException)
{
}

void SAL_CALL ConfigurationAccess_WindowState::elementReplaced( const ContainerEvent& ) throw(RuntimeException)
{
}

// lang.XEventListener
void SAL_CALL ConfigurationAccess_WindowState::disposing( const EventObject& aEvent ) throw(RuntimeException)
{
    // SAFE
    // remove our reference to the config access
    ResetableGuard aLock( m_aLock );

    Reference< XInterface > xIfac1( aEvent.Source, UNO_QUERY );
    Reference< XInterface > xIfac2( m_xConfigAccess, UNO_QUERY );
    if ( xIfac1 == xIfac2 )
        m_xConfigAccess.clear();
}

// private helper methods
Any ConfigurationAccess_WindowState::impl_getSequenceFromStruct( const WindowStateInfo& rWinStateInfo )
{
    sal_Int32                 i( 0 );
    sal_Int32                 nCount( m_aPropArray.size() );
    Sequence< PropertyValue > aPropSeq;

    for ( i = 0; i < nCount; i++ )
    {
        if ( rWinStateInfo.nMask & ( 1 << i ))
        {
            // put value into the return sequence
            sal_Int32 nIndex( aPropSeq.getLength());
            aPropSeq.realloc( nIndex+1 );
            aPropSeq[nIndex].Name = m_aPropArray[i];

            switch ( i )
            {
                case PROPERTY_LOCKED:
                    aPropSeq[nIndex].Value = makeAny( rWinStateInfo.bLocked ); break;
                case PROPERTY_DOCKED:
                    aPropSeq[nIndex].Value = makeAny( rWinStateInfo.bDocked ); break;
                case PROPERTY_VISIBLE:
                    aPropSeq[nIndex].Value = makeAny( rWinStateInfo.bVisible ); break;
                case PROPERTY_CONTEXT:
                    aPropSeq[nIndex].Value = makeAny( rWinStateInfo.bContext ); break;
                case PROPERTY_HIDEFROMMENU:
                    aPropSeq[nIndex].Value = makeAny( rWinStateInfo.bHideFromMenu ); break;
                case PROPERTY_NOCLOSE:
                    aPropSeq[nIndex].Value = makeAny( rWinStateInfo.bNoClose ); break;
                case PROPERTY_SOFTCLOSE:
                    aPropSeq[nIndex].Value = makeAny( rWinStateInfo.bSoftClose ); break;
                case PROPERTY_CONTEXTACTIVE:
                    aPropSeq[nIndex].Value = makeAny( rWinStateInfo.bContextActive ); break;
                case PROPERTY_DOCKINGAREA:
                    aPropSeq[nIndex].Value = makeAny( rWinStateInfo.aDockingArea ); break;
                case PROPERTY_POS:
                    aPropSeq[nIndex].Value = makeAny( rWinStateInfo.aPos ); break;
                case PROPERTY_SIZE:
                    aPropSeq[nIndex].Value = makeAny( rWinStateInfo.aSize ); break;
                case PROPERTY_UINAME:
                    aPropSeq[nIndex].Value = makeAny( rWinStateInfo.aUIName ); break;
                case PROPERTY_INTERNALSTATE:
                    aPropSeq[nIndex].Value = makeAny( sal_Int32( rWinStateInfo.nInternalState )); break;
                case PROPERTY_STYLE:
                    aPropSeq[nIndex].Value = makeAny( sal_Int16( rWinStateInfo.nStyle )); break;
                case PROPERTY_DOCKPOS:
                    aPropSeq[nIndex].Value = makeAny( rWinStateInfo.aDockPos ); break;
                case PROPERTY_DOCKSIZE:
                    aPropSeq[nIndex].Value = makeAny( rWinStateInfo.aDockSize ); break;
                default:
                    DBG_ASSERT( sal_False, "Wrong value for ConfigurationAccess_WindowState. Who has forgotten to add this new property!" );
            }
        }
    }

    return makeAny( aPropSeq );
}

Any ConfigurationAccess_WindowState::impl_insertCacheAndReturnSequence( const rtl::OUString& rResourceURL, Reference< XNameAccess >& xNameAccess )
{
    sal_Int32                 nMask( 0 );
    sal_Int32                 nCount( m_aPropArray.size() );
    sal_Int32                 i( 0 );
    sal_Int32                 nIndex( 0 );
    Sequence< PropertyValue > aPropSeq;
    WindowStateInfo           aWindowStateInfo;

    for ( i = 0; i < nCount; i++ )
    {
        try
        {
            bool    bAddToSeq( false );
            Any     a( xNameAccess->getByName( m_aPropArray[i] ) );
            switch ( i )
            {
                case PROPERTY_LOCKED:
                case PROPERTY_DOCKED:
                case PROPERTY_VISIBLE:
                case PROPERTY_CONTEXT:
                case PROPERTY_HIDEFROMMENU:
                case PROPERTY_NOCLOSE:
                case PROPERTY_SOFTCLOSE:
                case PROPERTY_CONTEXTACTIVE:
                {
                    sal_Bool bValue = sal_Bool();
                    if ( a >>= bValue )
                    {
                        sal_Int32 nValue( 1 << i );
                        nMask |= nValue;
                        bAddToSeq = true;
                        switch ( i )
                        {
                            case PROPERTY_LOCKED:
                                aWindowStateInfo.bLocked = bValue; break;
                            case PROPERTY_DOCKED:
                                aWindowStateInfo.bDocked = bValue; break;
                            case PROPERTY_VISIBLE:
                                aWindowStateInfo.bVisible = bValue; break;
                            case PROPERTY_CONTEXT:
                                aWindowStateInfo.bContext = bValue; break;
                            case PROPERTY_HIDEFROMMENU:
                                aWindowStateInfo.bHideFromMenu = bValue; break;
                            case PROPERTY_NOCLOSE:
                                aWindowStateInfo.bNoClose = bValue; break;
                            case PROPERTY_SOFTCLOSE:
                                aWindowStateInfo.bSoftClose = bValue; break;
                            case PROPERTY_CONTEXTACTIVE:
                                aWindowStateInfo.bContextActive = bValue; break;
                        }
                    }
                }
                break;

                case PROPERTY_DOCKINGAREA:
                {
                    sal_Int32 nDockingArea = 0;
                    if ( a >>= nDockingArea )
                    {
                        if (( nDockingArea >= 0 ) &&
                            ( nDockingArea <= sal_Int32( DockingArea_DOCKINGAREA_RIGHT )))
                        {
                            aWindowStateInfo.aDockingArea = (DockingArea)nDockingArea;
                            nMask |= WINDOWSTATE_MASK_DOCKINGAREA;
                            a = makeAny( aWindowStateInfo.aDockingArea );
                            bAddToSeq = true;
                        }
                    }
                }
                break;

                case PROPERTY_POS:
                case PROPERTY_DOCKPOS:
                {
                    ::rtl::OUString aString;
                    if ( a >>= aString )
                    {
                        sal_Int32 nToken( 0 );
                        ::rtl::OUString aXStr = aString.getToken( 0, ',', nToken );
                        if ( nToken > 0 )
                        {
                            com::sun::star::awt::Point aPos;
                            aPos.X = aXStr.toInt32();
                            aPos.Y = aString.getToken( 0, ',', nToken ).toInt32();

                            if ( i == PROPERTY_POS )
                            {
                                aWindowStateInfo.aPos = aPos;
                                nMask |= WINDOWSTATE_MASK_POS;
                            }
                            else
                            {
                                aWindowStateInfo.aDockPos = aPos;
                                nMask |= WINDOWSTATE_MASK_DOCKPOS;
                            }

                            a <<= aPos;
                            bAddToSeq = true;
                        }
                    }
                }
                break;

                case PROPERTY_SIZE:
                case PROPERTY_DOCKSIZE:
                {
                    ::rtl::OUString aString;
                    if ( a >>= aString )
                    {
                        sal_Int32 nToken( 0 );
                        ::rtl::OUString aStr = aString.getToken( 0, ',', nToken );
                        if ( nToken > 0 )
                        {
                            com::sun::star::awt::Size aSize;
                            aSize.Width = aStr.toInt32();
                            aSize.Height = aString.getToken( 0, ',', nToken ).toInt32();
                            if ( i == PROPERTY_SIZE )
                            {
                                aWindowStateInfo.aSize = aSize;
                                nMask |= WINDOWSTATE_MASK_SIZE;
                            }
                            else
                            {
                                aWindowStateInfo.aDockSize = aSize;
                                nMask |= WINDOWSTATE_MASK_DOCKSIZE;
                            }

                            a <<= aSize;
                            bAddToSeq = true;
                        }
                    }
                }
                break;

                case PROPERTY_UINAME:
                {
                    ::rtl::OUString aValue;
                    if ( a >>= aValue )
                    {
                        nMask |= WINDOWSTATE_MASK_UINAME;
                        aWindowStateInfo.aUIName = aValue;
                        bAddToSeq = true;
                    }
                }
                break;

                case PROPERTY_INTERNALSTATE:
                {
                    sal_uInt32 nValue = 0;
                    if ( a >>= nValue )
                    {
                        nMask |= WINDOWSTATE_MASK_INTERNALSTATE;
                        aWindowStateInfo.nInternalState = nValue;
                        bAddToSeq = true;
                    }
                }
                break;

                case PROPERTY_STYLE:
                {
                    sal_Int32 nValue = 0;
                    if ( a >>= nValue )
                    {
                        nMask |= WINDOWSTATE_MASK_STYLE;
                        aWindowStateInfo.nStyle = sal_uInt16( nValue );
                        bAddToSeq = true;
                    }
                }
                break;

                default:
                    DBG_ASSERT( sal_False, "Wrong value for ConfigurationAccess_WindowState. Who has forgotten to add this new property!" );
            }

            if ( bAddToSeq )
            {
                // put value into the return sequence
                nIndex = aPropSeq.getLength();
                aPropSeq.realloc( nIndex+1 );
                aPropSeq[nIndex].Name  = m_aPropArray[i];
                aPropSeq[nIndex].Value = a;
            }
        }
        catch( const com::sun::star::container::NoSuchElementException& )
        {
        }
        catch ( const com::sun::star::lang::WrappedTargetException& )
        {
        }
    }

    aWindowStateInfo.nMask = nMask;
    m_aResourceURLToInfoCache.insert( ResourceURLToInfoCache::value_type( rResourceURL, aWindowStateInfo ));
    return makeAny( aPropSeq );
}

ConfigurationAccess_WindowState::WindowStateInfo& ConfigurationAccess_WindowState::impl_insertCacheAndReturnWinState( const rtl::OUString& rResourceURL, Reference< XNameAccess >& rNameAccess )
{
    sal_Int32                 nMask( 0 );
    sal_Int32                 nCount( m_aPropArray.size() );
    sal_Int32                 i( 0 );
    Sequence< PropertyValue > aPropSeq;
    WindowStateInfo           aWindowStateInfo;

    for ( i = 0; i < nCount; i++ )
    {
        try
        {
            Any     a( rNameAccess->getByName( m_aPropArray[i] ) );
            switch ( i )
            {
                case PROPERTY_LOCKED:
                case PROPERTY_DOCKED:
                case PROPERTY_VISIBLE:
                case PROPERTY_CONTEXT:
                case PROPERTY_HIDEFROMMENU:
                case PROPERTY_NOCLOSE:
                case PROPERTY_SOFTCLOSE:
                case PROPERTY_CONTEXTACTIVE:
                {
                    sal_Bool bValue = sal_Bool();
                    if ( a >>= bValue )
                    {
                        sal_Int32 nValue( 1 << i );
                        nMask |= nValue;
                        switch ( i )
                        {
                            case PROPERTY_LOCKED:
                                aWindowStateInfo.bLocked = bValue; break;
                            case PROPERTY_DOCKED:
                                aWindowStateInfo.bDocked = bValue; break;
                            case PROPERTY_VISIBLE:
                                aWindowStateInfo.bVisible = bValue; break;
                            case PROPERTY_CONTEXT:
                                aWindowStateInfo.bContext = bValue; break;
                            case PROPERTY_HIDEFROMMENU:
                                aWindowStateInfo.bHideFromMenu = bValue; break;
                            case PROPERTY_NOCLOSE:
                                aWindowStateInfo.bNoClose = bValue; break;
                            case PROPERTY_SOFTCLOSE:
                                aWindowStateInfo.bNoClose = bValue; break;
                            case PROPERTY_CONTEXTACTIVE:
                                aWindowStateInfo.bContextActive = bValue; break;
                            default:
                                DBG_ASSERT( sal_False, "Unknown boolean property in WindowState found!" );
                        }
                    }
                }
                break;

                case PROPERTY_DOCKINGAREA:
                {
                    sal_Int32 nDockingArea = 0;
                    if ( a >>= nDockingArea )
                    {
                        if (( nDockingArea >= 0 ) &&
                            ( nDockingArea <= sal_Int32( DockingArea_DOCKINGAREA_RIGHT )))
                        {
                            aWindowStateInfo.aDockingArea = (DockingArea)nDockingArea;
                            nMask |= WINDOWSTATE_MASK_DOCKINGAREA;
                        }
                    }
                }
                break;

                case PROPERTY_POS:
                case PROPERTY_DOCKPOS:
                {
                    ::rtl::OUString aString;
                    if ( a >>= aString )
                    {
                        sal_Int32 nToken( 0 );
                        ::rtl::OUString aXStr = aString.getToken( 0, ',', nToken );
                        if ( nToken > 0 )
                        {
                            com::sun::star::awt::Point aPos;
                            aPos.X = aXStr.toInt32();
                            aPos.Y = aString.getToken( 0, ',', nToken ).toInt32();

                            if ( i == PROPERTY_POS )
                            {
                                aWindowStateInfo.aPos = aPos;
                                nMask |= WINDOWSTATE_MASK_POS;
                            }
                            else
                            {
                                aWindowStateInfo.aDockPos = aPos;
                                nMask |= WINDOWSTATE_MASK_DOCKPOS;
                            }
                        }
                    }
                }
                break;

                case PROPERTY_SIZE:
                case PROPERTY_DOCKSIZE:
                {
                    ::rtl::OUString aString;
                    if ( a >>= aString )
                    {
                        sal_Int32 nToken( 0 );
                        ::rtl::OUString aStr = aString.getToken( 0, ',', nToken );
                        if ( nToken > 0 )
                        {
                            com::sun::star::awt::Size aSize;
                            aSize.Width  = aStr.toInt32();
                            aSize.Height = aString.getToken( 0, ',', nToken ).toInt32();
                            if ( i == PROPERTY_SIZE )
                            {
                                aWindowStateInfo.aSize = aSize;
                                nMask |= WINDOWSTATE_MASK_SIZE;
                            }
                            else
                            {
                                aWindowStateInfo.aDockSize = aSize;
                                nMask |= WINDOWSTATE_MASK_DOCKSIZE;
                            }
                        }
                    }
                }
                break;

                case PROPERTY_UINAME:
                {
                    ::rtl::OUString aValue;
                    if ( a >>= aValue )
                    {
                        nMask |= WINDOWSTATE_MASK_UINAME;
                        aWindowStateInfo.aUIName = aValue;
                    }
                }
                break;

                case PROPERTY_INTERNALSTATE:
                {
                    sal_Int32 nValue = 0;
                    if ( a >>= nValue )
                    {
                        nMask |= WINDOWSTATE_MASK_INTERNALSTATE;
                        aWindowStateInfo.nInternalState = sal_uInt32( nValue );
                    }
                }
                break;

                case PROPERTY_STYLE:
                {
                    sal_Int32 nValue = 0;
                    if ( a >>= nValue )
                    {
                        nMask |= WINDOWSTATE_MASK_STYLE;
                        aWindowStateInfo.nStyle = sal_uInt16( nValue );
                    }
                }

                default:
                    DBG_ASSERT( sal_False, "Wrong value for ConfigurationAccess_WindowState. Who has forgotten to add this new property!" );
            }
        }
        catch( const com::sun::star::container::NoSuchElementException& )
        {
        }
        catch ( const com::sun::star::lang::WrappedTargetException& )
        {
        }
    }

    aWindowStateInfo.nMask = nMask;
    ResourceURLToInfoCache::iterator pIter = (m_aResourceURLToInfoCache.insert( ResourceURLToInfoCache::value_type( rResourceURL, aWindowStateInfo ))).first;
    return pIter->second;
}

Any ConfigurationAccess_WindowState::impl_getWindowStateFromResourceURL( const rtl::OUString& rResourceURL )
{
    if ( !m_bConfigAccessInitialized )
    {
        impl_initializeConfigAccess();
        m_bConfigAccessInitialized = sal_True;
    }

    try
    {
        // Try to ask our configuration access
        if ( m_xConfigAccess.is() && m_xConfigAccess->hasByName( rResourceURL ) )
        {

            Reference< XNameAccess > xNameAccess( m_xConfigAccess->getByName( rResourceURL ), UNO_QUERY );
            if ( xNameAccess.is() )
                return impl_insertCacheAndReturnSequence( rResourceURL, xNameAccess );
        }
    }
    catch( const com::sun::star::container::NoSuchElementException& )
    {
    }
    catch ( const com::sun::star::lang::WrappedTargetException& )
    {
    }

    return Any();
}

void ConfigurationAccess_WindowState::impl_fillStructFromSequence( WindowStateInfo& rWinStateInfo, const Sequence< PropertyValue >& rSeq )
{
    sal_Int32 nCompareCount( m_aPropArray.size() );
    sal_Int32 nCount( rSeq.getLength() );
    sal_Int32 i( 0 );

    for ( i = 0; i < nCount; i++ )
    {
        for ( sal_Int32 j = 0; j < nCompareCount; j++ )
        {
            if ( rSeq[i].Name.equals( m_aPropArray[j] ))
            {
                switch ( j )
                {
                    case PROPERTY_LOCKED:
                    case PROPERTY_DOCKED:
                    case PROPERTY_VISIBLE:
                    case PROPERTY_CONTEXT:
                    case PROPERTY_HIDEFROMMENU:
                    case PROPERTY_NOCLOSE:
                    case PROPERTY_SOFTCLOSE:
                    case PROPERTY_CONTEXTACTIVE:
                    {
                        sal_Bool bValue = sal_Bool();
                        if ( rSeq[i].Value >>= bValue )
                        {
                            sal_Int32 nValue( 1 << j );
                            rWinStateInfo.nMask |= nValue;
                            switch ( j )
                            {
                                case PROPERTY_LOCKED:
                                    rWinStateInfo.bLocked = bValue;
                                    break;
                                case PROPERTY_DOCKED:
                                    rWinStateInfo.bDocked = bValue;
                                    break;
                                case PROPERTY_VISIBLE:
                                    rWinStateInfo.bVisible = bValue;
                                    break;
                                case PROPERTY_CONTEXT:
                                    rWinStateInfo.bContext = bValue;
                                    break;
                                case PROPERTY_HIDEFROMMENU:
                                    rWinStateInfo.bHideFromMenu = bValue;
                                    break;
                                case PROPERTY_NOCLOSE:
                                    rWinStateInfo.bNoClose = bValue;
                                    break;
                                case PROPERTY_SOFTCLOSE:
                                    rWinStateInfo.bSoftClose = bValue;
                                    break;
                                case PROPERTY_CONTEXTACTIVE:
                                    rWinStateInfo.bContextActive = bValue;
                                    break;
                                default:
                                    DBG_ASSERT( sal_False, "Unknown boolean property in WindowState found!" );
                            }
                        }
                    }
                    break;

                    case PROPERTY_DOCKINGAREA:
                    {
                        ::com::sun::star::ui::DockingArea eDockingArea;
                        if ( rSeq[i].Value >>= eDockingArea )
                        {
                            rWinStateInfo.aDockingArea = eDockingArea;
                            rWinStateInfo.nMask |= WINDOWSTATE_MASK_DOCKINGAREA;
                        }
                    }
                    break;

                    case PROPERTY_POS:
                    case PROPERTY_DOCKPOS:
                    {
                        com::sun::star::awt::Point aPoint;
                        if ( rSeq[i].Value >>= aPoint )
                        {
                            if ( j == PROPERTY_POS )
                            {
                                rWinStateInfo.aPos = aPoint;
                                rWinStateInfo.nMask |= WINDOWSTATE_MASK_POS;
                            }
                            else
                            {
                                rWinStateInfo.aDockPos = aPoint;
                                rWinStateInfo.nMask |= WINDOWSTATE_MASK_DOCKPOS;
                            }
                        }
                    }
                    break;

                    case PROPERTY_SIZE:
                    case PROPERTY_DOCKSIZE:
                    {
                        com::sun::star::awt::Size aSize;
                        if ( rSeq[i].Value >>= aSize )
                        {
                            if ( j == PROPERTY_SIZE )
                            {
                                rWinStateInfo.aSize = aSize;
                                rWinStateInfo.nMask |= WINDOWSTATE_MASK_SIZE;
                            }
                            else
                            {
                                rWinStateInfo.aDockSize = aSize;
                                rWinStateInfo.nMask |= WINDOWSTATE_MASK_DOCKSIZE;
                            }
                        }
                    }
                    break;

                    case PROPERTY_UINAME:
                    {
                        ::rtl::OUString aValue;
                        if ( rSeq[i].Value >>= aValue )
                        {
                            rWinStateInfo.aUIName = aValue;
                            rWinStateInfo.nMask |= WINDOWSTATE_MASK_UINAME;
                        }
                    }
                    break;

                    case PROPERTY_INTERNALSTATE:
                    {
                        sal_Int32 nValue = 0;
                        if ( rSeq[i].Value >>= nValue )
                        {
                            rWinStateInfo.nInternalState = sal_uInt32( nValue );
                            rWinStateInfo.nMask |= WINDOWSTATE_MASK_INTERNALSTATE;
                        }
                    }
                    break;

                    case PROPERTY_STYLE:
                    {
                        sal_Int32 nValue = 0;
                        if ( rSeq[i].Value >>= nValue )
                        {
                            rWinStateInfo.nStyle = sal_uInt16( nValue );
                            rWinStateInfo.nMask |= WINDOWSTATE_MASK_STYLE;
                        }
                    }
                    break;

                    default:
                        DBG_ASSERT( sal_False, "Wrong value for ConfigurationAccess_WindowState. Who has forgotten to add this new property!" );
                }

                break;
            }
        }
    }
}

void ConfigurationAccess_WindowState::impl_putPropertiesFromStruct( const WindowStateInfo& rWinStateInfo, Reference< XPropertySet >& xPropSet )
{
    sal_Int32                 i( 0 );
    sal_Int32                 nCount( m_aPropArray.size() );
    Sequence< PropertyValue > aPropSeq;
    ::rtl::OUString                  aDelim( "," );

    for ( i = 0; i < nCount; i++ )
    {
        if ( rWinStateInfo.nMask & ( 1 << i ))
        {
            try
            {
                // put values into the property set
                switch ( i )
                {
                    case PROPERTY_LOCKED:
                        xPropSet->setPropertyValue( m_aPropArray[i], makeAny( sal_Bool( rWinStateInfo.bLocked )) ); break;
                    case PROPERTY_DOCKED:
                        xPropSet->setPropertyValue( m_aPropArray[i], makeAny( sal_Bool( rWinStateInfo.bDocked )) ); break;
                    case PROPERTY_VISIBLE:
                        xPropSet->setPropertyValue( m_aPropArray[i], makeAny( sal_Bool( rWinStateInfo.bVisible )) ); break;
                    case PROPERTY_CONTEXT:
                        xPropSet->setPropertyValue( m_aPropArray[i], makeAny( sal_Bool( rWinStateInfo.bContext )) ); break;
                    case PROPERTY_HIDEFROMMENU:
                        xPropSet->setPropertyValue( m_aPropArray[i], makeAny( sal_Bool( rWinStateInfo.bHideFromMenu )) ); break;
                    case PROPERTY_NOCLOSE:
                        xPropSet->setPropertyValue( m_aPropArray[i], makeAny( sal_Bool( rWinStateInfo.bNoClose )) ); break;
                    case PROPERTY_SOFTCLOSE:
                        xPropSet->setPropertyValue( m_aPropArray[i], makeAny( sal_Bool( rWinStateInfo.bSoftClose )) ); break;
                    case PROPERTY_CONTEXTACTIVE:
                        xPropSet->setPropertyValue( m_aPropArray[i], makeAny( sal_Bool( rWinStateInfo.bContextActive )) ); break;
                    case PROPERTY_DOCKINGAREA:
                        xPropSet->setPropertyValue( m_aPropArray[i], makeAny( sal_Int16( rWinStateInfo.aDockingArea ) ) ); break;
                    case PROPERTY_POS:
                    case PROPERTY_DOCKPOS:
                    {
                        ::rtl::OUString aPosStr;
                        if ( i == PROPERTY_POS )
                            aPosStr = ::rtl::OUString::valueOf( rWinStateInfo.aPos.X );
                        else
                            aPosStr = ::rtl::OUString::valueOf( rWinStateInfo.aDockPos.X );
                        aPosStr += aDelim;
                        if ( i == PROPERTY_POS )
                            aPosStr += ::rtl::OUString::valueOf( rWinStateInfo.aPos.Y );
                        else
                            aPosStr += ::rtl::OUString::valueOf( rWinStateInfo.aDockPos.Y );
                        xPropSet->setPropertyValue( m_aPropArray[i], makeAny( aPosStr ) );
                        break;
                    }
                    case PROPERTY_SIZE:
                    case PROPERTY_DOCKSIZE:
                    {
                        ::rtl::OUString aSizeStr;
                        if ( i == PROPERTY_SIZE )
                            aSizeStr = ( ::rtl::OUString::valueOf( rWinStateInfo.aSize.Width ));
                        else
                            aSizeStr = ( ::rtl::OUString::valueOf( rWinStateInfo.aDockSize.Width ));
                        aSizeStr += aDelim;
                        if ( i == PROPERTY_SIZE )
                            aSizeStr += ::rtl::OUString::valueOf( rWinStateInfo.aSize.Height );
                        else
                            aSizeStr += ::rtl::OUString::valueOf( rWinStateInfo.aDockSize.Height );
                        xPropSet->setPropertyValue( m_aPropArray[i], makeAny( aSizeStr ) );
                        break;
                    }
                    case PROPERTY_UINAME:
                        xPropSet->setPropertyValue( m_aPropArray[i], makeAny( rWinStateInfo.aUIName ) ); break;
                    case PROPERTY_INTERNALSTATE:
                        xPropSet->setPropertyValue( m_aPropArray[i], makeAny( sal_Int32( rWinStateInfo.nInternalState )) ); break;
                    case PROPERTY_STYLE:
                        xPropSet->setPropertyValue( m_aPropArray[i], makeAny( sal_Int32( rWinStateInfo.nStyle )) ); break;
                    default:
                        DBG_ASSERT( sal_False, "Wrong value for ConfigurationAccess_WindowState. Who has forgotten to add this new property!" );
                }
            }
            catch( const Exception& )
            {
            }
        }
    }
}

sal_Bool ConfigurationAccess_WindowState::impl_initializeConfigAccess()
{
    Sequence< Any > aArgs( 2 );
    PropertyValue   aPropValue;

    try
    {
        aPropValue.Name  = rtl::OUString( "nodepath" );
        aPropValue.Value <<= m_aConfigWindowAccess;
        aArgs[0] <<= aPropValue;
        aPropValue.Name = rtl::OUString( "lazywrite" );
        aPropValue.Value <<= sal_True;
        aArgs[1] <<= aPropValue;

        m_xConfigAccess = Reference< XNameAccess >( m_xConfigProvider->createInstanceWithArguments(
                                                        SERVICENAME_CFGUPDATEACCESS, aArgs ),
                                                    UNO_QUERY );
        if ( m_xConfigAccess.is() )
        {
            // Add as container listener
            Reference< XContainer > xContainer( m_xConfigAccess, UNO_QUERY );
            if ( xContainer.is() )
            {
                m_xConfigListener = new WeakContainerListener(this);
                xContainer->addContainerListener(m_xConfigListener);
            }
        }

        return sal_True;
    }
    catch ( const WrappedTargetException& )
    {
    }
    catch ( const Exception& )
    {
    }

    return sal_False;
}


//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XINTERFACE_4                    (    WindowStateConfiguration                                                            ,
                                            OWeakObject                                                                     ,
                                            DIRECT_INTERFACE( css::lang::XTypeProvider                                      ),
                                            DIRECT_INTERFACE( css::lang::XServiceInfo                                       ),
                                            DIRECT_INTERFACE( css::container::XNameAccess                                   ),
                                            DERIVED_INTERFACE( css::container::XElementAccess, css::container::XNameAccess  )
                                        )

DEFINE_XTYPEPROVIDER_4                  (   WindowStateConfiguration            ,
                                            css::lang::XTypeProvider        ,
                                            css::lang::XServiceInfo         ,
                                            css::container::XNameAccess     ,
                                            css::container::XElementAccess
                                        )

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   WindowStateConfiguration                    ,
                                            ::cppu::OWeakObject                         ,
                                            SERVICENAME_WINDOWSTATECONFIGURATION        ,
                                            IMPLEMENTATIONNAME_WINDOWSTATECONFIGURATION
                                        )

DEFINE_INIT_SERVICE                     (   WindowStateConfiguration, {} )

WindowStateConfiguration::WindowStateConfiguration( const Reference< XMultiServiceFactory >& xServiceManager ) :
    ThreadHelpBase(),
    m_xServiceManager( xServiceManager )
{
    m_xModuleManager = ModuleManager::create( comphelper::getComponentContext(m_xServiceManager) );
    Reference< XNameAccess > xEmptyNameAccess;
    Sequence< rtl::OUString > aElementNames;
    try
    {
        aElementNames = m_xModuleManager->getElementNames();
    }
    catch (const ::com::sun::star::uno::RuntimeException &)
    {
    }
    Sequence< PropertyValue > aSeq;
    ::rtl::OUString                  aModuleIdentifier;

    for ( sal_Int32 i = 0; i < aElementNames.getLength(); i++ )
    {
        aModuleIdentifier = aElementNames[i];
        if ( m_xModuleManager->getByName( aModuleIdentifier ) >>= aSeq )
        {
            ::rtl::OUString aWindowStateFileStr;
            for ( sal_Int32 y = 0; y < aSeq.getLength(); y++ )
            {
                if ( aSeq[y].Name == "ooSetupFactoryWindowStateConfigRef" )
                {
                    aSeq[y].Value >>= aWindowStateFileStr;
                    break;
                }
            }

            if ( !aWindowStateFileStr.isEmpty() )
            {
                // Create first mapping ModuleIdentifier ==> Window state configuration file
                m_aModuleToFileHashMap.insert( ModuleToWindowStateFileMap::value_type( aModuleIdentifier, aWindowStateFileStr ));

                // Create second mapping Command File ==> Window state configuration instance
                ModuleToWindowStateConfigHashMap::iterator pIter = m_aModuleToWindowStateHashMap.find( aWindowStateFileStr );
                if ( pIter == m_aModuleToWindowStateHashMap.end() )
                    m_aModuleToWindowStateHashMap.insert( ModuleToWindowStateConfigHashMap::value_type( aWindowStateFileStr, xEmptyNameAccess ));
            }
        }
    }
}

WindowStateConfiguration::~WindowStateConfiguration()
{
    ResetableGuard aLock( m_aLock );
    m_aModuleToFileHashMap.clear();
    m_aModuleToWindowStateHashMap.clear();
}

Any SAL_CALL WindowStateConfiguration::getByName( const ::rtl::OUString& aModuleIdentifier )
throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    ModuleToWindowStateFileMap::const_iterator pIter = m_aModuleToFileHashMap.find( aModuleIdentifier );
    if ( pIter != m_aModuleToFileHashMap.end() )
    {
        Any a;
        ::rtl::OUString aWindowStateConfigFile( pIter->second );

        ModuleToWindowStateConfigHashMap::iterator pModuleIter = m_aModuleToWindowStateHashMap.find( aWindowStateConfigFile );
        if ( pModuleIter != m_aModuleToWindowStateHashMap.end() )
        {
            if ( pModuleIter->second.is() )
                a = makeAny( pModuleIter->second );
            else
            {
                Reference< XNameAccess > xResourceURLWindowState;
                ConfigurationAccess_WindowState* pModuleWindowState = new ConfigurationAccess_WindowState( aWindowStateConfigFile, m_xServiceManager );
                xResourceURLWindowState = Reference< XNameAccess >( static_cast< cppu::OWeakObject* >( pModuleWindowState ),UNO_QUERY );
                pModuleIter->second = xResourceURLWindowState;
                a <<= xResourceURLWindowState;
            }

            return a;
        }
    }

    throw NoSuchElementException();
}

Sequence< ::rtl::OUString > SAL_CALL WindowStateConfiguration::getElementNames()
throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    Sequence< rtl::OUString > aSeq( m_aModuleToFileHashMap.size() );

    sal_Int32 n = 0;
    ModuleToWindowStateFileMap::const_iterator pIter = m_aModuleToFileHashMap.begin();
    while ( pIter != m_aModuleToFileHashMap.end() )
    {
        aSeq[n] = pIter->first;
        ++pIter;
    }

    return aSeq;
}

sal_Bool SAL_CALL WindowStateConfiguration::hasByName( const ::rtl::OUString& aName )
throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    ModuleToWindowStateFileMap::const_iterator pIter = m_aModuleToFileHashMap.find( aName );
    return ( pIter != m_aModuleToFileHashMap.end() );
}

// XElementAccess
Type SAL_CALL WindowStateConfiguration::getElementType()
throw (::com::sun::star::uno::RuntimeException)
{
    return( ::getCppuType( (const Reference< XNameAccess >*)NULL ) );
}

sal_Bool SAL_CALL WindowStateConfiguration::hasElements()
throw (::com::sun::star::uno::RuntimeException)
{
    // We always have at least one module. So it is valid to return true!
    return sal_True;
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
