/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dp_migration.cxx,v $
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

#include "dp_misc.h"
#include "dp_ucb.h"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/implbase2.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "ucbhelper/content.hxx"
#include "comphelper/anytostring.hxx"
#include "comphelper/servicedecl.hxx"
#include "com/sun/star/lang/WrappedTargetException.hpp"
#include "com/sun/star/task/XJob.hpp"
#include "com/sun/star/task/XInteractionAbort.hpp"
#include "com/sun/star/task/XInteractionApprove.hpp"
#include "com/sun/star/sdbc/XResultSet.hpp"
#include "com/sun/star/sdbc/XRow.hpp"
#include "com/sun/star/ucb/XContentAccess.hpp"
#include "com/sun/star/deployment/thePackageManagerFactory.hpp"


using namespace ::com::sun::star;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::dp_misc;
using ::rtl::OUString;

namespace dp_migration {

class MigrationImpl : public ::cppu::WeakImplHelper1<task::XJob>
{
    struct CommandEnvironmentImpl
        : public ::cppu::WeakImplHelper2< XCommandEnvironment,
                                          task::XInteractionHandler >
    {
        // XCommandEnvironment
        virtual Reference<task::XInteractionHandler> SAL_CALL
        getInteractionHandler() throw (RuntimeException);
        virtual Reference<XProgressHandler> SAL_CALL getProgressHandler()
            throw (RuntimeException);
        // XInteractionHandler
        virtual void SAL_CALL handle(
            Reference<task::XInteractionRequest> const & xRequest )
            throw (RuntimeException);
    };

    const Reference<XComponentContext> m_xContext;
    OUString m_userData;

protected:
    virtual ~MigrationImpl();
public:
    MigrationImpl( Sequence<Any> const & args,
                   Reference<XComponentContext> const & xComponentContext );

    // XJob
    virtual Any SAL_CALL execute( Sequence<beans::NamedValue> const & args )
        throw (lang::IllegalArgumentException, Exception, RuntimeException);
};

MigrationImpl::~MigrationImpl()
{
}

MigrationImpl::MigrationImpl(
    Sequence<Any> const & args, Reference<XComponentContext> const & xContext )
    : m_xContext(xContext)
{
    for ( sal_Int32 pos = args.getLength(); pos--; )
    {
        const beans::NamedValue nv(args[pos].get<beans::NamedValue>());
        if (nv.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("UserData") ))
            m_userData = nv.Value.get<OUString>();
    }
    if (m_userData.getLength() == 0)
        throw lang::IllegalArgumentException( OUSTR("missing UserData!"), 0,
                                              static_cast<sal_Int16>(-1) );
}

// XJob
Any MigrationImpl::execute( Sequence<beans::NamedValue> const & )
    throw (lang::IllegalArgumentException, Exception, RuntimeException)
{
    const Reference<deployment::XPackageManager> xManager(
        deployment::thePackageManagerFactory::get(
            m_xContext )->getPackageManager( OUSTR("user") ) );
    ::ucbhelper::Content packagesDir;
    if (create_ucb_content( &packagesDir,
                            makeURL( m_userData, OUSTR("user/uno_packages") ),
                            Reference<XCommandEnvironment>(),
                            false /* no throw */ ))
    {
        const Reference<XCommandEnvironment> xCmdEnv(
            new CommandEnvironmentImpl );
        OUString const & strTitle = StrTitle::get();
        const Reference<sdbc::XResultSet> xResultSet(
            packagesDir.createCursor( Sequence<OUString>( &strTitle, 1 ),
                                      ::ucbhelper::INCLUDE_DOCUMENTS_ONLY ) );
        while (xResultSet->next())
        {
            Reference<sdbc::XRow> xRow( xResultSet, UNO_QUERY_THROW );
            const OUString title( xRow->getString( 1 /* Title */ ) );
            // exclude stampIt, not migratable to OOo 2.0:
            if (title.matchIgnoreAsciiCaseAsciiL(
                    RTL_CONSTASCII_STRINGPARAM("SSICONCT.") ))
                continue;
            const OUString sourceURL( Reference<XContentAccess>(
                                          xResultSet, UNO_QUERY_THROW )
                                      ->queryContentIdentifierString() );
            try {
                xManager->addPackage(
                    sourceURL, OUString() /* detect media-type */,
                    Reference<task::XAbortChannel>(), xCmdEnv );
            }
            catch (RuntimeException &) {
                throw;
            }
            catch (Exception &) {
                OSL_ENSURE( 0, ::rtl::OUStringToOString(
                                ::comphelper::anyToString(
                                    ::cppu::getCaughtException() ),
                                RTL_TEXTENCODING_UTF8 ).getStr() );
            }
        }
    }
    return Any();
}

// XCommandEnvironment
Reference<task::XInteractionHandler>
MigrationImpl::CommandEnvironmentImpl::getInteractionHandler()
    throw (RuntimeException)
{
    return this;
}

Reference<XProgressHandler>
MigrationImpl::CommandEnvironmentImpl::getProgressHandler()
    throw (RuntimeException)
{
    return Reference<XProgressHandler>();
}

// XInteractionHandler
void MigrationImpl::CommandEnvironmentImpl::handle(
    Reference<task::XInteractionRequest> const & xRequest )
    throw (RuntimeException)
{
    Any request( xRequest->getRequest() );
    OSL_ASSERT( request.getValueTypeClass() == TypeClass_EXCEPTION );
#if OSL_DEBUG_LEVEL > 1
    OSL_TRACE( "[dp_migration.cxx] incoming request:\n%s\n",
               ::rtl::OUStringToOString( ::comphelper::anyToString(request),
                                         RTL_TEXTENCODING_UTF8 ).getStr() );
#endif

    // selections:
    bool approve = false;
    bool abort = false;

    lang::WrappedTargetException wtExc;
    if (request >>= wtExc) {
        OSL_ENSURE( 0, ::rtl::OUStringToOString(
                        ::comphelper::anyToString(wtExc.TargetException),
                        RTL_TEXTENCODING_UTF8 ).getStr() );

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
    }
    else
        return; // unknown request => no selection at all

    // select:
    const Sequence< Reference<task::XInteractionContinuation> > conts(
        xRequest->getContinuations() );
    for ( sal_Int32 pos = 0; pos < conts.getLength(); ++pos )
    {
        if (approve) {
            const Reference<task::XInteractionApprove> xInteractionApprove(
                conts[ pos ], UNO_QUERY );
            if (xInteractionApprove.is()) {
                xInteractionApprove->select();
                // don't query again for ongoing continuations:
                approve = false;
            }
        }
        else if (abort) {
            const Reference<task::XInteractionAbort> xInteractionAbort(
                conts[ pos ], UNO_QUERY );
            if (xInteractionAbort.is()) {
                xInteractionAbort->select();
                // don't query again for ongoing continuations:
                abort = false;
            }
        }
    }
}

namespace sdecl = comphelper::service_decl;
sdecl::class_<MigrationImpl, sdecl::with_args<true> > serviceMI;
extern sdecl::ServiceDecl const serviceDecl(
    serviceMI,
    // a private one (config entry):
    "com.sun.star.comp.deployment.migration.Migration_2_0",
    "com.sun.star.comp.deployment.migration.Migration_2_0" );

} // namespace dp_migration

