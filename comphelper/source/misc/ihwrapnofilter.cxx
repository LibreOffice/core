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


#include "comphelper/ihwrapnofilter.hxx"
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/document/NoSuchFilterRequest.hpp>


namespace comphelper
{


    using namespace ::com::sun::star;


    OIHWrapNoFilterDialog::OIHWrapNoFilterDialog( uno::Reference< task::XInteractionHandler > xInteraction )
        :m_xInter( xInteraction )
    {
    }

    OIHWrapNoFilterDialog::~OIHWrapNoFilterDialog()
    {
    }


    uno::Sequence< OUString > SAL_CALL OIHWrapNoFilterDialog::impl_staticGetSupportedServiceNames()
    {
        uno::Sequence< OUString > aRet(1);
        aRet[0] = "com.sun.star.task.InteractionHandlerWrapper";
        return aRet;
    }

    OUString SAL_CALL OIHWrapNoFilterDialog::impl_staticGetImplementationName()
    {
        return OUString("com.sun.star.comp.task.InteractionHandlerWrapper");
    }


    // XInteractionHandler

    void SAL_CALL OIHWrapNoFilterDialog::handle( const uno::Reference< task::XInteractionRequest >& xRequest)
            throw( com::sun::star::uno::RuntimeException, std::exception )
    {
        if( !m_xInter.is() )
            return;

        uno::Any aRequest = xRequest->getRequest();
        document::NoSuchFilterRequest aNoSuchFilterRequest;
        if ( aRequest >>= aNoSuchFilterRequest )
            return;
        else
            m_xInter->handle( xRequest );
    }


    // XInteractionHandler2

    sal_Bool SAL_CALL OIHWrapNoFilterDialog::handleInteractionRequest( const uno::Reference< task::XInteractionRequest >& xRequest)
            throw( com::sun::star::uno::RuntimeException, std::exception )
    {
        if( !m_xInter.is() )
            return sal_False;

        uno::Any aRequest = xRequest->getRequest();
        document::NoSuchFilterRequest aNoSuchFilterRequest;
        if ( aRequest >>= aNoSuchFilterRequest )
            return sal_False;
        else
        {
            m_xInter->handle( xRequest );
            return sal_True;
        }
    }

    // XServiceInfo
    OUString SAL_CALL OIHWrapNoFilterDialog::getImplementationName()
        throw ( uno::RuntimeException, std::exception )
    {
        return impl_staticGetImplementationName();
    }

    sal_Bool SAL_CALL OIHWrapNoFilterDialog::supportsService( const OUString& ServiceName )
        throw ( uno::RuntimeException, std::exception )
    {
        return cppu::supportsService(this, ServiceName);
    }

    uno::Sequence< OUString > SAL_CALL OIHWrapNoFilterDialog::getSupportedServiceNames()
        throw ( uno::RuntimeException, std::exception )
    {
        return impl_staticGetSupportedServiceNames();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
