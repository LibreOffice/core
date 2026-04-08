#pragma once

#include "sal/config.h"

#include "test/codemaker/codemakertests/exceptions/MyBaseError.hdl"

#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Type.hxx"
#include "cppu/unotype.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

namespace test { namespace codemaker { namespace codemakertests { namespace exceptions {

inline MyBaseError::MyBaseError(
#if defined LIBO_USE_SOURCE_LOCATION
    o3tl::source_location location
#endif
)
    : ::css::uno::Exception(
#if defined LIBO_USE_SOURCE_LOCATION
    location
#endif
)
    , ErrorCode(0)
    , Details()
{ }

inline MyBaseError::MyBaseError(const ::rtl::OUString& Message_, const ::css::uno::Reference< ::css::uno::XInterface >& Context_, const ::sal_Int32& ErrorCode_, const ::rtl::OUString& Details_
#if defined LIBO_USE_SOURCE_LOCATION
    , o3tl::source_location location
#endif
)
    : ::css::uno::Exception(Message_, Context_
#if defined LIBO_USE_SOURCE_LOCATION
    , location
#endif
)
    , ErrorCode(ErrorCode_)
    , Details(Details_)
{ }

#if !defined LIBO_INTERNAL_ONLY
MyBaseError::MyBaseError(MyBaseError const & the_other): ::css::uno::Exception(the_other), ErrorCode(the_other.ErrorCode), Details(the_other.Details) {}

MyBaseError::~MyBaseError() {}

MyBaseError & MyBaseError::operator =(MyBaseError const & the_other) {
    //TODO: Just like its implicitly-defined counterpart, this function definition is not exception-safe
    ::css::uno::Exception::operator =(the_other);
    ErrorCode = the_other.ErrorCode;
    Details = the_other.Details;
    return *this;
}
#endif

} } } }

namespace test { namespace codemaker { namespace codemakertests { namespace exceptions {

inline ::css::uno::Type const & cppu_detail_getUnoType(SAL_UNUSED_PARAMETER ::test::codemaker::codemakertests::exceptions::MyBaseError const *) {
    static typelib_TypeDescriptionReference * the_type = 0;
    if ( !the_type )
    {
        typelib_TypeDescriptionReference * aMemberRefs[2];
        const ::css::uno::Type& rMemberType_long = ::cppu::UnoType< ::sal_Int32 >::get();
        aMemberRefs[0] = rMemberType_long.getTypeLibType();
        const ::css::uno::Type& rMemberType_string = ::cppu::UnoType< ::rtl::OUString >::get();
        aMemberRefs[1] = rMemberType_string.getTypeLibType();

        typelib_static_compound_type_init( &the_type, typelib_TypeClass_EXCEPTION, "test.codemaker.codemakertests.exceptions.MyBaseError", * ::typelib_static_type_getByTypeClass( typelib_TypeClass_EXCEPTION ), 2,  aMemberRefs );
    }
    return * reinterpret_cast< const ::css::uno::Type * >( &the_type );
}

} } } }

SAL_DEPRECATED("use cppu::UnoType") inline ::css::uno::Type const & SAL_CALL getCppuType(SAL_UNUSED_PARAMETER ::test::codemaker::codemakertests::exceptions::MyBaseError const *) {
    return ::cppu::UnoType< ::test::codemaker::codemakertests::exceptions::MyBaseError >::get();
}
