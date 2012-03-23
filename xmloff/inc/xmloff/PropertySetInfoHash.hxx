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
#ifndef _XMLOFF_PROPERTYSETINFOHASH_HXX
#define _XMLOFF_PROPERTYSETINFOHASH_HXX

#include "sal/config.h"

#include <cstddef>

#include "com/sun/star/beans/XPropertySetInfo.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "osl/diagnose.h"
#include "sal/types.h"

struct PropertySetInfoHash
{
    inline std::size_t operator()( const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo >& r ) const;
    inline bool operator()( const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo >& r1,
                                const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo >& r2 ) const;
};

inline std::size_t PropertySetInfoHash::operator()(
        const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo >& r ) const
{
    OSL_ASSERT(r.is());
    return static_cast< std::size_t >(
        reinterpret_cast< sal_uIntPtr >(
            com::sun::star::uno::Reference< com::sun::star::uno::XInterface >(
                r, com::sun::star::uno::UNO_QUERY)
            .get()));
        // should be UNO_QUERY_THROW, but some clients are compiled with
        // EXCEPTIONS_OFF
}

inline bool PropertySetInfoHash::operator()(
        const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo >& r1,
        const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo >& r2 ) const
{
    return r1 == r2;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
