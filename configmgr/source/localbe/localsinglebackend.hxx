/*************************************************************************
 *
 *  $RCSfile: localsinglebackend.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: cyrillem $ $Date: 2002-06-07 16:43:14 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef CONFIGMGR_LOCALBE_LOCALSINGLEBACKEND_HXX_
#define CONFIGMGR_LOCALBE_LOCALSINGLEBACKEND_HXX_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XSINGLEBACKEND_HPP_
#include <drafts/com/sun/star/configuration/backend/XSingleBackend.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_XSINGLEBACKEND_HPP_

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif // _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif // _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif // _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_

#ifndef _CPPUHELPER_COMPBASE3_HXX_
#include <cppuhelper/compbase3.hxx>
#endif // _CPPUHELPER_COMPBASE3_HXX_

namespace configmgr {

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
//namespace backend = css::configuration::backend ;
namespace backend = drafts::com::sun::star::configuration::backend ;

typedef cppu::WeakComponentImplHelper3<backend::XSingleBackend,
                                       lang::XInitialization,
                                       lang::XServiceInfo> SingleBackendBase ;

/**
  Implements the SingleBackend service for local file access.
  Layer identifiers in that backend are file URLs.
  */
class LocalSingleBackend : public SingleBackendBase {
    public :
        /**
          Service constructor from a component context.

          @param xContext   component context
          */
        LocalSingleBackend(
                const uno::Reference<uno::XComponentContext>& xContext) ;
        /** Destructor */
        ~LocalSingleBackend(void) ;

        // XInitialize
        virtual void SAL_CALL initialize(
                                const uno::Sequence<uno::Any>& aParameters)
            throw (uno::RuntimeException, uno::Exception) ;
        // XSingleBackend
        virtual uno::Sequence<rtl::OUString> SAL_CALL listLayerIds(
                                                const rtl::OUString& aComponent,
                                                const rtl::OUString& aEntity)
            throw (backend::BackendAccessException,
                    lang::IllegalArgumentException,
                    uno::RuntimeException) ;
        virtual rtl::OUString SAL_CALL getUpdateLayerId(
                                                const rtl::OUString& aComponent,
                                                const rtl::OUString& aEntity)
            throw (backend::BackendAccessException,
                    lang::IllegalArgumentException,
                    uno::RuntimeException) ;
        virtual rtl::OUString SAL_CALL getOwnId(void)
            throw (uno::RuntimeException) ;
        virtual uno::Reference<backend::XLayer> SAL_CALL getLayer(
                                            const rtl::OUString& aLayerId,
                                            const rtl::OUString& aTimestamp)
            throw (backend::BackendAccessException,
                    lang::IllegalArgumentException,
                    uno::RuntimeException) ;
        virtual uno::Sequence<uno::Reference<backend::XLayer> > SAL_CALL
            getLayers(const uno::Sequence<rtl::OUString>& aLayerIds,
                      const rtl::OUString& aTimestamp)
            throw (backend::BackendAccessException,
                    lang::IllegalArgumentException,
                    uno::RuntimeException) ;
        virtual uno::Sequence<uno::Reference<backend::XLayer> > SAL_CALL
            getMultipleLayers(const uno::Sequence<rtl::OUString>& aLayerIds,
                              const uno::Sequence<rtl::OUString>& aTimestamps)
            throw (backend::BackendAccessException,
                    lang::IllegalArgumentException,
                    uno::RuntimeException) ;
        virtual uno::Reference<backend::XUpdatableLayer> SAL_CALL
            getUpdatableLayer(const rtl::OUString& aLayerId)
            throw (backend::BackendAccessException,
                    lang::IllegalArgumentException,
                    uno::RuntimeException) ;
        virtual uno::Reference<backend::XLayerHandler> SAL_CALL
            getWriteHandler(const rtl::OUString& aLayerId)
            throw (backend::BackendAccessException,
                    lang::IllegalArgumentException,
                    uno::RuntimeException) ;
        virtual uno::Reference<backend::XSchema> SAL_CALL getSchema(
                                            const rtl::OUString& aComponent)
            throw (backend::BackendAccessException,
                    lang::IllegalArgumentException,
                    uno::RuntimeException) ;
        // XServiceInfo
        virtual rtl::OUString SAL_CALL getImplementationName(void)
            throw (uno::RuntimeException) ;
        virtual sal_Bool SAL_CALL supportsService(
                                            const rtl::OUString& aServiceName)
            throw (uno::RuntimeException) ;
        virtual uno::Sequence<rtl::OUString> SAL_CALL
            getSupportedServiceNames(void)
            throw (uno::RuntimeException) ;

        /**
          Provides the implementation name.

          @return   implementation name
          */
        static rtl::OUString SAL_CALL getName(void) ;
        /**
          Provides the supported services names

          @return   service names
          */
        static uno::Sequence<rtl::OUString> SAL_CALL getServices(void) ;

    protected :
    private :
        /** Component context */
        const uno::Reference<uno::XComponentContext>& mContext ;
        /** Mutex for resources protection */
        osl::Mutex mMutex ;
        /** Identifier of the user of the backend */
        rtl::OUString mOwnId ;
        /** Base of the shared data */
        rtl::OUString mSharedDataUrl ;
        /** Base of the user data */
        rtl::OUString mUserDataUrl ;
} ;

} // configmgr

#endif // CONFIGMGR_LOCALBE_LOCALSINGLEBACKEND_HXX_
