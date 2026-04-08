#pragma once

#include "sal/config.h"

#include <cassert>

#include "com/sun/star/lang/ClassNotFoundException.hpp"
#include "com/sun/star/lang/IllegalAccessException.hpp"
#include "com/sun/star/uno/DeploymentException.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
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
#include "test/codemaker/codemakertests/XTest.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/Type.hxx"
#include "cppu/unotype.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#if defined ANDROID || defined IOS //TODO
#include <com/sun/star/lang/XInitialization.hpp>
#include <osl/detail/component-defines.h>
#endif

#if defined LO_URE_CURRENT_ENV && defined LO_URE_CTOR_ENV_test_dot_codemaker_dot_codemakertests_dot_S1 && (LO_URE_CURRENT_ENV) == (LO_URE_CTOR_ENV_test_dot_codemaker_dot_codemakertests_dot_S1) && defined LO_URE_CTOR_FUN_test_dot_codemaker_dot_codemakertests_dot_S1
extern "C" ::css::uno::XInterface * SAL_CALL LO_URE_CTOR_FUN_test_dot_codemaker_dot_codemakertests_dot_S1(::css::uno::XComponentContext *, ::css::uno::Sequence< ::css::uno::Any > const &);
#endif

namespace test { namespace codemaker { namespace codemakertests {

class S1 {
public:
    static ::css::uno::Reference< ::test::codemaker::codemakertests::XTest > create1(::css::uno::Reference< ::css::uno::XComponentContext > const & the_context) {
        assert(the_context.is());
        ::css::uno::Reference< ::test::codemaker::codemakertests::XTest > the_instance;
        try {
#if defined LO_URE_CURRENT_ENV && defined LO_URE_CTOR_ENV_test_dot_codemaker_dot_codemakertests_dot_S1 && (LO_URE_CURRENT_ENV) == (LO_URE_CTOR_ENV_test_dot_codemaker_dot_codemakertests_dot_S1) && defined LO_URE_CTOR_FUN_test_dot_codemaker_dot_codemakertests_dot_S1
            the_instance = ::css::uno::Reference< ::test::codemaker::codemakertests::XTest >(::css::uno::Reference< ::css::uno::XInterface >(static_cast< ::css::uno::XInterface * >((*LO_URE_CTOR_FUN_test_dot_codemaker_dot_codemakertests_dot_S1)(the_context.get(), ::css::uno::Sequence< ::css::uno::Any >())), ::SAL_NO_ACQUIRE), ::css::uno::UNO_QUERY);
            ::css::uno::Reference< ::css::lang::XInitialization > init(the_instance, ::css::uno::UNO_QUERY);
            if (init.is()) {
                init->initialize(::css::uno::Sequence< ::css::uno::Any >());
            }
#else
            the_instance = ::css::uno::Reference< ::test::codemaker::codemakertests::XTest >(the_context->getServiceManager()->createInstanceWithArgumentsAndContext( "test.codemaker.codemakertests.S1", ::css::uno::Sequence< ::css::uno::Any >(), the_context), ::css::uno::UNO_QUERY);
#endif
        } catch (const ::css::uno::RuntimeException &) {
            throw;
        } catch (const ::css::uno::Exception & the_exception) {
            throw ::css::uno::DeploymentException(::rtl::OUString("component context fails to supply service ") + "test.codemaker.codemakertests.S1" + " of type " + "test.codemaker.codemakertests.XTest" + ": " + the_exception.Message, the_context);
        }
        if (!the_instance.is()) {
            throw ::css::uno::DeploymentException(::rtl::OUString("component context fails to supply service ") + "test.codemaker.codemakertests.S1" + " of type " + "test.codemaker.codemakertests.XTest", the_context);
        }
        return the_instance;
    }

    static ::css::uno::Reference< ::test::codemaker::codemakertests::XTest > create2(::css::uno::Reference< ::css::uno::XComponentContext > const & the_context, const ::css::uno::Sequence< ::css::uno::Any >& create2) {
        assert(the_context.is());
        ::css::uno::Reference< ::test::codemaker::codemakertests::XTest > the_instance;
#if defined LO_URE_CURRENT_ENV && defined LO_URE_CTOR_ENV_test_dot_codemaker_dot_codemakertests_dot_S1 && (LO_URE_CURRENT_ENV) == (LO_URE_CTOR_ENV_test_dot_codemaker_dot_codemakertests_dot_S1) && defined LO_URE_CTOR_FUN_test_dot_codemaker_dot_codemakertests_dot_S1
        the_instance = ::css::uno::Reference< ::test::codemaker::codemakertests::XTest >(::css::uno::Reference< ::css::uno::XInterface >(static_cast< ::css::uno::XInterface * >((*LO_URE_CTOR_FUN_test_dot_codemaker_dot_codemakertests_dot_S1)(the_context.get(), create2)), ::SAL_NO_ACQUIRE), ::css::uno::UNO_QUERY);
        ::css::uno::Reference< ::css::lang::XInitialization > init(the_instance, ::css::uno::UNO_QUERY);
        if (init.is()) {
            init->initialize(the_arguments);
        }
#else
        the_instance = ::css::uno::Reference< ::test::codemaker::codemakertests::XTest >(the_context->getServiceManager()->createInstanceWithArgumentsAndContext( "test.codemaker.codemakertests.S1", create2, the_context), ::css::uno::UNO_QUERY);
#endif
        if (!the_instance.is()) {
            throw ::css::uno::DeploymentException(::rtl::OUString("component context fails to supply service ") + "test.codemaker.codemakertests.S1" + " of type " + "test.codemaker.codemakertests.XTest", the_context);
        }
        return the_instance;
    }

    static ::css::uno::Reference< ::test::codemaker::codemakertests::XTest > create3(::css::uno::Reference< ::css::uno::XComponentContext > const & the_context, const ::css::uno::Sequence< ::css::uno::Any >& S1) {
        assert(the_context.is());
        ::css::uno::Sequence< ::css::uno::Any > the_arguments(1);
        ::css::uno::Any* the_arguments_array = the_arguments.getArray();
        the_arguments_array[0] <<= S1;
        ::css::uno::Reference< ::test::codemaker::codemakertests::XTest > the_instance;
        try {
#if defined LO_URE_CURRENT_ENV && defined LO_URE_CTOR_ENV_test_dot_codemaker_dot_codemakertests_dot_S1 && (LO_URE_CURRENT_ENV) == (LO_URE_CTOR_ENV_test_dot_codemaker_dot_codemakertests_dot_S1) && defined LO_URE_CTOR_FUN_test_dot_codemaker_dot_codemakertests_dot_S1
            the_instance = ::css::uno::Reference< ::test::codemaker::codemakertests::XTest >(::css::uno::Reference< ::css::uno::XInterface >(static_cast< ::css::uno::XInterface * >((*LO_URE_CTOR_FUN_test_dot_codemaker_dot_codemakertests_dot_S1)(the_context.get(), the_arguments)), ::SAL_NO_ACQUIRE), ::css::uno::UNO_QUERY);
            ::css::uno::Reference< ::css::lang::XInitialization > init(the_instance, ::css::uno::UNO_QUERY);
            if (init.is()) {
                init->initialize(the_arguments);
            }
#else
            the_instance = ::css::uno::Reference< ::test::codemaker::codemakertests::XTest >(the_context->getServiceManager()->createInstanceWithArgumentsAndContext( "test.codemaker.codemakertests.S1", the_arguments, the_context), ::css::uno::UNO_QUERY);
#endif
        } catch (const ::css::uno::RuntimeException &) {
            throw;
        } catch (const ::css::lang::ClassNotFoundException &) {
            throw;
        } catch (const ::css::lang::IllegalAccessException &) {
            throw;
        } catch (const ::css::uno::Exception & the_exception) {
            throw ::css::uno::DeploymentException(::rtl::OUString("component context fails to supply service ") + "test.codemaker.codemakertests.S1" + " of type " + "test.codemaker.codemakertests.XTest" + ": " + the_exception.Message, the_context);
        }
        if (!the_instance.is()) {
            throw ::css::uno::DeploymentException(::rtl::OUString("component context fails to supply service ") + "test.codemaker.codemakertests.S1" + " of type " + "test.codemaker.codemakertests.XTest", the_context);
        }
        return the_instance;
    }

    static ::css::uno::Reference< ::test::codemaker::codemakertests::XTest > create4(::css::uno::Reference< ::css::uno::XComponentContext > const & the_context, ::sal_Int32 javamaker, ::sal_Int32 S1, ::sal_Int32 create4) {
        assert(the_context.is());
        ::css::uno::Sequence< ::css::uno::Any > the_arguments(3);
        ::css::uno::Any* the_arguments_array = the_arguments.getArray();
        the_arguments_array[0] <<= javamaker;
        the_arguments_array[1] <<= S1;
        the_arguments_array[2] <<= create4;
        ::css::uno::Reference< ::test::codemaker::codemakertests::XTest > the_instance;
        try {
#if defined LO_URE_CURRENT_ENV && defined LO_URE_CTOR_ENV_test_dot_codemaker_dot_codemakertests_dot_S1 && (LO_URE_CURRENT_ENV) == (LO_URE_CTOR_ENV_test_dot_codemaker_dot_codemakertests_dot_S1) && defined LO_URE_CTOR_FUN_test_dot_codemaker_dot_codemakertests_dot_S1
            the_instance = ::css::uno::Reference< ::test::codemaker::codemakertests::XTest >(::css::uno::Reference< ::css::uno::XInterface >(static_cast< ::css::uno::XInterface * >((*LO_URE_CTOR_FUN_test_dot_codemaker_dot_codemakertests_dot_S1)(the_context.get(), the_arguments)), ::SAL_NO_ACQUIRE), ::css::uno::UNO_QUERY);
            ::css::uno::Reference< ::css::lang::XInitialization > init(the_instance, ::css::uno::UNO_QUERY);
            if (init.is()) {
                init->initialize(the_arguments);
            }
#else
            the_instance = ::css::uno::Reference< ::test::codemaker::codemakertests::XTest >(the_context->getServiceManager()->createInstanceWithArgumentsAndContext( "test.codemaker.codemakertests.S1", the_arguments, the_context), ::css::uno::UNO_QUERY);
#endif
        } catch (const ::css::uno::RuntimeException &) {
            throw;
        } catch (const ::css::uno::Exception & the_exception) {
            throw ::css::uno::DeploymentException(::rtl::OUString("component context fails to supply service ") + "test.codemaker.codemakertests.S1" + " of type " + "test.codemaker.codemakertests.XTest" + ": " + the_exception.Message, the_context);
        }
        if (!the_instance.is()) {
            throw ::css::uno::DeploymentException(::rtl::OUString("component context fails to supply service ") + "test.codemaker.codemakertests.S1" + " of type " + "test.codemaker.codemakertests.XTest", the_context);
        }
        return the_instance;
    }

    static ::css::uno::Reference< ::test::codemaker::codemakertests::XTest > create5(::css::uno::Reference< ::css::uno::XComponentContext > const & the_context, ::sal_Bool p1, ::sal_Int8 p2, ::sal_Int16 p3, ::sal_uInt16 p4, ::sal_Int32 p5, ::sal_uInt32 p6, ::sal_Int64 p7, ::sal_uInt64 p8, float p9, double p10, ::sal_Unicode p11, const ::rtl::OUString& p12, const ::css::uno::Type& p13, const ::css::uno::Any& p14, ::test::codemaker::codemakertests::Enum2 p15, const ::test::codemaker::codemakertests::Struct1& p16, const ::css::uno::Reference< ::css::uno::XInterface >& p17, const ::css::uno::Reference< ::css::uno::XNamingService >& p18, ::sal_Bool t1, ::sal_Int8 t2, ::sal_Int16 t3, ::sal_uInt16 t4, ::sal_Int32 t5, ::sal_uInt32 t6, ::sal_Int64 t7, ::sal_uInt64 t8, float t9, double t10, ::sal_Unicode t11, const ::rtl::OUString& t12, const ::css::uno::Type& t13, const ::css::uno::Any& t14, ::test::codemaker::codemakertests::Enum2 t15, const ::test::codemaker::codemakertests::Struct1& t16, const ::css::uno::Reference< ::css::uno::XInterface >& t17, const ::css::uno::Reference< ::css::uno::XNamingService >& t18, const ::css::uno::Sequence< ::sal_Bool >& a1, const ::css::uno::Sequence< ::sal_Int8 >& a2, const ::css::uno::Sequence< ::sal_Int16 >& a3, const ::css::uno::Sequence< ::sal_uInt16 >& a4, const ::css::uno::Sequence< ::sal_Int32 >& a5, const ::css::uno::Sequence< ::sal_uInt32 >& a6, const ::css::uno::Sequence< ::sal_Int64 >& a7, const ::css::uno::Sequence< ::sal_uInt64 >& a8, const ::css::uno::Sequence< float >& a9, const ::css::uno::Sequence< double >& a10, const ::css::uno::Sequence< ::sal_Unicode >& a11, const ::css::uno::Sequence< ::rtl::OUString >& a12, const ::css::uno::Sequence< ::css::uno::Type >& a13, const ::css::uno::Sequence< ::css::uno::Any >& a14, const ::css::uno::Sequence< ::test::codemaker::codemakertests::Enum2 >& a15, const ::css::uno::Sequence< ::test::codemaker::codemakertests::Struct1 >& a16, const ::css::uno::Sequence< ::css::uno::Reference< ::css::uno::XInterface > >& a17, const ::css::uno::Sequence< ::css::uno::Reference< ::css::uno::XNamingService > >& a18, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_Bool > >& aa1, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_Int8 > >& aa2, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_Int16 > >& aa3, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_uInt16 > >& aa4, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_Int32 > >& aa5, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_uInt32 > >& aa6, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_Int64 > >& aa7, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_uInt64 > >& aa8, const ::css::uno::Sequence< ::css::uno::Sequence< float > >& aa9, const ::css::uno::Sequence< ::css::uno::Sequence< double > >& aa10, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_Unicode > >& aa11, const ::css::uno::Sequence< ::css::uno::Sequence< ::rtl::OUString > >& aa12, const ::css::uno::Sequence< ::css::uno::Sequence< ::css::uno::Type > >& aa13, const ::css::uno::Sequence< ::css::uno::Sequence< ::css::uno::Any > >& aa14, const ::css::uno::Sequence< ::css::uno::Sequence< ::test::codemaker::codemakertests::Enum2 > >& aa15, const ::css::uno::Sequence< ::css::uno::Sequence< ::test::codemaker::codemakertests::Struct1 > >& aa16, const ::css::uno::Sequence< ::css::uno::Sequence< ::css::uno::Reference< ::css::uno::XInterface > > >& aa17, const ::css::uno::Sequence< ::css::uno::Sequence< ::css::uno::Reference< ::css::uno::XNamingService > > >& aa18, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_Bool > >& at1, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_Int8 > >& at2, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_Int16 > >& at3, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_uInt16 > >& at4, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_Int32 > >& at5, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_uInt32 > >& at6, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_Int64 > >& at7, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_uInt64 > >& at8, const ::css::uno::Sequence< ::css::uno::Sequence< float > >& at9, const ::css::uno::Sequence< ::css::uno::Sequence< double > >& at10, const ::css::uno::Sequence< ::css::uno::Sequence< ::sal_Unicode > >& at11, const ::css::uno::Sequence< ::css::uno::Sequence< ::rtl::OUString > >& at12, const ::css::uno::Sequence< ::css::uno::Sequence< ::css::uno::Type > >& at13, const ::css::uno::Sequence< ::css::uno::Sequence< ::css::uno::Any > >& at14, const ::css::uno::Sequence< ::css::uno::Sequence< ::test::codemaker::codemakertests::Enum2 > >& at15, const ::css::uno::Sequence< ::css::uno::Sequence< ::test::codemaker::codemakertests::Struct1 > >& at16, const ::css::uno::Sequence< ::css::uno::Sequence< ::css::uno::Reference< ::css::uno::XInterface > > >& at17, const ::css::uno::Sequence< ::css::uno::Sequence< ::css::uno::Reference< ::css::uno::XNamingService > > >& at18) {
        assert(the_context.is());
        ::css::uno::Sequence< ::css::uno::Any > the_arguments(90);
        ::css::uno::Any* the_arguments_array = the_arguments.getArray();
        the_arguments_array[0] <<= p1;
        the_arguments_array[1] <<= p2;
        the_arguments_array[2] <<= p3;
        the_arguments_array[3] <<= p4;
        the_arguments_array[4] <<= p5;
        the_arguments_array[5] <<= p6;
        the_arguments_array[6] <<= p7;
        the_arguments_array[7] <<= p8;
        the_arguments_array[8] <<= p9;
        the_arguments_array[9] <<= p10;
        the_arguments_array[10] = ::css::uno::Any(&p11, ::cppu::UnoType< ::cppu::UnoCharType >::get());
        the_arguments_array[11] <<= p12;
        the_arguments_array[12] <<= p13;
        the_arguments_array[13] = p14;
        the_arguments_array[14] <<= p15;
        the_arguments_array[15] <<= p16;
        the_arguments_array[16] <<= p17;
        the_arguments_array[17] <<= p18;
        the_arguments_array[18] <<= t1;
        the_arguments_array[19] <<= t2;
        the_arguments_array[20] <<= t3;
        the_arguments_array[21] <<= t4;
        the_arguments_array[22] <<= t5;
        the_arguments_array[23] <<= t6;
        the_arguments_array[24] <<= t7;
        the_arguments_array[25] <<= t8;
        the_arguments_array[26] <<= t9;
        the_arguments_array[27] <<= t10;
        the_arguments_array[28] <<= t11;
        the_arguments_array[29] <<= t12;
        the_arguments_array[30] <<= t13;
        the_arguments_array[31] = t14;
        the_arguments_array[32] <<= t15;
        the_arguments_array[33] <<= t16;
        the_arguments_array[34] <<= t17;
        the_arguments_array[35] <<= t18;
        the_arguments_array[36] <<= a1;
        the_arguments_array[37] <<= a2;
        the_arguments_array[38] <<= a3;
        the_arguments_array[39] <<= a4;
        the_arguments_array[40] <<= a5;
        the_arguments_array[41] <<= a6;
        the_arguments_array[42] <<= a7;
        the_arguments_array[43] <<= a8;
        the_arguments_array[44] <<= a9;
        the_arguments_array[45] <<= a10;
        the_arguments_array[46] = ::css::uno::Any(&a11, ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoCharType > >::get());
        the_arguments_array[47] <<= a12;
        the_arguments_array[48] <<= a13;
        the_arguments_array[49] <<= a14;
        the_arguments_array[50] <<= a15;
        the_arguments_array[51] <<= a16;
        the_arguments_array[52] <<= a17;
        the_arguments_array[53] <<= a18;
        the_arguments_array[54] <<= aa1;
        the_arguments_array[55] <<= aa2;
        the_arguments_array[56] <<= aa3;
        the_arguments_array[57] <<= aa4;
        the_arguments_array[58] <<= aa5;
        the_arguments_array[59] <<= aa6;
        the_arguments_array[60] <<= aa7;
        the_arguments_array[61] <<= aa8;
        the_arguments_array[62] <<= aa9;
        the_arguments_array[63] <<= aa10;
        the_arguments_array[64] = ::css::uno::Any(&aa11, ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType< ::cppu::UnoCharType > > >::get());
        the_arguments_array[65] <<= aa12;
        the_arguments_array[66] <<= aa13;
        the_arguments_array[67] <<= aa14;
        the_arguments_array[68] <<= aa15;
        the_arguments_array[69] <<= aa16;
        the_arguments_array[70] <<= aa17;
        the_arguments_array[71] <<= aa18;
        the_arguments_array[72] <<= at1;
        the_arguments_array[73] <<= at2;
        the_arguments_array[74] <<= at3;
        the_arguments_array[75] <<= at4;
        the_arguments_array[76] <<= at5;
        the_arguments_array[77] <<= at6;
        the_arguments_array[78] <<= at7;
        the_arguments_array[79] <<= at8;
        the_arguments_array[80] <<= at9;
        the_arguments_array[81] <<= at10;
        the_arguments_array[82] <<= at11;
        the_arguments_array[83] <<= at12;
        the_arguments_array[84] <<= at13;
        the_arguments_array[85] <<= at14;
        the_arguments_array[86] <<= at15;
        the_arguments_array[87] <<= at16;
        the_arguments_array[88] <<= at17;
        the_arguments_array[89] <<= at18;
        ::css::uno::Reference< ::test::codemaker::codemakertests::XTest > the_instance;
        try {
#if defined LO_URE_CURRENT_ENV && defined LO_URE_CTOR_ENV_test_dot_codemaker_dot_codemakertests_dot_S1 && (LO_URE_CURRENT_ENV) == (LO_URE_CTOR_ENV_test_dot_codemaker_dot_codemakertests_dot_S1) && defined LO_URE_CTOR_FUN_test_dot_codemaker_dot_codemakertests_dot_S1
            the_instance = ::css::uno::Reference< ::test::codemaker::codemakertests::XTest >(::css::uno::Reference< ::css::uno::XInterface >(static_cast< ::css::uno::XInterface * >((*LO_URE_CTOR_FUN_test_dot_codemaker_dot_codemakertests_dot_S1)(the_context.get(), the_arguments)), ::SAL_NO_ACQUIRE), ::css::uno::UNO_QUERY);
            ::css::uno::Reference< ::css::lang::XInitialization > init(the_instance, ::css::uno::UNO_QUERY);
            if (init.is()) {
                init->initialize(the_arguments);
            }
#else
            the_instance = ::css::uno::Reference< ::test::codemaker::codemakertests::XTest >(the_context->getServiceManager()->createInstanceWithArgumentsAndContext( "test.codemaker.codemakertests.S1", the_arguments, the_context), ::css::uno::UNO_QUERY);
#endif
        } catch (const ::css::uno::RuntimeException &) {
            throw;
        } catch (const ::css::uno::Exception & the_exception) {
            throw ::css::uno::DeploymentException(::rtl::OUString("component context fails to supply service ") + "test.codemaker.codemakertests.S1" + " of type " + "test.codemaker.codemakertests.XTest" + ": " + the_exception.Message, the_context);
        }
        if (!the_instance.is()) {
            throw ::css::uno::DeploymentException(::rtl::OUString("component context fails to supply service ") + "test.codemaker.codemakertests.S1" + " of type " + "test.codemaker.codemakertests.XTest", the_context);
        }
        return the_instance;
    }

private:
    S1(); // not implemented
    S1(S1 &); // not implemented
    ~S1(); // not implemented
    void operator =(S1); // not implemented
};

} } }
