/*************************************************************************
 *
 *  $RCSfile: Type.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:25:51 $
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
#ifndef _COM_SUN_STAR_UNO_TYPE_HXX_
#define _COM_SUN_STAR_UNO_TYPE_HXX_

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_TYPE_H_
#include <com/sun/star/uno/Type.h>
#endif


/** */ //for docpp
namespace com
{
/** */ //for docpp
namespace sun
{
/** */ //for docpp
namespace star
{
/** */ //for docpp
namespace uno
{

//__________________________________________________________________________________________________
inline Type::Type()
{
    _pType = reinterpret_cast< const ::com::sun::star::uno::Type * >(
        ::typelib_static_type_getByTypeClass( typelib_TypeClass_VOID ) )->getTypeLibType();
    ::typelib_typedescriptionreference_acquire( _pType );
}
//__________________________________________________________________________________________________
inline Type::Type( TypeClass eTypeClass, const ::rtl::OUString & rTypeName )
    : _pType( 0 )
{
    ::typelib_typedescriptionreference_new( &_pType, (typelib_TypeClass)eTypeClass, rTypeName.pData );
}
//__________________________________________________________________________________________________
inline Type::Type( TypeClass eTypeClass, const sal_Char * pTypeName )
    : _pType( 0 )
{
    ::typelib_typedescriptionreference_newByAsciiName( &_pType, (typelib_TypeClass)eTypeClass, pTypeName );
}
//__________________________________________________________________________________________________
inline Type::Type( typelib_TypeDescriptionReference * pType )
    : _pType( pType )
{
    ::typelib_typedescriptionreference_acquire( _pType );
}
//__________________________________________________________________________________________________
inline Type::Type( typelib_TypeDescriptionReference * pType, __UnoType_NoAcquire )
    : _pType( pType )
{
}
//__________________________________________________________________________________________________
inline Type::Type( const Type & rType )
    : _pType( rType._pType )
{
    ::typelib_typedescriptionreference_acquire( _pType );
}
//__________________________________________________________________________________________________
inline Type & Type::operator = ( const Type & rType )
{
    ::typelib_typedescriptionreference_assign( &_pType, rType._pType );
    return *this;
}

}
}
}
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const ::com::sun::star::uno::Type * )
{
    return * reinterpret_cast< const ::com::sun::star::uno::Type * >(
        ::typelib_static_type_getByTypeClass( typelib_TypeClass_TYPE ) );
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuVoidType()
{
    return * reinterpret_cast< const ::com::sun::star::uno::Type * >(
        ::typelib_static_type_getByTypeClass( typelib_TypeClass_VOID ) );
}
inline const ::com::sun::star::uno::Type & SAL_CALL getVoidCppuType()
{
    return * reinterpret_cast< const ::com::sun::star::uno::Type * >(
        ::typelib_static_type_getByTypeClass( typelib_TypeClass_VOID ) );
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuBooleanType()
{
    return * reinterpret_cast< const ::com::sun::star::uno::Type * >(
        ::typelib_static_type_getByTypeClass( typelib_TypeClass_BOOLEAN ) );
}
inline const ::com::sun::star::uno::Type & SAL_CALL getBooleanCppuType()
{
    return * reinterpret_cast< const ::com::sun::star::uno::Type * >(
        ::typelib_static_type_getByTypeClass( typelib_TypeClass_BOOLEAN ) );
}
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const sal_Bool * )
{
    return * reinterpret_cast< const ::com::sun::star::uno::Type * >(
        ::typelib_static_type_getByTypeClass( typelib_TypeClass_BOOLEAN ) );
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCharCppuType()
{
    return * reinterpret_cast< const ::com::sun::star::uno::Type * >(
        ::typelib_static_type_getByTypeClass( typelib_TypeClass_CHAR ) );
}
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuCharType()
{
    return * reinterpret_cast< const ::com::sun::star::uno::Type * >(
        ::typelib_static_type_getByTypeClass( typelib_TypeClass_CHAR ) );
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const sal_Int8 * )
{
    return * reinterpret_cast< const ::com::sun::star::uno::Type * >(
        ::typelib_static_type_getByTypeClass( typelib_TypeClass_BYTE ) );
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const ::rtl::OUString * )
{
    return * reinterpret_cast< const ::com::sun::star::uno::Type * >(
        ::typelib_static_type_getByTypeClass( typelib_TypeClass_STRING ) );
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const sal_Int16 * )
{
    return * reinterpret_cast< const ::com::sun::star::uno::Type * >(
        ::typelib_static_type_getByTypeClass( typelib_TypeClass_SHORT ) );
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const sal_uInt16 * )
{
    return * reinterpret_cast< const ::com::sun::star::uno::Type * >(
        ::typelib_static_type_getByTypeClass( typelib_TypeClass_UNSIGNED_SHORT ) );
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const sal_Int32 * )
{
    return * reinterpret_cast< const ::com::sun::star::uno::Type * >(
        ::typelib_static_type_getByTypeClass( typelib_TypeClass_LONG ) );
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const sal_uInt32 * )
{
    return * reinterpret_cast< const ::com::sun::star::uno::Type * >(
        ::typelib_static_type_getByTypeClass( typelib_TypeClass_UNSIGNED_LONG ) );
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const sal_Int64 * )
{
    return * reinterpret_cast< const ::com::sun::star::uno::Type * >(
        ::typelib_static_type_getByTypeClass( typelib_TypeClass_HYPER ) );
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const sal_uInt64 * )
{
    return * reinterpret_cast< const ::com::sun::star::uno::Type * >(
        ::typelib_static_type_getByTypeClass( typelib_TypeClass_UNSIGNED_HYPER ) );
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const float * )
{
    return * reinterpret_cast< const ::com::sun::star::uno::Type * >(
        ::typelib_static_type_getByTypeClass( typelib_TypeClass_FLOAT ) );
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const double * )
{
    return * reinterpret_cast< const ::com::sun::star::uno::Type * >(
        ::typelib_static_type_getByTypeClass( typelib_TypeClass_DOUBLE ) );
}

#endif
