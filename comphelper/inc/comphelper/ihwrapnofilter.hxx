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

#ifndef _COMPHELPER_INTERACTIONHANDLERWRAPPER_HXX_
#define _COMPHELPER_INTERACTIONHANDLERWRAPPER_HXX_

#include <com/sun/star/task/XInteractionHandler2.hpp>
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

#include <cppuhelper/implbase2.hxx>
#include "comphelper/comphelperdllapi.h"

namespace comphelper {

    class COMPHELPER_DLLPUBLIC OIHWrapNoFilterDialog : public ::cppu::WeakImplHelper2
        <  ::com::sun::star::task::XInteractionHandler2
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
        // XInteractionHandler2
        //____________________________________________________________________________________________________

        virtual sal_Bool SAL_CALL handleInteractionRequest( const com::sun::star::uno::Reference< com::sun::star::task::XInteractionRequest >& xRequest)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
