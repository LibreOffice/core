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

#include <cppuhelper/supportsservice.hxx>
#include <svtools/popupmenucontrollerbase.hxx>
#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>

using namespace css;
using namespace com::sun::star::uno;
using namespace com::sun::star::frame;

namespace {

class SaveAsMenuController :  public svt::PopupMenuControllerBase
{
public:
    explicit SaveAsMenuController( const uno::Reference< uno::XComponentContext >& xContext );
    virtual ~SaveAsMenuController();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override
    {
        return OUString("com.sun.star.comp.framework.SaveAsMenuController");
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override
    {
        css::uno::Sequence< OUString > aSeq { "com.sun.star.frame.PopupMenuController" };
        return aSeq;
    }

    // XStatusListener
    virtual void SAL_CALL statusChanged( const frame::FeatureStateEvent& Event ) throw ( uno::RuntimeException, std::exception ) override;

private:
    virtual void impl_setPopupMenu() override;
};

SaveAsMenuController::SaveAsMenuController( const uno::Reference< uno::XComponentContext >& xContext ) :
    svt::PopupMenuControllerBase( xContext )
{
}

SaveAsMenuController::~SaveAsMenuController()
{
}

void SaveAsMenuController::impl_setPopupMenu()
{
    VCLXMenu* pPopupMenu    = VCLXMenu::GetImplementation( m_xPopupMenu );
    Menu*     pVCLPopupMenu = nullptr;

    SolarMutexGuard aSolarMutexGuard;

    if ( pPopupMenu )
        pVCLPopupMenu = pPopupMenu->GetMenu();

    if ( pVCLPopupMenu )
    {
        pVCLPopupMenu->InsertItem( ".uno:SaveAs", m_xFrame );
        pVCLPopupMenu->InsertItem( ".uno:SaveAsRemote", m_xFrame );
    }
}

// XStatusListener
void SAL_CALL SaveAsMenuController::statusChanged( const FeatureStateEvent& /*Event*/ ) throw ( RuntimeException, std::exception )
{
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_framework_SaveAsMenuController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SaveAsMenuController(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
