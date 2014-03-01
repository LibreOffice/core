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

#ifndef INCLUDED_COMPHELPER_IHWRAPNOFILTER_HXX
#define INCLUDED_COMPHELPER_IHWRAPNOFILTER_HXX

#include <com/sun/star/task/XInteractionHandler2.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/DoubleInitializationException.hpp>

#include <cppuhelper/implbase2.hxx>
#include <comphelper/comphelperdllapi.h>

namespace comphelper {

    class COMPHELPER_DLLPUBLIC OIHWrapNoFilterDialog : public ::cppu::WeakImplHelper2
        <  ::com::sun::star::task::XInteractionHandler2
        ,  ::com::sun::star::lang::XServiceInfo >
    {
        com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler > m_xInter;

    public:
        OIHWrapNoFilterDialog( com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler > xInteraction );
        ~OIHWrapNoFilterDialog();

        static OUString SAL_CALL impl_staticGetImplementationName();
        static ::com::sun::star::uno::Sequence< OUString > SAL_CALL impl_staticGetSupportedServiceNames();



        // XInteractionHandler


        virtual void SAL_CALL handle( const com::sun::star::uno::Reference< com::sun::star::task::XInteractionRequest >& xRequest)
            throw( com::sun::star::uno::RuntimeException, std::exception );


        // XInteractionHandler2


        virtual sal_Bool SAL_CALL handleInteractionRequest( const com::sun::star::uno::Reference< com::sun::star::task::XInteractionRequest >& xRequest)
            throw( com::sun::star::uno::RuntimeException, std::exception );


        // XInitialization


        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
            throw ( ::com::sun::star::uno::Exception,
            ::com::sun::star::uno::RuntimeException,
            ::com::sun::star::frame::DoubleInitializationException ) ;


        // XServiceInfo


        virtual OUString SAL_CALL getImplementationName()
            throw ( ::com::sun::star::uno::RuntimeException, std::exception );

        virtual ::sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw ( ::com::sun::star::uno::RuntimeException, std::exception );

        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw ( ::com::sun::star::uno::RuntimeException, std::exception );

    };
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
