#pragma once

#include "sal/config.h"

#include "test/codemaker/codemakertests/Struct1.hdl"

#include "com/sun/star/uno/Type.hxx"
#include "cppu/unotype.hxx"
#include "sal/types.h"
#include "typelib/typeclass.h"
#include "typelib/typedescription.h"

namespace test { namespace codemaker { namespace codemakertests {

inline Struct1::Struct1()
    : member1(0)
{
}

inline Struct1::Struct1(const ::sal_Int32& member1_)
    : member1(member1_)
{
}


inline bool operator==(const Struct1& the_lhs, const Struct1& the_rhs)
{
    return the_lhs.member1 == the_rhs.member1;
}

inline bool operator!=(const Struct1& the_lhs, const Struct1& the_rhs)
{
return !operator==(the_lhs, the_rhs);
}
} } }

namespace test { namespace codemaker { namespace codemakertests {

inline ::css::uno::Type const & cppu_detail_getUnoType(SAL_UNUSED_PARAMETER ::test::codemaker::codemakertests::Struct1 const *) {
    //TODO: On certain platforms with weak memory models, the following code can result in some threads observing that the_type points to garbage
    static ::typelib_TypeDescriptionReference * the_type = 0;
    if (the_type == 0) {
        ::typelib_TypeDescriptionReference * the_members[] = {
            ::cppu::UnoType< ::sal_Int32 >::get().getTypeLibType() };
        ::typelib_static_struct_type_init(&the_type, "test.codemaker.codemakertests.Struct1", 0, 1, the_members, 0);
    }
    return *reinterpret_cast< ::css::uno::Type * >(&the_type);
}

} } }

SAL_DEPRECATED("use cppu::UnoType") inline ::css::uno::Type const & SAL_CALL getCppuType(SAL_UNUSED_PARAMETER ::test::codemaker::codemakertests::Struct1 const *) {
    return ::cppu::UnoType< ::test::codemaker::codemakertests::Struct1 >::get();
}
