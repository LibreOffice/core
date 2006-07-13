/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_backend.h,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-13 17:06:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#if ! defined INCLUDED_DP_REGISTRY_H
#define INCLUDED_DP_REGISTRY_H

#include "dp_misc.h"
#include "dp_resource.h"
#include "dp_interact.h"
#include "rtl/ref.hxx"
#include "cppuhelper/weakref.hxx"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/compbase1.hxx"
#include "cppuhelper/compbase2.hxx"
#include "tools/inetmime.hxx"
#include "com/sun/star/lang/XEventListener.hpp"
#include "com/sun/star/deployment/XPackageRegistry.hpp"
#include <memory>
#include <hash_map>
#include "dp_registry.hrc"

namespace dp_registry
{
namespace backend
{

namespace css = ::com::sun::star;

class PackageRegistryBackend;

char const* const BACKEND_SERVICE_NAME =
"com.sun.star.deployment.PackageRegistryBackend";

typedef ::cppu::WeakComponentImplHelper1<
    css::deployment::XPackage > t_PackageBase;

//==============================================================================
class Package : protected ::dp_misc::MutexHolder, public t_PackageBase
{
    void processPackage_(
        bool registerPackage,
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv );

protected:
    ::rtl::Reference<PackageRegistryBackend> m_myBackend;
    const ::rtl::OUString m_url;
    ::rtl::OUString m_name;
    ::rtl::OUString m_displayName;
    const css::uno::Reference<css::deployment::XPackageTypeInfo> m_xPackageType;

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
             ::rtl::OUString const & name,
             ::rtl::OUString const & displayName,
             css::uno::Reference<css::deployment::XPackageTypeInfo> const &
             xPackageType );

public:

    class TypeInfo :
        public ::cppu::WeakImplHelper1<css::deployment::XPackageTypeInfo>
    {
        const ::rtl::OUString m_mediaType;
        const ::rtl::OUString m_fileFilter;
        const ::rtl::OUString m_shortDescr;
        const sal_uInt16 m_smallIcon, m_smallIcon_HC;
    public:
        virtual ~TypeInfo();
        TypeInfo( ::rtl::OUString const & mediaType,
                  ::rtl::OUString const & fileFilter,
                  ::rtl::OUString const & shortDescr,
                  sal_uInt16 smallIcon, sal_uInt16 smallIcon_HC )
            : m_mediaType(mediaType), m_fileFilter(fileFilter),
              m_shortDescr(shortDescr),
              m_smallIcon(smallIcon), m_smallIcon_HC(smallIcon_HC)
            {}
        // XPackageTypeInfo
        virtual ::rtl::OUString SAL_CALL getMediaType()
            throw (css::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getDescription()
            throw (css::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getShortDescription()
            throw (css::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getFileFilter()
            throw (css::uno::RuntimeException);
        virtual css::uno::Any SAL_CALL getIcon( sal_Bool highContrast,
                                                sal_Bool smallIcon )
            throw (css::uno::RuntimeException);
    };

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

    virtual ::sal_Bool SAL_CALL checkPrerequisites(
        const css::uno::Reference< css::task::XAbortChannel >& xAbortChannel,
        const css::uno::Reference< css::ucb::XCommandEnvironment >& xCmdEnv )
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
    virtual ::rtl::OUString SAL_CALL getURL()
        throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getDisplayName()
        throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getDescription()
        throw (css::uno::RuntimeException);
    virtual css::uno::Reference<css::deployment::XPackageTypeInfo> SAL_CALL
    getPackageType() throw (css::uno::RuntimeException);
    virtual void SAL_CALL exportTo(
        ::rtl::OUString const & destFolderURL,
        ::rtl::OUString const & newTitle,
        sal_Int32 nameClashAction,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        throw (css::ucb::CommandFailedException,
               css::ucb::CommandAbortedException, css::uno::RuntimeException);
};

typedef ::cppu::WeakComponentImplHelper2<
    css::lang::XEventListener,
    css::deployment::XPackageRegistry > t_BackendBase;

//==============================================================================
class PackageRegistryBackend
    : protected ::dp_misc::MutexHolder, public t_BackendBase
{
    css::uno::Reference<css::uno::XComponentContext> m_xComponentContext;
    ::rtl::OUString m_cachePath;

    typedef ::std::hash_map<
        ::rtl::OUString, css::uno::WeakReference<css::deployment::XPackage>,
        ::rtl::OUStringHash > t_string2weakref;
    t_string2weakref m_bound;

protected:
    ::rtl::OUString m_context;
    // currently only for library containers:
    enum context {
        CONTEXT_UNKNOWN,
        CONTEXT_USER, CONTEXT_SHARED,
        CONTEXT_DOCUMENT
    } m_eContext;
    bool m_readOnly;

    struct StrCannotDetectMediaType : public ::dp_misc::StaticResourceString<
        StrCannotDetectMediaType, RID_STR_CANNOT_DETECT_MEDIA_TYPE> {};
    struct StrUnsupportedMediaType : public ::dp_misc::StaticResourceString<
        StrUnsupportedMediaType, RID_STR_UNSUPPORTED_MEDIA_TYPE> {};

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
        css::uno::Reference<css::uno::XComponentContext> const & xContext );

public:
    struct StrRegisteringPackage : public ::dp_misc::StaticResourceString<
        StrRegisteringPackage, RID_STR_REGISTERING_PACKAGE> {};
    struct StrRevokingPackage : public ::dp_misc::StaticResourceString<
        StrRevokingPackage, RID_STR_REVOKING_PACKAGE> {};

    inline css::uno::Reference<css::uno::XComponentContext> const &
    getComponentContext() const { return m_xComponentContext; }

    inline ::rtl::OUString const & getCachePath() const { return m_cachePath; }
    inline bool transientMode() const { return m_cachePath.getLength() == 0; }

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
//     virtual css::uno::Sequence<
//         css::uno::Reference<css::deployment::XPackageTypeInfo> > SAL_CALL
//     getSupportedPackageTypes() throw (css::uno::RuntimeException);
};

}
}

#endif

