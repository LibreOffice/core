/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: legacysingletonfactory.hxx,v $
 * $Revision: 1.3 $
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

#ifndef COMPHELPER_LEGACYSINGLETONFACTORY_HXX
#define COMPHELPER_LEGACYSINGLETONFACTORY_HXX

#include "comphelper/comphelperdllapi.h"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <cppuhelper/factory.hxx>

//........................................................................
namespace comphelper
{
//........................................................................

/** creates a factory which can be used when implementing old-style singletons

    Before UNO having the component context concept, singletons were implemented by providing
    a factory which return the very same component instance for every creation request.

    Nowadays, singletons are implemented by making them available at the component context,
    as dedicated property.

    To bridge between both worlds - sometimes necessary to support legacy code, which instantiates
    singletons at the global service manager, instead of obtaining the property at the
    component context -, you can use the function below. I creates a single-component
    factory, which behaves like the old factories did: Upon multiple creation requests,
    it will always return the same instance. Additionally, the signature of the function
    is the same as the signature of <code>::cppu::createSingleComponentFactory</code>
    (which creates a "normal" component factory), so you can use both factory functions
    in the same context.
*/
::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleComponentFactory >
    COMPHELPER_DLLPUBLIC createLegacySingletonFactory(
        ::cppu::ComponentFactoryFunc _componentFactoryFunc,
        const ::rtl::OUString& _rImplementationName,
        const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rServiceNames,
        rtl_ModuleCount* _pModCount = NULL
    );

//........................................................................
} // namespace comphelper
//........................................................................

#endif // COMPHELPER_LEGACYSINGLETONFACTORY_HXX
