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

#ifndef INCLUDED_CPPUHELPER_SOURCE_SERVICEFACTORY_DETAIL_HXX
#define INCLUDED_CPPUHELPER_SOURCE_SERVICEFACTORY_DETAIL_HXX

#include "sal/config.h"

#include <vector>

#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "sal/types.h"

namespace com { namespace sun { namespace star { namespace lang {
    class XMultiComponentFactory;
} } } }
namespace cppu { struct ContextEntry_Init; }
namespace rtl {
    class Bootstrap;
    class OUString;
}

namespace cppu {

void add_access_control_entries(
    std::vector< cppu::ContextEntry_Init > * values,
    rtl::Bootstrap const & bootstrap)
    SAL_THROW((com::sun::star::uno::Exception));

SAL_DLLPUBLIC_EXPORT
com::sun::star::uno::Reference< com::sun::star::lang::XMultiComponentFactory >
bootstrapInitialSF(rtl::OUString const & rBootstrapPath)
    SAL_THROW((com::sun::star::uno::Exception));

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
