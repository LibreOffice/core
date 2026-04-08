#pragma once

#include "sal/config.h"

#include "test/codemaker/codemakertests/XIfc.hdl"

#include "com/sun/star/uno/XInterface.hpp"
#include "test/codemaker/codemakertests/PolyStruct.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Type.hxx"
#include "cppu/unotype.hxx"
#include "sal/types.h"

#if defined LIBO_INTERNAL_ONLY
#include <type_traits>
#endif

namespace test { namespace codemaker { namespace codemakertests {

inline ::css::uno::Type const & cppu_detail_getUnoType(SAL_UNUSED_PARAMETER ::test::codemaker::codemakertests::XIfc const *) {
    static typelib_TypeDescriptionReference * the_type = 0;
    if ( !the_type )
    {
        typelib_static_mi_interface_type_init( &the_type, "test.codemaker.codemakertests.XIfc", 0, 0 );
    }
    return * reinterpret_cast< ::css::uno::Type * >( &the_type );
}

} } }

SAL_DEPRECATED("use cppu::UnoType") inline ::css::uno::Type const & SAL_CALL getCppuType(SAL_UNUSED_PARAMETER ::css::uno::Reference< ::test::codemaker::codemakertests::XIfc > const *) {
    return ::cppu::UnoType< ::css::uno::Reference< ::test::codemaker::codemakertests::XIfc > >::get();
}

::css::uno::Type const & ::test::codemaker::codemakertests::XIfc::static_type(SAL_UNUSED_PARAMETER void *) {
    return ::cppu::UnoType< ::test::codemaker::codemakertests::XIfc >::get();
}

#if defined LIBO_INTERNAL_ONLY
namespace cppu::detail {
template<> struct IsUnoInterfaceType<::test::codemaker::codemakertests::XIfc>: ::std::true_type {};
}
#endif

