/*************************************************************************
 *
 *  $RCSfile: multistratumbackend.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-06-18 15:48:56 $
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

#ifndef CONFIGMGR_BACKEND_MULTISTRATUMBACKEND_HXX_
#define CONFIGMGR_BACKEND_MULTISTRATUMBACKEND_HXX_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XBACKEND_HPP_
#include <com/sun/star/configuration/backend/XBackend.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_XBACKEND_HPP_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XBACKENDENTITIES_HPP_
#include <com/sun/star/configuration/backend/XBackendEntities.hpp>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XSCHEMASUPPLIER_HPP_
#include <com/sun/star/configuration/backend/XSchemaSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif // _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif // _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_

#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_INVALIDBOOTSTRAPFILEEXCEPTION_HPP_
#include <com/sun/star/configuration/InvalidBootstrapFileException.hpp>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_BACKENDSETUPEXCEPTION_HPP_
#include <com/sun/star/configuration/backend/BackendSetupException.hpp>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XBACKENDCHANGESNOTIFIER_HPP_
#include <com/sun/star/configuration/backend/XBackendChangesNotifier.hpp>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XBACKENDCHANGESLISTENER_HPP_
#include <com/sun/star/configuration/backend/XBackendChangesListener.hpp>
#endif

#ifndef _CPPUHELPER_COMPBASE7_HXX_
#include <cppuhelper/compbase7.hxx>
#endif // _CPPUHELPER_COMPBASE7_HXX_

#ifndef INCLUDED_MAP
#include <map>
#define INCLUDED_MAP
#endif

namespace configmgr { namespace backend {

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backenduno = css::configuration::backend ;

typedef cppu::WeakComponentImplHelper7< backenduno::XBackend,
                                        backenduno::XBackendEntities,
                                        backenduno::XSchemaSupplier,
                                        backenduno::XBackendChangesNotifier,
                                        backenduno::XBackendChangesListener,
                                        lang::XInitialization,
                                        lang::XServiceInfo> BackendBase ;


/**
  Class implementing the Backend service for multibackend access.
  It creates the required backends and coordinates access to them.
  */
class MultiStratumBackend : public BackendBase {
    public :
        /**
          Service constructor from a service factory.

          @param xContext   component context
          */
        explicit
        MultiStratumBackend(
                const uno::Reference<uno::XComponentContext>& xContext) ;
        /** Destructor  */
        ~MultiStratumBackend() ;

        // XInitialize
        virtual void SAL_CALL initialize(
                                const uno::Sequence<uno::Any>& aParameters)
            throw (uno::RuntimeException, uno::Exception,
                   css::configuration::InvalidBootstrapFileException,
                   backenduno::BackendSetupException) ;

        // XSchemaSupplier
        virtual uno::Reference<backenduno::XSchema>
            SAL_CALL getComponentSchema(const rtl::OUString& aComponent)
                throw (backenduno::BackendAccessException,
                        lang::IllegalArgumentException,
                        uno::RuntimeException) ;

        // XBackend
        virtual uno::Sequence<uno::Reference<backenduno::XLayer> >
            SAL_CALL listOwnLayers(const rtl::OUString& aComponent)
                throw (backenduno::BackendAccessException,
                        lang::IllegalArgumentException,
                        uno::RuntimeException) ;

        virtual uno::Reference<backenduno::XUpdateHandler>
            SAL_CALL getOwnUpdateHandler(const rtl::OUString& aComponent)
            throw (backenduno::BackendAccessException,
                    lang::IllegalArgumentException,
                    lang::NoSupportException,
                    uno::RuntimeException) ;
        virtual uno::Sequence<uno::Reference<backenduno::XLayer> > SAL_CALL
            listLayers(const rtl::OUString& aComponent,
                       const rtl::OUString& aEntity)
            throw (backenduno::BackendAccessException,
                    lang::IllegalArgumentException,
                    uno::RuntimeException) ;
        virtual uno::Reference<backenduno::XUpdateHandler> SAL_CALL
            getUpdateHandler(const rtl::OUString& aComponent,
                             const rtl::OUString& aEntity)
            throw (backenduno::BackendAccessException,
                    lang::IllegalArgumentException,
                    lang::NoSupportException,
                    uno::RuntimeException) ;

        // XBackendEntities
        virtual rtl::OUString SAL_CALL
            getOwnerEntity(  )
                throw (uno::RuntimeException);

        virtual rtl::OUString SAL_CALL
            getAdminEntity(  )
                throw (uno::RuntimeException);

        virtual sal_Bool SAL_CALL
            supportsEntity( const rtl::OUString& aEntity )
                throw (backenduno::BackendAccessException, uno::RuntimeException);

        virtual sal_Bool SAL_CALL
            isEqualEntity( const rtl::OUString& aEntity, const rtl::OUString& aOtherEntity )
                throw ( backenduno::BackendAccessException,
                        lang::IllegalArgumentException,
                        uno::RuntimeException);

        // XServiceInfo
        virtual rtl::OUString SAL_CALL getImplementationName()
            throw (uno::RuntimeException) ;
        virtual sal_Bool SAL_CALL supportsService(
                                            const rtl::OUString& aServiceName)
            throw (uno::RuntimeException) ;
        virtual uno::Sequence<rtl::OUString> SAL_CALL
            getSupportedServiceNames(void) throw (uno::RuntimeException) ;

        // XBackendChangesNotifier
        virtual void SAL_CALL addChangesListener( const uno::Reference<backenduno::XBackendChangesListener>& xListner,
                                                  const rtl::OUString& aComponent)
            throw (::com::sun::star::uno::RuntimeException);


        virtual void SAL_CALL removeChangesListener( const uno::Reference<backenduno::XBackendChangesListener>& xListner,
                                                     const rtl::OUString& aComponent)
            throw (::com::sun::star::uno::RuntimeException);

       // XBackendChangesListener
        virtual void SAL_CALL componentDataChanged(const backenduno::ComponentChangeEvent& aEvent)
            throw (::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL disposing( lang::EventObject const & rSource )
           throw (uno::RuntimeException);

        void notifyListeners(const backenduno::ComponentChangeEvent& aEvent) const;
  protected:
    // ComponentHelper
        virtual void SAL_CALL disposing();
   private :
        /** Initialize the schema supplier backend

        */
        void initializeSchemaSupplier(const uno::Reference<uno::XComponentContext>& aContext);

        /** Initialize strata(SingleLayer or MultiLayer) backend
        */
        void initializeBackendStrata(const uno::Reference<uno::XComponentContext>& aContext);

        /** Get Layers from Backend Strata
        */
        uno::Sequence<uno::Reference<backenduno::XLayer> >
            searchSupportingStrata(sal_Int32 nNumLayer,
                                   rtl::OUString aEntity,
                                   const rtl::OUString& aComponent)  ;


        /** Find the Stratum that supports the specified Entity
        *   @return Number of Supported Strata
        */
        sal_Int32 findSupportingStratum(const rtl::OUString& aEntity) ;


        /**
            Check state of MultiStratumBackend  -
            @return    true if not disposed/uninitialized
        */
        bool checkOkState();
        /** Service factory */
        uno::Reference<lang::XMultiServiceFactory> mFactory ;
        /** Mutex for resource protection */
        osl::Mutex mMutex ;
        typedef std::vector< uno::Reference <uno::XInterface> > BackendStrata;

        uno::Reference<backenduno::XSchemaSupplier> mSchemaSupplier ;
        /** list of all backends */
        BackendStrata mBackendStrata;

        rtl::OUString mOwnerEntity;

        /** Helper object that listens to the Strata Backends */
        typedef uno::Reference<backenduno::XBackendChangesListener> ListenerRef;
        ListenerRef mStrataListener;
        /** List of higher level listeners */
        typedef std::multimap<rtl::OUString, ListenerRef> ListenerList;
        ListenerList mListenerList;
} ;

} } // configmgr.backend

#endif // CONFIGMGR_BACKEND_MULTISTRATUMBACKEND_HXX_
