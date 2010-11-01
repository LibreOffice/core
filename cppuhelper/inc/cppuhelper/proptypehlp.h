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
#ifndef _CPPUHELPER_PROPTYPEHLP_H
#define _CPPUHELPER_PROPTYPEHLP_H

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uno/TypeClass.hpp>


namespace cppu {

/** Converts the value stored in an any to a concrete C++ type.
    The function does the same as the operator >>= () at the
    Any class, except that it throws an IllegalArgumentException in case of
    failures (the value cannot be extracted without data loss )

   @exception com::sun::star::lang::IllegalArgumentException when the type could not be converted.
 */
template < class target >
inline void SAL_CALL convertPropertyValue( target &value , const  ::com::sun::star::uno::Any & a);

template < class target >
inline void SAL_CALL convertPropertyValue( target &value ,  ::com::sun::star::uno::Any & a);

/**
  conversion of basic types
*/
inline void SAL_CALL convertPropertyValue( sal_Int64 & target ,  const ::com::sun::star::uno::Any & source )
    SAL_THROW( (::com::sun::star::lang::IllegalArgumentException) );
inline void SAL_CALL convertPropertyValue( sal_uInt64 & target, const ::com::sun::star::uno::Any & source )
    SAL_THROW( (::com::sun::star::lang::IllegalArgumentException) );
inline void SAL_CALL convertPropertyValue( sal_Int32 & target , const ::com::sun::star::uno::Any & source )
    SAL_THROW( (::com::sun::star::lang::IllegalArgumentException) );
inline void SAL_CALL convertPropertyValue( sal_uInt32 & target, const ::com::sun::star::uno::Any & source )
    SAL_THROW( (::com::sun::star::lang::IllegalArgumentException) );
inline void SAL_CALL convertPropertyValue( sal_Int16 & target , const ::com::sun::star::uno::Any & source )
    SAL_THROW( (::com::sun::star::lang::IllegalArgumentException) );
inline void SAL_CALL convertPropertyValue( sal_uInt16 & target, const ::com::sun::star::uno::Any & source )
    SAL_THROW( (::com::sun::star::lang::IllegalArgumentException) );
inline void SAL_CALL convertPropertyValue( sal_Int8 & target     , const ::com::sun::star::uno::Any & source )
    SAL_THROW( (::com::sun::star::lang::IllegalArgumentException) );
inline void SAL_CALL convertPropertyValue( float & target     , const ::com::sun::star::uno::Any & source )
    SAL_THROW( (::com::sun::star::lang::IllegalArgumentException) );
inline void SAL_CALL convertPropertyValue( double &target     , const ::com::sun::star::uno::Any &source )
    SAL_THROW( (::com::sun::star::lang::IllegalArgumentException) );
inline void SAL_CALL convertPropertyValue( ::rtl::OUString &target  , const ::com::sun::star::uno::Any &source )
    SAL_THROW( (::com::sun::star::lang::IllegalArgumentException) );

} // end namespace cppu


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
