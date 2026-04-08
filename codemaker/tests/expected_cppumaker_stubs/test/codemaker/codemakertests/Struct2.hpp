#pragma once

#include "sal/config.h"

#include "test/codemaker/codemakertests/Struct2.hdl"

#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/uno/XNamingService.hpp"
#include "test/codemaker/codemakertests/Any.hpp"
#include "test/codemaker/codemakertests/Boolean.hpp"
#include "test/codemaker/codemakertests/Byte.hpp"
#include "test/codemaker/codemakertests/Char.hpp"
#include "test/codemaker/codemakertests/Double.hpp"
#include "test/codemaker/codemakertests/Enum.hpp"
#include "test/codemaker/codemakertests/Enum2.hpp"
#include "test/codemaker/codemakertests/Float.hpp"
#include "test/codemaker/codemakertests/Hyper.hpp"
#include "test/codemaker/codemakertests/Long.hpp"
#include "test/codemaker/codemakertests/SequenceAny.hpp"
#include "test/codemaker/codemakertests/SequenceBoolean.hpp"
#include "test/codemaker/codemakertests/SequenceByte.hpp"
#include "test/codemaker/codemakertests/SequenceChar.hpp"
#include "test/codemaker/codemakertests/SequenceDouble.hpp"
#include "test/codemaker/codemakertests/SequenceEnum.hpp"
#include "test/codemaker/codemakertests/SequenceFloat.hpp"
#include "test/codemaker/codemakertests/SequenceHyper.hpp"
#include "test/codemaker/codemakertests/SequenceLong.hpp"
#include "test/codemaker/codemakertests/SequenceShort.hpp"
#include "test/codemaker/codemakertests/SequenceString.hpp"
#include "test/codemaker/codemakertests/SequenceStruct.hpp"
#include "test/codemaker/codemakertests/SequenceType.hpp"
#include "test/codemaker/codemakertests/SequenceUnsignedHyper.hpp"
#include "test/codemaker/codemakertests/SequenceUnsignedLong.hpp"
#include "test/codemaker/codemakertests/SequenceUnsignedShort.hpp"
#include "test/codemaker/codemakertests/SequenceXInterface.hpp"
#include "test/codemaker/codemakertests/SequenceXNamingService.hpp"
#include "test/codemaker/codemakertests/Short.hpp"
#include "test/codemaker/codemakertests/String.hpp"
#include "test/codemaker/codemakertests/Struct.hpp"
#include "test/codemaker/codemakertests/Struct1.hpp"
#include "test/codemaker/codemakertests/Type.hpp"
#include "test/codemaker/codemakertests/UnsignedHyper.hpp"
#include "test/codemaker/codemakertests/UnsignedLong.hpp"
#include "test/codemaker/codemakertests/UnsignedShort.hpp"
#include "test/codemaker/codemakertests/XInterface.hpp"
#include "test/codemaker/codemakertests/XNamingService.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/Type.hxx"
#include "cppu/unotype.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "typelib/typeclass.h"
#include "typelib/typedescription.h"

namespace test { namespace codemaker { namespace codemakertests {

inline Struct2::Struct2()
    : p1(false)
    , p2(0)
    , p3(0)
    , p4(0)
    , p5(0)
    , p6(0)
    , p7(0)
    , p8(0)
    , p9(0)
    , p10(0)
    , p11(0)
    , p12()
    , p13()
    , p14()
    , p15(::test::codemaker::codemakertests::Enum2_FLAG_NONE)
    , p16()
    , p17()
    , p18()
    , t1(false)
    , t2(0)
    , t3(0)
    , t4(0)
    , t5(0)
    , t6(0)
    , t7(0)
    , t8(0)
    , t9(0)
    , t10(0)
    , t11(0)
    , t12()
    , t13()
    , t14()
    , t15(::test::codemaker::codemakertests::Enum2_FLAG_NONE)
    , t16()
    , t17()
    , t18()
    , a1()
    , a2()
    , a3()
    , a4()
    , a5()
    , a6()
    , a7()
    , a8()
    , a9()
    , a10()
    , a11()
    , a12()
    , a13()
    , a14()
    , a15()
    , a16()
    , a17()
    , a18()
    , aa1()
    , aa2()
    , aa3()
    , aa4()
    , aa5()
    , aa6()
    , aa7()
    , aa8()
    , aa9()
    , aa10()
    , aa11()
    , aa12()
    , aa13()
    , aa14()
    , aa15()
    , aa16()
    , aa17()
    , aa18()
    , at1()
    , at2()
    , at3()
    , at4()
    , at5()
    , at6()
    , at7()
    , at8()
    , at9()
    , at10()
    , at11()
    , at12()
    , at13()
    , at14()
    , at15()
    , at16()
    , at17()
    , at18()
{
}

inline Struct2::Struct2(const ::sal_Bool& p1_, const ::sal_Int8& p2_, const ::sal_Int16& p3_, const ::sal_uInt16& p4_, const ::sal_Int32& p5_, const ::sal_uInt32& p6_, const ::sal_Int64& p7_, const ::sal_uInt64& p8_, const float& p9_, const double& p10_, const ::sal_Unicode& p11_, const ::rtl::OUString& p12_, const ::css::uno::Type& p13_, const ::css::uno::Any& p14_, const ::test::codemaker::codemakertests::Enum2& p15_, const ::test::codemaker::codemakertests::Struct1& p16_, const ::css::uno::Reference< ::css::uno::XInterface >& p17_, const ::css::uno::Reference< ::css::uno::XNamingService >& p18_, const ::sal_Bool& t1_, const ::sal_Int8& t2_, const ::sal_Int16& t3_, const ::sal_uInt16& t4_, const ::sal_Int32& t5_, const ::sal_uInt32& t6_, const ::sal_Int64& t7_, const ::sal_uInt64& t8_, const float& t9_, const double& t10_, const ::sal_Unicode& t11_, const ::rtl::OUString& t12_, const ::css::uno::Type& t13_, const ::css::uno::Any& t14_, const ::test::codemaker::codemakertests::Enum2& t15_, const ::test::codemaker::codemakertests::Struct1& t16_, const ::css::uno::Reference< ::css::uno::XInterface >& t17_, const ::css::uno::Reference< ::css::uno::XNamingService >& t18_, const ::css::uno::Sequence< ::sal_Bool >& a1_, const ::css::uno::Sequence< ::sal_Int8 >& a2_, const ::css::uno::Sequence< ::sal_Int16 >& a3_, const ::css::uno::Sequence< ::sal_uInt16 >& a4_, const ::css::uno::Sequence< ::sal_Int32 >& a5_, const ::css::uno::Sequence< ::sal_uInt32 >& a6_, const ::css::uno::Sequence< ::sal_Int64 >& a7_, const ::css::uno::Sequence< ::sal_uInt64 >& a8_, const ::css::uno::Sequence< float >& a9_, const ::css::uno::Sequence< double >& a10_, const ::css::uno::Sequence< ::sal_Unicode >& a11_, const ::css::uno::Sequence< ::rtl::OUString >& a12_, const ::css::uno::Sequence< ::css::uno::Type >& a13_, const ::css::uno::Sequence< ::css::uno::Any >& a14_, const ::css::uno::Sequence< ::test::codemaker::codemakertests::Enum2 >& a15_, const ::css::uno::Sequence< ::test::codemaker::codemakertests::Struct1 >& a16_, const ::css::uno::Sequence< ::css::uno::Reference< ::css::uno::XInterface > >& a17_, const ::css::uno::Sequence< ::css::uno::Reference< ::css::uno::XNamingService > >& a18_, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_Bool > >& aa1_, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_Int8 > >& aa2_, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_Int16 > >& aa3_, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_uInt16 > >& aa4_, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_Int32 > >& aa5_, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_uInt32 > >& aa6_, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_Int64 > >& aa7_, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_uInt64 > >& aa8_, const ::css::uno::Sequence< ::css::uno::Sequence< float > >& aa9_, const ::css::uno::Sequence< ::css::uno::Sequence< double > >& aa10_, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_Unicode > >& aa11_, const ::css::uno::Sequence< ::css::uno::Sequence< ::rtl::OUString > >& aa12_, const ::css::uno::Sequence< ::css::uno::Sequence< ::css::uno::Type > >& aa13_, const ::css::uno::Sequence< ::css::uno::Sequence< ::css::uno::Any > >& aa14_, const ::css::uno::Sequence< ::css::uno::Sequence< ::test::codemaker::codemakertests::Enum2 > >& aa15_, const ::css::uno::Sequence< ::css::uno::Sequence< ::test::codemaker::codemakertests::Struct1 > >& aa16_, const ::css::uno::Sequence< ::css::uno::Sequence< ::css::uno::Reference< ::css::uno::XInterface > > >& aa17_, const ::css::uno::Sequence< ::css::uno::Sequence< ::css::uno::Reference< ::css::uno::XNamingService > > >& aa18_, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_Bool > >& at1_, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_Int8 > >& at2_, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_Int16 > >& at3_, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_uInt16 > >& at4_, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_Int32 > >& at5_, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_uInt32 > >& at6_, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_Int64 > >& at7_, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_uInt64 > >& at8_, const ::css::uno::Sequence< ::css::uno::Sequence< float > >& at9_, const ::css::uno::Sequence< ::css::uno::Sequence< double > >& at10_, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_Unicode > >& at11_, const ::css::uno::Sequence< ::css::uno::Sequence< ::rtl::OUString > >& at12_, const ::css::uno::Sequence< ::css::uno::Sequence< ::css::uno::Type > >& at13_, const ::css::uno::Sequence< ::css::uno::Sequence< ::css::uno::Any > >& at14_, const ::css::uno::Sequence< ::css::uno::Sequence< ::test::codemaker::codemakertests::Enum2 > >& at15_, const ::css::uno::Sequence< ::css::uno::Sequence< ::test::codemaker::codemakertests::Struct1 > >& at16_, const ::css::uno::Sequence< ::css::uno::Sequence< ::css::uno::Reference< ::css::uno::XInterface > > >& at17_, const ::css::uno::Sequence< ::css::uno::Sequence< ::css::uno::Reference< ::css::uno::XNamingService > > >& at18_)
    : p1(p1_)
    , p2(p2_)
    , p3(p3_)
    , p4(p4_)
    , p5(p5_)
    , p6(p6_)
    , p7(p7_)
    , p8(p8_)
    , p9(p9_)
    , p10(p10_)
    , p11(p11_)
    , p12(p12_)
    , p13(p13_)
    , p14(p14_)
    , p15(p15_)
    , p16(p16_)
    , p17(p17_)
    , p18(p18_)
    , t1(t1_)
    , t2(t2_)
    , t3(t3_)
    , t4(t4_)
    , t5(t5_)
    , t6(t6_)
    , t7(t7_)
    , t8(t8_)
    , t9(t9_)
    , t10(t10_)
    , t11(t11_)
    , t12(t12_)
    , t13(t13_)
    , t14(t14_)
    , t15(t15_)
    , t16(t16_)
    , t17(t17_)
    , t18(t18_)
    , a1(a1_)
    , a2(a2_)
    , a3(a3_)
    , a4(a4_)
    , a5(a5_)
    , a6(a6_)
    , a7(a7_)
    , a8(a8_)
    , a9(a9_)
    , a10(a10_)
    , a11(a11_)
    , a12(a12_)
    , a13(a13_)
    , a14(a14_)
    , a15(a15_)
    , a16(a16_)
    , a17(a17_)
    , a18(a18_)
    , aa1(aa1_)
    , aa2(aa2_)
    , aa3(aa3_)
    , aa4(aa4_)
    , aa5(aa5_)
    , aa6(aa6_)
    , aa7(aa7_)
    , aa8(aa8_)
    , aa9(aa9_)
    , aa10(aa10_)
    , aa11(aa11_)
    , aa12(aa12_)
    , aa13(aa13_)
    , aa14(aa14_)
    , aa15(aa15_)
    , aa16(aa16_)
    , aa17(aa17_)
    , aa18(aa18_)
    , at1(at1_)
    , at2(at2_)
    , at3(at3_)
    , at4(at4_)
    , at5(at5_)
    , at6(at6_)
    , at7(at7_)
    , at8(at8_)
    , at9(at9_)
    , at10(at10_)
    , at11(at11_)
    , at12(at12_)
    , at13(at13_)
    , at14(at14_)
    , at15(at15_)
    , at16(at16_)
    , at17(at17_)
    , at18(at18_)
{
}


inline bool operator==(const Struct2& the_lhs, const Struct2& the_rhs)
{
    return the_lhs.p1 == the_rhs.p1
        && the_lhs.p2 == the_rhs.p2
        && the_lhs.p3 == the_rhs.p3
        && the_lhs.p4 == the_rhs.p4
        && the_lhs.p5 == the_rhs.p5
        && the_lhs.p6 == the_rhs.p6
        && the_lhs.p7 == the_rhs.p7
        && the_lhs.p8 == the_rhs.p8
        && the_lhs.p9 == the_rhs.p9
        && the_lhs.p10 == the_rhs.p10
        && the_lhs.p11 == the_rhs.p11
        && the_lhs.p12 == the_rhs.p12
        && the_lhs.p13 == the_rhs.p13
        && the_lhs.p14 == the_rhs.p14
        && the_lhs.p15 == the_rhs.p15
        && the_lhs.p16 == the_rhs.p16
        && the_lhs.p17 == the_rhs.p17
        && the_lhs.p18 == the_rhs.p18
        && the_lhs.t1 == the_rhs.t1
        && the_lhs.t2 == the_rhs.t2
        && the_lhs.t3 == the_rhs.t3
        && the_lhs.t4 == the_rhs.t4
        && the_lhs.t5 == the_rhs.t5
        && the_lhs.t6 == the_rhs.t6
        && the_lhs.t7 == the_rhs.t7
        && the_lhs.t8 == the_rhs.t8
        && the_lhs.t9 == the_rhs.t9
        && the_lhs.t10 == the_rhs.t10
        && the_lhs.t11 == the_rhs.t11
        && the_lhs.t12 == the_rhs.t12
        && the_lhs.t13 == the_rhs.t13
        && the_lhs.t14 == the_rhs.t14
        && the_lhs.t15 == the_rhs.t15
        && the_lhs.t16 == the_rhs.t16
        && the_lhs.t17 == the_rhs.t17
        && the_lhs.t18 == the_rhs.t18
        && the_lhs.a1 == the_rhs.a1
        && the_lhs.a2 == the_rhs.a2
        && the_lhs.a3 == the_rhs.a3
        && the_lhs.a4 == the_rhs.a4
        && the_lhs.a5 == the_rhs.a5
        && the_lhs.a6 == the_rhs.a6
        && the_lhs.a7 == the_rhs.a7
        && the_lhs.a8 == the_rhs.a8
        && the_lhs.a9 == the_rhs.a9
        && the_lhs.a10 == the_rhs.a10
        && the_lhs.a11 == the_rhs.a11
        && the_lhs.a12 == the_rhs.a12
        && the_lhs.a13 == the_rhs.a13
        && the_lhs.a14 == the_rhs.a14
        && the_lhs.a15 == the_rhs.a15
        && the_lhs.a16 == the_rhs.a16
        && the_lhs.a17 == the_rhs.a17
        && the_lhs.a18 == the_rhs.a18
        && the_lhs.aa1 == the_rhs.aa1
        && the_lhs.aa2 == the_rhs.aa2
        && the_lhs.aa3 == the_rhs.aa3
        && the_lhs.aa4 == the_rhs.aa4
        && the_lhs.aa5 == the_rhs.aa5
        && the_lhs.aa6 == the_rhs.aa6
        && the_lhs.aa7 == the_rhs.aa7
        && the_lhs.aa8 == the_rhs.aa8
        && the_lhs.aa9 == the_rhs.aa9
        && the_lhs.aa10 == the_rhs.aa10
        && the_lhs.aa11 == the_rhs.aa11
        && the_lhs.aa12 == the_rhs.aa12
        && the_lhs.aa13 == the_rhs.aa13
        && the_lhs.aa14 == the_rhs.aa14
        && the_lhs.aa15 == the_rhs.aa15
        && the_lhs.aa16 == the_rhs.aa16
        && the_lhs.aa17 == the_rhs.aa17
        && the_lhs.aa18 == the_rhs.aa18
        && the_lhs.at1 == the_rhs.at1
        && the_lhs.at2 == the_rhs.at2
        && the_lhs.at3 == the_rhs.at3
        && the_lhs.at4 == the_rhs.at4
        && the_lhs.at5 == the_rhs.at5
        && the_lhs.at6 == the_rhs.at6
        && the_lhs.at7 == the_rhs.at7
        && the_lhs.at8 == the_rhs.at8
        && the_lhs.at9 == the_rhs.at9
        && the_lhs.at10 == the_rhs.at10
        && the_lhs.at11 == the_rhs.at11
        && the_lhs.at12 == the_rhs.at12
        && the_lhs.at13 == the_rhs.at13
        && the_lhs.at14 == the_rhs.at14
        && the_lhs.at15 == the_rhs.at15
        && the_lhs.at16 == the_rhs.at16
        && the_lhs.at17 == the_rhs.at17
        && the_lhs.at18 == the_rhs.at18;
}

inline bool operator!=(const Struct2& the_lhs, const Struct2& the_rhs)
{
return !operator==(the_lhs, the_rhs);
}
} } }

namespace test { namespace codemaker { namespace codemakertests {

inline ::css::uno::Type const & cppu_detail_getUnoType(SAL_UNUSED_PARAMETER ::test::codemaker::codemakertests::Struct2 const *) {
    //TODO: On certain platforms with weak memory models, the following code can result in some threads observing that the_type points to garbage
    static ::typelib_TypeDescriptionReference * the_type = 0;
    if (the_type == 0) {
        ::typelib_TypeDescriptionReference * the_members[] = {
            ::cppu::UnoType< ::sal_Bool >::get().getTypeLibType(),
            ::cppu::UnoType< ::sal_Int8 >::get().getTypeLibType(),
            ::cppu::UnoType< ::sal_Int16 >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoUnsignedShortType >::get().getTypeLibType(),
            ::cppu::UnoType< ::sal_Int32 >::get().getTypeLibType(),
            ::cppu::UnoType< ::sal_uInt32 >::get().getTypeLibType(),
            ::cppu::UnoType< ::sal_Int64 >::get().getTypeLibType(),
            ::cppu::UnoType< ::sal_uInt64 >::get().getTypeLibType(),
            ::cppu::UnoType< float >::get().getTypeLibType(),
            ::cppu::UnoType< double >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoCharType >::get().getTypeLibType(),
            ::cppu::UnoType< ::rtl::OUString >::get().getTypeLibType(),
            ::cppu::UnoType< ::css::uno::Type >::get().getTypeLibType(),
            ::cppu::UnoType< ::css::uno::Any >::get().getTypeLibType(),
            ::cppu::UnoType< ::test::codemaker::codemakertests::Enum2 >::get().getTypeLibType(),
            ::cppu::UnoType< ::test::codemaker::codemakertests::Struct1 >::get().getTypeLibType(),
            ::cppu::UnoType< ::css::uno::Reference< ::css::uno::XInterface > >::get().getTypeLibType(),
            ::cppu::UnoType< ::css::uno::Reference< ::css::uno::XNamingService > >::get().getTypeLibType(),
            ::cppu::UnoType< ::sal_Bool >::get().getTypeLibType(),
            ::cppu::UnoType< ::sal_Int8 >::get().getTypeLibType(),
            ::cppu::UnoType< ::sal_Int16 >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoUnsignedShortType >::get().getTypeLibType(),
            ::cppu::UnoType< ::sal_Int32 >::get().getTypeLibType(),
            ::cppu::UnoType< ::sal_uInt32 >::get().getTypeLibType(),
            ::cppu::UnoType< ::sal_Int64 >::get().getTypeLibType(),
            ::cppu::UnoType< ::sal_uInt64 >::get().getTypeLibType(),
            ::cppu::UnoType< float >::get().getTypeLibType(),
            ::cppu::UnoType< double >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoCharType >::get().getTypeLibType(),
            ::cppu::UnoType< ::rtl::OUString >::get().getTypeLibType(),
            ::cppu::UnoType< ::css::uno::Type >::get().getTypeLibType(),
            ::cppu::UnoType< ::css::uno::Any >::get().getTypeLibType(),
            ::cppu::UnoType< ::test::codemaker::codemakertests::Enum2 >::get().getTypeLibType(),
            ::cppu::UnoType< ::test::codemaker::codemakertests::Struct1 >::get().getTypeLibType(),
            ::cppu::UnoType< ::css::uno::Reference< ::css::uno::XInterface > >::get().getTypeLibType(),
            ::cppu::UnoType< ::css::uno::Reference< ::css::uno::XNamingService > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::sal_Bool > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::sal_Int8 > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::sal_Int16 > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoUnsignedShortType > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::sal_Int32 > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::sal_uInt32 > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::sal_Int64 > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::sal_uInt64 > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< float > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< double > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoCharType > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::rtl::OUString > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::css::uno::Type > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::css::uno::Any > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::test::codemaker::codemakertests::Enum2 > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::test::codemaker::codemakertests::Struct1 > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::css::uno::Reference< ::css::uno::XInterface > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::css::uno::Reference< ::css::uno::XNamingService > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::sal_Bool > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::sal_Int8 > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::sal_Int16 > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::cppu::UnoUnsignedShortType > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::sal_Int32 > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::sal_uInt32 > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::sal_Int64 > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::sal_uInt64 > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< float > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< double > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::cppu::UnoCharType > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::rtl::OUString > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::css::uno::Type > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::css::uno::Any > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::test::codemaker::codemakertests::Enum2 > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::test::codemaker::codemakertests::Struct1 > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::css::uno::Reference< ::css::uno::XInterface > > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::css::uno::Reference< ::css::uno::XNamingService > > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::sal_Bool > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::sal_Int8 > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::sal_Int16 > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::cppu::UnoUnsignedShortType > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::sal_Int32 > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::sal_uInt32 > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::sal_Int64 > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::sal_uInt64 > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< float > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< double > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::cppu::UnoCharType > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::rtl::OUString > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::css::uno::Type > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::css::uno::Any > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::test::codemaker::codemakertests::Enum2 > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::test::codemaker::codemakertests::Struct1 > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::css::uno::Reference< ::css::uno::XInterface > > > >::get().getTypeLibType(),
            ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::css::uno::Reference< ::css::uno::XNamingService > > > >::get().getTypeLibType() };
        ::typelib_static_struct_type_init(&the_type, "test.codemaker.codemakertests.Struct2", 0, 90, the_members, 0);
    }
    return *reinterpret_cast< ::css::uno::Type * >(&the_type);
}

} } }

SAL_DEPRECATED("use cppu::UnoType") inline ::css::uno::Type const & SAL_CALL getCppuType(SAL_UNUSED_PARAMETER ::test::codemaker::codemakertests::Struct2 const *) {
    return ::cppu::UnoType< ::test::codemaker::codemakertests::Struct2 >::get();
}
