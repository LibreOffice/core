/*************************************************************************
 *
 *  $RCSfile: Type.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-09 12:10:55 $
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
#ifndef _COM_SUN_STAR_UNO_TYPE_H_
#define _COM_SUN_STAR_UNO_TYPE_H_

#ifndef _TYPELIB_TYPEDESCRIPTION_H_
#include <typelib/typedescription.h>
#endif
#ifndef _COM_SUN_STAR_UNO_TYPECLASS_HDL_
#include <com/sun/star/uno/TypeClass.hdl>
#endif
#ifndef _CPPU_MACROS_HXX_
#include <cppu/macros.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
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

/** Enum defining UNO_TYPE_NO_ACQUIRE for type description reference transfer.
    <br>
*/
enum __UnoType_NoAcquire
{
    /** This enum value can be used for creating a Type object granting a given
        type description reference, i.e. transferring ownership to it.
        <br>
    */
    UNO_TYPE_NO_ACQUIRE
};

/** C++ class representing an IDL meta type.
    This class is used to represent a a type, i.e. a type name and its type class.<br>
    Internally the type holds a C type description reference of the runtime.
    You can obtain a full type description of a type by calling member function
    getDescription().
    <br>
*/
class Type
{
    /** the C typelib reference pointer<br>
    */
    typelib_TypeDescriptionReference * _pType;

public:
    // these are here to force memory de/allocation to sal lib.
    inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
        { return ::rtl_allocateMemory( nSize ); }
    inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
        { ::rtl_freeMemory( pMem ); }
    inline static void * SAL_CALL operator new( size_t, void * pMem ) SAL_THROW( () )
        { return pMem; }
    inline static void SAL_CALL operator delete( void *, void * ) SAL_THROW( () )
        {}

    /** Default Constructor:
        Type is set to void.
        <br>
    */
    inline Type() SAL_THROW( () );

    /** Constructor:
        Type is constructed by given name and type class.
        <br>
        @param eTypeClass type class of type
        @param rTypeName name of type
    */
    inline Type( TypeClass eTypeClass, const ::rtl::OUString & rTypeName ) SAL_THROW( () );

    /** Constructor:
        Type is constructed by given name and type class.
        <br>
        @param eTypeClass type class of type
        @param pTypeName name of type
    */
    inline Type( TypeClass eTypeClass, const sal_Char * pTypeName ) SAL_THROW( () );

    /** Constructor:
        Type is (copy) constructed by given C type description reference.
        <br>
        @param pType C type description reference
    */
    inline Type( typelib_TypeDescriptionReference * pType ) SAL_THROW( () );

    /** Constructor:
        Type is (copy) constructed by given C type description reference without acquiring it.
        <br>
        @param pType C type description reference
        @param dummy UNO_TYPE_NO_ACQUIRE to force obvious distinction to other constructors
    */
    inline Type( typelib_TypeDescriptionReference * pType, __UnoType_NoAcquire ) SAL_THROW( () );
    /** Constructor:
        Type is (copy) constructed by given C type description reference without acquiring it.
        <br>
        @param pType C type description reference
        @param dummy SAL_NO_ACQUIRE to force obvious distinction to other constructors
    */
    inline Type( typelib_TypeDescriptionReference * pType, __sal_NoAcquire ) SAL_THROW( () );

    /** Copy constructor:
        Type is copy constructed by given type.
        <br>
        @param rType another type
    */
    inline Type( const Type & rType ) SAL_THROW( () );

    /** Destructor:
        Releases acquired C type description reference.
        <br>
    */
    inline ~Type() SAL_THROW( () )
        { ::typelib_typedescriptionreference_release( _pType ); }

    /** Assignment operator:
        Acquires right side type and releases previously set type.
        <br>
        @param rType another type (right side)
        @return this type
    */
    inline Type & SAL_CALL operator = ( const Type & rType ) SAL_THROW( () );

    /** Gets the type class of set type.
        <br>
        @return type class of set type
    */
    inline TypeClass SAL_CALL getTypeClass() const SAL_THROW( () )
        { return (TypeClass)_pType->eTypeClass; }

    /** Gets the name of the set type.
        <br>
        @return name of the set type
    */
    inline ::rtl::OUString SAL_CALL getTypeName() const SAL_THROW( () )
        { return ::rtl::OUString( _pType->pTypeName ); }

    /** Obtains a full type description of set type.
        <br>
        @param ppDescr [inout] type description
    */
    inline void SAL_CALL getDescription( typelib_TypeDescription ** ppDescr ) const SAL_THROW( () )
        { ::typelib_typedescriptionreference_getDescription( ppDescr, _pType ); }

    /** Gets the C typelib type description reference pointer.
        Does <b>not</b> acquire the reference!
        <br>
        @return <b>un</b>acquired type description reference
    */
    inline typelib_TypeDescriptionReference * SAL_CALL getTypeLibType() const SAL_THROW( () )
        { return _pType; }

    /** Compares two types.
        <br>
        @param rType another type
        @return true if both types refer the same type, false otherwise
    */
    inline sal_Bool SAL_CALL equals( const Type & rType ) const SAL_THROW( () )
        { return ::typelib_typedescriptionreference_equals( _pType, rType._pType ); }
    /** Equality operator:
        Compares two types.
        <br>
        @param rType another type
        @return true if both types refer the same type, false otherwise
    */
    inline sal_Bool SAL_CALL operator == ( const Type & rType ) const SAL_THROW( () )
        { return equals( rType ); }
    /** Unequality operator:
        Compares two types.
        <br>
        @param rType another type
        @return false if both types refer the same type, true otherwise
    */
    inline sal_Bool SAL_CALL operator != ( const Type & rType ) const SAL_THROW( () )
        { return (! equals( rType )); }
};

}
}
}
}

/** Gets the meta type of IDL type <b>type</b>.
    <br>
    @param dummy typed pointer for function signature
    @return type of IDL type <b>type</b>
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const ::com::sun::star::uno::Type * ) SAL_THROW( () );

/** Gets the meta type of IDL type <b>void</b>.
    @return type of IDL type <b>void</b>
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuVoidType() SAL_THROW( () );
/** Gets the meta type of IDL type <b>void</b>.
    <br>
    @return type of IDL type <b>void</b>
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getVoidCppuType() SAL_THROW( () );

/** Gets the meta type of IDL type <b>boolean</b>.
    <br>
    @return type of IDL type <b>boolean</b>
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuBooleanType() SAL_THROW( () );
/** Gets the meta type of IDL type <b>boolean</b>.
    <br>
    @return type of IDL type <b>boolean</b>
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getBooleanCppuType() SAL_THROW( () );
/** Gets the meta type of IDL type <b>boolean</b>.
    <br>
    @param dummy typed pointer for function signature
    @return type of IDL type <b>boolean</b>
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const sal_Bool * ) SAL_THROW( () );

/** Gets the meta type of IDL type <b>char</b>.
    <br>
    @return type of IDL type <b>char</b>
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCharCppuType() SAL_THROW( () );
/** Gets the meta type of IDL type <b>char</b>.
    <br>
    @return type of IDL type <b>char</b>
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuCharType() SAL_THROW( () );

/** Gets the meta type of IDL type <b>byte</b>.
    <br>
    @param dummy typed pointer for function signature
    @return type of IDL type <b>byte</b>
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const sal_Int8 * ) SAL_THROW( () );

/** Gets the meta type of IDL type <b>string</b>.
    <br>
    @param dummy typed pointer for function signature
    @return type of IDL type <b>string</b>
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const ::rtl::OUString * ) SAL_THROW( () );

/** Gets the meta type of IDL type <b>short</b>.
    <br>
    @param dummy typed pointer for function signature
    @return type of IDL type <b>short</b>
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const sal_Int16 * ) SAL_THROW( () );

/** Gets the meta type of IDL type <b>unsigned short</b>.
    <br>
    @param dummy typed pointer for function signature
    @return type of IDL type <b>unsigned short</b>
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const sal_uInt16 * ) SAL_THROW( () );

/** Gets the meta type of IDL type <b>long</b>.
    <br>
    @param dummy typed pointer for function signature
    @return type of IDL type <b>long</b>
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const sal_Int32 * ) SAL_THROW( () );

/** Gets the meta type of IDL type <b>unsigned long</b>.
    <br>
    @param dummy typed pointer for function signature
    @return type of IDL type <b>unsigned long</b>
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const sal_uInt32 * ) SAL_THROW( () );

/** Gets the meta type of IDL type <b>hyper</b>.
    <br>
    @param dummy typed pointer for function signature
    @return type of IDL type <b>hyper</b>
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const sal_Int64 * ) SAL_THROW( () );

/** Gets the meta type of IDL type <b>unsigned hyper</b>.
    <br>
    @param dummy typed pointer for function signature
    @return type of IDL type <b>unsigned hyper</b>
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const sal_uInt64 * ) SAL_THROW( () );

/** Gets the meta type of IDL type <b>float</b>.
    <br>
    @param dummy typed pointer for function signature
    @return type of IDL type <b>float</b>
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const float * ) SAL_THROW( () );

/** Gets the meta type of IDL type <b>double</b>.
    <br>
    @param dummy typed pointer for function signature
    @return type of IDL type <b>double</b>
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const double * ) SAL_THROW( () );

#endif
