/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: importsvc.hxx,v $
 * $Revision: 1.7 $
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

#ifndef CONFIGMGR_BACKEND_IMPORTSVC_HXX
#define CONFIGMGR_BACKEND_IMPORTSVC_HXX

#include "serviceinfohelper.hxx"
#include <cppuhelper/implbase3.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/configuration/backend/XLayerImporter.hpp>
#include <com/sun/star/configuration/backend/XLayerHandler.hpp>

// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------
        namespace uno   = ::com::sun::star::uno;
        namespace lang  = ::com::sun::star::lang;
        namespace backenduno = ::com::sun::star::configuration::backend;
// -----------------------------------------------------------------------------

        class ImportService : public ::cppu::WeakImplHelper3<
                                            backenduno::XLayerImporter,
                                            lang::XInitialization,
                                            lang::XServiceInfo
                                        >
        {
        public:
            explicit
            ImportService(uno::Reference< uno::XComponentContext > const & _xContext, ServiceInfoHelper const & aSvcInfo);
            ~ImportService();

            // XInitialization
            virtual void SAL_CALL
                initialize( const uno::Sequence< uno::Any >& aArguments )
                    throw (uno::Exception, uno::RuntimeException);

            // XServiceInfo
            virtual rtl::OUString SAL_CALL
                getImplementationName(  )
                    throw (uno::RuntimeException);

            virtual sal_Bool SAL_CALL
                supportsService( const rtl::OUString& ServiceName )
                    throw (uno::RuntimeException);

            virtual uno::Sequence< rtl::OUString > SAL_CALL
                getSupportedServiceNames(  )
                    throw (uno::RuntimeException);

            // XLayerImporter
            virtual uno::Reference< backenduno::XBackend > SAL_CALL
                getTargetBackend(  )
                    throw (uno::RuntimeException);

            virtual void SAL_CALL
                setTargetBackend( const uno::Reference< backenduno::XBackend >& aBackend )
                    throw (lang::NullPointerException, uno::RuntimeException);

            virtual void SAL_CALL
                importLayer( const uno::Reference< backenduno::XLayer >& aLayer )
                throw ( backenduno::MalformedDataException,
                        lang::WrappedTargetException, lang::IllegalArgumentException,
                        lang::NullPointerException, uno::RuntimeException);

            virtual void SAL_CALL
                importLayerForEntity( const uno::Reference< backenduno::XLayer >& aLayer, const rtl::OUString& aEntity )
                    throw ( backenduno::MalformedDataException,
                            lang::WrappedTargetException, lang::IllegalArgumentException,
                            lang::NullPointerException, uno::RuntimeException);
        protected:
            uno::Reference< backenduno::XBackend > createDefaultBackend() const;

            //uno::Reference< lang::XMultiServiceFactory > getServiceFactory() const

            virtual sal_Bool setImplementationProperty( rtl::OUString const & aName, uno::Any const & aValue);
             /** Notify backend of import */
            sal_Bool        m_bSendNotification;
        private:
            // is pure virtual to allow different import services
            virtual uno::Reference< backenduno::XLayerHandler > createImportHandler(uno::Reference< backenduno::XBackend > const & xBackend, rtl::OUString const & aEntity = rtl::OUString()) = 0;

        private:
            osl::Mutex      m_aMutex;
            uno::Reference< uno::XComponentContext >         m_xContext;
            uno::Reference< backenduno::XBackend >         m_xDestinationBackend;


            ServiceInfoHelper m_aServiceInfo;

            ServiceInfoHelper const & getServiceInfo() const { return m_aServiceInfo; }
        };
// -----------------------------------------------------------------------------

        class MergeImportService : public ImportService
        {
        public:
            explicit MergeImportService(uno::Reference< uno::XComponentContext > const & _xContext);
        private:
            uno::Reference< backenduno::XLayerHandler > createImportHandler(uno::Reference< backenduno::XBackend > const & xBackend, rtl::OUString const & aEntity);
        };
// -----------------------------------------------------------------------------

        class CopyImportService : public ImportService
        {
        public:
            explicit CopyImportService(uno::Reference< uno::XComponentContext > const & _xContext);
        private:
            uno::Reference< backenduno::XLayerHandler > createImportHandler(uno::Reference< backenduno::XBackend > const & xBackend, rtl::OUString const & aEntity);
            sal_Bool setImplementationProperty( rtl::OUString const & aName, uno::Any const & aValue);
        private:
            sal_Bool m_bOverwrite;
        };
// -----------------------------------------------------------------------------
    } // namespace xml
// -----------------------------------------------------------------------------

} // namespace configmgr
#endif




