/*************************************************************************
 *
 *  $RCSfile: dp_backend.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 12:10:38 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#if ! defined INCLUDED_DP_REGISTRY_H
#define INCLUDED_DP_REGISTRY_H

#include "dp_misc.h"
#include "dp_resource.h"
#include "dp_interact.h"
#include "rtl/ref.hxx"
#include "cppuhelper/weakref.hxx"
#include "cppuhelper/compbase1.hxx"
#include "cppuhelper/compbase3.hxx"
#include "tools/inetmime.hxx"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/lang/XEventListener.hpp"
#include "com/sun/star/deployment/XPackageRegistry.hpp"
#include <memory>
#include <hash_map>


namespace dp_registry
{
namespace backend
{

namespace css = ::com::sun::star;

class PackageRegistryBackend;

typedef ::cppu::WeakComponentImplHelper1<
    css::deployment::XPackage > t_PackageBase;

//==============================================================================
class Package : protected ::dp_misc::MutexHolder, public t_PackageBase
{
protected:
    ::rtl::Reference<PackageRegistryBackend> m_myBackend;
    ::rtl::OUString m_url;
    ::rtl::OUString m_mediaType;
    ::rtl::OUString m_name;
    ::rtl::OUString m_displayName;
    ::rtl::OUString m_description;

    void check();
    void fireModified();
    virtual void SAL_CALL disposing();

    void checkAborted(
        ::rtl::Reference< ::dp_misc::AbortChannel > const & abortChannel );

    // @@@ to be implemented by specific backend:
    virtual css::beans::Optional< css::beans::Ambiguous<sal_Bool> >
    isRegistered_(
        ::osl::ResettableMutexGuard & guard,
        ::rtl::Reference< ::dp_misc::AbortChannel > const & abortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        = 0;
    virtual void processPackage_(
        ::osl::ResettableMutexGuard & guard,
        bool registerPackage,
        ::rtl::Reference< ::dp_misc::AbortChannel > const & abortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        = 0;

    virtual ~Package();
    Package( ::rtl::Reference<PackageRegistryBackend> const & myBackend,
             ::rtl::OUString const & url,
             ::rtl::OUString const & mediaType,
             ::rtl::OUString const & name,
             ::rtl::OUString const & displayName,
             ::rtl::OUString const & description );

public:
    // XComponent
    virtual void SAL_CALL dispose() throw (css::uno::RuntimeException);
    virtual void SAL_CALL addEventListener(
        css::uno::Reference<css::lang::XEventListener> const & xListener )
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener(
        css::uno::Reference<css::lang::XEventListener> const & xListener )
        throw (css::uno::RuntimeException);

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener(
        css::uno::Reference<css::util::XModifyListener> const & xListener )
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener(
        css::uno::Reference<css::util::XModifyListener> const & xListener )
        throw (css::uno::RuntimeException);

    // XPackage
    virtual css::uno::Reference<css::task::XAbortChannel> SAL_CALL
    createAbortChannel() throw (css::uno::RuntimeException);
    virtual css::beans::Optional< css::beans::Ambiguous<sal_Bool> >
    SAL_CALL isRegistered(
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        throw (css::deployment::DeploymentException,
               css::ucb::CommandFailedException,
               css::ucb::CommandAbortedException,
               css::uno::RuntimeException);
    virtual void SAL_CALL registerPackage(
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        throw (css::deployment::DeploymentException,
               css::ucb::CommandFailedException,
               css::ucb::CommandAbortedException,
               css::lang::IllegalArgumentException, css::uno::RuntimeException);
    virtual void SAL_CALL revokePackage(
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        throw (css::deployment::DeploymentException,
               css::ucb::CommandFailedException,
               css::ucb::CommandAbortedException,
               css::lang::IllegalArgumentException,
               css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isBundle()
        throw (css::uno::RuntimeException);
    virtual css::uno::Sequence< css::uno::Reference<css::deployment::XPackage> >
    SAL_CALL getBundle(
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        throw (css::deployment::DeploymentException,
               css::ucb::CommandFailedException,
               css::ucb::CommandAbortedException,
               css::lang::IllegalArgumentException,
               css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getName()
        throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getMediaType()
        throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getURL()
        throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getDisplayName()
        throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getDescription()
        throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getIcon(
        sal_Bool highContrast, sal_Bool smallIcon )
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL exportTo(
        ::rtl::OUString const & destFolderURL,
        ::rtl::OUString const & newTitle,
        sal_Int32 nameClashAction,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        throw (css::ucb::CommandFailedException,
               css::ucb::CommandAbortedException, css::uno::RuntimeException);
};

typedef ::cppu::WeakComponentImplHelper3<
    css::lang::XServiceInfo,
    css::lang::XEventListener,
    css::deployment::XPackageRegistry > t_BackendBase;

//==============================================================================
class PackageRegistryBackend
    : protected ::dp_misc::MutexHolder, public t_BackendBase
{
    css::uno::Reference<css::uno::XComponentContext> m_xComponentContext;
    ::rtl::OUString m_cachePath;
    ::rtl::OUString m_implName;
    css::uno::Sequence< ::rtl::OUString > m_supportedMediaTypes;

    typedef ::std::hash_map<
        ::rtl::OUString, css::uno::WeakReference<css::deployment::XPackage>,
        ::rtl::OUStringHash > t_string2weakref;
    t_string2weakref m_bound;

protected:
    ::rtl::OUString m_context;
    // currently only for library containers:
    enum {
        CONTEXT_UNKNOWN,
        CONTEXT_USER, CONTEXT_SHARED,
        CONTEXT_DOCUMENT
    } m_eContext;
    sal_Bool m_readOnly;

    ::rtl::OUString m_strCannotBindPackage;
    ::rtl::OUString m_strCannotDetectMediaType;
    ::rtl::OUString m_strUnsupportedMediaType;

    // @@@ to be implemented by specific backend:
    virtual css::uno::Reference<css::deployment::XPackage> bindPackage_(
        ::rtl::OUString const & url, ::rtl::OUString const & mediaType,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        = 0;

    void check();
    virtual void SAL_CALL disposing();

    virtual ~PackageRegistryBackend();
    PackageRegistryBackend(
        css::uno::Sequence<css::uno::Any> const & args,
        css::uno::Reference<css::uno::XComponentContext> const & xContext,
        ::rtl::OUString const & implName,
        css::uno::Sequence< ::rtl::OUString > const & supportedMediaTypes );

public:
    ::rtl::OUString m_strRegisteringPackage;
    ::rtl::OUString m_strRevokingPackage;

    inline css::uno::Reference<css::uno::XComponentContext> const &
    getComponentContext() const { return m_xComponentContext; }

    inline ::rtl::OUString const & getCachePath() const { return m_cachePath; }
    inline bool transientMode() const { return m_cachePath.getLength() == 0; }

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService(
        ::rtl::OUString const & serviceName )
        throw (css::uno::RuntimeException);
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing( css::lang::EventObject const & evt )
        throw (css::uno::RuntimeException);

    // XPackageRegistry
    virtual css::uno::Reference<css::deployment::XPackage> SAL_CALL bindPackage(
        ::rtl::OUString const & url, ::rtl::OUString const & mediaType,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        throw (css::deployment::DeploymentException,
               css::ucb::CommandFailedException,
               css::lang::IllegalArgumentException, css::uno::RuntimeException);
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL
    getSupportedMediaTypes() throw (css::uno::RuntimeException);
};

}
}

#endif

