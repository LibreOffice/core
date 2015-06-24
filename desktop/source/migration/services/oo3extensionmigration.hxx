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

#ifndef INCLUDED_DESKTOP_SOURCE_MIGRATION_SERVICES_OO3EXTENSIONMIGRATION_HXX
#define INCLUDED_DESKTOP_SOURCE_MIGRATION_SERVICES_OO3EXTENSIONMIGRATION_HXX

#include "misc.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>

#include <osl/mutex.hxx>
#include <osl/file.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/compbase3.hxx>
#include <ucbhelper/content.hxx>

namespace com { namespace sun { namespace star {
    namespace uno {
        class XComponentContext;
    }
}}}

class INetURLObject;


namespace migration
{

    OUString SAL_CALL OO3ExtensionMigration_getImplementationName();
    ::com::sun::star::uno::Sequence< OUString > SAL_CALL OO3ExtensionMigration_getSupportedServiceNames();
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL OO3ExtensionMigration_create(
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext );



    // class ExtensionMigration


    typedef ::cppu::WeakImplHelper3<
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::lang::XInitialization,
        ::com::sun::star::task::XJob > ExtensionMigration_BASE;

    class OO3ExtensionMigration : public ExtensionMigration_BASE
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >      m_ctx;
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::dom::XDocumentBuilder >  m_xDocBuilder;
        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess3 >       m_xSimpleFileAccess;
        ::osl::Mutex            m_aMutex;
        OUString         m_sSourceDir;
        OUString         m_sTargetDir;
        TStringVector           m_aBlackList;

        enum ScanResult
        {
            SCANRESULT_NOTFOUND,
            SCANRESULT_MIGRATE_EXTENSION,
            SCANRESULT_DONTMIGRATE_EXTENSION
        };

        ::osl::FileBase::RC     checkAndCreateDirectory( INetURLObject& rDirURL );
        ScanResult              scanExtensionFolder( const OUString& sExtFolder );
        void                    scanUserExtensions( const OUString& sSourceDir, TStringVector& aMigrateExtensions );
        bool                    scanDescriptionXml( const OUString& sDescriptionXmlFilePath );
        void                    migrateExtension( const OUString& sSourceDir );

    public:
        explicit OO3ExtensionMigration(::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > const & ctx);
        virtual ~OO3ExtensionMigration();

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName()
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
            throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XJob
        virtual ::com::sun::star::uno::Any SAL_CALL execute(
            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& Arguments )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    };

    class TmpRepositoryCommandEnv
        : public ::cppu::WeakImplHelper3< ::com::sun::star::ucb::XCommandEnvironment,
                                          ::com::sun::star::task::XInteractionHandler,
                                          ::com::sun::star::ucb::XProgressHandler >
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler> m_forwardHandler;
    public:
        virtual ~TmpRepositoryCommandEnv();
        TmpRepositoryCommandEnv();

        // XCommandEnvironment
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler > SAL_CALL
        getInteractionHandler() throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XProgressHandler >
        SAL_CALL getProgressHandler() throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // XInteractionHandler
        virtual void SAL_CALL handle(
            ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionRequest > const & xRequest )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XProgressHandler
        virtual void SAL_CALL push( ::com::sun::star::uno::Any const & Status )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL update( ::com::sun::star::uno::Any const & Status )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL pop() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    };


}   // namespace migration


#endif // INCLUDED_DESKTOP_SOURCE_MIGRATION_SERVICES_OO3EXTENSIONMIGRATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
