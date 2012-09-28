/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

#include "ucpext_provider.hxx"

#include <cppuhelper/implementationentry.hxx>

//......................................................................................................................
namespace ucb { namespace ucp { namespace ext
{
//......................................................................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::lang::XSingleComponentFactory;
    /** === end UNO using === **/

    //==================================================================================================================
    //= descriptors for the services implemented in this component
    //==================================================================================================================
    static struct ::cppu::ImplementationEntry s_aServiceEntries[] =
    {
        {
            ContentProvider::Create,
            ContentProvider::getImplementationName_static,
            ContentProvider::getSupportedServiceNames_static,
            ::cppu::createOneInstanceComponentFactory, NULL, 0
        },
        { 0, 0, 0, 0, 0, 0 }
    };

//......................................................................................................................
} } }   // namespace ucb::ucp::ext
//......................................................................................................................

extern "C"
{
    //------------------------------------------------------------------------------------------------------------------
    SAL_DLLPUBLIC_EXPORT void * SAL_CALL ucpext_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
    {
        return ::cppu::component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , ::ucb::ucp::ext::s_aServiceEntries );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
