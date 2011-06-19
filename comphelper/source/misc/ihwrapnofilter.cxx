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

#include "precompiled_comphelper.hxx"

#include "comphelper/ihwrapnofilter.hxx"
#include <com/sun/star/document/NoSuchFilterRequest.hpp>

//.........................................................................
namespace comphelper
{
//.........................................................................

    using namespace ::com::sun::star;

    //----------------------------------------------------------------------------------------------------
    OIHWrapNoFilterDialog::OIHWrapNoFilterDialog( uno::Reference< task::XInteractionHandler > xInteraction )
        :m_xInter( xInteraction )
    {
    }

    OIHWrapNoFilterDialog::~OIHWrapNoFilterDialog()
    {
    }

    //----------------------------------------------------------------------------------------------------
    uno::Sequence< ::rtl::OUString > SAL_CALL OIHWrapNoFilterDialog::impl_staticGetSupportedServiceNames()
    {
        uno::Sequence< ::rtl::OUString > aRet(1);
        aRet[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.task.InteractionHandlerWrapper"));
        return aRet;
    }

    ::rtl::OUString SAL_CALL OIHWrapNoFilterDialog::impl_staticGetImplementationName()
    {
        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.task.InteractionHandlerWrapper"));
    }

    //----------------------------------------------------------------------------------------------------
    // XInteractionHandler
    //----------------------------------------------------------------------------------------------------
    void SAL_CALL OIHWrapNoFilterDialog::handle( const uno::Reference< task::XInteractionRequest >& xRequest)
            throw( com::sun::star::uno::RuntimeException )
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

    //----------------------------------------------------------------------------------------------------
    // XInitialization
    //----------------------------------------------------------------------------------------------------
    void SAL_CALL OIHWrapNoFilterDialog::initialize( const uno::Sequence< uno::Any >& )
        throw ( uno::Exception,
        uno::RuntimeException,
        frame::DoubleInitializationException )
    {
    }

    //----------------------------------------------------------------------------------------------------
    // XServiceInfo
    //----------------------------------------------------------------------------------------------------

    ::rtl::OUString SAL_CALL OIHWrapNoFilterDialog::getImplementationName()
        throw ( uno::RuntimeException )
    {
        return impl_staticGetImplementationName();
    }

    ::sal_Bool SAL_CALL OIHWrapNoFilterDialog::supportsService( const ::rtl::OUString& ServiceName )
        throw ( uno::RuntimeException )
    {
        uno::Sequence< ::rtl::OUString > aSeq = impl_staticGetSupportedServiceNames();

        for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
            if ( ServiceName.compareTo( aSeq[nInd] ) == 0 )
                return sal_True;

        return sal_False;
    }

    uno::Sequence< ::rtl::OUString > SAL_CALL OIHWrapNoFilterDialog::getSupportedServiceNames()
        throw ( uno::RuntimeException )
    {
        return impl_staticGetSupportedServiceNames();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
