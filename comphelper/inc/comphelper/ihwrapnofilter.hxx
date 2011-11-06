/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _COMPHELPER_INTERACTIONHANDLERWRAPPER_HXX_
#define _COMPHELPER_INTERACTIONHANDLERWRAPPER_HXX_

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACITONREQUEST_
#include <com/sun/star/task/XInteractionRequest.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_DOUBLEINITIALIZATIONEXCEPTION_
#include <com/sun/star/frame/DoubleInitializationException.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef INCLUDED_COMPHELPERDLLAPI_H
#include "comphelper/comphelperdllapi.h"
#endif

namespace comphelper {

    class COMPHELPER_DLLPUBLIC OIHWrapNoFilterDialog : public ::cppu::WeakImplHelper3
        <  ::com::sun::star::task::XInteractionHandler
        ,  ::com::sun::star::lang::XInitialization
        ,  ::com::sun::star::lang::XServiceInfo >
    {
        com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler > m_xInter;

    public:
        OIHWrapNoFilterDialog( com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler > xInteraction );
        ~OIHWrapNoFilterDialog();

        static ::rtl::OUString SAL_CALL impl_staticGetImplementationName();
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL impl_staticGetSupportedServiceNames();


        //____________________________________________________________________________________________________
        // XInteractionHandler
        //____________________________________________________________________________________________________

        virtual void SAL_CALL handle( const com::sun::star::uno::Reference< com::sun::star::task::XInteractionRequest >& xRequest)
            throw( com::sun::star::uno::RuntimeException );

        //____________________________________________________________________________________________________
        // XInitialization
        //____________________________________________________________________________________________________

        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
            throw ( ::com::sun::star::uno::Exception,
            ::com::sun::star::uno::RuntimeException,
            ::com::sun::star::frame::DoubleInitializationException ) ;

        //____________________________________________________________________________________________________
        // XServiceInfo
        //____________________________________________________________________________________________________

        virtual ::rtl::OUString SAL_CALL getImplementationName()
            throw ( ::com::sun::star::uno::RuntimeException );

        virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
            throw ( ::com::sun::star::uno::RuntimeException );

        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
            throw ( ::com::sun::star::uno::RuntimeException );

    };
}
#endif
