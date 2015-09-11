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

#ifndef INCLUDED_DESKTOP_SOURCE_MIGRATION_SERVICES_BASICMIGRATION_HXX
#define INCLUDED_DESKTOP_SOURCE_MIGRATION_SERVICES_BASICMIGRATION_HXX

#include "misc.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>
#include <osl/file.hxx>


class INetURLObject;



namespace migration
{


    OUString SAL_CALL BasicMigration_getImplementationName();
    css::uno::Sequence< OUString > SAL_CALL BasicMigration_getSupportedServiceNames();
    css::uno::Reference< css::uno::XInterface > SAL_CALL BasicMigration_create(
        css::uno::Reference< css::uno::XComponentContext > const & xContext );



    // class BasicMigration


    typedef ::cppu::WeakImplHelper<
        css::lang::XServiceInfo,
        css::lang::XInitialization,
        css::task::XJob > BasicMigration_BASE;

    class BasicMigration : public BasicMigration_BASE
    {
    private:
        ::osl::Mutex            m_aMutex;
        OUString         m_sSourceDir;

        TStringVectorPtr        getFiles( const OUString& rBaseURL ) const;
        ::osl::FileBase::RC     checkAndCreateDirectory( INetURLObject& rDirURL );
        void                    copyFiles();

    public:
        BasicMigration();
        virtual ~BasicMigration();

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName()
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName )
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
            throw (css::uno::Exception, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XJob
        virtual css::uno::Any SAL_CALL execute(
            const css::uno::Sequence< css::beans::NamedValue >& Arguments )
            throw (css::lang::IllegalArgumentException, css::uno::Exception,
                css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    };


}   // namespace migration


#endif // INCLUDED_DESKTOP_SOURCE_MIGRATION_SERVICES_BASICMIGRATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
