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

#include <svtools/strings.hrc>
#include <com/sun/star/task/XInteractionContinuation.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionRetry.hpp>
#include <com/sun/star/java/JavaNotFoundException.hpp>
#include <com/sun/star/java/InvalidJavaSettingsException.hpp>
#include <com/sun/star/java/JavaDisabledException.hpp>
#include <com/sun/star/java/JavaVMCreationFailureException.hpp>
#include <com/sun/star/java/RestartRequiredException.hpp>
#include <comphelper/processfactory.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <jvmfwk/framework.hxx>

#include <svtools/restartdialog.hxx>
#include <svtools/svtresid.hxx>
#include <svtools/javainteractionhandler.hxx>
#include <unotools/configmgr.hxx>
#include <officecfg/Office/Common.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::task;

namespace
{
struct JavaEvents {
    bool bDisabledHandled : 1;
    bool bInvalidSettingsHandled : 1;
    bool bNotFoundHandled : 1;
    bool bVMCreationFailureHandled : 1;
    bool bRestartRequiredHandled : 1;
    sal_uInt16 nResult_JavaDisabled = RET_NO;
    JavaEvents()
        : bDisabledHandled(false)
        , bInvalidSettingsHandled(false)
        , bNotFoundHandled(false)
        , bVMCreationFailureHandled(false)
        , bRestartRequiredHandled(false)
    {}
} g_JavaEvents;
}

namespace svt
{

JavaInteractionHandler::JavaInteractionHandler() :
    m_aRefCount(0)
{
}

JavaInteractionHandler::~JavaInteractionHandler()
{
}

Any SAL_CALL JavaInteractionHandler::queryInterface(const Type& aType )
{
    if (aType == cppu::UnoType<XInterface>::get())
        return Any( static_cast<XInterface*>(this), aType);
    else if (aType == cppu::UnoType<XInteractionHandler>::get())
        return Any( static_cast<XInteractionHandler*>(this), aType);
    return Any();
}

void SAL_CALL JavaInteractionHandler::acquire(  ) noexcept
{
    osl_atomic_increment( &m_aRefCount );
}

void SAL_CALL JavaInteractionHandler::release(  ) noexcept
{
    if (! osl_atomic_decrement( &m_aRefCount ))
        delete this;
}

void SAL_CALL JavaInteractionHandler::handle( const Reference< XInteractionRequest >& Request )
{
    Any anyExc = Request->getRequest();
    const Sequence< Reference< XInteractionContinuation > > aSeqCont = Request->getContinuations();

    Reference< XInteractionAbort > abort;
    Reference< XInteractionRetry > retry;

    for ( const auto& rCont : aSeqCont )
    {
        abort.set( rCont, UNO_QUERY );
        if ( abort.is() )
            break;
    }

    for ( const auto& rCont : aSeqCont )
    {
        retry.set( rCont, UNO_QUERY );
        if ( retry.is() )
            break;
    }

    css::java::JavaNotFoundException           e1;
    css::java::InvalidJavaSettingsException    e2;
    css::java::JavaDisabledException           e3;
    css::java::JavaVMCreationFailureException  e4;
    css::java::RestartRequiredException        e5;
    // Try to recover the Exception type in the any and
    // react accordingly.
    sal_uInt16      nResult = RET_CANCEL;

    if ( anyExc >>= e1 )
    {
        SolarMutexGuard aSolarGuard;
        if( !g_JavaEvents.bNotFoundHandled )
        {
           // No suitable JRE found
            OUString sPrimTex;
            OUString urlLink(officecfg::Office::Common::Menus::InstallJavaURL::get() + // https://hub.libreoffice.org/InstallJava/
                "?LOlocale=" + utl::ConfigManager::getUILocale());
            g_JavaEvents.bNotFoundHandled = true;
#if defined(MACOSX)
            sPrimTex = SvtResId(STR_WARNING_JAVANOTFOUND_MAC);
#elif defined(_WIN32)
            sPrimTex = SvtResId(STR_WARNING_JAVANOTFOUND_WIN);
#if defined(_WIN64)
            sPrimTex = sPrimTex.replaceAll("%BITNESS", "64");
#else
            sPrimTex = sPrimTex.replaceAll("%BITNESS", "32");
#endif
#else
            sPrimTex = SvtResId(STR_WARNING_JAVANOTFOUND);
#endif
            sPrimTex = sPrimTex.replaceAll("%FAQLINK", urlLink);
            std::unique_ptr<weld::MessageDialog> xWarningBox(Application::CreateMessageDialog(
                nullptr, VclMessageType::Warning, VclButtonsType::Ok, sPrimTex));
            xWarningBox->set_title(SvtResId(STR_WARNING_JAVANOTFOUND_TITLE));
            nResult = xWarningBox->run();
        }
        else
        {
            nResult = RET_OK;
        }
    }
    else if ( anyExc >>= e2 )
    {
        SolarMutexGuard aSolarGuard;
        if( !g_JavaEvents.bInvalidSettingsHandled )
        {
           // javavendors.xml was updated and Java has not been configured yet
            g_JavaEvents.bInvalidSettingsHandled = true;
#ifdef MACOSX
            OUString sWarning(SvtResId(STR_WARNING_INVALIDJAVASETTINGS_MAC));
#else
            OUString sWarning(SvtResId(STR_WARNING_INVALIDJAVASETTINGS));
#endif
            std::unique_ptr<weld::MessageDialog> xWarningBox(Application::CreateMessageDialog(nullptr,
                                                             VclMessageType::Warning, VclButtonsType::Ok, sWarning));
            xWarningBox->set_title(SvtResId(STR_WARNING_INVALIDJAVASETTINGS_TITLE));
            nResult = xWarningBox->run();
        }
        else
        {
            nResult = RET_OK;
        }
    }
    else if ( anyExc >>= e3 )
    {
        SolarMutexGuard aSolarGuard;
        if( !g_JavaEvents.bDisabledHandled )
        {
            g_JavaEvents.bDisabledHandled = true;
            // Java disabled. Give user a chance to enable Java inside Office.
            std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(nullptr, u"svt/ui/javadisableddialog.ui"_ustr));
            std::unique_ptr<weld::MessageDialog> xQueryBox(xBuilder->weld_message_dialog(u"JavaDisabledDialog"_ustr));
            nResult = xQueryBox->run();
            if ( nResult == RET_YES )
            {
                jfw_setEnabled(true);
            }

            g_JavaEvents.nResult_JavaDisabled = nResult;

        }
        else
        {
            nResult = g_JavaEvents.nResult_JavaDisabled;
        }
    }
    else if ( anyExc >>= e4 )
    {
        SolarMutexGuard aSolarGuard;
        if( !g_JavaEvents.bVMCreationFailureHandled )
        {
            // Java not correctly installed, or damaged
            g_JavaEvents.bVMCreationFailureHandled = true;
#ifdef MACOSX
            OUString sWarning(SvtResId(STR_ERROR_JVMCREATIONFAILED_MAC));
#else
            OUString sWarning(SvtResId(STR_ERROR_JVMCREATIONFAILED));
#endif
            std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(nullptr,
                                                           VclMessageType::Warning, VclButtonsType::Ok, sWarning));
            xErrorBox->set_title(SvtResId(STR_ERROR_JVMCREATIONFAILED_TITLE));
            nResult = xErrorBox->run();
        }
        else
        {
            nResult = RET_OK;
        }
    }
    else if ( anyExc >>= e5 )
    {
        SolarMutexGuard aSolarGuard;
        if( !g_JavaEvents.bRestartRequiredHandled )
        {
            // a new JRE was selected, but office needs to be restarted
            //before it can be used.
            g_JavaEvents.bRestartRequiredHandled = true;
            svtools::executeRestartDialog(
                comphelper::getProcessComponentContext(), nullptr,
                svtools::RESTART_REASON_JAVA);
        }
        nResult = RET_OK;
    }

    if ( nResult == RET_CANCEL || nResult == RET_NO)
    {
        // Unknown exception type or user wants to cancel
        if ( abort.is() )
            abort->select();
    }
    else // nResult == RET_OK
    {
        // User selected OK => retry Java usage
        if ( retry.is() )
            retry->select();
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
