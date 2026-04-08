#pragma once

#include "sal/config.h"

#include "test/codemaker/codemakertests/PolyStruct.hdl"

#include "com/sun/star/uno/Type.hxx"
#include "cppu/unotype.hxx"
#include "rtl/strbuf.hxx"
#include "rtl/textenc.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "typelib/typeclass.h"
#include "typelib/typedescription.h"

namespace test { namespace codemaker { namespace codemakertests {

template< typename typeparam_if, typename typeparam_else > inline PolyStruct< typeparam_if, typeparam_else >::PolyStruct()
    : member1()
    , member2(0)
{
}

template< typename typeparam_if, typename typeparam_else > inline PolyStruct< typeparam_if, typeparam_else >::PolyStruct(typeparam_if const & member1_, const ::sal_Int32& member2_)
    : member1(member1_)
    , member2(member2_)
{
}

template< typename typeparam_if, typename typeparam_else > 
inline PolyStruct< typeparam_if, typeparam_else >
make_PolyStruct(typeparam_if const & member1_, const ::sal_Int32& member2_)
{
    return PolyStruct< typeparam_if, typeparam_else >(member1_, member2_);
}

template< typename typeparam_if, typename typeparam_else >  inline bool operator==(const PolyStruct< typeparam_if, typeparam_else >& the_lhs, const PolyStruct< typeparam_if, typeparam_else >& the_rhs)
{
    return the_lhs.member1 == the_rhs.member1
        && the_lhs.member2 == the_rhs.member2;
}
template< typename typeparam_if, typename typeparam_else >  inline bool operator!=(const PolyStruct< typeparam_if, typeparam_else >& the_lhs, const PolyStruct< typeparam_if, typeparam_else >& the_rhs)
{
return !operator==(the_lhs, the_rhs);
}
} } }

namespace cppu {

template< typename typeparam_if, typename typeparam_else > class UnoType< ::test::codemaker::codemakertests::PolyStruct< typeparam_if, typeparam_else > > {
public:
    static inline ::css::uno::Type const & get() {
        //TODO: On certain platforms with weak memory models, the following code can result in some threads observing that the_type points to garbage
        static ::typelib_TypeDescriptionReference * the_type = 0;
        if (the_type == 0) {
            ::rtl::OStringBuffer the_buffer("test.codemaker.codemakertests.PolyStruct<");
            the_buffer.append(::rtl::OUStringToOString(::cppu::getTypeFavourChar(static_cast< typeparam_if * >(0)).getTypeName(), RTL_TEXTENCODING_UTF8));
            the_buffer.append(',');
            the_buffer.append(::rtl::OUStringToOString(::cppu::getTypeFavourChar(static_cast< typeparam_else * >(0)).getTypeName(), RTL_TEXTENCODING_UTF8));
            the_buffer.append('>');
            ::typelib_TypeDescriptionReference * the_members[] = {
                ::cppu::getTypeFavourChar(static_cast< typeparam_if * >(0)).getTypeLibType(),
                ::cppu::UnoType< ::sal_Int32 >::get().getTypeLibType() };
            static ::sal_Bool const the_parameterizedTypes[] = { true, false };
            ::typelib_static_struct_type_init(&the_type, the_buffer.getStr(), 0, 2, the_members, the_parameterizedTypes);
        }
        return *reinterpret_cast< ::css::uno::Type * >(&the_type);
    }

private:
    UnoType(UnoType &); // not defined
    ~UnoType(); // not defined
    void operator =(UnoType); // not defined
};

}

template< typename typeparam_if, typename typeparam_else > SAL_DEPRECATED("use cppu::UnoType") inline ::css::uno::Type const & SAL_CALL getCppuType(SAL_UNUSED_PARAMETER ::test::codemaker::codemakertests::PolyStruct< typeparam_if, typeparam_else > const *) {
        return ::cppu::UnoType< ::test::codemaker::codemakertests::PolyStruct< typeparam_if, typeparam_else > >::get();
    }
