/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: updatesvc.hxx,v $
 * $Revision: 1.8 $
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

#ifndef CONFIGMGR_BACKEND_UPDATESVC_HXX
#define CONFIGMGR_BACKEND_UPDATESVC_HXX

#include "serviceinfohelper.hxx"
#include <cppuhelper/implbase3.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/configuration/backend/XUpdateHandler.hpp>

// -----------------------------------------------------------------------------

namespace com { namespace sun { namespace star { namespace configuration { namespace backend {
    class XLayerHandler;
    class XLayer;
} } } } }

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

        class UpdateService : public ::cppu::WeakImplHelper3<
                                            lang::XInitialization,
                                            lang::XServiceInfo,
                                            backenduno::XUpdateHandler
                                        >
        {
        public:
            explicit
            UpdateService(uno::Reference< uno::XComponentContext > const & _xContext);

            // XInitialization
            virtual void SAL_CALL
                initialize( const uno::Sequence< uno::Any >& aArguments )
                    throw (uno::Exception, uno::RuntimeException);

            // XServiceInfo
            virtual ::rtl::OUString SAL_CALL
                getImplementationName(  )
                    throw (uno::RuntimeException);

            virtual sal_Bool SAL_CALL
                supportsService( const ::rtl::OUString& ServiceName )
                    throw (uno::RuntimeException);

            virtual uno::Sequence< ::rtl::OUString > SAL_CALL
                getSupportedServiceNames(  )
                    throw (uno::RuntimeException);

        protected:
            uno::Reference< lang::XMultiServiceFactory > getServiceFactory() const
            { return m_xServiceFactory; }

            void  checkSourceLayer() SAL_THROW( (lang::IllegalAccessException) )
            { validateSourceLayerAndCheckNotEmpty(); }

            uno::Reference< backenduno::XLayer > getSourceLayer()   SAL_THROW( (lang::IllegalAccessException) );

            void writeUpdatedLayer(uno::Reference< backenduno::XLayer > const & _xLayer);

            virtual sal_Bool setImplementationProperty(rtl::OUString const & aName, uno::Any const & aValue);

            void raiseIllegalAccessException(sal_Char const * pMsg)
                SAL_THROW( (lang::IllegalAccessException) );

        private:
            bool validateSourceLayerAndCheckNotEmpty() SAL_THROW( (lang::IllegalAccessException) );

        private:
            uno::Reference< lang::XMultiServiceFactory >  m_xServiceFactory;
            uno::Reference< backenduno::XLayer >           m_xSourceLayer;
            uno::Reference< backenduno::XLayerHandler >     m_xLayerWriter;
            enum { merge, truncate, protect } m_aSourceMode;

            static ServiceInfoHelper getServiceInfo();
        };
// -----------------------------------------------------------------------------
    } // namespace xml
// -----------------------------------------------------------------------------

} // namespace configmgr
#endif




