#pragma once

#include "sal/config.h"

#include "test/codemaker/codemakertests/Enum4.hdl"

#include "com/sun/star/uno/Type.hxx"
#include "cppu/unotype.hxx"

namespace test { namespace codemaker { namespace codemakertests {

inline ::css::uno::Type const & cppu_detail_getUnoType(SAL_UNUSED_PARAMETER ::test::codemaker::codemakertests::Enum4 const *) {
    static typelib_TypeDescriptionReference * the_type = 0;
    if ( !the_type )
    {
        typelib_static_enum_type_init( &the_type,
                                       "test.codemaker.codemakertests.Enum4",
                                       ::test::codemaker::codemakertests::Enum4_AUTO_A );
    }
    return * reinterpret_cast< ::css::uno::Type * >( &the_type );
}

} } }

SAL_DEPRECATED("use cppu::UnoType") inline ::css::uno::Type const & SAL_CALL getCppuType(SAL_UNUSED_PARAMETER ::test::codemaker::codemakertests::Enum4 const *) {
    return ::cppu::UnoType< ::test::codemaker::codemakertests::Enum4 >::get();
}
