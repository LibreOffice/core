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


#include <strings.hrc>
#include <dp_shared.hxx>
#include "unopkg_shared.h"
#include <osl/thread.h>
#include <sal/log.hxx>
#include <unotools/resmgr.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <comphelper/anytostring.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/configmgr.hxx>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/deployment/DeploymentException.hpp>
#include <com/sun/star/deployment/InstallException.hpp>
#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/deployment/LicenseException.hpp>
#include <com/sun/star/deployment/VersionException.hpp>
#include <com/sun/star/deployment/PlatformException.hpp>
#include <com/sun/star/i18n/Collator.hpp>
#include <com/sun/star/i18n/CollatorOptions.hpp>

#include <stdio.h>
#include <dp_version.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::unopkg;


namespace {


class CommandEnvironmentImpl
    : public ::cppu::WeakImplHelper< XCommandEnvironment,
                                      task::XInteractionHandler,
                                      XProgressHandler >
{
    sal_Int32 m_logLevel;
    bool m_option_force_overwrite;
    bool m_option_verbose;
    bool m_option_suppress_license;
    Reference< XComponentContext > m_xComponentContext;
    Reference< XProgressHandler > m_xLogFile;

    /// @throws RuntimeException
    void update_( Any const & Status );
    void printLicense(const OUString & sName,const OUString& sLicense,
                      bool & accept, bool & decline);

public:
    virtual ~CommandEnvironmentImpl() override;
    CommandEnvironmentImpl(
        Reference<XComponentContext> const & xComponentContext,
        bool option_force_overwrite,
        bool option_verbose,
        bool option_suppress_license);

    // XCommandEnvironment
    virtual Reference< task::XInteractionHandler > SAL_CALL
    getInteractionHandler() override;
    virtual Reference< XProgressHandler > SAL_CALL getProgressHandler() override;

    // XInteractionHandler
    virtual void SAL_CALL handle(
        Reference< task::XInteractionRequest > const & xRequest ) override;

    // XProgressHandler
    virtual void SAL_CALL push( Any const & Status ) override;
    virtual void SAL_CALL update( Any const & Status ) override;
    virtual void SAL_CALL pop() override;
};


CommandEnvironmentImpl::CommandEnvironmentImpl(
    Reference<XComponentContext> const & xComponentContext,
    bool option_force_overwrite,
    bool option_verbose,
    bool option_suppressLicense)
    : m_logLevel(0),
      m_option_force_overwrite( option_force_overwrite ),
      m_option_verbose( option_verbose ),
      m_option_suppress_license( option_suppressLicense ),
      m_xComponentContext(xComponentContext)
{
    m_xLogFile.set(
        xComponentContext->getServiceManager()
        ->createInstanceWithArgumentsAndContext(
            "com.sun.star.comp.deployment.ProgressLog",
            Sequence<Any>(), xComponentContext ),
        UNO_QUERY_THROW );
}


CommandEnvironmentImpl::~CommandEnvironmentImpl()
{
    try {
        Reference< lang::XComponent > xComp( m_xLogFile, UNO_QUERY );
        if (xComp.is())
            xComp->dispose();
    }
    catch (const RuntimeException &) {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN( "desktop", exceptionToString(ex) );
    }
}

//May throw exceptions
void CommandEnvironmentImpl::printLicense(
    const OUString & sName, const OUString& sLicense, bool & accept, bool &decline)
{
    OUString s1tmp(DpResId(RID_STR_UNOPKG_ACCEPT_LIC_1));
    OUString s1(s1tmp.replaceAll("$NAME", sName));
    OUString s2 = DpResId(RID_STR_UNOPKG_ACCEPT_LIC_2);
    OUString s3 = DpResId(RID_STR_UNOPKG_ACCEPT_LIC_3);
    OUString s4 = DpResId(RID_STR_UNOPKG_ACCEPT_LIC_4);
    OUString sYES = DpResId(RID_STR_UNOPKG_ACCEPT_LIC_YES);
    OUString sY = DpResId(RID_STR_UNOPKG_ACCEPT_LIC_Y);
    OUString sNO = DpResId(RID_STR_UNOPKG_ACCEPT_LIC_NO);
    OUString sN = DpResId(RID_STR_UNOPKG_ACCEPT_LIC_N);

    OUString sNewLine("\n");

    dp_misc::writeConsole(sNewLine + sNewLine + s1 + sNewLine + sNewLine);
    dp_misc::writeConsole(sLicense + sNewLine + sNewLine);
    dp_misc::writeConsole(s2 + sNewLine);
    dp_misc::writeConsole(s3);

    //the user may enter "yes" or "no", we compare in a case insensitive way
    Reference< css::i18n::XCollator > xCollator =
        css::i18n::Collator::create( m_xComponentContext );
    xCollator->loadDefaultCollator(
        LanguageTag(utl::ConfigManager::getUILocale()).getLocale(),
        css::i18n::CollatorOptions::CollatorOptions_IGNORE_CASE);

    do
    {
        OUString sAnswer = dp_misc::readConsole();
        if (xCollator->compareString(sAnswer, sYES) == 0
            || xCollator->compareString(sAnswer, sY) == 0)
        {
            accept = true;
            break;
        }
        else if(xCollator->compareString(sAnswer, sNO) == 0
            || xCollator->compareString(sAnswer, sN) == 0)
        {
            decline = true;
            break;
        }
        else
        {
            dp_misc::writeConsole(sNewLine + sNewLine + s4 + sNewLine);
        }
    }
    while(true);
}

// XCommandEnvironment

Reference< task::XInteractionHandler >
CommandEnvironmentImpl::getInteractionHandler()
{
    return this;
}


Reference< XProgressHandler > CommandEnvironmentImpl::getProgressHandler()
{
    return this;
}

// XInteractionHandler

void CommandEnvironmentImpl::handle(
    Reference<task::XInteractionRequest> const & xRequest )
{
    Any request( xRequest->getRequest() );
    OSL_ASSERT( request.getValueTypeClass() == TypeClass_EXCEPTION );
    dp_misc::TRACE("[unopkg_cmdenv.cxx] incoming request:\n"
        + ::comphelper::anyToString(request) + "\n\n");

    // selections:
    bool approve = false;
    bool abort = false;

    lang::WrappedTargetException wtExc;
    deployment::LicenseException licExc;
    deployment::InstallException instExc;
    deployment::PlatformException platExc;

    if (request >>= wtExc) {
        // ignore intermediate errors of legacy packages, i.e.
        // former pkgchk behaviour:
        const Reference<deployment::XPackage> xPackage(
            wtExc.Context, UNO_QUERY );
        OSL_ASSERT( xPackage.is() );
        if (xPackage.is()) {
            const Reference<deployment::XPackageTypeInfo> xPackageType(
                xPackage->getPackageType() );
            OSL_ASSERT( xPackageType.is() );
            if (xPackageType.is()) {
                approve = (xPackage->isBundle() &&
                           xPackageType->getMediaType().match(
                               "application/vnd.sun.star.legacy-package-bundle") );
            }
        }
        abort = !approve;
        if (abort) {
            // notify cause as error:
            request = wtExc.TargetException;
        }
        else {
            // handable deployment error signalled, e.g.
            // bundle item registration failed, notify as warning:
            update_( wtExc.TargetException );
        }
    }
    else if (request >>= licExc)
    {
        if ( !m_option_suppress_license )
            printLicense(licExc.ExtensionName, licExc.Text, approve, abort);
        else
        {
            approve = true;
            abort = false;
        }
    }
    else if (request >>= instExc)
    {
        //Only if the unopgk was started with gui + extension then we user is asked.
        //In console mode there is no asking.
        approve = true;
    }
    else if (request >>= platExc)
    {
        OUString sMsg(DpResId(RID_STR_UNSUPPORTED_PLATFORM));
        sMsg = sMsg.replaceAll("%Name", platExc.package->getDisplayName());
        dp_misc::writeConsole("\n" + sMsg + "\n\n");
        approve = true;
    }
    else {
        deployment::VersionException nc_exc;
        if (request >>= nc_exc) {
            approve = m_option_force_overwrite ||
                (::dp_misc::compareVersions(nc_exc.NewVersion, nc_exc.Deployed->getVersion())
                 == ::dp_misc::GREATER);
            abort = !approve;
        }
        else
            return; // unknown request => no selection at all
    }

    if (abort && m_option_verbose)
    {
        OUString msg = ::comphelper::anyToString(request);
        dp_misc::writeConsoleError("\nERROR: " + msg + "\n");
    }

    // select:
    for ( auto const& rCont : xRequest->getContinuations() )
    {
        if (approve) {
            Reference<task::XInteractionApprove> xInteractionApprove(
                rCont, UNO_QUERY );
            if (xInteractionApprove.is()) {
                xInteractionApprove->select();
                break;
            }
        }
        else if (abort) {
            Reference<task::XInteractionAbort> xInteractionAbort(
                rCont, UNO_QUERY );
            if (xInteractionAbort.is()) {
                xInteractionAbort->select();
                break;
            }
        }
    }
}

// XProgressHandler

void CommandEnvironmentImpl::push( Any const & Status )
{
    update_( Status );
    OSL_ASSERT( m_logLevel >= 0 );
    ++m_logLevel;
    if (m_xLogFile.is())
        m_xLogFile->push( Status );
}


void CommandEnvironmentImpl::update_( Any const & Status )
{
    if (! Status.hasValue())
        return;
    bool bUseErr = false;
    OUString msg;
    if (Status >>= msg) {
        if (! m_option_verbose)
            return;
    }
    else {
        OUStringBuffer buf;
        buf.append( "WARNING: " );
        deployment::DeploymentException dp_exc;
        if (Status >>= dp_exc) {
            buf.append( dp_exc.Message );
            buf.append( ", Cause: " );
            buf.append( ::comphelper::anyToString(dp_exc.Cause) );
        }
        else {
            buf.append( ::comphelper::anyToString(Status) );
        }
        msg = buf.makeStringAndClear();
        bUseErr = true;
    }
    OSL_ASSERT( m_logLevel >= 0 );
    for ( sal_Int32 n = 0; n < m_logLevel; ++n )
    {
        if (bUseErr)
            dp_misc::writeConsoleError(" ");
        else
            dp_misc::writeConsole(" ");
    }

    if (bUseErr)
        dp_misc::writeConsoleError(msg + "\n");
    else
        dp_misc::writeConsole(msg + "\n");
}


void CommandEnvironmentImpl::update( Any const & Status )
{
    update_( Status );
    if (m_xLogFile.is())
        m_xLogFile->update( Status );
}


void CommandEnvironmentImpl::pop()
{
    OSL_ASSERT( m_logLevel > 0 );
    --m_logLevel;
    if (m_xLogFile.is())
        m_xLogFile->pop();
}


} // anon namespace

namespace unopkg {


Reference< XCommandEnvironment > createCmdEnv(
    Reference< XComponentContext > const & xContext,
    bool option_force_overwrite,
    bool option_verbose,
    bool option_suppress_license)
{
    return new CommandEnvironmentImpl(
        xContext, option_force_overwrite, option_verbose, option_suppress_license);
}
} // unopkg

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
