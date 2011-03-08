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

#ifndef EXTENSIONS_RESOURCE_SERVICES_HXX
#define EXTENSIONS_RESOURCE_SERVICES_HXX

/** === begin UNO includes === **/
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XInterface.hpp>
/** === end UNO includes === **/
#include <cppuhelper/factory.hxx>

//........................................................................
namespace res
{
//........................................................................

    struct ComponentInfo
    {
        /// services supported by the component
        ::com::sun::star::uno::Sequence< ::rtl::OUString >  aSupportedServices;
        /// implementation name of the component
        ::rtl::OUString                                     sImplementationName;
        /** name of the singleton instance of the component, if it is a singleton, empty otherwise
            If the component is a singleton, aSupportedServices must contain exactly one element.
        */
        ::rtl::OUString                                     sSingletonName;
        /// factory for creating the component
        ::cppu::ComponentFactoryFunc                        pFactory;
    };

    ComponentInfo   getComponentInfo_VclStringResourceLoader();
    ComponentInfo   getComponentInfo_OpenOfficeResourceLoader();

//........................................................................
}   // namespace res
//........................................................................

#endif // EXTENSIONS_RESOURCE_SERVICES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
