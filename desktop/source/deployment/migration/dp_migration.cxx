/*************************************************************************
 *
 *  $RCSfile: dp_migration.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 17:11:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "dp_misc.h"
#include "dp_ucb.h"
#include "cppuhelper/implbase2.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "ucbhelper/content.hxx"
#include "comphelper/anytostring.hxx"
#include "com/sun/star/lang/XServiceInfo.hpp"
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

OUString SAL_CALL getImplementationName()
{
    return OUSTR("com.sun.star.comp.deployment.migration.Migration_2_0");
}

namespace {

class MigrationImpl : public ::cppu::WeakImplHelper2<
    lang::XServiceInfo, task::XJob >
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
    OUString m_oldUserDir;

public:
    virtual ~MigrationImpl();
    MigrationImpl( Sequence<Any> const & args,
                   Reference<XComponentContext> const & xComponentContext );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( OUString const & serviceName )
        throw (RuntimeException);
    virtual Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (RuntimeException);
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
        const beans::NamedValue nv( args[pos].get<beans::NamedValue>() );
        if (nv.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("OldUserDir") ))
            m_oldUserDir = nv.Value.get<OUString>();
    }
    if (m_oldUserDir.getLength() == 0)
        throw lang::IllegalArgumentException( OUSTR("missing OldUserDir!"), 0,
                                              static_cast<sal_Int16>(-1) );
}

// XServiceInfo
OUString MigrationImpl::getImplementationName() throw (RuntimeException)
{
    return ::dp_migration::getImplementationName();
}

sal_Bool MigrationImpl::supportsService( OUString const & serviceName )
    throw (RuntimeException)
{
    return getImplementationName().equals(serviceName);
}

Sequence<OUString> MigrationImpl::getSupportedServiceNames()
    throw (RuntimeException)
{
    const OUString name( getImplementationName() );
    return Sequence<OUString>( &name, 1 );
}

// XJob
Any MigrationImpl::execute( Sequence<beans::NamedValue> const & )
    throw (lang::IllegalArgumentException, Exception, RuntimeException)
{
    const Reference<deployment::XPackageManager> xManager(
        deployment::thePackageManagerFactory::get(
            m_xContext )->getPackageManager( OUSTR("user") ) );
    ::ucb::Content packagesDir;
    if (create_ucb_content( &packagesDir,
                            makeURL( m_oldUserDir, OUSTR("user/uno_packages") ),
                            Reference<XCommandEnvironment>(),
                            false /* no throw */ ))
    {
        const Reference<XCommandEnvironment> xCmdEnv(
            new CommandEnvironmentImpl );
        OUString const & strTitle = StrTitle::get();
        const Reference<sdbc::XResultSet> xResultSet(
            packagesDir.createCursor( Sequence<OUString>( &strTitle, 1 ),
                                      ::ucb::INCLUDE_DOCUMENTS_ONLY ) );
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

} // anon namespace

Reference<XInterface> SAL_CALL create(
    Sequence<Any> const & args,
    Reference<XComponentContext> const & xComponentContext )
{
    return static_cast< ::cppu::OWeakObject * >(
        new MigrationImpl( args, xComponentContext ) );
}

} // namespace dp_migration

