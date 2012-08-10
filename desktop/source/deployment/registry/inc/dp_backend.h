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

#ifndef INCLUDED_DP_REGISTRY_H
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
#include "com/sun/star/deployment/XPackageManager.hpp"
#include "com/sun/star/deployment/InvalidRemovedParameterException.hpp"
#include <memory>
#include <boost/unordered_map.hpp>
#include <list>
#include "dp_registry.hrc"

namespace dp_registry
{
namespace backend
{

namespace css = ::com::sun::star;

class PackageRegistryBackend;

#define BACKEND_SERVICE_NAME "com.sun.star.deployment.PackageRegistryBackend"

typedef ::cppu::WeakComponentImplHelper1<
    css::deployment::XPackage > t_PackageBase;

//==============================================================================
class Package : protected ::dp_misc::MutexHolder, public t_PackageBase
{
    PackageRegistryBackend * getMyBackend() const;
    void processPackage_impl(
        bool registerPackage,
        bool startup,
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv );

protected:
    ::rtl::Reference<PackageRegistryBackend> m_myBackend;
    const ::rtl::OUString m_url;
    ::rtl::OUString m_name;
    ::rtl::OUString m_displayName;
    const css::uno::Reference<css::deployment::XPackageTypeInfo> m_xPackageType;
    const bool m_bRemoved;
    //Only set if m_bRemoved = true;
    const ::rtl::OUString m_identifier;

    void check() const;
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
        bool startup,
        ::rtl::Reference< ::dp_misc::AbortChannel > const & abortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        = 0;

    virtual ~Package();
    Package( ::rtl::Reference<PackageRegistryBackend> const & myBackend,
             ::rtl::OUString const & url,
             ::rtl::OUString const & name,
             ::rtl::OUString const & displayName,
             css::uno::Reference<css::deployment::XPackageTypeInfo> const &
             xPackageType,
             bool bRemoved,
             ::rtl::OUString const & identifier);

public:

    class TypeInfo :
        public ::cppu::WeakImplHelper1<css::deployment::XPackageTypeInfo>
    {
        const ::rtl::OUString m_mediaType;
        const ::rtl::OUString m_fileFilter;
        const ::rtl::OUString m_shortDescr;
        const sal_uInt16 m_smallIcon;
    public:
        virtual ~TypeInfo();
        TypeInfo( ::rtl::OUString const & mediaType,
                  ::rtl::OUString const & fileFilter,
                  ::rtl::OUString const & shortDescr,
                  sal_uInt16 smallIcon)
            : m_mediaType(mediaType), m_fileFilter(fileFilter),
              m_shortDescr(shortDescr),
              m_smallIcon(smallIcon)
            {}
        // XPackageTypeInfo
        virtual ::rtl::OUString SAL_CALL getMediaType()
            throw (css::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getDescription()
            throw (css::deployment::ExtensionRemovedException,
                   css::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getShortDescription()
            throw (css::deployment::ExtensionRemovedException,
                   css::uno::RuntimeException);
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

    virtual ::sal_Int32 SAL_CALL checkPrerequisites(
        const css::uno::Reference< css::task::XAbortChannel >& xAbortChannel,
        const css::uno::Reference< css::ucb::XCommandEnvironment >& xCmdEnv,
        sal_Bool noLicenseChecking)
        throw (css::deployment::DeploymentException,
               css::deployment::ExtensionRemovedException,
               css::ucb::CommandFailedException,
               css::ucb::CommandAbortedException,
               css::uno::RuntimeException);

    virtual ::sal_Bool SAL_CALL checkDependencies(
        const css::uno::Reference< css::ucb::XCommandEnvironment >& xCmdEnv )
        throw (css::deployment::DeploymentException,
               css::deployment::ExtensionRemovedException,
               css::ucb::CommandFailedException,
               css::uno::RuntimeException);

    virtual void SAL_CALL registerPackage(
        sal_Bool startup,
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        throw (css::deployment::DeploymentException,
               css::deployment::ExtensionRemovedException,
               css::ucb::CommandFailedException,
               css::ucb::CommandAbortedException,
               css::lang::IllegalArgumentException, css::uno::RuntimeException);
    virtual void SAL_CALL revokePackage(
        sal_Bool startup,
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
    virtual css::beans::Optional< ::rtl::OUString > SAL_CALL getIdentifier()
        throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getVersion()
        throw (css::deployment::ExtensionRemovedException,
               css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getURL()
        throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getDisplayName()
        throw (css::deployment::ExtensionRemovedException,
               css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getDescription()
        throw (css::deployment::ExtensionRemovedException,
               css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getLicenseText()
        throw (css::deployment::ExtensionRemovedException,
               css::uno::RuntimeException);
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL
    getUpdateInformationURLs()
        throw (css::deployment::ExtensionRemovedException,
               css::uno::RuntimeException);
    virtual css::beans::StringPair SAL_CALL getPublisherInfo()
        throw (css::deployment::ExtensionRemovedException,
               css::uno::RuntimeException);
    virtual css::uno::Reference< css::graphic::XGraphic > SAL_CALL
    getIcon( sal_Bool bHighContrast )
        throw (css::deployment::ExtensionRemovedException,
               css::uno::RuntimeException);
    virtual css::uno::Reference<css::deployment::XPackageTypeInfo> SAL_CALL
    getPackageType() throw (css::uno::RuntimeException);
    virtual void SAL_CALL exportTo(
        ::rtl::OUString const & destFolderURL,
        ::rtl::OUString const & newTitle,
        sal_Int32 nameClashAction,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        throw (css::deployment::ExtensionRemovedException,
               css::ucb::CommandFailedException,
               css::ucb::CommandAbortedException, css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getRepositoryName()
        throw (css::uno::RuntimeException);
    virtual css::beans::Optional< ::rtl::OUString > SAL_CALL getRegistrationDataURL()
        throw (css::deployment::ExtensionRemovedException,
               css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isRemoved()
        throw (css::uno::RuntimeException);

};

typedef ::cppu::WeakComponentImplHelper2<
    css::lang::XEventListener,
    css::deployment::XPackageRegistry > t_BackendBase;

//==============================================================================
class PackageRegistryBackend
    : protected ::dp_misc::MutexHolder, public t_BackendBase
{
    //The map held originally WeakReferences. The map entries are removed in the disposing
    //function, which is called when the XPackages are destructed or they are
    //explicitly disposed. The latter happens, for example, when a extension is
    //removed (see dp_manager.cxx). However, because of how the help systems work, now
    // XPackageManager::getDeployedPackages is called often. This results in a lot
    //of bindPackage calls which are costly. Therefore we keep hard references in
    //the map now.
    typedef ::boost::unordered_map<
        ::rtl::OUString, css::uno::Reference<css::deployment::XPackage>,
        ::rtl::OUStringHash > t_string2ref;
    t_string2ref m_bound;

protected:
    ::rtl::OUString m_cachePath;
    css::uno::Reference<css::uno::XComponentContext> m_xComponentContext;

    ::rtl::OUString m_context;
    // currently only for library containers:
    enum {
        CONTEXT_UNKNOWN,
        CONTEXT_USER, CONTEXT_SHARED,CONTEXT_BUNDLED, CONTEXT_TMP,
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
        sal_Bool bRemoved, ::rtl::OUString const & identifier,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        = 0;

    void check();
    virtual void SAL_CALL disposing();

    virtual ~PackageRegistryBackend();
    PackageRegistryBackend(
        css::uno::Sequence<css::uno::Any> const & args,
        css::uno::Reference<css::uno::XComponentContext> const & xContext );

    /* creates a folder with a unique name.
       If url is empty then it is created in the the backend folder, otherwise
       at a location relative to that folder specified by url.
    */
    ::rtl::OUString createFolder(
        ::rtl::OUString const & relUrl,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv);
    /* deletes folders and files.

       All folder all files which end with ".tmp" or ".tmp_" and which are
       not used are deleted.
     */
    void deleteUnusedFolders(
        ::rtl::OUString const & relUrl,
        ::std::list< ::rtl::OUString> const & usedFolders);
    /* deletes one folder with a "temporary" name and the corresponding
       tmp file, which was used to derive the folder name.
    */
    static void deleteTempFolder(
        ::rtl::OUString const & folderUrl);

    ::rtl::OUString getSharedRegistrationDataURL(
        css::uno::Reference<css::deployment::XPackage> const & extension,
        css::uno::Reference<css::deployment::XPackage> const & item);

    /* The backends must implement this function, which is called
       from XPackageRegistry::packageRemoved (also implemented here).
       This ensure that the backends clean up their registration data
       when an extension was removed.
    */
//    virtual void deleteDbEntry( ::rtl::OUString const & url) = 0;



public:
    struct StrRegisteringPackage : public ::dp_misc::StaticResourceString<
        StrRegisteringPackage, RID_STR_REGISTERING_PACKAGE> {};
    struct StrRevokingPackage : public ::dp_misc::StaticResourceString<
        StrRevokingPackage, RID_STR_REVOKING_PACKAGE> {};

    inline css::uno::Reference<css::uno::XComponentContext> const &
    getComponentContext() const { return m_xComponentContext; }

    inline ::rtl::OUString const & getCachePath() const { return m_cachePath; }
    inline bool transientMode() const { return m_cachePath.isEmpty(); }

    inline ::rtl::OUString getContext() const {return m_context; }

    // XEventListener
    virtual void SAL_CALL disposing( css::lang::EventObject const & evt )
        throw (css::uno::RuntimeException);

    // XPackageRegistry
    virtual css::uno::Reference<css::deployment::XPackage> SAL_CALL bindPackage(
        ::rtl::OUString const & url, ::rtl::OUString const & mediaType,
        sal_Bool bRemoved, ::rtl::OUString const & identifier,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        throw (css::deployment::DeploymentException,
               css::deployment::InvalidRemovedParameterException,
               css::ucb::CommandFailedException,
               css::lang::IllegalArgumentException, css::uno::RuntimeException);

//     virtual void SAL_CALL packageRemoved(
//         ::rtl::OUString const & url, ::rtl::OUString const & mediaType)
//         throw (css::deployment::DeploymentException,
//                css::uno::RuntimeException);

};

}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
