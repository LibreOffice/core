/*************************************************************************
 *
 *  $RCSfile: importsvc.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:18:47 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef CONFIGMGR_BACKEND_IMPORTSVC_HXX
#define CONFIGMGR_BACKEND_IMPORTSVC_HXX

#ifndef CONFIGMGR_SERVICEINFOHELPER_HXX_
#include "serviceinfohelper.hxx"
#endif

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#include <drafts/com/sun/star/configuration/backend/XImportLayer.hpp>
#include <drafts/com/sun/star/configuration/backend/XLayerHandler.hpp>

// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------
        using rtl::OUString;
        namespace uno   = ::com::sun::star::uno;
        namespace lang  = ::com::sun::star::lang;
        namespace backenduno = drafts::com::sun::star::configuration::backend;
// -----------------------------------------------------------------------------

        class ImportService : public ::cppu::WeakImplHelper3<
                                            backenduno::XImportLayer,
                                            lang::XInitialization,
                                            lang::XServiceInfo
                                        >
        {
        public:
            typedef uno::Reference< uno::XComponentContext > const & CreationArg;

            explicit
            ImportService(CreationArg _xContext, ServiceInfoHelper const & aSvcInfo);
            ~ImportService();

            // XInitialization
            virtual void SAL_CALL
                initialize( const uno::Sequence< uno::Any >& aArguments )
                    throw (uno::Exception, uno::RuntimeException);

            // XServiceInfo
            virtual OUString SAL_CALL
                getImplementationName(  )
                    throw (uno::RuntimeException);

            virtual sal_Bool SAL_CALL
                supportsService( const OUString& ServiceName )
                    throw (uno::RuntimeException);

            virtual uno::Sequence< OUString > SAL_CALL
                getSupportedServiceNames(  )
                    throw (uno::RuntimeException);

            // XImportLayer
            virtual uno::Reference< backenduno::XBackend > SAL_CALL
                getTargetBackend(  )
                    throw (uno::RuntimeException);

            virtual void SAL_CALL
                setTargetBackend( const uno::Reference< backenduno::XBackend >& aBackend )
                    throw (lang::NullPointerException, uno::RuntimeException);

            virtual void SAL_CALL
                importLayer( const uno::Reference< backenduno::XLayer >& aLayer )
                    throw ( lang::WrappedTargetException, lang::IllegalArgumentException,
                            lang::NullPointerException, uno::RuntimeException);

            virtual void SAL_CALL
                importLayerForEntity( const uno::Reference< backenduno::XLayer >& aLayer, const OUString& aEntity )
                    throw ( lang::WrappedTargetException, lang::IllegalArgumentException,
                            lang::NullPointerException, uno::RuntimeException);
        protected:
            typedef uno::Reference< uno::XComponentContext >        Context;
            typedef uno::Reference< lang::XMultiServiceFactory >    ServiceFactory;
            typedef uno::Reference< backenduno::XBackend >          Backend;
            typedef uno::Reference< backenduno::XLayerHandler >     InputHandler;

            Backend createDefaultBackend() const;

            //ServiceFactory getServiceFactory() const

            virtual sal_Bool setImplementationProperty( OUString const & aName, uno::Any const & aValue);
        private:
            // is pure virtual to allow different import services
            virtual InputHandler createImportHandler(Backend const & xBackend, OUString const & aEntity = OUString()) = 0;

        private:
            osl::Mutex      m_aMutex;
            Context         m_xContext;
            Backend         m_xDestinationBackend;

            ServiceInfoHelper m_aServiceInfo;

            ServiceInfoHelper const & getServiceInfo() const { return m_aServiceInfo; }
        };
// -----------------------------------------------------------------------------

        class MergeImportService : public ImportService
        {
        public:
            explicit MergeImportService(CreationArg _xContext);
        private:
            InputHandler createImportHandler(Backend const & xBackend, OUString const & aEntity);
        };
// -----------------------------------------------------------------------------

        class CopyImportService : public ImportService
        {
        public:
            explicit CopyImportService(CreationArg _xContext);
        private:
            InputHandler createImportHandler(Backend const & xBackend, OUString const & aEntity);
            sal_Bool setImplementationProperty( OUString const & aName, uno::Any const & aValue);
        private:
            sal_Bool m_bOverwrite;
        };
// -----------------------------------------------------------------------------
    } // namespace xml
// -----------------------------------------------------------------------------

} // namespace configmgr
#endif




