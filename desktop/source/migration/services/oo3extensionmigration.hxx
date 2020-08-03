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

#include "misc.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess3.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>

#include <osl/mutex.hxx>
#include <cppuhelper/implbase.hxx>

namespace com::sun::star::uno { class XComponentContext; }

class INetURLObject;


namespace migration
{

    typedef ::cppu::WeakImplHelper<
        css::lang::XServiceInfo,
        css::lang::XInitialization,
        css::task::XJob > ExtensionMigration_BASE;

    class OO3ExtensionMigration : public ExtensionMigration_BASE
    {
    private:
        css::uno::Reference< css::uno::XComponentContext >      m_ctx;
        css::uno::Reference< css::xml::dom::XDocumentBuilder >  m_xDocBuilder;
        css::uno::Reference< css::ucb::XSimpleFileAccess3 >     m_xSimpleFileAccess;
        ::osl::Mutex            m_aMutex;
        OUString                m_sSourceDir;
        OUString                m_sTargetDir;
        TStringVector           m_aDenyList;

        enum ScanResult
        {
            SCANRESULT_NOTFOUND,
            SCANRESULT_MIGRATE_EXTENSION,
            SCANRESULT_DONTMIGRATE_EXTENSION
        };

        ScanResult              scanExtensionFolder( const OUString& sExtFolder );
        void                    scanUserExtensions( const OUString& sSourceDir, TStringVector& aMigrateExtensions );
        bool                    scanDescriptionXml( const OUString& sDescriptionXmlFilePath );
        void                    migrateExtension( const OUString& sSourceDir );

    public:
        explicit OO3ExtensionMigration(css::uno::Reference<
            css::uno::XComponentContext > const & ctx);
        virtual ~OO3ExtensionMigration() override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

        // XJob
        virtual css::uno::Any SAL_CALL execute(
            const css::uno::Sequence< css::beans::NamedValue >& Arguments ) override;
    };

    class TmpRepositoryCommandEnv
        : public ::cppu::WeakImplHelper< css::ucb::XCommandEnvironment,
                                          css::task::XInteractionHandler,
                                          css::ucb::XProgressHandler >
    {
    public:
        virtual ~TmpRepositoryCommandEnv() override;
        TmpRepositoryCommandEnv();

        // XCommandEnvironment
        virtual css::uno::Reference< css::task::XInteractionHandler > SAL_CALL
        getInteractionHandler() override;
        virtual css::uno::Reference< css::ucb::XProgressHandler >
        SAL_CALL getProgressHandler() override;

        // XInteractionHandler
        virtual void SAL_CALL handle(
            css::uno::Reference< css::task::XInteractionRequest > const & xRequest ) override;

        // XProgressHandler
        virtual void SAL_CALL push( css::uno::Any const & Status ) override;
        virtual void SAL_CALL update( css::uno::Any const & Status ) override;
        virtual void SAL_CALL pop() override;
    };


}   // namespace migration


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
