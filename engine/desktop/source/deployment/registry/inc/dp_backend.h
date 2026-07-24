/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <dp_shared.hxx>
#include <dp_interact.h>
#include <rtl/ref.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/compbase.hxx>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/deployment/XPackageRegistry.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <unordered_map>
#include <strings.hrc>
#include <utility>

namespace dp_registry::backend
{

class PackageRegistryBackend;

inline constexpr OUString BACKEND_SERVICE_NAME  = u"com.sun.star.deployment.PackageRegistryBackend"_ustr;

typedef ::cppu::WeakComponentImplHelper<
    css::deployment::XPackage > t_PackageBase;


class Package : protected cppu::BaseMutex, public t_PackageBase
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
    virtual void disposing() override;

    void checkAborted(
        ::rtl::Reference< ::dp_misc::AbortChannel > const & abortChannel );

    // @@@ to be implemented by specific backend:
    virtual css::beans::Optional< css::beans::Ambiguous<bool> >
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
    Package( ::rtl::Reference<PackageRegistryBackend> myBackend,
             OUString url,
             OUString name,
             OUString displayName,
             css::uno::Reference<css::deployment::XPackageTypeInfo> const & xPackageType,
             bool bRemoved,
             OUString identifier);

public:

    class TypeInfo :
        public ::cppu::WeakImplHelper<css::deployment::XPackageTypeInfo>
    {
        const OUString m_mediaType;
        const OUString m_fileFilter;
        const OUString m_shortDescr;
    public:
        virtual ~TypeInfo() override;
        TypeInfo( OUString mediaType,
                  OUString fileFilter,
                  OUString shortDescr )
            : m_mediaType(std::move(mediaType)), m_fileFilter(std::move(fileFilter)),
              m_shortDescr(std::move(shortDescr))
            {}
        // XPackageTypeInfo
        virtual OUString getMediaType() override;
        virtual OUString getDescription() override;
        virtual OUString getShortDescription() override;
        virtual OUString getFileFilter() override;
        virtual cpo::uno::Any getIcon( bool highContrast,
                                                bool smallIcon ) override;
    };

    // XComponent
    virtual void dispose() override;
    virtual void addEventListener(
        css::uno::Reference<css::lang::XEventListener> const & xListener ) override;
    virtual void removeEventListener(
        css::uno::Reference<css::lang::XEventListener> const & xListener ) override;

    // XModifyBroadcaster
    virtual void addModifyListener(
        css::uno::Reference<css::util::XModifyListener> const & xListener ) override;
    virtual void removeModifyListener(
        css::uno::Reference<css::util::XModifyListener> const & xListener ) override;

    // XPackage
    virtual css::uno::Reference<css::task::XAbortChannel>
    createAbortChannel() override;
    virtual css::beans::Optional< css::beans::Ambiguous<bool> >
    isRegistered(
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv ) override;

    virtual ::sal_Int32 checkPrerequisites(
        const css::uno::Reference< css::task::XAbortChannel >& xAbortChannel,
        const css::uno::Reference< css::ucb::XCommandEnvironment >& xCmdEnv,
        bool noLicenseChecking) override;

    virtual bool checkDependencies(
        const css::uno::Reference< css::ucb::XCommandEnvironment >& xCmdEnv ) override;

    virtual void registerPackage(
        bool startup,
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv ) override;
    virtual void revokePackage(
        bool startup,
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv ) override;
    virtual bool isBundle() override;
    virtual cpo::uno::Sequence< css::uno::Reference<css::deployment::XPackage> >
    getBundle(
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv ) override;
    virtual OUString getName() override;
    virtual css::beans::Optional< OUString > getIdentifier() override;
    virtual OUString getVersion() override;
    virtual OUString getURL() override;
    virtual OUString getDisplayName() override;
    virtual OUString getDescription() override;
    virtual OUString getLicenseText() override;
    virtual cpo::uno::Sequence< OUString >
    getUpdateInformationURLs() override;
    virtual css::beans::StringPair getPublisherInfo() override;
    virtual css::uno::Reference< css::graphic::XGraphic >
    getIcon( bool bHighContrast ) override;
    virtual css::uno::Reference<css::deployment::XPackageTypeInfo>
    getPackageType() override;
    virtual void exportTo(
        OUString const & destFolderURL,
        OUString const & newTitle,
        sal_Int32 nameClashAction,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv ) override;
    virtual OUString getRepositoryName() override;
    virtual css::beans::Optional< OUString > getRegistrationDataURL() override;
    virtual bool isRemoved() override;

};

typedef ::cppu::WeakComponentImplHelper<
    css::lang::XEventListener,
    css::deployment::XPackageRegistry,
    css::lang::XServiceInfo > t_BackendBase;


class PackageRegistryBackend
    : protected cppu::BaseMutex, public t_BackendBase
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
    virtual void disposing() override;

    virtual ~PackageRegistryBackend() override;
    PackageRegistryBackend(
        cpo::uno::Sequence<cpo::uno::Any> const & args,
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
    virtual void disposing( css::lang::EventObject const & evt ) override;

    // XPackageRegistry
    virtual css::uno::Reference<css::deployment::XPackage> bindPackage(
        OUString const & url, OUString const & mediaType,
        bool bRemoved, OUString const & identifier,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv ) override;

//     virtual void packageRemoved(
//         OUString const & url, OUString const & mediaType)
//         throw (css::deployment::DeploymentException,
//                css::uno::RuntimeException);

};

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
