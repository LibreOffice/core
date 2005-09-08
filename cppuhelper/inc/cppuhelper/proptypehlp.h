/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: proptypehlp.h,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 09:19:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
