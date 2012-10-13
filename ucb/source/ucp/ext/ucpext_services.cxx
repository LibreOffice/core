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
