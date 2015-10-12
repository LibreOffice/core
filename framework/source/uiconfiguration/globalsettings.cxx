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

#include "uiconfiguration/globalsettings.hxx"
#include "services.h"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XEventListener.hpp>

#include <rtl/ustrbuf.hxx>
#include <rtl/instance.hxx>
#include <cppuhelper/implbase.hxx>

//  Defines

using namespace ::com::sun::star;

//  Namespace

static const char GLOBALSETTINGS_ROOT_ACCESS[]              = "/org.openoffice.Office.UI.GlobalSettings/Toolbars";

static const char GLOBALSETTINGS_NODEREF_STATES[]           = "States";
static const char GLOBALSETTINGS_PROPERTY_LOCKED[]          = "Locked";
static const char GLOBALSETTINGS_PROPERTY_DOCKED[]          = "Docked";
static const char GLOBALSETTINGS_PROPERTY_STATESENABLED[]   = "StatesEnabled";

namespace framework
{

//  Configuration access class for WindowState supplier implementation

class GlobalSettings_Access : public ::cppu::WeakImplHelper<
                                  ::com::sun::star::lang::XComponent,
                                  ::com::sun::star::lang::XEventListener>
{
    public:
        GlobalSettings_Access( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
        virtual ~GlobalSettings_Access();

        // XComponent
        virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // settings access
        bool HasStatesInfo( GlobalSettings::UIElementType eElementType );
        bool GetStateInfo( GlobalSettings::UIElementType eElementType, GlobalSettings::StateInfo eStateInfo, ::com::sun::star::uno::Any& aValue );

    private:
        bool impl_initConfigAccess();

        osl::Mutex m_mutex;
        bool                                                                            m_bDisposed   : 1,
                                                                                            m_bConfigRead : 1;
        OUString                                                                       m_aNodeRefStates;
        OUString                                                                       m_aPropStatesEnabled;
        OUString                                                                       m_aPropLocked;
        OUString                                                                       m_aPropDocked;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >        m_xConfigAccess;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>         m_xContext;
};

GlobalSettings_Access::GlobalSettings_Access( const css::uno::Reference< css::uno::XComponentContext >& rxContext ) :
    m_bDisposed( false ),
    m_bConfigRead( false ),
    m_aNodeRefStates( GLOBALSETTINGS_NODEREF_STATES ),
    m_aPropStatesEnabled( GLOBALSETTINGS_PROPERTY_STATESENABLED ),
    m_aPropLocked( GLOBALSETTINGS_PROPERTY_LOCKED ),
    m_aPropDocked( GLOBALSETTINGS_PROPERTY_DOCKED ),
    m_xContext( rxContext )
{
}

GlobalSettings_Access::~GlobalSettings_Access()
{
}

// XComponent
void SAL_CALL GlobalSettings_Access::dispose()
throw ( css::uno::RuntimeException, std::exception )
{
    osl::MutexGuard g(m_mutex);
    m_xConfigAccess.clear();
    m_bDisposed = true;
}

void SAL_CALL GlobalSettings_Access::addEventListener( const css::uno::Reference< css::lang::XEventListener >& )
throw (css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL GlobalSettings_Access::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& )
throw (css::uno::RuntimeException, std::exception)
{
}

// XEventListener
void SAL_CALL GlobalSettings_Access::disposing( const css::lang::EventObject& )
throw (css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard g(m_mutex);
    m_xConfigAccess.clear();
}

// settings access
bool GlobalSettings_Access::HasStatesInfo( GlobalSettings::UIElementType eElementType )
{
    osl::MutexGuard g(m_mutex);
    if ( eElementType == GlobalSettings::UIELEMENT_TYPE_DOCKWINDOW )
        return false;
    else if ( eElementType == GlobalSettings::UIELEMENT_TYPE_STATUSBAR )
        return false;

    if ( m_bDisposed )
        return false;

    if ( !m_bConfigRead )
    {
        m_bConfigRead = true;
        impl_initConfigAccess();
    }

    if ( m_xConfigAccess.is() )
    {
        try
        {
            css::uno::Any a;
            bool      bValue;
            a = m_xConfigAccess->getByName( m_aPropStatesEnabled );
            if ( a >>= bValue )
                return bValue;
        }
        catch ( const css::container::NoSuchElementException& )
        {
        }
        catch ( const css::uno::Exception& )
        {
        }
    }

    return false;
}

bool GlobalSettings_Access::GetStateInfo( GlobalSettings::UIElementType eElementType, GlobalSettings::StateInfo eStateInfo, ::com::sun::star::uno::Any& aValue )
{
    osl::MutexGuard g(m_mutex);
    if ( eElementType == GlobalSettings::UIELEMENT_TYPE_DOCKWINDOW )
        return false;
    else if ( eElementType == GlobalSettings::UIELEMENT_TYPE_STATUSBAR )
        return false;

    if ( m_bDisposed )
        return false;

    if ( !m_bConfigRead )
    {
        m_bConfigRead = true;
        impl_initConfigAccess();
    }

    if ( m_xConfigAccess.is() )
    {
        try
        {
            css::uno::Any a;
            a = m_xConfigAccess->getByName( m_aNodeRefStates );
            css::uno::Reference< css::container::XNameAccess > xNameAccess;
            if ( a >>= xNameAccess )
            {
                if ( eStateInfo == GlobalSettings::STATEINFO_LOCKED )
                    a = xNameAccess->getByName( m_aPropLocked );
                else if ( eStateInfo == GlobalSettings::STATEINFO_DOCKED )
                    a = xNameAccess->getByName( m_aPropDocked );

                aValue = a;
                return true;
            }
        }
        catch ( const css::container::NoSuchElementException& )
        {
        }
        catch ( const css::uno::Exception& )
        {
        }
    }

    return false;
}

bool GlobalSettings_Access::impl_initConfigAccess()
{
    css::uno::Sequence< css::uno::Any > aArgs( 2 );
    css::beans::PropertyValue           aPropValue;

    try
    {
        if ( m_xContext.is() )
        {
            css::uno::Reference< css::lang::XMultiServiceFactory > xConfigProvider =
                 css::configuration::theDefaultProvider::get( m_xContext );

            aPropValue.Name  = "nodepath";
            aPropValue.Value = css::uno::makeAny( OUString( GLOBALSETTINGS_ROOT_ACCESS ));
            aArgs[0] = css::uno::makeAny( aPropValue );
            aPropValue.Name = "lazywrite";
            aPropValue.Value = css::uno::makeAny( sal_True );
            aArgs[1] = css::uno::makeAny( aPropValue );

            m_xConfigAccess = css::uno::Reference< css::container::XNameAccess >(
                                xConfigProvider->createInstanceWithArguments(
                                    SERVICENAME_CFGREADACCESS, aArgs ),
                                css::uno::UNO_QUERY );

            css::uno::Reference< css::lang::XComponent >(
                xConfigProvider, css::uno::UNO_QUERY_THROW )->addEventListener(
                    css::uno::Reference< css::lang::XEventListener >(
                        static_cast< cppu::OWeakObject* >( this ),
                        css::uno::UNO_QUERY ));
        }

        return m_xConfigAccess.is();
    }
    catch ( const css::lang::WrappedTargetException& )
    {
    }
    catch ( const css::uno::Exception& )
    {
    }

    return false;
}

//  global class

struct mutexGlobalSettings : public rtl::Static< osl::Mutex, mutexGlobalSettings > {};
static GlobalSettings_Access* pStaticSettings = 0;

static GlobalSettings_Access* GetGlobalSettings( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext )
{
    osl::MutexGuard aGuard(mutexGlobalSettings::get());
    if ( !pStaticSettings )
        pStaticSettings = new GlobalSettings_Access( rxContext );
    return pStaticSettings;
}

GlobalSettings::GlobalSettings( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext ) :
    m_xContext( rxContext )
{
}

GlobalSettings::~GlobalSettings()
{
}

// settings access
bool GlobalSettings::HasStatesInfo( UIElementType eElementType )
{
    GlobalSettings_Access* pSettings( GetGlobalSettings( m_xContext ));

    if ( pSettings )
        return pSettings->HasStatesInfo( eElementType );
    else
        return false;
}

bool GlobalSettings::GetStateInfo( UIElementType eElementType, StateInfo eStateInfo, ::com::sun::star::uno::Any& aValue )
{
    GlobalSettings_Access* pSettings( GetGlobalSettings( m_xContext ));

    if ( pSettings )
        return pSettings->GetStateInfo( eElementType, eStateInfo, aValue );
    else
        return false;
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
