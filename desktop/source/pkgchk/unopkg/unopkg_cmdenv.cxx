/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include "../../deployment/gui/dp_gui.hrc"
#include "../../deployment/gui/dp_gui_shared.hxx"
#include "unopkg_shared.h"
#include "osl/thread.h"
#include "tools/string.hxx"
#include "tools/resmgr.hxx"
#include "cppuhelper/implbase3.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "comphelper/anytostring.hxx"
#include "unotools/configmgr.hxx"
#include "com/sun/star/lang/WrappedTargetException.hpp"
#include "com/sun/star/task/XInteractionAbort.hpp"
#include "com/sun/star/task/XInteractionApprove.hpp"
#include "com/sun/star/deployment/InstallException.hpp"
#include "com/sun/star/container/ElementExistException.hpp"
#include "com/sun/star/deployment/LicenseException.hpp"
#include "com/sun/star/deployment/VersionException.hpp"
#include "com/sun/star/deployment/PlatformException.hpp"
#include "com/sun/star/i18n/XCollator.hpp"
#include "com/sun/star/i18n/CollatorOptions.hpp"

#include <stdio.h>
#include "deployment.hrc"
#include "dp_version.hxx"

namespace css = ::com::sun::star;
using namespace ::com::sun::star;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::unopkg;
using ::rtl::OUString;


namespace {

//==============================================================================
class CommandEnvironmentImpl
    : public ::cppu::WeakImplHelper3< XCommandEnvironment,
                                      task::XInteractionHandler,
                                      XProgressHandler >
{
    sal_Int32 m_logLevel;
    bool m_option_force_overwrite;
    bool m_option_verbose;
    bool m_option_suppress_license;
    Reference< XComponentContext > m_xComponentContext;
    Reference< XProgressHandler > m_xLogFile;

    void update_( Any const & Status ) throw (RuntimeException);
    void printLicense(const OUString & sName,const OUString& sLicense,
                      bool & accept, bool & decline);

public:
    virtual ~CommandEnvironmentImpl();
    CommandEnvironmentImpl(
        Reference<XComponentContext> const & xComponentContext,
        OUString const & log_file,
        bool option_force_overwrite,
        bool option_verbose,
        bool option_suppress_license);

    // XCommandEnvironment
    virtual Reference< task::XInteractionHandler > SAL_CALL
    getInteractionHandler() throw (RuntimeException);
    virtual Reference< XProgressHandler > SAL_CALL getProgressHandler()
        throw (RuntimeException);

    // XInteractionHandler
    virtual void SAL_CALL handle(
        Reference< task::XInteractionRequest > const & xRequest )
        throw (RuntimeException);

    // XProgressHandler
    virtual void SAL_CALL push( Any const & Status ) throw (RuntimeException);
    virtual void SAL_CALL update( Any const & Status ) throw (RuntimeException);
    virtual void SAL_CALL pop() throw (RuntimeException);
};


//______________________________________________________________________________
CommandEnvironmentImpl::CommandEnvironmentImpl(
    Reference<XComponentContext> const & xComponentContext,
    OUString const & log_file,
    bool option_force_overwrite,
    bool option_verbose,
    bool option_suppressLicense)
    : m_logLevel(0),
      m_option_force_overwrite( option_force_overwrite ),
      m_option_verbose( option_verbose ),
      m_option_suppress_license( option_suppressLicense ),
      m_xComponentContext(xComponentContext)
{
    if (!log_file.isEmpty()) {
        const Any logfile(log_file);
        m_xLogFile.set(
            xComponentContext->getServiceManager()
            ->createInstanceWithArgumentsAndContext(
                OUSTR("com.sun.star.comp.deployment.ProgressLog"),
                Sequence<Any>( &logfile, 1 ), xComponentContext ),
            UNO_QUERY_THROW );
    }
}

//______________________________________________________________________________
CommandEnvironmentImpl::~CommandEnvironmentImpl()
{
    try {
        Reference< lang::XComponent > xComp( m_xLogFile, UNO_QUERY );
        if (xComp.is())
            xComp->dispose();
    }
    catch (const RuntimeException & exc) {
        (void) exc;
        OSL_FAIL( ::rtl::OUStringToOString(
                        exc.Message, osl_getThreadTextEncoding() ).getStr() );
    }
}

//May throw exceptions
void CommandEnvironmentImpl::printLicense(
    const OUString & sName, const OUString& sLicense, bool & accept, bool &decline)
{
    ResMgr * pResMgr = DeploymentResMgr::get();
    String s1tmp(ResId(RID_STR_UNOPKG_ACCEPT_LIC_1, *pResMgr));
    s1tmp.SearchAndReplaceAllAscii( "$NAME", sName );
    OUString s1(s1tmp);
    OUString s2 = String(ResId(RID_STR_UNOPKG_ACCEPT_LIC_2, *pResMgr));
    OUString s3 = String(ResId(RID_STR_UNOPKG_ACCEPT_LIC_3, *pResMgr));
    OUString s4 = String(ResId(RID_STR_UNOPKG_ACCEPT_LIC_4, *pResMgr));
    OUString sYES = String(ResId(RID_STR_UNOPKG_ACCEPT_LIC_YES, *pResMgr));
    OUString sY = String(ResId(RID_STR_UNOPKG_ACCEPT_LIC_Y, *pResMgr));
    OUString sNO = String(ResId(RID_STR_UNOPKG_ACCEPT_LIC_NO, *pResMgr));
    OUString sN = String(ResId(RID_STR_UNOPKG_ACCEPT_LIC_N, *pResMgr));

    OUString sNewLine(RTL_CONSTASCII_USTRINGPARAM("\n"));

    dp_misc::writeConsole(sNewLine + sNewLine + s1 + sNewLine + sNewLine);
    dp_misc::writeConsole(sLicense + sNewLine + sNewLine);
    dp_misc::writeConsole(s2 + sNewLine);
    dp_misc::writeConsole(s3);

    //the user may enter "yes" or "no", we compare in a case insensitive way
    Reference< css::i18n::XCollator > xCollator(
        m_xComponentContext->getServiceManager()
            ->createInstanceWithContext(
                OUSTR("com.sun.star.i18n.Collator"),m_xComponentContext),
            UNO_QUERY_THROW );
    xCollator->loadDefaultCollator(
        toLocale(utl::ConfigManager::getLocale()),
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
//______________________________________________________________________________
Reference< task::XInteractionHandler >
CommandEnvironmentImpl::getInteractionHandler() throw (RuntimeException)
{
    return this;
}

//______________________________________________________________________________
Reference< XProgressHandler > CommandEnvironmentImpl::getProgressHandler()
    throw (RuntimeException)
{
    return this;
}

// XInteractionHandler
//______________________________________________________________________________
void CommandEnvironmentImpl::handle(
    Reference<task::XInteractionRequest> const & xRequest )
    throw (RuntimeException)
{
    Any request( xRequest->getRequest() );
    OSL_ASSERT( request.getValueTypeClass() == TypeClass_EXCEPTION );
    dp_misc::TRACE(OUSTR("[unopkg_cmdenv.cxx] incoming request:\n")
        + ::comphelper::anyToString(request) + OUSTR("\n\n"));

    // selections:
    bool approve = false;
    bool abort = false;

    lang::WrappedTargetException wtExc;
    deployment::LicenseException licExc;
    deployment::InstallException instExc;
    deployment::PlatformException platExc;
    deployment::VersionException verExc;


    bool bLicenseException = false;
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
                           xPackageType->getMediaType().matchAsciiL(
                               RTL_CONSTASCII_STRINGPARAM(
                                   "application/"
                                   "vnd.sun.star.legacy-package-bundle") ));
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
        String sMsg(ResId(RID_STR_UNSUPPORTED_PLATFORM, *dp_gui::DeploymentGuiResMgr::get()));
        sMsg.SearchAndReplaceAllAscii("%Name", platExc.package->getDisplayName());
        dp_misc::writeConsole(OUSTR("\n") + sMsg + OUSTR("\n\n"));
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

    //In case of a user declining a license abort is true but this is intended,
    //therefore no logging
    if (abort && m_option_verbose && !bLicenseException)
    {
        OUString msg = ::comphelper::anyToString(request);
        dp_misc::writeConsoleError(
            OUSTR("\nERROR: ") + msg + OUSTR("\n"));
    }

    // select:
    Sequence< Reference<task::XInteractionContinuation> > conts(
        xRequest->getContinuations() );
    Reference<task::XInteractionContinuation> const * pConts =
        conts.getConstArray();
    sal_Int32 len = conts.getLength();
    for ( sal_Int32 pos = 0; pos < len; ++pos )
    {
        if (approve) {
            Reference<task::XInteractionApprove> xInteractionApprove(
                pConts[ pos ], UNO_QUERY );
            if (xInteractionApprove.is()) {
                xInteractionApprove->select();
                break;
            }
        }
        else if (abort) {
            Reference<task::XInteractionAbort> xInteractionAbort(
                pConts[ pos ], UNO_QUERY );
            if (xInteractionAbort.is()) {
                xInteractionAbort->select();
                break;
            }
        }
    }
}

// XProgressHandler
//______________________________________________________________________________
void CommandEnvironmentImpl::push( Any const & Status )
    throw (RuntimeException)
{
    update_( Status );
    OSL_ASSERT( m_logLevel >= 0 );
    ++m_logLevel;
    if (m_xLogFile.is())
        m_xLogFile->push( Status );
}

//______________________________________________________________________________
void CommandEnvironmentImpl::update_( Any const & Status )
    throw (RuntimeException)
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
        ::rtl::OUStringBuffer buf;
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("WARNING: ") );
        deployment::DeploymentException dp_exc;
        if (Status >>= dp_exc) {
            buf.append( dp_exc.Message );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(", Cause: ") );
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
        dp_misc::writeConsoleError(msg + OUSTR("\n"));
    else
        dp_misc::writeConsole(msg + OUSTR("\n"));
}

//______________________________________________________________________________
void CommandEnvironmentImpl::update( Any const & Status )
    throw (RuntimeException)
{
    update_( Status );
    if (m_xLogFile.is())
        m_xLogFile->update( Status );
}

//______________________________________________________________________________
void CommandEnvironmentImpl::pop() throw (RuntimeException)
{
    OSL_ASSERT( m_logLevel > 0 );
    --m_logLevel;
    if (m_xLogFile.is())
        m_xLogFile->pop();
}


} // anon namespace

namespace unopkg {

//==============================================================================
Reference< XCommandEnvironment > createCmdEnv(
    Reference< XComponentContext > const & xContext,
    OUString const & logFile,
    bool option_force_overwrite,
    bool option_verbose,
    bool option_suppress_license)
{
    return new CommandEnvironmentImpl(
        xContext, logFile, option_force_overwrite, option_verbose, option_suppress_license);
}
} // unopkg

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
