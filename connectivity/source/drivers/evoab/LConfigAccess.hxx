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

#ifndef _CONNECTIVITY_EVOAB_LCONFIGACCESS_HXX_
#define _CONNECTIVITY_EVOAB_LCONFIGACCESS_HXX_

// This is the extended version (for use on the SO side of the driver) of MConfigAccess
// (which is for use on the mozilla side only)

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

namespace connectivity
{
    namespace evoab
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                createDriverConfigNode( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > _rxORB, ::rtl::OUString _sDriverImplementationName );
        ::rtl::OUString getFullPathExportingCommand( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > _rxORB );
    }
}
#endif // _CONNECTIVITY_EVOAB_LCONFIGACCESS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
