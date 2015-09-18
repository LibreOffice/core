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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_REGISTRY_INC_DP_BACKEND_H
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_REGISTRY_INC_DP_BACKEND_H

#include "dp_misc.h"
#include "dp_resource.h"
#include "dp_interact.h"
#include <rtl/ref.hxx>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/compbase.hxx>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/deployment/XPackageRegistry.hpp>
#include <com/sun/star/deployment/XPackageManager.hpp>
#include <com/sun/star/deployment/InvalidRemovedParameterException.hpp>
#include <list>
#include <unordered_map>
#include "dp_registry.hrc"

namespace dp_registry
{
namespace backend
{

class PackageRegistryBackend;

#define BACKEND_SERVICE_NAME "com.sun.star.deployment.PackageRegistryBackend"

typedef ::cppu::WeakComponentImplHelper<
    css::deployment::XPackage > t_PackageBase;


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
    const OUString m_url;
    OUString m_name;
    OUString m_displayName;
    const css::uno::Reference<css::deployment::XPackageTypeInfo> m_xPackageType;
    const bool m_bRemoved;
    //Only set if m_bRemoved = true;
    const OUString m_identifier;

    void check() const;
    void fireModified();
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

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
             OUString const & url,
             OUString const & name,
             OUString const & displayName,
             css::uno::Reference<css::deployment::XPackageTypeInfo> const &
             xPackageType,
             bool bRemoved,
             OUString const & identifier);

public:

    class TypeInfo :
        public ::cppu::WeakImplHelper<css::deployment::XPackageTypeInfo>
    {
        const OUString m_mediaType;
        const OUString m_fileFilter;
        const OUString m_shortDescr;
        const sal_uInt16 m_smallIcon;
    public:
        virtual ~TypeInfo();
        TypeInfo( OUString const & mediaType,
                  OUString const & fileFilter,
                  OUString const & shortDescr,
                  sal_uInt16 smallIcon)
            : m_mediaType(mediaType), m_fileFilter(fileFilter),
              m_shortDescr(shortDescr),
              m_smallIcon(smallIcon)
            {}
        // XPackageTypeInfo
        virtual OUString SAL_CALL getMediaType()
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual OUString SAL_CALL getDescription()
            throw (css::deployment::ExtensionRemovedException,
                   css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual OUString SAL_CALL getShortDescription()
            throw (css::deployment::ExtensionRemovedException,
                   css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual OUString SAL_CALL getFileFilter()
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Any SAL_CALL getIcon( sal_Bool highContrast,
                                                sal_Bool smallIcon )
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    };

    // XComponent
    virtual void SAL_CALL dispose() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addEventListener(
        css::uno::Reference<css::lang::XEventListener> const & xListener )
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeEventListener(
        css::uno::Reference<css::lang::XEventListener> const & xListener )
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener(
        css::uno::Reference<css::util::XModifyListener> const & xListener )
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeModifyListener(
        css::uno::Reference<css::util::XModifyListener> const & xListener )
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XPackage
    virtual css::uno::Reference<css::task::XAbortChannel> SAL_CALL
    createAbortChannel() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::beans::Optional< css::beans::Ambiguous<sal_Bool> >
    SAL_CALL isRegistered(
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        throw (css::deployment::DeploymentException,
               css::ucb::CommandFailedException,
               css::ucb::CommandAbortedException,
               css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::sal_Int32 SAL_CALL checkPrerequisites(
        const css::uno::Reference< css::task::XAbortChannel >& xAbortChannel,
        const css::uno::Reference< css::ucb::XCommandEnvironment >& xCmdEnv,
        sal_Bool noLicenseChecking)
        throw (css::deployment::DeploymentException,
               css::deployment::ExtensionRemovedException,
               css::ucb::CommandFailedException,
               css::ucb::CommandAbortedException,
               css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::sal_Bool SAL_CALL checkDependencies(
        const css::uno::Reference< css::ucb::XCommandEnvironment >& xCmdEnv )
        throw (css::deployment::DeploymentException,
               css::deployment::ExtensionRemovedException,
               css::ucb::CommandFailedException,
               css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL registerPackage(
        sal_Bool startup,
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        throw (css::deployment::DeploymentException,
               css::deployment::ExtensionRemovedException,
               css::ucb::CommandFailedException,
               css::ucb::CommandAbortedException,
               css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL revokePackage(
        sal_Bool startup,
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        throw (css::deployment::DeploymentException,
               css::ucb::CommandFailedException,
               css::ucb::CommandAbortedException,
               css::lang::IllegalArgumentException,
               css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isBundle()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Sequence< css::uno::Reference<css::deployment::XPackage> >
    SAL_CALL getBundle(
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        throw (css::deployment::DeploymentException,
               css::ucb::CommandFailedException,
               css::ucb::CommandAbortedException,
               css::lang::IllegalArgumentException,
               css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::beans::Optional< OUString > SAL_CALL getIdentifier()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getVersion()
        throw (css::deployment::ExtensionRemovedException,
               css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getURL()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getDisplayName()
        throw (css::deployment::ExtensionRemovedException,
               css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getDescription()
        throw (css::deployment::ExtensionRemovedException,
               css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getLicenseText()
        throw (css::deployment::DeploymentException,
               css::deployment::ExtensionRemovedException,
               css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Sequence< OUString > SAL_CALL
    getUpdateInformationURLs()
        throw (css::deployment::ExtensionRemovedException,
               css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::beans::StringPair SAL_CALL getPublisherInfo()
        throw (css::deployment::ExtensionRemovedException,
               css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference< css::graphic::XGraphic > SAL_CALL
    getIcon( sal_Bool bHighContrast )
        throw (css::deployment::ExtensionRemovedException,
               css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference<css::deployment::XPackageTypeInfo> SAL_CALL
    getPackageType() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL exportTo(
        OUString const & destFolderURL,
        OUString const & newTitle,
        sal_Int32 nameClashAction,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        throw (css::deployment::ExtensionRemovedException,
               css::ucb::CommandFailedException, css::ucb::CommandAbortedException,
               css::ucb::ContentCreationException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getRepositoryName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::beans::Optional< OUString > SAL_CALL getRegistrationDataURL()
        throw (css::deployment::DeploymentException,
               css::deployment::ExtensionRemovedException,
               css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isRemoved()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

};

typedef ::cppu::WeakComponentImplHelper<
    css::lang::XEventListener,
    css::deployment::XPackageRegistry > t_BackendBase;


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
    typedef std::unordered_map<
        OUString, css::uno::Reference<css::deployment::XPackage>,
        OUStringHash > t_string2ref;
    t_string2ref m_bound;

protected:
    OUString m_cachePath;
    css::uno::Reference<css::uno::XComponentContext> m_xComponentContext;

    OUString m_context;
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
        OUString const & url, OUString const & mediaType,
        bool bRemoved, OUString const & identifier,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        = 0;

    void check();
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    virtual ~PackageRegistryBackend();
    PackageRegistryBackend(
        css::uno::Sequence<css::uno::Any> const & args,
        css::uno::Reference<css::uno::XComponentContext> const & xContext );

    /* creates a folder with a unique name.
       If url is empty then it is created in the backend folder, otherwise
       at a location relative to that folder specified by url.
    */
    OUString createFolder(
        OUString const & relUrl,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv);
    /* deletes folders and files.

       All folder all files which end with ".tmp" or ".tmp_" and which are
       not used are deleted.
     */
    void deleteUnusedFolders(
        OUString const & relUrl,
        ::std::list< OUString> const & usedFolders);
    /* deletes one folder with a "temporary" name and the corresponding
       tmp file, which was used to derive the folder name.
    */
    static void deleteTempFolder(
        OUString const & folderUrl);

public:
    struct StrRegisteringPackage : public ::dp_misc::StaticResourceString<
        StrRegisteringPackage, RID_STR_REGISTERING_PACKAGE> {};
    struct StrRevokingPackage : public ::dp_misc::StaticResourceString<
        StrRevokingPackage, RID_STR_REVOKING_PACKAGE> {};

    inline css::uno::Reference<css::uno::XComponentContext> const &
    getComponentContext() const { return m_xComponentContext; }

    inline OUString const & getCachePath() const { return m_cachePath; }
    inline bool transientMode() const { return m_cachePath.isEmpty(); }

    inline OUString getContext() const {return m_context; }

    // XEventListener
    virtual void SAL_CALL disposing( css::lang::EventObject const & evt )
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XPackageRegistry
    virtual css::uno::Reference<css::deployment::XPackage> SAL_CALL bindPackage(
        OUString const & url, OUString const & mediaType,
        sal_Bool bRemoved, OUString const & identifier,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        throw (css::deployment::DeploymentException,
               css::deployment::InvalidRemovedParameterException,
               css::ucb::CommandFailedException,
               css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

//     virtual void SAL_CALL packageRemoved(
//         OUString const & url, OUString const & mediaType)
//         throw (css::deployment::DeploymentException,
//                css::uno::RuntimeException);

};

}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
