/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
