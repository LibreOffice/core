/*************************************************************************
 *
 *  $RCSfile: Any.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:25:50 $
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
#ifndef _COM_SUN_STAR_UNO_ANY_H_
#define _COM_SUN_STAR_UNO_ANY_H_

#ifndef _CPPU_MACROS_HXX_
#include <cppu/macros.hxx>
#endif

#ifndef _UNO_ANY2_H_
#include <uno/any2.h>
#endif
#ifndef _TYPELIB_TYPEDESCRIPTION_H_
#include <typelib/typedescription.h>
#endif
#ifndef _COM_SUN_STAR_UNO_TYPE_H_
#include <com/sun/star/uno/Type.h>
#endif

#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif

class BinaryCompatible_Impl;

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

/** C++ class representing an IDL any.
    This class is used to transport any type defined in IDL.
    The class inherits from the binary C representation
    of <b>uno_Any</b>.<br>
    You can insert a value by either using the <<= operators
    or the template function makeAny(). No any can hold an any.<br>
    You can extract values from an any by using the >>= operators
    which return true if the any contains an assignable value
    (no data loss), e.g. the any contains a short and you >>= it
    into a long variable.
    <br>
*/
class Any : public uno_Any
{
public:
    // these are here to force memory de/allocation to sal lib.
    inline static void * SAL_CALL operator new( size_t nSize ) throw()
        { return ::rtl_allocateMemory( nSize ); }
    inline static void SAL_CALL operator delete( void * pMem ) throw()
        { ::rtl_freeMemory( pMem ); }
    inline static void * SAL_CALL operator new( size_t, void * pMem ) throw()
        { return pMem; }
    inline static void SAL_CALL operator delete( void *, void * ) throw()
        {}

    /** Default constructor:
        Any holds no value; its type is void.
        <br>
    */
    inline Any();

    /** Copy constructor:
        Sets value of the given any.
        <br>
        @param rAny another any
    */
    inline Any( const Any & rAny );

    /** Constructor:
        Sets a copy of the given data.
        <br>
        @param pData value
        @param rType type of value
    */
    inline Any( const void * pData, const Type & rType );

    /** Constructor:
        Sets a copy of the given data.
        <br>
        @param pData value
        @param pTypeDescr type of value
    */
    inline Any( const void * pData, typelib_TypeDescription * pTypeDescr );

    /** Constructor:
        Sets a copy of the given data.
        <br>
        @param pData value
        @param pType type of value
    */
    inline Any( const void * pData, typelib_TypeDescriptionReference * pType );

    /** Destructor:
        Destructs any content and frees memory.
        <br>
    */
    inline ~Any();

    /** Assignment operator:
        Sets the value of the given any.
        <br>
        @param rAny another any (right side)
        @return this any
    */
    inline Any & SAL_CALL operator = ( const Any & rAny );

    /** Gets the type of the set value.
        <br>
        @return a Type object of the set value
     */
    inline const Type & SAL_CALL getValueType() const
        { return * reinterpret_cast< const Type * >( &pType ); }
    /** Gets the type of the set value.
        <br>
        @return the <b>un</b>acquired type description reference of the set value
     */
    inline typelib_TypeDescriptionReference * SAL_CALL getValueTypeRef() const
        { return pType; }

    /** Gets the type description of the set value.<br>
        Provides <b>ownership</b> of the type description!
        Call an explicit typelib_typedescription_release() to release.
        <br>
        @param a pointer to type description pointer
    */
    inline void SAL_CALL getValueTypeDescription( typelib_TypeDescription ** ppTypeDescr ) const
        { ::typelib_typedescriptionreference_getDescription( ppTypeDescr, getValueTypeRef() ); }

    /** Gets the type class of the set value.
        <br>
        @return the type class of the set value
     */
    inline TypeClass SAL_CALL getValueTypeClass() const
        { return (TypeClass)pType->eTypeClass; }

    /** Gets the type name of the set value.
        <br>
        @return the type name of the set value
    */
    inline ::rtl::OUString SAL_CALL getValueTypeName() const
        { return ::rtl::OUString( pType->pTypeName ); }

    /** Tests if any contains a value.
        <br>
        @return true if any has a value, false otherwise
    */
    inline sal_Bool SAL_CALL hasValue() const
        { return (TypeClass_VOID != getValueTypeClass()); }

    /** Gets a pointer to the set value.
        <br>
        @return a pointer to the set value
    */
    inline const void * SAL_CALL getValue() const
        { return pData; }

    /** Sets a value. If the any already contains a value, that value will be destructed
        and its memory freed.
        <br>
        @param pData pointer to value
        @param rType type of value
     */
    inline void SAL_CALL setValue( const void * pData, const Type & rType );
    /** Sets a value. If the any already contains a value, that value will be destructed
        and its memory freed.
        <br>
        @param pData pointer to value
        @param pType type of value
     */
    inline void SAL_CALL setValue( const void * pData, typelib_TypeDescriptionReference * pType );
    /** Sets a value. If the any already contains a value, that value will be destructed
        and its memory freed.
        <br>
        @param pData pointer to value
        @param pTypeDescr type description of value
     */
    inline void SAL_CALL setValue( const void * pData, typelib_TypeDescription * pTypeDescr );

    /** Clears this any. If the any already contains a value, that value will be destructed
        and its memory freed. After this has been called, the any does not contain a value.
        <br>
    */
    inline void SAL_CALL clear();

    /** Equality operator: compares two anys.<br>
        The values need not be of equal type, e.g. a short integer is compared to
        a long integer.
        <br>
        @param rAny another any (right side)
        @return true if both any contains equal values
     */
    inline sal_Bool SAL_CALL operator == ( const Any & rAny ) const;
    /** Unequality operator: compares two anys.<br>
        The values need not be of equal type, e.g. a short integer is compared to
        a long integer.
        <br>
        @param rAny another any (right side)
        @return true if both any contains unequal values
     */
    inline sal_Bool SAL_CALL operator != ( const Any & rAny ) const
        { return (! operator == ( rAny )); }

    // test the binary compatibility
    friend class BinaryCompatible_Impl;
};

/** Template function to generically construct an any from a C++ value.
    <br>
    @param value a value
    @return an any
 */
template< class C >
inline Any SAL_CALL makeAny( const C & value );

class BaseReference;
class Type;

/** Template binary <<= operator to set the value of an any.
    <br>
    @param rAny destination any (left side)
    @param value source value (right side)
 */
template< class C >
inline void SAL_CALL operator <<= ( ::com::sun::star::uno::Any & rAny, const C & value );
/** Template binary >>= operator to assign a value from an any.<br>
    If the any does not contain a value that can be assigned <b>without</b>
    data loss, this operation will fail returning false.
    <br>
    @param rAny source any (left side)
    @param value destination value (right side)
    @return true if assignment was possible without data loss
 */
template< class C >
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, C & value );

/** Template equality operator: compares set value of left side any to right side value.<br>
    The values need not be of equal type, e.g. a short integer is compared to
    a long integer.<br>
    This operator can be implemented as template member function, if
    all supported compilers can cope with template member functions.
    <br>
    @param rAny another any (left side)
    @param value a value (right side)
    @return true if values are equal, false otherwise
*/
template< class C >
inline sal_Bool SAL_CALL operator == ( const ::com::sun::star::uno::Any & rAny, const C & value );
/** Template unequality operator: compares set value of left side any to right side value.<br>
    The values need not be of equal type, e.g. a short integer is compared to
    a long integer.<br>
    This operator can be implemented as template member function, if
    all supported compilers can cope with template member functions.
    <br>
    @param rAny another any (left side)
    @param value a value (right side)
    @return true if values are unequal, false otherwise
*/
template< class C >
inline sal_Bool SAL_CALL operator != ( const ::com::sun::star::uno::Any & rAny, const C & value )
{
    return (! operator == ( rAny, value ));
}

// additional specialized >>= and == operators
// bool
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, sal_Bool & value );
inline sal_Bool SAL_CALL operator == ( const ::com::sun::star::uno::Any & rAny, const sal_Bool & value );
// byte
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, sal_Int8 & value );
// short
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, sal_Int16 & value );
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, sal_uInt16 & value );
// long
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, sal_Int32 & value );
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, sal_uInt32 & value );
// hyper
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, sal_Int64 & value );
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, sal_uInt64 & value );
// float
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, float & value );
// double
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, double & value );
// string
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, ::rtl::OUString & value );
inline sal_Bool SAL_CALL operator == ( const ::com::sun::star::uno::Any & rAny, const ::rtl::OUString & value );
// type
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, ::com::sun::star::uno::Type & value );
inline sal_Bool SAL_CALL operator == ( const ::com::sun::star::uno::Any & rAny, const ::com::sun::star::uno::Type & value );
// any
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, ::com::sun::star::uno::Any & value );
// interface
inline sal_Bool SAL_CALL operator == ( const ::com::sun::star::uno::Any & rAny, const ::com::sun::star::uno::BaseReference & value );

}
}
}
}

/** Gets the meta type of IDL type <b>any</b>.
    <br>
    @param dummy typed pointer for function signature
    @return type of IDL type <b>any</b>
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const ::com::sun::star::uno::Any * )
{
    return * reinterpret_cast< const ::com::sun::star::uno::Type * >(
        ::typelib_static_type_getByTypeClass( typelib_TypeClass_ANY ) );
}

#endif
