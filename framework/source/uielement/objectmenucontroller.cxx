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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"
#include <uielement/objectmenucontroller.hxx>

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <threadhelp/resetableguard.hxx>
#include "services.h"

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>

#ifndef _COM_SUN_STAR_EMBED_VERBDATTRIBUTES_HPP_
#include <com/sun/star/embed/VerbAttributes.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _VCL_MENU_HXX_
#include <vcl/menu.hxx>
#endif
#include <vcl/svapp.hxx>
#include <vcl/i18nhelp.hxx>
#include <tools/urlobj.hxx>
#include <rtl/ustrbuf.hxx>
#include <dispatch/uieventloghelper.hxx>
#include <vos/mutex.hxx>

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
    svt::PopupMenuControllerBase( xServiceManager )
{
}

ObjectMenuController::~ObjectMenuController()
{
}

// private function
void ObjectMenuController::fillPopupMenu( const Sequence< com::sun::star::embed::VerbDescriptor >& rVerbCommandSeq, Reference< css::awt::XPopupMenu >& rPopupMenu )
{
    const com::sun::star::embed::VerbDescriptor* pVerbCommandArray = rVerbCommandSeq.getConstArray();
    VCLXPopupMenu*                                     pPopupMenu        = (VCLXPopupMenu *)VCLXMenu::GetImplementation( rPopupMenu );
    PopupMenu*                                         pVCLPopupMenu     = 0;

    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

    resetPopupMenu( rPopupMenu );
    if ( pPopupMenu )
        pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();

    if ( pVCLPopupMenu )
    {
        const rtl::OUString aVerbCommand( RTL_CONSTASCII_USTRINGPARAM( ".uno:ObjectMenue?VerbID:short=" ));
        for ( sal_uInt16 i = 0; i < rVerbCommandSeq.getLength(); i++ )
        {
            const com::sun::star::embed::VerbDescriptor& rVerb = pVerbCommandArray[i];
            if ( rVerb.VerbAttributes & com::sun::star::embed::VerbAttributes::MS_VERBATTR_ONCONTAINERMENU )
            {
                m_xPopupMenu->insertItem( i+1, rVerb.VerbName, 0, i );
                // use VCL popup menu pointer to set vital information that are not part of the awt implementation

                rtl::OUString aCommand( aVerbCommand );
                aCommand += rtl::OUString::valueOf( rVerb.VerbID );
                pVCLPopupMenu->SetItemCommand( i+1, aCommand ); // Store verb command
            }
        }
    }
}

// XEventListener
void SAL_CALL ObjectMenuController::disposing( const EventObject& ) throw ( RuntimeException )
{
    Reference< css::awt::XMenuListener > xHolder(( OWeakObject *)this, UNO_QUERY );

    osl::MutexGuard aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();
    m_xObjectUpdateDispatch.clear();
    m_xServiceManager.clear();

    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(( OWeakObject *)this, UNO_QUERY ));
    m_xPopupMenu.clear();
}

// XStatusListener
void SAL_CALL ObjectMenuController::statusChanged( const FeatureStateEvent& Event ) throw ( RuntimeException )
{
    Sequence < com::sun::star::embed::VerbDescriptor > aVerbCommandSeq;
    if ( Event.State >>= aVerbCommandSeq )
    {
        osl::MutexGuard aLock( m_aMutex );
        if ( m_xPopupMenu.is() )
            fillPopupMenu( aVerbCommandSeq, m_xPopupMenu );
    }
}

// XMenuListener
void ObjectMenuController::impl_select(const Reference< XDispatch >& _xDispatch,const ::com::sun::star::util::URL& aTargetURL)
{
    Sequence<PropertyValue>      aArgs;
    if(::comphelper::UiEventsLogger::isEnabled()) //#i88653#
        UiEventLogHelper(::rtl::OUString::createFromAscii("ObjectMenuController")).log(m_xServiceManager, m_xFrame, aTargetURL, aArgs);
    OSL_ENSURE(_xDispatch.is(),"ObjectMenuController::impl_select: No dispatch");
    if ( _xDispatch.is() )
        _xDispatch->dispatch( aTargetURL, aArgs );
}

}
