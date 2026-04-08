#pragma once

#include "sal/config.h"

#include "test/codemaker/codemakertests/exceptions/SimpleError.hdl"

#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Type.hxx"
#include "cppu/unotype.hxx"

namespace test { namespace codemaker { namespace codemakertests { namespace exceptions {

inline SimpleError::SimpleError(
#if defined LIBO_USE_SOURCE_LOCATION
    o3tl::source_location location
#endif
)
    : ::css::uno::Exception(
#if defined LIBO_USE_SOURCE_LOCATION
    location
#endif
)
{ }

inline SimpleError::SimpleError(const ::rtl::OUString& Message_, const ::css::uno::Reference< ::css::uno::XInterface >& Context_
#if defined LIBO_USE_SOURCE_LOCATION
    , o3tl::source_location location
#endif
)
    : ::css::uno::Exception(Message_, Context_
#if defined LIBO_USE_SOURCE_LOCATION
    , location
#endif
)
{ }

#if !defined LIBO_INTERNAL_ONLY
SimpleError::SimpleError(SimpleError const & the_other): ::css::uno::Exception(the_other) {}

SimpleError::~SimpleError() {}

SimpleError & SimpleError::operator =(SimpleError const & the_other) {
    //TODO: Just like its implicitly-defined counterpart, this function definition is not exception-safe
    ::css::uno::Exception::operator =(the_other);
    return *this;
}
#endif

} } } }

namespace test { namespace codemaker { namespace codemakertests { namespace exceptions {

inline ::css::uno::Type const & cppu_detail_getUnoType(SAL_UNUSED_PARAMETER ::test::codemaker::codemakertests::exceptions::SimpleError const *) {
    static typelib_TypeDescriptionReference * the_type = 0;
    if ( !the_type )
    {
        typelib_static_compound_type_init( &the_type, typelib_TypeClass_EXCEPTION, "test.codemaker.codemakertests.exceptions.SimpleError", * ::typelib_static_type_getByTypeClass( typelib_TypeClass_EXCEPTION ), 0,  0 );
    }
    return * reinterpret_cast< const ::css::uno::Type * >( &the_type );
}

} } } }

SAL_DEPRECATED("use cppu::UnoType") inline ::css::uno::Type const & SAL_CALL getCppuType(SAL_UNUSED_PARAMETER ::test::codemaker::codemakertests::exceptions::SimpleError const *) {
    return ::cppu::UnoType< ::test::codemaker::codemakertests::exceptions::SimpleError >::get();
}
