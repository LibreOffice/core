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
