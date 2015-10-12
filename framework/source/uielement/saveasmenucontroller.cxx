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

#include <classes/resource.hrc>
#include <classes/fwkresid.hxx>

#include <cppuhelper/supportsservice.hxx>
#include <osl/file.hxx>
#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <svtools/popupmenucontrollerbase.hxx>
#include <tools/urlobj.hxx>
#include <unotools/historyoptions.hxx>
#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>

using namespace css;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace framework;

namespace {

static const char CMD_SAVE_REMOTE[]  = ".uno:SaveAsRemote";

class SaveAsMenuController :  public svt::PopupMenuControllerBase
{
    using svt::PopupMenuControllerBase::disposing;

public:
    SaveAsMenuController( const uno::Reference< uno::XComponentContext >& xContext );
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
        css::uno::Sequence< OUString > aSeq(1);
        aSeq[0] = "com.sun.star.frame.PopupMenuController";
        return aSeq;
    }

    // XStatusListener
    virtual void SAL_CALL statusChanged( const frame::FeatureStateEvent& Event ) throw ( uno::RuntimeException, std::exception ) override;

    // XMenuListener
    virtual void SAL_CALL itemSelected( const awt::MenuEvent& rEvent ) throw (uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL itemActivated( const awt::MenuEvent& rEvent ) throw (uno::RuntimeException, std::exception) override;

    // XEventListener
    virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& Source ) throw ( uno::RuntimeException, std::exception ) override;

private:
    virtual void impl_setPopupMenu() override;

    void fillPopupMenu( com::sun::star::uno::Reference< com::sun::star::awt::XPopupMenu >& rPopupMenu );

    bool                  m_bDisabled : 1;
};

SaveAsMenuController::SaveAsMenuController( const uno::Reference< uno::XComponentContext >& xContext ) :
    svt::PopupMenuControllerBase( xContext ),
    m_bDisabled( false )
{
}

SaveAsMenuController::~SaveAsMenuController()
{
}

// private function
void SaveAsMenuController::fillPopupMenu( Reference< css::awt::XPopupMenu >& rPopupMenu )
{
    VCLXPopupMenu* pPopupMenu    = static_cast<VCLXPopupMenu *>(VCLXMenu::GetImplementation( rPopupMenu ));
    PopupMenu*     pVCLPopupMenu = 0;

    SolarMutexGuard aSolarMutexGuard;

    resetPopupMenu( rPopupMenu );
    if ( pPopupMenu )
        pVCLPopupMenu = static_cast<PopupMenu *>(pPopupMenu->GetMenu());

    if ( pVCLPopupMenu )
    {
        // Open remote menu entry
        pVCLPopupMenu->InsertItem( sal_uInt16( 1 ),
                                   FWK_RESSTR( STR_REMOTE_FILE ) );
        pVCLPopupMenu->SetItemCommand( sal_uInt16( 1 ),
                                       OUString( CMD_SAVE_REMOTE ) );
    }
}

// XEventListener
void SAL_CALL SaveAsMenuController::disposing( const EventObject& ) throw ( RuntimeException, std::exception )
{
    Reference< css::awt::XMenuListener > xHolder(static_cast<OWeakObject *>(this), UNO_QUERY );

    osl::MutexGuard aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();

    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(static_cast<OWeakObject *>(this), UNO_QUERY ));
    m_xPopupMenu.clear();
}

// XStatusListener
void SAL_CALL SaveAsMenuController::statusChanged( const FeatureStateEvent& Event ) throw ( RuntimeException, std::exception )
{
    osl::MutexGuard aLock( m_aMutex );
    m_bDisabled = !Event.IsEnabled;
}

void SAL_CALL SaveAsMenuController::itemSelected( const css::awt::MenuEvent& rEvent ) throw (RuntimeException, std::exception)
{
    Reference< css::awt::XPopupMenu > xPopupMenu;

    osl::ClearableMutexGuard aLock( m_aMutex );
    xPopupMenu = m_xPopupMenu;
    aLock.clear();

    if ( xPopupMenu.is() )
    {
        const OUString aCommand( xPopupMenu->getCommand( rEvent.MenuId ) );
        OSL_TRACE( "SaveAsMenuController::itemSelected() - Command : %s",
                   OUStringToOString( aCommand, RTL_TEXTENCODING_UTF8 ).getStr() );

        if ( aCommand == CMD_SAVE_REMOTE )
        {
            Sequence< PropertyValue > aArgsList( 0 );

            dispatchCommand( CMD_SAVE_REMOTE, aArgsList );
        }
    }
}

void SAL_CALL SaveAsMenuController::itemActivated( const css::awt::MenuEvent& ) throw (RuntimeException, std::exception)
{
    osl::MutexGuard aLock( m_aMutex );
    impl_setPopupMenu();
}

// XPopupMenuController
void SaveAsMenuController::impl_setPopupMenu()
{
    if ( m_xPopupMenu.is() )
        fillPopupMenu( m_xPopupMenu );
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
