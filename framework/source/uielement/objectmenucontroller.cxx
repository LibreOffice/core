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

#include <com/sun/star/embed/VerbAttributes.hpp>
#include <com/sun/star/embed/VerbDescriptor.hpp>

#include <svtools/popupmenucontrollerbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <toolkit/awt/vclxmenu.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;

namespace {

class ObjectMenuController :  public svt::PopupMenuControllerBase
{
    using svt::PopupMenuControllerBase::disposing;

public:
    explicit ObjectMenuController( const css::uno::Reference< css::uno::XComponentContext >& xContext );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override
    {
        return u"com.sun.star.comp.framework.ObjectMenuController"_ustr;
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return {u"com.sun.star.frame.PopupMenuController"_ustr};
    }

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) override;

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

private:
    void fillPopupMenu( const css::uno::Sequence< css::embed::VerbDescriptor >& rVerbCommandSeq, css::uno::Reference< css::awt::XPopupMenu > const & rPopupMenu );
};

ObjectMenuController::ObjectMenuController( const css::uno::Reference< css::uno::XComponentContext >& xContext ) :
    svt::PopupMenuControllerBase( xContext )
{
}

// private function
void ObjectMenuController::fillPopupMenu( const Sequence< css::embed::VerbDescriptor >& rVerbCommandSeq, Reference< css::awt::XPopupMenu > const & rPopupMenu )
{
    const css::embed::VerbDescriptor* pVerbCommandArray = rVerbCommandSeq.getConstArray();

    SolarMutexGuard aSolarMutexGuard;

    resetPopupMenu( rPopupMenu );

    static constexpr OUStringLiteral aVerbCommand( u".uno:ObjectMenue?VerbID:short=" );
    for ( sal_Int32 i = 0; i < rVerbCommandSeq.getLength(); i++ )
    {
        const css::embed::VerbDescriptor& rVerb = pVerbCommandArray[i];
        if ( rVerb.VerbAttributes & css::embed::VerbAttributes::MS_VERBATTR_ONCONTAINERMENU )
        {
            m_xPopupMenu->insertItem( i+1, rVerb.VerbName, 0, i );
            OUString aCommand = aVerbCommand + OUString::number( rVerb.VerbID );
            m_xPopupMenu->setCommand( i+1, aCommand ); // Store verb command
        }
    }
}

// XEventListener
void SAL_CALL ObjectMenuController::disposing( const EventObject& )
{
    Reference< css::awt::XMenuListener > xHolder(this);

    std::unique_lock aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();

    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(this) );
    m_xPopupMenu.clear();
}

// XStatusListener
void SAL_CALL ObjectMenuController::statusChanged( const FeatureStateEvent& Event )
{
    Sequence < css::embed::VerbDescriptor > aVerbCommandSeq;
    if ( Event.State >>= aVerbCommandSeq )
    {
        std::unique_lock aLock( m_aMutex );
        if ( m_xPopupMenu.is() )
            fillPopupMenu( aVerbCommandSeq, m_xPopupMenu );
    }
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_framework_ObjectMenuController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ObjectMenuController(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
