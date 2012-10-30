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


#include "imestatuswindow.hxx"

#include <sfx2/app.hxx>
#include <sfx2/sfxsids.hrc>

#include "com/sun/star/beans/PropertyState.hpp"
#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/configuration/theDefaultProvider.hpp"
#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/util/XChangesBatch.hpp"
#include "osl/diagnose.h"
#include "osl/mutex.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "vcl/svapp.hxx"

//TO-Do, merge into framework/inc/helpers/mischelpers.hxx and deliver
class WeakPropertyChangeListener : public ::cppu::WeakImplHelper1<com::sun::star::beans::XPropertyChangeListener>
{
    private:
        com::sun::star::uno::WeakReference<com::sun::star::beans::XPropertyChangeListener> mxOwner;

    public:
        WeakPropertyChangeListener(com::sun::star::uno::Reference<com::sun::star::beans::XPropertyChangeListener> xOwner)
            : mxOwner(xOwner)
        {
        }

        virtual ~WeakPropertyChangeListener()
        {
        }

        virtual void SAL_CALL propertyChange(const com::sun::star::beans::PropertyChangeEvent &rEvent )
            throw(com::sun::star::uno::RuntimeException)
        {
            com::sun::star::uno::Reference<com::sun::star::beans::XPropertyChangeListener> xOwner(mxOwner.get(),
                com::sun::star::uno::UNO_QUERY);
            if (xOwner.is())
                xOwner->propertyChange(rEvent);

        }

        // lang.XEventListener
        virtual void SAL_CALL disposing(const com::sun::star::lang::EventObject& rEvent)
            throw(com::sun::star::uno::RuntimeException)
        {
            com::sun::star::uno::Reference<com::sun::star::beans::XPropertyChangeListener> xOwner(mxOwner.get(),
                com::sun::star::uno::UNO_QUERY);
            if (xOwner.is())
                xOwner->disposing(rEvent);

        }
};

namespace css = com::sun::star;

using sfx2::appl::ImeStatusWindow;

ImeStatusWindow::ImeStatusWindow(
    css::uno::Reference< css::uno::XComponentContext > const & rxContext):
    m_xContext(rxContext),
    m_bDisposed(false)
{}

void ImeStatusWindow::init()
{
    if (Application::CanToggleImeStatusWindow())
        try
        {
            sal_Bool bShow = sal_Bool();
            if (getConfig()->getPropertyValue(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                      "ShowStatusWindow")))
                >>= bShow)
                Application::ShowImeStatusWindow(bShow);
        }
        catch (css::uno::Exception &)
        {
            OSL_FAIL("com.sun.star.uno.Exception");
            // Degrade gracefully and use the VCL-supplied default if no
            // configuration is available.
        }
}

bool ImeStatusWindow::isShowing()
{
    try
    {
        sal_Bool bShow = sal_Bool();
        if (getConfig()->getPropertyValue(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowStatusWindow")))
            >>= bShow)
            return bShow;
    }
    catch (css::uno::Exception &)
    {
        OSL_FAIL("com.sun.star.uno.Exception");
        // Degrade gracefully and use the VCL-supplied default if no
        // configuration is available.
    }
    return Application::GetShowImeStatusWindowDefault();
}

void ImeStatusWindow::show(bool bShow)
{
    try
    {
        css::uno::Reference< css::beans::XPropertySet > xConfig(getConfig());
        xConfig->setPropertyValue(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowStatusWindow")),
            css::uno::makeAny(static_cast< sal_Bool >(bShow)));
        css::uno::Reference< css::util::XChangesBatch > xCommit(
            xConfig, css::uno::UNO_QUERY);
        // Degrade gracefully by not saving the settings permanently:
        if (xCommit.is())
            xCommit->commitChanges();
        // Alternatively, setting the VCL status could be done even if updating
        // the configuration failed:
        Application::ShowImeStatusWindow(bShow);
    }
    catch (css::uno::Exception &)
    {
        OSL_FAIL("com.sun.star.uno.Exception");
    }
}

bool ImeStatusWindow::canToggle() const
{
    return Application::CanToggleImeStatusWindow();
}

ImeStatusWindow::~ImeStatusWindow()
{
    if (m_xConfig.is() && m_xConfigListener.is())
        // We should never get here, but just in case...
        try
        {
            m_xConfig->removePropertyChangeListener(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowStatusWindow")),
                m_xConfigListener);
        }
        catch (css::uno::Exception &)
        {
            OSL_FAIL("com.sun.star.uno.RuntimeException");
        }
}

void SAL_CALL ImeStatusWindow::disposing(css::lang::EventObject const & )
    throw (css::uno::RuntimeException)
{
    osl::MutexGuard aGuard(m_aMutex);
    m_xConfig = 0;
    m_bDisposed = true;
}

void SAL_CALL
ImeStatusWindow::propertyChange(css::beans::PropertyChangeEvent const & )
    throw (css::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SfxApplication* pApp = SfxApplication::Get();
    if (pApp)
    pApp->Invalidate(SID_SHOW_IME_STATUS_WINDOW);
}

css::uno::Reference< css::beans::XPropertySet > ImeStatusWindow::getConfig()
{
    css::uno::Reference< css::beans::XPropertySet > xConfig;
    bool bAdd = false;
    {
        osl::MutexGuard aGuard(m_aMutex);
        if (!m_xConfig.is())
        {
            if (m_bDisposed)
                throw css::lang::DisposedException();
            if (!m_xContext.is())
                throw css::uno::RuntimeException(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "null comphelper::getProcessServiceFactory")),
                    0);
            css::uno::Reference< css::lang::XMultiServiceFactory > xProvider =
                css::configuration::theDefaultProvider::get( m_xContext );
            css::beans::PropertyValue aArg(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("nodepath")), -1,
                css::uno::makeAny(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "/org.openoffice.Office.Common/I18N/InputMethod"))),
                css::beans::PropertyState_DIRECT_VALUE);
            css::uno::Sequence< css::uno::Any > aArgs(1);
            aArgs[0] <<= aArg;
            m_xConfig
                = css::uno::Reference< css::beans::XPropertySet >(
                    xProvider->createInstanceWithArguments(
                        rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                       "com.sun.star.configuration.ConfigurationUpdateAccess")),
                        aArgs),
                    css::uno::UNO_QUERY);
            if (!m_xConfig.is())
                throw css::uno::RuntimeException(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                      "null com.sun.star.configuration."
                                      "ConfigurationUpdateAccess")),
                    0);
            bAdd = true;
        }
        xConfig = m_xConfig;
    }
    if (bAdd)
    {
        // Exceptions here could be handled individually, to support graceful
        // degradation (no update notification mechanism in this case---but also
        // no dispose notifications):
        m_xConfigListener = new WeakPropertyChangeListener(this);
        xConfig->addPropertyChangeListener(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowStatusWindow")),
            m_xConfigListener);
    }
    return xConfig;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
