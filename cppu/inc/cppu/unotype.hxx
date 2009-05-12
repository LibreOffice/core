/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unotype.hxx,v $
 * $Revision: 1.5 $
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

#ifndef INCLUDED_CPPU_UNOTYPE_HXX
#define INCLUDED_CPPU_UNOTYPE_HXX

#include "sal/config.h"
#include "com/sun/star/uno/Type.h"
#include "sal/types.h"
#include "typelib/typeclass.h"
#include "typelib/typedescription.h"

namespace com { namespace sun { namespace star { namespace uno {
    class Any;
    class Exception;
    template< typename > class Reference;
    template< typename > class Sequence;
    class XInterface;
} } } }
namespace rtl { class OUString; }

namespace cppu {

template< typename > class UnoType;

/**
   A unique C++ type representing the UNO type VOID in cppu::UnoType.

   This type is declared but not defined.  Its only use is as a template
   argument to cppu::UnoType.

   @since UDK 3.2.2
*/
struct UnoVoidType;

/**
   A unique C++ type representing the UNO type UNSIGNED SHORT in cppu::UnoType.

   The UNO types UNSIGNED SHORT and CHAR map to the same C++ type, so this C++
   type is needed to unambiguously specify UNO types in cppu::UnoType.

   This type is declared but not defined.  Its only use is as a template
   argument to cppu::UnoType.

   @since UDK 3.2.2
*/
struct UnoUnsignedShortType;

/**
   A unique C++ type representing the UNO type UNSIGNED SHORT in cppu::UnoType.

   The UNO types UNSIGNED SHORT and CHAR map to the same C++ type, so this C++
   type is needed to unambiguously specify UNO types in cppu::UnoType.

   This type is declared but not defined.  Its only use is as a template
   argument to cppu::UnoType.

   @since UDK 3.2.2
*/
struct UnoCharType;

/**
   A unique C++ type template representing the UNO sequence types in
   cppu::UnoType.

   The UNO types UNSIGNED SHORT and CHAR map to the same C++ type, so this C++
   type is needed to unambiguously specify UNO types in cppu::UnoType.

   This type is declared but not defined.  Its only use is as a template
   argument to cppu::UnoType.

   @since UDK 3.2.2
*/
template< typename > struct UnoSequenceType;

namespace detail {

inline ::com::sun::star::uno::Type const & getTypeFromTypeDescriptionReference(
    ::typelib_TypeDescriptionReference * const * tdr)
{
    return *reinterpret_cast< ::com::sun::star::uno::Type const * >(tdr);
}

inline ::com::sun::star::uno::Type const &
getTypeFromTypeClass(::typelib_TypeClass tc) {
    return getTypeFromTypeDescriptionReference(
        ::typelib_static_type_getByTypeClass(tc));
}

}

}

// For _MSC_VER 1310, define cppu_detail_getUnoType in the global namespace, to
// avoid spurious compiler errors in code that calls cppu_detail_getUnoType:
#if !defined _MSC_VER || _MSC_VER > 1310
namespace cppu { namespace detail {
#endif

inline ::com::sun::star::uno::Type const &
cppu_detail_getUnoType(::cppu::UnoVoidType const *) {
    return ::cppu::detail::getTypeFromTypeClass(::typelib_TypeClass_VOID);
}

inline ::com::sun::star::uno::Type const &
cppu_detail_getUnoType(bool const *) {
    return ::cppu::detail::getTypeFromTypeClass(::typelib_TypeClass_BOOLEAN);
}

inline ::com::sun::star::uno::Type const &
cppu_detail_getUnoType(::sal_Bool const *) {
    return ::cppu::detail::getTypeFromTypeClass(::typelib_TypeClass_BOOLEAN);
}

inline ::com::sun::star::uno::Type const &
cppu_detail_getUnoType(::sal_Int8 const *) {
    return ::cppu::detail::getTypeFromTypeClass(::typelib_TypeClass_BYTE);
}

inline ::com::sun::star::uno::Type const &
cppu_detail_getUnoType(::sal_Int16 const *) {
    return ::cppu::detail::getTypeFromTypeClass(::typelib_TypeClass_SHORT);
}

inline ::com::sun::star::uno::Type const &
cppu_detail_getUnoType(::cppu::UnoUnsignedShortType const *) {
    return ::cppu::detail::getTypeFromTypeClass(
        ::typelib_TypeClass_UNSIGNED_SHORT);
}

inline ::com::sun::star::uno::Type const &
cppu_detail_getUnoType(::sal_Int32 const *) {
    return ::cppu::detail::getTypeFromTypeClass(::typelib_TypeClass_LONG);
}

inline ::com::sun::star::uno::Type const &
cppu_detail_getUnoType(::sal_uInt32 const *) {
    return ::cppu::detail::getTypeFromTypeClass(
        ::typelib_TypeClass_UNSIGNED_LONG);
}

inline ::com::sun::star::uno::Type const &
cppu_detail_getUnoType(::sal_Int64 const *) {
    return ::cppu::detail::getTypeFromTypeClass(::typelib_TypeClass_HYPER);
}

inline ::com::sun::star::uno::Type const &
cppu_detail_getUnoType(::sal_uInt64 const *) {
    return ::cppu::detail::getTypeFromTypeClass(
        ::typelib_TypeClass_UNSIGNED_HYPER);
}

inline ::com::sun::star::uno::Type const &
cppu_detail_getUnoType(float const *) {
    return ::cppu::detail::getTypeFromTypeClass(::typelib_TypeClass_FLOAT);
}

inline ::com::sun::star::uno::Type const &
cppu_detail_getUnoType(double const *) {
    return ::cppu::detail::getTypeFromTypeClass(::typelib_TypeClass_DOUBLE);
}

inline ::com::sun::star::uno::Type const &
cppu_detail_getUnoType(::cppu::UnoCharType const *) {
    return ::cppu::detail::getTypeFromTypeClass(::typelib_TypeClass_CHAR);
}

inline ::com::sun::star::uno::Type const &
cppu_detail_getUnoType(::rtl::OUString const *) {
    return ::cppu::detail::getTypeFromTypeClass(::typelib_TypeClass_STRING);
}

inline ::com::sun::star::uno::Type const &
cppu_detail_getUnoType(::com::sun::star::uno::Type const *) {
    return ::cppu::detail::getTypeFromTypeClass(::typelib_TypeClass_TYPE);
}

inline ::com::sun::star::uno::Type const &
cppu_detail_getUnoType(::com::sun::star::uno::Any const *) {
    return ::cppu::detail::getTypeFromTypeClass(::typelib_TypeClass_ANY);
}

template< typename T > inline ::com::sun::star::uno::Type const &
cppu_detail_getUnoType(::cppu::UnoSequenceType< T > const *) {
    //TODO: depending on memory model, the following might not work reliably
    static typelib_TypeDescriptionReference * p = 0;
    if (p == 0) {
        ::typelib_static_sequence_type_init(
            &p, ::cppu::UnoType< T >::get().getTypeLibType());
    }
    return ::cppu::detail::getTypeFromTypeDescriptionReference(&p);
}

template< typename T > inline ::com::sun::star::uno::Type const &
cppu_detail_getUnoType(::com::sun::star::uno::Sequence< T > const *) {
    return cppu_detail_getUnoType(
        static_cast< ::cppu::UnoSequenceType< T > * >(0));
}

inline ::com::sun::star::uno::Type const &
cppu_detail_getUnoType(::com::sun::star::uno::Exception const *) {
    return ::cppu::detail::getTypeFromTypeClass(::typelib_TypeClass_EXCEPTION);
}

inline ::com::sun::star::uno::Type const &
cppu_detail_getUnoType(::com::sun::star::uno::XInterface const *) {
    return ::cppu::detail::getTypeFromTypeClass(::typelib_TypeClass_INTERFACE);
}

template< typename T > inline ::com::sun::star::uno::Type const &
cppu_detail_getUnoType(::com::sun::star::uno::Reference< T > const *) {
    return ::cppu::UnoType< T >::get();
}

#if !defined _MSC_VER || _MSC_VER > 1310
} }
#endif

namespace cppu {

/**
   Get the com::sun::star::uno::Type instance representing a certain UNO type.

   For each C++ type representing a UNO type, the corresponding instantiation of
   this template has a public static member function get().  (The template is
   specialized for C++ templates representing polymorphic struct type templates
   of UNO.  In those cases, it does not work to instantiate UnoType with a C++
   type that is derived from a C++ type that represents a UNO type, but does not
   itself represent a UNO type.  In all other cases, UnoType even works for such
   C++ types that are unambiguously derived from one C++ type that represents a
   UNO type.)  In addition to those C++ types that are mappings of UNO types
   (except for sal_uInt16 and sal_Unicode, see below), the following C++ types
   are appropriate as template arguments: cppu::UnoVoidType, bool,
   cppu::UnoUnsignedShortType, cppu::UnoCharType, cppu::UnoSequenceType with any
   appropriate template argument (the latter three to unambiguously specify UNO
   types, as the UNO types UNSIGNED SHORT and CHAR map to the same C++ type),
   and com::sun::star::uno::Reference with any appropriate template argument.

   @since UDK 3.2.2
*/
template< typename T > class UnoType {
public:
    static inline ::com::sun::star::uno::Type const & get() {
        using namespace ::cppu::detail;
        return cppu_detail_getUnoType(static_cast< T * >(0));
    }

private:
    UnoType(UnoType &); // not defined
    ~UnoType(); // not defined
    void operator =(UnoType &); // not defined
};

/**
   A working replacement for getCppuType (see there).

   There are three overloads of this function that together form the replacement
   of getCppuType.  The replacement has exactly the same semantics as
   getCppuType, in that it returns correct results for the UNO type UNSIGNED
   SHORT but not for the UNO type CHAR.

   @since UDK 3.2.2
*/
template< typename T > inline ::com::sun::star::uno::Type const &
getTypeFavourUnsigned(T const *) {
    return ::cppu::UnoType< T >::get();
}

/**
   A working replacement for getCppuType (see there).

   There are three overloads of this function that together form the replacement
   of getCppuType.  The replacement has exactly the same semantics as
   getCppuType, in that it returns correct results for the UNO type UNSIGNED
   SHORT but not for the UNO type CHAR.

   @since UDK 3.2.2
*/
inline ::com::sun::star::uno::Type const &
getTypeFavourUnsigned(::sal_uInt16 const *) {
    return ::cppu::UnoType< ::cppu::UnoUnsignedShortType >::get();
}

/**
   A working replacement for getCppuType (see there).

   There are three overloads of this function that together form the replacement
   of getCppuType.  The replacement has exactly the same semantics as
   getCppuType, in that it returns correct results for the UNO type UNSIGNED
   SHORT but not for the UNO type CHAR.

   @since UDK 3.2.2
*/
template< typename T > inline ::com::sun::star::uno::Type const &
getTypeFavourUnsigned(::com::sun::star::uno::Sequence< T > const *);
    // defined in com/sun/star/uno/Sequence.hxx

/**
   A working replacement for getCppuType (see there).

   There are three overloads of this function that together form the replacement
   of the getCppuType template.  The replacement has exactly the same semantics
   as the getCppuType template, in that it returns correct results for the UNO
   type CHAR but not for the UNO type UNSIGNED SHORT.  Additionally, it also
   returns the intended results for sequence types.

   @internal

   @since UDK 3.2.3
*/
template< typename T > inline ::com::sun::star::uno::Type const &
getTypeFavourChar(T const *) {
    return ::cppu::UnoType< T >::get();
}

/**
   A working replacement for getCppuType (see there).

   There are three overloads of this function that together form the replacement
   of the getCppuType template.  The replacement has exactly the same semantics
   as the getCppuType template, in that it returns correct results for the UNO
   type CHAR but not for the UNO type UNSIGNED SHORT.  Additionally, it also
   returns the intended results for sequence types.

   @internal

   @since UDK 3.2.3
*/
inline ::com::sun::star::uno::Type const &
getTypeFavourChar(::sal_Unicode const *) {
    return ::cppu::UnoType< ::cppu::UnoCharType >::get();
}

/**
   A working replacement for getCppuType (see there).

   There are three overloads of this function that together form the replacement
   of the getCppuType template.  The replacement has exactly the same semantics
   as the getCppuType template, in that it returns correct results for the UNO
   type CHAR but not for the UNO type UNSIGNED SHORT.  Additionally, it also
   returns the intended results for sequence types.

   @internal

   @since UDK 3.2.3
*/
template< typename T > inline ::com::sun::star::uno::Type const &
getTypeFavourChar(::com::sun::star::uno::Sequence< T > const *);
    // defined in com/sun/star/uno/Sequence.hxx

}

#endif
