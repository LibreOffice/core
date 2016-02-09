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

#include <cppuhelper/factory.hxx>
#include <com/sun/star/container/XSet.hpp>
#include <osl/diagnose.h>
#include "cmdmailsuppl.hxx"


// namespace directives


using namespace ::com::sun::star::uno       ;
using namespace ::com::sun::star::container ;
using namespace ::com::sun::star::lang      ;
using namespace ::com::sun::star::registry  ;
using namespace ::cppu                      ;
using com::sun::star::system::XSimpleMailClientSupplier;

#define COMP_SERVICE_NAME  "com.sun.star.system.SimpleCommandMail"
#define COMP_IMPL_NAME     "com.sun.star.comp.system.SimpleCommandMail"


namespace
{
    Reference< XInterface > SAL_CALL createInstance( const Reference< XComponentContext >& xContext )
    {
        return Reference< XInterface >( static_cast< XSimpleMailClientSupplier* >( new CmdMailSuppl( xContext ) ) );
    }
}

extern "C"
{

// component_getFactory


SAL_DLLPUBLIC_EXPORT void* SAL_CALL cmdmail_component_getFactory(
    const sal_Char* pImplName,
    SAL_UNUSED_PARAMETER void* /*pSrvManager*/,
    SAL_UNUSED_PARAMETER void* /*pRegistryKey*/ )
{
    Reference< XSingleComponentFactory > xFactory;

    if (0 == ::rtl_str_compare( pImplName, COMP_IMPL_NAME ))
    {
        xFactory = ::cppu::createSingleComponentFactory(
            createInstance,
            COMP_IMPL_NAME,
            Sequence< OUString > { COMP_SERVICE_NAME } );
    }

    if (xFactory.is())
        xFactory->acquire();

    return xFactory.get();
}

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
