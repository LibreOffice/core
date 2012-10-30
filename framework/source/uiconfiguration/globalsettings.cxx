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

#include "uiconfiguration/globalsettings.hxx"
#include <threadhelp/resetableguard.hxx>
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
#include <cppuhelper/weak.hxx>

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________

using namespace ::com::sun::star;

using ::rtl::OUString;

//_________________________________________________________________________________________________________________
//  Namespace
//_________________________________________________________________________________________________________________

static const char GLOBALSETTINGS_ROOT_ACCESS[]              = "/org.openoffice.Office.UI.GlobalSettings/Toolbars";

static const char GLOBALSETTINGS_NODEREF_STATES[]           = "States";
static const char GLOBALSETTINGS_PROPERTY_LOCKED[]          = "Locked";
static const char GLOBALSETTINGS_PROPERTY_DOCKED[]          = "Docked";
static const char GLOBALSETTINGS_PROPERTY_STATESENABLED[]   = "StatesEnabled";

namespace framework
{

//*****************************************************************************************************************
//  Configuration access class for WindowState supplier implementation
//*****************************************************************************************************************

class GlobalSettings_Access : public ::com::sun::star::lang::XComponent      ,
                              public ::com::sun::star::lang::XEventListener  ,
                              private ThreadHelpBase                         ,  // Struct for right initalization of mutex member! Must be first of baseclasses.
                              public ::cppu::OWeakObject
{
    public:
        GlobalSettings_Access( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
        virtual ~GlobalSettings_Access();

        // XInterface, XTypeProvider, XServiceInfo
        FWK_DECLARE_XINTERFACE

        // XComponent
        virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

        // settings access
        sal_Bool HasStatesInfo( GlobalSettings::UIElementType eElementType );
        sal_Bool GetStateInfo( GlobalSettings::UIElementType eElementType, GlobalSettings::StateInfo eStateInfo, ::com::sun::star::uno::Any& aValue );

    private:
        sal_Bool impl_initConfigAccess();

        sal_Bool                                                                            m_bDisposed   : 1,
                                                                                            m_bConfigRead : 1;
        rtl::OUString                                                                       m_aConfigSettingsAccess;
        rtl::OUString                                                                       m_aNodeRefStates;
        rtl::OUString                                                                       m_aPropStatesEnabled;
        rtl::OUString                                                                       m_aPropLocked;
        rtl::OUString                                                                       m_aPropDocked;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >        m_xConfigAccess;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>         m_xContext;
};


//*****************************************************************************************************************
//  XInterface
//*****************************************************************************************************************
DEFINE_XINTERFACE_2     (   GlobalSettings_Access                           ,
                            OWeakObject                                     ,
                            DIRECT_INTERFACE ( css::lang::XComponent        ),
                            DIRECT_INTERFACE ( css::lang::XEventListener    )
                        )

GlobalSettings_Access::GlobalSettings_Access( const css::uno::Reference< css::uno::XComponentContext >& rxContext ) :
    ThreadHelpBase(),
    m_bDisposed( sal_False ),
    m_bConfigRead( sal_False ),
    m_aConfigSettingsAccess( RTL_CONSTASCII_USTRINGPARAM( GLOBALSETTINGS_ROOT_ACCESS )),
    m_aNodeRefStates( RTL_CONSTASCII_USTRINGPARAM( GLOBALSETTINGS_NODEREF_STATES )),
    m_aPropStatesEnabled( RTL_CONSTASCII_USTRINGPARAM( GLOBALSETTINGS_PROPERTY_STATESENABLED )),
    m_aPropLocked( RTL_CONSTASCII_USTRINGPARAM( GLOBALSETTINGS_PROPERTY_LOCKED )),
    m_aPropDocked( RTL_CONSTASCII_USTRINGPARAM( GLOBALSETTINGS_PROPERTY_DOCKED )),
    m_xContext( rxContext )
{
}

GlobalSettings_Access::~GlobalSettings_Access()
{
}

// XComponent
void SAL_CALL GlobalSettings_Access::dispose()
throw ( css::uno::RuntimeException )
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    m_xConfigAccess.clear();
    m_bDisposed = sal_True;
}

void SAL_CALL GlobalSettings_Access::addEventListener( const css::uno::Reference< css::lang::XEventListener >& )
throw (css::uno::RuntimeException)
{
}

void SAL_CALL GlobalSettings_Access::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& )
throw (css::uno::RuntimeException)
{
}

// XEventListener
void SAL_CALL GlobalSettings_Access::disposing( const css::lang::EventObject& )
throw (css::uno::RuntimeException)
{
    // SAFE
    ResetableGuard aLock( m_aLock );
    m_xConfigAccess.clear();
}

// settings access
sal_Bool GlobalSettings_Access::HasStatesInfo( GlobalSettings::UIElementType eElementType )
{
    ResetableGuard aLock( m_aLock );
    if ( eElementType == GlobalSettings::UIELEMENT_TYPE_DOCKWINDOW )
        return sal_False;
    else if ( eElementType == GlobalSettings::UIELEMENT_TYPE_STATUSBAR )
        return sal_False;

    if ( m_bDisposed )
        return sal_False;

    if ( !m_bConfigRead )
    {
        m_bConfigRead = sal_True;
        impl_initConfigAccess();
    }

    if ( m_xConfigAccess.is() )
    {
        try
        {
            css::uno::Any a;
            sal_Bool      bValue = sal_Bool();
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

    return sal_False;
}

sal_Bool GlobalSettings_Access::GetStateInfo( GlobalSettings::UIElementType eElementType, GlobalSettings::StateInfo eStateInfo, ::com::sun::star::uno::Any& aValue )
{
    ResetableGuard aLock( m_aLock );
    if ( eElementType == GlobalSettings::UIELEMENT_TYPE_DOCKWINDOW )
        return sal_False;
    else if ( eElementType == GlobalSettings::UIELEMENT_TYPE_STATUSBAR )
        return sal_False;

    if ( m_bDisposed )
        return sal_False;

    if ( !m_bConfigRead )
    {
        m_bConfigRead = sal_True;
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
                return sal_True;
            }
        }
        catch ( const css::container::NoSuchElementException& )
        {
        }
        catch ( const css::uno::Exception& )
        {
        }
    }

    return sal_False;
}

sal_Bool GlobalSettings_Access::impl_initConfigAccess()
{
    css::uno::Sequence< css::uno::Any > aArgs( 2 );
    css::beans::PropertyValue           aPropValue;

    try
    {
        if ( m_xContext.is() )
        {
            css::uno::Reference< css::lang::XMultiServiceFactory > xConfigProvider =
                 css::configuration::theDefaultProvider::get( m_xContext );

            aPropValue.Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "nodepath" ));
            aPropValue.Value = css::uno::makeAny( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( GLOBALSETTINGS_ROOT_ACCESS )));
            aArgs[0] = css::uno::makeAny( aPropValue );
            aPropValue.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "lazywrite" ));
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

    return sal_False;
}

//*****************************************************************************************************************
//  global class
//*****************************************************************************************************************

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
sal_Bool GlobalSettings::HasStatesInfo( UIElementType eElementType )
{
    GlobalSettings_Access* pSettings( GetGlobalSettings( m_xContext ));

    if ( pSettings )
        return pSettings->HasStatesInfo( eElementType );
    else
        return sal_False;
}

sal_Bool GlobalSettings::GetStateInfo( UIElementType eElementType, StateInfo eStateInfo, ::com::sun::star::uno::Any& aValue )
{
    GlobalSettings_Access* pSettings( GetGlobalSettings( m_xContext ));

    if ( pSettings )
        return pSettings->GetStateInfo( eElementType, eStateInfo, aValue );
    else
        return sal_False;
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
