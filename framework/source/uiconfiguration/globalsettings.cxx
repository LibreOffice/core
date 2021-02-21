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

#include <uiconfiguration/globalsettings.hxx>
#include <services.h>

#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XEventListener.hpp>

#include <rtl/instance.hxx>
#include <rtl/ref.hxx>
#include <comphelper/propertysequence.hxx>
#include <cppuhelper/implbase.hxx>

//  Defines

using namespace ::com::sun::star;

//  Namespace

namespace framework
{

//  Configuration access class for WindowState supplier implementation

namespace {

class GlobalSettings_Access : public ::cppu::WeakImplHelper<
                                  css::lang::XComponent,
                                  css::lang::XEventListener>
{
    public:
        explicit GlobalSettings_Access( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

        // XComponent
        virtual void SAL_CALL dispose() override;
        virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
        virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

        // settings access
        bool HasToolbarStatesInfo();
        bool GetToolbarStateInfo( GlobalSettings::StateInfo eStateInfo, css::uno::Any& aValue );

    private:
        void impl_initConfigAccess();

        osl::Mutex                                                m_mutex;
        bool                                                      m_bDisposed   : 1,
                                                                  m_bConfigRead : 1;
        OUString                                                  m_aNodeRefStates;
        OUString                                                  m_aPropStatesEnabled;
        OUString                                                  m_aPropLocked;
        OUString                                                  m_aPropDocked;
        css::uno::Reference< css::container::XNameAccess >        m_xConfigAccess;
        css::uno::Reference< css::uno::XComponentContext>         m_xContext;
};

}

GlobalSettings_Access::GlobalSettings_Access( const css::uno::Reference< css::uno::XComponentContext >& rxContext ) :
    m_bDisposed( false ),
    m_bConfigRead( false ),
    m_aNodeRefStates( "States" ),
    m_aPropStatesEnabled( "StatesEnabled" ),
    m_aPropLocked( "Locked" ),
    m_aPropDocked( "Docked" ),
    m_xContext( rxContext )
{
}

// XComponent
void SAL_CALL GlobalSettings_Access::dispose()
{
    osl::MutexGuard g(m_mutex);
    m_xConfigAccess.clear();
    m_bDisposed = true;
}

void SAL_CALL GlobalSettings_Access::addEventListener( const css::uno::Reference< css::lang::XEventListener >& )
{
}

void SAL_CALL GlobalSettings_Access::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& )
{
}

// XEventListener
void SAL_CALL GlobalSettings_Access::disposing( const css::lang::EventObject& )
{
    osl::MutexGuard g(m_mutex);
    m_xConfigAccess.clear();
}

// settings access
bool GlobalSettings_Access::HasToolbarStatesInfo()
{
    osl::MutexGuard g(m_mutex);

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

bool GlobalSettings_Access::GetToolbarStateInfo( GlobalSettings::StateInfo eStateInfo, css::uno::Any& aValue )
{
    osl::MutexGuard g(m_mutex);

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
            css::uno::Any a = m_xConfigAccess->getByName( m_aNodeRefStates );
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

void GlobalSettings_Access::impl_initConfigAccess()
{
    try
    {
        if ( m_xContext.is() )
        {
            css::uno::Reference< css::lang::XMultiServiceFactory > xConfigProvider =
                 css::configuration::theDefaultProvider::get( m_xContext );

            uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
            {
                {"nodepath", uno::Any(OUString("/org.openoffice.Office.UI.GlobalSettings/Toolbars"))}
            }));
            m_xConfigAccess.set(xConfigProvider->createInstanceWithArguments(
                                    SERVICENAME_CFGREADACCESS, aArgs ),
                                css::uno::UNO_QUERY );

            css::uno::Reference< css::lang::XComponent >(
                xConfigProvider, css::uno::UNO_QUERY_THROW )->addEventListener(
                    css::uno::Reference< css::lang::XEventListener >(
                        static_cast< cppu::OWeakObject* >( this ),
                        css::uno::UNO_QUERY ));
        }
    }
    catch ( const css::lang::WrappedTargetException& )
    {
    }
    catch ( const css::uno::Exception& )
    {
    }
}

//  global class

namespace {

struct mutexGlobalSettings : public rtl::Static< osl::Mutex, mutexGlobalSettings > {};

}

static rtl::Reference<GlobalSettings_Access> pStaticSettings;

static GlobalSettings_Access* GetGlobalSettings( const css::uno::Reference< css::uno::XComponentContext >& rxContext )
{
    osl::MutexGuard aGuard(mutexGlobalSettings::get());
    if ( !pStaticSettings )
        pStaticSettings = new GlobalSettings_Access( rxContext );
    return pStaticSettings.get();
}

GlobalSettings::GlobalSettings( const css::uno::Reference< css::uno::XComponentContext >& rxContext ) :
    m_xContext( rxContext )
{
}

GlobalSettings::~GlobalSettings()
{
}

// settings access
bool GlobalSettings::HasToolbarStatesInfo() const
{
    GlobalSettings_Access* pSettings( GetGlobalSettings( m_xContext ));

    if ( pSettings )
        return pSettings->HasToolbarStatesInfo();
    else
        return false;
}

bool GlobalSettings::GetToolbarStateInfo( StateInfo eStateInfo, css::uno::Any& aValue )
{
    GlobalSettings_Access* pSettings( GetGlobalSettings( m_xContext ));

    if ( pSettings )
        return pSettings->GetToolbarStateInfo( eStateInfo, aValue );
    else
        return false;
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
