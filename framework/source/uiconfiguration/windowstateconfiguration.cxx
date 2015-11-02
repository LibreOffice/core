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

#include <uiconfiguration/windowstateproperties.hxx>
#include <helper/mischelper.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XModuleManager2.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/ui/DockingArea.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <tools/debug.hxx>

#include <unordered_map>
#include <vector>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace com::sun::star::configuration;
using namespace com::sun::star::container;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::ui;
using namespace framework;

#undef WINDOWSTATE_MASK_POS

namespace {

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
    WINDOWSTATE_PROPERTY_LOCKED,
    WINDOWSTATE_PROPERTY_DOCKED,
    WINDOWSTATE_PROPERTY_VISIBLE,
    WINDOWSTATE_PROPERTY_CONTEXT,
    WINDOWSTATE_PROPERTY_HIDEFROMENU,
    WINDOWSTATE_PROPERTY_NOCLOSE,
    WINDOWSTATE_PROPERTY_SOFTCLOSE,
    WINDOWSTATE_PROPERTY_CONTEXTACTIVE,
    WINDOWSTATE_PROPERTY_DOCKINGAREA,
    WINDOWSTATE_PROPERTY_POS,
    WINDOWSTATE_PROPERTY_SIZE,
    WINDOWSTATE_PROPERTY_UINAME,
    WINDOWSTATE_PROPERTY_INTERNALSTATE,
    WINDOWSTATE_PROPERTY_STYLE,
    WINDOWSTATE_PROPERTY_DOCKPOS,
    WINDOWSTATE_PROPERTY_DOCKSIZE,
    0
};

//  Configuration access class for WindowState supplier implementation

class ConfigurationAccess_WindowState : public  ::cppu::WeakImplHelper< XNameContainer, XContainerListener >
{
    public:
                                  ConfigurationAccess_WindowState( const OUString& aWindowStateConfigFile, const Reference< XComponentContext >& rxContext );
        virtual                   ~ConfigurationAccess_WindowState();

        // XNameAccess
        virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
            throw (css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

        virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
            throw (css::uno::RuntimeException, std::exception) override;

        virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
            throw (css::uno::RuntimeException, std::exception) override;

        // XNameContainer
        virtual void SAL_CALL removeByName( const OUString& sName )
            throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL insertByName( const OUString& sName, const css::uno::Any&   aPropertySet )
            throw(css::lang::IllegalArgumentException, css::container::ElementExistException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;

        // XNameReplace
        virtual void SAL_CALL replaceByName( const OUString& sName, const css::uno::Any& aPropertySet )
            throw(css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;

        // XElementAccess
        virtual css::uno::Type SAL_CALL getElementType()
            throw (css::uno::RuntimeException, std::exception) override;

        virtual sal_Bool SAL_CALL hasElements()
            throw (css::uno::RuntimeException, std::exception) override;

        // container.XContainerListener
        virtual void SAL_CALL     elementInserted( const ContainerEvent& aEvent ) throw(RuntimeException, std::exception) override;
        virtual void SAL_CALL     elementRemoved ( const ContainerEvent& aEvent ) throw(RuntimeException, std::exception) override;
        virtual void SAL_CALL     elementReplaced( const ContainerEvent& aEvent ) throw(RuntimeException, std::exception) override;

        // lang.XEventListener
        virtual void SAL_CALL disposing( const EventObject& aEvent ) throw(RuntimeException, std::exception) override;

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

        // Cache structure. Valid values are described by the eMask member. All other values should not be
        // provided to outside code!
        struct WindowStateInfo
        {
            WindowStateInfo()
                : bLocked(false)
                , bDocked(false)
                , bVisible(false)
                , bContext(false)
                , bHideFromMenu(false)
                , bNoClose(false)
                , bSoftClose(false)
                , bContextActive(false)
                , aDockingArea(css::ui::DockingArea_DOCKINGAREA_TOP)
                , aDockPos(0, 0)
                , aPos(0, 0)
                , aSize(0, 0)
                , nInternalState(0)
                , nStyle(0)
                , nMask(0)
            {
            }

            bool                                    bLocked : 1,
                                                    bDocked : 1,
                                                    bVisible : 1,
                                                    bContext : 1,
                                                    bHideFromMenu : 1,
                                                    bNoClose : 1,
                                                    bSoftClose : 1,
                                                    bContextActive : 1;
            css::ui::DockingArea                    aDockingArea;
            css::awt::Point                         aDockPos;
            css::awt::Size                          aDockSize;
            css::awt::Point                         aPos;
            css::awt::Size                          aSize;
            OUString                                aUIName;
            sal_uInt32                              nInternalState;
            sal_uInt16                              nStyle;
            sal_uInt32                              nMask; // see WindowStateMask
        };

        void                      impl_putPropertiesFromStruct( const WindowStateInfo& rWinStateInfo, Reference< XPropertySet >& xPropSet );
        Any                       impl_insertCacheAndReturnSequence( const OUString& rResourceURL, Reference< XNameAccess >& rNameAccess );
        WindowStateInfo&          impl_insertCacheAndReturnWinState( const OUString& rResourceURL, Reference< XNameAccess >& rNameAccess );
        Any                       impl_getSequenceFromStruct( const WindowStateInfo& rWinStateInfo );
        void                      impl_fillStructFromSequence( WindowStateInfo& rWinStateInfo, const Sequence< PropertyValue >& rSeq );
        Any                       impl_getWindowStateFromResourceURL( const OUString& rResourceURL );
        bool                      impl_initializeConfigAccess();

    private:
        typedef std::unordered_map< OUString,
                                    WindowStateInfo,
                                    OUStringHash,
                                    std::equal_to< OUString > > ResourceURLToInfoCache;

        osl::Mutex                        m_aMutex;
        OUString                          m_aConfigWindowAccess;
        Reference< XMultiServiceFactory > m_xConfigProvider;
        Reference< XNameAccess >          m_xConfigAccess;
        Reference< XContainerListener >   m_xConfigListener;
        ResourceURLToInfoCache            m_aResourceURLToInfoCache;
        bool                              m_bConfigAccessInitialized : 1,
                                          m_bModified : 1;
        std::vector< OUString >           m_aPropArray;
};

ConfigurationAccess_WindowState::ConfigurationAccess_WindowState( const OUString& aModuleName, const Reference< XComponentContext >& rxContext ) :
    m_aConfigWindowAccess( "/org.openoffice.Office.UI." ),
    m_bConfigAccessInitialized( false ),
    m_bModified( false )
{
    // Create configuration hierarchical access name
    m_aConfigWindowAccess += aModuleName;
    m_aConfigWindowAccess += "/UIElements/States";
    m_xConfigProvider = theDefaultProvider::get( rxContext );

    // Initialize access array with property names.
    sal_Int32 n = 0;
    while ( CONFIGURATION_PROPERTIES[n] )
    {
        m_aPropArray.push_back( OUString::createFromAscii( CONFIGURATION_PROPERTIES[n] ));
        ++n;
    }
}

ConfigurationAccess_WindowState::~ConfigurationAccess_WindowState()
{
    // SAFE
    osl::MutexGuard g(m_aMutex);
    Reference< XContainer > xContainer( m_xConfigAccess, UNO_QUERY );
    if ( xContainer.is() )
        xContainer->removeContainerListener(m_xConfigListener);
}

// XNameAccess
Any SAL_CALL ConfigurationAccess_WindowState::getByName( const OUString& rResourceURL )
throw ( NoSuchElementException, WrappedTargetException, RuntimeException, std::exception)
{
    // SAFE
    osl::MutexGuard g(m_aMutex);

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

Sequence< OUString > SAL_CALL ConfigurationAccess_WindowState::getElementNames()
throw ( RuntimeException, std::exception )
{
    // SAFE
    osl::MutexGuard g(m_aMutex);

    if ( !m_bConfigAccessInitialized )
    {
        impl_initializeConfigAccess();
        m_bConfigAccessInitialized = true;
    }

    if ( m_xConfigAccess.is() )
        return m_xConfigAccess->getElementNames();
    else
        return Sequence< OUString > ();
}

sal_Bool SAL_CALL ConfigurationAccess_WindowState::hasByName( const OUString& rResourceURL )
throw (css::uno::RuntimeException, std::exception)
{
    // SAFE
    osl::MutexGuard g(m_aMutex);

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
throw ( RuntimeException, std::exception )
{
    return( cppu::UnoType<Sequence< PropertyValue >>::get() );
}

sal_Bool SAL_CALL ConfigurationAccess_WindowState::hasElements()
throw ( RuntimeException, std::exception )
{
    // SAFE
    osl::MutexGuard g(m_aMutex);

    if ( !m_bConfigAccessInitialized )
    {
        impl_initializeConfigAccess();
        m_bConfigAccessInitialized = true;
    }

    if ( m_xConfigAccess.is() )
        return m_xConfigAccess->hasElements();
    else
        return sal_False;
}

// XNameContainer
void SAL_CALL ConfigurationAccess_WindowState::removeByName( const OUString& rResourceURL )
throw( NoSuchElementException, WrappedTargetException, RuntimeException, std::exception )
{
    // SAFE
    osl::ResettableMutexGuard g(m_aMutex);

    ResourceURLToInfoCache::iterator pIter = m_aResourceURLToInfoCache.find( rResourceURL );
    if ( pIter != m_aResourceURLToInfoCache.end() )
        m_aResourceURLToInfoCache.erase( pIter );

    if ( !m_bConfigAccessInitialized )
    {
        impl_initializeConfigAccess();
        m_bConfigAccessInitialized = true;
    }

    try
    {
        // Remove must be write-through => remove element from configuration
        Reference< XNameContainer > xNameContainer( m_xConfigAccess, UNO_QUERY );
        if ( xNameContainer.is() )
        {
            g.clear();

            xNameContainer->removeByName( rResourceURL );
            Reference< XChangesBatch > xFlush( m_xConfigAccess, UNO_QUERY );
            if ( xFlush.is() )
                xFlush->commitChanges();
        }
    }
    catch ( const css::lang::WrappedTargetException& )
    {
    }
}

void SAL_CALL ConfigurationAccess_WindowState::insertByName( const OUString& rResourceURL, const css::uno::Any& aPropertySet )
throw( IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException, std::exception )
{
    // SAFE
    osl::ResettableMutexGuard g(m_aMutex);

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
                m_bConfigAccessInitialized = true;
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
                        g.clear();

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
void SAL_CALL ConfigurationAccess_WindowState::replaceByName( const OUString& rResourceURL, const css::uno::Any& aPropertySet )
throw( IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException, std::exception )
{
    // SAFE
    osl::ResettableMutexGuard g(m_aMutex);

    Sequence< PropertyValue > aPropSet;
    if ( aPropertySet >>= aPropSet )
    {
        ResourceURLToInfoCache::iterator pIter = m_aResourceURLToInfoCache.find( rResourceURL );
        if ( pIter != m_aResourceURLToInfoCache.end() )
        {
            WindowStateInfo& rWinStateInfo = pIter->second;
            impl_fillStructFromSequence( rWinStateInfo, aPropSet );
            m_bModified = true;
        }
        else
        {
            if ( !m_bConfigAccessInitialized )
            {
                impl_initializeConfigAccess();
                m_bConfigAccessInitialized = true;
            }

            // Try to ask our configuration access
            Reference< XNameAccess > xNameAccess;
            Any a( m_xConfigAccess->getByName( rResourceURL ));

            if ( a >>= xNameAccess )
            {
                WindowStateInfo& rWinStateInfo( impl_insertCacheAndReturnWinState( rResourceURL, xNameAccess ));
                impl_fillStructFromSequence( rWinStateInfo, aPropSet );
                m_bModified = true;
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
                OUString        aResourceURL( pIter->first );
                m_bModified = false;
                g.clear();

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
void SAL_CALL ConfigurationAccess_WindowState::elementInserted( const ContainerEvent& ) throw(RuntimeException, std::exception)
{
    // do nothing - next time someone wants to retrieve this node we will find it in the configuration
}

void SAL_CALL ConfigurationAccess_WindowState::elementRemoved ( const ContainerEvent& ) throw(RuntimeException, std::exception)
{
}

void SAL_CALL ConfigurationAccess_WindowState::elementReplaced( const ContainerEvent& ) throw(RuntimeException, std::exception)
{
}

// lang.XEventListener
void SAL_CALL ConfigurationAccess_WindowState::disposing( const EventObject& aEvent ) throw(RuntimeException, std::exception)
{
    // SAFE
    // remove our reference to the config access
    osl::MutexGuard g(m_aMutex);

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
                    DBG_ASSERT( false, "Wrong value for ConfigurationAccess_WindowState. Who has forgotten to add this new property!" );
            }
        }
    }

    return makeAny( aPropSeq );
}

Any ConfigurationAccess_WindowState::impl_insertCacheAndReturnSequence( const OUString& rResourceURL, Reference< XNameAccess >& xNameAccess )
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
                    bool bValue;
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
                    OUString aString;
                    if ( a >>= aString )
                    {
                        sal_Int32 nToken( 0 );
                        OUString aXStr = aString.getToken( 0, ',', nToken );
                        if ( nToken > 0 )
                        {
                            css::awt::Point aPos;
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
                    OUString aString;
                    if ( a >>= aString )
                    {
                        sal_Int32 nToken( 0 );
                        OUString aStr = aString.getToken( 0, ',', nToken );
                        if ( nToken > 0 )
                        {
                            css::awt::Size aSize;
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
                    OUString aValue;
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
                    DBG_ASSERT( false, "Wrong value for ConfigurationAccess_WindowState. Who has forgotten to add this new property!" );
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
        catch( const css::container::NoSuchElementException& )
        {
        }
        catch ( const css::lang::WrappedTargetException& )
        {
        }
    }

    aWindowStateInfo.nMask = nMask;
    m_aResourceURLToInfoCache.insert( ResourceURLToInfoCache::value_type( rResourceURL, aWindowStateInfo ));
    return makeAny( aPropSeq );
}

ConfigurationAccess_WindowState::WindowStateInfo& ConfigurationAccess_WindowState::impl_insertCacheAndReturnWinState( const OUString& rResourceURL, Reference< XNameAccess >& rNameAccess )
{
    sal_Int32                 nMask( 0 );
    sal_Int32                 nCount( m_aPropArray.size() );
    sal_Int32                 i( 0 );
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
                    bool bValue;
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
                                DBG_ASSERT( false, "Unknown boolean property in WindowState found!" );
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
                    OUString aString;
                    if ( a >>= aString )
                    {
                        sal_Int32 nToken( 0 );
                        OUString aXStr = aString.getToken( 0, ',', nToken );
                        if ( nToken > 0 )
                        {
                            css::awt::Point aPos;
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
                    OUString aString;
                    if ( a >>= aString )
                    {
                        sal_Int32 nToken( 0 );
                        OUString aStr = aString.getToken( 0, ',', nToken );
                        if ( nToken > 0 )
                        {
                            css::awt::Size aSize;
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
                    OUString aValue;
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
                break;

                default:
                    DBG_ASSERT( false, "Wrong value for ConfigurationAccess_WindowState. Who has forgotten to add this new property!" );
            }
        }
        catch( const css::container::NoSuchElementException& )
        {
        }
        catch ( const css::lang::WrappedTargetException& )
        {
        }
    }

    aWindowStateInfo.nMask = nMask;
    ResourceURLToInfoCache::iterator pIter = (m_aResourceURLToInfoCache.insert( ResourceURLToInfoCache::value_type( rResourceURL, aWindowStateInfo ))).first;
    return pIter->second;
}

Any ConfigurationAccess_WindowState::impl_getWindowStateFromResourceURL( const OUString& rResourceURL )
{
    if ( !m_bConfigAccessInitialized )
    {
        impl_initializeConfigAccess();
        m_bConfigAccessInitialized = true;
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
    catch( const css::container::NoSuchElementException& )
    {
    }
    catch ( const css::lang::WrappedTargetException& )
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
                        bool bValue;
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
                            }
                        }
                    }
                    break;

                    case PROPERTY_DOCKINGAREA:
                    {
                        css::ui::DockingArea eDockingArea;
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
                        css::awt::Point aPoint;
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
                        css::awt::Size aSize;
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
                        OUString aValue;
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
                        DBG_ASSERT( false, "Wrong value for ConfigurationAccess_WindowState. Who has forgotten to add this new property!" );
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
    OUString                  aDelim( "," );

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
                        xPropSet->setPropertyValue( m_aPropArray[i], makeAny( rWinStateInfo.bLocked ) ); break;
                    case PROPERTY_DOCKED:
                        xPropSet->setPropertyValue( m_aPropArray[i], makeAny( rWinStateInfo.bDocked ) ); break;
                    case PROPERTY_VISIBLE:
                        xPropSet->setPropertyValue( m_aPropArray[i], makeAny( rWinStateInfo.bVisible ) ); break;
                    case PROPERTY_CONTEXT:
                        xPropSet->setPropertyValue( m_aPropArray[i], makeAny( rWinStateInfo.bContext ) ); break;
                    case PROPERTY_HIDEFROMMENU:
                        xPropSet->setPropertyValue( m_aPropArray[i], makeAny( rWinStateInfo.bHideFromMenu ) ); break;
                    case PROPERTY_NOCLOSE:
                        xPropSet->setPropertyValue( m_aPropArray[i], makeAny( rWinStateInfo.bNoClose ) ); break;
                    case PROPERTY_SOFTCLOSE:
                        xPropSet->setPropertyValue( m_aPropArray[i], makeAny( rWinStateInfo.bSoftClose ) ); break;
                    case PROPERTY_CONTEXTACTIVE:
                        xPropSet->setPropertyValue( m_aPropArray[i], makeAny( rWinStateInfo.bContextActive ) ); break;
                    case PROPERTY_DOCKINGAREA:
                        xPropSet->setPropertyValue( m_aPropArray[i], makeAny( sal_Int16( rWinStateInfo.aDockingArea ) ) ); break;
                    case PROPERTY_POS:
                    case PROPERTY_DOCKPOS:
                    {
                        OUString aPosStr;
                        if ( i == PROPERTY_POS )
                            aPosStr = OUString::number( rWinStateInfo.aPos.X );
                        else
                            aPosStr = OUString::number( rWinStateInfo.aDockPos.X );
                        aPosStr += aDelim;
                        if ( i == PROPERTY_POS )
                            aPosStr += OUString::number( rWinStateInfo.aPos.Y );
                        else
                            aPosStr += OUString::number( rWinStateInfo.aDockPos.Y );
                        xPropSet->setPropertyValue( m_aPropArray[i], makeAny( aPosStr ) );
                        break;
                    }
                    case PROPERTY_SIZE:
                    case PROPERTY_DOCKSIZE:
                    {
                        OUString aSizeStr;
                        if ( i == PROPERTY_SIZE )
                            aSizeStr = ( OUString::number( rWinStateInfo.aSize.Width ));
                        else
                            aSizeStr = ( OUString::number( rWinStateInfo.aDockSize.Width ));
                        aSizeStr += aDelim;
                        if ( i == PROPERTY_SIZE )
                            aSizeStr += OUString::number( rWinStateInfo.aSize.Height );
                        else
                            aSizeStr += OUString::number( rWinStateInfo.aDockSize.Height );
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
                        DBG_ASSERT( false, "Wrong value for ConfigurationAccess_WindowState. Who has forgotten to add this new property!" );
                }
            }
            catch( const Exception& )
            {
            }
        }
    }
}

bool ConfigurationAccess_WindowState::impl_initializeConfigAccess()
{
    Sequence< Any > aArgs( 2 );
    PropertyValue   aPropValue;

    try
    {
        aPropValue.Name  = "nodepath";
        aPropValue.Value <<= m_aConfigWindowAccess;
        aArgs[0] <<= aPropValue;
        aPropValue.Name = "lazywrite";
        aPropValue.Value <<= sal_True;
        aArgs[1] <<= aPropValue;

        m_xConfigAccess.set( m_xConfigProvider->createInstanceWithArguments(
                    "com.sun.star.configuration.ConfigurationUpdateAccess", aArgs ), UNO_QUERY );
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

        return true;
    }
    catch ( const WrappedTargetException& )
    {
    }
    catch ( const Exception& )
    {
    }

    return false;
}

typedef ::cppu::WeakComponentImplHelper< css::container::XNameAccess,
        css::lang::XServiceInfo> WindowStateConfiguration_BASE;

class WindowStateConfiguration : private cppu::BaseMutex,
                                 public WindowStateConfiguration_BASE
{
public:
    WindowStateConfiguration( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~WindowStateConfiguration();

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override
    {
        return OUString("com.sun.star.comp.framework.WindowStateConfiguration");
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override
    {
        css::uno::Sequence< OUString > aSeq(1);
        aSeq[0] = "com.sun.star.ui.WindowStateConfiguration";
        return aSeq;
    }

    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
        throw ( css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw (css::uno::RuntimeException, std::exception) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements()
        throw (css::uno::RuntimeException, std::exception) override;

    typedef std::unordered_map< OUString,
                                OUString,
                                OUStringHash,
                                std::equal_to< OUString > > ModuleToWindowStateFileMap;

    typedef std::unordered_map< OUString,
                                css::uno::Reference< css::container::XNameAccess >,
                                OUStringHash,
                                std::equal_to< OUString > > ModuleToWindowStateConfigHashMap;

private:
    css::uno::Reference< css::uno::XComponentContext>         m_xContext;
    ModuleToWindowStateFileMap                                m_aModuleToFileHashMap;
    ModuleToWindowStateConfigHashMap                          m_aModuleToWindowStateHashMap;
};

WindowStateConfiguration::WindowStateConfiguration( const Reference< XComponentContext >& rxContext ) :
    WindowStateConfiguration_BASE(m_aMutex),
    m_xContext( rxContext )
{
    css::uno::Reference< css::frame::XModuleManager2 > xModuleManager =
        ModuleManager::create( m_xContext );
    Reference< XNameAccess > xEmptyNameAccess;
    Sequence< OUString > aElementNames;
    try
    {
        aElementNames = xModuleManager->getElementNames();
    }
    catch (const css::uno::RuntimeException &)
    {
    }
    Sequence< PropertyValue > aSeq;
    OUString                  aModuleIdentifier;

    for ( sal_Int32 i = 0; i < aElementNames.getLength(); i++ )
    {
        aModuleIdentifier = aElementNames[i];
        if ( xModuleManager->getByName( aModuleIdentifier ) >>= aSeq )
        {
            OUString aWindowStateFileStr;
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
    osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);
    m_aModuleToFileHashMap.clear();
    m_aModuleToWindowStateHashMap.clear();
}

Any SAL_CALL WindowStateConfiguration::getByName( const OUString& aModuleIdentifier )
throw (css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);

    ModuleToWindowStateFileMap::const_iterator pIter = m_aModuleToFileHashMap.find( aModuleIdentifier );
    if ( pIter != m_aModuleToFileHashMap.end() )
    {
        Any a;
        OUString aWindowStateConfigFile( pIter->second );

        ModuleToWindowStateConfigHashMap::iterator pModuleIter = m_aModuleToWindowStateHashMap.find( aWindowStateConfigFile );
        if ( pModuleIter != m_aModuleToWindowStateHashMap.end() )
        {
            if ( pModuleIter->second.is() )
                a = makeAny( pModuleIter->second );
            else
            {
                Reference< XNameAccess > xResourceURLWindowState;
                ConfigurationAccess_WindowState* pModuleWindowState = new ConfigurationAccess_WindowState( aWindowStateConfigFile, m_xContext );
                xResourceURLWindowState.set( static_cast< cppu::OWeakObject* >( pModuleWindowState ),UNO_QUERY );
                pModuleIter->second = xResourceURLWindowState;
                a <<= xResourceURLWindowState;
            }

            return a;
        }
    }

    throw NoSuchElementException();
}

Sequence< OUString > SAL_CALL WindowStateConfiguration::getElementNames()
throw (css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);

    Sequence< OUString > aSeq( m_aModuleToFileHashMap.size() );

    sal_Int32 n = 0;
    ModuleToWindowStateFileMap::const_iterator pIter = m_aModuleToFileHashMap.begin();
    while ( pIter != m_aModuleToFileHashMap.end() )
    {
        aSeq[n] = pIter->first;
        ++pIter;
    }

    return aSeq;
}

sal_Bool SAL_CALL WindowStateConfiguration::hasByName( const OUString& aName )
throw (css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);

    ModuleToWindowStateFileMap::const_iterator pIter = m_aModuleToFileHashMap.find( aName );
    return ( pIter != m_aModuleToFileHashMap.end() );
}

// XElementAccess
Type SAL_CALL WindowStateConfiguration::getElementType()
throw (css::uno::RuntimeException, std::exception)
{
    return( cppu::UnoType<XNameAccess>::get());
}

sal_Bool SAL_CALL WindowStateConfiguration::hasElements()
throw (css::uno::RuntimeException, std::exception)
{
    // We always have at least one module. So it is valid to return true!
    return sal_True;
}

struct Instance {
    explicit Instance(
        css::uno::Reference<css::uno::XComponentContext> const & context):
        instance(static_cast<cppu::OWeakObject *>(
                    new WindowStateConfiguration(context)))
    {
    }

    css::uno::Reference<css::uno::XInterface> instance;
};

struct Singleton:
    public rtl::StaticWithArg<
        Instance, css::uno::Reference<css::uno::XComponentContext>, Singleton>
{};

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_framework_WindowStateConfiguration_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(static_cast<cppu::OWeakObject *>(
                Singleton::get(context).instance.get()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
