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

#pragma once

#include <dp_misc.h>
#include <dp_shared.hxx>
#include <dp_interact.h>
#include <rtl/ref.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/compbase.hxx>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/deployment/XPackageRegistry.hpp>
#include <unordered_map>
#include <strings.hrc>

namespace dp_registry::backend
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
    virtual void SAL_CALL disposing() override;

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

    virtual ~Package() override;
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
    public:
        virtual ~TypeInfo() override;
        TypeInfo( OUString const & mediaType,
                  OUString const & fileFilter,
                  OUString const & shortDescr )
            : m_mediaType(mediaType), m_fileFilter(fileFilter),
              m_shortDescr(shortDescr)
            {}
        // XPackageTypeInfo
        virtual OUString SAL_CALL getMediaType() override;
        virtual OUString SAL_CALL getDescription() override;
        virtual OUString SAL_CALL getShortDescription() override;
        virtual OUString SAL_CALL getFileFilter() override;
        virtual css::uno::Any SAL_CALL getIcon( sal_Bool highContrast,
                                                sal_Bool smallIcon ) override;
    };

    // XComponent
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL addEventListener(
        css::uno::Reference<css::lang::XEventListener> const & xListener ) override;
    virtual void SAL_CALL removeEventListener(
        css::uno::Reference<css::lang::XEventListener> const & xListener ) override;

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener(
        css::uno::Reference<css::util::XModifyListener> const & xListener ) override;
    virtual void SAL_CALL removeModifyListener(
        css::uno::Reference<css::util::XModifyListener> const & xListener ) override;

    // XPackage
    virtual css::uno::Reference<css::task::XAbortChannel> SAL_CALL
    createAbortChannel() override;
    virtual css::beans::Optional< css::beans::Ambiguous<sal_Bool> >
    SAL_CALL isRegistered(
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv ) override;

    virtual ::sal_Int32 SAL_CALL checkPrerequisites(
        const css::uno::Reference< css::task::XAbortChannel >& xAbortChannel,
        const css::uno::Reference< css::ucb::XCommandEnvironment >& xCmdEnv,
        sal_Bool noLicenseChecking) override;

    virtual ::sal_Bool SAL_CALL checkDependencies(
        const css::uno::Reference< css::ucb::XCommandEnvironment >& xCmdEnv ) override;

    virtual void SAL_CALL registerPackage(
        sal_Bool startup,
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv ) override;
    virtual void SAL_CALL revokePackage(
        sal_Bool startup,
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv ) override;
    virtual sal_Bool SAL_CALL isBundle() override;
    virtual css::uno::Sequence< css::uno::Reference<css::deployment::XPackage> >
    SAL_CALL getBundle(
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv ) override;
    virtual OUString SAL_CALL getName() override;
    virtual css::beans::Optional< OUString > SAL_CALL getIdentifier() override;
    virtual OUString SAL_CALL getVersion() override;
    virtual OUString SAL_CALL getURL() override;
    virtual OUString SAL_CALL getDisplayName() override;
    virtual OUString SAL_CALL getDescription() override;
    virtual OUString SAL_CALL getLicenseText() override;
    virtual css::uno::Sequence< OUString > SAL_CALL
    getUpdateInformationURLs() override;
    virtual css::beans::StringPair SAL_CALL getPublisherInfo() override;
    virtual css::uno::Reference< css::graphic::XGraphic > SAL_CALL
    getIcon( sal_Bool bHighContrast ) override;
    virtual css::uno::Reference<css::deployment::XPackageTypeInfo> SAL_CALL
    getPackageType() override;
    virtual void SAL_CALL exportTo(
        OUString const & destFolderURL,
        OUString const & newTitle,
        sal_Int32 nameClashAction,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv ) override;
    virtual OUString SAL_CALL getRepositoryName() override;
    virtual css::beans::Optional< OUString > SAL_CALL getRegistrationDataURL() override;
    virtual sal_Bool SAL_CALL isRemoved() override;

};

typedef ::cppu::WeakComponentImplHelper<
    css::lang::XEventListener,
    css::deployment::XPackageRegistry,
    css::lang::XServiceInfo > t_BackendBase;


class PackageRegistryBackend
    : protected ::dp_misc::MutexHolder, public t_BackendBase
{
    //The map held originally WeakReferences. The map entries are removed in the disposing
    //function, which is called when the XPackages are destructed or they are
    //explicitly disposed. The latter happens, for example, when an extension is
    //removed (see dp_manager.cxx). However, because of how the help systems work, now
    // XPackageManager::getDeployedPackages is called often. This results in a lot
    //of bindPackage calls which are costly. Therefore we keep hard references in
    //the map now.
    typedef std::unordered_map<
        OUString, css::uno::Reference<css::deployment::XPackage> > t_string2ref;
    t_string2ref m_bound;

protected:
    OUString m_cachePath;
    css::uno::Reference<css::uno::XComponentContext> m_xComponentContext;

    OUString m_context;
    // currently only for library containers:
    enum class Context {
        Unknown, User, Shared, Bundled, Tmp, Document
    } m_eContext;

    static OUString StrCannotDetectMediaType() { return DpResId(RID_STR_CANNOT_DETECT_MEDIA_TYPE); }
    static OUString StrUnsupportedMediaType() { return DpResId(RID_STR_UNSUPPORTED_MEDIA_TYPE); }

    // @@@ to be implemented by specific backend:
    virtual css::uno::Reference<css::deployment::XPackage> bindPackage_(
        OUString const & url, OUString const & mediaType,
        bool bRemoved, OUString const & identifier,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        = 0;

    void check();
    virtual void SAL_CALL disposing() override;

    virtual ~PackageRegistryBackend() override;
    PackageRegistryBackend(
        css::uno::Sequence<css::uno::Any> const & args,
        css::uno::Reference<css::uno::XComponentContext> const & xContext );

    /* creates a folder with a unique name.
       If url is empty then it is created in the backend folder, otherwise
       at a location relative to that folder specified by url.
    */
    OUString createFolder(
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv);
    /* deletes folders and files.

       All folder all files which end with ".tmp" or ".tmp_" and which are
       not used are deleted.
     */
    void deleteUnusedFolders(
        std::vector< OUString> const & usedFolders);
    /* deletes one folder with a "temporary" name and the corresponding
       tmp file, which was used to derive the folder name.
    */
    static void deleteTempFolder(
        OUString const & folderUrl);

public:
    static OUString StrRegisteringPackage() { return DpResId(RID_STR_REGISTERING_PACKAGE); }
    static OUString StrRevokingPackage() { return DpResId(RID_STR_REVOKING_PACKAGE); }

    css::uno::Reference<css::uno::XComponentContext> const &
    getComponentContext() const { return m_xComponentContext; }

    OUString const & getCachePath() const { return m_cachePath; }
    bool transientMode() const { return m_cachePath.isEmpty(); }

    const OUString& getContext() const {return m_context; }

    // XEventListener
    virtual void SAL_CALL disposing( css::lang::EventObject const & evt ) override;

    // XPackageRegistry
    virtual css::uno::Reference<css::deployment::XPackage> SAL_CALL bindPackage(
        OUString const & url, OUString const & mediaType,
        sal_Bool bRemoved, OUString const & identifier,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv ) override;

//     virtual void SAL_CALL packageRemoved(
//         OUString const & url, OUString const & mediaType)
//         throw (css::deployment::DeploymentException,
//                css::uno::RuntimeException);

};

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
