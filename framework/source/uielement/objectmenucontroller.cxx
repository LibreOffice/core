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

#include <stdtypes.h>

#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>

#include <com/sun/star/embed/VerbAttributes.hpp>
#include <com/sun/star/embed/VerbDescriptor.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <svtools/popupmenucontrollerbase.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>
#include <vcl/i18nhelp.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/mutex.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace framework;

namespace {

class ObjectMenuController :  public svt::PopupMenuControllerBase
{
    using svt::PopupMenuControllerBase::disposing;

public:
    ObjectMenuController( const css::uno::Reference< css::uno::XComponentContext >& xContext );
    virtual ~ObjectMenuController();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    {
        return OUString("com.sun.star.comp.framework.ObjectMenuController");
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    {
        css::uno::Sequence< OUString > aSeq(1);
        aSeq[0] = "com.sun.star.frame.PopupMenuController";
        return aSeq;
    }

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

private:
    void fillPopupMenu( const css::uno::Sequence< css::embed::VerbDescriptor >& rVerbCommandSeq, css::uno::Reference< css::awt::XPopupMenu >& rPopupMenu );
    virtual void impl_select(const css::uno::Reference< css::frame::XDispatch >& _xDispatch,const css::util::URL& aURL) SAL_OVERRIDE;

    css::uno::Reference< css::frame::XDispatch >  m_xObjectUpdateDispatch;
};

ObjectMenuController::ObjectMenuController( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext ) :
    svt::PopupMenuControllerBase( xContext )
{
}

ObjectMenuController::~ObjectMenuController()
{
}

// private function
void ObjectMenuController::fillPopupMenu( const Sequence< com::sun::star::embed::VerbDescriptor >& rVerbCommandSeq, Reference< css::awt::XPopupMenu >& rPopupMenu )
{
    const css::embed::VerbDescriptor* pVerbCommandArray = rVerbCommandSeq.getConstArray();
    VCLXPopupMenu*                    pPopupMenu        = static_cast<VCLXPopupMenu *>(VCLXMenu::GetImplementation( rPopupMenu ));
    PopupMenu*                        pVCLPopupMenu     = 0;

    SolarMutexGuard aSolarMutexGuard;

    resetPopupMenu( rPopupMenu );
    if ( pPopupMenu )
        pVCLPopupMenu = static_cast<PopupMenu *>(pPopupMenu->GetMenu());

    if ( pVCLPopupMenu )
    {
        const OUString aVerbCommand( ".uno:ObjectMenue?VerbID:short=" );
        for ( sal_Int32 i = 0; i < rVerbCommandSeq.getLength(); i++ )
        {
            const com::sun::star::embed::VerbDescriptor& rVerb = pVerbCommandArray[i];
            if ( rVerb.VerbAttributes & com::sun::star::embed::VerbAttributes::MS_VERBATTR_ONCONTAINERMENU )
            {
                m_xPopupMenu->insertItem( i+1, rVerb.VerbName, 0, i );
                // use VCL popup menu pointer to set vital information that are not part of the awt implementation

                OUString aCommand( aVerbCommand );
                aCommand += OUString::number( rVerb.VerbID );
                pVCLPopupMenu->SetItemCommand( i+1, aCommand ); // Store verb command
            }
        }
    }
}

// XEventListener
void SAL_CALL ObjectMenuController::disposing( const EventObject& ) throw ( RuntimeException, std::exception )
{
    Reference< css::awt::XMenuListener > xHolder(( OWeakObject *)this, UNO_QUERY );

    osl::MutexGuard aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();
    m_xObjectUpdateDispatch.clear();

    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(( OWeakObject *)this, UNO_QUERY ));
    m_xPopupMenu.clear();
}

// XStatusListener
void SAL_CALL ObjectMenuController::statusChanged( const FeatureStateEvent& Event ) throw ( RuntimeException, std::exception )
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
    OSL_ENSURE(_xDispatch.is(),"ObjectMenuController::impl_select: No dispatch");
    if ( _xDispatch.is() )
        _xDispatch->dispatch( aTargetURL, aArgs );
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_framework_ObjectMenuController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ObjectMenuController(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
