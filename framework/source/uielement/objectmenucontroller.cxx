/*************************************************************************
 *
 *  $RCSfile: objectmenucontroller.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2004-06-10 13:24:23 $
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

#ifndef __FRAMEWORK_UIELEMENT_OBJECTMENUCONTROLLER_HXX_
#include <uielement/objectmenucontroller.hxx>
#endif

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include "services.h"
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_AWT_XDEVICE_HPP_
#include <com/sun/star/awt/XDevice.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_MENUITEMSTYLE_HPP_
#include <com/sun/star/awt/MenuItemStyle.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _VCL_MENU_HXX_
#include <vcl/menu.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VCL_I18NHELP_HXX
#include <vcl/i18nhelp.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________
//

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;

namespace framework
{

DEFINE_XSERVICEINFO_MULTISERVICE        (   ObjectMenuController                    ,
                                            OWeakObject                             ,
                                            SERVICENAME_POPUPMENUCONTROLLER         ,
                                            IMPLEMENTATIONNAME_OBJECTMENUCONTROLLER
                                        )

DEFINE_INIT_SERVICE                     (   ObjectMenuController, {} )

ObjectMenuController::ObjectMenuController( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager ) :
    PopupMenuControllerBase( xServiceManager )
{
}

ObjectMenuController::~ObjectMenuController()
{
}

// private function
void ObjectMenuController::fillPopupMenu( const Sequence< drafts::com::sun::star::frame::status::Verb >& rVerbCommandSeq, Reference< css::awt::XPopupMenu >& rPopupMenu )
{
    const drafts::com::sun::star::frame::status::Verb* pVerbCommandArray = rVerbCommandSeq.getConstArray();
    VCLXPopupMenu*                                     pPopupMenu        = (VCLXPopupMenu *)VCLXMenu::GetImplementation( rPopupMenu );
    PopupMenu*                                         pVCLPopupMenu     = 0;

    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

    resetPopupMenu( rPopupMenu );
    if ( pPopupMenu )
        pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();

    if ( pVCLPopupMenu )
    {
        const rtl::OUString aVerbCommand( RTL_CONSTASCII_USTRINGPARAM( ".uno:ObjectMenue?VerbID:short=" ));
        for ( USHORT i = 0; i < rVerbCommandSeq.getLength(); i++ )
        {
            const drafts::com::sun::star::frame::status::Verb& rVerb = pVerbCommandArray[i];
            if ( rVerb.VerbIsOnMenu )
            {
                m_xPopupMenu->insertItem( i+1, rVerb.VerbName, 0, i );
                // use VCL popup menu pointer to set vital information that are not part of the awt implementation

                rtl::OUString aCommand( aVerbCommand );
                aCommand += rtl::OUString::valueOf( rVerb.VerbId );
                pVCLPopupMenu->SetItemCommand( i+1, aCommand ); // Store verb command
            }
        }
    }
}

// XEventListener
void SAL_CALL ObjectMenuController::disposing( const EventObject& Source ) throw ( RuntimeException )
{
    Reference< css::awt::XMenuListener > xHolder(( OWeakObject *)this, UNO_QUERY );

    ResetableGuard aLock( m_aLock );
    m_xFrame.clear();
    m_xDispatch.clear();
    m_xObjectUpdateDispatch.clear();

    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(( OWeakObject *)this, UNO_QUERY ));
    m_xPopupMenu.clear();
}

// XStatusListener
void SAL_CALL ObjectMenuController::statusChanged( const FeatureStateEvent& Event ) throw ( RuntimeException )
{
    Sequence< drafts::com::sun::star::frame::status::Verb >   aVerbCommandSeq;

    if ( Event.State >>= aVerbCommandSeq )
    {
        ResetableGuard aLock( m_aLock );
        if ( m_xPopupMenu.is() )
            fillPopupMenu( aVerbCommandSeq, m_xPopupMenu );
    }
}

// XMenuListener
void SAL_CALL ObjectMenuController::highlight( const css::awt::MenuEvent& rEvent ) throw (RuntimeException)
{
}

void SAL_CALL ObjectMenuController::select( const css::awt::MenuEvent& rEvent ) throw (RuntimeException)
{
    Reference< css::awt::XPopupMenu >   xPopupMenu;
    Reference< XDispatch >              xDispatch;
    Reference< XMultiServiceFactory >   xServiceManager;

    ResetableGuard aLock( m_aLock );
    xPopupMenu      = m_xPopupMenu;
    xDispatch       = m_xDispatch;
    xServiceManager = m_xServiceManager;
    aLock.unlock();

    if ( xPopupMenu.is() && xDispatch.is() )
    {
        VCLXPopupMenu* pPopupMenu = (VCLXPopupMenu *)VCLXPopupMenu::GetImplementation( xPopupMenu );
        if ( pPopupMenu )
        {
            css::util::URL               aTargetURL;
            Sequence<PropertyValue>      aArgs;
            Reference< XURLTransformer > xURLTransformer( xServiceManager->createInstance(
                                                            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                        UNO_QUERY );

            {
                vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
                PopupMenu* pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();

                aTargetURL.Complete = pVCLPopupMenu->GetItemCommand( rEvent.MenuId );
            }

            xURLTransformer->parseStrict( aTargetURL );
            xDispatch->dispatch( aTargetURL, aArgs );
        }
    }
}

void SAL_CALL ObjectMenuController::activate( const css::awt::MenuEvent& rEvent ) throw (RuntimeException)
{
}

void SAL_CALL ObjectMenuController::deactivate( const css::awt::MenuEvent& rEvent ) throw (RuntimeException)
{
}

// XPopupMenuController
void SAL_CALL ObjectMenuController::setPopupMenu( const Reference< css::awt::XPopupMenu >& xPopupMenu ) throw ( RuntimeException )
{
    ResetableGuard aLock( m_aLock );

    if ( m_xFrame.is() && !m_xPopupMenu.is() )
    {
        // Create popup menu on demand
        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

        m_xPopupMenu = xPopupMenu;
        m_xPopupMenu->addMenuListener( Reference< css::awt::XMenuListener >( (OWeakObject*)this, UNO_QUERY ));

        Reference< XURLTransformer > xURLTransformer( m_xServiceManager->createInstance(
                                                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                    UNO_QUERY );
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );

        com::sun::star::util::URL aTargetURL;
        aTargetURL.Complete = m_aCommandURL;
        xURLTransformer->parseStrict( aTargetURL );
        m_xDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );

        updatePopupMenu();
    }
}

// XInitialization
void SAL_CALL ObjectMenuController::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException )
{
    const rtl::OUString aFrameName( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
    const rtl::OUString aCommandURLName( RTL_CONSTASCII_USTRINGPARAM( "CommandURL" ));

    ResetableGuard aLock( m_aLock );

    sal_Bool bInitalized( m_bInitialized );
    if ( !bInitalized )
    {
        PropertyValue       aPropValue;
        rtl::OUString       aCommandURL;
        Reference< XFrame > xFrame;

        for ( int i = 0; i < aArguments.getLength(); i++ )
        {
            if ( aArguments[i] >>= aPropValue )
            {
                if ( aPropValue.Name.equalsAscii( "Frame" ))
                    aPropValue.Value >>= xFrame;
                else if ( aPropValue.Name.equalsAscii( "CommandURL" ))
                    aPropValue.Value >>= aCommandURL;
            }
        }

        if ( xFrame.is() && aCommandURL.getLength() )
        {
            ResetableGuard aLock( m_aLock );
            m_xFrame        = xFrame;
            m_aCommandURL   = aCommandURL;
            m_bInitialized = sal_True;
        }
    }
}

}
