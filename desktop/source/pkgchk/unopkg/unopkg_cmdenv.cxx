/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unopkg_cmdenv.cxx,v $
 * $Revision: 1.10 $
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
#include "precompiled_desktop.hxx"

#include "unopkg_shared.h"
#include "osl/thread.h"
#include "rtl/memory.h"
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
#include "com/sun/star/i18n/XCollator.hpp"
#include "com/sun/star/i18n/CollatorOptions.hpp"
#include "com/sun/star/deployment/LicenseIndividualAgreementException.hpp"
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

inline ::com::sun::star::lang::Locale toLocale( ::rtl::OUString const & slang )
{
    ::com::sun::star::lang::Locale locale;
    sal_Int32 nIndex = 0;
    locale.Language = slang.getToken( 0, '-', nIndex );
    locale.Country = slang.getToken( 0, '-', nIndex );
    locale.Variant = slang.getToken( 0, '-', nIndex );
    return locale;
}


//==============================================================================
struct OfficeLocale :
        public rtl::StaticWithInit<const lang::Locale, OfficeLocale> {
    const lang::Locale operator () () {
        OUString slang;
        if (! (::utl::ConfigManager::GetDirectConfigProperty(
                   ::utl::ConfigManager::LOCALE ) >>= slang))
            throw RuntimeException( OUSTR("Cannot determine language!"), 0 );
        return toLocale(slang);
    }
};

//==============================================================================
class CommandEnvironmentImpl
    : public ::cppu::WeakImplHelper3< XCommandEnvironment,
                                      task::XInteractionHandler,
                                      XProgressHandler >
{
    sal_Int32 m_logLevel;
    bool m_option_force_overwrite;
    bool m_option_verbose;
    Reference< XComponentContext > m_xComponentContext;
    Reference< XProgressHandler > m_xLogFile;

    void update_( Any const & Status ) throw (RuntimeException);
    void printLicense(const OUString& sLicense, bool & accept, bool & decline);

public:
    virtual ~CommandEnvironmentImpl();
    CommandEnvironmentImpl(
        Reference<XComponentContext> const & xComponentContext,
        OUString const & log_file,
        bool option_force_overwrite,
        bool option_verbose);

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
    bool option_verbose)
    : m_logLevel(0),
      m_option_force_overwrite( option_force_overwrite ),
      m_option_verbose( option_verbose ),
      m_xComponentContext(xComponentContext)
{
    if (log_file.getLength() > 0) {
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
    catch (RuntimeException & exc) {
        (void) exc;
        OSL_ENSURE( 0, ::rtl::OUStringToOString(
                        exc.Message, osl_getThreadTextEncoding() ).getStr() );
    }
}

//May throw exceptions
void CommandEnvironmentImpl::printLicense(const OUString& sLicense, bool & accept, bool &decline)
{
    ResMgr * pResMgr = DeploymentResMgr::get();
    OUString s1 = String(ResId(RID_STR_UNOPKG_ACCEPT_LIC_1, *pResMgr));
    ::rtl::OString os1 = ::rtl::OUStringToOString(s1, osl_getThreadTextEncoding());
    OUString s2 = String(ResId(RID_STR_UNOPKG_ACCEPT_LIC_2, *pResMgr));
    ::rtl::OString os2 = ::rtl::OUStringToOString(s2, osl_getThreadTextEncoding());
    OUString s3 = String(ResId(RID_STR_UNOPKG_ACCEPT_LIC_3, *pResMgr));
    ::rtl::OString os3 = ::rtl::OUStringToOString(s3, osl_getThreadTextEncoding());
    OUString s4 = String(ResId(RID_STR_UNOPKG_ACCEPT_LIC_4, *pResMgr));
    ::rtl::OString os4 = ::rtl::OUStringToOString(s4, osl_getThreadTextEncoding());
    OUString sYES = String(ResId(RID_STR_UNOPKG_ACCEPT_LIC_YES, *pResMgr));
    OUString sY = String(ResId(RID_STR_UNOPKG_ACCEPT_LIC_Y, *pResMgr));
    OUString sNO = String(ResId(RID_STR_UNOPKG_ACCEPT_LIC_NO, *pResMgr));
    OUString sN = String(ResId(RID_STR_UNOPKG_ACCEPT_LIC_N, *pResMgr));

    fprintf(stdout, "\n\n%s\n\n", os1.getStr());
    fprintf(stdout, "%s\n\n", OUStringToOString(sLicense, osl_getThreadTextEncoding()).getStr());
    fprintf(stdout, "%s\n", os2.getStr());
    fprintf(stdout, "%s", os3.getStr());
    fflush(stdout);


    //the user may enter "yes" or "no", we compare in a case insensitive way
    Reference< css::i18n::XCollator > xCollator(
        m_xComponentContext->getServiceManager()
            ->createInstanceWithContext(
                OUSTR("com.sun.star.i18n.Collator"),m_xComponentContext),
            UNO_QUERY_THROW );
    xCollator->loadDefaultCollator(OfficeLocale::get(),
        css::i18n::CollatorOptions::CollatorOptions_IGNORE_CASE);

    do
    {
        char buf[16];
        rtl_zeroMemory(buf, 16);
        // read one char less so that the last char in buf is always zero
        fgets(buf, 15, stdin);
        OUString sAnswer = ::rtl::OStringToOUString(::rtl::OString(buf), osl_getThreadTextEncoding());
        sAnswer = sAnswer.trim();

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
            fprintf(stdout, "\n\n%s\n", os4.getStr());
            fflush(stdout);
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
#if OSL_DEBUG_LEVEL > 1
    OSL_TRACE( "[unopkg_cmdenv.cxx] incoming request:\n%s\n",
               ::rtl::OUStringToOString( ::comphelper::anyToString(request),
                                         RTL_TEXTENCODING_UTF8 ).getStr() );
#endif

    // selections:
    bool approve = false;
    bool abort = false;

    lang::WrappedTargetException wtExc;
    deployment::LicenseException licExc;
    deployment::InstallException instExc;
    deployment::LicenseIndividualAgreementException licAgreementExc;

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
    else if (request >>= licAgreementExc)
    {
        String sResMsg( ResId( RID_STR_UNOPKG_NO_SHARED_ALLOWED, *DeploymentResMgr::get() ) );
        sResMsg.SearchAndReplaceAllAscii( "%NAME", licAgreementExc.ExtensionName );
        ::rtl::OString oMsg = ::rtl::OUStringToOString(sResMsg, osl_getThreadTextEncoding());
        fprintf(stdout, "\n%s\n\n", oMsg.getStr());
        abort = true;
    }
    else if (request >>= licExc)
    {
        bLicenseException = true;
        printLicense(licExc.Text, approve, abort);
    }
       else if (request >>= instExc)
    {
        //Only if the unopgk was started with gui + extension then we user is asked.
        //In console mode there is no asking.
        approve = true;
    }
    else {
        deployment::VersionException nc_exc;
        if (request >>= nc_exc) {
            approve = m_option_force_overwrite ||
                (::dp_misc::comparePackageVersions(nc_exc.New, nc_exc.Deployed)
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
        fprintf( stderr, "\nERROR: %s\n",
            ::rtl::OUStringToOString(
            msg, osl_getThreadTextEncoding() ).getStr() );
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

    FILE * stream;
    OUString msg;
    if (Status >>= msg) {
        if (! m_option_verbose)
            return;
        stream = stdout;
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
        stream = stderr;
    }
    OSL_ASSERT( m_logLevel >= 0 );
    for ( sal_Int32 n = 0; n < m_logLevel; ++n )
        fprintf( stream, " " );
    fprintf( stream, "%s\n", ::rtl::OUStringToOString(
                 msg, osl_getThreadTextEncoding() ).getStr() );
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
    bool option_verbose)
{
    return new CommandEnvironmentImpl(
        xContext, logFile, option_force_overwrite, option_verbose);
}

} // unopkg

