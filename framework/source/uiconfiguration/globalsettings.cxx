/*************************************************************************
 *
 *  $RCSfile: globalsettings.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-11 12:07:10 $
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

#ifndef __FRAMEWORK_UICONFIGURATION_GLOBALSETTINGS_HXX_
#include "uiconfiguration/globalsettings.hxx"
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
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef INCLUDED_RTL_INSTANCE_HXX
#include <rtl/instance.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________
//

using namespace rtl;
using namespace ::com::sun::star;

//_________________________________________________________________________________________________________________
//  Namespace
//_________________________________________________________________________________________________________________
//

static const char GLOBALSETTINGS_ROOT_ACCESS[]              = "/org.openoffice.Office.UI.GlobalSettings/Toolbars";

static const char GLOBALSETTINGS_NODEREF_STATES[]           = "States";
static const char GLOBALSETTINGS_PROPERTY_LOCKED[]          = "Locked";
static const char GLOBALSETTINGS_PROPERTY_DOCKED[]          = "Docked";
static const char GLOBALSETTINGS_PROPERTY_STATESENABLED[]   = "StatesEnabled";

// Order must be the same as WindowStateMask!!
static const char* CONFIGURATION_PROPERTIES[]           =
{
    "Toolbars",
    "DockWindow",
    "Statusbar",
    0
};

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
        GlobalSettings_Access( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );
        virtual ~GlobalSettings_Access();

        // XInterface, XTypeProvider, XServiceInfo
        DECLARE_XINTERFACE

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
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xServiceManager;
};


//*****************************************************************************************************************
//  XInterface
//*****************************************************************************************************************
DEFINE_XINTERFACE_2     (   GlobalSettings_Access                           ,
                            OWeakObject                                     ,
                            DIRECT_INTERFACE ( css::lang::XComponent        ),
                            DIRECT_INTERFACE ( css::lang::XEventListener    )
                        )

GlobalSettings_Access::GlobalSettings_Access( const css::uno::Reference< css::lang::XMultiServiceFactory >& rServiceManager ) :
    ThreadHelpBase(),
    m_xServiceManager( rServiceManager ),
    m_bConfigRead( sal_False ),
    m_bDisposed( sal_False ),
    m_aConfigSettingsAccess( RTL_CONSTASCII_USTRINGPARAM( GLOBALSETTINGS_ROOT_ACCESS )),
    m_aPropStatesEnabled( RTL_CONSTASCII_USTRINGPARAM( GLOBALSETTINGS_PROPERTY_STATESENABLED )),
    m_aPropLocked( RTL_CONSTASCII_USTRINGPARAM( GLOBALSETTINGS_PROPERTY_LOCKED )),
    m_aPropDocked( RTL_CONSTASCII_USTRINGPARAM( GLOBALSETTINGS_PROPERTY_DOCKED )),
    m_aNodeRefStates( RTL_CONSTASCII_USTRINGPARAM( GLOBALSETTINGS_NODEREF_STATES ))
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

void SAL_CALL GlobalSettings_Access::addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener )
throw (css::uno::RuntimeException)
{
}

void SAL_CALL GlobalSettings_Access::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener )
throw (css::uno::RuntimeException)
{
}

// XEventListener
void SAL_CALL GlobalSettings_Access::disposing( const css::lang::EventObject& Source )
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
            sal_Bool      bValue;
            a = m_xConfigAccess->getByName( m_aPropStatesEnabled );
            if ( a >>= bValue )
                return bValue;
        }
        catch ( css::container::NoSuchElementException& )
        {
        }
        catch ( css::uno::Exception& )
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
        catch ( css::container::NoSuchElementException& )
        {
        }
        catch ( css::uno::Exception& )
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
        css::uno::Reference< css::lang::XMultiServiceFactory > xConfigProvider;
        if ( m_xServiceManager.is() )
            xConfigProvider = css::uno::Reference< css::lang::XMultiServiceFactory >(
                                    m_xServiceManager->createInstance( SERVICENAME_CFGPROVIDER ),
                                    css::uno::UNO_QUERY );

        if ( xConfigProvider.is() )
        {
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

            css::uno::Reference< css::lang::XComponent > xComponent( xConfigProvider, css::uno::UNO_QUERY );
            if ( xComponent.is() )
                xComponent->addEventListener(
                    css::uno::Reference< css::lang::XEventListener >(
                        static_cast< cppu::OWeakObject* >( this ),
                        css::uno::UNO_QUERY ));
        }

        return m_xConfigAccess.is();
    }
    catch ( css::lang::WrappedTargetException& )
    {
    }
    catch ( css::uno::Exception& )
    {
    }

    return sal_False;
}

//*****************************************************************************************************************
//  global class
//*****************************************************************************************************************

struct mutexGlobalSettings : public rtl::Static< osl::Mutex, mutexGlobalSettings > {};
static GlobalSettings_Access* pStaticSettings = 0;

static GlobalSettings_Access* GetGlobalSettings( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rSrvMgr )
{
    osl::MutexGuard aGuard(mutexGlobalSettings::get());
    if ( !pStaticSettings )
        pStaticSettings = new GlobalSettings_Access( rSrvMgr );
    return pStaticSettings;
}

GlobalSettings::GlobalSettings( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rSrvMgr ) :
    m_xSrvMgr( rSrvMgr )
{
}

GlobalSettings::~GlobalSettings()
{
}

// settings access
sal_Bool GlobalSettings::HasStatesInfo( UIElementType eElementType )
{
    GlobalSettings_Access* pSettings( GetGlobalSettings( m_xSrvMgr ));

    if ( pSettings )
        return pSettings->HasStatesInfo( eElementType );
    else
        return sal_False;
}

sal_Bool GlobalSettings::GetStateInfo( UIElementType eElementType, StateInfo eStateInfo, ::com::sun::star::uno::Any& aValue )
{
    GlobalSettings_Access* pSettings( GetGlobalSettings( m_xSrvMgr ));

    if ( pSettings )
        return pSettings->GetStateInfo( eElementType, eStateInfo, aValue );
    else
        return sal_False;
}

} // namespace framework
