/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <cppuhelper/implbase3.hxx>
#include "comphelper/comphelperdllapi.h"

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
