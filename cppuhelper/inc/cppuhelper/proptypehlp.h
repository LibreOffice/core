/*************************************************************************
 *
 *  $RCSfile: proptypehlp.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-09 12:47:35 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CPPUHELPER_PROPTYPEHLP_H
#define _CPPUHELPER_PROPTYPEHLP_H

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uno/TypeClass.hpp>


namespace cppu {

/**
 * Convert a property value to the specified type of the property.
 * Look at the XPropertySet interface about the conversion rules.<BR>
 *
 * If a property could not be converted a IllegalArgumentException is thrown.
 *
 */

template < class target >
inline void SAL_CALL convertPropertyValue( target &value , const  ::com::sun::star::uno::Any & a)
//      SAL_THROW( (::com::sun::star::lang::IllegalArgumentException) )
        ;

// This template is needed at least for msci4 compiler
template < class target >
inline void SAL_CALL convertPropertyValue( target &value ,  ::com::sun::star::uno::Any & a)
//      SAL_THROW( (::com::sun::star::lang::IllegalArgumentException) )
        ;

/****
*
* conversion of basic types
*
*****/
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
