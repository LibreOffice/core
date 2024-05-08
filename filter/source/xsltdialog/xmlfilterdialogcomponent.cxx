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

#include <comphelper/servicehelper.hxx>
#include <cppuhelper/factory.hxx>
#include <comphelper/compbase.hxx>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <vcl/svapp.hxx>

#include "xmlfiltersettingsdialog.hxx"

using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;

namespace {


class XMLFilterDialogComponent : public comphelper::WeakComponentImplHelper<
                                    css::ui::dialogs::XExecutableDialog,
                                    XServiceInfo,
                                    XInitialization,
                                    XTerminateListener>
{
public:
    explicit XMLFilterDialogComponent( const Reference< XComponentContext >& rxContext );

protected:
    // XTypeProvider
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // XExecutableDialog
    virtual void SAL_CALL setTitle( const OUString& aTitle ) override;
    virtual sal_Int16 SAL_CALL execute(  ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) override;

    // XTerminateListener
    virtual void SAL_CALL queryTermination( const EventObject& Event ) override;
    virtual void SAL_CALL notifyTermination( const EventObject& Event ) override;
    virtual void SAL_CALL disposing( const EventObject& Source ) override;

    /** Called in dispose method after the listeners were notified.
    */
    virtual void disposing(std::unique_lock<std::mutex>& rGuard) override;

private:
    css::uno::Reference<css::awt::XWindow>   mxParent;  /// parent window
    css::uno::Reference< XComponentContext > mxContext;

    std::shared_ptr<XMLFilterSettingsDialog> mxDialog;
};

}

XMLFilterDialogComponent::XMLFilterDialogComponent(const css::uno::Reference< XComponentContext >& rxContext)
    : mxContext(rxContext)
{
    Reference< XDesktop2 > xDesktop = Desktop::create( rxContext );
    Reference< XTerminateListener > xListener( this );
    xDesktop->addTerminateListener( xListener );
}

OUString SAL_CALL XMLFilterDialogComponent::getImplementationName()
{
    return u"com.sun.star.comp.ui.XSLTFilterDialog"_ustr;
}

Sequence< sal_Int8 > SAL_CALL XMLFilterDialogComponent::getImplementationId()
{
    static const comphelper::UnoIdInit implId;
    return implId.getSeq();
}


Sequence< OUString > SAL_CALL XMLFilterDialogComponent::getSupportedServiceNames()
{
    return { u"com.sun.star.ui.dialogs.XSLTFilterDialog"_ustr };
}

sal_Bool SAL_CALL XMLFilterDialogComponent::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService( this, ServiceName );
}

/** Called in dispose method after the listeners were notified.
*/
void XMLFilterDialogComponent::disposing(std::unique_lock<std::mutex>& rGuard)
{
    rGuard.unlock();
    {
        ::SolarMutexGuard aGuard;

        if (mxDialog)
            mxDialog->response(RET_CLOSE);
    }
    rGuard.lock();
}


// XTerminateListener
void SAL_CALL XMLFilterDialogComponent::queryTermination( const EventObject& /* Event */ )
{
    ::SolarMutexGuard aGuard;
    if (!mxDialog)
        return;
    mxDialog->present();
}

void SAL_CALL XMLFilterDialogComponent::notifyTermination( const EventObject& /* Event */ )
{
    {
        ::SolarMutexGuard aGuard;
        if (mxDialog)
            mxDialog->response(RET_CLOSE);
    }

    // we are going down, so dispose us!
    dispose();
}

void SAL_CALL XMLFilterDialogComponent::disposing( const EventObject& /* Source */ )
{
}

void SAL_CALL XMLFilterDialogComponent::setTitle( const OUString& /* _rTitle */ )
{
}

sal_Int16 SAL_CALL XMLFilterDialogComponent::execute()
{
    ::SolarMutexGuard aGuard;

    bool bLaunch = false;
    if (!mxDialog)
    {
        Reference< XComponent > xKeepAlive( this );
        mxDialog = std::make_shared<XMLFilterSettingsDialog>(Application::GetFrameWeld(mxParent), mxContext);
        bLaunch = true;
    }

    mxDialog->UpdateWindow();

    if (!bLaunch)
    {
        mxDialog->present();
        return 0;
    }

    weld::DialogController::runAsync(mxDialog, [this](sal_Int32)
    {
        mxDialog.reset();
    });

    return 0;
}

void SAL_CALL XMLFilterDialogComponent::initialize( const Sequence< Any >& aArguments )
{
    for(const Any& rArgument : aArguments)
    {
        PropertyValue aProperty;
        if(rArgument >>= aProperty)
        {
            if( aProperty.Name == "ParentWindow" )
            {
                aProperty.Value >>= mxParent;
            }
        }
    }
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
filter_XSLTFilterDialog_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new XMLFilterDialogComponent(context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
